/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopoleFieldSetup.h>
#include <simulation/monopoles/G4MonopoleEquation.h>

#include <G4MagneticField.hh>
#include <G4FieldManager.hh>
#include <G4TransportationManager.hh>
#include <G4MagIntegratorStepper.hh>
#include <G4ChordFinder.hh>

#include <G4ClassicalRK4.hh>

#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;
using namespace CLHEP;

G4MonopoleFieldSetup* G4MonopoleFieldSetup::fMonopoleFieldSetup = 0;

G4MonopoleFieldSetup::G4MonopoleFieldSetup()
{
  //Take existing setup from basf2
  fFieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fbasf2ChordFinder = fFieldManager->GetChordFinder();
  fMagneticField = (G4MagneticField*)fFieldManager->GetDetectorField();
  fMonopoleEquation = new G4MonopoleEquation(fMagneticField);
  fMinStep     = 0.01 * mm ; // minimal step of 1 mm is default
  fMonopoleStepper = new G4ClassicalRK4(fMonopoleEquation, 8);   // for time information..
  fMonopoleChordFinder = new G4ChordFinder(fMagneticField, fMinStep, fMonopoleStepper);
}

G4MonopoleFieldSetup* G4MonopoleFieldSetup::GetMonopoleFieldSetup()
{
  if (0 == fMonopoleFieldSetup) {
    static G4MonopoleFieldSetup theInstance;
    fMonopoleFieldSetup = &theInstance;
  }

  return fMonopoleFieldSetup;
}

G4MonopoleFieldSetup::~G4MonopoleFieldSetup()
{
  if (fMonopoleChordFinder) delete fMonopoleChordFinder;
  if (fMonopoleEquation)   delete fMonopoleEquation;
  if (fMonopoleStepper)  delete fMonopoleStepper;
}

void G4MonopoleFieldSetup::SwitchChordFinder(G4int val)
{
  switch (val) {
    case 0:
      fFieldManager->SetChordFinder(fbasf2ChordFinder);
      break;
    case 1:
      fFieldManager->SetChordFinder(fMonopoleChordFinder);
      break;
  }
}
