
#include <time.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <random>

#include <fftw3.h>

#include "../src/e3e_detection.h"
#include "../src/stft.h"

#define FFT_SIZE 512
#define SHIFT (256 + 100)
#define NFRAMES 100
#define ZB 0
#define ZF 0
#define CHANNELS 8
#define WFLAG STFT_WINDOW_BOTH

// Initialize RNG with random seed
unsigned int time_ui = static_cast<unsigned int>( time(NULL) );
std::default_random_engine generator(time_ui);
std::uniform_real_distribution<float> dist(0,1.);

// A wrapper to fill the arrays
float rand_val()
{
  return dist(generator);
}

int main(int argc, char **argv)
{
  float buf_in[NFRAMES * SHIFT * CHANNELS];
  float buf_out[NFRAMES * SHIFT * CHANNELS];

  STFT engine(SHIFT, FFT_SIZE, ZB, ZF, CHANNELS, WFLAG);

  double total_error = 0.;

  float *buf_in_ptr = buf_in;
  float *buf_out_ptr = buf_out;

  for (int frame = 0 ; frame < NFRAMES ; frame++)
  {
    std::cout << "Frame: " << frame << "\n";
    std::cout.flush();

    // generate new values
    for (int sample = 0 ; sample < SHIFT ; sample++)
      for (int ch = 0 ; ch < CHANNELS ; ch++)
        buf_in_ptr[sample * CHANNELS + ch] = rand_val();

    // process forward and backward
    engine.analysis(buf_in_ptr);
    std::cout << "Checkpoint 1\n";
    std::cout.flush();
    engine.synthesis(buf_out_ptr);
    std::cout << "Checkpoint 2\n";
    std::cout.flush();

    buf_in_ptr += SHIFT * CHANNELS;
    buf_out_ptr += SHIFT * CHANNELS;
  }

  std::cout << "Processing done\n";
  std::cout.flush();

  int offset = FFT_SIZE - SHIFT;
  for (int i = 0 ; i < NFRAMES * SHIFT * CHANNELS - offset ; i++)
  {
    double e = (buf_in[i] - buf_out[i+offset]);

    std::cout << "frame " << i << " error " << e << " " << buf_in[i] << " " << buf_out[i+offset] << "\n";
    std::cout.flush();

    total_error = e * e;
  }

    
  std::cout << "Average error: " << total_error / (SHIFT * CHANNELS * NFRAMES) << "\n";

  /*
  for (int frame = 0 ; frame < NFRAMES ; frame++)
  {
    for (int ch = 0 ; ch < CHANNELS ; ch++)
    {
      if (errors[frame * CHANNELS + ch] > 1e-5)
      {
        std::cout << "** Ouch this is too large!! **" << std::endl;
        std::cout << "Error: " << errors[frame * CHANNELS + ch] << std::endl;
        std::cout << "Input:" << std::endl;
        for (int i = 0 ; i < FFT_SIZE ; i++)
          std::cout << engine.get_td_sample(frame, i, ch) << " ";
        std::cout << std::endl;

        std::cout << "Output:" << std::endl;
        for (int i = 0 ; i < FFT_SIZE / 2 + 1 ; i++)
          std::cout << engine.get_fd_sample(frame, i, ch) << " ";
        std::cout << std::endl;
      }
    }
  }
  */
  return 0;

}
