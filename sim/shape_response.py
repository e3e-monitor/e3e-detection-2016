import math
from scipy.signal import medfilt
import numpy as np
import pyroomacoustics as pra

from utilities import ShortTimeAverage, Plottable

class ShapeResponse(Plottable):

    def __init__(self, array, look_dir, beam_width, n_dir, nfft, fs, c):
        '''
        Trying out a kind of non-linear beamformer

        Parameters
        ----------
        array: numpy array (3, nchannels)
            The microphone locations
        look_dir: numpy array (3,)
            A vector pointing towards the look direction
        beam_width: float
            The beamwidth in degrees
        n_dir: int
            The number of steering vectors to use
        nfft: int
            The length of the FFT used
        fs: int
            The sampling frequency
        c: float
            The speed of sound
        '''

        Plottable.__init__(self)  # parent constructor

        self.array = array
        self.array -= array[:,0,None]
        self.look_dir = look_dir / np.linalg.norm(look_dir)  # ensure unit vector
        self.beta = np.radians(beam_width)
        self.n_dir = n_dir
        self.nfft = nfft
        self.fs = fs
        self.c = c


        # Pseudo-inverse to estimate DOA from delays
        self.A = self.array[:,1:] / self.c
        self.pinv = np.dot(np.linalg.inv(np.dot(self.A, self.A.T)), self.A)

        # The gain function of the beamformer
        cos_beta = np.cos(self.beta / 2)
        q = (1 + cos_beta) / (1 - cos_beta)
        self.gain = lambda v: np.maximum(0., 0.5 * (1 - q + (1 + q) * np.dot(-v, self.look_dir)))

        # steering vectors have shaped (n_dir, n_freq, n_channels)
        self.f_hertz = np.arange(nfft // 2 + 1) / nfft * fs  # shape (n_freq,)

        mic_dist = np.linalg.norm(self.array[:,1:], axis=0, keepdims=True)
        self.cc_diff = np.sinc(2. * np.pi * self.f_hertz[:,None] * mic_dist / self.c)  # (n_freq, n_channel - 1)

        self.gamma_diffuse = 0.05
        self.gain_limit = 0.97

        self.grid = pra.doa.GridSphere(n_points=n_dir).cartesian
        delays = np.dot(self.grid.T, self.array) / c  # shape: (n_dir, n_channels,)
        self.steering_vectors = np.exp(2j * np.pi * delays[:,None,:] * self.f_hertz[None,:,None])  # (n_dir, n_freq, n_channels,)
        self.gain_values = self.gain(self.grid.T)

        # used for xcorr short time averaging
        self.estimation_len = 5  # number of frames to use in short time average
        self.estimates = {
                'cross_prod' : ShortTimeAverage(
                    self.estimation_len, 
                    lambda X : X[1:,0,None] * np.conj(X[1:,1:]) ,
                    ),
                'var' : ShortTimeAverage(
                    self.estimation_len,
                    lambda X : X[1:,:] * np.conj(X[1:,:]),
                    ),
                'spatial_spectrum' : ShortTimeAverage(
                    self.estimation_len,
                    lambda X : np.mean(
                        (X[None,1:,:] / (1e-7 + np.abs(X[None,1:,:])))
                        * self.steering_vectors[:,1:,:],
                        axis=2),
                    ),
                }

        self.frame_delay = 2
        self.frame_pipe = []

    def process(self, X):
        '''
        Process a new frame of the input signal

        Parameters
        ----------
        X: numpy array (n_freq, n_channels)
            The new STFT frame to process
        '''

        assert X.shape[0] == self.nfft // 2 + 1
        assert X.shape[1] == self.array.shape[1]

        output = np.zeros(self.nfft // 2 + 1, dtype=X.dtype)

        omega = 2 * np.pi * self.f_hertz[1:,None]

        # update all short time averages
        for name, v in self.estimates.items():
            v.update(X)

        # I believe for this, we should use a short time average, not instantaneous
        # compute correlation between channel 1 and other microphones
        cross_prod = self.estimates['cross_prod'].get()
        var = self.estimates['var'].get()
        a_avg = cross_prod / (var[:,1:] + 1e-7) # (n_freq - 1, n_chan - 1)
        cc = np.sqrt(var[:,1:] / (var[:,0,None] + 1e-7)) * a_avg

        # compute the cancellation factors
        cross_prod_last = self.estimates['cross_prod'].last()
        var_last = self.estimates['var'].last()
        a = cross_prod_last / (var_last[:,1:] + 1e-7)

        # spatial spectrum and doa estimation
        sspec = self.estimates['spatial_spectrum'].get()  # (n_dir, n_freq - 1,)
        sspec_max = np.max(np.abs(sspec), axis=0)  # (n_freq - 1,)
        dir_index = np.argmax(np.abs(sspec), axis=0)  # (n_freq - 1,)
        dir_index = medfilt(dir_index, 5).astype(np.int)
        doa = np.array([self.grid[:,n] for n in dir_index]).T  # (3, n_freq - 1,)

        azimuth = np.arctan2(doa[1,:], doa[0,:])
        self.store('azimuth', azimuth)
        colatitude = np.arccos(doa[2,:])
        self.store('colatitude', colatitude)

        # the directional gain
        #gain_dir = 1. - self.gain(doa.T)
        gain_dir = 1. - np.array([self.gain_values[n] for n in dir_index])
        gain_dir[gain_dir > self.gain_limit] = self.gain_limit
        self.store('gain', gain_dir)

        # diffuse noise gain:
        ratio = self._compute_diffuse_coef(cc)
        ratio = np.maximum(0., ratio - 0.4) / 0.6  # not sure why I need to adjust like this.
        self.store('dir/diff ratio', ratio)
        c = ratio * (1 - self.gamma_diffuse) + self.gamma_diffuse

        # First out
        #output[1:] = c * (X[1:,0] - gain_dir * np.mean(a * X[1:,1:], axis=1))
        output[1:] = ratio * (X[1:,0] - gain_dir * np.mean(a * X[1:,1:], axis=1)) + (1 - ratio) * 0.01 * X[1:,0]

        '''
        # high pass filter
        output[0] = 0.
        output[1] *= 0.1
        output[2] *= 0.5
        output[3] *= 0.7
        output[4] *= 0.9
        '''

        self.store('output', 20. * np.log10(np.abs(output) + 1e-7))

        # We might want to delay by a few frames
        self.frame_pipe.insert(0, output)
        if len(self.frame_pipe) > self.frame_delay:
            return self.frame_pipe.pop()
        else:
            return np.zeros(self.nfft // 2 + 1, dtype=X.dtype)

    def _compute_diffuse_coef(self, cc):

        diff_coef = []

        for f in range(1, self.nfft // 2 + 1):
            v_dir = np.ones(self.cc_diff[f,:].shape)
            v_diff = np.abs(self.cc_diff[f,:])
            v_test = np.abs(cc[f-1,:])

            A = np.linalg.norm(v_dir - v_test)
            B = np.linalg.norm(v_test - v_diff)
            C = np.linalg.norm(v_dir - v_diff)

            # Area using Heron's formula
            s = 0.5 * (A + B + C)
            area = np.sqrt(s * (s - A) * (s - B) * (s - C))

            # height
            H = 2 * area / C

            C1 = np.sqrt(B ** 2 - H ** 2)
            C2 = np.sqrt(A ** 2 - H ** 2)

            if C2 > C:
                r = 0.
            elif C1 > C:
                r = 1.
            else:
                r = C1 / C

            diff_coef.append(r)

        return np.array(diff_coef)

        
