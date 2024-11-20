
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  double CDCDedxSigmaPars::sigmaCurve(double x, const double* par, int version) const
  {
    // calculate the predicted mean value as a function of beta-gamma (bg)
    // this is done with a different function depending dE/dx, nhit, and sin(theta)
    double f = 0;

    if (version == 0) {
      if (par[0] == 1) { // return dedx parameterization
        f = par[1] + par[2] * x;
      } else if (par[0] == 2) { // return nhit or sin(theta) parameterization
        f = par[1] * std::pow(x, 4) + par[2] * std::pow(x, 3) +
            par[3] * x * x + par[4] * x + par[5];
      } else if (par[0] == 3) { // return cos(theta) parameterization
        f = par[1] * exp(-0.5 * pow(((x - par[2]) / par[3]), 2)) +
            par[4] * pow(x, 6) + par[5] * pow(x, 5) + par[6] * pow(x, 4) +
            par[7] * pow(x, 3) + par[8] * x * x + par[9] * x + par[10];
      }
    }

    return f;
  }

  double CDCDedxSigmaPars::getSigma(double dedx, double nhit, double cos, double timereso) const
  {

    double x;
    double dedxpar[3];
    double nhitpar[6];
    double cospar[11];

    const auto& params = m_sigmapars;
    if (params.size() < 17) B2FATAL("CDCDedxSigmaPars: vector of parameters too short");

    dedxpar[0] = 1; nhitpar[0] = 2; cospar[0] = 3;
    for (int i = 0; i < 10; ++i) {
      if (i < 2) dedxpar[i + 1] = params[i];
      if (i < 5) nhitpar[i + 1] = params[i + 2];
      cospar[i + 1] = params[i + 7];
    }

    // determine sigma from the parameterization
    x = dedx;
    double corDedx = sigmaCurve(x, dedxpar, 0);

    x = nhit;
    double corNHit;
    int nhit_min = 8, nhit_max = 37;

    if (nhit <  nhit_min) {
      x = nhit_min;
      corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_min / nhit);
    } else if (nhit > nhit_max) {
      x = nhit_max;
      corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_max / nhit);
    } else corNHit = sigmaCurve(x, nhitpar, 0);

    x = cos;
    double corCos = sigmaCurve(x, cospar, 0);

    return (corDedx * corCos * corNHit * timereso);
  }

}
