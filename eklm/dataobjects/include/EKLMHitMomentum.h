/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITMOMENTUM_H
#define EKLMHITMOMENTUM_H

/* Extrenal headers. */
#include <CLHEP/Vector/LorentzVector.h>
#include <TObject.h>
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
    virtual ~EKLMHitMomentum();

    /**
     * Set momentum.
     * @param[in] p momentum
     */
    void setMomentum(CLHEP::HepLorentzVector p);

    /**
     * Get momentum.
     * @return Momentum.
     */
    CLHEP::HepLorentzVector getMomentum() const;

    /**
     * Get momentum, TLorentzVector version.
     * @return Momentum.
     */
    TLorentzVector getMomentumRoot() const;

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

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMHitMomentum, 1);

  };

}

#endif

