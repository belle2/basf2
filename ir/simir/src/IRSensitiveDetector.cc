/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroshi Nakano, Andreas Moll                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ir/simir/IRSensitiveDetector.h>

#include <framework/logging/Logger.h>

// Geant4
#include <G4Types.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Step.hh>
#include <G4UserLimits.hh>

using namespace std;
using namespace Belle2;
using namespace Simulation;


IRSensitiveDetector::IRSensitiveDetector(G4String name) :
    SensitiveDetectorBase(name)
{

}


void IRSensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
{

}


IRSensitiveDetector::~IRSensitiveDetector()
{

}


G4bool IRSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  // Get track
  const G4Track& track  = *step->GetTrack();

  // Get information about the active medium
  const G4VPhysicalVolume& g4Volume    = *track.GetVolume();

  B2INFO("Step in volume: " << g4Volume.GetName())

  return true;
}
