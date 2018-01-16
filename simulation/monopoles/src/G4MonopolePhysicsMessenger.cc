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

#include <simulation/monopoles/G4MonopolePhysicsMessenger.h>
#include <simulation/monopoles/G4MonopolePhysics.h>

#include <G4UIdirectory.hh>
#include <G4UIcommand.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4UIcmdWithADoubleAndUnit.hh>
#include <sstream>

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;

G4MonopolePhysicsMessenger::G4MonopolePhysicsMessenger(G4MonopolePhysics* p)
  : G4UImessenger(),
    fPhys(p),
    fPhysicsDir(0),
    fPhysicsCmd(0),
    fMCmd(0),
    fZCmd(0),
    fMassCmd(0)
{
  fPhysicsDir = new G4UIdirectory("/monopole/");
  fPhysicsDir->SetGuidance("monopole properties");

  fPhysicsCmd = new G4UIcommand("/monopole/setup", this);
  fPhysicsCmd->SetGuidance("Setup monopole");

  G4UIparameter* qmag = new G4UIparameter("qmag", 'd', false);
  qmag->SetGuidance("Magnetic charge");
  qmag->SetDefaultValue("1");
  fPhysicsCmd->SetParameter(qmag);

  G4UIparameter* q = new G4UIparameter("qelec", 'd', false);
  q->SetGuidance("Electric charge charge");
  q->SetDefaultValue("0");
  fPhysicsCmd->SetParameter(q);

  G4UIparameter* mass = new G4UIparameter("mass", 'd', false);
  mass->SetGuidance("mass");
  mass->SetParameterRange("mass>0.");
  qmag->SetDefaultValue("4.5");
  fPhysicsCmd->SetParameter(mass);

  G4UIparameter* unit = new G4UIparameter("unit", 's', false);
  fPhysicsCmd->SetParameter(unit);
  qmag->SetDefaultValue("GeV");
  fPhysicsCmd->AvailableForStates(G4State_PreInit);

  fMCmd = new G4UIcmdWithADouble("/monopole/magCharge", this);
  fMCmd->SetGuidance("Set monopole magnetic charge number");
  fMCmd->SetParameterName("Qmag", false);
  fMCmd->AvailableForStates(G4State_PreInit);

  fZCmd = new G4UIcmdWithADouble("/monopole/elCharge", this);
  fZCmd->SetGuidance("Set monopole electric charge number");
  fZCmd->SetParameterName("Qel", false);
  fZCmd->AvailableForStates(G4State_PreInit);

  fMassCmd = new G4UIcmdWithADoubleAndUnit("/monopole/Mass", this);
  fMassCmd->SetGuidance("Set monopole fMass");
  fMassCmd->SetParameterName("Mass", false);
  fMassCmd->SetRange("Mass>0.");
  fMassCmd->SetUnitCategory("Energy");
  fMassCmd->AvailableForStates(G4State_PreInit);
}

G4MonopolePhysicsMessenger::~G4MonopolePhysicsMessenger()
{
  delete fPhysicsCmd;
  delete fMCmd;
  delete fZCmd;
  delete fMassCmd;
  delete fPhysicsDir;
}


void G4MonopolePhysicsMessenger::SetNewValue(G4UIcommand* command,
                                             G4String newValue)
{
  if (command == fPhysicsCmd) {
    G4double q, m; G4double mass;
    G4String unts;
    std::istringstream is(newValue);
    is >> m >> q >> mass >> unts;
    G4String unit = unts;
    G4double vUnit = G4UIcommand::ValueOf(unit);
    fPhys->SetMagneticCharge(m);
    fPhys->SetElectricCharge(q);
    fPhys->SetMonopoleMass(mass * vUnit);
  }
  if (command == fMCmd) {
    fPhys->SetMagneticCharge(fMCmd->GetNewDoubleValue(newValue));
  }
  if (command == fZCmd) {
    fPhys->SetElectricCharge(fZCmd->GetNewDoubleValue(newValue));
  }
  if (command == fMassCmd) {
    fPhys->SetMonopoleMass(fMassCmd->GetNewDoubleValue(newValue));
  }
}
