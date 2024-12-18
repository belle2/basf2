/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/database/DBObjPtr.h>

#include <reconstruction/dbobjects/CDCDedxMeanPars.h>
#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>

#include <vector>

#include <memory>
#include <cmath>

namespace Belle2 {

  /**
  * Class to hold the prediction of resolution depending dE/dx, nhit, and cos(theta)
  */

  class CDCDedxSigmaPred {

  public:

    /**
    * Return the resolution vector from payload
    */
    std::vector<double> getSigmaVector() const
    {

      // make sure the resolution parameters are reasonable
      if (!m_DBSigmaPars || m_DBSigmaPars->getSize() == 0) {
        B2WARNING("No dE/dx sigma parameters!");
        std::vector<double> sigmapar;
        for (int i = 0; i < 12; ++i)
          sigmapar.push_back(1.0);
        return sigmapar;
      } else return m_DBSigmaPars->getSigmaPars();
    }

    /**
      * resolution functions depending on dE/dx, nhit, and cos(theta)
      */
    double sigmaCurve(double* x, const double* par, int version) const;

    /**
      * Return the predicted resolution depending on dE/dx, nhit, and cos(theta)
      */
    double getSigma(double dedx, double nhit, double cos, double timereso);

    /**
      * Return sigma from the nhit parameterization
      */
    double nhitPrediction(double nhit);

    /**
    *  Return sigma from the ionization parameterization
    */
    double ionzPrediction(double dedx);

    /**
    *  Return sigma from the cos parameterization
    */
    double cosPrediction(double cos);

  private:

    std::vector<double> m_sigmapars; /**< dE/dx resolution parameters */

    const DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< db object for dE/dx resolution parameters */

  };
} // Belle2 namespace