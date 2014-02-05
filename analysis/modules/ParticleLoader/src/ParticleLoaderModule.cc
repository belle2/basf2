/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleLoader/ParticleLoaderModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <ecl/dataobjects/ECLShower.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

#include <utility>

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
    // set module description (e.g. insert text)
    setDescription("Loads mdst particles to StoreArray<Particle>");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("UseMCParticles", m_useMCParticles, "use MCParticles", false);
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
    if (m_useMCParticles) {loadFromMCParticles();}
    else {loadFromReconstruction();}

  }


  void ParticleLoaderModule::endRun()
  {
  }

  void ParticleLoaderModule::terminate()
  {
  }

  void ParticleLoaderModule::printModuleParams() const
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
          Particle particle(mc);
          Particle* newPart = Particles.appendNew(particle);
          DataStore::addRelationFromTo(newPart, mc);
          break;
        }
      }
    }
    B2INFO("ParticleLoader::loadFromMCParticles size=" << Particles.getEntries());
  }

  void ParticleLoaderModule::loadFromReconstruction()
  {
    StoreArray<Track> Tracks;
    StoreArray<ECLGamma> Gammas;
    StoreArray<ECLPi0> Pi0s;
    StoreArray<ECLShower> ECLShowers;
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
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(track);
      const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(track);
      for (int k = 0; k < 5; k++) {
        Particle particle(track, charged[k], i);
        if (particle.getParticleType() == Particle::c_Track) { // should always hold but...
          Particle* newPart = Particles.appendNew(particle);
          DataStore::addRelationFromTo(newPart, pid);
          DataStore::addRelationFromTo(newPart, mcParticle);
        }
      }
    }

    // load reconstructed gamma's

    vector< pair<int, int> > gammaShowerId; // needed to set pi0 daughters

    for (int i = 0; i < Gammas.getEntries(); i++) {
      const ECLGamma* gamma = Gammas[i];
      const ECLShower* eclshower   = DataStore::getRelated<ECLShower>(gamma);
      const MCParticle* mcParticle = DataStore::getRelated<MCParticle>(eclshower);
      Particle particle(gamma, i);
      if (particle.getParticleType() == Particle::c_ECLGamma) { // should always hold but...
        Particle* newPart = Particles.appendNew(particle);
        DataStore::addRelationFromTo(newPart, mcParticle);
        int lastIndex = Particles.getEntries() - 1;
        int showerId = gamma->getShowerId();
        gammaShowerId.push_back(make_pair(lastIndex, showerId));
      }
    }

    // load reconstructed pi0's

    for (int i = 0; i < Pi0s.getEntries(); i++) {
      const ECLPi0* pi0 = Pi0s[i];
      Particle particle(pi0, i);
      if (particle.getParticleType() == Particle::c_Pi0) { // should always hold but...
        int showerId1 = pi0->getShowerId1();
        int showerId2 = pi0->getShowerId2();
        //find corresponding gamma Particles from shower ID
        for (unsigned k = 0; k < gammaShowerId.size(); k++) {
          int showerId = gammaShowerId[k].second;
          if (showerId == showerId1 || showerId == showerId2) {
            particle.appendDaughter(gammaShowerId[k].first);
          }
        }
        Particle* newPart = Particles.appendNew(particle);

        const MCParticle* gammaMc1 = ECLShowers[showerId1]->getRelated<MCParticle>();
        const MCParticle* gammaMc2 = ECLShowers[showerId2]->getRelated<MCParticle>();
        if (gammaMc1 and gammaMc2 and gammaMc1->getMother() == gammaMc2->getMother()) {
          //both ECLShowers have same mother, save MC info
          newPart->addRelationTo(gammaMc1->getMother());
        }
      }
    }

    B2INFO("ParticleLoader::loadFromReconstruction size=" << Particles.getEntries());

  }


} // end Belle2 namespace

