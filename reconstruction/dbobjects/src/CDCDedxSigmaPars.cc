
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>
#include <framework/logging/Logger.h>
#include <cmath>

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
        f = par[1] * x * x * x * x + par[2] * x * x * x + par[3] * x * x + par[4] * x + par[5];
      } else if (par[0] == 3) { // return cos(theta) parameterization
        f = par[1] * std::exp(-0.5 * std::pow(((x - par[2]) / par[3]), 2)) +
            par[4] * std::pow(x, 6) + par[5] * std::pow(x, 5) + par[6] * std::pow(x, 4) +
            par[7] * x * x * x + par[8] * x * x + par[9] * x + par[10];
      }
    }

    return f;
  }

  double CDCDedxSigmaPars::getSigma(double dedx, double nhit, double cosTheta, double timereso) const
  {
    const auto& params = m_sigmapars;
    if (params.size() < 17) B2FATAL("CDCDedxSigmaPars: vector of parameters too short");

    double dedxpar[3];
    double nhitpar[6];
    double cospar[11];
    dedxpar[0] = 1; nhitpar[0] = 2; cospar[0] = 3;
    for (int i = 0; i < 10; ++i) {
      if (i < 2) dedxpar[i + 1] = params[i];
      if (i < 5) nhitpar[i + 1] = params[i + 2];
      cospar[i + 1] = params[i + 7];
    }

    // determine sigma from the parameterization
    double corDedx = sigmaCurve(dedx, dedxpar, 0);

    double nhit_min = 8, nhit_max = 37;
    double corNHit = 0;
    if (nhit <  nhit_min) {
      corNHit = sigmaCurve(nhit_min, nhitpar, 0) * std::sqrt(nhit_min / nhit);
    } else if (nhit > nhit_max) {
      corNHit = sigmaCurve(nhit_max, nhitpar, 0) * std::sqrt(nhit_max / nhit);
    } else {
      corNHit = sigmaCurve(nhit, nhitpar, 0);
    }

    double corCos = sigmaCurve(cosTheta, cospar, 0);

    return (corDedx * corCos * corNHit * timereso);
  }

}
