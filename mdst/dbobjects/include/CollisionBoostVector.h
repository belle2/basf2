/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TMatrixDSym.h>
#include <TVector3.h>

namespace Belle2 {
  /** This class contains the measured average boost vector
   * vec(beta) = (beta_x, beta_y, beta_z) = vec(p_e+e-)/E_e+e-
   * of the center-of-mass system in the lab frame and its uncertainty.
   */
  class CollisionBoostVector: public TObject {
  public:

    /** equality operator */
    bool operator==(const CollisionBoostVector& other) const
    {
      return other.m_boost == m_boost && other.m_boostCovariance == m_boostCovariance;
    }

    /** Set the boost vector and its error matrix.
     * @param boost boost vector (beta_x, beta_y, beta_z)
     * @param covariance error matrix of the boost vector.
     */
    void setBoost(const TVector3& boost, const TMatrixDSym& covariance)
    {
      m_boost = boost;
      m_boostCovariance = covariance;
    }

    /** Get the measured average boost vector */
    const TVector3& getBoost() const
    {
      return m_boost;
    }

    /** Get the error matrix of the measured average boost vector */
    const TMatrixDSym& getBoostCovariance() const
    {
      return m_boostCovariance;
    }

  private:

    /** Average boost vector of the center-of-mass system in the lab frame */
    TVector3 m_boost;

    /** Covariance matrix of the boost vector */
    TMatrixDSym m_boostCovariance{3};

    ClassDef(CollisionBoostVector, 1); /**<  Measured average boost vector of CMS in lab frame and its uncertainty **/
  };

} //Belle2 namespace
