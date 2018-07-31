#ifndef __STFT_H__
#define __STFT_H__

#include <complex>
#include <cmath>
#include <fftw3.h>

#include "../src/e3e_detection.h"
#include "../src/windows.h"

#define STFT_NO_WINDOW 0
#define STFT_WINDOW_ANALYSIS 1
#define STFT_WINDOW_SYNTHESIS 2
#define STFT_WINDOW_BOTH 3

class STFT
{
  public:

    // The different sizes involved in the STFT
    int shift;
    int fft_size;
    int zeropad_b;
    int zeropad_f;
    int channels;
    int flags;  // only used for the windows now
    int num_samples;

    // The FFTW plans (forward and backward)
    fftwf_plan plan_analysis;
    fftwf_plan plan_synthesis;
    float fftw_scale;

    // The window functions
    Window *win_a=NULL, *win_s=NULL;

    // This buffer keeps the overlapping input data
    float *state_in_buffer;
    size_t state_in_size;

    // This buffer is used for overlap-add at the output
    float *state_out_buffer;
    size_t state_out_size;

    float *time_buffer;       // Buffer for real FFT input data
    e3e_complex *freq_buffer; // Buffer for complex FFT output data

    STFT(int _shift, int _fft_size, int _zpb, int _zpf, int _channels, int _flags);
    ~STFT();

    e3e_complex *analysis(float *new_frame);
    void synthesis(float *buffer);
};


#endif // __STFT_H__
