/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/randombarrier/RandomBarrierModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RandomBarrier)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RandomBarrierModule::RandomBarrierModule() : Module(), m_random(NULL)
{
  setDescription("Sets gRandom to an independent generator for the following modules.  E.g. a module chain of the sort [ParticleGun -> RandomBarrier -> FullSim] would use one RNG instance for the ParticleGun, and another for FullSim and all following modules. You may find this useful if you want to change the simulation, but don't want differences to affect the particle generation.  The output is equivalent to saving the output of ParticleGun in a file, and reading it again to do the simulation.  Correct separation is not provided for terminate(), don't use random numbers there.");
}

RandomBarrierModule::~RandomBarrierModule()
{
  delete m_random;
}

void RandomBarrierModule::initialize()
{
  //initialize from existing RNG (using a subset of the current state)
  //if no random numbers have been used yet, m_random and gRandom now have the same state.
  //Since they're used for different things, this should be ok.
  m_random = new TRandom3(gRandom->GetSeed());

  gRandom = m_random;
}
void RandomBarrierModule::terminate()
{
  //terminate() is called in reverse order. If you use random numbers there, we can't help you.
}

void RandomBarrierModule::beginRun()
{
  gRandom = m_random;
}
void RandomBarrierModule::endRun()
{
  gRandom = m_random;
}
void RandomBarrierModule::event()
{
  gRandom = m_random;
}
