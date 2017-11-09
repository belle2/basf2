/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/EventT0ShifterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

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
