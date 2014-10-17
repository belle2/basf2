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
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/V0.h>
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
    addParam("useMCParticles", m_useMCParticles, "use MCParticles instead of tracks, ECL clusters", false);
  }

  ParticleLoaderModule::~ParticleLoaderModule()
  {
  }

  void ParticleLoaderModule::initialize()
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    StoreArray<PIDLikelihood> pidlikelihoods;
    StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;

    particles.registerInDataStore();
    extraInfoMap.registerInDataStore();
    //register relations if these things exists
    if (mcparticles.isOptional()) {
      particles.registerRelationTo(mcparticles);
    }
    if (pidlikelihoods.isOptional()) {
      particles.registerRelationTo(pidlikelihoods);
    }

    if (m_useMCParticles) {
      mcparticles.isRequired();
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
    StoreArray<Particle> particles;
    if (particles.isValid()) {
      B2INFO("ParticleLoader::loadFromFile size=" << particles.getEntries());
      B2WARNING("loadFromFile not fully implemented yet and may result in double counting!");
      return;
    }

    StoreObjPtr<ParticleExtraInfoMap> particleExtraInfoMap;
    if (not particleExtraInfoMap) {
      particleExtraInfoMap.create();
    }

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
    StoreArray<Particle> particles;

    unsigned int bitmask = MCParticle::c_PrimaryParticle;
    const int Npdg = 7;
    int pdgCode[Npdg] = {11, 13, 211, 321, 2212, 22, 130}; // e,mu,pi,K,p,gamma,K_L

    for (int i = 0; i < mcParticles.getEntries(); i++) {
      const MCParticle* mc = mcParticles[i];
      if (!mc->hasStatus(bitmask)) continue;
      for (int k = 0; k < Npdg; k++) {
        if (abs(mc->getPDG()) == pdgCode[k]) {
          Particle* newPart = particles.appendNew(mc);
          newPart->addRelationTo(mc);
          break;
        }
      }
    }
    //B2INFO("ParticleLoader::loadFromMCParticles size=" << particles.getEntries());
  }

  void ParticleLoaderModule::loadFromReconstruction()
  {
    StoreArray<Track> Tracks;
    StoreArray<ECLCluster> ECLClusters;
    StoreArray<KLMCluster> KLMClusters;
    StoreArray<V0> V0s;
    StoreArray<Particle> particles;

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
          Particle* newPart = particles.appendNew(particle);
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
        Particle* newPart = particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);
      }
    }

    // load reconstructed neutral KLM cluster's as Klongs
    for (int i = 0; i < KLMClusters.getEntries(); i++) {
      const KLMCluster* cluster      = KLMClusters[i];

      if (cluster->getAssociatedTrackFlag())
        continue;

      const MCParticle* mcParticle = cluster->getRelated<MCParticle>();

      Particle particle(cluster);

      if (particle.getParticleType() == Particle::c_KLMCluster) { // should always hold but...
        Particle* newPart = particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);
      }
    }

    // load reconstructed V0s as Kshorts (pi-pi+ combination), Lambdas (p+pi- combinations), and converted photons (e-e+ combinations)
    for (int i = 0; i < V0s.getEntries(); i++) {
      // TODO: make it const once V0 dataobject is corrected (const qualifier properly applied)
      V0* v0 = V0s[i];

      std::pair<Track*, Track*> v0Tracks = v0->getTrackPtrs();
      std::pair<TrackFitResult*, TrackFitResult*> v0TrackFitResults = v0->getTrackFitResultPtrs();

      // load Kshort -> pi- pi+
      Particle piP((v0Tracks.first)->getArrayIndex(), v0TrackFitResults.first, Const::pion);
      Particle piM((v0Tracks.second)->getArrayIndex(), v0TrackFitResults.second, Const::pion);

      const PIDLikelihood* pidP = (v0Tracks.first)->getRelated<PIDLikelihood>();
      const PIDLikelihood* pidM = (v0Tracks.second)->getRelated<PIDLikelihood>();

      const MCParticle* mcParticleP = (v0Tracks.first)->getRelated<MCParticle>();
      const MCParticle* mcParticleM = (v0Tracks.second)->getRelated<MCParticle>();

      // add V0 daughters to the Particle StoreArray
      Particle* newPiP = particles.appendNew(piP);
      if (pidP)
        newPiP->addRelationTo(pidP);
      if (mcParticleP)
        newPiP->addRelationTo(mcParticleP);

      Particle* newPiM = particles.appendNew(piM);
      if (pidM)
        newPiM->addRelationTo(pidM);
      if (mcParticleM)
        newPiM->addRelationTo(mcParticleM);

      TLorentzVector v0Momentum = newPiP->get4Vector() + newPiM->get4Vector();

      // TODO: avoid hard-coded values
      Particle v0P(v0Momentum, 310);
      v0P.appendDaughter(newPiP);
      v0P.appendDaughter(newPiM);

      particles.appendNew(v0P);
    }

    //B2INFO("ParticleLoader::loadFromReconstruction size=" << particles.getEntries());
  }
} // end Belle2 namespace

