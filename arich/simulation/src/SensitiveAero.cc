/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/simulation/SensitiveAero.h>
#include <arich/dataobjects/ARICHAeroHit.h>

#include <G4Step.hh>
#include <G4Track.hh>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace arich {

    SensitiveAero::SensitiveAero():
      Simulation::SensitiveDetectorBase("ARICH", Const::ARICH)
    {

      // registration of store arrays and relations

      StoreArray<MCParticle> mcParticles;
      StoreArray<ARICHAeroHit> aeroHits;
      aeroHits.registerInDataStore();
      mcParticles.registerRelationTo(aeroHits);

      // additional registration of MCParticle relation (required for correct relations)

      RelationArray relation(mcParticles, aeroHits);
      registerMCParticleRelation(relation, RelationArray::c_deleteElement);

    }

    bool SensitiveAero::step(G4Step* aStep, G4TouchableHistory*)
    {
      // Get track parameters

      G4Track* aTrack = aStep->GetTrack();

      G4StepPoint* PostPosition = aStep->GetPostStepPoint();
      G4ThreeVector worldPosition = PostPosition->GetPosition();
      G4ParticleDefinition* particle = aTrack->GetDefinition();
      G4double  PDGCharge = particle->GetPDGCharge();
      G4ThreeVector momentum = PostPosition->GetMomentum();

      // Save only tracks of charged particles
      if (PDGCharge == 0) return (true);

      // Track parameters are saved at the entrance in aerogel

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
        int PDGEncoding = particle->GetPDGEncoding();

        TVector3 TPosition(worldPosition.x() * Unit::mm, worldPosition.y() * Unit::mm, worldPosition.z() * Unit::mm);
        TVector3 TMomentum(momentum.x() * Unit::MeV, momentum.y() * Unit::MeV , momentum.z() * Unit::MeV);

        // write the hit in datastore"
        StoreArray<ARICHAeroHit> aeroHits;
        ARICHAeroHit* aeroHit = aeroHits.appendNew(PDGEncoding, TPosition, TMomentum);

        // Create relation to MCParticle
        StoreArray<MCParticle> mcParticles;
        RelationArray rel(mcParticles, aeroHits);
        rel.add(trackID, aeroHit->getArrayIndex());

      }

      return true;

    }

  } // end of namespace arich
} // end of namespace Belle2
