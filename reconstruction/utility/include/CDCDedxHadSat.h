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
    void setParameters(std::string infile);

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
    double D2I(double cosTheta, double D, double alpha, double gamma, double delta, double power, double ratio) const;

    /**
    * hadron saturation parameterization part 2
    */
    double I2D(double cosTheta, double I, double alpha, double gamma, double delta, double power, double ratio) const;

  private:

    double m_alpha; /*< the alpha parameter for the hadron saturation correction */
    double m_gamma; /*< the gamma parameter for the hadron saturation correction */
    double m_delta; /*< the delta parameter for the hadron saturation correction */
    double m_power; /*< the power parameter for the hadron saturation correction */
    double m_ratio; /*< the ratio parameter for the hadron saturation correction */

    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< db object for dE/dx hadron saturation parameters */

  };
} // Belle2 namespace
