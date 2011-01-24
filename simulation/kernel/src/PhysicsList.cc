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

#include <G4UnitsTable.hh>
#include <G4OpticalPhysics.hh>
#include <G4PhysListFactory.hh>

using namespace std;
using namespace Belle2;
using namespace Simulation;


PhysicsList::PhysicsList(const string& physicsListName)
{
  defaultCutValue  = 0.7 * mm; //Default production cut value. Unit given in Geant4 units.

  //First register the physics form Geant4 predefined PhysicsList.
  G4PhysListFactory physListFactory;
  G4VModularPhysicsList* physList = NULL;
  if (physListFactory.IsReferencePhysList(physicsListName)) physList = physListFactory.GetReferencePhysList(physicsListName);

  if (physList == NULL) B2FATAL("Could not load the physics list " << physicsListName)

    G4VPhysicsConstructor* regPhys = const_cast<G4VPhysicsConstructor*>(physList->GetPhysics(0));
  int iPhysList = 1;
  while (regPhys != NULL) {
    B2DEBUG(10, "RegisterPhysics: " << regPhys->GetPhysicsName())
    RegisterPhysics(regPhys);
    iPhysList++;
    regPhys = const_cast<G4VPhysicsConstructor*>(physList->GetPhysics(iPhysList));
  }
}


PhysicsList::~PhysicsList()
{

}


void PhysicsList::SetCuts()
{
  B2INFO("B4PhysicsList::SetCuts, CutLength: " << G4BestUnit(defaultCutValue, "Length"))

  // Set cuts to the defaultCutValue.
  SetCutsWithDefault();

  if (LogSystem::Instance().getCurrentLogLevel() == LogConfig::c_Debug) DumpCutValuesTable();
}


void PhysicsList::setProductionCutValue(double productionCut)
{
  defaultCutValue = (productionCut / Unit::mm) * mm;
}


void PhysicsList::registerOpticalPhysicsList()
{
  RegisterPhysics(new G4OpticalPhysics());
}
