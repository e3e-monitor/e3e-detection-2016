#ifndef __MFCC_H__
#define __MFCC_H__

/*
 * Computes the Mel-Frequency Cepstrum Coefficients (MFCC) according
 * to the description by Huang-Acera-Hon 6.5.2 (2001)
 * The MFCC are features mimicing the human perception usually
 * used for some learning task.
 */

#include <iostream>
#include <complex>
#include <fftw3.h>

#include "../src/e3e_detection.h"

class MFCC
{
  public:

    // array of filter center frequencies
    float *center_freq;

    // we need a dct plan
    fftwf_plan dct_plan;
    float *dct_buf_in;
    float *dct_buf_out;

    int mfcc_size;
    int fft_size;
    int fs;
    float fl;
    float fh;

    MFCC(int mfcc_size, int fft_size, int fs, float fl, float fh);
    ~MFCC();

    // This function gets an array of several FFT and computes the MFCC
    void transform(e3e_complex *arr_fft, float *arr_mfcc, int howmany);
    void transform_many(e3e_complex *arr_fft, int istride, int idist, float *arr_mfcc, int ostride, int odist, int howmany);

};

#endif // __MFCC_H__
