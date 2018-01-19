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
#include <simulation/dataobjects/BeamBackHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>

#include "TGraph.h"
#include "TFile.h"
#include <string>

namespace Belle2 {
  namespace ECL {
    class ECLGeometryPar;
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
      TGraph* m_HadronEmissionFunction = nullptr;  /**< Graph for hadron scintillation component emission function */
      double GetHadronIntensityFromDEDX(double);  /**< Evaluates hadron scintillation component emission function */
      /** Create ECLSimHit and ECLHit and relations from MCParticle and put them in datastore */
      int saveSimHit(G4int, G4int, G4int, G4double, G4double, const G4ThreeVector&, const G4ThreeVector&, double);
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
      double m_hadronenergyDeposit;     /**< energy deposited resulting in hadronic scint component */
      G4ThreeVector m_WeightedPos;  /**< average track position weighted by energy deposition */
      G4ThreeVector m_momentum;     /**< initial momentum of track before energy deposition inside sensitive volume */

    };
    /** Class for ECL Sensitive Detector */
    class SensitiveDiode: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      explicit SensitiveDiode(const G4String&);

      /** Destructor */
      ~SensitiveDiode();

      /** Register ECL hits collection into G4HCofThisEvent */
      void Initialize(G4HCofThisEvent* HCTE);

      /** Process each step and calculate variables defined in ECLHit */
      bool step(G4Step* aStep, G4TouchableHistory* history);

      /** Do what you want to do at the end of each event */
      void EndOfEvent(G4HCofThisEvent* eventHC);

    private:
      // members of SensitiveDiode
      ECLGeometryPar* m_eclp; /**< pointer to ECLGeometryPar */
      /** simple hit structure */
      struct hit_t {
        int cellId; /**< cell id */
        double e; /**< energy deposition in a crystal */
        double t; /**< time of energy deposition in a crystal */
      };
      int m_trackID; /**< current track id */
      double m_tsum; /**< average track time weighted by energy deposition */
      double m_esum; /**< total energy deposited in a volume by a track */

      std::vector<hit_t> m_hits; /**< array of hits*/
      std::vector<int> m_cells; /**< array of hitted crystals */
      StoreArray<ECLHit> m_eclHits;  /**< ECLHit array */
    };

    /** Class for ECL Sensitive Detector for neutron background study*/
    class BkgSensitiveDiode: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor */
      explicit BkgSensitiveDiode(const G4String&);

      /** Process each step and calculate variables defined in ECLHit */
      bool step(G4Step* aStep, G4TouchableHistory* history);
    private:
      int m_trackID;          /**< track id */
      TVector3 m_startPos;    /**< particle position at the entrance in volume */
      TVector3 m_startMom;    /**< particle momentum at the entrance in volume */
      double m_startTime;     /**< global time */
      double m_startEnergy;   /**< particle energy at the entrance in volume */
      double m_energyDeposit; /**< energy deposited in volume */
      double m_trackLength;   /**< length of the track in the volume */
      ECLGeometryPar* m_eclp; /**< pointer to ECLGeometryPar */
      StoreArray<MCParticle> m_mcParticles;     /**< MCParticle array */
      StoreArray<BeamBackHit> m_eclBeamBkgHits; /**< BeamBackHit array */
      RelationArray m_eclBeamBkgHitRel;         /**< MCParticle to BeamBackHit relation array */
    };
  } // end of namespace ecl
} // end of namespace Belle2

#endif /* ECLSENSITIVEDETECTOR_H_ */
