/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TMatrixDSym.h>
#include <TVector3.h>

namespace Belle2 {
  /** This class contains the measured average boost vector (beta_x, beta_y, beta_z)
   * of the center-of-mass system in the lab frame and its uncertainty.
   */
  class CollisionBoostVector: public TObject {
  public:

    /** equality operator */
    bool operator==(const CollisionBoostVector& b) const
    {
      return b.m_boost == m_boost && b.m_boostCovariance == m_boostCovariance;
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
    const TVector3& getBoost()
    {
      return m_boost;
    }

    /** Get the error matrix of the measured average boost vector */
    const TMatrixDSym& getBoostCovariance()
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
