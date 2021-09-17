/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>
#include <Math/Vector4D.h>

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
    virtual ~EKLMHitMomentum();

    /**
     * Set momentum.
     * @param[in] p momentum
     */
    void setMomentum(const ROOT::Math::PxPyPzEVector& p);

    /**
     * Get momentum.
     * @return Momentum.
     */
    ROOT::Math::PxPyPzEVector getMomentum() const
    {
      return ROOT::Math::PxPyPzEVector(m_pX, m_pY, m_pZ, m_e);
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
