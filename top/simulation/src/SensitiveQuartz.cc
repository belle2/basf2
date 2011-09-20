/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveQuartz.h>
#include <top/dataobjects/TOPQuartzHit.h>
#include <framework/logging/Logger.h>
#include <top/geometry/TOPGeometryPar.h>

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
  namespace top {

    SensitiveQuartz::SensitiveQuartz():
        Simulation::SensitiveDetectorBase("TOP", SensitiveQuartz::TOP), m_topgp(TOPGeometryPar::Instance())
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPQuartzHit>  topQuartzHits;
    }


    bool SensitiveQuartz::step(G4Step* aStep, G4TouchableHistory*)
    {
      // Get track parameters


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

//        if (fabs(localPosition.y() + m_topgp->getQthickness() / 2.0) > 10e-5) return(true);


        /**
         B2INFO ("SensQuartz: " << aTrack->GetDefinition()->GetParticleName()
         << " " << aTrack->GetTrackID()
         << " " << aTrack->GetParentID()
         << " " << material
         << " " << G4BestUnit(localPosition,"Length")
         << " " << G4BestUnit(worldPosition,"Length")
         << " " << G4BestUnit(aTrack->GetMomentum(), "Energy")
         << " " << G4BestUnit(aTrack->GetGlobalTime(), "Time")
         << " Edep is " << G4BestUnit(aStep->GetTotalEnergyDeposit(),"Energy"));
         */



        int trackID = aTrack->GetTrackID();
        int  PDGEncoding = particle->GetPDGEncoding();
        double tracklength = aTrack->GetTrackLength();

        TVector3 TPosition(worldPosition.x() * Unit::mm, worldPosition.y() * Unit::mm, worldPosition.z() * Unit::mm);
        TVector3 TMomentum(momentum.x() * Unit::MeV, momentum.y() * Unit::MeV , momentum.z() * Unit::MeV);

        // Tracks are saved in "topQuartzHits"
        StoreArray<TOPQuartzHit> topQuartzHits;
        G4int nentr = topQuartzHits->GetEntries();
        new(topQuartzHits->AddrAt(nentr)) TOPQuartzHit(trackID, PDGEncoding, PDGCharge, TPosition, TMomentum, tracklength, 0, 0, 0, 0, 0, 0, 0);

      }
      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
