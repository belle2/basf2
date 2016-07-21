/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/SignalSideParticleFilter/SignalSideParticleFilterModule.h>

#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SignalSideParticleFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SignalSideParticleFilterModule::SignalSideParticleFilterModule() : Module()
{
  // Set module properties
  setDescription("The module returns true if the current RestOfEvent object is related to\n"
                 "any of the Particles from the input ParticleList and passes selection criteria.\n"
                 "The module should be executed only in the for_each ROE path.");

  // Parameter definitions
  addParam("particleListName", m_particleListName, "Input ParticleList name", std::string(""));
  addParam("selection", m_selection, "Additional selection criteria", std::string(""));
}

void SignalSideParticleFilterModule::initialize()
{
  StoreArray<Particle>::required();
  m_inputList.isRequired(m_particleListName);

  m_cut = Variable::Cut::compile(m_selection);

}

void SignalSideParticleFilterModule::event()
{
  setReturnValue(false);
  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    return;
  }

  const unsigned int numParticles = m_inputList->getListSize();
  if (numParticles == 0)
    return;

  bool inTheList = false;
  StoreObjPtr<RestOfEvent> roe("RestOfEvent");
  if (roe.isValid()) {
    const Particle* particle = roe->getRelated<Particle>();
    if (particle)
      if (m_inputList->contains(particle) && m_cut->check(particle))
        inTheList = true;
  }

  setReturnValue(inTheList);
}


