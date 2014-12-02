/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <globals.hh>

#include <simulation/kernel/ExtPhysicsList.h>
#include <simulation/kernel/ExtPhysicsConstructor.h>
#include <framework/gearbox/Unit.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <G4EmStandardPhysics.hh>
#include <G4UnitsTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtPhysicsList::ExtPhysicsList() : G4VModularPhysicsList()
{
  defaultCutValue  = 0.7 * CLHEP::mm; //Default production cut value. Unit given in Geant4 units.
  //defaultCutValue = 1.0E+9 * CLHEP::cm; // set big step so that AlongStep computes all the energy
  RegisterPhysics(new G4EmStandardPhysics);
  RegisterPhysics(new Simulation::ExtPhysicsConstructor);
  if (false) SetCuts();
}

ExtPhysicsList::~ExtPhysicsList()
{
}

void ExtPhysicsList::SetCuts()
{
  // Set cuts to the defaultCutValue.
  SetCutsWithDefault();
  // LEP: For geant4e-specific particles, set a big step so that AlongStep computes
  // all the energy (as is done in G4ErrorPhysicsList)
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator* myParticleIterator = theParticleTable->GetIterator();
  myParticleIterator->reset();
  while ((*myParticleIterator)()) {
    G4ParticleDefinition* particle = myParticleIterator->value();
    if (particle->GetParticleName().substr(0, 4) == "g4e_") {
      SetParticleCuts(1.0E+9 * CLHEP::cm, particle);
    }
  }

}
