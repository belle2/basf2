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
// #include <algorithm>

namespace Belle2 {

  /**
  * A calibration algorithm for CDC dE/dx electron: 1D enta cleanup correction
  *
  */

  class CDCDedxSigmaPred {

  public:

    /** Initialize the module */
    //virtual void initialize();
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

    double sigmaCurve(double* x, const double* par, int version) const;


    double getSigma(double dedx, double nhit, double cos, double timereso);

    double nhitPrediction(double nhit);
    double ionzPrediction(double dedx);
    double cosPrediction(double cos);

  private:

    // parameters to determine the predicted resolutions
    std::vector<double> m_sigmapars; /**< dE/dx resolution parameters */

    // parameters to determine the predicted resolutions
    const DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< dE/dx resolution parameters */

  };
} // Belle2 namespace