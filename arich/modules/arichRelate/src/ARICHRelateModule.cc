/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dino Tahirovic, Luka Santelj                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichRelate/ARICHRelateModule.h>



#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHAeroHit.h>

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

  REG_MODULE(ARICHRelate)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHRelateModule::ARICHRelateModule()
  {
    // set module description (e.g. insert text)
    setDescription("Creates relations between ARICHAeroHits and ExtHits. Allows to store simulation output without MCParticles");
    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

  }

  ARICHRelateModule::~ARICHRelateModule()
  {
  }

  void ARICHRelateModule::initialize()

  {
    // Dependecies check
    StoreArray<MCParticle>::required();
    StoreArray<ARICHAeroHit>::required();
    StoreArray<Track>::required();
    StoreArray<ExtHit>::required();

    // Prepare the relation matrix for write access
    StoreArray<ARICHAeroHit> aeroHits;
    StoreArray<ExtHit> extHits;
    extHits.registerRelationTo(aeroHits);
  }

  void ARICHRelateModule::beginRun()
  {
  }

  void ARICHRelateModule::event()
  {
    // Input: reconstructed tracks
    StoreArray<MCParticle> MCParticles("");
    StoreArray<Track> mdstTracks("");
    StoreArray<ExtHit> extHits("");
    StoreArray<ARICHAeroHit> aeroHits("");

    int nHits = aeroHits.getEntries();
    B2DEBUG(50, "No. of hits " << nHits);

    for (int iHit = 0; iHit < nHits; ++iHit) {
      const ARICHAeroHit* aeroHit = aeroHits[iHit];
      const MCParticle* particle = DataStore::getRelated<MCParticle>(aeroHit);
      if (!particle) {
        B2DEBUG(50, "No MCParticle for AeroHit!");
        continue;
      }

      int pdg = 211;

      // Find the track produced by MCParticle
      const Track* track = DataStore::getRelated<Track>(particle);
      if (!track) continue;

      const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
      if (!fitResult) continue;

      RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
      Const::EDetector myDetID = Const::EDetector::ARICH;
      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != pdg) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getCopyID() != 6789) continue; // aerogel Al support plate
        if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
        extHit->addRelationTo(aeroHit);
        break;
      }
    }
  }


  void ARICHRelateModule::endRun()
  {
  }

  void ARICHRelateModule::terminate()
  {
  }

  void ARICHRelateModule::printModuleParams() const
  {
  }


} // end Belle2 namespace

