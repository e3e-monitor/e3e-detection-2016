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

from gsc import calibration, project_null, GSC, GSC_Newton

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
source_powers = [1., 1., 1., 1., 0.1]
nfft = 2048
shift = nfft // 2

# gsc parameters
step_size = 0.01
pb_ff = 0.9  # projection back forgetting factor


#################################
# Create/Read the sound signals #

# The calibration signal is a random binary sequence
T_calib = 10.  # seconds
calib_seq = np.random.choice([-1.,1.], size=int(T_calib * fs))

# Now import the speech
source_signals = []
for fn, pwr in zip(source_files, source_powers):
    _, sig = wavfile.read(fn)
    sig = sig.astype(np.float)
    sig *= np.sqrt(pwr) / np.std(sig)
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

#gsc = GSC(room.mic_array.signals.T, step_size, pb_ff, nfft)
gsc = GSC_Newton(room.mic_array.signals.T, 0.005, pb_ff, nfft, 8)

output_signal = np.zeros(recording.shape[0], dtype=recording.dtype)

# processing loop
n = 0
while n + shift < recording.shape[0]:

    newframe = recording[n:n+shift,:]
    X = stft_input.analysis(newframe)

    out_frame = gsc.process(X)

    # synthesize the output signal
    output_signal[n:n+shift] = stft_output.synthesis(out_frame)

    n += shift

m = 0.85 / np.max(np.abs(recording[:,0]))
wavfile.write('output_mic1.wav', fs, recording[:,0] * m)
wavfile.write('output_gsc.wav', fs, output_signal * m)

