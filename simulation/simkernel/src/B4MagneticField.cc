/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/simkernel/B4MagneticField.h>

#include "G4TransportationManager.hh"
#include "G4ios.hh"
#include "G4PropagatorInField.hh"

#include "G4ExplicitEuler.hh"
#include "G4ImplicitEuler.hh"
#include "G4SimpleRunge.hh"
#include "G4SimpleHeum.hh"
#include "G4ClassicalRK4.hh"
#include "G4HelixExplicitEuler.hh"
#include "G4HelixImplicitEuler.hh"
#include "G4HelixSimpleRunge.hh"
#include "G4CashKarpRKF45.hh"
#include "G4RKG3_Stepper.hh"

#include <cstdlib>   // for strtol
#include <fstream>    // for file streams
#include <cmath>

using namespace Belle2;

B4MagneticField::B4MagneticField(): m_chordFinder(0), m_equation(0), m_stepper(0)
{
  //----------------------
  // Set default values.
  //----------------------
  m_stepperType       = 4;
  m_minStep           = 0.01 * mm;
  m_deltaOneStep      = 1.0e-2 * mm;
  m_deltaIntersection = 1.0e-3 * mm;
  m_minEpsilonStep    = 5.0e-5;
  m_maxEpsilonStep    = 1.0e-3;

  G4FieldManager* global_field_mgr
  = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  global_field_mgr->SetDetectorField(this);
  initialiseAll();
}


B4MagneticField :: ~B4MagneticField()
{
  if (m_fieldMessenger) delete m_fieldMessenger;
  if (m_chordFinder)    delete m_chordFinder;
  if (m_equation)       delete m_equation;
  if (m_stepper)        delete m_stepper;
}


void B4MagneticField :: GetFieldValue(const G4double Point[3], G4double *Bfield) const
{
  const G4double x = Point[0];
  const G4double y = Point[1];
  const G4double z = Point[2];
  const G4double r = sqrt(x * x + y * y);
  Bfield[0] = 0;
  Bfield[1] = 0;
  Bfield[2] = 0;

  if (r < 3.2*m && fabs(z) < 5*m) {
    Bfield[2] = 1.5 * tesla;
  }
}


void B4MagneticField::initialiseAll()
{
  m_fieldMessenger = new B4MagneticFieldMessenger(this);

  m_equation = new G4Mag_UsualEqRhs(this);

  m_fieldManager = G4TransportationManager::GetTransportationManager()
                   ->GetFieldManager();

  createStepperAndChordFinder();
}


void B4MagneticField::createStepperAndChordFinder()
{
  setStepper();

  m_fieldManager->SetDetectorField(this);

  if (m_chordFinder) delete m_chordFinder;

  m_chordFinder = new G4ChordFinder(this , m_minStep, m_stepper);

  m_chordFinder->SetDeltaChord(0.25*mm);
  m_fieldManager->SetChordFinder(m_chordFinder);
  m_fieldManager->SetDeltaOneStep(m_deltaOneStep);
  m_fieldManager->SetDeltaIntersection(m_deltaIntersection);
  m_fieldManager->SetMinimumEpsilonStep(m_minEpsilonStep);
  m_fieldManager->SetMaximumEpsilonStep(m_maxEpsilonStep);

  return;
}


//--------------------------------------------
// Set stepper according to the stepper type
//--------------------------------------------
void B4MagneticField::setStepper()
{
  if (m_stepper) delete m_stepper;

  switch (m_stepperType) {
    case 0:
      m_stepper = new G4ExplicitEuler(m_equation);
      G4cout << "G4ExplicitEuler is called" << G4endl;
      break;
    case 1:
      m_stepper = new G4ImplicitEuler(m_equation);
      G4cout << "G4ImplicitEuler is called" << G4endl;
      break;
    case 2:
      m_stepper = new G4SimpleRunge(m_equation);
      G4cout << "G4SimpleRunge is called" << G4endl;
      break;
    case 3:
      m_stepper = new G4SimpleHeum(m_equation);
      G4cout << "G4SimpleHeum is called" << G4endl;
      break;
    case 4:
      m_stepper = new G4ClassicalRK4(m_equation);
      G4cout << "G4ClassicalRK4 (default) is called" << G4endl;
      break;
    case 5:
      m_stepper = new G4HelixExplicitEuler(m_equation);
      G4cout << "G4HelixExplicitEuler is called" << G4endl;
      break;
    case 6:
      m_stepper = new G4HelixImplicitEuler(m_equation);
      G4cout << "G4HelixImplicitEuler is called" << G4endl;
      break;
    case 7:
      m_stepper = new G4HelixSimpleRunge(m_equation);
      G4cout << "G4HelixSimpleRunge is called" << G4endl;
      break;
    case 8:
      m_stepper = new G4CashKarpRKF45(m_equation);
      G4cout << "G4CashKarpRKF45 is called" << G4endl;
      break;
    case 9:
      m_stepper = new G4RKG3_Stepper(m_equation);
      G4cout << "G4RKG3_Stepper is called" << G4endl;
      break;
    default: m_stepper = 0;
  }
  return;
}
