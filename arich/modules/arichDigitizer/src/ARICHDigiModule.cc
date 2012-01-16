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
#include <arich/modules/arichDigitizer/ARICHDigiModule.h>
#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <arich/dataobjects/ARICHSimHit.h>
#include <arich/dataobjects/ARICHHit.h>

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
  namespace arich {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(ARICHDigi)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    ARICHDigiModule::ARICHDigiModule() : Module(),
      m_random(new TRandom3(0)), m_arichgp(ARICHGeometryPar::Instance())
    {
      // Set description()
      setDescription("ARICHDigitizer");

      // Add parameters
      addParam("InputColName", m_inColName, "Input collection name", string("ARICHSimHitArray"));
      addParam("OutputColName", m_outColName, "Output collection name", string("ARICHHitArray"));
    }

    ARICHDigiModule::~ARICHDigiModule()
    {
      if (m_random) delete m_random;
      if (m_arichgp) delete m_arichgp;
    }

    void ARICHDigiModule::initialize()
    {
      // Initialize variables
      m_nRun    = 0 ;
      m_nEvent  = 0 ;

      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      StoreArray<ARICHSimHit> arichSimHits;
      StoreArray<ARICHHit> arichHits;
    }

    void ARICHDigiModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHDigi: Processing run: " << m_nRun);

    }

    void ARICHDigiModule::event()
    {

      // Get the collection of ARICHSimHits from the Data store.
      //------------------------------------------------------
      StoreArray<ARICHSimHit> arichSimHits;
      if (!arichSimHits) B2ERROR("ARICHDigitizerModule: Cannot find ARICHSimHit array.");
      //-----------------------------------------------------

      // Get the collection of ARICHHits from the Data store,
      // (or have one created)
      //-----------------------------------------------------
      StoreArray<ARICHHit> arichHits;

      //---------------------------------------------------------------------
      // Convert SimHits one by one to digitizer hits.
      //---------------------------------------------------------------------


      // Get number of hits in this event
      int nHits = arichSimHits->GetLast() + 1;
      // Loop over all hits
      for (int iHit = 0; iHit < nHits; ++iHit) {
        // Get a simhit
        ARICHSimHit* aSimHit = arichSimHits[iHit];

        double energy = aSimHit->getEnergy();

        // Apply q.e. of detector
        if (!DetectorQE(energy)) continue;

        TVector2 locpos(aSimHit->getLocalPosition().X(), aSimHit->getLocalPosition().Y());

        // Get id number of hit channel
        int channelID = m_arichgp->getChannelID(locpos);
        if (channelID < 0) continue;

        double globaltime = aSimHit->getGlobalTime();
        int moduleID = aSimHit->getModuleID();

        TVector3 center = m_arichgp->getChannelCenterGlob(moduleID, channelID);
        // Check if channel already registered hit in this event(no multiple hits)
        bool newhit = true;
        int nSig = arichHits->GetLast();
        for (int iSig = 0; iSig <= nSig; ++iSig) {
          ARICHHit* aHit = arichHits[iSig];
          if (aHit->getModuleID() == moduleID && aHit->getChannelID() == channelID) { newhit = false; break; }
        }
        if (!newhit) continue;

        // Add new ARIHCHit to datastore
        new(arichHits->AddrAt(nSig + 1)) ARICHHit();
        ARICHHit* newHit = arichHits[nSig + 1];
        newHit->setModuleID(moduleID);
        newHit->setChannelID(channelID);
        newHit->setGlobalTime(globaltime);
      } // for iHit

      m_nEvent++;
    }

    void ARICHDigiModule::endRun()
    {
      m_nRun++;
    }

    void ARICHDigiModule::terminate()
    {
      // CPU time end
      m_timeCPU = clock() * Unit::us - m_timeCPU;

      // Announce
      B2INFO("ARICHDigi finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

    }

    void ARICHDigiModule::printModuleParams() const
    {

    }

    double ARICHDigiModule::QESuperBialkali(double energy)
    {
      const float qe[44] = {0.197, 0.251 , 0.29 , 0.313 , 0.333 , 0.343 , 0.348 , 0.35 , 0.35, 0.348,
                            0.346, 0.343, 0.34, 0.335, 0.327 , 0.317 , 0.306 , 0.294 , 0.280 , 0.263 , 0.244 ,
                            0.220 , 0.197 , 0.174 , 0.153 , 0.133 , 0.116 , 0.990E-01, 0.830E-01, 0.700E-01,
                            0.580E-01, 0.480E-01, 0.410E-01, 0.330E-01, 0.260E-01, 0.190E-01, 0.140E-01, 0.100E-01, 0.600E-02,
                            0.400E-02, 0.200E-02, 0.100E-02, 0.00 , 0.00
                           };


      int ich = (int)((1239.85 / energy - 270) / 10.);
      if (ich < 0 || ich >= 44) return 0;
      return 2.857 * 0.8 * qe[ich]; // collection efficiency 0.8

      /*    const float qe[19] = {11.6399277036,
      19.1442321092,
      23.313881771,
      25.0033455,
      24.918330626,
      23.6281703504,
      21.5790653983,
      19.1081799712,
      16.458592,
      13.7950976979,
      11.2208704133,
      8.7949737834,
      6.5507291873,
      4.5149375,
      2.7279551457,
      1.2646244522,
      0.7,
      0.2,
      0.1
      };

      int ich = (int)((1239.85 / energy - 240) / 20.);
      if (ich < 0 || ich >= 19) return 0;
      return 2.857*0.7*qe[ich]/100.;// collection efficiency 0.8
      */
    }

    int ARICHDigiModule::DetectorQE(double energy)
    {
      return (m_random->Uniform(1) < QESuperBialkali(energy));
    }

  } // end arich namespace
} // end Belle2 namespace
