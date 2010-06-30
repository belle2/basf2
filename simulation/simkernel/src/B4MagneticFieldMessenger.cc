/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simkernel/B4MagneticFieldMessenger.h>
#include <simkernel/B4MagneticField.h>

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"

using namespace Belle2;

B4MagneticFieldMessenger::B4MagneticFieldMessenger(B4MagneticField* fieldSetup) : m_fieldSetup(fieldSetup)
{
  //-----------------------------------
  // Create a directory for field.
  //-----------------------------------
  m_fieldDir = new G4UIdirectory("/field/");
  m_fieldDir->SetGuidance("Belle2 magnetic field parameters used during simulation");

  //-----------------------------------
  // Create a command for field.
  //-----------------------------------
  m_fieldCmd = new G4UIcommand("/field/fieldParameter", this);
  m_fieldCmd->SetGuidance("The Belle2 field parameters");
  m_fieldCmd->SetGuidance("StepperType MinStep DeltaOneStep DeltaIntersection MinEpsilonStep MaxEpsilonStep");

  //------------------------------------------
  // Define a empty pointer of G4UIparameter.
  //------------------------------------------
  G4UIparameter* parameter = NULL;

  //----------------------------------------------------------------------------
  // Define and set parameters, please see guidance for each parameter meaning.
  //----------------------------------------------------------------------------
  G4bool omittable;
  parameter = new G4UIparameter("stepperType", 'i', omittable = true);
  parameter->SetGuidance("Select stepper type for magnetic field");
  parameter->SetParameterRange("stepperType >= 0 && stepperType <= 9");
  parameter->SetDefaultValue(4);
  m_fieldCmd->SetParameter(parameter);

  parameter = new G4UIparameter("minStep", 'd', omittable = true);
  parameter->SetGuidance("Set value for minimal step [mm]");
  parameter->SetParameterRange("minStep >= 0 && minStep <= 1");
  parameter->SetDefaultValue(0.01);
  m_fieldCmd->SetParameter(parameter);

  parameter = new G4UIparameter("deltaOneStep", 'd', omittable = true);
  parameter->SetGuidance("Set value for delta one step [mm]");
  parameter->SetParameterRange("deltaOneStep >= 0");
  parameter->SetDefaultValue(0.01);
  m_fieldCmd->SetParameter(parameter);

  parameter = new G4UIparameter("deltaIntersection", 'd', omittable = true);
  parameter->SetGuidance("Set value for delta intersection [mm]");
  parameter->SetParameterRange("deltaIntersection >= 0");
  parameter->SetDefaultValue(0.001);
  m_fieldCmd->SetParameter(parameter);

  parameter = new G4UIparameter("minEpsilonStep", 'd', omittable = true);
  parameter->SetGuidance("Set value for miminal epsilon step [mm]");
  parameter->SetParameterRange("minEpsilonStep >= 0");
  parameter->SetDefaultValue(5.0e-5);
  m_fieldCmd->SetParameter(parameter);

  parameter = new G4UIparameter("maxEpsilonStep", 'd', omittable = true);
  parameter->SetGuidance("Set value for maximum epsilon step [mm]");
  parameter->SetParameterRange("maxEpsilonStep >= 0");
  parameter->SetDefaultValue(1.0e-3);
  m_fieldCmd->SetParameter(parameter);
}

B4MagneticFieldMessenger::~B4MagneticFieldMessenger()
{
  if (m_fieldDir)   delete m_fieldDir;
  if (m_fieldCmd)   delete m_fieldCmd;
}

void B4MagneticFieldMessenger::SetNewValue(G4UIcommand* command, G4String newvalue)
{
  //-------------------------------------------
  // Get parameters from a string -- newvalue.
  //-------------------------------------------
  G4int stepperType;
  G4double minStep, deltaOneStep, deltaIntersection, minEpsilonStep, maxEpsilonStep;
  std::istringstream is((char*)newvalue.data());
  is >> stepperType >> minStep >> deltaOneStep >> deltaIntersection >> minEpsilonStep >> maxEpsilonStep;

  if (command == m_fieldCmd) {
    m_fieldSetup->setStepperType(stepperType);
    m_fieldSetup->setMinStep(minStep);
    m_fieldSetup->setDeltaOneStep(deltaOneStep);
    m_fieldSetup->setDeltaIntersection(deltaIntersection);
    m_fieldSetup->setMinEpsilonStep(minEpsilonStep);
    m_fieldSetup->setMaxEpsilonStep(maxEpsilonStep);
    m_fieldSetup->createStepperAndChordFinder();
  }

}
