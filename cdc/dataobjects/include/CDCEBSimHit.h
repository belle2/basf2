/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCEBSIMHIT_H
#define CDCEBSIMHIT_H

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationsObject.h>

#include <TVector3.h>

namespace Belle2 {
  /**
   * This SimHit type is used for background studies.
   *  EB stands for electronic board.
   *
   *  @todo CDCEBSimHit : calling a double to save phi a wire id is kind of strange...
   */
  class CDCEBSimHit : public RelationsObject {
  public:
    //! The layer id of this hit.
    int m_layerId;

    //! The wire id of this hit.
    double m_phi;

    //! The track id of this hit.
    int m_trackId;

    //! Particle PDG (can be one of secondaries).
    int m_pdg;

    //! Deposited energy of this hit.
    double m_edep;

    //! The momentum of pre-step.
    TVector3 m_momentum;

    //! The method to set layer id
    void setLayerId(int layerId) { m_layerId = layerId; }

    //! The method to set wire id
    void setPhi(double phi) { m_phi = phi; }

    //! The method to set track id
    void setTrackId(int trackId) { m_trackId = trackId; }

    //! The method to set PDG code
    void setPDGCode(int pdg) { m_pdg = pdg; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set momentum
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }

    //! The method to get layer id
    int getLayerId() const { return m_layerId; }

    //! The method to get wire id
    double getPhi() const { return m_phi; }

    //! The method to get track id
    int getTrackId() const { return m_trackId; }

    //! The method to get PDG code
    int getPDGCode() const { return m_pdg; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get momentum
    TVector3 getMomentum() const { return m_momentum; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    CDCEBSimHit(): m_layerId(0), m_phi(0.0), m_trackId(0),
      m_pdg(0), m_edep(0.0) {}

    //! Useful Constructor
    CDCEBSimHit(int layerId,
                double phi,
                int trackId,
                int pdg,
                double edep,
                TVector3 momentum) {
      m_layerId = layerId;
      m_phi = phi;
      m_trackId = trackId;
      m_pdg = pdg;
      m_edep = edep;
      m_momentum = momentum;
    }

    ClassDef(CDCEBSimHit, 1);
  };
} // end namespace Belle2
#endif
