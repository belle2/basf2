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
    CDCDedxCurvePars(std::vector<double> curvepars): m_curvepars(curvepars) {};

    /**
     * Destructor
     */
    ~CDCDedxCurvePars() {};

    /** Get the number of parameters
     */
    double getSize() const {return m_curvepars.size(); };

    /** Return run gain
     * @return run gain
     */
    double getCurvePar(int par) const {return m_curvepars[par]; };

    /** Set run gain
     * @param run gain
     */
    void setCurvePar(int par, double value) {m_curvepars[par] = value; };

    /** Get the predicted mean according to the beta-gamma curve parameterization
     */
    double getMean(double bg) const
    {
      // define the section of the curve to use
      double A = 0, B = 0, C = 0;
      if (bg < 4.5) A = 1;
      else if (bg < 10) B = 1;
      else C = 1;

      // calculate dE/dx from the parameterized Bethe-Bloch curve
      if (A != 0) A *= m_curvepars[0] * std::pow(std::sqrt(bg * bg + 1), m_curvepars[2]) / std::pow(bg,
                         m_curvepars[2]) * (m_curvepars[1] - m_curvepars[4] * std::log(1 / bg)) - m_curvepars[3] + std::exp(
                           m_curvepars[5] + m_curvepars[6] * bg);
      if (B != 0) B *= m_curvepars[7] * std::pow(bg, 3) + m_curvepars[8] * bg * bg + m_curvepars[9] * bg + m_curvepars[10];
      if (C != 0) C *= -1.0 * m_curvepars[11] * std::log(m_curvepars[14] + std::pow(1 / bg, m_curvepars[12])) + m_curvepars[13];

      return (A + B + C);
    }

  private:
    std::vector<double> m_curvepars; /**< dE/dx curve parameters */

    ClassDef(CDCDedxCurvePars, 1); /**< ClassDef */
  };
} // end namespace Belle2
