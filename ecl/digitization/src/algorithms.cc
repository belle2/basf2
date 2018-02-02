#include <ecl/digitization/algorithms.h>
#include <cmath>
using namespace std;
using namespace Belle2::ECL;

double Belle2::ECL::ShaperDSP(double Ti)
{
  return ShaperDSP(Ti, (const double*)0);
}

double Belle2::ECL::ShaperDSP(double Ti, const float* sf)
{
  if (sf) {
    double sd[10];
    for (int i = 0; i < 10; i++) sd[i] = sf[i]; // conversion to double
    return ShaperDSP(Ti, sd);
  }
  return ShaperDSP(Ti, (const double*)0);
}

double Belle2::ECL::ShaperDSP(double Ti, const double* s)
{
  const double scale = 27.7221;
  static const double defs[10] = {0.5, 0.6483, 0.4017, 0.3741, 0.8494, 0.00144547, 4.7071, 0.8156, 0.5556, 0.2752};
  if (!s) s = defs;
  double t  = Ti * (127. / 144.) - s[0];
  const double dt = 0.2;
  // Sv123 is defined everywhere so no restriction on t
  double fm = Sv123(t - dt, s[2], s[3], s[7], s[8], s[1], s[4]);
  double f0 = Sv123(t   , s[2], s[3], s[7], s[8], s[1], s[4]);
  double fp = Sv123(t + dt, s[2], s[3], s[7], s[8], s[1], s[4]);
  double w = s[9];
  // Is this some kind of a low pass filter?
  double svp = (1 - w) * f0 + (0.5 * w) * (fp + fm);

  if (t > 0) {
    double y = t / s[6], z = t / s[2];
    svp -= s[5] * exp(-y) * (1 - exp(-z) * (1 + z * (1 + z * (1 / 2. + z * (1 / 6. + z * (1 / 24. + z * (1 / 120.)))))));
  }
  return svp * scale;
}
//
double Belle2::ECL::ShaperDSPofflineFit(double Ti, const double* s, double scale)
{
  //
  //Used to fit and draw offline fit results.
  //
  double t  = Ti - s[0];
  const double dt = 0.2;
  // Sv123 is defined everywhere so no restriction on t
  double fm = Sv123(t - dt, s[2], s[3], s[7], s[8], s[1], s[4]);
  double f0 = Sv123(t   , s[2], s[3], s[7], s[8], s[1], s[4]);
  double fp = Sv123(t + dt, s[2], s[3], s[7], s[8], s[1], s[4]);
  double w = s[9];
  // Is this some kind of a low pass filter?
  double svp = (1 - w) * f0 + (0.5 * w) * (fp + fm);

  if (t > 0) {
    double y = t / s[6], z = t / s[2];
    svp -= s[5] * exp(-y) * (1 - exp(-z) * (1 + z * (1 + z * (1 / 2. + z * (1 / 6. + z * (1 / 24. + z * (1 / 120.)))))));
  }
  return svp * scale;
}
