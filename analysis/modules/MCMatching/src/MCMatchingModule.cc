/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/MCMatching/MCMatchingModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// utility
#include <analysis/utility/mcParticleMatching.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(MCMatching)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  MCMatchingModule::MCMatchingModule() : Module()
  {
    setDescription("Module performs MC matching (sets relation Particle<->MCParticle) for all particles (and its (grand)^N-daughter particles) in the ParticleList.");

    addParam("ListName", m_listName, "Name of the input ParticleList.", string(""));
  }


  MCMatchingModule::~MCMatchingModule()
  {
  }

  void MCMatchingModule::initialize()
  {
    StoreArray<Particle>::required();
    StoreArray<MCParticle>::required();
    RelationArray::registerPersistent<Particle, MCParticle>();
  }

  void MCMatchingModule::beginRun()
  {
  }

  void MCMatchingModule::event()
  {

    StoreObjPtr<ParticleList> plist(m_listName);
    if (!plist) {
      B2ERROR("ParticleList " << m_listName << " not found");
      return;
    }

    for (unsigned i = 0; i < plist->getListSize(); i++) {
      const Particle* part = plist->getParticle(i);

      setMCTruth(part);
    }
  }

  void MCMatchingModule::endRun()
  {
  }

  void MCMatchingModule::terminate()
  {
  }

} // end Belle2 namespace
