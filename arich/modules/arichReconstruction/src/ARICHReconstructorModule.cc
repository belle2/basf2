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
#include <arich/dataobjects/ARICHDigit.h>



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

    std::vector<double> defMerit;
    defMerit.push_back(8.0);
    defMerit.push_back(8.0);
    defMerit.push_back(8.0);
    // Add parameters
    addParam("beamtest", m_beamtest, "ARICH beamtest switch (beamtest data=1, beamtest MC=2)", 0);
    addParam("trackPositionResolution", m_trackPositionResolution,
             "Resolution of track position on aerogel plane (for additional smearing of MC tracks)", 1.0 * Unit::mm);
    addParam("trackAngleResolution", m_trackAngleResolution,
             "Resolution of track direction angle on aerogel plane (for additional smearing of MC tracks)", 2.0 * Unit::mrad);
    addParam("backgroundLevel", m_backgroundLevel, "Background level in photon hits per m^2", 50.0);
    addParam("singleResolution", m_singleResolution, "Single photon resolution without pad", 0.010 * Unit::rad);
    addParam("aerogelMerit", m_aerogelMerit, "Aerogel figure of merit", defMerit);
    addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0), from AeroHits - MC info (1)", 0);
    addParam("storePhotons", m_storePhot, "Set to 1 to store reconstructed photon information (Ch. angle,...)", 0);
  }

  ARICHReconstructorModule::~ARICHReconstructorModule()
  {
    if (m_ana) delete m_ana;
  }

  void ARICHReconstructorModule::initialize()
  {
    // Initialize variables

    m_ana = new ARICHReconstruction(m_storePhot, m_beamtest);
    m_ana->setBackgroundLevel(m_backgroundLevel);
    m_ana->setTrackPositionResolution(m_trackPositionResolution);
    m_ana->setTrackAngleResolution(m_trackAngleResolution);
    m_ana->setSinglePhotonResolution(m_singleResolution);
    m_ana->setAerogelFigureOfMerit(m_aerogelMerit);
    m_ana->initialize();


    StoreArray<ARICHDigit> arichDigits;
    arichDigits.isRequired();

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
    arichTracks.registerInDataStore(DataStore::c_DontWriteOut);
    arichTracks.registerRelationTo(likelihoods, DataStore::c_Event, DataStore::c_DontWriteOut);

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
    StoreArray<ARICHDigit> arichDigits;

    // using track information form tracking system (mdst Track)
    if (m_inputTrackType == 0) {

      StoreArray<Track> Tracks;

      Const::EDetector myDetID = Const::EDetector::ARICH; // arich
      Const::ChargedStable hypothesis = Const::pion;
      int pdgCode = abs(hypothesis.getPDGCode());

      for (int itrk = 0; itrk < Tracks.getEntries(); ++itrk) {

        const Track* track = Tracks[itrk];
        const TrackFitResult* fitResult = track->getTrackFitResult(hypothesis);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << hypothesis.getPDGCode());
          continue;
        }

        const MCParticle* particle = track->getRelated<MCParticle>();

        ARICHAeroHit* aeroHit = NULL;
        if (particle) aeroHit = particle->getRelated<ARICHAeroHit>();

        RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
        ARICHTrack* arichTrack = NULL;
        const ExtHit* arichExtHit = NULL;
        for (unsigned i = 0; i < extHits.size(); i++) {
          const ExtHit* extHit = extHits[i];
          if (abs(extHit->getPdgCode()) != pdgCode) continue;
          if (extHit->getDetectorID() != myDetID) continue;
          if (extHit->getCopyID() != 12345) continue; // aerogel Al support plate
          if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
          if (extHit->getMomentum().Z() < 0.0) continue; // track passes in backward
          // if aeroHit cannot be found using MCParticle check if it was already related to the extHit (by ARICHRelate module)
          if (!aeroHit) aeroHit = extHit->getRelated<ARICHAeroHit>();

          // make new ARICHTrack
          arichTrack = arichTracks.appendNew(extHit);
          arichExtHit = extHit;
          break;
        } // extHits loop

        // skip if track has no extHit in ARICH
        if (!arichTrack) continue;

        // make new ARICHLikelihood
        ARICHLikelihood* like = arichLikelihoods.appendNew();
        // calculate and set likelihood values
        m_ana->likelihood2(*arichTrack, arichDigits, *like);
        // make relations
        track->addRelationTo(like);
        arichTrack->addRelationTo(like);
        like->addRelationTo(arichExtHit);
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
        // make associated ARICHLikelihood
        ARICHLikelihood* like = arichLikelihoods.appendNew();
        // calculate and set likelihood values
        m_ana->likelihood2(*arichTrack, arichDigits, *like);
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
    if (m_inputTrackType == 0) { B2INFO("ARICHReconstructorModule: track infromation is taken from mdst Tracks.");}
    else  B2INFO("ARICHReconstructorModule: track information is taken from MC (ARICHAeroHit).");
  }

} // namespace Belle2
