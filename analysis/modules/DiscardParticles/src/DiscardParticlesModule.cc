/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/DiscardParticles/DiscardParticlesModule.h>

#include <framework/logging/Logger.h>

#include <unordered_set>
#include <algorithm>


using namespace std;
using namespace Belle2;


REG_MODULE(DiscardParticles)

DiscardParticlesModule::DiscardParticlesModule()
{
  setDescription("Removes all Particles that are not in a given list of ParticleLists (or daughters of those). All relations from/to Particles, daughter indices, and other ParticleLists are fixed.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("particleLists", m_particleLists, "Keep the Particles and their daughters in these ParticleLists.");

}

DiscardParticlesModule::~DiscardParticlesModule()
{
}

void DiscardParticlesModule::initialize()
{
  StoreArray<Particle> particles;
  particles.isRequired();
  m_subset.registerSubset(particles);

  for (auto l : m_particleLists) {
    StoreObjPtr<ParticleList>(l).isRequired();
  }
}

void keepParticle(const Particle* p, std::unordered_set<int>* indicesToKeep)
{
  indicesToKeep->insert(p->getArrayIndex());
  unsigned int n = p->getNDaughters();
  for (unsigned int i = 0; i < n; i++) {
    keepParticle(p->getDaughter(i), indicesToKeep);
  }
}

void DiscardParticlesModule::event()
{
  std::unordered_set<int> indicesToKeep;
  for (auto l : m_particleLists) {
    StoreObjPtr<ParticleList> list(l);
    if (!list)
      continue;
    //TODO: getParticleCollectionName() might be different for some lists...
    const int n = list->getListSize();
    for (int i = 0; i < n; i++) {
      const Particle* p = list->getParticle(i);
      keepParticle(p, &indicesToKeep);
    }
  }

  //remove everything not in indicesToKeep
  auto selector = [indicesToKeep](const Particle * p) -> bool {
    int idx = p->getArrayIndex();
    return indicesToKeep.count(idx) == 1;
  };
  m_subset.select(selector);
}

void DiscardParticlesModule::terminate()
{
}
