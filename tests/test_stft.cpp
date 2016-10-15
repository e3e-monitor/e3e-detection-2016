
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
#define FRAME_SIZE 512
#define NFRAMES 100
#define CHANNELS 8

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
  float buf_in[FFT_SIZE];
  e3e_complex buf_out[FFT_SIZE/2 + 1];
  fftwf_complex *X = reinterpret_cast<fftwf_complex *>(buf_out);

  STFT engine(FFT_SIZE, NFRAMES, CHANNELS);

  fftwf_plan p_bac = fftwf_plan_dft_c2r_1d(FFT_SIZE, X, buf_in, FFTW_ESTIMATE);

  double total_error = 0.;
  double errors[NFRAMES * CHANNELS] = {0};
  float *buf_ptr;

  for (int frame = 0 ; frame < NFRAMES + 5 ; frame++)
  {
    buf_ptr = engine.get_in_buffer();

    // fill buffer with random data
    for (int i = 0 ; i < FRAME_SIZE ; i++)
      for (int ch = 0 ; ch < CHANNELS ; ch++)
        buf_ptr[i*CHANNELS + ch] = rand_val();

    engine.transform();
  }

  for (int frame = 0 ; frame < NFRAMES ; frame++)
  {
    for (int ch = 0 ; ch < CHANNELS ; ch++)
    {
      // fill in test buffer
      for (int i = 0 ; i < FFT_SIZE / 2 + 1 ; i++)
        buf_out[i] = engine.get_fd_sample(frame, i, ch);

      // now do inverse transform
      fftwf_execute(p_bac);

      for (int i = 0 ; i < FFT_SIZE ; i++)
      {
        double e = buf_in[i] / FFT_SIZE - engine.get_td_sample(frame, i, ch);
        errors[frame * CHANNELS + ch] += e*e;
      }

      total_error += errors[frame * CHANNELS + ch];
    }

  }
    
  std::cout << "Average error: " << total_error / (CHANNELS * NFRAMES) << std::endl;

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

}
