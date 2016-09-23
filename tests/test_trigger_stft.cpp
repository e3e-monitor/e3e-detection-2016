
#include <time.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <random>

#include <assert.h>
#include <fftw3.h>

#include "../src/e3e_detection.h"
#include "../src/stft.h"
#include "../src/srpphat.h"

#include <wiringPi.h>

#include <string>
#include <iostream>
#include <valarray>
#include <unistd.h>

#include "matrix_hal/everloop_image.h"
#include "matrix_hal/everloop.h"
#include "matrix_hal/microphone_array.h"
#include "matrix_hal/wishbone_bus.h"

#define FFT_SIZE 128
#define FRAME_SIZE 128
#define NFRAMES 4
#define CHANNELS 8

#define FS 16000
#define C 343

#define SRP_N_GRID 35
#define SRP_NFRAMES 1
#define SRP_K_MIN 10
#define SRP_K_LEN 40
#define SRP_DIM 2
#define CONFIG_FILE "./CONFIG"

namespace hal = matrix_hal;


int main(int argc,char** argv) {
  hal::WishboneBus bus;
  bus.SpiInit();

  hal::MicrophoneArray mics;
  mics.Setup(&bus);

  hal::Everloop everloop;
  everloop.Setup(&bus);

  hal::EverloopImage image1d;

  std::valarray<int> lookup = {23, 27, 32, 1, 6, 10, 14, 19};

  std::valarray<float> magnitude(mics.Channels());

  STFT *engine = new STFT(FFT_SIZE, NFRAMES, CHANNELS);
  SRPPHAT *srpphat = new SRPPHAT(engine, CONFIG_FILE, SRP_K_MIN, SRP_K_LEN, SRP_N_GRID, SRP_NFRAMES, float(FS), C, SRP_DIM);

  float *buf_ptr;
  e3e_complex *fd_ptr;
  int argmax = 0;

  int16_t trgthr=atoi(argv[1]);

  assert(mics.NumberOfSamples() == FFT_SIZE);

  while (true) 
  {

    mics.Read();
    magnitude = 0.0;
    bool trgDetected=false;

    buf_ptr = engine->get_in_buffer();

    for (unsigned int s = 0; s < mics.NumberOfSamples(); s++) 
      for (int ch = 0 ; ch < CHANNELS ; ch++)
        buf_ptr[s*CHANNELS + ch] = mics.At(s, ch);

    fd_ptr = engine->transform();

    argmax = srpphat->process();

    std::cout << srpphat->grid[argmax][0] / M_PI * 180. << std::endl;
    
  }

  delete engine;

  return 0;
}
