/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <top/dbobjects/TOPPulseHeightPar.h>
#include <cmath>

using namespace std;

namespace Belle2 {

  double TOPPulseHeightPar::getValue(double x) const
  {
    if (x <= 0) return 0;
    double xx = x / x0;
    double f = std::pow(xx, p1) * std::exp(-std::pow(xx, p2));
    return f;
  }

  double TOPPulseHeightPar::getNorm() const
  {
    return p2 / std::tgamma((p1 + 1) / p2) / x0;
  }

  double TOPPulseHeightPar::getMean() const
  {
    return std::tgamma((p1 + 2) / p2) / std::tgamma((p1 + 1) / p2) * x0;
  }

  double TOPPulseHeightPar::getRMS() const
  {
    double x2 = std::tgamma((p1 + 3) / p2) / std::tgamma((p1 + 1) / p2) * x0 * x0;
    double x = getMean();
    return std::sqrt(x2 - x * x);
  }

  double TOPPulseHeightPar::getPeakPosition() const
  {
    return std::pow(p1 / p2, 1 / p2) * x0;
  }

  double TOPPulseHeightPar::getThresholdEffi(double threshold, double rmsNoise, int n) const
  {
    double s = 0;
    double dx = (threshold + rmsNoise) / n;
    double f0 = 0;
    for (int i = 1; i <= n; i++) {
      double f = getValue(dx * i);
      s += (f0 + f) / 2;
      f0 = f;
    }
    return 1 - s * dx * getNorm();
  }

}
