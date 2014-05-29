/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/simulation/SensitiveAero.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <framework/logging/Logger.h>

#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace arich {

    SensitiveAero::SensitiveAero():
      Simulation::SensitiveDetectorBase("ARICH", Const::ARICH)
    {
      StoreArray<ARICHAeroHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, ARICHAeroHit>();

      StoreArray<MCParticle> particles;
      StoreArray<ARICHAeroHit> hits;
      RelationArray relation(particles, hits);
      registerMCParticleRelation(relation, RelationArray::c_deleteElement);

    }

    bool SensitiveAero::step(G4Step* aStep, G4TouchableHistory*)
    {
      // Get track parameters

      G4Track* aTrack = aStep->GetTrack();
      //G4StepPoint* PrePosition =  aStep->GetPreStepPoint();
      G4StepPoint* PostPosition = aStep->GetPostStepPoint();
      G4ThreeVector worldPosition = PostPosition->GetPosition();
      G4ParticleDefinition* particle = aTrack->GetDefinition();
      G4double  PDGCharge = particle->GetPDGCharge();
      G4ThreeVector momentum = PostPosition->GetMomentum();
      // Save only tracks of charged particles
      if (PDGCharge == 0) return (true);

      // Track parameters are saved at the entrance in aerogel
      //if (((PrePosition->GetStepStatus() == fGeomBoundary)) && (momentum.z() > 0)) {
      if ((PostPosition->GetStepStatus() == fGeomBoundary) && (momentum.z() > 0.0)) {

        /*       B2INFO ("SensAero: " << aTrack->GetDefinition()->GetParticleName()
           << " " << aTrack->GetTrackID()
           << " " << aTrack->GetParentID()
           << " " << G4BestUnit(worldPosition,"Length")
           << " " << G4BestUnit(aTrack->GetMomentum(), "Energy")
           << " " << G4BestUnit(aTrack->GetGlobalTime(), "Time")
           << " Edep is " << G4BestUnit(aStep->GetTotalEnergyDeposit(),"Energy"));
        */

        int trackID = aTrack->GetTrackID();
        int  PDGEncoding = particle->GetPDGEncoding();

        G4ThreeVector localPosition = PostPosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);
        TVector3 TPosition(worldPosition.x() * Unit::mm, worldPosition.y() * Unit::mm, worldPosition.z() * Unit::mm);
        TVector3 TMomentum(momentum.x() * Unit::MeV, momentum.y() * Unit::MeV , momentum.z() * Unit::MeV);

        // Tracks are saved in "arichAeroHits"
        StoreArray<ARICHAeroHit> arichAeroHits;
        if (!arichAeroHits.isValid()) arichAeroHits.create();

        new(arichAeroHits.nextFreeAddress()) ARICHAeroHit(PDGEncoding, TPosition, TMomentum);

        // Create relation to MCParticle
        StoreArray<MCParticle> mcParticles;
        RelationArray  arichAeroHitRel(mcParticles, arichAeroHits);
        int nentr = arichAeroHits.getEntries() - 1;
        arichAeroHitRel.add(trackID, nentr);
      }
      return true;
    }


  } // end of namespace arich
} // end of namespace Belle2
