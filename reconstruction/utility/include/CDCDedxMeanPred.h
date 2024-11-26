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

#include <vector>

namespace Belle2 {

  /**
  * Class to hold the prediction of mean as a function of beta-gamma (bg)
  */

  class CDCDedxMeanPred {

  public:

    /**
     * Return the mean vector from payload
     */
    std::vector<double> getMeanVector() const
    {
      if (!m_DBMeanPars || m_DBMeanPars->getSize() == 0) {
        B2WARNING("No dE/dx mean parameters!");
        std::vector<double> meanpar;
        for (int i = 0; i < 15; ++i)
          meanpar.push_back(1.0);
        return meanpar;
      } else
        return m_DBMeanPars->getMeanPars();
    }

    /**
    * Return the predicted mean value as a function of beta-gamma (bg)
    */
    double getMean(double bg);

    /**
      * beta-gamma (bg) curve function
      */
    double meanCurve(double* x, double* par, int version) const;


  private:

    std::vector<double> m_meanpars; /**< dE/dx mean parameters */

    const DBObjPtr<CDCDedxMeanPars> m_DBMeanPars; /**< db object for dE/dx mean parameters */

  };
} // Belle2 namespace