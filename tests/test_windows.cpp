#include <iostream>
#include <iomanip>
#include <cmath>

#include "windows.h"

/*
 * The test arrays were generated with pyroomacoustics
 */

// Hann window length 16
float h16[16] = {
  0.        , 0.03806023, 0.14644661, 0.30865828, 0.5       ,
  0.69134172, 0.85355339, 0.96193977, 1.        , 0.96193977,
  0.85355339, 0.69134172, 0.5       , 0.30865828, 0.14644661,
  0.03806023
};

// GL complementary to Hann length 16, shift 2
float h16_s2_gl[16] = {
  0.        , 0.01268674, 0.04881554, 0.10288609, 0.16666667,
  0.23044724, 0.2845178 , 0.32064659, 0.33333333, 0.32064659,
  0.2845178 , 0.23044724, 0.16666667, 0.10288609, 0.04881554,
  0.01268674
};

// GL complementary to Hann length 16, shift 4
float h16_s4_gl[16] = {
  0.        , 0.02537349, 0.09763107, 0.20577219, 0.33333333,
  0.46089448, 0.56903559, 0.64129318, 0.66666667, 0.64129318,
  0.56903559, 0.46089448, 0.33333333, 0.20577219, 0.09763107,
  0.02537349
};

// GL complementary to Hann length 16, shift 6
float h16_s6_gl[16] = {
  0.        , 0.03723923, 0.1404234 , 0.3020002 , 0.51095832,
  0.72323135, 0.87226042, 0.9411897 , 0.95887094, 0.9411897 ,
  0.87226042, 0.72323135, 0.51095832, 0.3020002 , 0.1404234 ,
  0.03723923
};

// GL complementary to Hann length 16, shift 12
float h16_s12_gl[16] = {
  0.        , 0.39351548, 3.41421356, 3.19130495, 2.        ,
  1.44646269, 1.17157288, 1.03956613, 1.        , 1.03956613,
  1.17157288, 1.44646269, 2.        , 3.19130495, 3.41421356,
  0.39351548
};

// Hann window length 128
float h128[128] = {
  0.00000000e+00, 6.02271897e-04, 2.40763666e-03, 5.41174502e-03,
  9.60735980e-03, 1.49843734e-02, 2.15298321e-02, 2.92279674e-02,
  3.80602337e-02, 4.80053534e-02, 5.90393678e-02, 7.11356950e-02,
  8.42651938e-02, 9.83962343e-02, 1.13494773e-01, 1.29524437e-01,
  1.46446609e-01, 1.64220523e-01, 1.82803358e-01, 2.02150348e-01,
  2.22214883e-01, 2.42948628e-01, 2.64301632e-01, 2.86222453e-01,
  3.08658284e-01, 3.31555073e-01, 3.54857661e-01, 3.78509910e-01,
  4.02454839e-01, 4.26634763e-01, 4.50991430e-01, 4.75466163e-01,
  5.00000000e-01, 5.24533837e-01, 5.49008570e-01, 5.73365237e-01,
  5.97545161e-01, 6.21490090e-01, 6.45142339e-01, 6.68444927e-01,
  6.91341716e-01, 7.13777547e-01, 7.35698368e-01, 7.57051372e-01,
  7.77785117e-01, 7.97849652e-01, 8.17196642e-01, 8.35779477e-01,
  8.53553391e-01, 8.70475563e-01, 8.86505227e-01, 9.01603766e-01,
  9.15734806e-01, 9.28864305e-01, 9.40960632e-01, 9.51994647e-01,
  9.61939766e-01, 9.70772033e-01, 9.78470168e-01, 9.85015627e-01,
  9.90392640e-01, 9.94588255e-01, 9.97592363e-01, 9.99397728e-01,
  1.00000000e+00, 9.99397728e-01, 9.97592363e-01, 9.94588255e-01,
  9.90392640e-01, 9.85015627e-01, 9.78470168e-01, 9.70772033e-01,
  9.61939766e-01, 9.51994647e-01, 9.40960632e-01, 9.28864305e-01,
  9.15734806e-01, 9.01603766e-01, 8.86505227e-01, 8.70475563e-01,
  8.53553391e-01, 8.35779477e-01, 8.17196642e-01, 7.97849652e-01,
  7.77785117e-01, 7.57051372e-01, 7.35698368e-01, 7.13777547e-01,
  6.91341716e-01, 6.68444927e-01, 6.45142339e-01, 6.21490090e-01,
  5.97545161e-01, 5.73365237e-01, 5.49008570e-01, 5.24533837e-01,
  5.00000000e-01, 4.75466163e-01, 4.50991430e-01, 4.26634763e-01,
  4.02454839e-01, 3.78509910e-01, 3.54857661e-01, 3.31555073e-01,
  3.08658284e-01, 2.86222453e-01, 2.64301632e-01, 2.42948628e-01,
  2.22214883e-01, 2.02150348e-01, 1.82803358e-01, 1.64220523e-01,
  1.46446609e-01, 1.29524437e-01, 1.13494773e-01, 9.83962343e-02,
  8.42651938e-02, 7.11356950e-02, 5.90393678e-02, 4.80053534e-02,
  3.80602337e-02, 2.92279674e-02, 2.15298321e-02, 1.49843734e-02,
  9.60735980e-03, 5.41174502e-03, 2.40763666e-03, 6.02271897e-04
};

