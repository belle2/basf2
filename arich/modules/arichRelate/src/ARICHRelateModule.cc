/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <arich/modules/arichRelate/ARICHRelateModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHAeroHit.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  ///                Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHRelate);

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

  void ARICHRelateModule::initialize()

  {
    // Dependecies check
    m_mcParticles.isRequired();
    m_mdstTracks.isRequired();
    m_aeroHits.isRequired();
    m_extHits.isRequired();
    m_extHits.registerRelationTo(m_aeroHits);
  }

  void ARICHRelateModule::event()
  {
    int nHits = m_aeroHits.getEntries();
    B2DEBUG(50, "No. of hits " << nHits);

    for (int iHit = 0; iHit < nHits; ++iHit) {
      const ARICHAeroHit* aeroHit = m_aeroHits[iHit];
      const MCParticle* particle = DataStore::getRelated<MCParticle>(aeroHit);
      if (!particle) {
        B2DEBUG(50, "No MCParticle for AeroHit!");
        continue;
      }

      // Find the track produced by MCParticle
      const Track* track = DataStore::getRelated<Track>(particle);
      if (!track) continue;

      const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(Const::pion);
      if (!fitResult) continue;

      RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
      Const::EDetector myDetID = Const::EDetector::ARICH;
      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != Const::pion.getPDGCode()) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getCopyID() != 6789) continue; // aerogel Al support plate
        if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
        extHit->addRelationTo(aeroHit);
        break;
      }
    }
  }

} // end Belle2 namespace

