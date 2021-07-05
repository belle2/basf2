/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/physicslist/Belle2PhysicsList.h>
#include "G4SystemOfUnits.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"

// EM and decay physics
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4OpticalPhysics.hh"
#include "G4DecayPhysics.hh"
#include <simulation/physicslist/Geant4ePhysics.h>

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

// Long lived
#include "simulation/longlivedneutral/G4LongLivedNeutralPhysics.h"
#include <framework/logging/Logger.h>

#define g4ePDGcode 0

using namespace Belle2;
using namespace Simulation;


Belle2PhysicsList::Belle2PhysicsList(const G4String& physicsListName)
  : G4VModularPhysicsList(), m_globalCutValue(0.07)
{
  B2INFO("Using " << physicsListName << " physics list");

  m_pxdCutValue = m_globalCutValue;
  m_svdCutValue = m_globalCutValue;
  m_cdcCutValue = m_globalCutValue;
  m_arichtopCutValue = m_globalCutValue;
  m_eclCutValue = m_globalCutValue;
  m_klmCutValue = m_globalCutValue;

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
  RegisterPhysics(new Geant4ePhysics());
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

  ConstructG4eParticles();
}


void Belle2PhysicsList::ConstructG4eParticles()
{
  static G4ParticleDefinition* g4eParticle = NULL;

  if (g4eParticle == NULL) {
    // Bohr Magneton for positron and positive muon
    G4double muBpositron = 0.5 * CLHEP::eplus * CLHEP::hbar_Planck / (0.51099906 * CLHEP::MeV / CLHEP::c_squared);
    G4double muBmuon = 0.5 * CLHEP::eplus * CLHEP::hbar_Planck / (0.1056584 * CLHEP::GeV / CLHEP::c_squared);
    G4double muNucleon = CLHEP::eplus * CLHEP::hbar_Planck / 2. / (CLHEP::proton_mass_c2 / CLHEP::c_squared);
    // Copied from G4Gamma.cc
    g4eParticle = new G4ParticleDefinition(
      "g4e_gamma", 0.0 * CLHEP::MeV, 0.0 * CLHEP::MeV, 0.0,
      2,               -1,          -1,
      0,                0,           0,
      "gamma",          0,           0,  g4ePDGcode,
      true,           0.0,        NULL,
      false,     "photon",  g4ePDGcode
    );
    // Copied from G4Electron.cc
    new G4ParticleDefinition(
      "g4e_e-", 0.51099906 * CLHEP::MeV, 0.0 * CLHEP::MeV, -1.0 * CLHEP::eplus,
      1,                0,           0,
      0,                0,           0,
      "lepton",         1,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,          "e",  g4ePDGcode, -1.0011596521859 * muBpositron
    );
    // Copied from G4Positron.cc
    new G4ParticleDefinition(
      "g4e_e+", 0.51099906 * CLHEP::MeV, 0.0 * CLHEP::MeV, +1.0 * CLHEP::eplus,
      1,                0,           0,
      0,                0,           0,
      "lepton",        -1,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,          "e",  g4ePDGcode,  1.0011596521859 * muBpositron
    );
    // Copied from G4MuonPlus.cc
    new G4ParticleDefinition(
      "g4e_mu+", 0.1056584 * CLHEP::GeV, 2.99591e-16 * CLHEP::MeV, +1.0 * CLHEP::eplus,
      1,                0,           0,
      0,                0,           0,
      "lepton",        -1,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,         "mu",  g4ePDGcode,  1.0011659208 * muBmuon
    );
    // Copied from G4MuonMinus.cc
    new G4ParticleDefinition(
      "g4e_mu-", 0.1056584 * CLHEP::GeV, 2.99591e-16 * CLHEP::MeV, -1.0 * CLHEP::eplus,
      1,                0,           0,
      0,                0,           0,
      "lepton",         1,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,         "mu",  g4ePDGcode, -1.0011659208 * muBmuon
    );
    // Copied from G4PionPlus.cc
    new G4ParticleDefinition(
      "g4e_pi+", 0.1395700 * CLHEP::GeV, 2.5284e-14 * CLHEP::MeV, +1.0 * CLHEP::eplus,
      0,               -1,           0,
      2,               +2,          -1,
      "meson",          0,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,         "pi",  g4ePDGcode
    );
    // Copied from G4PionMinus.cc
    new G4ParticleDefinition(
      "g4e_pi-", 0.1395700 * CLHEP::GeV, 2.5284e-14 * CLHEP::MeV, -1.0 * CLHEP::eplus,
      0,               -1,           0,
      2,               -2,          -1,
      "meson",          0,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,         "pi",  g4ePDGcode
    );
    // Copied from G4KaonPlus.cc
    new G4ParticleDefinition(
      "g4e_kaon+", 0.493677 * CLHEP::GeV, 5.315e-14 * CLHEP::MeV, +1.0 * CLHEP::eplus,
      0,               -1,           0,
      1,               +1,           0,
      "meson",          0,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,       "kaon",  g4ePDGcode
    );
    // Copied from G4KaonMinus.cc
    new G4ParticleDefinition(
      "g4e_kaon-", 0.493677 * CLHEP::GeV, 5.315e-14 * CLHEP::MeV, -1.0 * CLHEP::eplus,
      0,               -1,           0,
      1,               -1,           0,
      "meson",          0,           0,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,       "kaon",  g4ePDGcode
    );
    // Copied from G4Proton.cc except use G4ParticleDefinition instead of G4Ions
    new G4ParticleDefinition(
      "g4e_proton", 0.9382723 * CLHEP::GeV, 0.0 * CLHEP::MeV, +1.0 * CLHEP::eplus,
      1,               +1,           0,
      1,               +1,           0,
      "baryon",         0,          +1,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,    "nucleon",  g4ePDGcode,  2.792847351 * muNucleon
    );
    // Copied from G4AntiProton.cc except use G4ParticleDefinition instead of G4Ions
    new G4ParticleDefinition(
      "g4e_anti_proton", 0.9382723 * CLHEP::GeV, 0.0 * CLHEP::MeV, -1.0 * CLHEP::eplus,
      1,               +1,           0,
      1,               -1,           0,
      "baryon",         0,          -1,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,    "nucleon",  g4ePDGcode, -2.792847351 * muNucleon
    );
    // copied from G4Deuteron.hh except use G4ParticleDefinition instead of G4Ions
    new G4ParticleDefinition(
      "g4e_deuteron", 1.875613 * CLHEP::GeV, 0.0 * CLHEP::MeV, +1.0 * CLHEP::eplus,
      2,               +1,           0,
      0,                0,           0,
      "nucleus",        0,          +2,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,     "static",  g4ePDGcode,  0.857438230 *  muNucleon
    );
    // copied from G4AntiDeuteron.hh except use G4ParticleDefinition instead of G4Ions
    new G4ParticleDefinition(
      "g4e_anti_deuteron", 1.875613 * CLHEP::GeV, 0.0 * CLHEP::MeV, -1.0 * CLHEP::eplus,
      2,               +1,           0,
      0,                0,           0,
      "anti_nucleus",   0,          -2,  g4ePDGcode,
      true,          -1.0,        NULL,
      false,     "static",  g4ePDGcode, -0.857438230 * muNucleon
    );
  }
}

