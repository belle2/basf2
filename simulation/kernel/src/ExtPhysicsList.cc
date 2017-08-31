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

#include <G4UnitsTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4Gamma.hh>
#include <G4Electron.hh>
#include <G4Positron.hh>
#include <G4MuonMinus.hh>
#include <G4MuonPlus.hh>
#include <G4PionPlus.hh>
#include <G4PionMinus.hh>
#include <G4KaonPlus.hh>
#include <G4KaonMinus.hh>
#include <G4Proton.hh>
#include <G4AntiProton.hh>
#include <G4Deuteron.hh>
#include <G4AntiDeuteron.hh>
#include <G4GenericIon.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtPhysicsList::ExtPhysicsList() : G4VModularPhysicsList()
{
  defaultCutValue  = 0.7 * CLHEP::mm; //Default production cut value. Unit given in Geant4 units.
  //defaultCutValue = 1.0E+9 * CLHEP::cm; // set big step so that AlongStep computes all the energy

  // Instead of calling G4EmStandardPhysics to create process tables for standard particles that
  // will never be used, we just define the standard particles that are in Const::ChargedStable but
  // don't assign any processes to them.  Note that G4PhysicsListHelper::CheckParticleList requires
  // G4Gamma and G4GenericIon.  G4EmStandardPhysics() defines Gamma, Electron, Positron, MuonPlus,
  // MuonMinus, PionPlus, PionMinus, KaonPlus, KaonMinus, Proton, AntiProton, Deuteron, Triton, He3,
  // Alpha, and GenericIon -- but not AntiDeuteron.
  //- G4AntiDeuteron::AntiDeuteron();
  //- RegisterPhysics(new G4EmStandardPhysics);
  G4Gamma::Gamma();
  G4Electron::Electron();
  G4Positron::Positron();
  G4MuonMinus::MuonMinus();
  G4MuonPlus::MuonPlus();
  G4PionPlus::PionPlus();
  G4PionMinus::PionMinus();
  G4KaonPlus::KaonPlus();
  G4KaonMinus::KaonMinus();
  G4Proton::Proton();
  G4AntiProton::AntiProton();
  G4Deuteron::Deuteron();
  G4AntiDeuteron::AntiDeuteron();
  G4GenericIon::GenericIon();
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
  G4ParticleTable* myParticleTable = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator* myParticleIterator = myParticleTable->GetIterator();
  myParticleIterator->reset();
  while ((*myParticleIterator)()) {
    G4ParticleDefinition* particle = myParticleIterator->value();
    if (particle->GetParticleName().substr(0, 4) == "g4e_") {
      SetParticleCuts(1.0E+9 * CLHEP::cm, particle);
    }
  }

}
