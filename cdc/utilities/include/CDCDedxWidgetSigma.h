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

  /**
  * Class to hold the beta-gamma (bg) resolution function
  */
  class CDCDedxWidgetSigma {

  public:

    /**
    * Constructor
    */
    CDCDedxWidgetSigma() {};

    /**
    * Destructor
    */
    virtual ~CDCDedxWidgetSigma() {};

    /**
    * calculate the predicted sigma value as a function of beta-gamma (bg)
    * this is done with a different function depending dE/dx, nhit, and cos(theta)
    */
    double sigmaCurve(double* x, const double* par) const
    {

      double f = 0;

      if (par[0] == 1) { // return dedx parameterization
        f = par[1] + par[2] * x[0];
      }

      else if (par[0] == 2) { // return nhit or sin(theta) parameterization
        f = par[1] * std::pow(x[0], 4) + par[2] * std::pow(x[0], 3) +
            par[3] * x[0] * x[0] + par[4] * x[0] + par[5];
      }

      else if (par[0] == 3) { // return cos(theta) parameterization
        f = par[1] * exp(-0.5 * pow(((x[0] - par[2]) / par[3]), 2)) +
            par[4] * pow(x[0], 6) + par[5] * pow(x[0], 5) + par[6] * pow(x[0], 4) +
            par[7] * pow(x[0], 3) + par[8] * x[0] * x[0] + par[9] * x[0] + par[10];
      }

      return f;
    }

    /**
    *  Opertaor to call mean function
    */
    double operator()(double* x, double* par)
    {
      return sigmaCurve(x, par);
    }

  };
}  // Belle2 namespace