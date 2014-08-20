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
#include <arich/modules/arichReconstruction/ARICHTrack.h>


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
    m_ana(0),
    m_timeCPU(0),
    m_nRun(0),
    m_nEvent(0),
    m_storeHist(0),
    m_beamtest(0),
    m_file(NULL)
  {
    // Set description()
    setDescription("This module calculates the ARICHLikelihood values for all particle id. hypotheses, for all tracks that enter ARICH in the event.");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    std::vector<double> defMerit;
    defMerit.push_back(24.0);
    defMerit.push_back(24.0);
    defMerit.push_back(24.0);
    // Add parameters
    addParam("beamtest", m_beamtest, "ARICH beamtest switch (beamtest data=1, beamtest MC=2)", 0);
    addParam("storeHist", m_storeHist, "Store histograms with individual photon information (cherenkov angle distribution)", 0);
    addParam("inputAeroHits", m_inputAeroHits, "MCParticle hits on aerogel", string(""));
    addParam("inputTracks", m_inputTracks, "Mdst tracks", string(""));
    addParam("inputExtHits", m_inputExtHits, "ExtHits collection name", string(""));
    addParam("outputLikelihoods", m_outputLikelihoods, "ARICHLikelihoods collection name",  string(""));
    addParam("outfileName", m_outfileName, "File to store individual photon information",  string("thc.root"));
    addParam("trackPositionResolution", m_trackPositionResolution, "Resolution of track position on aerogel plane (for additional smearing of MC tracks)", 1.0 * Unit::mm);
    addParam("trackAngleResolution", m_trackAngleResolution, "Resolution of track direction angle on aerogel plane (for additional smearing of MC tracks)", 2.0 * Unit::mrad);
    addParam("backgroundLevel", m_backgroundLevel, "Background level in photon hits per m^2", 50.0);
    addParam("singleResolution", m_singleResolution, "Single photon resolution without pad", 0.010 * Unit::rad);
    addParam("aerogelMerit", m_aerogelMerit, "Aerogel figure of merit", defMerit);
    addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0), from AeroHits - MC info (1)", 0);
  }

  ARICHReconstructorModule::~ARICHReconstructorModule()
  {
    if (m_ana) delete m_ana;
  }

  void ARICHReconstructorModule::initialize()
  {
    // Initialize variables
    if (m_storeHist) m_file = new TFile(m_outfileName.c_str(), "RECREATE");
    m_nRun    = 0 ;
    m_nEvent  = 0 ;
    m_ana = new ARICHReconstruction(m_storeHist, m_beamtest);
    m_ana->setBackgroundLevel(m_backgroundLevel);
    m_ana->setTrackPositionResolution(m_trackPositionResolution);
    m_ana->setTrackAngleResolution(m_trackAngleResolution);
    m_ana->setSinglePhotonResolution(m_singleResolution);
    m_ana->setAerogelFigureOfMerit(m_aerogelMerit);
    // Print set parameters
    printModuleParams();

    // CPU time start
    m_timeCPU = clock() * Unit::us;

    StoreArray<ARICHLikelihood> likelihoods(m_outputLikelihoods);
    StoreArray<Track> tracks;
    StoreArray<ExtHit> extHits;
    StoreArray<ARICHAeroHit> aeroHits;

    likelihoods.registerInDataStore();

    tracks.registerRelationTo(likelihoods);
    extHits.registerRelationTo(aeroHits);
    aeroHits.registerRelationTo(likelihoods);

  }

  void ARICHReconstructorModule::beginRun()
  {
    // Print run number
    B2INFO("ARICHReconstruction: Processing run: " << m_nRun);
  }

  void ARICHReconstructorModule::event()
  {

    // Output - log likelihoods
    StoreArray<ARICHLikelihood> arichLikelihoods(m_outputLikelihoods);
    arichLikelihoods.create();

    // input AeroHits
    StoreArray<ARICHAeroHit> arichAeroHits(m_inputAeroHits);

    // track information from CDC reconstructed tracks
    if (m_inputTrackType == 0) {

      B2DEBUG(100, "New part of ARICHReconstructorModule::event");

      // Input: reconstructed tracks
      StoreArray<Track> mdstTracks(m_inputTracks);

      std::vector<ARICHTrack> arichTracks;
      getTracks(arichTracks, Const::pion);
      B2DEBUG(100, "Number of tracks from ext" << arichTracks.size());
      if (arichTracks.empty()) return;

      m_ana->likelihood2(arichTracks);

      int nTracks = arichTracks.size();
      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHTrack* track = &arichTracks[iTrack];
        double likelihoods[5];
        double expectedPhotons[5];
        int detectedPhotons[5];
        track->getLikelihood(likelihoods);
        track->getExpectedPhotons(expectedPhotons);
        track->getDetectedPhotons(detectedPhotons);

        B2DEBUG(50, "Number of expected photons " << expectedPhotons[0]);
        B2DEBUG(50, "Number of detected photons " << detectedPhotons);

        ARICHLikelihood* like = arichLikelihoods.appendNew(track->getFlag(), likelihoods, detectedPhotons, expectedPhotons);

        // Add relations
        const Track* mdstTrack =  mdstTracks[track->getTrackID()];
        mdstTrack->addRelationTo(like);

        if (track->getAeroIndex() > -1) {
          const ARICHAeroHit* aeroHit = arichAeroHits[track->getAeroIndex()];
          aeroHit->addRelationTo(like);
        } else {
          B2DEBUG(50, "No AeroHit for Track " << track->getTrackID());
        }
      }

      // trackToArich.consolidate();
      // aeroHitToArich.consolidate();

    }
    // track information from ARICHAeroHits
    else {

      std::vector<ARICHTrack> arichTracks;

      // Get number of hits in this event
      int nTracks = arichAeroHits.getEntries();

      // Loop over all ARICHAeroHits
      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHAeroHit* aeroHit = arichAeroHits[iTrack];
        arichTracks.push_back(ARICHTrack(*aeroHit));
      } // for iTrack

      // apply smearing of track position and detector parameters (to mimic tracking resolution)
      m_ana->smearTracks(arichTracks);

      nTracks = arichTracks.size();
      m_ana->likelihood2(arichTracks);

      // build the relations (ARICHAeroHit-ARICHLikelihood))

      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHTrack* track = &arichTracks[iTrack];
        double expectedPhotons[5];
        double likelihoods[5];
        int detectedPhotons[5];
        track->getExpectedPhotons(expectedPhotons);
        track->getLikelihood(likelihoods);
        track->getDetectedPhotons(detectedPhotons);

        ARICHLikelihood* like = arichLikelihoods.appendNew(track->getFlag(), likelihoods, detectedPhotons, expectedPhotons);

        if (track->getAeroIndex() > -1) {
          const ARICHAeroHit* aeroHit = arichAeroHits[track->getAeroIndex()];
          aeroHit->addRelationTo(like);
        } else {
          B2DEBUG(50, "No AeroHit for Track " << track->getTrackID());
        }
      }
    }
    m_nEvent++;
  }

  void ARICHReconstructorModule::endRun()
  {
    m_nRun++;
    if (m_file) {
      m_file->Write();
      m_file->Close();
    }
  }

  void ARICHReconstructorModule::terminate()
  {
    // CPU time end
    m_timeCPU = clock() * Unit::us - m_timeCPU;
    // Announce
    B2INFO("ARICHReconstructorModule finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

  }

  void ARICHReconstructorModule::printModuleParams() const
  {
    B2INFO("ARICHReconstructorModule parameters:")
    B2INFO("Input tracks switch: " << m_inputTrackType);
  }

  void ARICHReconstructorModule::getTracks(std::vector<ARICHTrack>& tracks,
                                           Const::ChargedStable hypothesis)
  {
    Const::EDetector myDetID = Const::EDetector::ARICH; // arich
    int pdgCode = abs(hypothesis.getPDGCode());

    StoreArray<Track> Tracks(m_inputTracks);

    for (int itra = 0; itra < Tracks.getEntries(); ++itra) {
      const Track* track = Tracks[itra];
      const TrackFitResult* fitResult = track->getTrackFitResult(hypothesis);
      if (!fitResult) {
        B2ERROR("No TrackFitResult for " << hypothesis.getPDGCode());
        continue;
      }
      int charge = fitResult->getChargeSign();
      B2DEBUG(50, "Track.Charge " << charge);
      const MCParticle* particle = track->getRelated<MCParticle>(m_inputTracks);
      ARICHAeroHit* aeroHit = particle->getRelated<ARICHAeroHit>(m_inputAeroHits);

      int aeroHitIndex = -1;
      int truePDGCode = 0;

      RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);

      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != pdgCode) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getCopyID() != 12345) continue; // aerogel Al support plate
        if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
        B2DEBUG(100, "getTracks: z = " << extHit->getPosition().Z());
        // if no MCParticle information is available, try to get AeroHit from its relation to ExtHit (created with ARICHRelate module)
        bool from_rel = false;
        if (!aeroHit) {aeroHit = extHit->getRelated<ARICHAeroHit>(m_inputAeroHits); from_rel = true;};
        if (aeroHit) {
          aeroHitIndex = aeroHit->getArrayIndex(); truePDGCode = aeroHit->getPDG();
          if (!from_rel)  extHit->addRelationTo(aeroHit);
        }
        ARICHTrack trk(extHit, charge, truePDGCode, (int)itra, aeroHitIndex);
        tracks.push_back(trk);
      }
    }
  }

} // namespace Belle2
