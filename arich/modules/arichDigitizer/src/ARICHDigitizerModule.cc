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
    setDescription("This module creates ARICHDigits from ARICHSimHits. Here spatial digitization is done, channel-by-channel QE is applied, and readout time window cut is applied.");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("InputColName", m_inColName, "ARICHSimHits collection name", string("ARICHSimHitArray"));
    addParam("OutputColName", m_outColName, "ARICHDigit collection name", string("ARICHDigitArray"));
    addParam("TimeWindow", m_timeWindow, "Readout time window width in ns", 250.0);
    addParam("BackgroundHits", m_bkgLevel, "Number of background hits per hapd per readout (electronics noise)", 0.4);

  }

  ARICHDigitizerModule::~ARICHDigitizerModule()
  {

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

    StoreArray<ARICHDigit> digits;
    digits.registerInDataStore();
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

    //---------------------------------------------------------------------
    // Convert SimHits one by one to digitizer hits.
    //---------------------------------------------------------------------

    // We try to include the effect of opposite-polarity crosstalk among channels
    // on each chip, which depend on number of p.e. on chip

    std::map<pair<int, int>, int> photoElectrons; // this contains number of photoelectrons falling on each channel
    std::map<pair<int, int>, int> chipHits; // this contains number of photoelectrons on each chip

    // Get number of photon hits in this event
    int nHits = arichSimHits.getEntries();

    // Loop over all photon hits
    for (int iHit = 0; iHit < nHits; ++iHit) {

      ARICHSimHit* aSimHit = arichSimHits[iHit];

      // check for time window
      double globaltime = aSimHit->getGlobalTime();

      if (globaltime < 0. || globaltime > m_timeWindow) continue;

      TVector2 locpos(aSimHit->getLocalPosition().X(), aSimHit->getLocalPosition().Y());

      // Get id number of hit channel
      int channelID = m_arichgp->getChannelID(locpos);
      // Get id of module
      int moduleID = aSimHit->getModuleID();
      // eliminate un-active channels
      if (channelID < 0 || !m_arichgp->isActive(moduleID, channelID)) continue;
      // apply channel dependent QE scale factor
      double qe_scale =  m_arichgp->getChannelQE(moduleID, channelID) / m_maxQE;
      if (qe_scale > 1.) B2ERROR("Channel QE is higher than QE applied in SensitiveDetector");
      if (gRandom->Uniform(1.) > qe_scale) continue;

      // photon was converted to photoelectron
      chipHits[make_pair(moduleID, channelID / 36)] += 1;
      photoElectrons[make_pair(moduleID, channelID)] += 1;

    }

    // loop over produced photoelectrons. Apply suppression due to the reverse polarization crosstalk
    // among channels on the same chip, and produce hit bitmap (4 bits).

    for (std::map<std::pair<int, int> , int>::iterator it = photoElectrons.begin(); it != photoElectrons.end(); ++it) {

      std::pair<int, int> modch = it->first;
      double npe = double(it->second);

      // reduce efficiency
      npe /= (1.0 + m_arichgp->getChipNegativeCrosstalk() * (double(chipHits[make_pair(modch.first, modch.second / 36)]) - 1.0));
      if (npe < 1.0 && gRandom->Uniform(1) > npe) continue;

      // Make hit bitmap (depends on number of p.e. on channel). For now bitmap is 0001 for signle p.e., 0011 for 2 p.e., ...
      // More proper implementation is to be done ...
      uint8_t bitmap = 0;
      for (int i = 0; i < npe; i++) {
        bitmap |= 1 << i;
        if (i == 3) break;
      }

      // make new digit!
      arichDigits.appendNew(modch.first, modch.second, bitmap);

    }

    //--- add electronic noise hits
    uint8_t bitmap = 1;
    for (int id = 1; id < m_arichgp->getNMCopies() + 1; id++) {
      int nbkg = gRandom->Poisson(m_bkgLevel);
      for (int i = 0; i < nbkg; i++) {
        arichDigits.appendNew(id, gRandom->Integer(144), bitmap);
      }
    }

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

} // end Belle2 namespace
