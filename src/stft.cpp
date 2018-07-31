
#include <iostream>
#include <string.h>
#include "stft.h"

/* Allocate all the buffers and create the FFTW plans */
STFT::STFT(int _shift, int _fft_size, int _zpb, int _zpf, int _channels, int _flags)
: shift(_shift), fft_size(_fft_size), zeropad_b(_zpb), zeropad_f(_zpf),
  channels(_channels), flags(_flags)
{
  int dims[] = { this->fft_size };

  // the size of the input buffer
  this->num_samples = this->fft_size - this->zeropad_b - this->zeropad_f;
  this->state_in_size = this->num_samples - this->shift;
  this->state_out_size = this->fft_size - this->shift;

  // allocate the buffers
  this->state_in_buffer = new float[this->channels * this->state_in_size];
  this->state_out_buffer = new float[this->channels * this->state_out_size];
  this->time_buffer = new float[this->channels * this->fft_size];
  this->freq_buffer = new e3e_complex[this->channels * (this->fft_size / 2 + 1)];

  // A cast is necessary to interface with FFTW
  fftwf_complex *fbuf = 
    reinterpret_cast<fftwf_complex *>(this->freq_buffer);

  this->plan_analysis = fftwf_plan_many_dft_r2c(
      1, dims, this->channels,
      this->time_buffer, NULL,
      this->channels, 1,
      fbuf, NULL,
      this->channels, 1,
      FFTW_ESTIMATE);

  this->plan_synthesis = fftwf_plan_many_dft_c2r(
      1, dims, this->channels,
      fbuf, NULL,
      this->channels, 1,
      this->time_buffer, NULL,
      this->channels, 1,
      FFTW_ESTIMATE);

  this->fftw_scale = 1. / this->fft_size;

  // zero all the buffers
  memset(this->state_in_buffer, 0, (this->channels * this->state_in_size) * sizeof(this->state_in_buffer[0]));
  memset(this->state_out_buffer, 0, (this->channels * this->state_out_size) * sizeof(this->state_out_buffer[0]));
  memset(this->time_buffer, 0, (this->channels * this->fft_size) * sizeof(this->time_buffer[0]));
  memset(this->freq_buffer, 0, (this->channels * (this->fft_size / 2 + 1)) * sizeof(this->freq_buffer[0]));

  // prepare the required windows
  if (flags & STFT_WINDOW_ANALYSIS)
  {
    this->win_a = new HannWindow(this->num_samples);

    if (flags & STFT_WINDOW_SYNTHESIS)
      this->win_s = new GrifLimWindow(*this->win_a, this->shift);
  }
  else if (flags & STFT_WINDOW_SYNTHESIS)
  {
    std::cout << "Warning: use of synthesis window without an analysis one is ignored.\n";
  }
}

/* Here we just delete the dynamically allocated arrays */
STFT::~STFT()
{
  delete this->state_in_buffer;
  delete this->state_out_buffer;
  delete this->time_buffer;
  delete this->freq_buffer;
  if (this->win_a != NULL) delete this->win_a;
  if (this->win_s != NULL) delete this->win_s;
}

e3e_complex *STFT::analysis(float *buffer)
{
  /**
    Performs analysis of new input data.

    @param buffer A buffer that contains the new data. The size is (channels * shift).
    @return A pointer to the frequency domain buffer.

    */
  int offset = 0;

  /* move state data into transform input buffer */
  offset += this->zeropad_f * this->channels;
  memcpy(this->time_buffer + offset, this->state_in_buffer, (this->state_in_size * this->channels) * sizeof(this->time_buffer[0]));

  /* move new frame into transform input buffer */
  offset += this->state_in_size * this->channels;
  memcpy(this->time_buffer + offset, buffer, (this->shift * this->channels) * sizeof(this->time_buffer[0]));

  /* save the new state for next frame */
  offset = (this->zeropad_f + this->shift) * this->channels;
  memcpy(this->state_in_buffer, this->time_buffer + offset, (this->state_in_size * this->channels) * sizeof(this->time_buffer[0]));

  /* apply analysis window if required */
  if (this->win_a != NULL)
  {
    offset = this->zeropad_f * this->channels;
    for (int m = 0 ; m < this->num_samples ; m++)
    {
      offset += this->channels;
      for (int ch = 0 ; ch < this->channels ; ch++)
        this->time_buffer[offset + ch] *= (*this->win_a)[m];
    }
  }

  /* now transform to frequency! */
  fftwf_execute(this->plan_analysis);

  return this->freq_buffer;
}

void STFT::synthesis(float *buffer)
{
  /**
    Performs sythesis of newest complete frame

    @param buffer A buffer to store the freshly produced data. The size is (channels * shift).
    */
  int offset = 0;

  /* now invert the transform! */
  fftwf_execute(this->plan_synthesis);

  /* apply the synthesis window, if required */
  if (this->win_s != NULL)
  {
    offset = this->zeropad_f * this->channels;
    for (int m = 0 ; m < this->num_samples ; m++)
    {
      offset += this->channels;
      for (int ch = 0 ; ch < this->channels ; ch++)
        this->time_buffer[offset + ch] *= (*this->win_s)[m];
    }
  }

  if (this->shift <= this->state_out_size)
  {
    // Case for overlap more than half the frame

    /* prepare the data to send out */
    for (int i = 0 ; i < this->channels * this->shift ; i++)
      buffer[i] = this->time_buffer[i] + this->state_out_buffer[i];

    /* sum up reminder of state buffer with overlapping fft buffer */
    offset = this->channels * this->shift;
    for (int i = offset, m = 0 ; m < this->channels * (this->state_out_size - this->shift) ; i++, m++)
      this->state_out_buffer[m] = this->state_out_buffer[i] + this->time_buffer[i];

    /* simply copy the non-overlapping final part of the fft buffer */
    memcpy(
        this->state_out_buffer + this->channels * (this->state_out_size - this->shift),
        this->time_buffer + this->channels * this->state_out_size,
        this->channels * this->shift * sizeof(this->state_in_buffer[0])
        );

  }
  else
  {
    // Case for overlap less than half the frame

    /* prepare the data to send out */
    for (int i = 0 ; i < this->channels * this->state_out_size ; i++)
      buffer[i] = this->time_buffer[i] + this->state_out_buffer[i];

    for (int i = this->channels * this->state_out_size ; i < this->channels * this->shift ; i++)
      buffer[i] = this->time_buffer[i];

    memcpy(
        this->state_out_buffer,
        this->time_buffer + this->channels * this->shift,
        this->channels * this->state_out_size * sizeof(this->state_out_buffer[0])
        );
  }

  /* apply inverse FFTW scaling */
  for (int i = 0 ; i < this->channels * this->shift ; i++)
    buffer[i] *= this->fftw_scale;
}
