
#include <framework/gearbox/Unit.h>

#include "simulation/simkernel/B4PhysicsList.h"

#include "G4OpticalPhysics.hh"
#include "G4PhysListFactory.hh"
#include "G4UnitsTable.hh"

using namespace Belle2;

B4PhysicsList::B4PhysicsList(G4String physicsList, G4bool opt) : G4VModularPhysicsList()
{

  defaultCutValue  = 0.7 * Unit::mm;

  //  First the physics form Geant4 predefined PhysicsList (specified by physicsList) is registered.

  G4PhysListFactory factory;
  G4VModularPhysicsList* phys = 0;
  if (factory.IsReferencePhysList(physicsList))  phys = factory.GetReferencePhysList(physicsList);
  // Physics List is defined via environment variable PHYSLIST
  if (!phys) phys = factory.ReferencePhysList();

  for (G4int i = 0; ; ++i) {
    G4VPhysicsConstructor* elem = const_cast<G4VPhysicsConstructor*>(phys->GetPhysics(i));
    if (elem == NULL) break;
    G4cout << "RegisterPhysics: " << elem->GetPhysicsName() << G4endl;
    RegisterPhysics(elem);
  }

  // If choosen also the optical physics is registered

  if (opt) RegisterPhysics(new G4OpticalPhysics());
}

B4PhysicsList::~B4PhysicsList()
{
}

void B4PhysicsList::SetCuts()
{
  if (verboseLevel > 0) {
    G4cout << "B4PhysicsList::SetCuts:";
    G4cout << "CutLength : " << G4BestUnit(defaultCutValue, "Length")
    << G4endl;
  }

  // Set cuts to the defaultCutValue.

  SetCutsWithDefault();
  if (verboseLevel > 0) DumpCutValuesTable();
}


