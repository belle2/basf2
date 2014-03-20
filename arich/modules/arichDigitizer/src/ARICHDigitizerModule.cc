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
#include <arich/modules/arichDigitizer/ARICHDigitizerModule.h>
#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <arich/dataobjects/ARICHSimHit.h>
#include <arich/dataobjects/ARICHDigit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector2.h>
#include <TRandom3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace arich {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(ARICHDigitizer)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    ARICHDigitizerModule::ARICHDigitizerModule() :
      Module(),
      m_inColName(""),
      m_outColName(""),
      m_timeCPU(0),
      m_nRun(0),
      m_nEvent(0),
      m_maxQE(0),
      m_arichgp(ARICHGeometryPar::Instance())
    {
      // Set description()
      setDescription("ARICHDigitizer");
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
      // Add parameters
      addParam("InputColName", m_inColName, "ARICHSimHits collection name", string("ARICHSimHitArray"));
      addParam("OutputColName", m_outColName, "ARICHDigit collection name", string("ARICHDigitArray"));
      addParam("TimeWindow", m_timeWindow, "Readout time window width in ns", 250.0);

    }

    ARICHDigitizerModule::~ARICHDigitizerModule()
    {
      if (m_arichgp) delete m_arichgp;
    }

    void ARICHDigitizerModule::initialize()
    {
      // Initialize variables
      m_nRun    = 0 ;
      m_nEvent  = 0 ;

      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      // QE at 400nm (3.1eV) applied in SensitiveDetector
      m_maxQE = m_arichgp->QE(3.1);

      StoreArray<ARICHDigit>::registerPersistent();
    }

    void ARICHDigitizerModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHDigitizer: Processing run: " << m_nRun);

    }

    void ARICHDigitizerModule::event()
    {

      // Get the collection of ARICHSimHits from the Data store.
      //------------------------------------------------------
      StoreArray<ARICHSimHit> arichSimHits;
      //-----------------------------------------------------

      // Get the collection of arichDigits from the Data store,
      // (or have one created)
      //-----------------------------------------------------
      StoreArray<ARICHDigit> arichDigits;
      if (!arichDigits.isValid()) arichDigits.create();

      //---------------------------------------------------------------------
      // Convert SimHits one by one to digitizer hits.
      //---------------------------------------------------------------------

      // Get number of hits in this event
      int nHits = arichSimHits.getEntries();
      // Loop over all hits
      for (int iHit = 0; iHit < nHits; ++iHit) {

        ARICHSimHit* aSimHit = arichSimHits[iHit];

        // check for time window
        double globaltime = aSimHit->getGlobalTime();

        if (globaltime < 0 || globaltime > m_timeWindow) continue;

        TVector2 locpos(aSimHit->getLocalPosition().X(), aSimHit->getLocalPosition().Y());

        // Get id number of hit channel
        int channelID = m_arichgp->getChannelID(locpos);
        // Get id of module
        int moduleID = aSimHit->getModuleID();
        // eliminate un-active channels
        if (channelID < 0 || !m_arichgp->isActive(moduleID, channelID)) continue;
        // apply channel dependent QE scale factor
        double qe_scale =  m_arichgp->getChannelQE(moduleID, channelID) / m_maxQE;
        if (qe_scale > 1) B2ERROR("Channel QE is higher than QE  applied in SensitiveDetector");
        if (gRandom->Uniform(1) > qe_scale) continue;


        // Check if channel already registered hit in this event (no multiple hits)
        bool newhit = true;
        int nSig = arichDigits.getEntries();
        for (int iSig = 0; iSig < nSig; ++iSig) {
          ARICHDigit* aHit = arichDigits[iSig];
          if (aHit->getModuleID() == moduleID && aHit->getChannelID() == channelID) { newhit = false; break; }
        }
        if (!newhit) continue;

        // register new digit
        ARICHDigit* newHit = arichDigits.appendNew();
        newHit->setModuleID(moduleID);
        newHit->setChannelID(channelID);
        newHit->setGlobalTime(globaltime);
      } // for iHit
      m_nEvent++;
    }

    void ARICHDigitizerModule::endRun()
    {
      m_nRun++;
    }

    void ARICHDigitizerModule::terminate()
    {
      // CPU time end
      m_timeCPU = clock() * Unit::us - m_timeCPU;

      // Announce
      B2INFO("ARICHDigitizer finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

    }

  } // end arich namespace
} // end Belle2 namespace
