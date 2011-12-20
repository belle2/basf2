/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveTrack.h>
#include <top/dataobjects/TOPQuartzHit.h>
#include <top/geometry/TOPGeometryPar.h>

#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitiveTrack::SensitiveTrack():
      Simulation::SensitiveDetectorBase("TOP", SensitiveTrack::TOP), m_topgp(TOPGeometryPar::Instance())
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPQuartzHit>  topQuartzHits;

      RelationArray relTOPQuartzHitToMCParticle(topQuartzHits, mcParticles);
      registerMCParticleRelation(relTOPQuartzHitToMCParticle);

    }


    bool SensitiveTrack::step(G4Step* aStep, G4TouchableHistory*)
    {
      // Get track parameters
      StoreArray<MCParticle> mcParticles;


      G4Track* aTrack = aStep->GetTrack();
      G4StepPoint* PrePosition =  aStep->GetPreStepPoint();
      G4ThreeVector worldPosition = PrePosition->GetPosition();
      G4ParticleDefinition* particle = aTrack->GetDefinition();
      int  PDGCharge = (int)(particle->GetPDGCharge());
      G4ThreeVector momentum = PrePosition->GetMomentum();

      // Save only tracks of charged particles
      if (PDGCharge == 0) return(true);

      // Track parameters are saved at the entrance in quarz bar

      if (((PrePosition->GetStepStatus() == fGeomBoundary))) {

        G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

        if (fabs(localPosition.y() + m_topgp->getQthickness() / 2.0) > 10e-10) return(true);


        /*
         B2INFO("SensQuartz: " << aTrack->GetDefinition()->GetParticleName()
         << " " << aTrack->GetTrackID()
         << " " << aTrack->GetParentID()
         << " " << G4BestUnit(localPosition, "Length")
         << " " << G4BestUnit(worldPosition, "Length")
         << " " << G4BestUnit(aTrack->GetMomentum(), "Energy")
         << " " << G4BestUnit(aTrack->GetGlobalTime(), "Time")
         << " Edep is " << G4BestUnit(aStep->GetTotalEnergyDeposit(), "Energy"));*/




        int trackID = aTrack->GetTrackID();
        int  PDGEncoding = particle->GetPDGEncoding();
        double tracklength = aTrack->GetTrackLength() - aStep->GetStepLength();
        const G4double globalTime = PrePosition->GetGlobalTime();
        const G4double localTime = PrePosition->GetLocalTime();
        double vmomentum = sqrt(aTrack->GetVertexKineticEnergy() * aTrack->GetVertexKineticEnergy() + 2 * aTrack->GetVertexKineticEnergy() * particle->GetPDGMass());

        // B2INFO(vmomentum)

        TVector3 TPosition(worldPosition.x() , worldPosition.y() , worldPosition.z());
        TVector3 TMomentum(momentum.x() , momentum.y()  , momentum.z());

        TVector3 TVPosition(aTrack->GetVertexPosition().x() , aTrack->GetVertexPosition().y() , aTrack->GetVertexPosition().z());
        TVector3 TVMomentum(vmomentum * aTrack->GetVertexMomentumDirection().x(), vmomentum * aTrack->GetVertexMomentumDirection().y() , vmomentum * aTrack->GetVertexMomentumDirection().z());
        //B2INFO(aTrack->GetVertexMomentumDirection().x())

        const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
        const int barID = preStep.GetTouchableHandle()->GetReplicaNumber(1);

        // Tracks are saved in "topQuartzHits"
        StoreArray<TOPQuartzHit> topQuartzHits;
        G4int nentr = topQuartzHits->GetEntries();
        new(topQuartzHits->AddrAt(nentr)) TOPQuartzHit(trackID, PDGEncoding, PDGCharge, TPosition, TVPosition, TMomentum, TVMomentum, barID, tracklength, globalTime, localTime, 0, 0, 0, 0, 0, 0, 0);

        RelationArray relTOPQuartzHitToMCParticle(topQuartzHits, mcParticles);
        relTOPQuartzHitToMCParticle.add(nentr, trackID);

      }
      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
