#include <cmath>
#include "windows.h"

std::ostream& operator<<(std::ostream& os, const Window& win)
{
    for (int m = 0 ; m < win.size() ; m++)
    {
      os << win[m] << " ";
      if ((m + 1) % 10 == 0)
        os << "\n";
    }
    return os;
}

HannWindow::HannWindow(int s) : Window(s)
{
  float N_inv = 1. / _size;
  for (int n = 0 ; n < _size ; n++)
    buffer[n] = 0.5 * (1. - cos(2 * M_PI * N_inv * n));
}

GrifLimWindow::GrifLimWindow(const Window & awindow, int _shift)
  : Window(awindow.size()), shift(_shift)
{
  int n = 0;
  float norm[this->_size];

  for (int m = 0 ; m < this->_size ; m++)
    norm[m] = 0.;

  // move the window back as far as possible while still overlapping
  while (n - this->shift > -this->_size)
    n -= this->shift;

  while (n < this->_size)
  {
    if (n == 0)
      for (int m = 0 ; m < this->_size ; m++) 
        norm[m] += awindow[m] * awindow[m];

    else if (n < 0)
      for (int m = 0 ; m < n + this->_size ; m++)
        norm[m] += awindow[m-n] * awindow[m-n];

    else
      for (int m = n ; m < this->_size ; m++)
        norm[m] += awindow[m-n] * awindow[m-n];

    n += this->shift;
  }

  for (int m = 0 ; m < this->_size ; m++)
    this->buffer[m] = awindow[m] / norm[m];
}
