/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from BelleII monopole simulation

#include <simulation/longlivedneutral/G4LongLivedNeutralPhysics.h>
#include <simulation/longlivedneutral/G4LongLivedNeutral.h>
#include <simulation/longlivedneutral/G4LongLivedNeutralDecay.h>
#include <simulation/longlivedneutral/G4LongLivedNeutralTransportation.h>
#include <framework/logging/Logger.h>

#include <G4ProcessManager.hh>
#include <CLHEP/Units/SystemOfUnits.h>

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
