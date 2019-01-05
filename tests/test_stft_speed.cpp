
#include <time.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <vector>
#include <random>

#include <fftw3.h>

#include "stft.h"

#define NFRAMES 1000
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
  std::vector<int> frame_size = { 64, 128, 256, 512, 1024, 2048, 4096 };
  time_t now, ellapsed;
  STFT *engine;

  for (int i = 0 ; i < frame_size.size() ; i++)
  {

    engine = new STFT(frame_size[i], frame_size[i] * 2, 0, 0, CHANNELS, STFT_WINDOW_BOTH);

    float buf_in[frame_size[i] * CHANNELS];
    float buf_out[frame_size[i] * CHANNELS];

    for (int frame = 0 ; frame < frame_size[i] ; frame++)
      for (int ch = 0 ; ch < CHANNELS ; ch++)
        buf_in[frame*CHANNELS + ch] = rand_val();

    now = clock();
    for (int frame = 0 ; frame < NFRAMES ; frame++)
    {
      engine->analysis(buf_in);
      engine->synthesis(buf_out);
    }
    ellapsed = clock() - now;

    delete engine;

    std::cout << frame_size[i] << ": ";
    std::cout << float(ellapsed) / NFRAMES << " us, ";
    std::cout << " n samples @ 48kHz: " << 1000 * 1000 * float(frame_size[i]) / 48000 << " us " << std::endl;

  }

}
