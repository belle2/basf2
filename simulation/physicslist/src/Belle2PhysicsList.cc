/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/physicslist/Belle2PhysicsList.h>
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

// EM and decay physics
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4OpticalPhysics.hh"
#include "G4EmParameters.hh"
#include "G4DecayPhysics.hh"

// Hadronic physics
#include <simulation/physicslist/ProtonPhysics.h>
#include <simulation/physicslist/NeutronPhysics.h>
#include <simulation/physicslist/PionPhysics.h>
#include <simulation/physicslist/KaonPhysics.h>
#include <simulation/physicslist/HyperonPhysics.h>
#include <simulation/physicslist/AntiBaryonPhysics.h>
#include <simulation/physicslist/IonPhysics.h>
#include <simulation/physicslist/GammaLeptoNuclearPhysics.h>

// Particles
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

using namespace Belle2;
using namespace Simulation;


Belle2PhysicsList::Belle2PhysicsList(const G4String& physicsListName)
  : G4VModularPhysicsList(), m_globalCutValue(0.07)
{
  G4cout << " Using " << physicsListName << " physics list " << G4endl;

  // Decay
  RegisterPhysics(new G4DecayPhysics());

  // Hadronic physics
  RegisterPhysics(new ProtonPhysics());
  RegisterPhysics(new NeutronPhysics());
  RegisterPhysics(new PionPhysics());
  RegisterPhysics(new KaonPhysics());
  RegisterPhysics(new HyperonPhysics());
  RegisterPhysics(new AntiBaryonPhysics());
  RegisterPhysics(new IonPhysics());
  RegisterPhysics(new GammaLeptoNuclearPhysics());
}


Belle2PhysicsList::~Belle2PhysicsList()
{}


void Belle2PhysicsList::ConstructParticle()
{
  G4BosonConstructor pBosonConstructor;
  pBosonConstructor.ConstructParticle();

  G4LeptonConstructor pLeptonConstructor;
  pLeptonConstructor.ConstructParticle();

  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4IonConstructor pIonConstructor;
  pIonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();
}


void Belle2PhysicsList::SetCuts()
{
  // Belle2 assumes input units are cm
  SetCutValue(m_globalCutValue * cm, "proton");
  SetCutValue(m_globalCutValue * cm, "e-");
  SetCutValue(m_globalCutValue * cm, "e+");
  SetCutValue(m_globalCutValue * cm, "gamma");
}


void Belle2PhysicsList::SetVerbosity(G4int verb)
{
  SetVerboseLevel(verb);
}


void Belle2PhysicsList::SetProductionCutValue(G4double value)
{
  m_globalCutValue = value;
}


void Belle2PhysicsList::UseStandardEMPhysics(G4bool yesno)
{
  if (yesno) {
    RegisterPhysics(new G4EmStandardPhysics());
  } else {
    RegisterPhysics(new G4EmStandardPhysics_option1());
  }
}


void Belle2PhysicsList::UseOpticalPhysics(G4bool yesno)
{
  if (yesno) RegisterPhysics(new G4OpticalPhysics());
}


void Belle2PhysicsList::UseHighPrecisionNeutrons(G4bool yesno)
{
  if (yesno) G4cout << " High precision neutron option not yet ready " << G4endl;
}


