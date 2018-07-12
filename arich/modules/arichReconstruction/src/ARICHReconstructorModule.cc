/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Dino Tahirovic                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include

#include <arich/modules/arichReconstruction/ARICHReconstructorModule.h>
#include <time.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHHit.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;

namespace Belle2 {

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
  REG_MODULE(ARICHReconstructor)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  ARICHReconstructorModule::ARICHReconstructorModule() :
    m_ana(NULL)
  {
    // Set description()
    setDescription("This module calculates the ARICHLikelihood values for all particle id. hypotheses, for all tracks that enter ARICH in the event.");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("trackPositionResolution", m_trackPositionResolution,
             "Resolution of track position on aerogel plane (for additional smearing of MC tracks)", 1.0 * Unit::mm);
    addParam("trackAngleResolution", m_trackAngleResolution,
             "Resolution of track direction angle on aerogel plane (for additional smearing of MC tracks)", 2.0 * Unit::mrad);
    addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0), from AeroHits - MC info (1)", 0);
    addParam("storePhotons", m_storePhot, "Set to 1 to store reconstructed photon information (Ch. angle,...)", 0);
    addParam("useAlignment", m_align, "Use ARICH position alignment constatns", false);
  }

  ARICHReconstructorModule::~ARICHReconstructorModule()
  {
    if (m_ana) delete m_ana;
  }

  void ARICHReconstructorModule::initialize()
  {
    // Initialize variables

    m_ana = new ARICHReconstruction(m_storePhot);
    m_ana->setTrackPositionResolution(m_trackPositionResolution);
    m_ana->setTrackAngleResolution(m_trackAngleResolution);
    m_ana->initialize();


    StoreArray<ARICHHit> arichHits;
    arichHits.isRequired();

    StoreArray<Track> tracks;
    StoreArray<ExtHit> extHits;
    StoreArray<ARICHAeroHit> aeroHits;
    tracks.isOptional();
    extHits.isOptional();
    aeroHits.isOptional();

    if (!aeroHits.isOptional()) {
      tracks.isRequired();
      extHits.isRequired();
    }

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    StoreArray<ARICHLikelihood> likelihoods;
    likelihoods.registerInDataStore();

    likelihoods.registerRelationTo(extHits);
    likelihoods.registerRelationTo(aeroHits);
    tracks.registerRelationTo(likelihoods);

    StoreArray<ARICHTrack> arichTracks;
    //arichTracks.registerInDataStore(DataStore::c_DontWriteOut);
    //arichTracks.registerRelationTo(likelihoods, DataStore::c_Event, DataStore::c_DontWriteOut);
    arichTracks.registerInDataStore();
    arichTracks.registerRelationTo(likelihoods);
    printModuleParams();
  }

  void ARICHReconstructorModule::beginRun()
  {
  }

  void ARICHReconstructorModule::event()
  {

    // Output - log likelihoods
    StoreArray<ARICHLikelihood> arichLikelihoods;

    // input AeroHits
    StoreArray<ARICHTrack> arichTracks;

    // Input: ARICHDigits
    StoreArray<ARICHHit> arichHits;

    // using track information form tracking system (mdst Track)
    if (m_inputTrackType == 0) {

      StoreArray<Track> Tracks;

      Const::EDetector myDetID = Const::EDetector::ARICH; // arich
      Const::ChargedStable hypothesis = Const::pion;
      int pdgCode = abs(hypothesis.getPDGCode());

      for (int itrk = 0; itrk < Tracks.getEntries(); ++itrk) {

        const Track* track = Tracks[itrk];
        const TrackFitResult* fitResult = track->getTrackFitResultWithClosestMass(hypothesis);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << hypothesis.getPDGCode());
          continue;
        }

        const MCParticle* particle = track->getRelated<MCParticle>();

        ARICHAeroHit* aeroHit = NULL;
        if (particle) aeroHit = particle->getRelated<ARICHAeroHit>();

        RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
        ARICHTrack* arichTrack = NULL;

        //const ExtHit* arich1stHit = NULL;
        const ExtHit* arich2ndHit = NULL;
        const ExtHit* arichWinHit = NULL;

        for (unsigned i = 0; i < extHits.size(); i++) {
          const ExtHit* extHit = extHits[i];
          if (abs(extHit->getPdgCode()) != pdgCode) continue;
          if (extHit->getDetectorID() != myDetID) continue;
          if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
          if (extHit->getMomentum().Z() < 0.0) continue; // track passes in backward
          if (extHit->getCopyID() == 12345) { continue;}
          if (extHit->getCopyID() == 6789) {  arich2ndHit = extHit; continue;}
          arichWinHit = extHit;
        }

        if (arich2ndHit) {
          // if aeroHit cannot be found using MCParticle check if it was already related to the extHit (by ARICHRelate module)
          if (!aeroHit) aeroHit = arich2ndHit->getRelated<ARICHAeroHit>();

          // make new ARICHTrack
          arichTrack = arichTracks.appendNew(arich2ndHit);
          if (arichWinHit) arichTrack->setHapdWindowHit(arichWinHit);
        }

        // skip if track has no extHit in ARICH
        if (!arichTrack) continue;
        // transform track parameters to ARICH local frame
        m_ana->transformTrackToLocal(*arichTrack, m_align);
        // make new ARICHLikelihood
        ARICHLikelihood* like = arichLikelihoods.appendNew();
        // calculate and set likelihood values
        m_ana->likelihood2(*arichTrack, arichHits, *like);
        // make relations
        track->addRelationTo(like);
        arichTrack->addRelationTo(like);
        like->addRelationTo(arich2ndHit);
        if (aeroHit) like->addRelationTo(aeroHit);

      } // Tracks loop
    } // input type if


    // using track information form MC (stored in ARICHAeroHit)
    else {

      StoreArray<ARICHAeroHit> aeroHits;
      int nTracks = aeroHits.getEntries();

      // Loop over all ARICHAeroHits
      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHAeroHit* aeroHit = aeroHits[iTrack];

        // make new ARICHTrack
        ARICHTrack* arichTrack = arichTracks.appendNew(aeroHit);
        // smearing of track parameters (to mimic tracking system resolutions)
        m_ana->smearTrack(*arichTrack);
        // transform track parameters to ARICH local frame
        m_ana->transformTrackToLocal(*arichTrack, m_align);
        // make associated ARICHLikelihood
        ARICHLikelihood* like = arichLikelihoods.appendNew();
        // calculate and set likelihood values
        m_ana->likelihood2(*arichTrack, arichHits, *like);
        // make relation
        arichTrack->addRelationTo(like);
        like->addRelationTo(aeroHit);
      } // for iTrack

    }
  }

  void ARICHReconstructorModule::endRun()
  {
  }

  void ARICHReconstructorModule::terminate()
  {
  }

  void ARICHReconstructorModule::printModuleParams()
  {
    if (m_inputTrackType == 0) { B2DEBUG(100, "ARICHReconstructorModule: track infromation is taken from mdst Tracks.");}
    else  B2DEBUG(100, "ARICHReconstructorModule: track information is taken from MC (ARICHAeroHit).");
  }

} // namespace Belle2
