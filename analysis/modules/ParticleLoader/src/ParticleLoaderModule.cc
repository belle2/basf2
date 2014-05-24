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
#include <analysis/modules/ParticleLoader/ParticleLoaderModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleLoader)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleLoaderModule::ParticleLoaderModule() : Module()

  {
    setDescription("Fills StoreArray<Particle> from MDST data");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("UseMCParticles", m_useMCParticles, "use MCParticles instead of tracks, ECL clusters", false);
  }

  ParticleLoaderModule::~ParticleLoaderModule()
  {
  }

  void ParticleLoaderModule::initialize()
  {
    StoreArray<Particle>::registerPersistent();
    StoreObjPtr<ParticleExtraInfoMap>::registerPersistent("", DataStore::c_Event, false); //allow reregistration
    RelationArray::registerPersistent<Particle, PIDLikelihood>();
    RelationArray::registerPersistent<Particle, MCParticle>();

    if (m_useMCParticles) {
      B2INFO("ParticleLoader: Loading Particle(s) from final state primary MCParticle(s)");
    } else {
      B2INFO("ParticleLoader: Loading Particle(s) from reconstructed Track(s) (as e/mu/pi/K/p), neutral ECLCluster(s) (as photons) and neutral KLMClusters (as Klongs)");
    }
  }

  void ParticleLoaderModule::beginRun()
  {
  }

  void ParticleLoaderModule::event()
  {
    StoreArray<Particle> Particles;
    if (Particles.isValid()) {
      B2INFO("ParticleLoader::loadFromFile size=" << Particles.getEntries());
      B2WARNING("loadFromFile not fully implemented yet and may result in double counting!");
      return;
    }

    Particles.create();
    if (m_useMCParticles)
      loadFromMCParticles();
    else
      loadFromReconstruction();

  }


  void ParticleLoaderModule::endRun()
  {
  }

  void ParticleLoaderModule::terminate()
  {
  }

  void ParticleLoaderModule::loadFromMCParticles()
  {

    StoreArray<MCParticle> mcParticles;
    StoreArray<Particle> Particles;

    unsigned int bitmask = MCParticle::c_PrimaryParticle;
    const int Npdg = 7;
    int pdgCode[Npdg] = {11, 13, 211, 321, 2212, 22, 130}; // e,mu,pi,K,p,gamma,K_L

    for (int i = 0; i < mcParticles.getEntries(); i++) {
      const MCParticle* mc = mcParticles[i];
      if (!mc->hasStatus(bitmask)) continue;
      for (int k = 0; k < Npdg; k++) {
        if (abs(mc->getPDG()) == pdgCode[k]) {
          Particle* newPart = Particles.appendNew(mc);
          newPart->addRelationTo(mc);
          break;
        }
      }
    }
    //B2INFO("ParticleLoader::loadFromMCParticles size=" << Particles.getEntries());
  }

  void ParticleLoaderModule::loadFromReconstruction()
  {
    StoreArray<Track> Tracks;
    StoreArray<ECLCluster> ECLClusters;
    StoreArray<KLMCluster> KLMClusters;
    StoreArray<Particle> Particles;

    const Const::ChargedStable charged[] = {Const::electron,
                                            Const::muon,
                                            Const::pion,
                                            Const::kaon,
                                            Const::proton
                                           };

    // load reconstructed tracks as e, mu, pi, K, p
    for (int i = 0; i < Tracks.getEntries(); i++) {
      const Track* track = Tracks[i];
      const PIDLikelihood* pid = track->getRelated<PIDLikelihood>();
      const MCParticle* mcParticle = track->getRelated<MCParticle>();
      for (int k = 0; k < 5; k++) {
        Particle particle(track, charged[k]);
        if (particle.getParticleType() == Particle::c_Track) { // should always hold but...
          Particle* newPart = Particles.appendNew(particle);
          if (pid)
            newPart->addRelationTo(pid);
          if (mcParticle)
            newPart->addRelationTo(mcParticle);
        }
      }
    }

    // load reconstructed neutral ECL cluster's as photons
    for (int i = 0; i < ECLClusters.getEntries(); i++) {
      const ECLCluster* cluster      = ECLClusters[i];

      if (!cluster->isNeutral())
        continue;

      const MCParticle* mcParticle = cluster->getRelated<MCParticle>();

      Particle particle(cluster);

      if (particle.getParticleType() == Particle::c_ECLCluster) { // should always hold but...
        Particle* newPart = Particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);
      }
    }

    // load reconstructed neutral KLM cluster's as photons
    for (int i = 0; i < KLMClusters.getEntries(); i++) {
      const KLMCluster* cluster      = KLMClusters[i];

      if (cluster->getAssociatedTrackFlag())
        continue;

      const MCParticle* mcParticle = cluster->getRelated<MCParticle>();

      Particle particle(cluster);

      if (particle.getParticleType() == Particle::c_KLMCluster) { // should always hold but...
        Particle* newPart = Particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);
      }
    }

    //B2INFO("ParticleLoader::loadFromReconstruction size=" << Particles.getEntries());
  }
} // end Belle2 namespace

