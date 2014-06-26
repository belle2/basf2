/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/simulation/SensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <arich/dataobjects/ARICHSimHit.h>

#include <simulation/kernel/UserInfo.h>
#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <TVector2.h>
#include <TRandom3.h>

using namespace std;

namespace Belle2 {
  namespace arich {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("ARICH", Const::ARICH),
      m_arichgp(ARICHGeometryPar::Instance())
    {

      StoreArray<ARICHSimHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, ARICHSimHit>();

      StoreArray<MCParticle> particles;
      StoreArray<ARICHSimHit> hits;
      RelationArray relation(particles, hits);
      registerMCParticleRelation(relation);

    }


    G4bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {
      //Get particle ID
      G4Track& track  = *aStep->GetTrack();
      if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      //Get time (check for proper global time)
      const G4double globalTime = track.GetGlobalTime();
      if (std::isnan(globalTime)) {
        B2ERROR("ARICH Sensitive Detector: global time is nan !");
        return false;
      }

      // apply quantum efficiency if not yet done
      bool applyQE = true;
      Simulation::TrackInfo* info =
        dynamic_cast<Simulation::TrackInfo*>(track.GetUserInformation());
      if (info) applyQE = info->getStatus() < 2;
      if (applyQE) {
        double energy = track.GetKineticEnergy() * Unit::MeV / Unit::eV;
        double qeffi  = m_arichgp->QE(energy) * m_arichgp->getColEffi();
        double fraction = info->getFraction();
        if (gRandom->Uniform() * fraction > qeffi) {
          track.SetTrackStatus(fStopAndKill);
          return false;
        }
      }

      //Get step information
      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
      //Get world position
      const G4ThreeVector& worldPosition = preStep.GetPosition();
      //Transform to local position
      const G4ThreeVector localPosition = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);
      //Get module ID number
      const G4int moduleID = preStep.GetTouchableHandle()->GetReplicaNumber(1);
      //Get photon energy
      const G4double energy = track.GetKineticEnergy() / eV;

      //------------------------------------------------------------
      //                Create ARICHSimHit and save it to datastore
      //------------------------------------------------------------

      TVector2 locpos(localPosition.x() / cm, localPosition.y() / cm);
      StoreArray<ARICHSimHit> arichSimHits;
      if (!arichSimHits.isValid()) arichSimHits.create();
      ARICHSimHit* simHit = arichSimHits.appendNew(moduleID, locpos, globalTime, energy);


      // add relation to MCParticle
      StoreArray<MCParticle> mcParticles;
      RelationArray  arichSimHitRel(mcParticles, arichSimHits);
      int parentID = track.GetParentID();
      arichSimHitRel.add(parentID, simHit->getArrayIndex());

      // after detection photon track is killed
      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace arich
} // end of namespace Belle2
