/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 * Derived from: G4ErrorStepLengthLimitProcess.cc                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/ExtStepLengthLimitProcess.h>
#include <G4Track.hh>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtStepLengthLimitProcess::ExtStepLengthLimitProcess(const G4String& processName) :
  G4VDiscreteProcess(processName)
{
  m_stepLimit = kInfinity; // user may change this with a geant4 UI command
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

