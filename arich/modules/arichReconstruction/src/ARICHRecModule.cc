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

#include <arich/modules/arichReconstruction/ARICHRecModule.h>
#include <time.h>

#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/modules/arichReconstruction/ARICHTrack.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

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
    REG_MODULE(ARICHRec)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

    ARICHRecModule::ARICHRecModule() : Module() , m_ana(0)
    {
      // Set description()
      setDescription("ARICHRec");
      std::vector<double> defMerit; defMerit.push_back(30.0); defMerit.push_back(30.0); defMerit.push_back(30.0);
      // Add parameters
      addParam("InputColName", m_inColName, "Input collection name", string("AeroHitARICHArray"));
      addParam("OutputColName", m_outColName, "Output col lection name",  string("ARICHTrackArray"));
      addParam("TrackPositionResolution", m_trackPosRes, "Resolution of track position on aerogel plane", 1.0 * Unit::mm);
      addParam("TrackAngleResolution", m_trackAngRes, "Resolution of track direction angle on aerogel plane", 1.0 * Unit::mrad);
      addParam("AerogelFigureOfMerit", m_aeroMerit, "Aerogel figure of merit", defMerit);
      addParam("BackgroundLevel", m_bkgLevel, "Background level in photon hits per m^2", 0.0);
      addParam("SinglePhotonResolution", m_singleRes, "Single photon resolution without pad", 0.03 * Unit::mm);
    }

    ARICHRecModule::~ARICHRecModule()
    {
      if (m_ana) delete m_ana;
    }

    void ARICHRecModule::initialize()
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

      StoreArray<ARICHAeroHit> arichAeroHits;
      StoreArray<ARICHTrack> arichTracks;

    }

    void ARICHRecModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHReconstruction: Processing run: " << m_nRun);
    }

    void ARICHRecModule::event()
    {
      //------------------------------------------------------
      // Get the collection of ARICHSimHits from the DataStore.
      //------------------------------------------------------

      StoreArray<ARICHAeroHit> arichAeroHits;
      if (!arichAeroHits) {
        B2ERROR("ARICHRecModule: ARICHAeroHits unavailable.");
      }

      //-----------------------------------------------------
      // Get the collection of ARICHHits from the Data store,
      // (or have one created)
      //-----------------------------------------------------
      StoreArray<ARICHTrack> arichTracks;
      if (!arichTracks) {
        B2ERROR("ARICHRecModule: ARICHTracks unavailable.");
      }

      //---------------------------------------------------------------------
      // Convert SimHits one by one to digitizer hits.
      //---------------------------------------------------------------------

      // Get number of hits in this event
      int nTracks = arichAeroHits->GetLast() + 1;

      // Loop over all tracks
      for (int iTrack = 0; iTrack < nTracks; ++iTrack) {
        ARICHAeroHit* aeroHit = arichAeroHits[iTrack];
        new(arichTracks->AddrAt(iTrack)) ARICHTrack(*aeroHit);
      } // for iTrack

      m_ana->ReconstructParticles();
      m_ana->Likelihood2();

      m_nEvent++;
    }

    void ARICHRecModule::endRun()
    {
      m_nRun++;
    }

    void ARICHRecModule::terminate()
    {
      // CPU time end
      m_timeCPU = clock() * Unit::us - m_timeCPU;
      // Announce
      B2INFO("ARICHRecModule finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

    }

    void ARICHRecModule::printModuleParams() const
    {
      B2INFO("ARICHRecModule parameters:")
      B2INFO("Input collection name:  " << m_inColName)
      B2INFO("Output collection name: " << m_outColName)
    }

  } // namespace arich
} // namespace Belle2
