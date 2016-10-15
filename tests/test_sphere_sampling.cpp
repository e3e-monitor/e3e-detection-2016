#include <math.h>
#include <random>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <complex>
#include <fftw3.h>
#include <string>
#include "../src/e3e_detection.h"
#include "../src/stft.h"
#include "../src/srpphat.h"

#define NPTS 1000

int main(void){

	//Test for reading microphone channel #, and cartesian coordinates
//	read_mics_locs()
	
	//Sample from spherical coordinates
	float ** spherical = new float *[NPTS];
	float ** cartesian = new float *[NPTS];
  for (int n = 0 ; n < NPTS ; n++)
  {
    spherical[n] = new float[2];
    cartesian[n] = new float[3];
  }
	
	sample_sp_even_points_2p5D(spherical, cartesian, NPTS);

	for(int i = 1; i < NPTS; i++){
		std::cout << cartesian[i][0] << " " << cartesian[i][1] << " " << cartesian[i][2] << std::endl;
	}

	return 0;
}
