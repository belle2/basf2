/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveBar.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/geometry/TOPGeometryPar.h>

#include <simulation/kernel/UserInfo.h>
#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <TRandom3.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitiveBar::SensitiveBar():
      Simulation::SensitiveDetectorBase("TOP", Const::TOP),
      m_topgp(TOPGeometryPar::Instance())
    {
      // registration
      StoreArray<TOPBarHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, TOPBarHit>();

      // additional registration of MCParticle relation (required for correct relations)
      StoreArray<MCParticle> particles;
      StoreArray<TOPBarHit>  barhits;
      RelationArray  relation(particles, barhits);
      registerMCParticleRelation(relation, RelationArray::c_deleteElement);
    }


    bool SensitiveBar::step(G4Step* aStep, G4TouchableHistory*)
    {

      m_topgp->setGeanUnits();

      //! get track and particle definition
      G4Track* aTrack = aStep->GetTrack();
      G4ParticleDefinition* particle = aTrack->GetDefinition();

      //! if optical photon, apply QE and return false
      if (particle == G4OpticalPhoton::OpticalPhotonDefinition()) {
        Simulation::TrackInfo* info =
          dynamic_cast<Simulation::TrackInfo*>(aTrack->GetUserInformation());
        if (!info) return false;
        if (info->getStatus() < 2) {
          double energy = aTrack->GetKineticEnergy() * Unit::MeV / Unit::eV;
          double qeffi = m_topgp->QE(energy) * m_topgp->getColEffi();
          double fraction = info->getFraction();
          if (gRandom->Uniform() * fraction > qeffi) {
            aTrack->SetTrackStatus(fStopAndKill);
            return false;
          }
          info->setStatus(2);
          info->setFraction(qeffi);
        }
        return false;
      }

      //! continue for other particles

      //! get the prestep position, a step before current position
      G4StepPoint* PrePosition =  aStep->GetPreStepPoint();

      //! Check that the hit comes from the bar boundary
      if (PrePosition->GetStepStatus() != fGeomBoundary) return false;

      //! get lab frame position of the prestep point
      G4ThreeVector worldPosition = PrePosition->GetPosition();

      //! Transform lab frame to bar frame
      G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

      //! Check that it is not on the glue boundary or similar
      //!!!! this check is not a whole story -> to be re-written
      if (fabs(fabs(localPosition.y()) - (m_topgp->getQthickness() / 2)) > 10e-6) {
        return false ;
      }

      //! Get track ID
      int trackID = aTrack->GetTrackID();

      //! get track length and subtract step length to get the length to the boundary
      double tracklength = aTrack->GetTrackLength() - aStep->GetStepLength();

      //! get global time
      double globalTime = PrePosition->GetGlobalTime();

      //! momentum on the boundary
      G4ThreeVector momentum = PrePosition->GetMomentum();

      //! Fill three vectors that hold momentum and position
      TVector3 TPosition(worldPosition.x(), worldPosition.y(), worldPosition.z());
      TVector3 TMomentum(momentum.x(), momentum.y(), momentum.z());
      TVector3 TOrigin(aTrack->GetVertexPosition().x(),
                       aTrack->GetVertexPosition().y(),
                       aTrack->GetVertexPosition().z());

      //! convert to Basf units
      TPosition = TPosition * Unit::mm;
      TMomentum = TMomentum  * Unit::MeV;
      TOrigin = TOrigin * Unit::mm;
      tracklength = tracklength * Unit::mm;

      //! Get bar ID
      int barID = PrePosition->GetTouchableHandle()->GetReplicaNumber(2);

      //! Get PDG
      int PDG = (int)(particle->GetPDGEncoding());

      //! write the hit to datastore
      StoreArray<TOPBarHit> barHits;
      if (!barHits.isValid()) barHits.create();
      TOPBarHit* hit = barHits.appendNew(barID, PDG, TOrigin, TPosition, TMomentum,
                                         globalTime, tracklength);

      //! set the relation
      StoreArray<MCParticle> mcParticles;
      RelationArray rel(mcParticles, barHits);
      rel.add(trackID, hit->getArrayIndex());

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
