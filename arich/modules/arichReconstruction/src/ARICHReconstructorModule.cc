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
  namespace arich {

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
      m_beamtest(0),
      m_file(NULL)
    {
      // Set description()
      setDescription("ARICHReconstructor");
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
      std::vector<double> defMerit;
      defMerit.push_back(30.0);
      defMerit.push_back(30.0);
      defMerit.push_back(30.0);
      // Add parameters
      addParam("beamtest", m_beamtest, "ARICH beamtest switch (beamtest>=1)", 0);
      addParam("MCColName", m_mcColName, "MCParticles collection name", string(""));
      addParam("tracksColName", m_tracksColName, "Tracks collection name", string(""));
      addParam("extHitsColName", m_extHitsColName, "ExtHits collection name", string(""));
      addParam("outColName", m_outColName, "ARICHLikelihoods collection name",  string(""));
      addParam("outfileName", m_outfileName, "File to store single photon information",  string("beamtest.root"));
      addParam("trackPositionResolution", m_trackPositionResolution, "Resolution of track position on aerogel plane", 1.0 * Unit::mm);
      addParam("trackAngleResolution", m_trackAngleResolution, "Resolution of track direction angle on aerogel plane", 1.0 * Unit::mrad);
      addParam("backgroundLevel", m_backgroundLevel, "Background level in photon hits per m^2", 50.0);
      addParam("singleResolution", m_singleResolution, "Single photon resolution without pad", 0.010 * Unit::rad);
      addParam("aerogelMerit", m_aerogelMerit, "Aerogel figure of merit", defMerit);
      addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0), tracks from AeroHits (1), AeroHits (2)", 0);
    }

    ARICHReconstructorModule::~ARICHReconstructorModule()
    {
      if (m_ana) delete m_ana;
    }

    void ARICHReconstructorModule::initialize()
    {
      // Initialize variables
      if (m_beamtest) m_file = new TFile(m_outfileName.c_str(), "RECREATE");
      m_nRun    = 0 ;
      m_nEvent  = 0 ;
      m_ana = new ARICHReconstruction(m_beamtest);
      m_ana->setBackgroundLevel(m_backgroundLevel);
      m_ana->setTrackPositionResolution(m_trackPositionResolution);
      m_ana->setTrackAngleResolution(m_trackAngleResolution);
      m_ana->setSinglePhotonResolution(m_singleResolution);
      m_ana->setAerogelFigureOfMerit(m_aerogelMerit);
      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      StoreArray<ARICHLikelihood>::registerPersistent(m_outColName);
      RelationArray::registerPersistent<ARICHAeroHit, ARICHLikelihood>(m_mcColName, m_outColName);
      RelationArray::registerPersistent<Track, ARICHLikelihood>(m_tracksColName, m_outColName);
      RelationArray::registerPersistent<ARICHAeroHit, ExtHit>(m_mcColName, m_extHitsColName);
    }

    void ARICHReconstructorModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHReconstruction: Processing run: " << m_nRun);
    }

    void ARICHReconstructorModule::event()
    {
      if (m_inputTrackType == 0) {

        B2DEBUG(100, "New part of ARICHReconstructorModule::event");

        // Input: reconstructed tracks
        StoreArray<Track> mdstTracks(m_tracksColName);
        StoreArray<ExtHit> extHits(m_extHitsColName);
        StoreArray<ARICHAeroHit> arichAeroHits(m_mcColName);

        // Output - likelihoods
        StoreArray<ARICHLikelihood> arichLikelihoods(m_outColName);
        if (!arichLikelihoods.isValid()) arichLikelihoods.create();

        // Output - relations
        RelationArray trackToArich(mdstTracks, arichLikelihoods);
        trackToArich.clear();
        RelationArray aeroHitToExt(arichAeroHits, extHits);
        aeroHitToExt.clear();
        RelationArray aeroHitToArich(arichAeroHits, arichLikelihoods);
        aeroHitToArich.clear();

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
          track->getLikelihood(likelihoods);
          track->getExpectedPhotons(expectedPhotons);
          double detectedPhotons = track->getDetectedPhotons();
          B2DEBUG(50, "Number of expected photons " << expectedPhotons[0]);
          B2DEBUG(50, "Number of detected photons " << detectedPhotons);

          new(arichLikelihoods.nextFreeAddress()) ARICHLikelihood(1, likelihoods, detectedPhotons, expectedPhotons);

          // Add relations
          int last = arichLikelihoods.getEntries() - 1;
          trackToArich.add(track->getTrackID(), last);
          int aeroIndex = track->getAeroIndex();
          if (aeroIndex >= 0) {
            aeroHitToArich.add(aeroIndex, last);
            aeroHitToExt.add(aeroIndex, track->getHitID());
          } else {
            B2DEBUG(50, "No AeroHit for Track " << track->getTrackID());
          }
        }

        trackToArich.consolidate();
        aeroHitToExt.consolidate();
        aeroHitToArich.consolidate();

      } else {
        //------------------------------------------------------
        // Get the collection of ARICHSimHits from the DataStore.
        // For inputTrackType=2 this assumes existence of
        // ExtHit-ARICHAeroHit relation
        //------------------------------------------------------

        StoreArray<ARICHAeroHit> arichAeroHits(m_mcColName);
        StoreArray<ExtHit> extHits(m_extHitsColName);

        // Output: ARICH likelihoods
        StoreArray<ARICHLikelihood> arichLikelihoods(m_outColName);
        arichLikelihoods.create();

        // Output: AeroHits to ARICH likelihoods relations
        RelationArray  aeroHitToArich(arichAeroHits, arichLikelihoods);
        aeroHitToArich.clear();

        std::vector<ARICHTrack> arichTracks;

        // Get number of hits in this event
        int nTracks = arichAeroHits.getEntries();

        // Loop over all ARICHAeroHits
        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHAeroHit* aeroHit = arichAeroHits[iTrack];
          if (m_inputTrackType == 2) { arichTracks.push_back(ARICHTrack(*aeroHit)); continue;}

          ExtHit* extHit = DataStore::getRelated<ExtHit>(aeroHit);
          if (extHit) {
            ARICHTrack trk(extHit,  aeroHit->getPDG() > 0 ? 1 : -1, aeroHit->getPDG(), (int)iTrack, aeroHit->getArrayIndex());
            arichTracks.push_back(trk);
          }
        } // for iTrack

        // if tracks from ARICHAeroHits apply smearing of track parameters
        if (m_inputTrackType == 2) m_ana->smearTracks(arichTracks);

        nTracks = arichTracks.size();
        if (nTracks > 0) m_ana->likelihood2(arichTracks);

        // build the relations (ARICHAeroHit-ARICHLikelihood))

        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHTrack* track = &arichTracks[iTrack];
          double expectedPhotons[5];
          double likelihoods[5];
          track->getExpectedPhotons(expectedPhotons);
          track->getLikelihood(likelihoods);
          int detectedPhotons = track->getDetectedPhotons();
          new(arichLikelihoods.nextFreeAddress()) ARICHLikelihood(1, likelihoods, detectedPhotons, expectedPhotons);
          int last = arichLikelihoods.getEntries() - 1;
          int aeroIndex = track->getAeroIndex();
          if (aeroIndex >= 0) {
            aeroHitToArich.add(aeroIndex, last);
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
      ExtDetectorID myDetID = EXT_ARICH; // arich
      int pdgCode = abs(hypothesis.getPDGCode());

      StoreArray<Track> Tracks(m_tracksColName);

      for (int itra = 0; itra < Tracks.getEntries(); ++itra) {
        const Track* track = Tracks[itra];
        const TrackFitResult* fitResult = track->getTrackFitResult(hypothesis);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << hypothesis.getPDGCode());
          continue;
        }
        int charge = fitResult->getChargeSign();
        B2DEBUG(50, "Track.Charge " << charge);
        const MCParticle* particle = DataStore::getRelated<MCParticle>(track);
        const ARICHAeroHit* aeroHit = DataStore::getRelated<ARICHAeroHit>(particle);
        int aeroHitIndex = -1;
        if (aeroHit) aeroHitIndex = aeroHit->getArrayIndex();
        int truePDGCode = 0;
        if (particle) truePDGCode = particle->getPDG();

        RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);

        for (unsigned i = 0; i < extHits.size(); i++) {
          const ExtHit* extHit = extHits[i];
          if (abs(extHit->getPdgCode()) != pdgCode) continue;
          if (extHit->getDetectorID() != myDetID) continue;
          if (extHit->getCopyID() != 12345) continue; // aerogel Al support plate
          if (extHit->getStatus() != EXT_EXIT) continue; // particles registered at the EXIT of the Al plate
          B2DEBUG(100, "getTracks: z = " << extHit->getPosition().Z());
          ARICHTrack trk(extHit, charge, truePDGCode, (int)itra, aeroHitIndex);
          tracks.push_back(trk);
        }
      }
    }

  } // namespace arich
} // namespace Belle2