// GL complementary to Hann length 128, shift 64
float h128_s64_gl[128] = {
  0.00000000e+00, 6.02997797e-04, 2.41925800e-03, 5.47063595e-03,
  9.79373573e-03, 1.54401621e-02, 2.24768398e-02, 3.09863625e-02,
  4.10673185e-02, 5.28345170e-02, 6.64190146e-02, 8.19678076e-02,
  9.96430148e-02, 1.19620329e-01, 1.42086466e-01, 1.67235275e-01,
  1.95262146e-01, 2.26356296e-01, 2.60690545e-01, 2.98408244e-01,
  3.39607193e-01, 3.84320674e-01, 4.32496176e-01, 4.83973014e-01,
  5.38460808e-01, 5.95521562e-01, 6.54558799e-01, 7.14817450e-01,
  7.75397854e-01, 8.35285959e-01, 8.93399648e-01, 9.48648325e-01,
  1.00000000e+00, 1.04654797e+00, 1.08756848e+00, 1.12256191e+00,
  1.15127262e+00, 1.17368647e+00, 1.19000839e+00, 1.20062517e+00,
  1.20606003e+00, 1.20692513e+00, 1.20387729e+00, 1.19758031e+00,
  1.18867565e+00, 1.17776158e+00, 1.16538033e+00, 1.15201160e+00,
  1.13807119e+00, 1.12391317e+00, 1.10983432e+00, 1.09607995e+00,
  1.08285014e+00, 1.07030613e+00, 1.05857634e+00, 1.04776184e+00,
  1.03794126e+00, 1.02917503e+00, 1.02150899e+00, 1.01497744e+00,
  1.00960555e+00, 1.00541142e+00, 1.00240761e+00, 1.00060227e+00,
  1.00000000e+00, 1.00060227e+00, 1.00240761e+00, 1.00541142e+00,
  1.00960555e+00, 1.01497744e+00, 1.02150899e+00, 1.02917503e+00,
  1.03794126e+00, 1.04776184e+00, 1.05857634e+00, 1.07030613e+00,
  1.08285014e+00, 1.09607995e+00, 1.10983432e+00, 1.12391317e+00,
  1.13807119e+00, 1.15201160e+00, 1.16538033e+00, 1.17776158e+00,
  1.18867565e+00, 1.19758031e+00, 1.20387729e+00, 1.20692513e+00,
  1.20606003e+00, 1.20062517e+00, 1.19000839e+00, 1.17368647e+00,
  1.15127262e+00, 1.12256191e+00, 1.08756848e+00, 1.04654797e+00,
  1.00000000e+00, 9.48648325e-01, 8.93399648e-01, 8.35285959e-01,
  7.75397854e-01, 7.14817450e-01, 6.54558799e-01, 5.95521562e-01,
  5.38460808e-01, 4.83973014e-01, 4.32496176e-01, 3.84320674e-01,
  3.39607193e-01, 2.98408244e-01, 2.60690545e-01, 2.26356296e-01,
  1.95262146e-01, 1.67235275e-01, 1.42086466e-01, 1.19620329e-01,
  9.96430148e-02, 8.19678076e-02, 6.64190146e-02, 5.28345170e-02,
  4.10673185e-02, 3.09863625e-02, 2.24768398e-02, 1.54401621e-02,
  9.79373573e-03, 5.47063595e-03, 2.41925800e-03, 6.02997797e-04
};

