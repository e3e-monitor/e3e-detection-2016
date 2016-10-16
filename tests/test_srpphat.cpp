
#include <iostream>
#include <fstream>
#include <vector>

#include "../src/e3e_detection.h"
#include "../src/srpphat.h"

#define FFT_SIZE 128
#define CHANNELS 8
#define NFRAMES 10

#define FS 16000

#define SRP_N_GRID 18
#define SRP_NFRAMES 8
#define SRP_K_MIN 30
#define SRP_K_LEN 15
#define SRP_DIM 2

#define CONFIG_FILE "./CONFIG"
#define TEST_FILE "../tests/synthetic_data/test_signal.raw"

#define C 343.

int main(int argc, char **argv)
{
  STFT *stft = new STFT(FFT_SIZE, NFRAMES, CHANNELS);
  SRPPHAT *srpphat = new SRPPHAT(stft, CONFIG_FILE, SRP_K_MIN, SRP_K_LEN, SRP_N_GRID, SRP_NFRAMES, float(FS), C, SRP_DIM);

  if (argc != 2)
  {
    std::cerr << "Please provide test signal filename as argument." << std::endl;
    return 1;
  }

  std::cout << "Opening file: " << argv[1] << std::endl;
  std::ifstream fin(argv[1], std::ifstream::in | std::ifstream::binary);

  int count = 0;

  while (!fin.eof())
  {
    float sample;
    int argmax;
    float *ptr = stft->get_in_buffer();

    fin.read((char*)ptr, FFT_SIZE * CHANNELS * sizeof(float));

    if (fin.eof())
      goto end;

    stft->transform();

    argmax = srpphat->process();

    for (int i = 0 ; i < srpphat->n_grid ; i++)
    {
      std::cout << srpphat->spatial_spectrum[i] << " ";
    }
    
    std::cout << std::endl;

    count++;
  }

end:
  std::cout << "******" << std::endl;
  for (int i = 0 ; i < srpphat->n_grid ; i++)
  {
    std::cout << srpphat->grid[i][0] / M_PI * 180. << " " ;
    std::cout << srpphat->grid[i][1] / M_PI * 180. << " " ;
    std::cout << srpphat->spatial_spectrum[i] << std::endl;
  }

  fin.close();
  delete stft;
  delete srpphat;

  return 0;
}
