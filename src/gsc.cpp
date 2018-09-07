#include <fstream>
#include <cmath>

#include "json.hpp"
#include "gsc.h"

GSC::GSC(
    std::string config_file,
    float _f_max,   // maximum frequency to process, the rest is set to zero
    float _fs       // the sampling frequency
    )
    : rls_ff(_rls_ff), pb_ff(_pb_ff), f_max(_f_max), fs(_fs)
{
  // read in the JSON file containing all the parameters
  std::ifstream i(config_file, std::ifstream::in);
  json config;
  i >> config;
  i.close();

  // assign parameters to object attributes
  this->nchannel = config['nchannel'];
  this->nchannel_adapt = config['nchannel_adapt'];
  this->nfreq = config['nfft'] / 2 + 1;
  this->nfft = config['nfft'];

  // algorithms parameters
  this->rls_ff;  // forgetting factor for RLS
  this->pb_ff;   // forgetting factor for projection back

  // Limit frequencies
  this->f_max = config['f_max'];
  this->fs = config['fs'];
  this->f_min_index = 1;  // we skip the DC component in the processing
  this->f_max_index = int(fceilf(f_max / fs + 0.5)); // round to closest bin

  // Get the fixed weights from config file
  const std::complex<double> j(0, 1);  // imaginary number
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
  this->adaptive_weights.resize(this->nfreq * this->nchannel, std::complex(0., 0.));

  this->output_fixed.resize(this->nfreq, std::complex(0., 0.));
  this->output_null.resize(this->nfreq * this->nchannel, std::complex(0., 0.));
  this->output_null_downsampled.resize(this->nfreq * this->nchannel_adapt, std::complex(0., 0.));
  
}

GSC~GSC()
{

}

void GSC::process(e3e_complex_vector &input, e3e_complex_vector &output)
{

}
