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
#include <top/modules/TOPMCTrackMaker/TOPMCTrackMakerModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/BFieldManager.h>

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPMCTrackMaker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPMCTrackMakerModule::TOPMCTrackMakerModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Constructs Tracks and ExtHits from MCParticles and TOPBarHits. "
                   "Utility needed for testing and debugging of TOP reconstruction.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters

  }

  TOPMCTrackMakerModule::~TOPMCTrackMakerModule()
  {
  }

  void TOPMCTrackMakerModule::initialize()
  {

    // input

    StoreArray<MCParticle> mcParticles;
    mcParticles.isRequired();

    StoreArray<TOPBarHit> barHits;
    barHits.isRequired();

    // output

    StoreArray<Track> tracks;
    tracks.registerInDataStore();

    StoreArray<TrackFitResult> fitResults;
    fitResults.registerInDataStore();

    StoreArray<ExtHit> extHits;
    extHits.registerInDataStore();

    tracks.registerRelationTo(mcParticles);
    tracks.registerRelationTo(extHits);

  }

  void TOPMCTrackMakerModule::beginRun()
  {
  }

  void TOPMCTrackMakerModule::event()
  {

    StoreArray<MCParticle> mcParticles;
    StoreArray<TOPBarHit> barHits;

    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> fitResults;
    StoreArray<ExtHit> extHits;

    for (const auto& mcParticle : mcParticles) {
      if (mcParticle.getStatus(MCParticle::c_PrimaryParticle) == 0) continue;
      if (mcParticle.getCharge() == 0) continue;
      const auto* barHit = mcParticle.getRelated<TOPBarHit>();
      if (!barHit) continue;

      TMatrixDSym cov(6); // infinite precission
      fitResults.appendNew(mcParticle.getVertex(),
                           mcParticle.getMomentum(),
                           cov,
                           mcParticle.getCharge(),
                           Const::pion,
                           1.0,            // pValue
                           BFieldManager::getField(0, 0, 0).Z() / Unit::T,
                           0x38FFFFFFFFFFFFFF, // 56 hits, in all CDC layers
                           0, 56 - 5); // NDF = 56-5
      auto* track = tracks.appendNew();
      track->setTrackFitResultIndex(Const::pion, fitResults.getEntries() - 1);
      track->addRelationTo(&mcParticle);

      const Const::ChargedStable& chargedStable = Const::pion;
      double pmom = barHit->getMomentum().Mag();
      double mass = chargedStable.getMass();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      double tof = barHit->getLength() / beta / Const::speedOfLight;
      auto* extHit = extHits.appendNew(chargedStable.getPDGCode(),
                                       Const::TOP,
                                       barHit->getModuleID(),
                                       EXT_ENTER,
                                       false,
                                       tof,
                                       barHit->getPosition(),
                                       barHit->getMomentum(),
                                       cov);
      track->addRelationTo(extHit);
    }

  }


  void TOPMCTrackMakerModule::endRun()
  {
  }

  void TOPMCTrackMakerModule::terminate()
  {
  }


} // end Belle2 namespace

