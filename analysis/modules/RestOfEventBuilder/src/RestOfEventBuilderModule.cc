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
  addParam("particleList", m_particleList, "Name of the ParticleList");
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

    //TODO: check if roe with this name already exists

    // create RestOfEvent object
    RestOfEvent* roe = roeArray.appendNew();

    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(roe);

    // fill RestOfEvent with content
    addRemainingTracks(particle, roe);
    addRemainingECLClusters(particle, roe);
    addRemainingKLMClusters(particle, roe);
  }
}

void RestOfEventBuilderModule::addRemainingTracks(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<Track> tracks;

  // vector of all final state particle daughters created from Tracks
  std::vector<int> fspTracks = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  for (int i = 0; i < tracks.getEntries(); i++) {
    const Track* track = tracks[i];

    // ignore tracks with charge = 0
    const TrackFitResult* trackFit = track->getTrackFitResult(Const::pion);
    int charge = trackFit->getChargeSign();
    if (charge == 0) {
      B2WARNING("Track with charge = 0 not added to ROE!");
      continue;
    }

    bool remainingTrack = true;
    for (unsigned j = 0; j < fspTracks.size(); j++) {
      if (track->getArrayIndex() == fspTracks[j]) {
        remainingTrack = false;
        break;
      }
    }

    if (remainingTrack)
      roe->addTrack(track);
  }
}

void RestOfEventBuilderModule::addRemainingECLClusters(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<ECLCluster> eclClusters;
  StoreArray<Track>      tracks;

  // vector of all final state particle daughters created from energy cluster or charged track
  std::vector<int> eclFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_ECLCluster);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  // Add remaining ECLClusters
  for (int i = 0; i < eclClusters.getEntries(); i++) {
    const ECLCluster* shower = eclClusters[i];

    bool remainingCluster = true;
    for (unsigned j = 0; j < eclFSPs.size(); j++) {
      if (shower->getArrayIndex() == eclFSPs[j]) {
        remainingCluster = false;
        break;
      }
    }

    if (!remainingCluster)
      continue;

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

    if (remainingCluster)
      roe->addECLCluster(shower);
  }
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
