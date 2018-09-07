#ifndef __GSC_H__
#define __GSC_H__
/*
 * This is the class definition for the Generalized Sidelobe Canceller
 * 
 * 2018 (c) Robin Scheibler
 */

#include <string>

#include "json.hpp"
#include "e3e_detection.h"

class GSC
{
  public:

    // parameter attributes
    float rls_ff;
    float pb_ff;
    float f_max;
    int fs;

    // other attributes
    int nfft;

    // The beamforming weights
    e3e_complex_vector fixed_weights;
    e3e_complex_vector adaptive_weights;

    // The intermediate buffers
    e3e_complex_vector output_fixed;
    e3e_complex_vector output_null;
    e3e_complex_vector output_null_downsampled;

    // Projection back variables
    e3e_complex_vector projback_num;  // numerator
    e3e_complex_vector projback_den;  // denominator

    // RLS variables
    e3e_complex_vector covmat_inv;  // inverse covariance matrices
    e3e_complex_vector xcov;        // cross covariance vectors

    GSC(
        std::string fixed_beamformer_file,  // name of file storing the fixed beamforming weights
        float rls_ff,  // forgetting factor for RLS
        float pb_ff,   // forgetting factor for projection back
        float f_max,   // maximum frequency to process, the rest is set to zero
        float fs       // the sampling frequency
       );
    ~GSC();

    void process(e3e_complex_vector &input, e3e_complex_vector &output);    
    void update_rls(e3e_complex_vector &data);
    void estimate_rls();
};

#endif // __GSC_H__
