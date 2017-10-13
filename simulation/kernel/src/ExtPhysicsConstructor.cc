/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/ExtPhysicsConstructor.h>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <G4ParticleTable.hh>
#include <G4ParticleDefinition.hh>
#include <G4Ions.hh>
#include <G4ProcessManager.hh>
#include <G4ComptonScattering.hh>
#include <G4GammaConversion.hh>
#include <G4PhotoElectricEffect.hh>
#include <simulation/kernel/ExtStepLengthLimitProcess.h>
#include <simulation/kernel/ExtMagFieldLimitProcess.h>
#include <simulation/kernel/ExtEnergyLoss.h>
#include <simulation/kernel/ExtMessenger.h>

#include <framework/logging/Logger.h>

#define g4ePDGcode 0

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtPhysicsConstructor::ExtPhysicsConstructor() : G4VPhysicsConstructor("ExtPhysicsConstructor"),
  m_StepLengthLimitProcess(NULL), m_MagFieldLimitProcess(NULL), m_ELossProcess(NULL), m_Messenger(NULL)
{
  if (false) {
    ConstructParticle();
    ConstructProcess();
  }
}

ExtPhysicsConstructor::~ExtPhysicsConstructor()
{
  if (m_StepLengthLimitProcess) delete m_StepLengthLimitProcess;
  if (m_MagFieldLimitProcess) delete m_MagFieldLimitProcess;
  if (m_ELossProcess) delete m_ELossProcess;
  if (m_Messenger) delete m_Messenger;
}

void ExtPhysicsConstructor::ConstructParticle()
{
  // Create ext-specific copies of useful particles, in case we are
  // running the geant4e extrapolator.  These particles will have a
  // limited process list and can coexist with geant4 particles: they
  // are distinguished by their name having the prefix "g4e_" and their
  // "PDG code" always being the reserved value of 0 (which means that
  // the user must use FindParticle(name) rather than FindParticle(#)
  // to look up these special particles.
  // Arguments for G4ParticleDefinition() are as follows:
  //               name             mass          width         charge
  //             2*spin           parity  C-conjugation
  //          2*Isospin       2*Isospin3       G-parity
  //               type    lepton number  baryon number   PDG encoding
  //             stable         lifetime    decay table
  //             shortlived      subType    anti_encoding
  // Values copied verbatim from geant4, except our particles don't decay
  // and the names/PDGcodes are unique.

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

void ExtPhysicsConstructor::ConstructProcess()
{
  // Define the limited set of processes that will be suffered by the
  // geant4e-specific particles
  m_StepLengthLimitProcess = new ExtStepLengthLimitProcess;
  m_MagFieldLimitProcess = new ExtMagFieldLimitProcess;
  m_ELossProcess = new ExtEnergyLoss;
  m_Messenger = new ExtMessenger(m_StepLengthLimitProcess, m_MagFieldLimitProcess, m_ELossProcess);
  G4ParticleTable* myParticleTable = G4ParticleTable::GetParticleTable();
  // theParticleIterator is a Geant4 macro since version 10.
  G4ParticleTable::G4PTblDicIterator* myParticleIterator = myParticleTable->GetIterator();
  myParticleIterator->reset();
  while ((*myParticleIterator)()) {
    G4ParticleDefinition* particle = myParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
    if (particleName.compare(0, 4, "g4e_") == 0) {
      if (particleName == "g4e_gamma") {
        pmanager->AddDiscreteProcess(new G4GammaConversion());
        pmanager->AddDiscreteProcess(new G4ComptonScattering());
        pmanager->AddDiscreteProcess(new G4PhotoElectricEffect());
      } else {
        pmanager->AddContinuousProcess(m_ELossProcess, 1);
        pmanager->AddDiscreteProcess(m_StepLengthLimitProcess, 2);
        pmanager->AddDiscreteProcess(m_MagFieldLimitProcess, 3);
      }
    }
  }
}

