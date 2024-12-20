/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <math.h>

namespace Belle2 {
  class CDCDedxWidgetCurve {

  public:

    CDCDedxWidgetCurve() {};
    virtual ~CDCDedxWidgetCurve() {};

    double meanCurve(double* x, double* par) const
    {
      // calculate the predicted mean value as a function of beta-gamma (bg)
      // this is done with a different function depending on the value of bg
      double f = 0;

      if (par[0] == 1)
        f = par[1] * std::pow(std::sqrt(x[0] * x[0] + 1), par[3]) / std::pow(x[0], par[3]) *
            (par[2] - par[5] * std::log(1 / x[0])) - par[4] + std::exp(par[6] + par[7] * x[0]);
      else if (par[0] == 2)
        f = par[1] * std::pow(x[0], 3) + par[2] * x[0] * x[0] + par[3] * x[0] + par[4];
      else if (par[0] == 3)
        f = -1.0 * par[1] * std::log(par[4] + std::pow(1 / x[0], par[2])) + par[3];

      return f;
    }

    double operator()(double* x, double* par)
    {
      return meanCurve(x, par);
    }

  };
}