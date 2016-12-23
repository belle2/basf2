/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/MCMatcherParticles/MCMatcherParticlesModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>

// utility
#include <analysis/utility/MCMatching.h>
#include <analysis/utility/AnalysisConfiguration.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(MCMatcherParticles)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  MCMatcherParticlesModule::MCMatcherParticlesModule() : Module()
  {
    setDescription("Performs MC matching (sets relation Particle->MCParticle) for all particles (and its (grand)^N-daughter particles) in the ParticleList. The relation can be used in conjuction with MCMatching::MCErrorFlags flags, e.g. using the isSignal or mcPDG & mcErrors variables.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("listName", m_listName, "Name of the input ParticleList.");
  }


  void MCMatcherParticlesModule::initialize()
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    particles.isRequired();
    mcparticles.isRequired();
    particles.registerRelationTo(mcparticles);
    m_plist.isRequired(m_listName);

    bool legacyAlgorithm = AnalysisConfiguration::instance()->useLegacyMCMatching();
    B2INFO("MCMatcher module will search for Particle -> MCParticle associations for the ParticleList " << m_listName << ".");
    if (legacyAlgorithm)
      B2INFO(" - The MCMatcher will use legacy algorithm suitable for analysis of Belle MC or Belle II MC5.");
    else
      B2INFO("  - The MCMatcher will use default algorithm suitable for analysis of Belle II MC (for MC5 use legacy algorithm).");
  }


  void MCMatcherParticlesModule::event()
  {
    if (!m_plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }
    const unsigned int n = m_plist->getListSize();
    for (unsigned i = 0; i < n; i++) {
      const Particle* part = m_plist->getParticle(i);

      MCMatching::setMCTruth(part);
    }
  }


} // end Belle2 namespace
