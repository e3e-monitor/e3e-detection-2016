#include <math.h>
#include <iostream>
#include <random>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#ifndef N
#define N 1000
#endif

#define _USE_MATH_DEFINES_

void sample_sp_rand_points(float ** coordinates, double r){
	srand (time(NULL));
        // float * rnd;
        // rnd = (float*) malloc(sizeof(float)*N);
	float rnd;

	for(int i = 0; i < N; i++){
		rnd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		coordinates[i][0] =  acos(-1+2*rnd); // Phi
		rnd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		coordinates[i][1] =  2*M_PI*rnd;   // Theta
	}
}

void sample_sp_even_points(float ** coordinates, double r){ 

	float offset = 2.0/N;
	float increment = M_PI * (3.0 - sqrt(5.0));
	float x,y,z,phi,rho;	

	float phi2, theta;	
	for(int i = 0 ; i < N; i ++){
		y = ((i * offset) - 1) + (offset / 2);
	        rho = sqrt(1 - pow(y,2));
		phi = ((i) % N) * increment;

       		x = cos(phi) * rho;
	        z = sin(phi) * rho;
		
		phi2 = atan2(y,x);
		theta = atan2(z,sqrt(x*x + y*y));
		
		coordinates[i][0] = phi2;
		coordinates[i][1] = theta;
		//std::cerr << x << " "<< y <<" "<< z << std::endl; 
	}





}
int main(void) {

	float** coordinates = new float*[N];
	for(int i = 0; i < N; ++i)
    		coordinates[i] = new float[2]; 

	//sample_sp_rand_points(coordinates,15.0);
	sample_sp_even_points(coordinates,15.0);
   	
	for(int i = 1; i < N; i++)
		std::cerr << coordinates[i][0] << " " <<coordinates[i][1] <<std::endl;





  return 0;
}

