/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TLorentzVector.h>

namespace Belle2 {

  /**
   * Hit momentum.
   */
  class EKLMHitMomentum {

  public:

    /**
     * Constructor.
     */
    EKLMHitMomentum();

    /**
     * Destructor.
     */
    ~EKLMHitMomentum();

    /**
     * Set momentum.
     * @param[in] p momentum
     */
    void setMomentum(const TLorentzVector& p);

    /**
     * Get momentum.
     * @return Momentum.
     */
    TLorentzVector getMomentum() const
    {
      return TLorentzVector(m_pX, m_pY, m_pZ, m_e);
    }

  protected:

    /** Energy. */
    float m_e;

    /** Momentum X component. */
    float m_pX;

    /** Momentum Y component. */
    float m_pY;

    /**  Momentum Z component.. */
    float m_pZ;

  private:

    /** Class version. */
    ClassDef(Belle2::EKLMHitMomentum, 1);

  };

}
