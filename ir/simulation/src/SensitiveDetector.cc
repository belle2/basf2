/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ir/simulation/SensitiveDetector.h>
#include <ir/dataobjects/IRSimHit.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>

// Geant4
#include <G4Types.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Step.hh>
#include <G4UserLimits.hh>

#include <string>

using namespace std;

namespace Belle2 {
  namespace ir {

    SensitiveDetector::SensitiveDetector() :
      Simulation::SensitiveDetectorBase("IR ", Const::IR)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<IRSimHit>  irSimHits;
      RelationArray  irSimHitRel(mcParticles, irSimHits);
      registerMCParticleRelation(irSimHitRel);
    }

    G4bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      const G4Track& track  = *step->GetTrack();
      const G4int trackID = track.GetTrackID();
      const G4int partPDGCode = track.GetDefinition()->GetPDGEncoding();

      const G4VPhysicalVolume& g4Volume    = *track.GetVolume();
      string Volname = g4Volume.GetName();

      const G4double depEnergy = step->GetTotalEnergyDeposit() * Unit::MeV;

      const G4StepPoint& preStep      = *step->GetPreStepPoint();
      const G4StepPoint& postStep      = *step->GetPostStepPoint();
      const G4ThreeVector& preStepPos = preStep.GetPosition();
      const G4ThreeVector& postStepPos = postStep.GetPosition();
      const G4ThreeVector momIn(preStep.GetMomentum());
      const G4ThreeVector momOut(postStep.GetMomentum());
      const G4ThreeVector preStepPosLocal = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(preStepPos);
      const G4ThreeVector postStepPosLocal = postStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(postStepPos);
      const G4ThreeVector momInLocal = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformAxis(momIn);
      const G4ThreeVector momOutLocal = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformAxis(momOut);
      TVector3 posInVec(preStepPosLocal.x() * Unit::mm, preStepPosLocal.y() * Unit::mm, preStepPosLocal.z() * Unit::mm);
      TVector3 posOutVec(postStepPosLocal.x() * Unit::mm, postStepPosLocal.y() * Unit::mm, postStepPosLocal.z() * Unit::mm);
      TVector3 momInVec(momInLocal.x() * Unit::MeV, momInLocal.y() * Unit::MeV, momInLocal.z() * Unit::MeV);
      TVector3 momOutVec(momOutLocal.x() * Unit::MeV, momOutLocal.y() * Unit::MeV, momOutLocal.z() * Unit::MeV);

      // B2INFO("Step in volume: " << g4Volume.GetName())
      // check that user limits are set properly
      G4UserLimits* userLimits = g4Volume.GetLogicalVolume()->GetUserLimits();
      if (userLimits) {
        B2DEBUG(100, "Volume " << g4Volume.GetName() << ": max. allowed step set to " << userLimits->GetMaxAllowedStep(track));
      }

      StoreArray<IRSimHit> irSimHits;
      IRSimHit* simHit = irSimHits.appendNew(posInVec, momInVec, posOutVec, momOutVec, partPDGCode, depEnergy, Volname);

      // add relation to MCParticles
      StoreArray<MCParticle> mcParticles;
      RelationArray irSimHitRel(mcParticles, irSimHits);
      irSimHitRel.add(trackID, simHit->getArrayIndex());

      return true;
    }

  }
}
