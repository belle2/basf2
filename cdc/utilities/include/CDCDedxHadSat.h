/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/database/DBObjPtr.h>

#include <cdc/dbobjects/CDCDedxHadronCor.h>

#include <vector>
#include <memory>
#include <cmath>
#include <iostream>
#include <fstream>

namespace Belle2 {

  /**
  * Class to hold the hadron saturation functions
  */

  class CDCDedxHadSat {

  public:

    /**
    * set the parameters
    */
    void setParameters();

    /**
    * set the parameters from file
    */
    void setParameters(const std::string& infile);

    /**
    * hadron saturation parameterization part 1
    */
    double D2I(double cosTheta,  double D) const;

    /**
    * hadron saturation parameterization part 2
    */
    double I2D(double cosTheta,  double I) const;

    /**
    * hadron saturation parameterization part 1
    */
    static double D2I(double cosTheta, double D, double alpha, double gamma, double delta, double power, double ratio);

    /**
    * hadron saturation parameterization part 2
    */
    static double I2D(double cosTheta, double I, double alpha, double gamma, double delta, double power, double ratio);

  private:

    double m_alpha = 0.; /**< the alpha parameter for the hadron saturation correction */
    double m_gamma = 0.; /**< the gamma parameter for the hadron saturation correction */
    double m_delta = 0.; /**< the delta parameter for the hadron saturation correction */
    double m_power = 0.; /**< the power parameter for the hadron saturation correction */
    double m_ratio = 0.; /**< the ratio parameter for the hadron saturation correction */

    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< db object for dE/dx hadron saturation parameters */

  };
} // Belle2 namespace
