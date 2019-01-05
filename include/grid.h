#ifndef __GRID_H__
#define __GRID_H__

/*
 * Defines a class that allows to create grids of points on the sphere in a uniform or non-uniform way
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

class NonUniformGrid 
{
  public:

    int look_dir, nfft, n_dir, fft_size;
    double beta;
    int n_dir;
    int fs;
    int c;          // Speed of sound
    int frac_cap;
    int frac_back;
    int ndim;

    int dense_dir;
    int sparse_dir;

    double **response;
    double *look_I;

    int *f_hertz;
    e3e_complex *steering_vectors;

    const double gamma_diffuse = 0.1;
    const double gamma_direct = 0.001;

    // Methods definition
    float* process();
    void read_mic_locs();   // Read JSON files
    void build_lut(); 
     
    NonUniformGrid(nfft, std::string config, int *look_dir, double beta, int n_dir, double fs, double c, int ndim);
    ~NonUniformGrid();
};

#endif // __NON_UNIFORM_GRID_H__
