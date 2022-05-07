/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <Math/Vector3D.h>
#include <vector>

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
    explicit SphericityEigenvalues(const std::vector<ROOT::Math::XYZVector>& momenta)
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
    void setMomenta(const std::vector<ROOT::Math::XYZVector>& momenta)
    {
      m_momenta.clear();
      m_momenta = momenta;
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
    ROOT::Math::XYZVector getEigenvector(short i) const
    {
      ROOT::Math::XYZVector nullVector(0., 0., 0.);
      return (i < 0 || i > 3) ? nullVector : m_eVector[i];
    }

  private:

    double m_lambda[3] = {0.}; /**<  The eigenvalues */
    ROOT::Math::XYZVector m_eVector[3]; /**<  The eigenvectors */
    std::vector<ROOT::Math::XYZVector> m_momenta; /**< The particles' momenta */
  };

} // Belle2 namespace