void Belle2PhysicsList::setRegionCuts(const std::string& name, const std::vector<std::string>& regions, double cutValue)
{
  G4RegionStore* theRegionStore = G4RegionStore::GetInstance();
  if (cutValue == 0) {
    cutValue = m_globalCutValue;
  } else {
    B2INFO("Set production cut for detector region" << LogVar("detector", name) << LogVar("production_cut", cutValue));
  }
  auto* regionCuts = new G4ProductionCuts;
  regionCuts->SetProductionCut(cutValue * cm);
  bool foundOne{false};
  for (const auto& regionName : regions) {
    auto* region = theRegionStore->GetRegion(regionName, false);
    if (!region) {
      B2WARNING("Cannot find Geant4 region for sub detector. Probably detector not present?"
                << LogVar("detector", name) << LogVar("region", regionName));
      continue;
    }
    region->SetProductionCuts(regionCuts);
    foundOne = true;
  }
  if (!foundOne) delete regionCuts;
}


void Belle2PhysicsList::SetCuts()
{
  // Belle2 assumes input units are cm
  B2INFO("Setting global Geant4 production cut" << LogVar("cutValue", m_globalCutValue));

  SetCutValue(m_globalCutValue * cm, "proton");
  SetCutValue(m_globalCutValue * cm, "e-");
  SetCutValue(m_globalCutValue * cm, "e+");
  SetCutValue(m_globalCutValue * cm, "gamma");

  setRegionCuts("PXD", {"PXDEnvelope"}, m_pxdCutValue);
  setRegionCuts("SVD", {"SVDEnvelope"}, m_svdCutValue);
  setRegionCuts("CDC", {"CDCEnvelope"}, m_cdcCutValue);
  setRegionCuts("ARICH", {"ARICHEnvelope"}, m_arichtopCutValue);
  setRegionCuts("TOP", {"TOPEnvelope"}, m_arichtopCutValue);
  setRegionCuts("ECL", {"ECLForwardEnvelope", "ECLBarrelSector", "ECLBackwardEnvelope"}, m_eclCutValue);
  setRegionCuts("KLM", {"BKLMEnvelope", "EKLMEnvelope"}, m_klmCutValue);
}


void Belle2PhysicsList::SetVerbosity(G4int verb)
{
  SetVerboseLevel(verb);
}


void Belle2PhysicsList::SetProductionCutValue(G4double value)
{
  m_globalCutValue = value;
}


void Belle2PhysicsList::SetPXDProductionCutValue(G4double value)
{
  m_pxdCutValue = value;
}


void Belle2PhysicsList::SetSVDProductionCutValue(G4double value)
{
  m_svdCutValue = value;
}


void Belle2PhysicsList::SetCDCProductionCutValue(G4double value)
{
  m_cdcCutValue = value;
}


void Belle2PhysicsList::SetARICHTOPProductionCutValue(G4double value)
{
  m_arichtopCutValue = value;
}


void Belle2PhysicsList::SetECLProductionCutValue(G4double value)
{
  m_eclCutValue = value;
}


void Belle2PhysicsList::SetKLMProductionCutValue(G4double value)
{
  m_klmCutValue = value;
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

void Belle2PhysicsList::UseLongLivedNeutralParticles()
{
  G4LongLivedNeutralPhysics* pLongLivedNeutral = new G4LongLivedNeutralPhysics();
  RegisterPhysics(pLongLivedNeutral);
  pLongLivedNeutral->ConstructParticle();
}
