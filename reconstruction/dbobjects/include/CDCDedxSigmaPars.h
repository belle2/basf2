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
   *   dE/dx sigma (versus beta-gamma) parameterization constants
   */

  class CDCDedxSigmaPars: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxSigmaPars(): m_sigmapars() {};

    /**
     * Constructor
     */
    CDCDedxSigmaPars(std::vector<double> sigmapars): m_sigmapars(sigmapars) {};

    /**
     * Destructor
     */
    ~CDCDedxSigmaPars() {};

    /** Get the number of parameters
     */
    double getSize() const {return m_sigmapars.size(); };

    /** Return run gain
     * @return run gain
     */
    double getSigmaPar(int par) const {return m_sigmapars[par]; };

    /** Set run gain
     * @param run gain
     */
    void setSigmaPar(int par, double value) {m_sigmapars[par] = value; };

    /** Get the predicted resolution according to the parameterization
     */
    double getSigma(double dedx, double nhit, double sin) const
    {
      if (nhit < 5) nhit = 5;
      if (sin > 0.99) sin = 0.99;

      double corDedx = m_sigmapars[0] + m_sigmapars[1] * dedx;
      double corNHit = m_sigmapars[2] * std::pow(nhit, 4) + m_sigmapars[3] * std::pow(nhit,
                       3) + m_sigmapars[4] * nhit * nhit + m_sigmapars[5] * nhit + m_sigmapars[6];
      double corSin = m_sigmapars[7] * std::pow(sin, 4) + m_sigmapars[8] * std::pow(sin,
                      3) + m_sigmapars[9] * sin * sin + m_sigmapars[10] * sin + m_sigmapars[11];

      return (corDedx * corSin * corNHit);
    }

  private:
    std::vector<double> m_sigmapars; /**< dE/dx resolution parameters */

    ClassDef(CDCDedxSigmaPars, 1); /**< ClassDef */
  };
} // end namespace Belle2
