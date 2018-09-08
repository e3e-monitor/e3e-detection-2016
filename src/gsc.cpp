#include <fstream>
#include <cmath>

#include "json.hpp"
#include "gsc.h"

GSC::GSC(
    std::string config_file,
    float _f_max,   // maximum frequency to process, the rest is set to zero
    float _fs       // the sampling frequency
    )
    : f_max(_f_max), fs(_fs)
{
  // read in the JSON file containing all the parameters
  std::ifstream i(config_file, std::ifstream::in);
  json config;
  i >> config;
  i.close();

  // assign parameters to object attributes
  this->nchannel = config['nchannel'];
  this->nchannel_ds = config['nchannel_ds'];
  this->nfreq = config['nfft'] / 2 + 1;
  this->nfft = config['nfft'];

  // algorithms parameters
  this->rls_ff = config['rls_ff'];    // forgetting factor for RLS
  this->rls_reg = config['rls_reg'];  // regularization factor for RLS
  this->pb_ff = config['pb_ff'];      // forgetting factor for projection back
  this->pb_ref_channel = config['pb_ref_channel'];  // The reference channel for projection back

  // Limit frequencies
  this->f_max = config['f_max'];
  this->fs = config['fs'];
  this->f_min_index = 1;  // we skip the DC component in the processing
  this->f_max_index = int(fceilf(f_max / fs + 0.5)); // round to closest bin

  // Get the fixed weights from config file
  std::vector<double> &real = config['fixed_weights_real']
  std::vector<double> &imag = config['fixed_weights_imag']
  this->fixed_weights.resize(this->nfreq * this->nchannel);
  std::transform(
      real.begin(), real.end(),
      imag.begin(),
      this->fixed_weights.begin(),
      [](double da, double db) { return e3e_complex( (float)da, (float)db ); }
      );
  
  // Size the other buffers as needed
  this->adaptive_weights.resize(this->nfreq * this->nchannel, e3e_complex(0., 0.));

  // Intermediate buffers
  this->output_fixed.resize(this->nfreq, e3e_complex(0., 0.));
  this->output_null.resize(this->nfreq * this->nchannel, e3e_complex(0., 0.));
  this->output_null_downsampled.resize(this->nfreq * this->nchannel_ds, e3e_complex(0., 0.));

  // Projection back buffers
  this->projback_num.resize(this->nfreq, e3e_complex(1., 0.));
  this->projback_den.resize(this->nfreq, e3e_complex(1., 0.));

  // RLS variables
  this->covmat_inv.resize(this->nfreq * this->nchannel_ds * this->nchannel_ds, e3e_complex(0., 0.));
  this->xcov.resize(this->nfreq * this->nchannel_ds, e3e_complex(0., 0.));

  this->rls_init();
  
}

GSC::~GSC()
{

}

void GSC::process(e3e_complex_vector &input, e3e_complex_vector &output)
{
  // Compute the fixed beamformer output

  // Apply the blocking matrix

  // Downsample the channels to a reasonnable number

  // Update the adaptive weights
  this->rls_update( ... )

  // Do the null branch beamforming

  // Compute the output signal
  for (int f = this->f_min_index ; f <= this->f_max_index ; f++)
    output[f] = this->output_fixed[f] - this->output_adaptive[f];

  // Make sure that what we don't need in the output is zero
  for (int f = 0 ; f < this->f_min_index ; f++)
    output[f] = e3e_complex(0., 0.);
  for (int f = this->f_max_index + 1 ; f < this->nfreq ; f++)
    output[f] = e3e_complex(0., 0.);

  // projection back: apply scale to match the output to channel 1
  this->projback(input, output, this->pb_ref_channel);
}

void GSC::rls_init()
{
  /*
   * Initializes the inverse matrix in RLS
   */
  int stride = this->nchannel_ds * this_nchannel_ds;
  for (int f = 0, offset1 = 0 ; f < this->nfreq ; f++, offset1 += stride)
    for (int ch = 0, offset2 = offset1 ; ch < this->nchannel_ds ; ch++, offset2 += this->nchannel_ds)
      this->covmat_inv[offset2 + ch] = this->rls_reg;
}

void GSC::rls_update(e3e_complex_vector &input, e3e_complex_vector &error)
{
  /*
   * Updates the inverse covariance matrix and cross-covariance vector.
   * Then, solves for the new adaptive weights
   *
   * @param input The input reference signal vector
   * @param error The error signal
   */


  int covmat_stride = this->nchannel_ds * this->nchannel_ds;
  int xcov_stride = this->nchannel_ds;
  for (int f = this->f_min_index ; f <= this->f_max_index ; f++)
  {
    // Update covariance matrix using Sherman-Morrison Identity
    
    // Update cross-covariance vector
    
    // Multiply the two to obtain the new adaptive weight vector

  }
}

void GSC::projback(e3e_complex &input, e3e_complex &output, int input_ref_channel)
{
  int input_stride = this->nfreq * this->nchannel;
  for (int f = this->f_min_index, i_offset = 0 ; f <= this->f_max_index ; f++, i_offset += input_stride)
  {
    // Update the projback factor
    this->projback_num[f] = ( this->pb_ff * this->projback_num
        + (1. - this->pb_ff) * std::conj(output[f]) * input[i_offset + input_ref_channel] );
    this->projback_den[f] = ( this->pb_ff * this->projback_den
        + (1. - this->pb_ff) * std::conj(output[f]) * output[f] );

    // Apply the projback gain
    output[f] *= (this->projback_num[f] / this->projback_den[f]);
  }
}

