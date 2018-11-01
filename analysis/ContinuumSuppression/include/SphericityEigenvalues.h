/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Umberto Tamponi (tamponi@to.infn.it)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TVector3.h>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <string>
#include <sstream>


namespace Belle2 {
  /**
   * Class to calculate the Sphericity tensor eigenvalues and eigenvectors starting from an
   * array of 3-momenta
   * The tensor itself is not stored, only its eigenvalues and eigenvectors are.
   */

  class SphericityEigenvalues {
  public:

    /**
     * Constructor with an array of 3-momenta.
     */
    SphericityEigenvalues(const std::vector<TVector3> momenta)
    {
      m_momenta.clear();
      m_momenta = momenta;
    }

    /**
     * Default destructor
     */
    ~SphericityEigenvalues() {};


    /**
     * Sets the list of momenta to be used in the calculation
     * overwriting the previous values
     */
    void setMomenta(std::vector<TVector3> momenta)
    {
      m_momenta.clear();
      m_momenta = momenta;
      return;
    }


    /**
     * Calculates eigenvalues and eigenvectors
     */
    void calculateEigenvalues();


    /**
     * Returns the i-th Eigenvalue
     */
    double getEigenvalue(short i) const
    {
      return (i < 0 || i > 3) ? 0. : m_lambda[i];
    }

    /**
     * Returns the i-th Eigenvector
     */
    TVector3 getEigenvector(short i) const
    {
      TVector3 nullVector(0., 0., 0.);
      return (i < 0 || i > 3) ? nullVector : m_eVector[i];
    }

  private:

    double m_lambda[3] = {0.}; /**<  The eigenvalues */
    TVector3 m_eVector[3]; /**<  The eigenvectors */
    std::vector<TVector3> m_momenta; /**< The particles' momenta */
  };

} // Belle2 namespace
