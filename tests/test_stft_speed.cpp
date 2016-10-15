
#include <time.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <vector>
#include <random>

#include <fftw3.h>

#include "../src/stft.h"

#define FFT_SIZE 512
#define FRAME_SIZE 512
#define NFRAMES 1000
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
  std::vector<int> fft_size = { 64, 128, 256, 512, 1024, 2048, 4096 };
  time_t now, ellapsed;
  STFT *engine;

  for (int i = 0 ; i < fft_size.size() ; i++)
  {

    engine = new STFT(fft_size[i], NFRAMES, CHANNELS);


    for (int frame = 0 ; frame < NFRAMES ; frame++)
    {
      float *buf_ptr = engine->get_in_buffer();

      // fill buffer with random data
      for (int i = 0 ; i < FRAME_SIZE ; i++)
        for (int ch = 0 ; ch < CHANNELS ; ch++)
          buf_ptr[i*CHANNELS + ch] = rand_val();
    }

    now = clock();
    for (int frame = 0 ; frame < NFRAMES ; frame++)
      engine->transform();
    ellapsed = clock() - now;

    delete engine;

    std::cout << fft_size[i] << ": ";
    std::cout << float(ellapsed) / NFRAMES << " us, ";
    std::cout << " n samples @ 16kHz: " << 1000 * 1000 * float(fft_size[i]) / 16000 << " us " << std::endl;

  }

}
