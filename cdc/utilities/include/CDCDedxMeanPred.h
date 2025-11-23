/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/database/DBObjPtr.h>
#include <cdc/dbobjects/CDCDedxMeanPars.h>

#include <cdc/utilities/CDCDedxWidgetCurve.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

namespace Belle2 {

  /**
  * Class to hold the prediction of mean as a function of beta-gamma (bg)
  */

  class CDCDedxMeanPred {

  public:

    /**
    * set the parameters from file
    */
    void setParameters(std::string infile);

    /**
    * set the parameters
    */
    void setParameters();

    /**
    * write the parameters in file
    */
    void printParameters(std::string infile);

    /**
    * Return the predicted mean value as a function of beta-gamma (bg)
    */
    double getMean(double bg);

    /**
    * get the curve parameters
    */
    double getCurvePars(int i) { return m_meanpars[i]; };

    /**
    * set the curve parameters
    */
    void setCurvePars(int i, double val) { m_meanpars[i] = val; };

  private:

    double m_meanpars[15]; /**< parameters for beta-gamma curve */

    const DBObjPtr<CDCDedxMeanPars> m_DBMeanPars; /**< db object for dE/dx mean parameters */

  };
} // Belle2 namespace