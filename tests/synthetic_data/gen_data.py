from __future__ import division

import numpy as np
from scipy import linalg
from scipy.signal import fftconvolve
import datetime
import os
import matplotlib.pyplot as plt

import pyroomacoustics as pra


def unit_vec(doa):
    """
    This function takes a 2D (phi) or 3D (phi,theta) polar coordinates
    and returns a unit vector in cartesian coordinates.

    :param doa: (ndarray) An (D-1)-by-N array where D is the dimension and
                N the number of vectors.

    :return: (ndarray) A D-by-N array of unit vectors (each column is a vector)
    """

    if doa.ndim != 1 and doa.ndim != 2:
        raise ValueError("DoA array should be 1D or 2D.")

    doa = np.array(doa)

    if doa.ndim == 0 or doa.ndim == 1:
        return np.array([np.cos(doa), np.sin(doa)])

    elif doa.ndim == 2 and doa.shape[0] == 1:
        return np.array([np.cos(doa[0]), np.sin(doa[0])])

    elif doa.ndim == 2 and doa.shape[0] == 2:
        s = np.sin(doa[1])
        return np.array([s * np.cos(doa[0]), s * np.sin(doa[0]), np.cos(doa[1])])


def gen_far_field_ir(doa, R, fs):
    """
    This function generates the impulse responses for all microphones for
    K sources in the far field.

    :param doa: (nd-array) The sources direction of arrivals. This should
                be a (D-1)xK array where D is the dimension (2 or 3) and K
                is the number of sources
    :param R: the locations of the microphones
    :param fs: sampling frequency

    :return ir: (ndarray) A KxMxL array containing all the fractional delay
                filters between each source (axis 0) and microphone (axis 1)
                L is the length of the filter
    """

    # make sure these guys are nd-arrays
    doa = np.array(doa)

    if doa.ndim == 0:
        doa = np.array([[doa]])

    elif doa.ndim == 1:
        doa = np.array([doa])

    # the number of microphones
    M = R.shape[1]
    dim = R.shape[0]

    # the number of sources
    K = doa.shape[1]

    # convert the spherical coordinates to unit propagation vectors
    p_vec = -unit_vec(doa)

    # the delays are the inner product between unit vectors and mic locations
    # set zero delay at earliest microphone
    delays = np.dot(p_vec.T, R) / pra.constants.get('c')
    delays -= delays.min()

    # figure out the maximal length of the impulse responses
    L = pra.constants.get('frac_delay_length')
    t_max = delays.max()
    D = int(L + np.ceil(np.abs(t_max * fs)))

    # the impulse response filter bank
    fb = np.zeros((K, M, D))

    # create all the impulse responses
    for k in xrange(K):
        for m in xrange(M):
            t = delays[k, m]
            delay_s = t * fs
            delay_i = int(np.round(delay_s))
            delay_f = delay_s - delay_i
            fb[k, m, delay_i:delay_i + (L - 1) + 1] += pra.fractional_delay(delay_f)

    return fb

def gen_white_noise_at_mic_stft(phi_ks, source_signals, mic_array_coord, noise_power, fs):
    """
    generate microphone signals with short time Fourier transform
    :param phi_ks: azimuth of the acoustic sources
    :param source_signals: speech signals for each arrival angle, one per row
    :param mic_array_coord: x and y coordinates of the microphone array
    :param noise_power: the variance of the microphone noise signal
    :param fs: sampling frequency
    :return: y: received signal at microphones
             
    """
    K = source_signals.shape[0]  # number of point sources
    num_mic = mic_array_coord.shape[1]  # number of microphones

    # Generate the impulse responses for the array and source directions
    impulse_response = gen_far_field_ir(phi_ks, mic_array_coord, fs)

    # Now generate all the microphone signals
    y = np.zeros((num_mic, source_signals.shape[1] + impulse_response.shape[2] - 1), dtype=np.float32)
    for src in xrange(K):
        for mic in xrange(num_mic):
            y[mic] += fftconvolve(impulse_response[src, mic], source_signals[src])

    return y

if __name__ == "__main__":

    from scipy.signal import butter, lfilter

    # import array coordinates
    R = []
    with open("../../CONFIG", 'r') as f:
        lines = f.readlines()
        for line in lines:

            vals = line.split(' ')

            if vals[0][0] == '#':
                continue

            print vals
            R.append([float(v) for v in vals[1:]])

    R = np.array(R).T


    doa = np.array([[np.pi/2], [np.pi/2.]])

    fs = 16000
    SNR = 20
    T = 10

    sigma2 = 10**(SNR / 10)

    N = int(T * fs)
    src_signal = np.random.randn(doa.shape[1],N)

    b,a = butter(6, 0.5)
    src_signal = lfilter(b, a, src_signal)

    y = gen_white_noise_at_mic_stft(doa, src_signal, R, sigma2, fs)

    with open('test_signal.raw', 'wb') as f:
        for n in range(y.shape[1]):
            for ch in range(y.shape[0]):
                f.write(y[ch,n])
        f.close()
    

