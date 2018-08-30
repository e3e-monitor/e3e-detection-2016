import math
from scipy.signal import medfilt
import numpy as np
import pyroomacoustics as pra

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


class MatchResponse:

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
        self.array -= np.mean(array, axis=1, keepdims=True)
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
        self.steering_vectors = np.exp(2j * np.pi * delays[:,None,:] * self.f_hertz[None,:,None])  # (n_dir, n_freq, n_channels,)

        self.gamma_diffuse = 0.1
        self.gamma_direct = 0.001


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

        # compute correlation with steering vectors
        cc = np.mean(X[None,:,:] * np.conj(self.steering_vectors), axis=2)  # (n_dir, n_freq)
        cc_norm = np.mean(X[None,:,:] / (1e-7 + np.abs(X)) * np.conj(self.steering_vectors), axis=2)  # (n_dir, n_freq)

        # Find the maximum direction at each frequency
        dir_index = np.argmax(np.abs(cc), axis=0)
        dir_index = medfilt(dir_index, 5).astype(np.int)

        diffuse = np.sum(cc[self.look_I,:] * self.response[self.look_I,None], axis=0)

        for f, (n, f_hz,) in enumerate(zip(dir_index, self.f_hertz)):

            # estimate proportion of direct and diffuse component
            ratio = (np.abs(cc_norm[n,f]) - (1 / np.sqrt(X.shape[1]))) / (1 - 1 / np.sqrt(X.shape[1]))
            if np.abs(ratio) > 1.:
                ratio = 1.
            elif ratio < 0.:
                ratio = 0.

            if n in self.look_I:
                output[f] = ratio * cc[n,f] * self.response[n] + (1. - ratio) * self.gamma_diffuse * diffuse[f]
            else:
                c = (1 - ratio) * (self.gamma_diffuse - self.gamma_direct) + self.gamma_direct
                output[f] = c * diffuse[f]

        # high pass filter
        output[0] = 0.
        output[1] *= 0.1
        output[2] *= 0.5
        output[3] *= 0.7
        output[4] *= 0.9

        return output
