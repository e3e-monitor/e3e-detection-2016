#include <fstream>
#include <cmath>

#include "json.hpp"
#include "gsc.h"

using nlohmann::json;

GSC::GSC(
    std::string config_file,
    float _fs,     // the sampling frequency
    int _nchannel  // the number of input channels
    )
    : fs(_fs), nchannel(_nchannel)
{
  // read in the JSON file containing all the parameters
  std::ifstream i(config_file, std::ifstream::in);
  json config;
  i >> config;
  i.close();

  // assign parameters to object attributes
  this->nchannel_ds = config.at("nchannel_ds").get<int>();
  this->nfft = config.at("nfft").get<int>();

  // algorithms parameters
  this->rls_ff = config.at("rls_ff").get<float>();    // forgetting factor for RLS
  this->rls_reg = config.at("rls_reg").get<float>();  // regularization factor for RLS
  this->pb_ff = config.at("pb_ff").get<float>();      // forgetting factor for projection back
  this->pb_ref_channel = config.at("pb_ref_channel").get<int>();  // The reference channel for projection back

  // Limit frequencies
  this->f_max = config.at("f_max").get<float>();
  this->f_min_index = 1;  // we skip the DC component in the processing
  this->f_max_index = int(ceilf(this->f_max / this->fs + 0.5)); // round to closest bin
  this->nfreq = this->f_max_index - this->f_min_index;  // only consider the number of bands processed

  // Get the fixed weights from config file, the complex numbers are stored
  // with real/imag parts interleaved i.e. [r0, i0, r1, i1, r2,  ...]
  // in row-major order
  std::vector<float> w = config.at("fixed_weights").get<std::vector<float>>();
  this->fixed_weights = Eigen::ArrayXXcf::Zero(this->nfreq, this->nchannel);
  for (int f = 0, offset = this->f_min_index * this->nchannel ; f < this->nfreq ; f++, offset += this->nchannel)
    for (int ch = 0 ; ch < this->nchannel ; ch++)
      this->fixed_weights(f, ch) = e3e_complex(w[2 * (offset + ch)], w[2 * (offset + ch) + 1]);
  
  // Size the other buffers as needed
  this->adaptive_weights = Eigen::ArrayXXcf::Zero(this->nfreq, this->nchannel);

  // Intermediate buffers
  this->output_fixed = Eigen::ArrayXcf::Zero(this->nfreq);
  this->output_null = Eigen::ArrayXXcf::Zero(this->nfreq, this->nchannel);
  this->output_adaptive = Eigen::ArrayXcf::Zero(this->nfreq);

  // Projection back buffers
  this->projback_num = Eigen::ArrayXcf::Zero(this->nfreq);
  this->projback_num = 1.f;
  this->projback_den = Eigen::ArrayXf::Zero(this->nfreq);
  this->projback_den = 1.f;

  // RLS variables
  this->covmat_inv.resize(this->nfreq);
  for (auto it = this->covmat_inv.begin() ; it != this->covmat_inv.end() ; ++it)
    *it = Eigen::MatrixXcf::Identity(this->nchannel_ds, this->nchannel_ds) * (1.f / this->rls_reg);
  this->xcov = Eigen::MatrixXcf::Zero(this->nfreq, this->nchannel_ds);
}

GSC::~GSC()
{

}

void GSC::process(e3e_complex_vector &input, e3e_complex_vector &output)
{
  // Pre-emptivaly zero-out the content of output buffer
  for (int f = 0 ; f < this->nchannel * (this->nfft / 2 + 1) ; f++)
    output[f] = 0.f;

  // Wrap input/output in Eigen::Array
  int input_offset = this->f_min_index * this->nchannel;
  Eigen::Map<Eigen::ArrayXXcf> X(&input[input_offset], this->nfreq, this->nchannel);
  Eigen::Map<Eigen::ArrayXcf> Y(&output[this->f_min_index], this->nfreq);

  // Compute the fixed beamformer output
  this->output_fixed = (fixed_weights.conjugate() * X).rowwise().sum();

  // Apply the blocking matrix

  // Downsample the channels to a reasonnable number

  // Update the adaptive weights
  //this->rls_update( ... )

  // Do the null branch beamforming

  // Compute the output signal
  Y = this->output_fixed - this->output_adaptive;

  // projection back: apply scale to match the output to channel 1
  this->projback(X, Y, this->pb_ref_channel);
}

void GSC::rls_update(Eigen::Map<Eigen::ArrayXXcf> &input, e3e_complex_vector &error)
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

void GSC::projback(Eigen::Map<Eigen::ArrayXXcf> &input, Eigen::Map<Eigen::ArrayXcf> &output, int input_ref_channel)
{
  /*
   * This function updates the projection back weight and scales
   * the output with the new coefficient
   */

  // slice out the chosen columns of input
  this->projback_num = this->pb_ff * this->projback_num + (1.f - this->pb_ff) * (output.conjugate() * input.col(input_ref_channel));
  this->projback_den = this->pb_ff * this->projback_den + (1.f - this->pb_ff) * output.abs2();

  // weight the output
  output *= (this->projback_num / this->projback_den);
}

