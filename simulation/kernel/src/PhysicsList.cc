/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Andreas Moll                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/PhysicsList.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <G4UnitsTable.hh>
#include <G4OpticalPhysics.hh>
#include <G4PhysListFactory.hh>
// LEP: includes for geant4e track extrapolation
#include <simulation/kernel/ExtPhysicsConstructor.h>
#include <G4ParticleTable.hh>
#include <G4ParticleDefinition.hh>

using namespace std;
using namespace Belle2;
using namespace Simulation;


PhysicsList::PhysicsList(const string& physicsListName) : G4VModularPhysicsList()
{
  defaultCutValue  = 0.7 * CLHEP::mm; //Default production cut value. Unit given in Geant4 units.

  //First register the physics form Geant4 predefined PhysicsList.
  G4PhysListFactory physListFactory;
  G4VModularPhysicsList* physList = NULL;

  if (physListFactory.IsReferencePhysList(physicsListName)) physList = physListFactory.GetReferencePhysList(physicsListName);
  if (physList == NULL) B2FATAL("Could not load the physics list " << physicsListName);

  int iPhysList = 1;
  G4VPhysicsConstructor* regPhys = const_cast<G4VPhysicsConstructor*>(physList->GetPhysics(0));
  while (regPhys != NULL) {
    B2DEBUG(10, "RegisterPhysics: " << regPhys->GetPhysicsName());
    RegisterPhysics(regPhys);
    regPhys = const_cast<G4VPhysicsConstructor*>(physList->GetPhysics(iPhysList++));
  }
  // LEP: Append the geant4e-specific physics constructor to the list
  RegisterPhysics(new ExtPhysicsConstructor);
}


PhysicsList::~PhysicsList()
{

}


void PhysicsList::SetCuts()
{
  B2INFO("B4PhysicsList::SetCuts, CutLength: " << G4BestUnit(defaultCutValue, "Length"));

  // Set cuts to the defaultCutValue.
  SetCutsWithDefault();
  // LEP: For geant4e-specific particles, set a big step so that AlongStep computes
  // all the energy (as is done in G4ErrorPhysicsList)
  G4ParticleTable* myParticleTable = G4ParticleTable::GetParticleTable();
  // theParticleIterator is a Geant4 macro since version 10.
  G4ParticleTable::G4PTblDicIterator* myParticleIterator = myParticleTable->GetIterator();
  myParticleIterator->reset();
  while ((*myParticleIterator)()) {
    G4ParticleDefinition* particle = myParticleIterator->value();
    if (particle->GetParticleName().substr(0, 4) == "g4e_") {
      SetParticleCuts(1.0E+9 * CLHEP::cm, particle);
    }
  }

  if (LogSystem::Instance().getCurrentLogLevel() == LogConfig::c_Debug) DumpCutValuesTable();
}


void PhysicsList::setProductionCutValue(double productionCut)
{
  defaultCutValue = (productionCut / Unit::mm) * CLHEP::mm;
}


void PhysicsList::registerOpticalPhysicsList()
{
  RegisterPhysics(new G4OpticalPhysics());
}

