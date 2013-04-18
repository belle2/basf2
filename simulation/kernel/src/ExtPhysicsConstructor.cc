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
#include <simulation/kernel/ExtNoHits.h>
#include <simulation/kernel/ExtMessenger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtPhysicsConstructor::ExtPhysicsConstructor() : G4VPhysicsConstructor("ExtPhysicsConstructor")
{
}

ExtPhysicsConstructor::~ExtPhysicsConstructor()
{
}

void ExtPhysicsConstructor::ConstructParticle()
{
  // Create ext-specific copies of useful particles, in case we are
  // running the geant4e extrapolator.  These particles will have a
  // limited process list and can coexist with geant4 particles.
  // Arguments for G4ParticleDefinition() are as follows:
  //               name             mass          width         charge
  //             2*spin           parity  C-conjugation
  //          2*Isospin       2*Isospin3       G-parity
  //               type    lepton number  baryon number   PDG encoding
  //             stable         lifetime    decay table
  //             shortlived      subType    anti_encoding
  // Values copied verbatim from geant4, except these particles don't decay
  // and names/PDGcodes are unique.

  static G4ParticleDefinition* g4eParticle = NULL;

  if (g4eParticle == NULL) {
    // Bohr Magneton for positron and positive muon
    G4double muBpositron = 0.5 * eplus * hbar_Planck / (0.51099906 * MeV / c_squared);
    G4double muBmuon = 0.5 * eplus * hbar_Planck / (0.1056584 * GeV / c_squared);
    G4double muNucleon = eplus * hbar_Planck / 2. / (proton_mass_c2 / c_squared);
    g4eParticle = new G4ParticleDefinition(
      "g4e_gamma",         0.0 * MeV,       0.0 * MeV,          0.0,
      2,              -1,            -1,
      0,               0,             0,
      "gamma",               0,             0,   1000000022,
      true,             0.0,          NULL,
      false,        "photon",    1000000022
    );
    new G4ParticleDefinition(
      "g4e_e-",  0.51099906 * MeV,       0.0 * MeV,    -1.*eplus,
      1,               0,             0,
      0,               0,             0,
      "lepton",               1,             0,   1000000011,
      true,            -1.0,          NULL,
      false,             "e",   -1000000011, -muBpositron * 1.0011596521859
    );
    new G4ParticleDefinition(
      "g4e_e+",  0.51099906 * MeV,       0.0 * MeV,    +1.*eplus,
      1,               0,             0,
      0,               0,             0,
      "lepton",              -1,             0,  -1000000011,
      true,            -1.0,          NULL,
      false,             "e",    1000000011, muBpositron * 1.0011596521859
    );
    new G4ParticleDefinition(
      "g4e_mu+",   0.1056584 * GeV, 2.99591e-16 * MeV,  +1.*eplus,
      1,               0,             0,
      0,               0,             0,
      "lepton",              -1,             0,  -1000000013,
      true,            -1.0,          NULL,
      false,            "mu",    1000000013, muBmuon * 1.0011659208
    );
    new G4ParticleDefinition(
      "g4e_mu-",   0.1056584 * GeV, 2.99591e-16 * MeV,  -1.*eplus,
      1,               0,             0,
      0,               0,             0,
      "lepton",               1,             0,   1000000013,
      true,            -1.0,          NULL,
      false,            "mu",   -1000000013, -muBmuon * 1.0011659208
    );
    new G4ParticleDefinition(
      "g4e_pi+",    0.1395700 * GeV,  2.5284e-14 * MeV, +1.*eplus,
      0,              -1,             0,
      2,              +2,            -1,
      "meson",               0,             0,   1000000211,
      true,            -1.0,          NULL,
      false,            "pi",   -1000000211
    );
    new G4ParticleDefinition(
      "g4e_pi-",    0.1395700 * GeV, 2.5284e-14 * MeV,  -1.*eplus,
      0,              -1,             0,
      2,              -2,            -1,
      "meson",               0,             0,  -1000000211,
      true,            -1.0,          NULL,
      false,            "pi",    1000000211
    );
    new G4ParticleDefinition(
      "g4e_kaon+",    0.493677 * GeV,  5.315e-14 * MeV,   +1.*eplus,
      0,              -1,             0,
      1,              +1,             0,
      "meson",               0,             0,   1000000321,
      true,            -1.0,          NULL,
      false,          "kaon",   -1000000321
    );
    new G4ParticleDefinition(
      "g4e_kaon-",    0.493677 * GeV,   5.315e-14 * MeV,  -1.*eplus,
      0,              -1,             0,
      1,              -1,             0,
      "meson",               0,             0,  -1000000321,
      true,            -1.0,          NULL,
      false,          "kaon",    1000000321
    );
    new G4Ions(
      "g4e_proton",   0.9382723 * GeV,       0.0 * MeV,       eplus,
      1,              +1,             0,
      1,              +1,             0,
      "baryon",               0,            +1,   1000002212,
      true,            -1.0,          NULL,
      false,       "nucleon",   -1000002212, muNucleon * 2.792847351
    );
    new G4Ions(
      "g4e_anti_proton",   0.9382723 * GeV,       0.0 * MeV,   -1.0 * eplus,
      1,              +1,             0,
      1,              -1,             0,
      "baryon",               0,            -1,  -1000002212,
      true,            -1.0,          NULL,
      false,       "nucleon",    1000002212, -muNucleon * 2.792847351
    );
  }
}

void ExtPhysicsConstructor::ConstructProcess()
{
  // Define the limited set of processes that will be suffered by the
  // geant4e-specific particles
  ExtStepLengthLimitProcess* stepLengthLimitProcess = new ExtStepLengthLimitProcess;
  ExtMagFieldLimitProcess* magFieldLimitProcess = new ExtMagFieldLimitProcess;
  ExtEnergyLoss* eLossProcess = new ExtEnergyLoss;
  ExtNoHits* noHitsProcess = new ExtNoHits;
  new ExtMessenger(stepLengthLimitProcess, magFieldLimitProcess, eLossProcess);
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator* theParticleIterator = theParticleTable->GetIterator();
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
    if (particleName.substr(0, 4) == "g4e_") {
      if (particleName == "g4e_gamma") {
        pmanager->AddDiscreteProcess(new G4GammaConversion());
        pmanager->AddDiscreteProcess(new G4ComptonScattering());
        pmanager->AddDiscreteProcess(new G4PhotoElectricEffect());
      } else {
        pmanager->AddContinuousProcess(eLossProcess, 1);
        pmanager->AddDiscreteProcess(stepLengthLimitProcess, 2);
        pmanager->AddDiscreteProcess(magFieldLimitProcess, 3);
      }
      pmanager->AddDiscreteProcess(noHitsProcess);
    }
  }
}

