Simulation of GSC
=================

This folder contains python code simulating the generalized sidelobe canceller
that we are planning to implement in Pyramic.

Run the simulation
------------------

    python ./sim_gsc.py

The outputs are two `wav` files

    output_mic1.wav  # the signal at microphone 1 of pyramic
    output_gsc.wav  # the output of the GSC beamformer
