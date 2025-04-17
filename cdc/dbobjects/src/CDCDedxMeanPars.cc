/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/dbobjects/CDCDedxMeanPars.h>
#include <framework/logging/Logger.h>
#include <cmath>

namespace Belle2 {

  double CDCDedxMeanPars::meanCurve(double x, const double* par, int version) const
  {
    // calculate the predicted mean value as a function of beta-gamma (bg)
    // this is done with a different function depending on the value of bg
    double f = 0;

    if (version == 0) {
      if (par[0] == 1)
        f = par[1] * std::pow(std::sqrt(x * x + 1), par[3]) / std::pow(x, par[3]) *
            (par[2] - par[5] * std::log(1 / x)) - par[4] + std::exp(par[6] + par[7] * x);
      else if (par[0] == 2)
        f = par[1] * x * x * x + par[2] * x * x + par[3] * x + par[4];
      else if (par[0] == 3)
        f = -1.0 * par[1] * std::log(par[4] + std::pow(1 / x, par[2])) + par[3];
    }

    return f;
  }

  double CDCDedxMeanPars::getMean(double bg) const
  {
    // define the section of the mean to use
    double A = 0, B = 0, C = 0;
    if (bg < 4.5)
      A = 1;
    else if (bg < 10)
      B = 1;
    else
      C = 1;

    double parsA[9];
    double parsB[5];
    double parsC[5];

    const auto& params = m_meanpars;
    if (params.size() < 15) B2FATAL("CDCDedxMeanPars: vector of parameters too short");

    parsA[0] = 1; parsB[0] = 2; parsC[0] = 3;
    for (int i = 0; i < 15; ++i) {
      if (i < 7) parsA[i + 1] = params[i];
      else if (i < 11) parsB[i % 7 + 1] = params[i];
      else parsC[i % 11 + 1] = params[i];
    }

    // calculate dE/dx from the Bethe-Bloch mean
    double partA = meanCurve(bg, parsA, 0);
    double partB = meanCurve(bg, parsB, 0);
    double partC = meanCurve(bg, parsC, 0);

    return (A * partA + B * partB + C * partC);
  }

}
