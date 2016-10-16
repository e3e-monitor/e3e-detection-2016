#ifndef __SRPPHAT_H__
#define __SRPPHAT_H__

/*
 * Imagine a good file description..  
 */

#include <cmath>
#include <random>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <complex>
#include <fftw3.h>
#include <string>
#include "e3e_detection.h"
#include "stft.h"

class SRPPHAT
{
  public:

    float ** grid;
    float ** grid_cart;
    float * spatial_spectrum;
    e3e_complex *twiddle_lut;
    int n_pairs;
    int *pairs;

    e3e_complex *G;
    STFT * stft;

    std::string config_name;

    int n_grid;
    int channels;
    float * mics_loc;
    int fft_size;
    int k_min, k_len;
    int n_frames;

    float fs; // sampling frequency

    float c;  // speed of sound

    float theta;
    int argmax;

    void read_mic_locs();
    void build_lut();
   
    int process();
     

    SRPPHAT(STFT * stft, std::string config, int k_min, int k_len, int n_grid, int n_frames, float fs, float c, int dim);
    ~SRPPHAT();

};

void sample_sp_randpoints(float ** coordinates, int N_samples);
void sample_sp_even_points_2D(float ** spherical, float **cartesian, int N_samples);
void sample_sp_even_points_3D(float ** spherical, float **cartesian, int N_samples);
void sample_sp_even_points_2p5D(float ** spherical, float **cartesian, int N_samples);



#endif // __SRPPHAT_H__
