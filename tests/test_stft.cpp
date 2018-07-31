
#include <time.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <random>
#include <stdio.h>

#include <fftw3.h>

#include "../src/e3e_detection.h"
#include "../src/stft.h"


// Initialize RNG with random seed
unsigned int time_ui = static_cast<unsigned int>( time(NULL) );
std::default_random_engine generator(time_ui);
std::uniform_real_distribution<float> dist(0,5.);

// A wrapper to fill the arrays
float rand_val()
{
  return dist(generator);
}

void test_stft(int NFRAMES, int SHIFT, int FFT_SIZE, int CHANNELS, int ZB, int ZF, int WFLAG)
{

  float buf_in[NFRAMES * SHIFT * CHANNELS];
  float buf_out[NFRAMES * SHIFT * CHANNELS];

  STFT engine(SHIFT, FFT_SIZE, ZB, ZF, CHANNELS, WFLAG);

  double total_error = 0.;

  float *buf_in_ptr = buf_in;
  float *buf_out_ptr = buf_out;

  for (int frame = 0 ; frame < NFRAMES ; frame++)
  {
    // generate new values
    for (int sample = 0 ; sample < SHIFT ; sample++)
      for (int ch = 0 ; ch < CHANNELS ; ch++)
        buf_in_ptr[sample * CHANNELS + ch] = rand_val();

    // process forward and backward
    engine.analysis(buf_in_ptr);
    engine.synthesis(buf_out_ptr);

    buf_in_ptr += SHIFT * CHANNELS;
    buf_out_ptr += SHIFT * CHANNELS;
  }

  int offset = (FFT_SIZE - SHIFT) * CHANNELS;
  for (int i = 0 ; i < NFRAMES * SHIFT * CHANNELS - offset ; i++)
  {
    double e = (buf_in[i] - buf_out[i+offset]);

    /*
    std::cout << "frame " << i << " error " << e << " " << buf_in[i] << " " << buf_out[i+offset] << "\n";
    std::cout.flush();
    */

    total_error = e * e;
  }

  printf("fft_size=%d shift=%d channels=%d zb=%d zf=%d ", FFT_SIZE, SHIFT, CHANNELS, ZB, ZF);
  if (WFLAG == STFT_NO_WINDOW)
    printf("no window");
  else if (WFLAG == STFT_WINDOW_ANALYSIS)
    printf("analysis window");
  else if (WFLAG == STFT_WINDOW_BOTH)
    printf("analysis and synthesis windows");
  printf("\n");

  printf("  Average error: %f\n", total_error / (SHIFT * CHANNELS * NFRAMES));
}

int main(int argc, char **argv)
{
  test_stft(100, 128, 128, 1, 0, 0, STFT_NO_WINDOW);
  test_stft(100, 128, 128, 4, 0, 0, STFT_NO_WINDOW);
  test_stft(100, 128, 256, 1, 0, 0, STFT_WINDOW_ANALYSIS);
  test_stft(100, 128, 256, 4, 0, 0, STFT_WINDOW_ANALYSIS);
  test_stft(100, 128, 256, 1, 0, 0, STFT_WINDOW_BOTH);
  test_stft(100, 128, 256, 4, 0, 0, STFT_WINDOW_BOTH);
  test_stft(100, 68, 256, 1, 0, 0, STFT_WINDOW_BOTH);
  test_stft(100, 68, 256, 4, 0, 0, STFT_WINDOW_BOTH);
  test_stft(100, 150, 256, 1, 0, 0, STFT_WINDOW_BOTH);
  test_stft(100, 150, 256, 4, 0, 0, STFT_WINDOW_BOTH);

  return 0;
}
