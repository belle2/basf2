/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveTrack.h>
#include <top/dataobjects/TOPBarHit.h>
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
      Simulation::SensitiveDetectorBase("TOP", SensitiveTrack::TOP),
      m_topgp(TOPGeometryPar::Instance())
    {
      // registration
      StoreArray<TOPBarHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, TOPBarHit>();

      // additional registration of MCParticle relation (required for correct relations)
      StoreArray<MCParticle> particles;
      StoreArray<TOPBarHit>  barhits;
      RelationArray  relation(particles, barhits);
      registerMCParticleRelation(relation);
    }


    bool SensitiveTrack::step(G4Step* aStep, G4TouchableHistory*)
    {

      m_topgp->setGeanUnits();

      //! get particle track
      G4Track* aTrack = aStep->GetTrack();

      //! check which particle hit the bar
      G4ParticleDefinition* particle = aTrack->GetDefinition();

      //! query for it's PDG number
      int  PDG = (int)(particle->GetPDGEncoding());

      // Save all tracks excluding optical photons
      if (PDG == 0) return false;

      //! get the preposition, a step before current position
      G4StepPoint* PrePosition =  aStep->GetPreStepPoint();

      //! get lab frame position of the prestep point
      G4ThreeVector worldPosition = PrePosition->GetPosition();

      //! Check that the hit come from the boundary
      if (PrePosition->GetStepStatus() != fGeomBoundary) return false;

      //! Transform lab frame to bar frame
      G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

      //! Check that it is on the outside boundary not on the glue boundary
      if (fabs(fabs(localPosition.y()) - (m_topgp->getQthickness() / 2)) > 10e-6) {
        return false ;
      }

      //! Get track ID
      int trackID = aTrack->GetTrackID();

      //! get track length and subtract step length to get the length to the boundary
      double tracklength = aTrack->GetTrackLength() - aStep->GetStepLength();

      //! get global time
      double globalTime = PrePosition->GetGlobalTime();

      //! get local time
      double localTime = PrePosition->GetLocalTime();

      //! momentum on the boundary
      G4ThreeVector momentum = PrePosition->GetMomentum();

      //! calculate momentum at vertex position
      double Ekin = aTrack->GetVertexKineticEnergy();
      double vmomentum = sqrt(Ekin * Ekin + 2 * Ekin * particle->GetPDGMass());

      //! Fill three vectors that hold momentum and position
      TVector3 TPosition(worldPosition.x(), worldPosition.y(), worldPosition.z());
      TVector3 TMomentum(momentum.x(), momentum.y(), momentum.z());

      TVector3 TVPosition(aTrack->GetVertexPosition().x(),
                          aTrack->GetVertexPosition().y(),
                          aTrack->GetVertexPosition().z());
      TVector3 TVMomentum(vmomentum * aTrack->GetVertexMomentumDirection().x(),
                          vmomentum * aTrack->GetVertexMomentumDirection().y(),
                          vmomentum * aTrack->GetVertexMomentumDirection().z());

      //! Get bar ID
      int barID = PrePosition->GetTouchableHandle()->GetReplicaNumber(2);

      //! Get the charge of the particle
      int PDGCharge = (int)particle->GetPDGCharge();

      //! convert to Basf units
      TPosition = TPosition * Unit::mm;
      TVPosition = TVPosition * Unit::mm;
      TMomentum = TMomentum  * Unit::MeV;
      TVMomentum = TVMomentum  * Unit::MeV;
      tracklength = tracklength * Unit::mm;


      /*!------------------------------------------------------------
       *                Create TOPBarHit and save it to datastore
       * ------------------------------------------------------------
       */

      StoreArray<TOPBarHit> topTracks;
      if (!topTracks.isValid()) topTracks.create();

      new(topTracks.nextFreeAddress()) TOPBarHit(trackID, PDG, PDGCharge, TPosition,
                                                 TVPosition, TMomentum, TVMomentum,
                                                 barID, tracklength, globalTime, localTime);


      /*!--------------------------------------------------------------------------
       *                Make relation between TOPBarHit and MCParticle
       * --------------------------------------------------------------------------
       */

      StoreArray<MCParticle> mcParticles;
      RelationArray relMCParticleToTOPBarHit(mcParticles, topTracks);
      int last = topTracks.getEntries() - 1;
      relMCParticleToTOPBarHit.add(trackID, last);

      //! everything done successfully
      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
