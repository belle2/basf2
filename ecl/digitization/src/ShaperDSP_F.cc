#include <ecl/digitization/algorithms.h>
#include <cmath>
using namespace std;
using namespace Belle2::ECL;

double Belle2::ECL::ShaperDSP_F(double Ti, float* ss)
{

  double svp = 0;
  double tr1 = Ti * 0.881944444;

  double s[12] = {0, 27.7221, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  s[2] = (double) * (ss + 0);
  s[3] = (double) * (ss + 1);
  s[4] = (double) * (ss + 2);
  s[5] = (double) * (ss + 3);
  s[6] = (double) * (ss + 4);
  s[7] = (double) * (ss + 5);
  s[8] = (double) * (ss + 6);
  s[9] = (double) * (ss + 7);
  s[10] = (double) * (ss + 8);
  s[11] = (double) * (ss + 9);

  double tr = tr1 - s[2];
  double tr2 = tr + .2;
  double tr3 = tr - .2;
  if (tr2 > 0.) {

    svp = (Sv123(tr , s[4], s[5], s[9], s[10], s[3], s[6]) * (1 - s[11])
           + s[11] * .5 * (Sv123(tr2, s[4], s[5], s[9], s[10], s[3], s[6])
                           + Sv123(tr3, s[4], s[5], s[9], s[10], s[3], s[6])));
    double x = tr / s[4];


    svp = s[1] * (svp - s[7] * (exp(-tr / s[8]) *
                                (1 - exp(-x) * (1 + x + x * x / 2 + x * x * x / 6 + x * x * x * x / 24 + x * x * x * x * x / 120))));
  } else svp = 0 ;
  return svp;

}
