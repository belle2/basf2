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
#include <arich/modules/arichMCParticles/ARICHMCParticlesModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
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

  REG_MODULE(ARICHMCParticles)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHMCParticlesModule::ARICHMCParticlesModule()
  {
    // set module description (e.g. insert text)
    setDescription("Creates collection of MCParticles related to tracks that hit ARICH.");
    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

  }

  ARICHMCParticlesModule::~ARICHMCParticlesModule()
  {
  }

  void ARICHMCParticlesModule::initialize()

  {
    // Dependecies check
    m_tracks.isRequired();
    m_extHits.isRequired();

    m_arichMCPs.registerInDataStore();
    m_tracks.registerRelationTo(m_arichMCPs);

  }

  void ARICHMCParticlesModule::beginRun()
  {
  }

  void ARICHMCParticlesModule::event()
  {
    Const::EDetector myDetID = Const::EDetector::ARICH; // arich
    Const::ChargedStable hypothesis = Const::pion;
    int pdgCode = abs(hypothesis.getPDGCode());

    for (int itrk = 0; itrk < m_tracks.getEntries(); ++itrk) {

      const Track* track = m_tracks[itrk];
      const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(hypothesis);
      if (!fitResult) {
        B2ERROR("No TrackFitResult for " << hypothesis.getPDGCode());
        continue;
      }

      const MCParticle* particle = track->getRelated<MCParticle>();
      if (!particle) continue;

      RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);

      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != pdgCode) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
        if (extHit->getMomentum().Z() < 0.0) continue; // track passes in backward
        if (extHit->getCopyID() == 6789) {
          //MCParticle arichMCP = *particle;
          MCParticle* arichP = m_arichMCPs.appendNew(*particle);
          track->addRelationTo(arichP);

        }

      }
    }

  }


  void ARICHMCParticlesModule::endRun()
  {
  }

  void ARICHMCParticlesModule::terminate()
  {
  }

  void ARICHMCParticlesModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

