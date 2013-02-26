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
      addParam("InputColName", m_inColName, "Input collection name", string("AeroHitARICHArray"));
      addParam("OutputColName", m_outColName, "Output col lection name",  string("ARICHTrackArray"));
      addParam("TrackPositionResolution", m_trackPosRes, "Resolution of track position on aerogel plane", 1.0 * Unit::mm);
      addParam("TrackAngleResolution", m_trackAngRes, "Resolution of track direction angle on aerogel plane", 1.0 * Unit::mrad);
      addParam("BackgroundLevel", m_bkgLevel, "Background level in photon hits per m^2", 0.0);
      addParam("SinglePhotonResolution", m_singleRes, "Single photon resolution without pad", 0.03 * Unit::mm);
      addParam("AerogelFigureOfMerit", m_aeroMerit, "Aerogel figure of merit", defMerit);
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

      StoreArray<ARICHTrack>::registerPersistent();
      StoreArray<ARICHLikelihoods>::registerPersistent();
      RelationArray::registerPersistent<ARICHAeroHit, ARICHLikelihoods>();
    }

    void ARICHReconstructorModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHReconstruction: Processing run: " << m_nRun);
    }

    void ARICHReconstructorModule::event()
    {
      //------------------------------------------------------
      // Get the collection of ARICHSimHits from the DataStore.
      //------------------------------------------------------

      StoreArray<ARICHAeroHit> arichAeroHits;

      //-----------------------------------------------------
      // Get the collection of arichDigits from the Data store,
      // (or have one created)
      //-----------------------------------------------------
      StoreArray<ARICHTrack> arichTracks;
      if (!arichTracks.isValid()) arichTracks.create();
      //---------------------------------------------------------------------
      // Convert SimHits one by one to digitizer hits.
      //---------------------------------------------------------------------

      // Get number of hits in this event
      int nTracks = arichAeroHits.getEntries();

      // Loop over all tracks
      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHAeroHit* aeroHit = arichAeroHits[iTrack];
        new(arichTracks.nextFreeAddress()) ARICHTrack(*aeroHit);
      } // for iTrack

      m_ana->ReconstructParticles();
      m_ana->Likelihood2();

      StoreArray<ARICHLikelihoods> arichLikelihoods;
      if (!arichLikelihoods.isValid()) arichLikelihoods.create();
      RelationArray  relAeroToLikelihood(arichAeroHits, arichLikelihoods);
      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHTrack* track = arichTracks[iTrack];
        double like[5]; double exp_phot[5];
        track->getLikelihood(like); track->getExpectedNOfPhotons(exp_phot);
        new(arichLikelihoods.nextFreeAddress()) ARICHLikelihoods(1, like, 1, exp_phot);
        relAeroToLikelihood.add(iTrack, iTrack);
      } // for iTrack

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
      B2INFO("Input collection name:  " << m_inColName)
      B2INFO("Output collection name: " << m_outColName)
    }

  } // namespace arich
} // namespace Belle2
