'''
Simulation of Pyramic Demo at IWAENC 2018
=========================================

The goal is to implement a low complexity Generalized Sidelobe Canceller (GSC)
working real-time on 48 channels.
'''
import json
import numpy as np
from scipy.io import wavfile
import pyroomacoustics as pra

from gsc import calibration, project_null

fs = 16000
room_size = [9.9, 7.5, 3.1]
array_location = [5.5, 5.3, 1.1]
source_locations = [
        [2.5, 2.3, 1.5],
        [ 2.1, 5.7,  1.8 ],
        [ 4.8, 2.95, 1.65 ],
        [ 6.5, 3.5,  1.2],
        [ 8.2, 6., 1.7],
        ]
source_files = [
        'long_speech.wav',
        'fq_sample3.wav',
        'fq_sample4.wav',
        'fq_sample5.wav',
        'fan_noise_short.wav',
        ]
source_delays = [ 0., 1.5, 3.4, 7.5, 0. ]
source_vars = [1., 1., 1., 1., 0.1]
nfft = 2048
shift = nfft / 2

# gsc parameters
step_size = 0.05
pb_ff = 0.99  # projection back forgetting factor


#################################
# Create/Read the sound signals #

# The calibration signal is a random binary sequence
T_calib = 10.  # seconds
calib_seq = np.random.choice([-1.,1.], size=int(T_calib * fs))

# Now import the speech
source_signals = []
for fn in source_files:
    _, sig = wavfile.read(fn)
    sig = sig.astype(np.float)
    sig /= np.std(sig)
    source_signals.append(sig)


#####################
# Setup of the Room #

# Create the room (target RT60 is 0.5 s)
room = pra.ShoeBox(
        [9.9, 7.5, 3.1],
        fs=fs,
        absorption=0.25,
        max_order=25,
        )

# Read in the pyramic microphone locations
with open('pyramic.json') as f:
    data = json.load(f)
    array = np.array(data['pyramic']).T

# Position the array in the room
array -= array.mean(axis=1, keepdims=True)
array += np.array([[5.5, 5.3, 1.1]]).T
room.add_microphone_array(pra.MicrophoneArray(array, room.fs))


####################
# Prepare the STFT #

awin = pra.hann(nfft)
swin = pra.transform.compute_synthesis_window(awin, shift)
stft_input = pra.transform.STFT(
        nfft,
        shift,
        analysis_window=awin,
        synthesis_window=swin,
        channels=array.shape[1],
        )
stft_output = pra.transform.STFT(
        nfft,
        shift,
        analysis_window=awin,
        synthesis_window=swin,
        channels=1,
        )
        
###############
# CALIBRATION #

room.add_source(source_locations[0], signal=calib_seq)
room.simulate()
room.sources.pop()  # remove the calibration source

X = pra.transform.analysis(room.mic_array.signals.T, nfft, shift, win=awin)
fixed_weights = calibration(X)


##############
# Simulation #

room.rir = None
room.visibility = None
for loc, signal, delay in zip(source_locations, source_signals, source_delays):
    room.add_source(loc, signal=signal, delay=delay)
room.simulate()
recording = room.mic_array.signals.T

###############
# Run the GSC #

output_signal = np.zeros(recording.shape[0], dtype=recording.dtype)

# gsc adaptive weights
adaptive_weights = np.zeros_like(fixed_weights)

# projection back weights
pb_den = np.ones(fixed_weights.shape[0], dtype=fixed_weights.dtype)
pb_num = np.ones(fixed_weights.shape[0], dtype=np.float)

# processing loop
n = 0
while n + shift < recording.shape[0]:

    newframe = recording[n:n+shift,:]
    X = stft_input.analysis(newframe)

    # the fixed branch of the GSC
    out_fixed_bf = np.sum(np.conj(fixed_weights) * X, axis=1)

    # the adaptive branch of the GSC
    noise_ss_signal = project_null(X, fixed_weights)
    noise_ss_norm = np.sum(noise_ss_signal * np.conj(noise_ss_signal), axis=1)
    out_adaptive_bf = np.sum(np.conj(adaptive_weights) * noise_ss_signal, axis=1)

    # compute the error signal and update the beamformer
    out_frame = out_fixed_bf - out_adaptive_bf
    adaptive_weights += (step_size / noise_ss_norm[:,None]) * noise_ss_signal * out_frame[:,None]

    # Now run the online projection back
    pb_den = pb_ff * pb_den + (1 - pb_ff) * np.conj(out_frame) * X[:,0]
    pb_num = pb_ff * pb_num + (1 - pb_ff) * np.conj(out_frame) * out_frame
    out_frame *= (pb_den / pb_num)

    # synthesize the output signal
    output_signal[n:n+shift] = stft_output.synthesis(out_frame)

    n += shift

wavfile.write('output_mic1.wav', fs, pra.normalize(recording[:,0]) * 0.85)
wavfile.write('output_gsc.wav', fs, pra.normalize(output_signal) * 0.85)
