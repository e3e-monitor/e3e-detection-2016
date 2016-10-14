#ifndef __STFT_H__
#define __STFT_H__

#include <complex>
#include <cmath>
#include <fftw3.h>

#define FRAME_SIZE 512
#define FFT_SIZE 512
#define N_FRAMES 10
#define CHANNELS 8

class STFT
{
  public:
    int n_frames;
    int frame_size;
    int fft_size;
    int channels;

    fftwf_plan *plans;

    int n_samples_per_in_frame;
    int n_samples_per_out_frame;
    int circ_in_buffer_size;
    int circ_out_buffer_size;

    float *circ_in_buffer;
    std::complex<float> *circ_out_buffer;  // circular buffer pointer
    int current_frame = 0;

    STFT(int _fft_size, int _n_frames, int _channels);
    ~STFT();

    float *get_in_buffer();
    std::complex<float> *get_out_buffer();
    void transform();

    std::complex<float> get_sample(int frame, int frequency, int channel);

};


#endif // __STFT_H__
