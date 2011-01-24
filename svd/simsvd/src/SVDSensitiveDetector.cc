/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/simsvd/SVDSensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <TVector3.h>

#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>

#include <cmath>

using namespace Belle2;
using namespace Simulation;

const G4double c_Epsilon = 1.0e-8;

SVDSensitiveDetector::SVDSensitiveDetector(G4String name) : SensitiveDetectorBase(name), m_hitNumber(0)
{

}


SVDSensitiveDetector::~SVDSensitiveDetector()
{

}


G4bool SVDSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  //-------------------------------------------------------
  //               Some basic checks
  //-------------------------------------------------------

  //Get deposited energy (make sure energy was deposited in the sensitive volume)
  const G4double depEnergy = aStep->GetTotalEnergyDeposit();
  if (fabs(depEnergy) < c_Epsilon) return false;

  // Get step length (make sure step length is not zero)
  const G4double stepLength = aStep->GetStepLength();
  if (fabs(stepLength) < c_Epsilon) return false;

  //Get particle charge (only charged tracks produce a signal)
  const G4Track& track  = *aStep->GetTrack();
  const G4double charge = track.GetDefinition()->GetPDGCharge();
  if (fabs(charge) < c_Epsilon) return false;

  //Get time (check for proper global time)
  const G4double globalTime = track.GetGlobalTime();
  if (isnan(globalTime)) {
    B2ERROR("SVD Sensitive Detector: global time is nan !");
    return false;
  }

  //-------------------------------------------------------
  //                Collect information
  //-------------------------------------------------------

  // Get step information
  const G4StepPoint& preStep      = *aStep->GetPreStepPoint();
  const G4StepPoint& posStep      = *aStep->GetPostStepPoint();

  //Get additional information
  const G4int partPDG             = track.GetDefinition()->GetPDGEncoding();
  const G4int trackID             = track.GetTrackID();

  const G4ThreeVector& preStepPos = preStep.GetPosition();
  const G4ThreeVector& posStepPos = posStep.GetPosition();

  G4ThreeVector preStepMomentum   = preStep.GetMomentum();
  G4ThreeVector posStepMomentum   = posStep.GetMomentum();

  const G4VPhysicalVolume& vol    = *track.GetVolume();
  const G4ThreeVector momIn(preStep.GetMomentum());
  const G4ThreeVector preStepPosLocal =
    preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(preStepPos);
  const G4ThreeVector posStepPosLocal =
    preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(posStepPos);
  G4ThreeVector midStepPos(preStepPos); midStepPos += posStepPos;
  midStepPos *= 0.5;
  double theta = midStepPos.theta();

  //-------------------------------------------------------
  //              Add SimHit to the DataStore
  //-------------------------------------------------------
  /* Parse volume name for layer, ladder and sensor numbers */
  TString vname(vol.GetName().data());
  int layerID = TString(vname(vname.Index("Layer_") + 6)).Atoi();
  int ladderID = TString(vname(vname.Index("Ladder_") + 7)).Atoi();
  int sensorID = TString(vname(vname.Index("Sensor_") + 7)).Atoi();

  StoreArray<SVDSimHit> svdArray("SVDSimHitArray");

  //Create new SVDSim hit. Convert Geant4 units into basf2 units.
  TVector3 posIn(preStepPosLocal.x() / cm, preStepPosLocal.y() / cm, preStepPosLocal.z() / cm);
  TVector3 posOut(posStepPosLocal.x() / cm, posStepPosLocal.y() / cm, posStepPosLocal.z() / cm);
  TVector3 momInVec(momIn.x() / GeV, momIn.y() / GeV, momIn.z() / GeV);

  new(svdArray->AddrAt(m_hitNumber)) SVDSimHit(layerID, ladderID, sensorID,
                                               posIn, posOut, theta / radian,
                                               momInVec, partPDG, trackID,
                                               depEnergy / GeV,
                                               stepLength / cm,
                                               globalTime / ns);
  m_hitNumber++;

  return true;
}


void SVDSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{

}
