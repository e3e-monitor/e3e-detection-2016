import numpy as np
from scipy import linalg
import pyroomacoustics as pra

from utilities import Plottable

def calibration(signal):
    '''
    This routine will compute the fixed beamforming weights
    from the STFT of a recorded calibration signal.

    Parameters
    ----------
    signal: array_like (nframes, nfrequencies, nchannels)
        The STFT of the recorded calibration signal

    Returns
    -------
    array_like (nfrequencies, nchannels)
        The beamforming weights (in frequency domain)
    '''

    nframes, nfreq, nchan = signal.shape

    weights = np.zeros(signal.shape[1:], dtype=signal.dtype)

    for f in range(nfreq):

        R = np.dot(signal[:,f,:].T, np.conj(signal[:,f,:])) / nframes
        val, vec = linalg.eigh(R, eigvals=(R.shape[0]-1,R.shape[0]-1))
        weights[f,:] = vec[:,0]

    # normalize
    weights /= np.linalg.norm(weights, axis=1, keepdims=True)

    return weights



def project_null(X, weights):
    '''
    Projects the input signal into the null space of the provided weights

    Parameters
    ----------
    X: array_like (nfrequencies, nchannels)
        The input vector
    weights: array_like (nfrequencies, nchannels)
        The fixed beamforming weights (should be row-wise unit norm)
    '''

    nfreq, nchan = X.shape
    out = np.zeros_like(X)

    for f in range(nfreq):
        out[f,:] = X[f,:] - np.inner(np.conj(weights[f,:]), X[f,:]) * weights[f,:]

    return out


class GSC(Plottable):

    def __init__(self, calibration_signal, step_size, pb_ff, nfft, shift=None, win=None):

        self.step_size = step_size  # for the LMS
        self.pb_ff = pb_ff  # forgetting factor for the projection back
        self.nfft = nfft
        self.nchannel = calibration_signal.shape[1]

        if shift is None:
            shift = nfft // 2

        if win is None:
            win = pra.hann(nfft)

        # Compute the fixed beamformer
        X = pra.transform.analysis(calibration_signal, nfft, shift, win=win)
        self.fixed_weights = calibration(X)

        # gsc adaptive weights
        self.adaptive_weights = np.zeros_like(self.fixed_weights)

        # projection back weights
        self.pb_den = np.ones(self.fixed_weights.shape[0], dtype=self.fixed_weights.dtype)
        self.pb_num = np.ones(self.fixed_weights.shape[0], dtype=np.float)


    def process(self, X):

        assert X.shape[0] == self.nfft // 2 + 1
        assert X.shape[1] == self.nchannel
        output = np.zeros(self.nfft // 2 + 1, dtype=X.dtype)

        # the fixed branch of the GSC
        out_fixed_bf = np.sum(np.conj(self.fixed_weights) * X, axis=1)

        # Now run the online projection back (seems more natural to do it on the fixed branch)
        self.pb_den = ( self.pb_ff * self.pb_den 
                + (1 - self.pb_ff) * np.conj(out_fixed_bf) * X[:,0] )
        self.pb_num = ( self.pb_ff * self.pb_num
                + (1 - self.pb_ff) * np.conj(out_fixed_bf) * out_fixed_bf )
        out_fixed_bf *= (self.pb_den / self.pb_num)

        # the adaptive branch of the GSC
        noise_ss_signal = project_null(X, self.fixed_weights)
        noise_ss_norm = np.sum(noise_ss_signal * np.conj(noise_ss_signal), axis=1)
        out_adaptive_bf = np.sum(np.conj(self.adaptive_weights) * noise_ss_signal, axis=1)

        # compute the error signal and update the beamformer
        output[:] = out_fixed_bf - out_adaptive_bf
        mu = (self.step_size / noise_ss_norm[:,None])
        self.adaptive_weights +=  mu * noise_ss_signal * output[:,None]

        return output
