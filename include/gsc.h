#ifndef __GSC_H__
#define __GSC_H__
/*
 * This is the class definition for the Generalized Sidelobe Canceller
 * 
 * 2018 (c) Robin Scheibler
 */

#include <string>

#include <Eigen/Dense>
#include <e3e_detection.h>

class GSC
{
  public:



    // parameter attributes
    float fs;
    int nchannel;

    // parameters coming from config file
    int nchannel_ds;
    int nfft;
    int nfreq;
    
    // algorithm parameters
    float rls_ff;
    float rls_reg;
    float pb_ff;
    int pb_ref_channel;

    // The limit of the processing band in frequency
    float f_max;
    int f_min_index, f_max_index;

    // The beamforming weights
    Eigen::ArrayXXcf fixed_weights;    // size: (nfreq, nchannel)
    Eigen::ArrayXXcf adaptive_weights; // size: (nfreq, nchannel_ds)

    // The intermediate buffers
    Eigen::ArrayXcf output_fixed;  // size: (nfreq)
    Eigen::ArrayXXcf output_null;   // size: (nfreq, nchannels_ds)
    Eigen::ArrayXcf output_adaptive;  // size: (nfreq)

    // Projection back variables
    Eigen::ArrayXcf projback_num;  // numerator, size: (nfreq), complex
    Eigen::ArrayXf projback_den;  // denominator, size: (nfreq), real-valued

    // RLS variables
    std::vector<Eigen::MatrixXcf> covmat_inv;  // inverse covariance matrices, size: (nfreq, nchannel_ds, nchannel_ds)
    Eigen::MatrixXcf xcov;                     // cross covariance vectors, size: (nfreq, nchannel_ds)

    GSC(
        std::string fixed_beamformer_file,  // name of file storing the fixed beamforming weights
        float fs,      // the sampling frequency
        int nchannel   // the number of input channels
       );
    ~GSC();

    void process(e3e_complex_vector &input, e3e_complex_vector &output);    
    void rls_update(e3e_complex_vector &input, e3e_complex_vector &error);
    void projback(Eigen::Map<Eigen::ArrayXXcf> &input, Eigen::Map<Eigen::ArrayXcf> &output, int input_ref_channel);
};

#endif // __GSC_H__
