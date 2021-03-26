/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <awesome/simulation/SensitiveDetector.h>
#include <awesome/dataobjects/AwesomeSimHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the AWESOME detector */
  namespace awesome {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("AwesomeSensitiveDetector", Const::EDetector::TEST)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<AwesomeSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      //      mcParticles.registerAsPersistent();
      //      simHits.registerAsPersistent();
      //      relMCSimHit.registerAsPersistent();

      //Register the Relation so that the TrackIDs get replaced by the actual
      //MCParticle indices after simulating the events. This is needed as
      //secondary particles might not be stored so everything relating to those
      //particles will be attributed to the last saved mother particle
      registerMCParticleRelation(relMCSimHit);
    }

    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      //Get Track information
      const G4Track& track    = *step->GetTrack();
      const int trackID       = track.GetTrackID();
      const double depEnergy  = step->GetTotalEnergyDeposit() * Unit::MeV;

      //Ignore everything below 1eV
      if (depEnergy < Unit::eV) return false;

      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<AwesomeSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Add SimHit
      simHits.appendNew(depEnergy);

      //const int hitIndex = simHits.GetLast() + 1 ;
      //new(simHits.AddrAt(hitIndex)) AwesomeSimHit(depEnergy);

      //Add Relation between SimHit and MCParticle with a weight of 1. Since
      //the MCParticle index is not yet defined we use the trackID from Geant4
      //relMCSimHit.add(trackID, hitIndex, 1.0);

      return true;
    }

  } //awesome namespace
} //Belle2 namespace
