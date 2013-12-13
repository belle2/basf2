/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/ModuleManager.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <analysis/dataobjects/ParticleInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/modules/ParticleInfo/ParticleInfoModule.h>


using namespace std;

namespace Belle2 {

  REG_MODULE(ParticleInfo)

  ParticleInfoModule::ParticleInfoModule() : Module()
  {
    setDescription("Creates a ParticleInfo and relates it with given Particles.");
    setPropertyFlags(c_ParallelProcessingCertified);

    vector<string> defaultList;
    addParam("InputListNames", m_inputListNames, "list of ParticleList names", defaultList);

  }

  ParticleInfoModule::~ParticleInfoModule()
  {
  }

  void ParticleInfoModule::initialize()
  {
    StoreArray<ParticleInfo>::registerPersistent();
    RelationArray::registerPersistent<Particle, ParticleInfo>();
  }

  void ParticleInfoModule::beginRun()
  {
  }

  void ParticleInfoModule::endRun()
  {
  }

  void ParticleInfoModule::terminate()
  {
  }

  void ParticleInfoModule::event()
  {
    // If StoreArray<ParticleInfo> doesn't exist yet, create it
    StoreArray<ParticleInfo> particleInfos;
    if (not particleInfos.isValid()) {
      particleInfos.create();
    }

    // Loop over all given ParticleLists
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList> list(m_inputListNames[i]);
      // Loop over all Particles and Anti-Particles in the current ParticleList
      for (unsigned i = 0; i < list->getNumofParticles() + list->getNumofAntiParticles(); ++i) {
        const Particle* particle = list->getParticle(i);
        if (particle == nullptr) {
          B2ERROR("Encountered nullptr in ParticleList");
        }
        // Append new particleInfo to the StoreArray<ParticleInfo> object and relate the current particle to it.
        ParticleInfo* particleInfo = particleInfos.appendNew();
        if (particleInfo == nullptr) {
          B2ERROR("Failed to append new Particle Info to StoreArray");
        }
        DataStore::addRelationFromTo(particle, particleInfo);
      }
    }
  }


  void ParticleInfoModule::printModuleParams() const
  {
  }


} // Belle2 namespace

