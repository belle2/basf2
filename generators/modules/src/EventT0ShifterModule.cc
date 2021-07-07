/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/EventT0ShifterModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

REG_MODULE(EventT0Shifter)

EventT0ShifterModule::EventT0ShifterModule() : Module()
{
  // set module description
  setDescription("Shift the time of all MCParticles so that the collision time is t=0."
                 "It reverts the shift done by the EventT0Generator module and should be used after the simulation."
                 "Note that the collisionTime in the MCInitialParticles is not changed.");

  setPropertyFlags(c_ParallelProcessingCertified);
}


void EventT0ShifterModule::initialize()
{
  m_mcParticles.isRequired();
  m_initialParticles.isRequired();
}


void EventT0ShifterModule::event()
{
  // shift MC particles times
  double collisionTime = m_initialParticles->getTime();
  for (auto& particle : m_mcParticles) {
    particle.setProductionTime(particle.getProductionTime() - collisionTime);
    particle.setDecayTime(particle.getDecayTime() - collisionTime);
  }
}
