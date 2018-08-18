import numpy as np
from scipy import linalg
import pyroomacoustics as pra

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
