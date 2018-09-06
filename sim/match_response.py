import math
from scipy.signal import medfilt
import numpy as np
import pyroomacoustics as pra

from utilities import ShortTimeAverage, Plottable

def align_matrix(vec, ref):
    '''
    This routine will return the rotation matrix
    that will align vec with ref (3D vectors)

    Following this solution:
    https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d

    Parameters
    ----------
    vec: numpy array length 3
        The vector that we want to align
    ref: numpy array length 3
        The reference vector to which we want to align the other
    '''

    # make unit vectors
    vec = vec / np.linalg.norm(vec)
    ref = ref / np.linalg.norm(ref)

    c = np.inner(vec, ref)  # cos of angle
    I = np.eye(3)

    if np.abs(-1 - c) < 1e-10:
        return -I

    elif np.abs(1 - c) < 1e-10:
        return I

    else:
        v = np.cross(vec, ref)
        s = np.linalg.norm(v)  # sin of angle

        # skew-symmetric cross-product matrix of v
        V = np.array([
            [ 0,    -v[2],  v[1]],
            [ v[2],  0,    -v[0]],
            [-v[1],  v[0],  0   ],
            ])

        V2 = np.dot(V, V)

        return I + V + V2 * (1 - c) / s


class MatchResponse(Plottable):

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
        #self.array -= np.mean(array, axis=1, keepdims=True)
        self.array -= array[:,0,None]
        self.look_dir = look_dir / np.linalg.norm(look_dir)  # ensure unit vector
        self.beta = np.radians(beam_width)
        self.n_dir = n_dir
        self.nfft = nfft
        self.fs = fs
        self.c = c

        frac_cap = math.ceil(n_dir / 4)
        frac_back = n_dir - frac_cap

        # compute percentage of sphere surface covered by beamformer
        A = 2 * np.pi * (1. - np.cos(self.beta / 2))
        dense_dir = math.ceil((frac_cap * 4 * np.pi) / A)  # fill the cap with half the budget
        sparse_dir = math.ceil((frac_back * 4 * np.pi) / (4 * np.pi - A))  # fill the rest with the other half


        # create a grid for the steering vectors
        dense_grid = pra.doa.GridSphere(n_points=dense_dir)
        sparse_grid = pra.doa.GridSphere(n_points=sparse_dir)

        # align the grid with look direction
        # create a dense grid to fill the look direction spherical cap,
        # and a sparse one to cover the rest of the sphere
        R = align_matrix(np.r_[0., 0., 1.], self.look_dir)
        dense_pts = np.dot(R, dense_grid.cartesian)
        sparse_pts = np.dot(R, sparse_grid.cartesian)
        dense_I = np.argsort(np.arccos(np.dot(dense_pts.T, self.look_dir)))
        sparse_I = np.argsort(np.arccos(np.dot(sparse_pts.T, self.look_dir)))

        # The grid and the desired beamresponse
        grid = np.concatenate((
            dense_pts[:,dense_I[:n_dir//2]],
            sparse_pts[:,sparse_I[-n_dir//2:]],
            ), axis=1)

        # Compute the target response of the beamformer
        cos_beta = np.cos(self.beta / 2)
        q = (1 + cos_beta) / (1 - cos_beta)
        self.response = np.maximum(0., 1 - q + (1 + q) * np.dot(grid.T, self.look_dir))

        self.look_I = np.where(self.response > 0.)[0]

        # steering vectors have shaped (n_dir, n_freq, n_channels)
        self.f_hertz = np.arange(nfft // 2 + 1) / nfft * fs  # shape (n_freq,)
        delays = np.dot(grid.T, self.array) / c  # shape: (n_dir, n_channels,)
        self.steering_vectors = np.exp(-2j * np.pi * delays[:,None,:] * self.f_hertz[None,:,None])  # (n_dir, n_freq, n_channels,)

        # correlation of microphones in diffuse noise
        mic_dist = np.linalg.norm(self.array[:,1:], axis=0, keepdims=True)
        self.cc_diff = np.sinc(2. * np.pi * self.f_hertz[:,None] * mic_dist / self.c)  # (n_freq, n_channel - 1)

        self.gamma_diffuse = 0.01
        self.gamma_direct = 0.001
        
        self.estimation_len = 5
        self.estimates = {
                'cross_prod' : ShortTimeAverage(
                    self.estimation_len, 
                    lambda X : X[1:,0,None] * np.conj(X[1:,1:]) ,
                    ),
                'var' : ShortTimeAverage(
                    self.estimation_len,
                    lambda X : X[1:,:] * np.conj(X[1:,:]),
                    ),
                }

        self.frame_delay = self.estimation_len // 2
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

        # update all short time averages
        for name, v in self.estimates.items():
            v.update(X)

        # I believe for this, we should use a short time average, not instantaneous
        # compute correlation between channel 1 and other microphones
        cross_prod = self.estimates['cross_prod'].get()
        var = self.estimates['var'].get()
        a_avg = cross_prod / (var[:,1:] + 1e-7) # (n_freq - 1, n_chan - 1)
        cc = np.sqrt(var[:,1:] / (var[:,0,None] + 1e-7)) * a_avg

        # diffuse noise gain:
        ratio = self._compute_diffuse_coef(cc)
        alpha = 0.5
        ratio = np.maximum(0., ratio - alpha) / (1. - alpha)  # not sure why I need to adjust like this.
        self.store('dir/diff ratio', ratio)
        c = ratio * (1 - self.gamma_diffuse) + self.gamma_diffuse

        # compute correlation with steering vectors
        bf_ds = np.mean(X[None,1:,:] * self.steering_vectors[:,1:,:], axis=2)  # (n_dir, n_freq)

        # Find the maximum direction at each frequency (DOA)
        dir_index = np.argmax(np.abs(bf_ds), axis=0)
        dir_index = medfilt(dir_index, 5).astype(np.int)

        diffuse = np.sum(bf_ds[self.look_I,:] * self.response[self.look_I,None], axis=0)

        for f, (n, f_hz,) in enumerate(zip(dir_index, self.f_hertz)):

            if n in self.look_I:
                output[f] = ratio[f] * bf_ds[n,f] * self.response[n] + (1. - ratio[f]) * self.gamma_diffuse * diffuse[f]
            else:
                c = (1 - ratio[f]) * (self.gamma_diffuse - self.gamma_direct) + self.gamma_direct
                output[f] = c * diffuse[f]

        # high pass filter
        output[0] = 0.
        output[1] *= 0.1
        output[2] *= 0.5
        output[3] *= 0.7
        output[4] *= 0.9

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

        
