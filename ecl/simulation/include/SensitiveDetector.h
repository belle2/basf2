/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSENSITIVEDETECTOR_H_
#define ECLSENSITIVEDETECTOR_H_

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>

using namespace std;

namespace Belle2 {
  namespace ECL {
    //! The Class for ECL Sensitive Detector
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      //! Constructor
      SensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);

      //! Destructor
      ~SensitiveDetector();

      //! Register ECL hits collection into G4HCofThisEvent
      void Initialize(G4HCofThisEvent* HCTE);

      //! Process each step and calculate variables defined in ECLHit (not yet prepared)
      bool step(G4Step* aStep, G4TouchableHistory* history);

      //! Do what you want to do at the end of each event
      void EndOfEvent(G4HCofThisEvent* eventHC);

      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
      StoreArray<ECLSimHit> m_eclSimHits;
      StoreArray<ECLHit> m_eclHits;
      StoreArray<MCParticle> m_mcParticles;
      RelationArray m_eclSimHitRel;
      RelationArray m_eclHitRel;
    private:
      //! Save ECLSimHit into datastore
      int saveSimHit(G4int, G4int, G4int, G4double, G4double, const G4ThreeVector&, const G4ThreeVector&);

      // members of SensitiveDetector
      G4double m_thresholdEnergyDeposit;// Energy Deposit  threshold
      G4double m_thresholdKineticEnergy;// Kinetic Energy  threshold

      int m_oldEvnetNumber;        // Current event number, change of event number triggers clearing of array m_ECLHitIndex
      int m_trackID;               // track id
      double m_WeightedTime;       // average track time weighted by energy deposition
      double m_energyDeposit;      // total energy deposited in a volume by a track
      G4ThreeVector m_WeightedPos; // average track position weighted by energy deposition
      G4ThreeVector m_momentum;    // initial momentum of track before energy deposition inside sensitive volume

      int m_ECLHitIndex[8736][80];   // Hit index of StoreArray
    };
  } // end of namespace ecl
} // end of namespace Belle2

#endif /* ECLSENSITIVEDETECTOR_H_ */
