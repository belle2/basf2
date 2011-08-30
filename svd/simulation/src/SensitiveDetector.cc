/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/simulation/SensitiveDetector.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <pxd/vxd/VxdID.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
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
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace svd {

    SensitiveDetector::SensitiveDetector(VXD::SensorInfoBase* sensorInfo):
        VXD::SensitiveDetectorBase(sensorInfo)
    {
      StoreArray<MCParticle> mcParticles;
      StoreArray<SVDSimHit>  svdSimHits;
      StoreArray<SVDTrueHit> svdTrueHits;
      RelationArray  svdSimHitRel(mcParticles, svdSimHits);
      RelationArray  svdTrueHitRel(mcParticles, svdTrueHits);
      registerMCParticleRelation(svdSimHitRel);
      registerMCParticleRelation(svdTrueHitRel);
    }

    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      //--------------------------------------------------------------------------------
      //               Collect information and do some checks
      //--------------------------------------------------------------------------------

      //Get deposited energy (make sure energy was deposited in the sensitive volume)
      const double depEnergy = step->GetTotalEnergyDeposit() * Unit::MeV;
      //Get steplength
      //const double stepLength = step->GetStepLength() * Unit::mm ;

      // Get track
      const G4Track& track  = *step->GetTrack();
      // Get track ID
      const G4int trackID = track.GetTrackID();
      // Get particle PDG code
      const G4int partPDGCode = track.GetDefinition()->GetPDGEncoding();

      //Get particle charge (only keep charged tracks and photons)
      const double minCharge = 0.01 * Unit::e;
      const double partPDGCharge = track.GetDefinition()->GetPDGCharge() * Unit::e;
      if ((fabs(partPDGCharge) < minCharge) && (partPDGCode != 22)) return false;

      // Get particle mass
      //const double partPDGMass = track.GetDefinition()->GetPDGMass() * Unit::MeV;
      //Get time (a better time info will be that from the MCParticle)
      const double globalTime = track.GetGlobalTime() * Unit::ns ;
      // Get step information
      const G4StepPoint& preStep      = *step->GetPreStepPoint();
      const G4StepPoint& posStep      = *step->GetPostStepPoint();

      const G4ThreeVector& preStepPos = preStep.GetPosition();
      const G4ThreeVector& posStepPos = posStep.GetPosition();
      const G4ThreeVector& momIn      = preStep.GetMomentum();

      //G4ThreeVector preStepMomentum   = preStep.GetMomentum();
      //G4ThreeVector posStepMomentum   = posStep.GetMomentum();

      // Get information about the active medium
      // const G4VPhysicalVolume& g4Volume    = *track.GetVolume();

      const G4AffineTransform &topTransform = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform();
      const G4ThreeVector momInLocal = topTransform.TransformAxis(momIn);
      const G4ThreeVector preStepPosLocal = topTransform.TransformPoint(preStepPos);
      const G4ThreeVector posStepPosLocal = topTransform.TransformPoint(posStepPos);

      G4ThreeVector trackVector(posStepPos - preStepPos);
      double theta = trackVector.theta() * Unit::rad;

      //-------------------------------------------------------
      //              Add SimHit to the DataStore
      //-------------------------------------------------------

      VxdID vxdID = preStep.GetTouchableHandle()->GetCopyNumber();
      if (!vxdID) {
        B2ERROR("Could not determine Sensor ID");
        return false;
      }

      TVector3 posIn(preStepPosLocal.x() * Unit::mm, preStepPosLocal.y() * Unit::mm, preStepPosLocal.z() * Unit::mm);
      TVector3 posOut(posStepPosLocal.x() * Unit::mm, posStepPosLocal.y() * Unit::mm, posStepPosLocal.z() * Unit::mm);
      TVector3 momInVec(momInLocal.x() * Unit::MeV, momInLocal.y() * Unit::MeV, momInLocal.z() * Unit::MeV);

      StoreArray<MCParticle> mcParticles;

      //-------------------------------------------------------
      // If Step crossed local x=0, add TrueHit
      //-------------------------------------------------------
      if (posIn.X()*posOut.X() < 0) {
        TVector3 dir = posOut - posIn;
        TVector3 posZero = posIn - posIn.X() / dir.X() * dir;
        StoreArray<SVDTrueHit> svdTrueHits;
        RelationArray   svdTrueHitRel(mcParticles, svdTrueHits);
        int hitIndex = svdTrueHits->GetLast() + 1;
        new(svdTrueHits->AddrAt(hitIndex))
        SVDTrueHit(vxdID, posZero.Y(), posZero.Z(), momInVec, globalTime);
        svdTrueHitRel.add(trackID, hitIndex);
      }

      //Ignore all Steps with less than 1eV Energydeposition
      const double thresholdEnergy = 1 * Unit::eV;
      if (fabs(depEnergy) < thresholdEnergy) return false;

      //Create new PXDSimHit.
      StoreArray<SVDSimHit> svdSimHits;
      RelationArray svdSimHitRel(mcParticles, svdSimHits);

      int hitIndex = svdSimHits->GetLast() + 1 ;
      new(svdSimHits->AddrAt(hitIndex)) SVDSimHit(vxdID, partPDGCode, theta, depEnergy, globalTime,
                                                  posIn, posOut, momInVec);

      //Add relation between the MCParticle and the hit.
      //The index of the MCParticle has to be set to the TrackID and will be
      //replaced later by the correct MCParticle index automatically.
      svdSimHitRel.add(trackID, hitIndex);

      return true;
    }

  } // namespace svd
}  // namespace Belle2
