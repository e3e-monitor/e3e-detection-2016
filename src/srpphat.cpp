
#include "srpphat.h"

//#define _USE_MATH_DEFINES_



void sample_sp_rand_points(float ** coordinates, int N_samples){
  srand (time(NULL));
  float rnd;

  for(int i = 0; i < N_samples; i++){
    rnd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    coordinates[i][0] =  acos(-1+2*rnd); // Phi
    rnd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    coordinates[i][1] =  2*M_PI*rnd;   // Theta
  }
}

void sample_sp_even_points_2D(float ** coordinates, float **grid_cart, int N_samples)
{

  float omega = 2 * M_PI / float(N_samples);
  float x, y, z;

  float phi2, theta;      
  for (int n = 0 ; n < N_samples ; n++)
  {

    x = cos(float(n) * omega);
    y = sin(float(n) * omega);
    z = 0.;

    phi2 = atan2(y,x);
    theta = atan2(z, sqrt(x*x + y*y));

    coordinates[n][0] = phi2;
    coordinates[n][1] = theta;

    grid_cart[n][0] = x;
    grid_cart[n][1] = y;
    grid_cart[n][2] = z;
  }
}

void sample_sp_even_points_3D(float ** coordinates, float **grid_cart, int N_samples){ 

  float offset = 2.0/N_samples;
  float increment = M_PI * (3.0 - sqrt(5.0));
  float x,y,z,phi,rho;    

  float phi2, theta;      
  for(int i = 0 ; i < N_samples; i ++){
    y = ((i * offset) - 1) + (offset / 2);
    rho = sqrt(1 - pow(y,2));
    phi = ((i) % N_samples) * increment;

    x = cos(phi) * rho;
    z = sin(phi) * rho;

    phi2 = atan2(y,x);
    theta = atan2(z,sqrt(x*x + y*y));

    coordinates[i][0] = phi2;
    coordinates[i][1] = theta;

    grid_cart[i][0] = x;
    grid_cart[i][1] = y;
    grid_cart[i][2] = z;
    //std::cerr << x << " "<< y <<" "<< z << std::endl; 
  }

}

void sample_sp_even_points_2p5D(float ** coordinates, float **grid_cart, int N_samples)
{ 
  int n_flat = 360;
  int n_top = N_samples - n_flat;

  float omega = 2 * M_PI / float(n_flat);
  float x,y,z,phi,rho;    

  float phi2, theta;      

  // sampling on the circle
  for (int n = 0 ; n < n_flat ; n++)
  {

    x = cos(float(n) * omega);
    y = sin(float(n) * omega);
    z = 0.;

    phi2 = atan2(y,x);
    theta = atan2(z, sqrt(x*x + y*y));

    coordinates[n][0] = phi2;
    coordinates[n][1] = theta;

    grid_cart[n][0] = x;
    grid_cart[n][1] = y;
    grid_cart[n][2] = z;
  }

  // Sampling of top half of the sphere
  float offset = 2.0/(2. * n_top);
  float increment = M_PI * (3.0 - sqrt(5.0));

  for(int i = 0 ; i < n_top ; i ++)
  {
    y = ((i * offset) - 1) + (offset / 2);
    rho = sqrt(1 - pow(y,2));
    phi = ((i) % (2 * n_top)) * increment;

    z = sin(phi) * rho;
    x = cos(phi) * rho;

    float t = z;
    z = -y;
    y = t;

    phi2 = atan2(y,x);
    theta = atan2(z,sqrt(x*x + y*y));

    coordinates[n_flat + i][0] = phi2;
    coordinates[n_flat + i][1] = theta;

    grid_cart[n_flat + i][0] = x;
    grid_cart[n_flat + i][1] = y;
    grid_cart[n_flat + i][2] = z;
    //std::cerr << x << " "<< y <<" "<< z << std::endl; 
  }

}



SRPPHAT::SRPPHAT(STFT * stft, std::string config, int k_min, int k_len, int n_grid, int n_frames, float fs, float c, int dim)
  : stft(stft), config_name(config), n_grid(n_grid), k_min(k_min), k_len(k_len), n_frames(n_frames), fs(fs), c(c)
{
  this->fft_size = stft->fft_size;
  this->channels = stft->channels;

  // Create the grid
  this->grid = new float *[n_grid];
  this->grid_cart = new float *[n_grid];
  for (int n = 0 ; n < n_grid ; n++)
  {
    this->grid[n] = new float[2];
    this->grid_cart[n] = new float[3];
  }

  if (dim == 2)
    sample_sp_even_points_2D(this->grid, this->grid_cart, n_grid);
  else if (dim == 3)
    sample_sp_even_points_3D(this->grid, this->grid_cart, n_grid);
  else
    sample_sp_even_points_2p5D(this->grid, this->grid_cart, n_grid);

  // allocate all pairs
  this->pairs = new int[channels * (channels - 1)];
  this->n_pairs = 0;
  for (int i = 0 ; i < channels ; i++)
    for (int j = i+1 ; j < channels ; j++)
    {
      this->pairs[this->n_pairs*2] = i;
      this->pairs[this->n_pairs*2 + 1] = j;
      this->n_pairs++;
    }

  // allocate mic loc array
  this->mics_loc = new float[3 * channels];
  this->read_mic_locs();

  // allocate the spatial spectrum vector
  this->spatial_spectrum = new float[n_grid];

  // allocatee the twiddle look-up factors
  this->build_lut();

  // allocate G matrix
  this->G = new e3e_complex[k_len * this->n_pairs];
  for (int i = 0 ; i < k_len * this->n_pairs ; i++)
    this->G[i] = 0.;

}

