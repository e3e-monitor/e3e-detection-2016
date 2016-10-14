
#include "stft.h"

/* Allocate all the buffers and create the FFTW plans */
STFT::STFT(int _fft_size, int _n_frames, int _channels)
: n_frames(_n_frames), channels(_channels), fft_size(_fft_size)
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
  this->circ_out_buffer = new std::complex<float>[this->circ_out_buffer_size];

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
std::complex<float> *STFT::get_out_buffer()
{
  int buf_index = (this->current_frame * this->n_samples_per_out_frame) % this->circ_out_buffer_size;
  return this->circ_out_buffer + buf_index;
}


/* transform the current frame and returns a pointer to it, then move to the next frame */
void STFT::transform(void)
{
  
  fftwf_execute(this->plans[this->current_frame]);
  this->current_frame += 1;
}

/* return a specific sample from the output buffer */
std::complex<float> STFT::get_sample(int frame, int frequency, int channel)
{
  int i = frame * this->n_samples_per_out_frame + frequency * this->channels + channel;
  return this->circ_out_buffer[i];
}

