/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <cmath>

namespace Belle2 {

  /**
   *   dE/dx curve (versus beta-gamma) parameterization constants
   */

  class CDCDedxCurvePars: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxCurvePars(): m_curvepars() {};

    /**
     * Constructor
     */
    CDCDedxCurvePars(short version, std::vector<double>& curvepars): m_version(version), m_curvepars(curvepars) {};

    /**
     * Destructor
     */
    ~CDCDedxCurvePars() {};

    /** Get the number of parameters
     */
    double getSize() const {return m_curvepars.size(); };

    /** Get the version for the curve parameterization
     */
    short getVersion() const {return m_version; };

    /** Return vector of curve parameters
     * @return vector of curve parameters
     */
    std::vector<double> getCurvePars() const {return m_curvepars; };

    /** Return specific curve parameter
     * @return curve parameter
     */
    double getCurvePar(int par) const {return m_curvepars[par]; };

    /** Set version number
     * @param version
     */
    void setVersion(short version) {m_version = version; };

    /** Set a parameter
     * @param parameter key
     * @param parameter value
     */
    void setCurvePar(int par, double value) {m_curvepars[par] = value; };

  private:
    short m_version; /**< version number for curve parameterization */
    std::vector<double> m_curvepars; /**< dE/dx curve parameters */

    ClassDef(CDCDedxCurvePars, 1); /**< ClassDef */
  };
} // end namespace Belle2
