/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleSelector/ParticleSelectorModule.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleSelector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleSelectorModule::ParticleSelectorModule() : Module()
  {
    setDescription("Removes Particles from given ParticleList that do not pass specified selection criteria.");

    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("decayString", m_decayString,
             "Input ParticleList name (see https://confluence.desy.de/display/BI/Physics+DecayString).");

    addParam("cut", m_cutParameter,
             "Selection criteria to be applied, see https://confluence.desy.de/display/BI/Physics+ParticleSelectorFunctions",
             std::string(""));
  }

  void ParticleSelectorModule::initialize()
  {
    // obtain the input and output particle lists from the decay string
    bool valid = m_decaydescriptor.init(m_decayString);
    if (!valid)
      B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString: " << m_decayString);

    int nProducts = m_decaydescriptor.getNDaughters();
    if (nProducts > 0)
      B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString " << m_decayString
              << ". DecayString should not contain any daughters, only the mother particle.");

    // Mother particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

    m_listName = mother->getFullName();

    StoreObjPtr<ParticleList> particleList(m_listName);
    particleList.isRequired(m_listName);

    m_cut = Variable::Cut::compile(m_cutParameter);

    B2INFO("ParticleSelector: " << m_listName);
    B2INFO("   -> With cuts  : " << m_cutParameter);
  }

  void ParticleSelectorModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);
    bool existingList = plist.isValid();

    if (!existingList) {
      B2WARNING("Input list " << m_listName << " was not created?");
      return;
    }

    // loop over list only if cuts should be applied
    std::vector<unsigned int> toRemove;
    unsigned int n = plist->getListSize();
    for (unsigned i = 0; i < n; i++) {
      const Particle* part = plist->getParticle(i);
      if (!m_cut->check(part)) toRemove.push_back(part->getArrayIndex());
    }
    plist->removeParticles(toRemove);
  }
} // end Belle2 namespace