SRPPHAT::~SRPPHAT()
{
  for (int n  = 0 ; n < this->n_grid ; n++)
  {
    delete this->grid[n];
    delete this->grid_cart[n];
  }
  delete this->grid;
  delete this->grid_cart;

  delete this->pairs;
  delete this->spatial_spectrum;
  delete this->mics_loc;

  delete this->G;

  delete this->twiddle_lut;
}

int SRPPHAT::process()
{
  // Update G
  for (int k = 0 ; k < k_len ; k++)
    for (int p = 0 ; p < this->n_pairs ; p++)
      {
        int i = this->pairs[p*2];
        int j = this->pairs[p*2 + 1];

        // remove oldest frame
        e3e_complex xi = this->stft->get_fd_sample(this->n_frames, k + this->k_min, i);
        e3e_complex xj = this->stft->get_fd_sample(this->n_frames, k + this->k_min, j);

        this->G[k*this->n_pairs + p] -= xi * std::conj(xj);

        // add newest frame
        xi = this->stft->get_fd_sample(0, k + this->k_min, i);
        xj = this->stft->get_fd_sample(0, k + this->k_min, j);

        this->G[k*this->n_pairs + p] += xi * std::conj(xj);
      }

  // Compute the cost function for all grid points
  this->argmax = 0;
  float max = 0.;

  for (int n = 0 ; n < this->n_grid ; n++)
  {
    e3e_complex tmp(0,0);

    for (int p = 0 ; p < this->n_pairs ; p++)
    {
      for (int k = 0 ; k < this->k_len ; k++)
      {
        int tw_ind = n + p*this->n_grid + k*this->n_grid*this->n_pairs;
        int g_ind = k*this->n_pairs + p;

        e3e_complex Gval = this->G[g_ind];
        float Gabs = std::abs(Gval);
        if (Gabs > 1e-5)
          tmp += Gval / Gabs * this->twiddle_lut[tw_ind];
      }
    }

    //std::cerr << tmp << std::endl;

    this->spatial_spectrum[n] = std::norm(tmp);

    if (this->spatial_spectrum[n] > max)
    {
      max = this->spatial_spectrum[n];
      this->argmax = n;
    }
  }

  return this->argmax;
}

void SRPPHAT::build_lut()
{
  this->twiddle_lut = new e3e_complex[k_len*this->n_pairs*this->n_grid];
  e3e_complex imag(0,1);
  float pi = M_PI;

  for (int k = 0 ; k < k_len ; k++)
    for (int p = 0 ; p < this->n_pairs ; p++)
      for (int n = 0 ; n < n_grid ; n++)
      {
        int i = this->pairs[2*p];
        int j = this->pairs[2*p + 1];

        float ip = 0.;
        for (int v = 0 ; v < 3 ; v++)
        { 
          float delta = this->mics_loc[j*3+v] - this->mics_loc[i*3+v];
          ip += delta * this->grid_cart[n][v];
        }

        float exponent = 2 * pi * float(this->k_min + k) / float(this->fft_size) * this->fs * ip / this->c;
        int ind = n + p*this->n_grid + k*this->n_grid*this->n_pairs;
        this->twiddle_lut[ind] = std::exp(imag * exponent);
        std::cerr << exponent << " " << this->twiddle_lut[ind] << std::endl;
      }

}

void SRPPHAT::read_mic_locs()
{
  std::ifstream fin (this->config_name);

  if(!fin){
    std::cerr << "Failure to open" << std::endl;
    exit(1);
  }  

  int ch;
  float x, y, z;
  std::string header;
  getline(fin, header);

  while(!fin.eof()){

    if (!(fin >> ch))
      break;

    fin >> x >> y >> z;

    if (ch < 0 || ch >= this->channels)
      std::cout << "Error: channel number too large!!!" << std::endl;

    this->mics_loc[ch*3] = x;
    this->mics_loc[ch*3 + 1] = y;
    this->mics_loc[ch*3 + 2] = z;

  }

  fin.close(); //close the input file

}

