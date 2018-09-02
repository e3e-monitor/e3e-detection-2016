import math
from scipy.signal import medfilt
import numpy as np
import pyroomacoustics as pra

class ShapeResponse:

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
        self.gain = lambda v: np.maximum(0., 1 - q + (1 + q) * np.dot(-v, self.look_dir))

        # steering vectors have shaped (n_dir, n_freq, n_channels)
        self.f_hertz = np.arange(nfft // 2 + 1) / nfft * fs  # shape (n_freq,)

        mic_dist = np.linalg.norm(self.array[:,1:], axis=0, keepdims=True)
        self.cc_diff = np.sinc(2. * np.pi * self.f_hertz[:,None] * mic_dist / self.c)  # (n_freq, n_channel - 1)

        self.gamma_diffuse = 0.1
        self.gamma_direct = 0.001

        self.grid = pra.doa.GridSphere(n_points=n_dir).cartesian
        delays = np.dot(self.grid.T, self.array) / c  # shape: (n_dir, n_channels,)
        self.steering_vectors = np.exp(2j * np.pi * delays[:,None,:] * self.f_hertz[None,:,None])  # (n_dir, n_freq, n_channels,)
        self.gain_values = self.gain(self.grid.T)

        self.data = {}

        # used for xcorr short time averaging
        self.buffered_estimates = {
                'cross_prod' : {
                    'buf' : [],
                    'val' : None,
                    'f' : lambda X : X[1:,0,None] * np.conj(X[1:,1:]) ,
                    },
                'var' : {
                    'buf' : [],
                    'val' : None,
                    'f' : lambda X : X[1:,:] * np.conj(X[1:,:]),
                    }
                }
        self.estimation_len = 3  # number of frames to use in short time average


    def _update_estimates(self, X):
        '''
        This function updates all the estimates that use a short time averaging
        '''

        for name, v in self.buffered_estimates.items():

            v['buf'].insert(0, v['f'](X))

            # initialize the output variable
            if v['val'] is None:
                v['val'] = np.zeros_like(v['buf'][0])

            # remove oldest element
            if len(v['buf']) > self.estimation_len:
                v['val'] -= v['buf'][-1]
                v['buf'].pop()

            # add newest element
            v['val'] += v['buf'][0]


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

        # I believe for this, we should use a short time average, not instantaneous
        # compute correlation between channel 1 and other microphones
        a = X[1:,0,None] * np.conj(X[1:,1:]) / (np.abs(X[1:,1:]) ** 2 + 1e-7)  # (n_freq - 1, n_chan - 1)
        cc = (np.abs(X[1:,1:]) / np.abs(X[1:,0,None]) + 1e-7) * a

        # spatial spectrum and doa estimation
        sspec = np.mean((X[None,1:,:] / (1e-7 + np.abs(X[None,1:,:]))) * self.steering_vectors[:,1:,:], axis=2)  # (n_dir, n_freq - 1,)
        sspec_max = np.max(np.abs(sspec), axis=0)  # (n_freq - 1,)
        dir_index = np.argmax(np.abs(sspec), axis=0)  # (n_freq - 1,)
        doa = np.array([self.grid[:,n] for n in dir_index]).T  # (3, n_freq - 1,)

        azimuth = np.arctan2(doa[1,:], doa[0,:])
        self.store('azimuth', azimuth)
        colatitude = np.arccos(doa[2,:])
        self.store('colatitude', colatitude)

        # the directional gain
        #gain_dir = 1. - self.gain(doa.T)
        gain_dir = 1. - np.array([self.gain_values[n] for n in dir_index])
        #gain_dir[gain_dir > 0.95] = 0.95

        # diffuse noise gain:
        # Here we want to assess how "directional" the field is
        # when directional, then the CC coeff are just complex exponentials (steering vectors)
        # when diffuse, then the CC follows sinc(2 * pi * d / c)
        # How can we do that ?
        sigma = np.mean(np.abs(cc), axis=1)
        self.store('coherence', sigma)
        m = np.mean(np.abs(self.cc_diff[1:,:]), axis=1)
        self.store('m', m)
        ratio = np.maximum(0, sigma - m) / (1 - m)
        c = ratio * (1 - self.gamma_diffuse) + self.gamma_diffuse

        # First out
        output[1:] = c * (X[1:,0] - gain_dir * np.mean(a * X[1:,1:], axis=1))

        # high pass filter
        output[0] = 0.
        output[1] *= 0.1
        output[2] *= 0.5
        output[3] *= 0.7
        output[4] *= 0.9

        self.store('output', 20. * np.log10(np.abs(output) + 1e-7))

        return output


    def store(self, name, data):
        ''' Store some data for later plotting '''

        if name in self.data:
            self.data[name].append(data)
        else:
            self.data[name] = [data]


    def plot(self):
        ''' Plot debugging data '''
        import matplotlib.pyplot as plt

        for name, a in self.data.items():
            plt.figure()
            plt.title(name)
            plt.xlabel('Time')
            plt.ylabel('Frequency')
            plt.imshow(np.array(a).T, origin='lower', aspect='auto')
            plt.colorbar()
        plt.show()
