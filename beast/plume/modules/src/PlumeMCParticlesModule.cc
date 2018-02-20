/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <beast/plume/modules/PlumeMCParticlesModule.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>

#include <mdst/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(PLUMEMCParticles)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  PLUMEMCParticlesModule::PLUMEMCParticlesModule()
  {
    // set module description (e.g. insert text)
    setDescription("Creates collection of MCParticles related to tracks that hit ARICH.");
    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

  }

  PLUMEMCParticlesModule::~PLUMEMCParticlesModule()
  {
  }

  void PLUMEMCParticlesModule::initialize()

  {
    // Prepare the relation matrix for write access
    StoreArray<MCParticle> plumeMCP("plumeMCParticles");
    plumeMCP.registerInDataStore();

    StoreArray<PlumeSimHit> simHits;
    simHits.registerRelationTo(plumeMCP);
  }

  void PLUMEMCParticlesModule::beginRun()
  {
  }

  void PLUMEMCParticlesModule::event()
  {
    // Input: reconstructed tracks
    StoreArray<PlumeSimHit> simHits;
    StoreArray<MCParticle> plumeMCP("plumeMCParticles");

    for (int ihit = 0; ihit < simHits.getEntries(); ++ihit) {

      const PlumeSimHit* hit = simHits[ihit];

      const MCParticle* particle = hit->getRelated<MCParticle>();
      if (!particle) continue;

      MCParticle* plumeP = plumeMCP.appendNew(*particle);
      hit->addRelationTo(plumeP);

    }

  }


  void PLUMEMCParticlesModule::endRun()
  {
  }

  void PLUMEMCParticlesModule::terminate()
  {
  }

  void PLUMEMCParticlesModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

