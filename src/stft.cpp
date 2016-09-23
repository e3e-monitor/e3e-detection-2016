
#include <iostream>
#include "stft.h"

/* Allocate all the buffers and create the FFTW plans */
STFT::STFT(int _fft_size, int _n_frames, int _channels)
: fft_size(_fft_size), n_frames(_n_frames), channels(_channels)
{
  int dims[] = { _fft_size };

  // the size of the input buffer
  this->n_samples_per_in_frame = _channels * _fft_size;
  this->circ_in_buffer_size = _n_frames * this->n_samples_per_in_frame;

  // the size of the big circular buffer
  this->n_samples_per_out_frame = _channels * (_fft_size / 2 + 1);
  this->circ_out_buffer_size = _n_frames * this->n_samples_per_out_frame;

  // allocate the buffers
  this->circ_in_buffer = new float[this->circ_in_buffer_size];
  this->circ_out_buffer = new e3e_complex[this->circ_out_buffer_size];

  // Allocate space for the plans
  this->plans = new fftwf_plan[_n_frames];

  for (int i = 0 ; i < _n_frames ; i++)
  {
    float *in_buffer = this->circ_in_buffer + i * this->n_samples_per_in_frame;
    fftwf_complex *out_buffer = 
      reinterpret_cast<fftwf_complex *>(this->circ_out_buffer + i * this->n_samples_per_out_frame);

    this->plans[i] = fftwf_plan_many_dft_r2c(
        1, dims, _channels,
        in_buffer, NULL,
        _channels, 1,
        out_buffer, NULL,
        _channels, 1,
        FFTW_ESTIMATE);
  }

  for (int i = 0 ; i < this->circ_in_buffer_size ; i++)
    this->circ_in_buffer[i] = 0;

  for (int i = 0 ; i < this->circ_out_buffer_size ; i++)
    this->circ_out_buffer[i] = 0;
  
}

/* Here we just delete the dynamically allocated arrays */
STFT::~STFT()
{
  delete this->circ_in_buffer; 
  delete this->circ_out_buffer;
  delete this->plans;
}

/* return a pointer to the current input buffer */
float *STFT::get_in_buffer()
{
  int buf_index = (this->current_frame * this->n_samples_per_in_frame) % this->circ_in_buffer_size;
  return this->circ_in_buffer + buf_index;
}

/* return a pointer to the current output buffer */
e3e_complex *STFT::get_out_buffer()
{
  int buf_index = (this->current_frame * this->n_samples_per_out_frame) % this->circ_out_buffer_size;
  return this->circ_out_buffer + buf_index;
}


/* transform the current frame and returns a pointer to it, then move to the next frame */
e3e_complex *STFT::transform(void)
{
  // This is a pointer to the chunk of data we will transform
  e3e_complex *ret_buf = this->circ_out_buffer 
                                + this->current_frame * this->circ_out_buffer_size;
  
  fftwf_execute(this->plans[this->current_frame]);

  // increment frame counter and loop if necessary
  this->current_frame += 1;
  if (this->current_frame == this->n_frames)
    this->current_frame = 0;

  return ret_buf;
}

/* return a specific sample from the output buffer */
e3e_complex STFT::get_fd_sample(int frame, int frequency, int channel)
{
  int circular_index = (this->n_frames + this->current_frame - 1 - frame) % this->n_frames;
  int i = circular_index * this->n_samples_per_out_frame + frequency * this->channels + channel;
  return this->circ_out_buffer[i];
}

float STFT::get_td_sample(int frame, int index, int channel)
{
  int circular_index = (this->n_frames + this->current_frame - 1 - frame) % this->n_frames;
  int i = circular_index * this->n_samples_per_in_frame + index * this->channels + channel;
  return this->circ_in_buffer[i];
}

