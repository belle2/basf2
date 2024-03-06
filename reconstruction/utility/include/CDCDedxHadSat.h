/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/database/DBObjPtr.h>

#include <reconstruction/dbobjects/CDCDedxHadronCor.h>

#include <vector>

#include <memory>
#include <cmath>
#include <iostream>

namespace Belle2 {

  /**
  * Class to hold the prediction of resolution depending dE/dx, nhit, and cos(theta)
  */

  class CDCDedxHadSat {

  public:

    /**
    * Return the resolution vector from payload
    */
    void getHadronVector()
    {

      // get the hadron correction parameters
      if (!m_DBHadronCor || m_DBHadronCor->getSize() == 0) {
        B2WARNING("No hadron correction parameters!");
        for (int i = 0; i < 4; ++i)
          m_hadronpars.push_back(0.0);
        m_hadronpars.push_back(1.0);
      } else m_hadronpars = m_DBHadronCor->getHadronPars();

    }

    /**
      * resolution functions depending on dE/dx, nhit, and cos(theta)
      */
    double D2I(double cosTheta,  double D) ;
    double I2D(double cosTheta,  double I) ;

  private:

    std::vector<double> m_hadronpars; /**< dE/dx resolution parameters */

    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< db object for dE/dx resolution parameters */

  };
} // Belle2 namespace
