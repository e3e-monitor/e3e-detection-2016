Pyramic Demo at IWAENC 2018
===========================

We will use [Pyramic](https://github.com/LCAV/Pyramic) for a demo at IWAENC in Tokyo.

### Compile and run tests

    make tests
    ./tests/bin/test_stft
    ./tests/bin/test_stft_speed


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

Install compile tools

    apt-get install build-essential gfortran manpages-dev

To run the code with matrix creator, one needs to install

* FFTW

#### Compile FFTW

Compile FFTW on ARM with floating point NEON support

    apt-get install gfortran

    wget http://www.fftw.org/fftw-3.3.4.tar.gz
    tar xzfv fftw-3.3.4.tar.gz
    cd fftw-3.3.4
    ./configure --enable-single --enable-neon ARM_CPU_TYPE=<ARCH> --enable-shared
    make
    make install
    ldconfig

Replace <ARCH> by

* cortex-a8 for BBB
* cortex-a9 for DE1-SoC

#### Install GCC with std14 support

The code uses some C++14 specific commands and requires g++-4.9 minimum to be compiled.
The current Pyramic image is Ubuntu 14.04 which requires some patching to get the right compiler.

[source](http://scholtyssek.org/blog/2015/06/11/install-gcc-with-c14-support-on-ubuntumint/)

    # install the add-apt-repository command
    apt-get install software-properties-common python-software-properties

    # now try to upgrade g++
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install g++-4.9

Set the default gcc version used

    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 10
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 20
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 10
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 20

    update-alternatives --set cc /usr/bin/gcc
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
    update-alternatives --set c++ /usr/bin/g++
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30

Check that version 4.9 is called when running

    g++ --version
