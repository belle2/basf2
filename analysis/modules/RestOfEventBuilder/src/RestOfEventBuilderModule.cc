/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/RestOfEventBuilder/RestOfEventBuilderModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <analysis/VariableManager/Variables.h>
#include <mdst/dataobjects/Track.h>
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
  addParam("particleList", m_particleList, "Name of the ParticleList");
  addParam("onlyGoodECLClusters", m_onlyGoodECLClusters, "If true, only good ECL clusters are added", true);
  addParam("chargedStableFractions", m_fractions, "A set of probabilities of the ChargedStable particles in the process", {0, 0, 1, 0, 0, 0});

  std::string defaultSelection = std::string("");
  addParam("trackSelection", m_trackSelection, "Remaining track(s) selection criteria", defaultSelection);
  addParam("eclClusterSelection", m_eclClusterSelection, "Remaining ECL shower(s) selection criteria", defaultSelection);
}

void RestOfEventBuilderModule::initialize()
{
  // input
  StoreObjPtr<ParticleList>::required(m_particleList);
  StoreArray<Particle> particles;
  particles.isRequired();

  // output
  StoreArray<RestOfEvent> roeArray;
  roeArray.registerInDataStore();
  particles.registerRelationTo(roeArray);

  m_trackCut = Variable::Cut::Compile(m_trackSelection);
  B2INFO("RestOfEventBuilder with track cuts  : " << m_trackSelection);

  m_eclClusterCut = Variable::Cut::Compile(m_eclClusterSelection);
  B2INFO("RestOfEventBuilder with ECLCluster cuts  : " << m_eclClusterSelection);
}

void RestOfEventBuilderModule::event()
{
  // input Particle
  StoreObjPtr<ParticleList> plist(m_particleList);

  // output
  StoreArray<RestOfEvent> roeArray;
  unsigned int nParts = plist->getListSize();
  for (unsigned i = 0; i < nParts; i++) {
    const Particle* particle = plist->getParticle(i);

    // create RestOfEvent object
    RestOfEvent* roe = roeArray.appendNew();

    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(roe);

    // fill RestOfEvent with content
    addRemainingTracks(particle, roe);
    addRemainingECLClusters(particle, roe);
    addRemainingKLMClusters(particle, roe);

    roe->setChargedStableFractions(m_fractions);
  }
}

void RestOfEventBuilderModule::addRemainingTracks(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<Track> tracks;
  std::map<int, bool> trackMasks;

  // vector of all final state particle daughters created from Tracks
  std::vector<int> fspTracks = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  for (int i = 0; i < tracks.getEntries(); i++) {
    const Track* track = tracks[i];

    bool remainingTrack = true;
    for (unsigned j = 0; j < fspTracks.size(); j++) {
      if (track->getArrayIndex() == fspTracks[j]) {
        remainingTrack = false;
        break;
      }
    }

    if (remainingTrack) {
      const PIDLikelihood* pid = track->getRelatedTo<PIDLikelihood>();
      Particle p(track, pid->getMostLikely(m_frArray));
      Particle* tempPart = &p;

      if (m_trackCut->check(tempPart))
        trackMasks[track->getArrayIndex()] = true;
      else
        trackMasks[track->getArrayIndex()] = false;

      roe->addTrack(track);
    }
  }
  roe->setTrackMasks(trackMasks);
}

void RestOfEventBuilderModule::addRemainingECLClusters(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<ECLCluster> eclClusters;
  StoreArray<Track>      tracks;
  std::map<int, bool> eclClusterMasks;

  // vector of all final state particle daughters created from energy cluster or charged track
  std::vector<int> eclFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_ECLCluster);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  // Add remaining ECLClusters
  for (int i = 0; i < eclClusters.getEntries(); i++) {
    const ECLCluster* shower = eclClusters[i];
    bool passGoodCut = true;

    if (m_onlyGoodECLClusters) {
      // TODO: make this steerable
      double energy = shower->getEnergy();
      //double e9e25  = shower->getE9oE25();
      int    region = 0;

      float theta = shower->getTheta();
      if (theta < 0.555) {
        region = 1.0;
      } else if (theta < 2.26) {
        region = 2.0;
      } else {
        region = 3.0;
      }

      if (!Variable::isGoodGamma(region, energy, false))
        passGoodCut = false;
    }

    bool remainingCluster = true;
    for (unsigned j = 0; j < eclFSPs.size(); j++) {
      if (shower->getArrayIndex() == eclFSPs[j]) {
        remainingCluster = false;
        break;
      }
    }

    if (!remainingCluster) {
      continue;
    }

    // check if the ECLCluster is matched to any track used in reconstruction
    for (unsigned j = 0; j < trackFSPs.size(); j++) {
      const Track* track = tracks[trackFSPs[j]];
      const ECLCluster* trackCluster = track->getRelated<ECLCluster>();

      if (!trackCluster)
        continue;

      if (shower->getArrayIndex() == trackCluster->getArrayIndex()) {
        remainingCluster = false;
        break;
      }
    }

    if (remainingCluster) {
      Particle p(shower);
      Particle* tempPart = &p;

      if (m_eclClusterCut->check(tempPart) and passGoodCut)
        eclClusterMasks[shower->getArrayIndex()] = true;
      else
        eclClusterMasks[shower->getArrayIndex()] = false;

      roe->addECLCluster(shower);
    }
  }
  roe->setECLClusterMasks(eclClusterMasks);
}

void RestOfEventBuilderModule::addRemainingKLMClusters(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<Track>      tracks;

  // vector of all final state particle daughters created from energy cluster or charged track
  std::vector<int> klmFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_KLMCluster);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  // Add remaining KLMClusters
  for (int i = 0; i < klmClusters.getEntries(); i++) {
    const KLMCluster* cluster = klmClusters[i];

    bool remainingCluster = true;
    for (unsigned j = 0; j < klmFSPs.size(); j++) {
      if (cluster->getArrayIndex() == klmFSPs[j]) {
        remainingCluster = false;
        break;
      }
    }

    if (!remainingCluster)
      continue;

    // check if the KLMCluster is matched to any track used in reconstruction
    for (unsigned j = 0; j < trackFSPs.size(); j++) {
      const Track* track = tracks[trackFSPs[j]];
      const KLMCluster* trackCluster = track->getRelated<KLMCluster>();

      if (!trackCluster)
        continue;

      if (cluster->getArrayIndex() == trackCluster->getArrayIndex()) {
        remainingCluster = false;
        break;
      }
    }

    if (remainingCluster)
      roe->addKLMCluster(cluster);
  }
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
  for (unsigned i = 0; i < trackFSPs.size(); i++)
    std::cout << trackFSPs[i] << " ";
  std::cout << std::endl;

  B2INFO("[RestOfEventBuilderModule] eclFSPs : ");
  for (unsigned i = 0; i < eclFSPs.size(); i++)
    std::cout << eclFSPs[i] << " ";
  std::cout << std::endl;

  B2INFO("[RestOfEventBuilderModule] klmFSPs : ");
  for (unsigned i = 0; i < klmFSPs.size(); i++)
    std::cout << klmFSPs[i] << " ";
  std::cout << std::endl;

}

