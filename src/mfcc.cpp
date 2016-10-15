
#include <cmath>

#include "../src/mfcc.h"

float mel_scale(float f)
{
  return 1125.*log(1.+f/700.);
}

float inv_mel_scale(float b)
{
  return 700.*(exp(b/1125.)-1.);
}

MFCC::MFCC(int _mfcc_size, int _fft_size, int _fs, float _fl, float _fh)
  : mfcc_size(_mfcc_size), fft_size(_fft_size), fs(_fs), fl(_fl), fh(_fh)
{
  // allocate array of center frequencies
  this->center_freq = new float[_mfcc_size + 2];

  // fill the center frequency array
  float scale = float(_fft_size) / float(_fs);
  float a = mel_scale(_fl * _fs);
  float b = (mel_scale(_fh * _fs) - a) / float(_mfcc_size + 1);
  for (int m = 0 ; m < _mfcc_size + 2 ; m++)
    this->center_freq[m] = scale * inv_mel_scale( a + m * b );

  // allocate buffers for the DCT
  this->dct_buf_in = new float[_mfcc_size];
  this->dct_buf_out = new float[_mfcc_size];

  // create the DCT-II plan
  this->dct_plan = fftwf_plan_r2r_1d(_mfcc_size, this->dct_buf_in, this->dct_buf_out,
                           FFTW_REDFT10, FFTW_ESTIMATE);

}

MFCC::~MFCC()
{
  delete this->center_freq;
  delete this->dct_buf_in;
  delete this->dct_buf_out;
}

void MFCC::transform(e3e_complex *arr_fft, float *arr_mfcc, int howmany)
{
  int istride = howmany;
  int idist = 1;
  int ostride = howmany;
  int odist = 1;

  this->transform_many(arr_fft, istride, idist, arr_mfcc, ostride, odist, howmany);
}

void MFCC::transform_many(e3e_complex *arr_fft, int istride, int idist, float *arr_mfcc, int ostride, int odist, int howmany)
{
  // initialize mfcc array to zero
  memset(arr_mfcc, howmany * this->mfcc_size, sizeof(float));

  // lighten notation
  float *f = this->center_freq;

  for (int n = 0 ; n < howmany ; n++)
  {
    // the fft array offset
    int i_fft = n * idist;
    int i_mfcc = n * odist;

    // Compute all the output of the critical filters
    for (int m = 1 ; m < this->mfcc_size + 1 ; m++)
    {
      float tmp = 0.;
      for (int k = ceil(f[m-1]) ; k < f[m] ; k++)
      {
        float c = (k - f[m-1]) / ((f[m+1]-f[m-1]) * (f[m]-f[m-1]));
        tmp += 2 * c * norm(arr_fft[i_fft + k * istride]);
      }
      for (int k = ceil(f[m]) ; k < f[m+1] ; k++)
      {
        float c = (f[m+1]-k) / ((f[m+1]-f[m-1]) * (f[m+1]-f[m]));
        tmp += 2 * c * norm(arr_fft[i_fft + k * istride]);
      }

      // fill the DCT buffer
      this->dct_buf_in[m-1] = log(tmp);

    }

    // Now apply the DCT-II
    fftwf_execute(this->dct_plan);

    // Now copy the output from the DCT to the output buffer
    for (int m = 0 ; m < this->mfcc_size ; m++)
    {
      arr_mfcc[i_mfcc + m * ostride] = this->dct_buf_out[m];
    }
  }
}
