/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMHIT_H
#define BKLMSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <framework/datastore/RelationsObject.h>
#include <bklm/dataobjects/BKLMStatus.h>

#define BKLM_INNER 1
#define BKLM_OUTER 2

namespace Belle2 {

  //! Store one simulation hit as a ROOT object
  class BKLMSimHit : public SimHitBase {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMSimHit();

    //! Constructor with initial values (from simulation step)
    BKLMSimHit(unsigned int, int, int, int, bool, int, int, bool, int, int,
               const TVector3&, const TVector3&, double, double, const TVector3&, double, double);

    //! Destructor
    virtual ~BKLMSimHit() {}

    //! Copy constructor
    BKLMSimHit(const BKLMSimHit&);

    //! returns status word
    unsigned int getStatus() const { return m_Status; }

    //! returns true if this hit is in an RPC
    bool isInRPC() const { return ((m_Status & STATUS_INRPC) != 0); }

    //! returns PDG code of leading particle
    unsigned int getPDG() const { return m_PDG; }

    //! returns GEANT4 track ID of leading particle
    unsigned int getTrackID() const { return m_TrackID; }

    //! returns GEANT4 track ID of parent particle
    unsigned int getParentID() const { return m_ParentID; }

    //! returns axial end (TRUE=forward or FALSE=backward) of this hit
    bool isForward() const { return m_IsForward; }

    //! returns sector number of this hit
    int getSector() const { return m_Sector; }

    //! returns layer number of this hit
    int getLayer() const { return m_Layer; }

    //! returns plane (0=inner or 1=outer) of this hit
    int getPlane() const { return (m_IsPhiReadout ? BKLM_INNER : BKLM_OUTER); }

    //! returns readout-coordinate (TRUE=phi, FALSE=z) of this hit
    bool isPhiReadout() const { return m_IsPhiReadout; }

    //! returns unique readout strip number of this hit (assumes one hit)
    int getStrip() const { return m_StripMin; }

    //! returns lowest readout strip number of this hit
    int getStripMin() const { return m_StripMin; }

    //! returns highest readout strip number of this hit
    int getStripMax() const { return m_StripMax; }

    //! returns unique detector-module identifier
    unsigned int getModuleID() const { return m_ModuleID; }

    //! returns global position of the hit
    TVector3 getGlobalPosition() const { return m_GlobalPosition; }

    //! returns global position of the hit (alias for EVEVisualization.cc)
    TVector3 getHitPosition() const { return m_GlobalPosition; }

    //! returns local position of the hit
    TVector3 getLocalPosition() const { return m_LocalPosition; }

    //! returns the event hit time
    double getTime() const { return m_Time; }

    //! returns energy deposition
    double getEDep() const { return m_EDep; }

    //! returns momentum of throughgoing particle
    TVector3 getMomentum() const { return m_Momentum; }

    //! returns total energy of throughgoing particle
    double getEnergy() const { return m_Energy; }

    //! returns kinetic energy of throughgoing particle
    double getKineticEnergy() const { return m_KineticEnergy; }

    //! increase energy deposition
    void increaseEDep(double eDep) { m_EDep += eDep; }

  private:

    //! status word
    unsigned int m_Status;

    //! PDG code of leading particle
    int m_PDG;

    //! GEANT4 track identifier
    int m_TrackID;

    //! GEANT4 parent-track identifier
    int m_ParentID;

    //! axial end (TRUE=forward or FALSE=backward) of the hit
    bool m_IsForward;

    //! sector number of the hit
    int m_Sector;

    //! layer number of the hit
    int m_Layer;

    //! readout-coordinate (TRUE=phi, FALSE=z) of this hit
    bool m_IsPhiReadout;

    //! lowest readout strip number for this hit
    int m_StripMin;

    //! highest readout strip number for this hit
    int m_StripMax;

    //! detector-module identifier, internally calculated
    //! bits 0-5   = 0; reserved bits for strip identifier
    //! bit 6      = plane-1 [0..1]; inner is 0 and phiReadout
    //! bits 7-10  = layer-1 [0..14]
    //! bits 11-13 = sector-1 [0..7]
    //! bit 14     = end-1 [0..1]; forward is 0
    int m_ModuleID;

    //! global-coordinates hit position (cm)
    TVector3 m_GlobalPosition;

    //! local-coordinates hit position (cm)
    TVector3 m_LocalPosition;

    //! event hit time (ns)
    double m_Time;

    //! energy deposition (MeV)
    double m_EDep;

    //! momentum (MeV/c) of throughgoing particle
    TVector3 m_Momentum;

    //! total energy (MeV) of throughgoing particle
    double m_Energy;

    //! kinetic energy (MeV) of throughgoing particle
    double m_KineticEnergy;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMSimHit, 2);

  };

} // end of namespace Belle2

#endif //BKLMSIMHIT_H
