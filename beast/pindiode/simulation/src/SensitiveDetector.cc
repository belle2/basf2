/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/pindiode/simulation/SensitiveDetector.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PINDIODE detector */
  namespace pindiode {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("PindiodeSensitiveDetector", Const::invalidDetector)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<PindiodeSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerAsPersistent();
      simHits.registerAsPersistent();
      relMCSimHit.registerAsPersistent();

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
      const int detNb = step->GetTrack()->GetVolume()->GetCopyNo();

      //Ignore everything below 1eV
      if (depEnergy < Unit::eV) return false;



      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<PindiodeSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Add SimHit
      const int hitIndex = simHits.getEntries();
      new(simHits.nextFreeAddress()) PindiodeSimHit(
        depEnergy,
        detNb
      );

      //Add Relation between SimHit and MCParticle with a weight of 1. Since
      //the MCParticle index is not yet defined we use the trackID from Geant4
      relMCSimHit.add(trackID, hitIndex, 1.0);

      return true;
    }

  } //pindiode namespace
} //Belle2 namespace
