/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/ExtMessenger.h>
#include <simulation/kernel/ExtStepLengthLimitProcess.h>
#include <simulation/kernel/ExtMagFieldLimitProcess.h>
#include <simulation/kernel/ExtEnergyLoss.h>
#include <globals.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADoubleAndUnit.hh>
#include <G4UIcmdWithADouble.hh>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtMessenger::ExtMessenger(ExtStepLengthLimitProcess* lAct, ExtMagFieldLimitProcess* mAct, ExtEnergyLoss* eAct) :
  m_stepLengthAction(lAct),
  m_magFieldAction(mAct),
  m_energyLossAction(eAct)
{

  m_dir = new G4UIdirectory("/geant4e/");
  m_dir->SetGuidance("GEANT4e control commands");
  m_dirLimits = new G4UIdirectory("/geant4e/limits/");
  m_dirLimits->SetGuidance("GEANT4e commands to limit the step");

  m_stepLengthLimitCmd = new G4UIcmdWithADoubleAndUnit("/geant4e/limits/stepLength", this);
  m_stepLengthLimitCmd->SetGuidance("Limit the length of a step (distance)");
  m_stepLengthLimitCmd->SetDefaultUnit("mm");
  m_stepLengthLimitCmd->AvailableForStates(G4State_PreInit, G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  m_magFieldLimitCmd = new G4UIcmdWithADouble("/geant4e/limits/magField", this);
  m_magFieldLimitCmd->SetGuidance("Limit the length of a step (field/momentum)");
  m_magFieldLimitCmd->AvailableForStates(G4State_PreInit, G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  m_energyLossCmd = new G4UIcmdWithADouble("/geant4e/limits/energyLoss", this);
  m_energyLossCmd->SetGuidance("Limit the length of a step (fractional KE loss)");
  m_energyLossCmd->AvailableForStates(G4State_PreInit, G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  if (false) {
    G4UIcommand* command = nullptr;
    SetNewValue(command, G4String(""));
  }

}

ExtMessenger::~ExtMessenger()
{
  delete m_stepLengthLimitCmd;
  delete m_magFieldLimitCmd;
  delete m_energyLossCmd;
  delete m_dir;
  delete m_dirLimits;
}

void ExtMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if (command == m_stepLengthLimitCmd) {
    B2DEBUG(200, "ExtMessenger::SetNewValue() StepLengthAction SetStepLimit "
            << m_stepLengthLimitCmd->GetNewDoubleValue(newValue));
    m_stepLengthAction->SetStepLimit(m_stepLengthLimitCmd->GetNewDoubleValue(newValue));
  } else if (command == m_magFieldLimitCmd) {
    B2DEBUG(200, "ExtMessenger::SetNewValue() MagFieldAction SetStepLimit "
            << m_magFieldLimitCmd->GetNewDoubleValue(newValue));
    m_magFieldAction->SetStepLimit(m_magFieldLimitCmd->GetNewDoubleValue(newValue));
  } else if (command == m_energyLossCmd) {
    B2DEBUG(200, "ExtMessenger::SetNewValue() EnergyLossAction SetStepLimit "
            << m_energyLossCmd->GetNewDoubleValue(newValue));
    m_energyLossAction->SetStepLimit(m_energyLossCmd->GetNewDoubleValue(newValue));
  }
}
