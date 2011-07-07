/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSENSETIVEDETECTOR_H
#define BKLMSENSETIVEDETECTOR_H

#include <bklm/bklmhit/BKLMSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>


namespace Belle2 {

  //! The Class for BKLM Sensitive Detector
  //! Each qualified simulation step is saved into a collection of BKLMSimHits.

  class BKLMSensitiveDetector: public Simulation::SensitiveDetectorBase {

  public:

    //! Constructor
    BKLMSensitiveDetector(G4String name);

    //! Destructor
    ~BKLMSensitiveDetector() {};

    //! Register BKLM hits collection
    void Initialize(G4HCofThisEvent*);

    //! Process each step in the BKLM
    G4bool ProcessHits(G4Step*, G4TouchableHistory*);

    //! Process each event
    void EndOfEvent(G4HCofThisEvent*);

    //! Overlay one random background event
    void AddbgOne(bool);

  private:

    //! Get HEP particle ID of this track's oldest ancestor
    G4int  GetAncestorPID(G4Track*);

    //! Get HEP particle ID of the event-level parent of this track
    G4int  GetFirstPID();

    //! simulated-hits collection
    BKLMSimHitsCollection* m_HitsCollection;

    //! unique ID for simulated-hits collection
    G4int m_HCID;

    //! identifier of RPC gas material
    G4Material* m_RPCGas;

    //! PDG encoding for neutron
    G4int m_neutronPID;

    //! maximum permissible hit time (based on overflow of LeCroy 1877 TDC)
    G4double m_hitTimeMax;

  };

} // end of namespace Belle2

#endif
