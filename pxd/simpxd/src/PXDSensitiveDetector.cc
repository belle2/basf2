/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/simpxd/PXDSensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <TVector3.h>

#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>

#include <string>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace Simulation;

const G4double c_Epsilon = 1.0e-8;

PXDSensitiveDetector::PXDSensitiveDetector(G4String name) : SensitiveDetectorBase(name)
{
  //Tell the framework that this sensitive detector creates
  //a relation Hits->MCParticle
  addRelationCollection(DEFAULT_PXDSIMHITSREL);

  StoreArray<PXDSimHit> pxdArray(DEFAULT_PXDSIMHITS);
}


PXDSensitiveDetector::~PXDSensitiveDetector()
{

}


G4bool PXDSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  //-------------------------------------------------------
  //               Some basic checks
  //-------------------------------------------------------

  //Get deposited energy (make sure energy was deposited in the sensitive volume)
  const G4double depEnergy = step->GetTotalEnergyDeposit();
  if (fabs(depEnergy) < c_Epsilon) return false;

  // Get step length (make sure step length is not zero)
  const G4double stepLength = step->GetStepLength();
  if (fabs(stepLength) < c_Epsilon) return false;

  //Get particle charge (only charged tracks produce a signal)
  const G4Track& track  = *step->GetTrack();
  const G4double charge = track.GetDefinition()->GetPDGCharge();
  if (fabs(charge) < c_Epsilon) return false;

  //Get time (check for proper global time)
  const G4double globalTime = track.GetGlobalTime();
  if (isnan(globalTime)) {
    B2ERROR("PXD Sensitive Detector: global time is nan !");
    return false;
  }

  //-------------------------------------------------------
  //                Collect information
  //-------------------------------------------------------

  // Get step information
  const G4StepPoint& preStep      = *step->GetPreStepPoint();
  const G4StepPoint& posStep      = *step->GetPostStepPoint();

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
  //TString vname(vol.GetName().data());
  //int layerID = TString(vname(vname.Index("Layer_") + 6)).Atoi();
  //int ladderID = TString(vname(vname.Index("Ladder_") + 7)).Atoi();
  //int sensorID = TString(vname(vname.Index("Sensor_") + 7)).Atoi();

  //Fixes bug in the code above which only returned number from 0 to 9
  //Should be replaced by user info in the future
  int layerID = -1;
  int ladderID = -1;
  int sensorID = -1;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("_");
  tokenizer tokens(string(vol.GetName().data()), sep);

  int index = 0;
  BOOST_FOREACH(const string &tok, tokens) {
    switch (index) {
      case 3 : layerID  = boost::lexical_cast<int>(tok);
        break;
      case 5 : ladderID = boost::lexical_cast<int>(tok);
        break;
      case 7 : sensorID = boost::lexical_cast<int>(tok);
        break;
    }
    index++;
  }

  StoreArray<PXDSimHit> pxdArray(DEFAULT_PXDSIMHITS);

  //Create new PXDSim hit. Convert Geant4 units into basf2 units.
  TVector3 posIn(preStepPosLocal.x() / cm, preStepPosLocal.y() / cm, preStepPosLocal.z() / cm);
  TVector3 posOut(posStepPosLocal.x() / cm, posStepPosLocal.y() / cm, posStepPosLocal.z() / cm);
  TVector3 momInVec(momIn.x() / GeV, momIn.y() / GeV, momIn.z() / GeV);

  int hitIndex = pxdArray->GetLast() + 1;
  new(pxdArray->AddrAt(hitIndex)) PXDSimHit(layerID, ladderID, sensorID,
                                            posIn, posOut, theta / radian,
                                            momInVec, partPDG, trackID,
                                            depEnergy / GeV,
                                            stepLength / cm,
                                            globalTime / ns);

  //Set the SeenInDetector flag
  setSeenInDetectorFlag(step, MCParticle::c_SeenInPXD);

  //Add relation between the created hit and the MCParticle that caused it.
  //The index of the MCParticle has to be set to the TrackID and will be
  //replaced later by the correct MCParticle index automatically.
  StoreArray<Relation> mcPartRelation(getRelationCollectionName());
  StoreArray<MCParticle> mcPartArray(DEFAULT_MCPARTICLES);
  new(mcPartRelation->AddrAt(hitIndex)) Relation(pxdArray, mcPartArray, hitIndex, trackID);

  return true;
}


void PXDSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{

}
