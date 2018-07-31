Pyramic Demo at IWAENC 2018
===========================

We will use [Pyramic](https://github.com/LCAV/Pyramic) for a demo at IWAENC in Tokyo.

### Compile and run tests

    make tests
    ./tests/test_stft
    ./tests/test_stft_speed


### Use the STFT

    #include "src/e3e_detection.h"
    #include "src/stft.h"

    // We'll need these to store sample, etc
    float audio_input_buffer[FRAME_SIZE];
    float audio_output_buffer[FRAME_SIZE];

    // We only need a pointer for this one, the array
    // will be allocated by the STFT engine
    e3e_complex *spectrum;

    STFT engine(FRAME_SIZE, FFT_SIZE, ZB, ZF, CHANNELS, WFLAG);

    while (1)
    {
      // get FRAME_SIZE new audio samples
      get_new_audio_samples(audio_input_buffer, FRAME_SIZE);

      // Analyze them. Spectrum contains (FFT_SIZE / 2 + 1) complex numbers
      spectrum = engine.analysis(audio_input_buffer);

      // Run some processing on spectrum
      ...

      // Now synthesize into the output buffer
      engine.synthesis(audio_output_buffer);

      // Send the processed samples to the output
      play_audio_samples(audio_output_buffer, FRAME_SIZE);
    }


The docstring for the STFT constructor

    STFT(int shift, int fft_size, int zpb, int zpf, int channels, int flags)
    /**
      Constructor for the STFT engine.

      @param shift The frame shift
      @param fft_size The size of the FFT (including padding, if any)
      @param zpb The zero-padding at the end of the array
      @param zpf The zero-padding at the front of the array
      @param channels The number of channels
      @param flags Specify which window scheme to use (STFT_NO_WINDOW, STFT_WINDOW_ANALYSIS, STFT_WINDOW_BOTH)
      */

### Dependencies

To run the code with matrix creator, one needs to install

* FFTW
