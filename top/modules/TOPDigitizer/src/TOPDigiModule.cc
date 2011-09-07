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
#include <top/modules/TOPDigitizer/TOPDigiModule.h>

#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector2.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace top {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(TOPDigi)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPDigiModule::TOPDigiModule() : Module(),
        m_random(new TRandom1(0)), m_topgp(TOPGeometryPar::Instance())
    {
      // Set description()
      setDescription("TOPDigitizer");

      // Add parameters
      addParam("InputColName", m_inColName, "Input collection name", string("TOPSimHitArray"));
      addParam("OutputColName", m_outColName, "Output collection name", string("TOPHitArray"));
    }

    TOPDigiModule::~TOPDigiModule()
    {
      if (m_random) delete m_random;
      if (m_topgp) delete m_topgp;
    }

    void TOPDigiModule::initialize()
    {
      // Initialize variables
      m_nRun    = 0 ;
      m_nEvent  = 0 ;

      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      StoreArray<TOPSimHit> topSimHits;
      StoreArray<TOPHit> topHits;
    }

    void TOPDigiModule::beginRun()
    {
      // Print run number
      B2INFO("TOPDigi: Processing run: " << m_nRun);

    }

    void TOPDigiModule::event()
    {

      // Get the collection of TOPSimHits from the Data store.
      //------------------------------------------------------
      StoreArray<TOPSimHit> topSimHits;
      if (!topSimHits) B2ERROR("TOPDigitizerModule: Cannot find TOPSimHit array.");
      //-----------------------------------------------------

      // Get the collection of TOPHits from the Data store,
      // (or have one created)
      //-----------------------------------------------------
      StoreArray<TOPHit> topHits;

      //---------------------------------------------------------------------
      // Convert SimHits one by one to digitizer hits.
      //---------------------------------------------------------------------


      // Get number of hits in this event
      int nHits = topSimHits->GetLast();
      B2INFO("nHits: " << nHits);
      // Loop over all hits

      int dighit = 0;
      for (int iHit = 0; iHit < nHits; ++iHit) {
        // Get a simhit
        TOPSimHit* aSimHit = topSimHits[iHit];

        double energy = aSimHit->getEnergy();

        // Apply q.e. of detector
        if (!DetectorQE(energy)) continue;

        TVector2 locpos(aSimHit->getLocalPosition().X(), aSimHit->getLocalPosition().Y());

        // Get id number of hit channel

        int moduleID = aSimHit->getModuleID();
        int BarID = aSimHit->getBarID();

        int channelID = m_topgp->getChannelID(locpos, moduleID);


        if (channelID < 0) continue;

        double globaltime = aSimHit->getGlobalTime();

        B2INFO("ihit: " << iHit  << " channel ID: " << channelID << " bar ID " << BarID);
        // Check if channel already registered hit in this event(no multiple hits)

        int nentr = topHits->GetEntries();
        new(topHits->AddrAt(nentr)) TOPHit(BarID, channelID, globaltime, energy, aSimHit->getParentID());
        if (aSimHit->getParentID() == 1) {
          dighit++;
        }
      } // for iHit

      m_nEvent++;

      B2INFO("nHits: " << nHits << "digitized hits: " << dighit);
    }

    void TOPDigiModule::endRun()
    {
      m_nRun++;
    }

    void TOPDigiModule::terminate()
    {
      // CPU time end
      m_timeCPU = clock() * Unit::us - m_timeCPU;

      // Announce
      B2INFO("TOPDigi finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

    }

    void TOPDigiModule::printModuleParams() const
    {

    }

    double TOPDigiModule::QEMultiAlkali(double energy)
    {
      const float qe[62] = {0.0, 0.11, 0.11, 0.18, 0.18, 0.21, 0.21, 0.23, 0.23, 0.23, 0.23, 0.24, 0.24, 0.25, 0.25, 0.23, 0.23, 0.21, 0.21, 0.2, 0.2, 0.17, 0.17, 0.14, 0.14, 0.12, 0.12, 0.11, 0.11, 0.095, 0.095, 0.081, 0.081, 0.07, 0.07, 0.06, 0.06, 0.05, 0.05, 0.042, 0.042, 0.036, 0.036, 0.03, 0.03, 0.023, 0.023, 0.019, 0.019, 0.013, 0.013, 0.009, 0.009, 0.006, 0.006, 0.003, 0.003, 0.002, 0.002, 0.001, 0.001, 0.0};

      int ich = (int)((1239.85 / energy - 250) / 10.);
      if (ich < 0 || ich >= 62) return 0;
      return 4.0*0.6*qe[ich];// collection efficiency 0.8 and 4 denoes the fraction of photons killed by staking action

    }

    double TOPDigiModule::QESuperBialkali(double energy)
    {
      const float qe[45] = {0.0, 0.197, 0.251, 0.29, 0.313, 0.333, 0.343, 0.348, 0.35, 0.35, 0.348, 0.346, 0.343, 0.34, 0.335, 0.327, 0.317, 0.306, 0.294, 0.28, 0.263, 0.244, 0.22, 0.197, 0.174, 0.153, 0.133, 0.116, 0.099, 0.083, 0.07, 0.058, 0.048, 0.041, 0.033, 0.026, 0.019, 0.014, 0.01, 0.006, 0.004, 0.002, 0.001, 0.0, 0.0};

      int ich = (int)((1239.85 / energy - 260) / 10.);
      if (ich < 0 || ich >= 45) return 0;
      return 1 / 0.35*0.6*qe[ich];// collection efficiency 0.8 and 1/0.35 denoes the fraction of photons killed by staking action
    }

    bool TOPDigiModule::DetectorQE(double energy)
    {
      return (m_random->Rndm(0) < QEMultiAlkali(energy));
    }

  } // end top namespace
} // end Belle2 namespace
