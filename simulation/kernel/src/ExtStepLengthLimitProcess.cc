/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/ExtStepLengthLimitProcess.h>
#include <G4Track.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtStepLengthLimitProcess::ExtStepLengthLimitProcess(const G4String& processName) :
  G4VDiscreteProcess(processName),
  m_stepLimit(kInfinity)  // user may change this with a geant4 UI command
{
}

ExtStepLengthLimitProcess::~ExtStepLengthLimitProcess()
{
}

G4VParticleChange* ExtStepLengthLimitProcess::PostStepDoIt(const G4Track& track, const G4Step&)
{
  aParticleChange.Initialize(track);
  aParticleChange.ProposeSteppingControl(AvoidHitInvocation);
  return &aParticleChange;
}

