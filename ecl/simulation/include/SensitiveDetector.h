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
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>


namespace Belle2 {
  namespace ECL {
    /** Class for ECL Sensitive Detector */
    class SensitiveDetector: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      SensitiveDetector(G4String, G4double, G4double);

      /** Destructor */
      ~SensitiveDetector();

      /** Register ECL hits collection into G4HCofThisEvent */
      void Initialize(G4HCofThisEvent* HCTE);

      /** Process each step and calculate variables defined in ECLHit */
      bool step(G4Step* aStep, G4TouchableHistory* history);

      /** Do what you want to do at the end of each event */
      void EndOfEvent(G4HCofThisEvent* eventHC);

    private:
      /** Create ECLSimHit and ECLHit and relations from MCParticle and put them in datastore */
      int saveSimHit(G4int, G4int, G4int, G4double, G4double, const G4ThreeVector&, const G4ThreeVector&);
      // members of SensitiveDetector
      // G4double m_thresholdEnergyDeposit;// Energy Deposit  threshold
      // G4double m_thresholdKineticEnergy;// Kinetic Energy  threshold

      StoreArray<ECLSimHit> m_eclSimHits;   /**< ECLSimHit array */
      StoreArray<ECLHit> m_eclHits;         /**< ECLHit array */
      StoreArray<MCParticle> m_mcParticles; /**<  MCParticle array */
      RelationArray m_eclSimHitRel; /**< MCParticle to ECLSimHit relation array */
      RelationArray m_eclHitRel;    /**< MCParticle to ECLHit relation array */
      int m_trackID;                /**< current track id */
      double m_WeightedTime;        /**< average track time weighted by energy deposition */
      double m_energyDeposit;       /**< total energy deposited in a volume by a track */
      G4ThreeVector m_WeightedPos;  /**< average track position weighted by energy deposition */
      G4ThreeVector m_momentum;     /**< initial momentum of track before energy deposition inside sensitive volume */

    };
  } // end of namespace ecl
} // end of namespace Belle2

#endif /* ECLSENSITIVEDETECTOR_H_ */
