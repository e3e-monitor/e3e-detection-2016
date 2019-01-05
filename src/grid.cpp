#include "grid.h"
#include "json.hpp"
using nlohmann::json as json;


NonUniformGrid::NonUniformGrid(nfft, std::string config, double *look_dir, double beta, int n_dir, double fs, double c, int ndim)
: nfft(nfft), config_name(config), look_dir(look_dir), beta(beta), n_dir(n_dir), fs(fs), c(c)
{
  this->fft_size = nfft;
  this->ndim = ndim;
  
  double A = 2 * M_PI * (1 - cos(beta/2));
  this->dense_dir = ceil(frac_cap * 4 * M_PI / A);
  this->sparse_dir = ceil(frac_back * 4 * M_PI) / (4 * M_PI - A);

  this->frac_cap = ceil(n_dir/4);
  this->frac_back = n_dir - frac_cap;

  // Grid for the steering vectors
  double dense_grid[dense_dir * ndim], dense_grid_cart[dense_dir * ndim];
  double sparse_grid[sparse_dir * ndim], sparse_grid_cart[dense_dir * ndim];
  double R;   // matrix
  double dense_points;  // matrix
  float sparse_points;  // matrix

  // Normalize steering direction
  float look_dir_acc = 0;
  for (int i=0;i<ndim;i++)
  {
      look_dir_acc += look_dir[i] * look_dir[i];
  }
   for (int i=0;i<ndim;i++)
  {
      look_dir[i] /= sqrt(look_dir_acc);
  }

  if (ndim == 2)
  {
    sample_sp_even_points_2D(dense_grid, dense_grid_cart, this->dense_dir);
    sample_sp_even_points_2D(sparse_grid, sparse_grid_cart, this->sparse_dir);
  }
  else if (ndim == 3)
  {
    sample_sp_even_points_3D(dense_grid, dense_grid_cart, this->dense_dir);
    sample_sp_even_points_3D(sparse_grid, sparse_grid_cart, this->sparse_dir);
  }
  else
  {
    sample_sp_even_points_2p5D(dense_grid, dense_grid_cart, this->dense_dir);
    sample_sp_even_points_2p5D(sparse_grid, sparse_grid_cart, this->sparse_dir);
  }

  // Create align matrix R in the look_dir


  // Create the dense points using the cartesian coordinates


  // Crete the sparse points using the cartesian coordinates
  

  // Create dense_I and sparse_I


  // Concatante sparse and dense grids


  // Compute target response of the beamformer



  // Deallocate memory
  for (int n  = 0 ; n < this->dense_dir ; n++)
  {
    delete dense_grid[n];
  }

  for (int n  = 0 ; n < this->sparse_dir ; n++)
  {
    delete sparse_grid[n];
  }

}

NonUniformGrid::~NonUniformGrid()
{
  
}

float* NonUniformGrid::process()
{
  
}

void NonUniformGrid::build_lut()
{
  /*
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

        float freq = float(this->k_min + k) / float(this->fft_size) * this->fs;
        float exponent = 2 * pi * freq * ip / this->c;

        int ind = n + p*this->n_grid + k*this->n_grid*this->n_pairs;
        this->twiddle_lut[ind] = std::exp(imag * exponent);
      }
      */

}

void NonUniformGrid::read_mic_locs()
{
  // read pyramic positions from JSON file
  std::ifstream read_positions(this->config_name);
  json positions;
  read_positions >> positions;
}
