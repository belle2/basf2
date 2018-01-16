/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopoleFieldSetup.h>
#include <simulation/monopoles/G4MonopoleFieldMessenger.h>

#include <G4UIdirectory.hh>
#include <G4UIcmdWithADoubleAndUnit.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

G4MonopoleFieldMessenger::G4MonopoleFieldMessenger(G4MonopoleFieldSetup* fld)
  : G4UImessenger(),
    fField(fld),
    fFieldDir(0),
    fSetFieldCmd(0)

{
  fFieldDir = new G4UIdirectory("/testex/fld/");
  fFieldDir->SetGuidance("field setup commands");

  fSetFieldCmd = new G4UIcmdWithADoubleAndUnit("/testex/fld/setField", this);
  fSetFieldCmd->SetGuidance("Define magnetic field.");
  fSetFieldCmd->SetGuidance("Magnetic field will be in Z direction.");
  fSetFieldCmd->SetParameterName("Bz", false);
  fSetFieldCmd->SetUnitCategory("Magnetic flux density");
  fSetFieldCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

G4MonopoleFieldMessenger::~G4MonopoleFieldMessenger()
{
  delete fSetFieldCmd;
  delete fFieldDir;
}

void G4MonopoleFieldMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
  if (command == fSetFieldCmd)
  { fField->SetMagField(fSetFieldCmd->GetNewDoubleValue(newValue));}
}
