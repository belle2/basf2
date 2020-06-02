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
#include <simulation/longlivedneutral/G4LongLivedNeutralDecay.h>
#include <simulation/longlivedneutral/G4LongLivedNeutralTransportation.h>
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
#include <G4Decay.hh>
#include <G4PhysicsListHelper.hh>
#include <G4BuilderType.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace CLHEP;

G4LongLivedNeutralPhysics::G4LongLivedNeutralPhysics()
  : G4VPhysicsConstructor("LongLivedNeutralPhysics"),
    fLLPN(0)
{

}

G4LongLivedNeutralPhysics::~G4LongLivedNeutralPhysics()
{
}

void G4LongLivedNeutralPhysics::ConstructParticle()
{
  fLLPN = new G4LongLivedNeutral("LongLivedNeutralParticle");
}


void G4LongLivedNeutralPhysics::ConstructProcess()
{
  B2DEBUG(20, "G4LongLivedNeutralPhysics::ConstructProcess");

  G4ProcessManager* pmanager = fLLPN->GetProcessManager();

  // remove all processes added per default (em scintillation and transport)
  for (int i = 0; i <= pmanager->GetProcessListLength(); ++i) {
    pmanager->RemoveProcess(i);
  }
  pmanager->RemoveProcess(0);

  pmanager->AddProcess(new G4LongLivedNeutralTransportation(), -1, 0, 0);
  pmanager->AddProcess(new G4LongLivedNeutralDecay(), 0, -1, 0);
}
