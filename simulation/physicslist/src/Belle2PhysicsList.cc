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


Belle2PhysicsList::Belle2PhysicsList()
  : G4VModularPhysicsList()
{
  G4int verb = 1;
  SetVerboseLevel(verb);

  // EM physics
  RegisterPhysics(new G4EmStandardPhysics());
//  G4EmParameters* param = G4EmParameters::Instance();
//  param->SetAugerCascade(true);

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

  // Optical physics
  RegisterPhysics(new G4OpticalPhysics());
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
  SetCutValue(0.7 * mm, "proton");
  SetCutValue(0.7 * mm, "e-");
  SetCutValue(0.7 * mm, "e+");
  SetCutValue(0.7 * mm, "gamma");
}

