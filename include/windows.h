#ifndef __WINDOWS_H__
#define __WINDOWS_H__
#include <iostream>

class Window
{
  /*
   * This a class implementing the HannWindow
   */
  protected:
    int _size;
    float *buffer;

  public:

    Window(int n) : _size(n), buffer(new float[n]) {}
    ~Window() { delete[] buffer; }

    size_t size() const { return _size; }

    // get operator (no set!)
    float operator [](int i) const { return buffer[i]; }
    // out stream operator
    friend std::ostream& operator<<(std::ostream& os, const Window& win);
};

class HannWindow: public Window
{
  /*
   * This a class implementing the HannWindow
   */
  public:
    HannWindow(int _size);
};

class GrifLimWindow : public Window
{
  /*
   * This class creates the complementary window according
   * to Griffin-Lim rules for STFT least-squares reconstruction
   */
  public:
    int shift; 

    GrifLimWindow(const Window & awindow, int _shift);
};

#endif // __WINDOWS_H__
