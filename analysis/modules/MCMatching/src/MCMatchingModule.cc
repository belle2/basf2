/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/MCMatching/MCMatchingModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>

// utility
#include <analysis/utility/MCMatching.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(MCMatching)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  MCMatchingModule::MCMatchingModule() : Module()
  {
    setDescription("Performs MC matching (sets relation Particle->MCParticle) for all particles (and its (grand)^N-daughter particles) in the ParticleList. The relation can be used in conjuction with MCMatching::MCMatchStatus flags, e.g. using the isSignal or mcPDG & mcStatus variables.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("listName", m_listName, "Name of the input ParticleList.", string(""));
  }


  MCMatchingModule::~MCMatchingModule()
  {
  }

  void MCMatchingModule::initialize()
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    particles.isRequired();
    mcparticles.isRequired();
    particles.registerRelationTo(mcparticles);
    m_plist.isRequired(m_listName);
  }

  void MCMatchingModule::beginRun()
  {
  }

  void MCMatchingModule::event()
  {
    if (!m_plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    for (unsigned i = 0; i < m_plist->getListSize(); i++) {
      const Particle* part = m_plist->getParticle(i);

      MCMatching::setMCTruth(part);
    }
  }

  void MCMatchingModule::endRun()
  {
  }

  void MCMatchingModule::terminate()
  {
  }

} // end Belle2 namespace
