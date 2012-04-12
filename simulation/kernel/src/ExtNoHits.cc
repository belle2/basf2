/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/ExtNoHits.h>
#include <G4SteppingControl.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtNoHits::ExtNoHits(const G4String& processName, G4ProcessType type) : G4VDiscreteProcess(processName, type)
{
  m_particleChange = new G4ParticleChange;
}

ExtNoHits::~ExtNoHits()
{
  delete m_particleChange;
}

G4VParticleChange* ExtNoHits::PostStepDoIt(const G4Track& track, const G4Step&)
{
  m_particleChange->Initialize(track);
  m_particleChange->ProposeSteppingControl(AvoidHitInvocation);
  return m_particleChange;
}
