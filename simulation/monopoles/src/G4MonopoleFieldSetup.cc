/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopoleFieldSetup.h>
#include <simulation/monopoles/G4MonopoleFieldMessenger.h>
#include <simulation/monopoles/G4MonopoleEquation.h>

#include <G4MagneticField.hh>
#include <G4UniformMagField.hh>
#include <G4FieldManager.hh>
#include <G4TransportationManager.hh>
#include <G4Mag_UsualEqRhs.hh>
#include <G4MagIntegratorStepper.hh>
#include <G4ChordFinder.hh>

// #include "G4ExplicitEuler.hh"
// #include "G4ImplicitEuler.hh"
// #include "G4SimpleRunge.hh"
// #include "G4SimpleHeum.hh"
#include <G4ClassicalRK4.hh>
// #include "G4HelixExplicitEuler.hh"
// #include "G4HelixImplicitEuler.hh"
// #include "G4HelixSimpleRunge.hh"
// #include "G4CashKarpRKF45.hh"
// #include "G4RKG3_Stepper.hh"

#include <G4SystemOfUnits.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

G4MonopoleFieldSetup* G4MonopoleFieldSetup::fMonopoleFieldSetup = 0;

G4MonopoleFieldSetup::G4MonopoleFieldSetup()
  : fFieldManager(0),
    fChordFinder(0),
    fEquation(0),
    fMonopoleEquation(0),
    fMagneticField(0),
    fStepper(0),
    fMonopoleStepper(0),
    fMinStep(0.0),
    fMonopoleFieldMessenger(0)
{
  fMonopoleFieldMessenger = new G4MonopoleFieldMessenger(this);
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
  delete fMonopoleFieldMessenger;
  if (fMagneticField) delete fMagneticField;
  if (fChordFinder)   delete fChordFinder;
  if (fStepper)       delete fStepper;
  if (fMonopoleStepper)  delete fMonopoleStepper;
}

void G4MonopoleFieldSetup::SetMagField(G4double fieldValue)
{
  //apply a global uniform magnetic field along Z axis
  if (fMagneticField) { delete fMagneticField; }  //delete the existing magn field

  if (fieldValue != 0.) {   // create a new one if non nul
    fMagneticField = new G4UniformMagField(G4ThreeVector(0., 0., fieldValue));
    InitialiseAll();
  } else {
    fMagneticField = 0;
    fFieldManager->SetDetectorField(fMagneticField);
  }
}

void
G4MonopoleFieldSetup::InitialiseAll()
{
  fFieldManager = G4TransportationManager::GetTransportationManager()
                  ->GetFieldManager();

  fEquation = new G4Mag_UsualEqRhs(fMagneticField);
  fMonopoleEquation = new G4MonopoleEquation(fMagneticField);

  fMinStep     = 0.01 * mm ; // minimal step of 1 mm is default

  fMonopoleStepper = new G4ClassicalRK4(fMonopoleEquation, 8);   // for time information..
  fStepper = new G4ClassicalRK4(fEquation);

  SetStepperAndChordFinder(0);
}

void G4MonopoleFieldSetup::SetStepperAndChordFinder(G4int val)
{
  if (fMagneticField) {
    fFieldManager->SetDetectorField(fMagneticField);

    if (fChordFinder) delete fChordFinder;

    switch (val) {
      case 0:
        fChordFinder = new G4ChordFinder(fMagneticField, fMinStep, fStepper);
        break;
      case 1:
        fChordFinder = new G4ChordFinder(fMagneticField, fMinStep, fMonopoleStepper);
        break;
    }

    fFieldManager->SetChordFinder(fChordFinder);
  }
}

G4FieldManager*  G4MonopoleFieldSetup::GetGlobalFieldManager()
{
  return G4TransportationManager::GetTransportationManager()
         ->GetFieldManager();
}
