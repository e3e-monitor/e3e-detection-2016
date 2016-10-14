
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

#include <fftw3.h>

#define N 512

int main(int argc, char **argv)
{
  float in_array[N], back_array[N];
  std::complex<float> out_array[N/2+1];
  fftwf_complex *X;
  int dims[] = { N };

  X = reinterpret_cast<fftwf_complex *>(out_array);

  fftwf_plan p_for, p_bac;

  double error = 0.;

  p_for = fftwf_plan_dft_r2c_1d(N, in_array, X, FFTW_ESTIMATE);
  p_bac = fftwf_plan_dft_c2r_1d(N, X, back_array, FFTW_ESTIMATE);

  // simple test array
  for (int i = 0 ; i < N ; i++)
    in_array[i] = i+1;

  fftwf_execute(p_for);
  fftwf_execute(p_bac);

  // run fftw
  for (int i = 0 ; i < N ; i++)
  {
    double e = in_array[i] - back_array[i] / N;
    error += e*e;
  }

  std::cout << "Error: " << error << std::endl;

}
 
