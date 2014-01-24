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

#include <tracking/dataobjects/Track.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <ecl/dataobjects/ECLShower.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

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

  // Parameter definitions
  addParam("particleList", m_particleList, "Name of the ParticleList", std::string(""));

  //std::vector<std::string> defaultSelection;
  //addParam("trackSelection",     m_trackSelection,     "Remaining track(s) selection criteria",      defaultSelection);
  //addParam("eclShowerSelection", m_eclShowerSelection, "Remaining ECL shower(s) selection criteria", defaultSelection);
  //addParam("eclGammaSelection",  m_eclGammaSelection,  "Remaining ECL gamma(s) selection criteria",  defaultSelection);
  //addParam("eclPi0Selection",    m_eclPi0Selection,    "Remaining ECL pi0(s) selection criteria",    defaultSelection);

}

void RestOfEventBuilderModule::initialize()
{
  // input
  StoreObjPtr<ParticleList>::required(m_particleList);
  StoreArray<Particle>::required();

  // output
  StoreArray<RestOfEvent>::registerPersistent();
  RelationArray::registerPersistent<Particle, RestOfEvent>();
}

void RestOfEventBuilderModule::event()
{
  // input Particle
  StoreObjPtr<ParticleList> plist(m_particleList);
  StoreObjPtr<Particle>     particles;

  // output
  StoreArray<RestOfEvent> roeArray;

  for (unsigned i = 0; i < plist->getListSize(); i++) {
    const Particle* particle = plist->getParticle(i);

    // create RestOfEvent object
    RestOfEvent* roe = roeArray.appendNew(RestOfEvent());

    // create relation: Particle <-> RestOfEvent
    particle->addRelationTo(roe);

    // fill RestOfEvent with content
    addRemainingTracks(particle, roe);
    addRemainingECLObjects(particle, roe);
  }
}

void RestOfEventBuilderModule::addRemainingTracks(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<Track> tracks;

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

    if (remainingTrack)
      roe->addTrack(track);
  }
}

void RestOfEventBuilderModule::addRemainingECLObjects(const Particle* particle, RestOfEvent* roe)
{
  StoreArray<ECLGamma>  eclGammas;
  StoreArray<ECLPi0>    eclPi0s;
  StoreArray<ECLShower> eclShowers;
  StoreArray<Track>     tracks;

  // vector of all final state particle daughters created from ECLGamma
  std::vector<int> eclFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_ECLGamma);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  // Add remaining ECLGammas
  for (int i = 0; i < eclGammas.getEntries(); i++) {
    const ECLGamma* gamma = eclGammas[i];

    bool remainingGamma = true;
    for (unsigned j = 0; j < eclFSPs.size(); j++) {
      if (gamma->getArrayIndex() == eclFSPs[j]) {
        remainingGamma = false;
        break;
      }
    }

    if (remainingGamma)
      roe->addECLGamma(gamma);
  }

  // Add remaining ECLPi0s
  for (int i = 0; i < eclPi0s.getEntries(); i++) {
    const ECLPi0* pi0 = eclPi0s[i];

    bool remainingPi0 = true;
    for (unsigned j = 0; j < eclFSPs.size(); j++) {
      const ECLGamma* gamma = eclGammas[eclFSPs[j]];

      if (pi0->getShowerId1() == gamma->getShowerId()
          || pi0->getShowerId2() == gamma->getShowerId()) {
        remainingPi0 = false;
        break;
      }
    }

    if (remainingPi0)
      roe->addECLPi0(pi0);
  }

  // Add remaining ECLShowers
  for (int i = 0; i < eclShowers.getEntries(); i++) {
    const ECLShower* shower = eclShowers[i];

    bool remainingShower = true;
    for (unsigned j = 0; j < eclFSPs.size(); j++) {
      const ECLGamma* gamma = eclGammas[eclFSPs[j]];

      if (shower->getShowerId() == gamma->getShowerId()) {
        remainingShower = false;
        break;
      }
    }

    if (!remainingShower)
      continue;

    // check if the ECLShower is matched to any track used in reconstruction
    for (unsigned j = 0; j < trackFSPs.size(); j++) {
      const Track* track = tracks[trackFSPs[j]];
      const ECLShower* trackShower = DataStore::getRelated<ECLShower>(track);

      if (!trackShower)
        continue;

      if (shower->getShowerId() == trackShower->getShowerId()) {
        remainingShower = false;
        break;
      }
    }

    if (remainingShower)
      roe->addECLShower(shower);
  }
}

void RestOfEventBuilderModule::printEvent()
{
  StoreArray<ECLGamma>  eclGammas;
  StoreArray<ECLPi0>    eclPi0s;
  StoreArray<ECLShower> eclShowers;
  StoreArray<Track>     tracks;

  B2INFO("[RestOfEventBuilderModule] *** Print Event ***");
  B2INFO("[RestOfEventBuilderModule] Tracks: " << tracks.getEntries());
  for (int i = 0; i < tracks.getEntries(); i++) {
    const Track* track = tracks[i];
    const ECLShower* trackShower = DataStore::getRelated<ECLShower>(track);
    if (trackShower) {
      B2INFO("[RestOfEventBuilderModule]  -> track " << track->getArrayIndex() << " -> ECLShower " << trackShower->getArrayIndex());
    } else {
      B2INFO("[RestOfEventBuilderModule]  -> track " << track->getArrayIndex() << " -> ECLShower (NO RELATION)");
    }
  }

  B2INFO("[RestOfEventBuilderModule] ECLPi0: " << eclPi0s.getEntries());
  for (int i = 0; i < eclPi0s.getEntries(); i++) {
    const ECLPi0* eclPi0 = eclPi0s[i];

    B2INFO("[RestOfEventBuilderModule]  -> pi0 " << eclPi0->getArrayIndex()
           << " -> ECLShower1 " << eclPi0->getShowerId1()
           << " + ECLShower2 " << eclPi0->getShowerId2());
  }

  B2INFO("[RestOfEventBuilderModule] ECLGamma: " << eclGammas.getEntries());
  for (int i = 0; i < eclGammas.getEntries(); i++) {
    const ECLGamma* eclGamma = eclGammas[i];

    B2INFO("[RestOfEventBuilderModule]  -> gamma " << eclGamma->getArrayIndex()
           << " -> ECLShower " << eclGamma->getShowerId());
  }

  B2INFO("[RestOfEventBuilderModule] ECLShower: " << eclShowers.getEntries());
}

void RestOfEventBuilderModule::printParticle(const Particle* particle)
{
  std::vector<int> eclFSPs   = particle->getMdstArrayIndices(Particle::EParticleType::c_ECLGamma);
  std::vector<int> trackFSPs = particle->getMdstArrayIndices(Particle::EParticleType::c_Track);

  B2INFO("[RestOfEventBuilderModule] tracks  : ");
  for (unsigned i = 0; i < trackFSPs.size(); i++)
    std::cout << trackFSPs[i] << " ";
  std::cout << std::endl;

  B2INFO("[RestOfEventBuilderModule] eclFSPs : ");
  for (unsigned i = 0; i < eclFSPs.size(); i++)
    std::cout << eclFSPs[i] << " ";
  std::cout << std::endl;
}

