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

#include <arich/modules/arichReconstruction/ARICHReconstructorModule.h>
#include <time.h>

// Hit classes
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <generators/dataobjects/MCParticle.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
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
      m_nEvent(0)
    {
      // Set description()
      setDescription("ARICHReconstructor");
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
      std::vector<double> defMerit;
      defMerit.push_back(30.0);
      defMerit.push_back(30.0);
      defMerit.push_back(30.0);
      // Add parameters
      addParam("InputColName", m_MCColName, "Input from MC", string(""));
      addParam("TracksColName", m_TracksColName, "Mdst tracks", string(""));
      addParam("ExtHitsColName", m_extHitsColName, "Extrapolated tracks", string(""));
      addParam("OutputColName", m_outColName, "Output: ARICH Likelihoods",  string(""));
      addParam("TrackPositionResolution", m_trackPosRes, "Resolution of track position on aerogel plane", 1.0 * Unit::mm);
      addParam("TrackAngleResolution", m_trackAngRes, "Resolution of track direction angle on aerogel plane", 1.0 * Unit::mrad);
      addParam("BackgroundLevel", m_bkgLevel, "Background level in photon hits per m^2", 0.0);
      addParam("SinglePhotonResolution", m_singleRes, "Single photon resolution without pad", 0.03 * Unit::mm);
      addParam("AerogelFigureOfMerit", m_aeroMerit, "Aerogel figure of merit", defMerit);
      addParam("InputTrackType", m_inputTrackType, "Input tracks from the tracking (0) or from simulation (1)", 0);
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
      m_ana = new ARICHReconstruction();
      m_ana->setBackgroundLevel(m_bkgLevel);
      m_ana->setTrackPositionResolution(m_trackPosRes);
      m_ana->setTrackAngleResolution(m_trackAngRes);
      m_ana->setSinglePhotonResolution(m_singleRes);
      m_ana->setAerogelFigureOfMerit(m_aeroMerit);
      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      StoreArray<ARICHLikelihoods>::registerPersistent(m_outColName);
      RelationArray::registerPersistent<ARICHAeroHit, ARICHLikelihoods>(m_MCColName, m_outColName);
      RelationArray::registerPersistent<Track, ARICHLikelihoods>(m_TracksColName, m_outColName);
      RelationArray::registerPersistent<ExtHit, ARICHLikelihoods>(m_extHitsColName, m_outColName);
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
        StoreArray<Track> mdstTracks(m_TracksColName);
        StoreArray<ExtHit> extHits(m_extHitsColName);

        // Output - likelihoods
        StoreArray<ARICHLikelihoods> arichLikelihoods(m_outColName);
        if (!arichLikelihoods.isValid()) arichLikelihoods.create();

        // Output - relations
        RelationArray trackToArich(mdstTracks, arichLikelihoods);
        trackToArich.clear();
        RelationArray extToArich(extHits, arichLikelihoods);
        extToArich.clear();

        std::vector<ARICHTrack> arichTracks;
        getTracks(arichTracks, Const::pion); // pion hypothesis
        B2DEBUG(100, "Number of tracks from ext" << arichTracks.size());
        if (arichTracks.empty()) return;

        m_ana->Likelihood2(arichTracks);

        int nTracks = arichTracks.size();
        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHTrack* track = &arichTracks[iTrack];

          double like[5];
          double exp_phot[5];
          track->getLikelihood(like);
          track->getExpectedNOfPhotons(exp_phot);

          new(arichLikelihoods.nextFreeAddress()) ARICHLikelihoods(1, like, 1, exp_phot);

          int last = arichLikelihoods.getEntries() - 1;
          trackToArich.add(track->getTrackID(), last);
          extToArich.add(track->getHitID(), last);
        }
      } else if (m_inputTrackType == 1) {
        //------------------------------------------------------
        // Get the collection of ARICHSimHits from the DataStore.
        //------------------------------------------------------

        StoreArray<ARICHAeroHit> arichAeroHits(m_MCColName);

        // Output: ARICH likelihoods
        StoreArray<ARICHLikelihoods> arichLikelihoods(m_outColName);
        arichLikelihoods.create();

        // Output: AeroHits to ARICH likelihoods relations
        RelationArray  relAeroToLikelihood(arichAeroHits, arichLikelihoods);
        relAeroToLikelihood.clear();

        std::vector<ARICHTrack> arichTracks;

        // Get number of hits in this event
        int nTracks = arichAeroHits.getEntries();

        // Loop over all tracks
        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHAeroHit* aeroHit = arichAeroHits[iTrack];
          arichTracks.push_back(ARICHTrack(*aeroHit));
        } // for iTrack

        m_ana->ReconstructParticles(arichTracks);
        m_ana->Likelihood2(arichTracks);



        for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
          ARICHTrack* track = &arichTracks[iTrack];
          double like[5];
          double exp_phot[5];
          track->getLikelihood(like);
          track->getExpectedNOfPhotons(exp_phot);
          new(arichLikelihoods.nextFreeAddress()) ARICHLikelihoods(1, like, 1, exp_phot);
          relAeroToLikelihood.add(iTrack, iTrack);
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
      std::string input[2] = {"extrapolation", "simulation"};
      B2INFO("Input from " << input[m_inputTrackType]);
    }

    void ARICHReconstructorModule::getTracks(std::vector<ARICHTrack> & tracks,
                                             Const::ChargedStable chargedStable)
    {
      ExtDetectorID myDetID = EXT_ARICH; // arich
      int pdgCode = abs(chargedStable.getPDGCode());

      StoreArray<Track> Tracks(m_TracksColName);

      for (int itra = 0; itra < Tracks.getEntries(); ++itra) {
        const Track* track = Tracks[itra];
        const TrackFitResult* fitResult = track->getTrackFitResult(chargedStable);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << chargedStable.getPDGCode());
          continue;
        }
        int charge = fitResult->getCharge();
        const MCParticle* particle = DataStore::getRelated<MCParticle>(track);
        int truePDGCode = 0;
        if (particle) truePDGCode = particle->getPDG();

        RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
        for (unsigned i = 0; i < extHits.size(); i++) {
          const ExtHit* extHit = extHits[i];
          if (abs(extHit->getPdgCode()) != pdgCode) continue;
          if (extHit->getDetectorID() != myDetID) continue;
          if (extHit->getCopyID() != 12345) continue;
          if (extHit->getStatus() != EXT_EXIT) continue;
          B2DEBUG(100, "getTracks: z = " << extHit->getPosition().Z());
          ARICHTrack trk(extHit, charge, (int)itra);
          tracks.push_back(trk);
        }
      }
    }

  } // namespace arich
} // namespace Belle2
