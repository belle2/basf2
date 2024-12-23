/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/database/DBObjPtr.h>

#include <cdc/utilities/CDCDedxWidgetSigma.h>
#include <cdc/dbobjects/CDCDedxMeanPars.h>
#include <cdc/dbobjects/CDCDedxSigmaPars.h>

#include <vector>

#include <memory>
#include <cmath>

#include <string>
#include <iostream>
#include <fstream>

namespace Belle2 {

  /**
  * Class to hold the prediction of resolution depending dE/dx, nhit, and cos(theta)
  */

  class CDCDedxSigmaPred {

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

    /**
    * get the dedx parameters
    */
    double getDedxPars(int i) { return m_dedxpars[i]; };

    /**
    * set the dedx parameters
    */
    void setDedxPars(int i, double val) { m_dedxpars[i] = val; };

    /**
    * get the nhit parameters
    */
    double getNHitPars(int i) { return m_nhitpars[i]; };

    /**
    * set the nhit parameters
    */
    void setNHitPars(int i, double val) { m_nhitpars[i] = val; };

    /**
    * get the cos(theta) parameters
    */
    double getCosPars(int i) { return m_cospars[i]; };

    /**
    * set the cos(theta) parameters
    */
    void setCosPars(int i, double val) { m_cospars[i] = val; };

  private:

    double m_dedxpars[2]; // parameters for sigma vs. dE/dx curve
    double m_cospars[10]; // parameters for sigma vs. cos(theta) curve
    double m_nhitpars[5]; // parameters for sigma vs. nhit curve

    const DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< db object for dE/dx resolution parameters */

  };
} // Belle2 namespace