// GL complementary to Hann length 128, shift 88
float h128_s88_gl[128] = {
  0.00000000e+00, 1.34791027e-03, 5.78460637e-03, 1.40099305e-02,
  2.68998796e-02, 4.55490530e-02, 7.13206044e-02, 1.05902262e-01,
  1.51363883e-01, 2.10206287e-01, 2.85381269e-01, 3.80247181e-01,
  4.98402641e-01, 6.43316271e-01, 8.17656094e-01, 1.02224919e+00,
  1.25472022e+00, 1.50810910e+00, 1.77010820e+00, 2.02375391e+00,
  2.25007431e+00, 2.43219090e+00, 2.55926637e+00, 2.62850635e+00,
  2.64451182e+00, 2.61674099e+00, 2.55651887e+00, 2.47470430e+00,
  2.38039629e+00, 2.28052409e+00, 2.17997772e+00, 2.08197565e+00,
  1.98847811e+00, 1.90055364e+00, 1.81866829e+00, 1.74289861e+00,
  1.67308118e+00, 1.60891412e+00, 1.55002394e+00, 1.49600834e+00,
  1.44646269e+00, 1.40099672e+00, 1.35925271e+00, 1.32091432e+00,
  1.28570215e+00, 1.25336897e+00, 1.22369568e+00, 1.19648786e+00,
  1.17157288e+00, 1.14879733e+00, 1.12802493e+00, 1.10913468e+00,
  1.09201921e+00, 1.07658352e+00, 1.06274372e+00, 1.05042607e+00,
  1.03956613e+00, 1.03010796e+00, 1.02200357e+00, 1.01521232e+00,
  1.00970056e+00, 1.00544119e+00, 1.00241345e+00, 1.00060263e+00,
  1.00000000e+00, 1.00060263e+00, 1.00241345e+00, 1.00544119e+00,
  1.00970056e+00, 1.01521232e+00, 1.02200357e+00, 1.03010796e+00,
  1.03956613e+00, 1.05042607e+00, 1.06274372e+00, 1.07658352e+00,
  1.09201921e+00, 1.10913468e+00, 1.12802493e+00, 1.14879733e+00,
  1.17157288e+00, 1.19648786e+00, 1.22369568e+00, 1.25336897e+00,
  1.28570215e+00, 1.32091432e+00, 1.35925271e+00, 1.40099672e+00,
  1.44646269e+00, 1.49600834e+00, 1.55002394e+00, 1.60891412e+00,
  1.67308118e+00, 1.74289861e+00, 1.81866829e+00, 1.90055364e+00,
  1.98847811e+00, 2.08197565e+00, 2.17997772e+00, 2.28052409e+00,
  2.38039629e+00, 2.47470430e+00, 2.55651887e+00, 2.61674099e+00,
  2.64451182e+00, 2.62850635e+00, 2.55926637e+00, 2.43219090e+00,
  2.25007431e+00, 2.02375391e+00, 1.77010820e+00, 1.50810910e+00,
  1.25472022e+00, 1.02224919e+00, 8.17656094e-01, 6.43316271e-01,
  4.98402641e-01, 3.80247181e-01, 2.85381269e-01, 2.10206287e-01,
  1.51363883e-01, 1.05902262e-01, 7.13206044e-02, 4.55490530e-02,
  2.68998796e-02, 1.40099305e-02, 5.78460637e-03, 1.34791027e-03
};

bool check_equal(const Window & win, float *arr)
{
  for (int m = 0 ; m < win.size() ; m++)
    if (fabsf(win[m] - arr[m]) > 1e-5)
      return false;

  return true;
}

int main(int argc, char **argv)
{
  std::cout << "Checking a few sizes. 0: failed, 1: passed\n";

  HannWindow hann16(16);
  std::cout << "Hann 16: " << check_equal(hann16, h16) << '\n';
  std::cout.flush();

  HannWindow hann128(128);
  std::cout << "Hann 128: " << check_equal(hann128, h128) << '\n';
  std::cout.flush();

  GrifLimWindow glh128_s64(hann128, 64);
  std::cout << "GriLim H128 S64: " << check_equal(glh128_s64, h128_s64_gl) << '\n';
  std::cout.flush();

  GrifLimWindow glh128_s88(hann128, 88);
  std::cout << "GriLim H128 S88: " << check_equal(glh128_s88, h128_s88_gl) << '\n';
  std::cout.flush();

  GrifLimWindow glh16_s2(hann16, 2);
  std::cout << "GriLim H16 S2: " << check_equal(glh16_s2, h16_s2_gl) << '\n';
  std::cout.flush();

  GrifLimWindow glh16_s4(hann16, 4);
  std::cout << "GriLim H16 S4: " << check_equal(glh16_s4, h16_s4_gl) << '\n';
  std::cout.flush();

  GrifLimWindow glh16_s6(hann16, 6);
  std::cout << "GriLim H16 S6: " << check_equal(glh16_s6, h16_s6_gl) << '\n';
  std::cout.flush();

  GrifLimWindow glh16_s12(hann16, 12);
  std::cout << "GriLim H16 S12: " << check_equal(glh16_s12, h16_s12_gl) << '\n';
  std::cout.flush();
}
