/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sascha Dreyer                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from BelleII monopole simulation

#include <simulation/longlivedneutral/G4LongLivedNeutralPhysics.h>
#include <simulation/longlivedneutral/G4LongLivedNeutral.h>
#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>

#include <G4ParticleDefinition.hh>
#include <G4ProcessManager.hh>
#include <G4ProcessVector.hh>
#include <G4StepLimiter.hh>
#include <G4Transportation.hh>
#include <G4hMultipleScattering.hh>
#include <G4hhIonisation.hh>
#include <G4hIonisation.hh>
#include <G4PhysicsListHelper.hh>
#include <G4BuilderType.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace CLHEP;

G4LongLivedNeutralPhysics::G4LongLivedNeutralPhysics(int pdg, double mass)
  : G4VPhysicsConstructor("LongLivedNeutralPhysics_PDG" + std::to_string(pdg)),
    fLLPN(0)
{
  fParticleName = "LongLivedNeutralParticle_PDG" + std::to_string(pdg);
  fMass = mass;
  fEncoding = pdg;
}

G4LongLivedNeutralPhysics::~G4LongLivedNeutralPhysics()
{
}

void G4LongLivedNeutralPhysics::ConstructParticle()
{
  fLLPN = new G4LongLivedNeutral(fParticleName, fMass,  fEncoding);
}


void G4LongLivedNeutralPhysics::ConstructProcess()
{
  G4cout << "G4LongLivedNeutralPhysics::ConstructProcess" << G4endl;
}
