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
      m_arichgp(ARICHGeometryPar::Instance())
    {
      // Set description()
      setDescription("ARICHDigitizer");
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
      // Add parameters
      addParam("InputColName", m_inColName, "ARICHSimHits collection name", string("ARICHSimHitArray"));
      addParam("OutputColName", m_outColName, "ARICHDigit collection name", string("ARICHDigitArray"));
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
        int nSig = arichDigits.getEntries();
        for (int iSig = 0; iSig < nSig; ++iSig) {
          ARICHDigit* aHit = arichDigits[iSig];
          if (aHit->getModuleID() == moduleID && aHit->getChannelID() == channelID) { newhit = false; break; }
        }
        if (!newhit) continue;
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

    void ARICHDigitizerModule::printModuleParams() const
    {

    }

    double ARICHDigitizerModule::QESuperBialkali(double energy)
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

    int ARICHDigitizerModule::DetectorQE(double energy)
    {
      return (gRandom->Uniform(1) < QESuperBialkali(energy));
    }

  } // end arich namespace
} // end Belle2 namespace
