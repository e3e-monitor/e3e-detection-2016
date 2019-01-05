#ifndef __AuxIVA_H__
#define __AuxIVA_H__

/*
 * Defines class for online implementation of AuxIVA for Blind Source Separation
 * 
 */
#include <string>

#include <e3e_detection.h>
#include <Eigen/Dense>

class AuxIVA
{
  public:
    // class attributes
    int nfft;
    float fs;
    int nchannel, nsources;
    int nfreq;
    bool first_block;

    // algorithm parameters
    std::vector<Eigen::MatrixXcf> W;            // size(nsources, nchannels, nfreq)
    std::vector<std::vector<Eigen::MatrixXcf>> V;   // size(nsources, nfreq, nchannels, nchannels)

    // The limit of the processing band in frequency
    float f_max;
    int f_min_index, f_max_index;

    // Projection back variables
    Eigen::ArrayXcf projback_num;  // numerator, size: (nfreq), complex
    Eigen::ArrayXf projback_den;  // denominator, size: (nfreq), real-valued
    float pb_ff;        // projection back forgetting factor
    int pb_ref_channel;     // projection back reference channel

    // class methods
    AuxIVA(
        std::string auxiva_file,
        float fs,       // the sampling frequency
        int nchannel    // the number of input channels
    );
    ~AuxIVA();

    void process(e3e_complex_vector &input, e3e_complex_vector &output);
    void projback(Eigen::Map<Eigen::ArrayXXcf> &input, Eigen::Map<Eigen::ArrayXcf> &output, int input_ref_channel);
};

#endif // __AUXIVA_H__
