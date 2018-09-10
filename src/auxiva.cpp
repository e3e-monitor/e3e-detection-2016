#include <fstream>
#include <cmath>
#include <fstream>

#include "json.hpp"
#include "auxiva.h"

using nlohmann::json as json;

AuxIVA::AuxIVA(
    std::string config_file,
    float _fs,      // the sampling frequency
    int _nchannel   // the number of input channels
    )
    : fs(_fs), nchannel(_nchannel)
{
  // read in the JSON file containing all the parameters
  std::ifstream i(config_file, std::ifstream::in);
  json config;
  i >> config;
  i.close();

  this->nsources = nchannel; // For now this AuxIVA implementation only supports the determined case 
  // assign parameters to object attributes
  this->pb_ff = config.at("pb_ff").get<float>();  // forgetting factor for projection back
  this->pb_ref_channel = config.at("pb_ref_channel").get<int>();  // The reference channel for projection back
  this->nfft = config.at("nfft").get<int>();

  // Limit frequencies
  this->f_max = config.at("f_max").get<float>();
  this->f_min_index = 1;  // we skip the DC component in the processing
  this->f_max_index = int(ceilf(this->f_max / this->fs + 0.5)); // round to closest bin
  this->nfreq = this->f_max_index - this->f_min_index;  // only consider the number of bands processed

  // Projection back buffers
  this->projback_num = Eigen::ArrayXcf::Zero(this->nfreq);
  this->projback_num = 1.f;
  this->projback_den = Eigen::ArrayXf::Zero(this->nfreq);
  this->projback_den = 1.f;

  // AuxIVA variables
  this->W.resize(this->nfreq);
  for (auto it = this->W.begin() ; it != this->W.end() ; ++it)
    *it = Eigen::MatrixXcf::Identity(this->nchannel, this->nchannel);

  this->V.resize(this->nsources);
  for (auto it_nsources = this->V.begin() ; it_nsources != this-> V.end(); it_nsources++)
  {
      *it_nsources.resize(this->nfreq);
      for (auto it_nfreq = it_nsources.begin(); it_nfreq != it_nsources.end(); it_nfreq++)
      {
          *it_nfreq = Eigen::MatrixXcf::Zero(this->nchannel, this->nchannel);
      }
  }
}

void AuxIVA::process(e3e_complex_vector *input, e3e_complex_vector *output)
{    
    Eigen::ArrayXf r;  // size (nsources)
    Eigen::ArrayXf G_r;  // size (nsources)
    float regularization = 1e6;

    // Wrap input/output in Eigen::Array
    int input_offset = this->f_min_index * this->nchannel;
    Eigen::Map<Eigen::ArrayXXcf> X(&input[input_offset], this->nfreq, this->nchannel);
    Eigen::Map<Eigen::ArrayXcf> Y(&output[this->f_min_index], this->nfreq, this->nsources);

    // Pre-emptivaly zero-out the content of output buffer
    for (int f = 0 ; f < this->nchannel * (this->nfreq) ; f++)
        output[f,:] = pow(abs(X[f,:].dot(W.at(f))), 2.);

    for (int f=0;f<this->nfreq;f++)
    {
        // Calculate r for current frame
        // Calculalate G_r for current frame
        if (this->first_block)
        {
            // Update V with regularization factor
            this->first_block = false;
        }

        else
        {
            // Update V with forgetting factor
        }

        // Update W
        // Use WoodBury

        // Update Y
    }

}

void AuxIVA::demix(e3e_complex_vector &input, e3e_complex_vector &output, std::vector<Eigen::MatrixXcf> W)
{
    // Wrap input/output in Eigen::Array
    int input_offset = this->f_min_index * this->nchannel;
    Eigen::Map<Eigen::ArrayXXcf> X(&input[input_offset], this->nfreq, this->nchannel);
    Eigen::Map<Eigen::ArrayXcf> Y(&output[this->f_min_index], this->nfreq);

    for (int f = this->f_min_index;i<this->f_max_index;f++)
    {
        output[f]
    }
}

void AuxIVA::projback(Eigen::Map<Eigen::ArrayXXcf> &input, Eigen::Map<Eigen::ArrayXcf> &output, int input_ref_channel)
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


