/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Sviatoslav Bilokin                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/RestOfEventBuilder/RestOfEventBuilderModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

#include <iostream>

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
  setDescription("Creates for each Particle in the given ParticleList a RestOfEvent dataobject and makes BASF2 relation between them.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  std::vector<std::string> emptyList;
  addParam("particleList", m_particleList, "Name of the ParticleList");
  addParam("particleListsInput", m_particleListsInput, "List of the particle lists, which serve as a source of particles", emptyList);
  addParam("createNestedROE", m_createNestedROE, "A switch to create nested ROE", false);
  addParam("nestedROEMask", m_nestedMask, "A switch to create nested ROE", std::string(""));
  m_nestedROEArrayName = "NestedRestOfEvents";
}

void RestOfEventBuilderModule::initialize()
{
  // input
  StoreObjPtr<ParticleList>().isRequired(m_particleList);
  StoreArray<Particle> particles;
  particles.isRequired();

  // output
  StoreArray<RestOfEvent> roeArray;
  roeArray.registerInDataStore();
  particles.registerRelationTo(roeArray);
  if (m_createNestedROE) {
    StoreArray<RestOfEvent> nestedROEArray(m_nestedROEArrayName);
    nestedROEArray.registerInDataStore();
    particles.registerRelationTo(nestedROEArray);
    roeArray.registerRelationTo(nestedROEArray);
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
  StoreObjPtr<ParticleList> plist(m_particleList);
  StoreArray<RestOfEvent> nestedROEArray(m_nestedROEArrayName);
  StoreObjPtr<RestOfEvent> hostROE("RestOfEvent");
  if (!hostROE.isValid()) {
    B2WARNING("ROE list is not valid somehow, nested ROE is not created!");
    return;
  }
  auto outerROEParticles = hostROE->getParticles(m_nestedMask);
  unsigned int nParticles = plist->getListSize();
  for (unsigned i = 0; i < nParticles; i++) {
    const Particle* particle = plist->getParticle(i);
    // check if a Particle object is already related to a RestOfEvent object
    RestOfEvent* check_roe = particle->getRelated<RestOfEvent>();
    if (check_roe != nullptr) {
      return;
    }
    // create nested RestOfEvent object:
    RestOfEvent* nestedROE = nestedROEArray.appendNew(true);
    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(nestedROE);
    // create relation: host ROE <-> nested ROE
    hostROE->addRelationTo(nestedROE);
    auto fsdaughters =  particle->getFinalStateDaughters();
    std::vector<const Particle* > particlesToAdd;
    for (auto* outerROEParticle : outerROEParticles) {
      bool toAdd = true;
      for (auto* daughter : fsdaughters) {
        if (RestOfEvent::compareParticles(outerROEParticle, daughter)) {
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
  // input target Particle
  StoreObjPtr<ParticleList> plist(m_particleList);

  // output
  StoreArray<RestOfEvent> roeArray;
  unsigned int nParts = plist->getListSize();
  for (unsigned i = 0; i < nParts; i++) {
    const Particle* particle = plist->getParticle(i);

    // check if a Particle object is already related to a RestOfEvent object
    RestOfEvent* check_roe = particle->getRelated<RestOfEvent>();
    if (check_roe != nullptr)
      return;

    // create RestOfEvent object
    RestOfEvent* roe = roeArray.appendNew();

    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(roe);

    // fill RestOfEvent with content
    addRemainingParticles(particle, roe);
  }
}

void RestOfEventBuilderModule::addRemainingParticles(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<Particle> particlesArray;
  auto fsdaughters =  particle->getFinalStateDaughters();
  int nParticleLists = m_particleListsInput.size();
  B2DEBUG(10, "Particle has " + std::to_string(fsdaughters.size()) + " daughters");
  for (auto* daughter : fsdaughters) {
    B2DEBUG(10, "\t" << daughter->getArrayIndex() << ": pdg " << daughter->getPDGCode());
    B2DEBUG(10, "\t\t Store array particle: " << particlesArray[daughter->getArrayIndex()]->getPDGCode());
  }
  unsigned int nExcludedParticles = 0;
  std::vector<const Particle* > particlesToAdd;
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
        for (auto* daughter : fsdaughters) {
          if (RestOfEvent::compareParticles(storedParticleDaughter, daughter)) {
            B2DEBUG(10, "Ignoring Particle with PDG " << storedParticleDaughter->getPDGCode() << " index " <<
                    storedParticleDaughter->getMdstArrayIndex() << " to " << daughter->getMdstArrayIndex());
            B2DEBUG(10, "Is copy " << storedParticleDaughter->isCopyOf(daughter));
            toAdd = false;
            nExcludedParticles++;
            break;
          }
        }
        if (toAdd) {
          //roe->addParticle(storedParticle);
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
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  StoreArray<Track>      tracks;

  B2INFO("[RestOfEventBuilderModule] *** Print Event ***");
  B2INFO("[RestOfEventBuilderModule] Tracks: " << tracks.getEntries());
  for (int i = 0; i < tracks.getEntries(); i++) {
    const Track* track = tracks[i];
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

  B2INFO("[RestOfEventBuilderModule] ECLCluster: " << eclClusters.getEntries());
  for (int i = 0; i < eclClusters.getEntries(); i++) {
    const ECLCluster* eclCluster = eclClusters[i];

    B2INFO("[RestOfEventBuilderModule]  -> cluster " << eclCluster->getArrayIndex());
  }

  B2INFO("[RestOfEventBuilderModule] KLMCluster: " << klmClusters.getEntries());
  for (int i = 0; i < klmClusters.getEntries(); i++) {
    const KLMCluster* klmCluster = klmClusters[i];

    B2INFO("[RestOfEventBuilderModule]  -> cluster " << klmCluster->getArrayIndex());
  }
}

void RestOfEventBuilderModule::printParticle(const Particle* particle)
{
  std::vector<int> eclFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_ECLCluster);
  std::vector<int> klmFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_KLMCluster);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  B2INFO("[RestOfEventBuilderModule] tracks  : ");

  std::string printout;
  for (unsigned i = 0; i < trackFSPs.size(); i++)
    printout += std::to_string(trackFSPs[i]) + " ";
  B2INFO(printout);

  printout.clear();

  B2INFO("[RestOfEventBuilderModule] eclFSPs : ");
  for (unsigned i = 0; i < eclFSPs.size(); i++)
    printout += std::to_string(eclFSPs[i]) + " ";
  B2INFO(printout);

  printout.clear();

  B2INFO("[RestOfEventBuilderModule] klmFSPs : ");
  for (unsigned i = 0; i < klmFSPs.size(); i++)
    printout += std::to_string(klmFSPs[i]) + " ";
  B2INFO(printout);

}
