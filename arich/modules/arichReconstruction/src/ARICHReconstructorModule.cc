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
#include <tracking/dataobjects/Track.h>
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
      m_beamtest(0)
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
      addParam("trackPositionResolution", m_trackPositionResolution, "Resolution of track position on aerogel plane", 1.0 * Unit::mm);
      addParam("trackAngleResolution", m_trackAngleResolution, "Resolution of track direction angle on aerogel plane", 1.0 * Unit::mrad);
      addParam("backgroundLevel", m_backgroundLevel, "Background level in photon hits per m^2", 0.0);
      addParam("singleResolution", m_singleResolution, "Single photon resolution without pad", 0.03 * Unit::mm);
      addParam("aerogelMerit", m_aerogelMerit, "Aerogel figure of merit", defMerit);
      addParam("inputTrackType", m_inputTrackType, "Input tracks switch: tracking (0) or AeroHit (1)", 0);
    }

    ARICHReconstructorModule::~ARICHReconstructorModule()
    {
      if (m_ana) delete m_ana;
    }

    void ARICHReconstructorModule::initialize()
    {
      // Initialize variables
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
      RelationArray::registerPersistent<ExtHit, ARICHLikelihood>(m_extHitsColName, m_outColName);
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
        RelationArray extToArich(extHits, arichLikelihoods);
        extToArich.clear();
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
          extToArich.add(track->getHitID(), last);
          int aeroIndex = track->getAeroIndex();
          if (aeroIndex >= 0) {
            aeroHitToArich.add(aeroIndex, last);
          } else {
            B2DEBUG(50, "No AeroHit for Track " << track->getTrackID());
          }
        }

        trackToArich.consolidate();
        extToArich.consolidate();
        aeroHitToArich.consolidate();

      } else if (m_inputTrackType == 1) {
        //------------------------------------------------------
        // Get the collection of ARICHSimHits from the DataStore.
        //------------------------------------------------------

        StoreArray<ARICHAeroHit> arichAeroHits(m_mcColName);

        // Output: ARICH likelihoods
        StoreArray<ARICHLikelihood> arichLikelihoods(m_outColName);
        arichLikelihoods.create();

        // Output: AeroHits to ARICH likelihoods relations
        RelationArray  aeroHitToArich(arichAeroHits, arichLikelihoods);
        aeroHitToArich.clear();

        std::vector<ARICHTrack> arichTracks;

        // Get number of hits in this event
        int nTracks = arichAeroHits.getEntries();

        // Loop over all tracks
        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHAeroHit* aeroHit = arichAeroHits[iTrack];
          arichTracks.push_back(ARICHTrack(*aeroHit));
        } // for iTrack

        m_ana->smearTracks(arichTracks);
        m_ana->likelihood2(arichTracks);

        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHTrack* track = &arichTracks[iTrack];
          double expectedPhotons[5];
          double likelihoods[5];
          track->getExpectedPhotons(expectedPhotons);
          track->getLikelihood(likelihoods);
          int detectedPhotons = track->getDetectedPhotons();
          new(arichLikelihoods.nextFreeAddress()) ARICHLikelihood(1, likelihoods, detectedPhotons, expectedPhotons);
          aeroHitToArich.add(iTrack, iTrack);
        } // for iTrack
      }

      m_nEvent++;
    }

    void ARICHReconstructorModule::endRun()
    {
      m_nRun++;
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
        int charge = fitResult->getCharge();
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
