/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/modules/core/RandomBarrierModule.h>
#include <framework/core/RandomNumbers.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RandomBarrier)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RandomBarrierModule::RandomBarrierModule() : Module()
{
  setDescription("Sets gRandom to an independent generator for the following modules.  E.g. a module chain of the sort [ParticleGun -> RandomBarrier -> FullSim] would use one RNG instance for the ParticleGun, and another for FullSim and all following modules. You may find this useful if you want to change the simulation, but don't want differences to affect the particle generation.  The output is equivalent to saving the output of ParticleGun in a file, and reading it again to do the simulation.  Correct separation is not provided for terminate(), don't use random numbers there.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

RandomBarrierModule::~RandomBarrierModule() = default;

void RandomBarrierModule::initialize()
{
  RandomNumbers::barrier();
}
void RandomBarrierModule::terminate()
{
  //terminate() is called in reverse order. If you use random numbers there, we can't help you.
}

void RandomBarrierModule::beginRun()
{
  RandomNumbers::barrier();
}
void RandomBarrierModule::endRun()
{
  RandomNumbers::barrier();
}
void RandomBarrierModule::event()
{
  RandomNumbers::barrier();
}
