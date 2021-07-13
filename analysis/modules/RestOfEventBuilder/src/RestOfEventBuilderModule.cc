/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/RestOfEventBuilder/RestOfEventBuilderModule.h>

#include <mdst/dataobjects/MCParticle.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RestOfEventBuilder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RestOfEventBuilderModule::RestOfEventBuilderModule() : Module()
{
  // Set module properties
  setDescription("Creates for each Particle in the given ParticleList a RestOfEvent dataobject and makes basf2 relation between them.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  std::vector<std::string> emptyList;
  addParam("particleList", m_particleListName, "Name of the ParticleList");
  addParam("particleListsInput", m_particleListsInput, "List of the particle lists, which serve as a source of particles", emptyList);
  addParam("createNestedROE", m_createNestedROE, "A switch to create nested ROE", false);
  addParam("nestedROEMask", m_nestedMask, "A switch to create nested ROE", std::string(""));
  addParam("fromMC", m_fromMC, "A switch to create MC ROE", false);
  addParam("useKLMEnergy", m_useKLMEnergy, "A switch to create ROE with KLM energy included", false);
  addParam("mostLikely", m_builtWithMostLikely, "whether input particle lists contain most-likely lists", true);
  m_nestedROEArrayName = "NestedRestOfEvents";
}

void RestOfEventBuilderModule::initialize()
{
  // input
  m_particleList.isRequired(m_particleListName);
  m_particles.isRequired();

  // output
  m_roeArray.registerInDataStore();
  m_particles.registerRelationTo(m_roeArray);
  if (m_createNestedROE) {
    m_nestedROEArray.registerInDataStore(m_nestedROEArrayName);
    m_particles.registerRelationTo(m_nestedROEArray);
    m_roeArray.registerRelationTo(m_nestedROEArray);
  }
  if (m_useKLMEnergy) {
    B2WARNING("*** The ROE for " << m_particleListName << " list will have KLM energy included into its 4-momentum. ***");
  }
}

void RestOfEventBuilderModule::event()
{
  if (!m_createNestedROE) {
    createROE();
  } else {
    createNestedROE();
  }

}

void RestOfEventBuilderModule::createNestedROE()
{
  // input target Particle
  StoreObjPtr<RestOfEvent> hostROE("RestOfEvent");
  if (!hostROE.isValid()) {
    B2WARNING("ROE list is not valid somehow, nested ROE is not created!");
    return;
  }
  auto outerROEParticles = hostROE->getParticles(m_nestedMask);
  unsigned int nParticles = m_particleList->getListSize();
  for (unsigned i = 0; i < nParticles; i++) {
    const Particle* particle = m_particleList->getParticle(i);
    // check if a Particle object is already related to a RestOfEvent object
    auto* check_roe = particle->getRelated<RestOfEvent>();
    if (check_roe != nullptr) {
      return;
    }
    // create nested RestOfEvent object:
    RestOfEvent* nestedROE = m_nestedROEArray.appendNew(particle->getPDGCode(), true);
    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(nestedROE);
    // create relation: host ROE <-> nested ROE
    hostROE->addRelationTo(nestedROE);
    auto fsdaughters =  particle->getFinalStateDaughters();
    std::vector<const Particle* > particlesToAdd;
    for (auto* outerROEParticle : outerROEParticles) {
      bool toAdd = true;
      for (auto* daughter : fsdaughters) {
        if (outerROEParticle->isCopyOf(daughter, true)) {
          toAdd = false;
          break;
        }
      }
      if (toAdd) {
        particlesToAdd.push_back(outerROEParticle);
      }
    }
    nestedROE->addParticles(particlesToAdd);
  }
}

void RestOfEventBuilderModule::createROE()
{
  unsigned int nParts = m_particleList->getListSize();
  for (unsigned i = 0; i < nParts; i++) {
    const Particle* particle = m_particleList->getParticle(i);

    // check if a Particle object is already related to a RestOfEvent object
    auto* check_roe = particle->getRelated<RestOfEvent>();
    if (check_roe != nullptr)
      return;

    // create RestOfEvent object
    RestOfEvent* roe = m_roeArray.appendNew(particle->getPDGCode(), false, m_fromMC, m_useKLMEnergy, m_builtWithMostLikely);

    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(roe);

    // fill RestOfEvent with content
    addRemainingParticles(particle, roe);
  }
}

void RestOfEventBuilderModule::addRemainingParticles(const Particle* particle, RestOfEvent* roe)
{
  auto fsdaughters =  particle->getFinalStateDaughters();
  int nParticleLists = m_particleListsInput.size();
  B2DEBUG(10, "Particle has " + std::to_string(fsdaughters.size()) + " daughters");
  for (auto* daughter : fsdaughters) {
    B2DEBUG(10, "\t" << daughter->getArrayIndex() << ": pdg " << daughter->getPDGCode());
    B2DEBUG(10, "\t\t Store array particle: " << m_particles[daughter->getArrayIndex()]->getPDGCode());
  }
  unsigned int nExcludedParticles = 0;
  std::vector<const Particle* > particlesToAdd;
  B2DEBUG(10, "nLists: " << nParticleLists);
  for (int i_pl = 0; i_pl != nParticleLists; ++i_pl) {

    std::string particleListName = m_particleListsInput[i_pl];
    B2DEBUG(10, "ParticleList: " << particleListName);
    StoreObjPtr<ParticleList> plist(particleListName);
    int m_part = plist->getListSize();
    for (int i = 0; i < m_part; i++) {
      Particle* storedParticle = plist->getParticle(i);

      std::vector<const Particle*> storedParticleDaughters = storedParticle->getFinalStateDaughters();
      for (auto* storedParticleDaughter : storedParticleDaughters) {
        bool toAdd = true;
        if ((m_fromMC and storedParticleDaughter->getParticleSource() != Particle::EParticleSourceObject::c_MCParticle)
            or (!m_fromMC and storedParticleDaughter->getParticleSource() == Particle::EParticleSourceObject::c_MCParticle)) {
          B2FATAL("The value of fromMC parameter is not consisted with the type of provided particles, MC vs Reco");
        }
        // Remove non primary MCParticles
        if (m_fromMC and !storedParticleDaughter->getMCParticle()->hasStatus(MCParticle::c_PrimaryParticle)) {
          nExcludedParticles++;
          continue;
        }
        for (auto* daughter : fsdaughters) {
          if (storedParticleDaughter->isCopyOf(daughter, true)) {
            B2DEBUG(10, "Ignoring Particle with PDG " << daughter->getPDGCode() << " index " <<
                    storedParticleDaughter->getArrayIndex() << " to " << daughter->getArrayIndex());
            B2DEBUG(10, "Is copy " << storedParticleDaughter->isCopyOf(daughter));
            toAdd = false;
            nExcludedParticles++;
            break;
          }
        }
        if (toAdd) {
          particlesToAdd.push_back(storedParticleDaughter);
        }
      }
    }
  }
  if (fsdaughters.size() > nExcludedParticles) {
    B2WARNING("Number of excluded particles do not coincide with the number of target FSP daughters! Provided lists must be incomplete");
  }
  roe->addParticles(particlesToAdd);
}

void RestOfEventBuilderModule::printEvent()
{
  B2INFO("[RestOfEventBuilderModule] *** Print Event ***");
  B2INFO("[RestOfEventBuilderModule] Tracks: " << m_tracks.getEntries());
  for (int i = 0; i < m_tracks.getEntries(); i++) {
    const Track* track = m_tracks[i];
    const ECLCluster* trackECLCluster = track->getRelated<ECLCluster>();
    const KLMCluster* trackKLMCluster = track->getRelated<KLMCluster>();
    if (trackECLCluster) {
      B2INFO("[RestOfEventBuilderModule]  -> track " << track->getArrayIndex() << " -> ECLCluster " << trackECLCluster->getArrayIndex());
    } else {
      B2INFO("[RestOfEventBuilderModule]  -> track " << track->getArrayIndex() << " -> ECLCluster (NO RELATION)");
    }
    if (trackKLMCluster) {
      B2INFO("[RestOfEventBuilderModule]  -> track " << track->getArrayIndex() << " -> KLMCluster " << trackKLMCluster->getArrayIndex());
    } else {
      B2INFO("[RestOfEventBuilderModule]  -> track " << track->getArrayIndex() << " -> KLMCluster (NO RELATION)");
    }
  }

  B2INFO("[RestOfEventBuilderModule] ECLCluster: " << m_eclClusters.getEntries());
  for (int i = 0; i < m_eclClusters.getEntries(); i++) {
    const ECLCluster* eclCluster = m_eclClusters[i];

    B2INFO("[RestOfEventBuilderModule]  -> cluster " << eclCluster->getArrayIndex());
  }

  B2INFO("[RestOfEventBuilderModule] KLMCluster: " << m_klmClusters.getEntries());
  for (int i = 0; i < m_klmClusters.getEntries(); i++) {
    const KLMCluster* klmCluster = m_klmClusters[i];

    B2INFO("[RestOfEventBuilderModule]  -> cluster " << klmCluster->getArrayIndex());
  }
}

void RestOfEventBuilderModule::printParticle(const Particle* particle)
{
  std::vector<int> eclFSPs   = particle->getMdstArrayIndices(Particle::EParticleSourceObject::c_ECLCluster);
  std::vector<int> klmFSPs   = particle->getMdstArrayIndices(Particle::EParticleSourceObject::c_KLMCluster);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleSourceObject::c_Track);

  B2INFO("[RestOfEventBuilderModule] tracks  : ");

  std::string printout;
  for (int trackFSP : trackFSPs)
    printout += std::to_string(trackFSP) + " ";
  B2INFO(printout);

  printout.clear();

  B2INFO("[RestOfEventBuilderModule] eclFSPs : ");
  for (int eclFSP : eclFSPs)
    printout += std::to_string(eclFSP) + " ";
  B2INFO(printout);

  printout.clear();

  B2INFO("[RestOfEventBuilderModule] klmFSPs : ");
  for (int klmFSP : klmFSPs)
    printout += std::to_string(klmFSP) + " ";
  B2INFO(printout);

}
