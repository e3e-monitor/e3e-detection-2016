
#include <time.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <vector>
#include <random>

#include <fftw3.h>

#include "stft.h"

#define NFRAMES 100
#define CHANNELS 48

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
  using namespace std::complex_literals;

  std::vector<int> frame_size = { 64, 128, 256, 512, 1024, 2048, 4096 };
  time_t now, ellapsed;
  STFT *engine_in, *engine_out;

  for (int i = 0 ; i < frame_size.size() ; i++)
  {

    engine_in = new STFT(frame_size[i], frame_size[i] * 2, 0, 0, CHANNELS, STFT_WINDOW_BOTH);
    engine_out = new STFT(frame_size[i], frame_size[i] * 2, 0, 0, 1, STFT_WINDOW_BOTH);

    float buf_in[frame_size[i] * CHANNELS];
    float buf_out[frame_size[i]];

    // An array of beamforming weights
    e3e_complex weights[(frame_size[i] + 1) * CHANNELS];

    // cheap trick to get the buffers. this should change
    e3e_complex *spectrum_in = engine_in->analysis(buf_in);
    e3e_complex *spectrum_out = engine_out->analysis(buf_out);

    for (int frame = 0 ; frame < frame_size[i] ; frame++)
      for (int ch = 0 ; ch < CHANNELS ; ch++)
      {
        buf_in[frame*CHANNELS + ch] = rand_val();
        weights[frame*CHANNELS + ch] = rand_val() + 1if * rand_val();  // complex
      }

    now = clock();
    for (int frame = 0 ; frame < NFRAMES ; frame++)
    {
      engine_in->analysis(buf_in);

      // apply beamforming weights
      for (int m = 0 ; m < frame_size[i] ; m++)
      {
        spectrum_out[m] = 0;
        for (int ch = 0 ; ch < CHANNELS ; ch++)
          spectrum_out[m] += weights[m*CHANNELS+ch] * spectrum_in[m*CHANNELS+ch];
      }
      engine_out->synthesis(buf_out);
    }
    ellapsed = clock() - now;

    std::cout << frame_size[i] << ": ";
    std::cout << float(ellapsed) / NFRAMES << " us, ";
    std::cout << " n samples @ 48kHz: " << 1000 * 1000 * float(frame_size[i]) / 48000 << " us " << std::endl;

    delete engine_in;
    delete engine_out;

  }

}
