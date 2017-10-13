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

#include <background/dataobjects/BackgroundInfo.h>


// ROOT
#include <TVector2.h>
#include <TVector3.h>
#include <TRandom3.h>
#include <TGraph2D.h>

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

  ARICHDigitizerModule::ARICHDigitizerModule() :  Module(),
    m_maxQE(0)
  {

    // Set description()
    setDescription("This module creates ARICHDigits from ARICHSimHits. Here spatial digitization is done, channel-by-channel QE is applied, and readout time window cut is applied.");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("TimeWindow", m_timeWindow, "Readout time window width in ns", 250.0);
    addParam("BackgroundHits", m_bkgLevel, "Number of background hits per hapd per readout (electronics noise)", 0.2);

  }

  ARICHDigitizerModule::~ARICHDigitizerModule()
  {

  }

  void ARICHDigitizerModule::initialize()
  {
    // Print set parameters
    printModuleParams();

    // QE at 400nm (3.1eV) applied in SensitiveDetector
    m_maxQE = m_simPar->getQE(3.1);

    StoreArray<ARICHDigit> digits;
    digits.registerInDataStore();

    m_bgOverlay = false;
    StoreObjPtr<BackgroundInfo> bgInfo("", DataStore::c_Persistent);
    if (bgInfo.isValid()) {
      if (bgInfo->getMethod() == BackgroundInfo::c_Overlay) m_bgOverlay = true;
    }

  }

  void ARICHDigitizerModule::beginRun()
  {
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

      // Get id of module
      int moduleID = aSimHit->getModuleID();

      // skip if not active
      if (!m_modInfo->isActive(moduleID)) continue;

      // Get id number of hit channel
      int chX, chY;
      m_geoPar->getHAPDGeometry().getXYChannel(aSimHit->getLocalPosition().X(), aSimHit->getLocalPosition().Y(), chX, chY);
      if (chX < 0 && chY < 0) continue;

      int asicChannel = m_chnMap->getAsicFromXY(chX, chY);

      // eliminate un-active channels
      if (asicChannel < 0 || !m_chnMask->isActive(moduleID, asicChannel)) continue;

      // apply channel dependent QE scale factor
      //double qe_scale =  0.27 / m_maxQE;
      //double qe_scale = m_modInfo->getChannelQE(moduleID, asicChannel) * m_simPar->getColEff() / m_maxQE; // eventually move collection efficiency to here!
      double qe_scale = m_modInfo->getChannelQE(moduleID, asicChannel) / m_maxQE;

      if (qe_scale > 1.) B2ERROR("Channel QE is higher than QE applied in SensitiveDetector");
      if (gRandom->Uniform(1.) > qe_scale) continue;

      // photon was converted to photoelectron
      chipHits[make_pair(moduleID, asicChannel / 36)] += 1;
      photoElectrons[make_pair(moduleID, asicChannel)] += 1;

    }

    // loop over produced photoelectrons. Apply suppression due to the reverse polarization crosstalk
    // among channels on the same chip, and produce hit bitmap (4 bits).

    for (std::map<std::pair<int, int> , int>::iterator it = photoElectrons.begin(); it != photoElectrons.end(); ++it) {

      std::pair<int, int> modch = it->first;
      double npe = double(it->second);

      // reduce efficiency
      npe /= (1.0 + m_simPar->getChipNegativeCrosstalk() * (double(chipHits[make_pair(modch.first, modch.second / 36)]) - 1.0));
      if (npe < 1.0 && gRandom->Uniform(1) > npe) continue;

      // Make hit bitmap (depends on number of p.e. on channel). For now bitmap is 0001 for single p.e., 0011 for 2 p.e., ...
      // More proper implementation is to be done ...
      uint8_t bitmap = 0;
      for (int i = 0; i < npe; i++) {
        bitmap |= 1 << i;
        if (i == 3) break;
      }

      // make new digit!
      arichDigits.appendNew(modch.first, modch.second, bitmap);

    }

    //--- if background not overlayed add electronic noise hits
    if (m_bgOverlay) return;
    uint8_t bitmap = 1;
    unsigned nSlots = m_geoPar->getDetectorPlane().getNSlots();
    for (unsigned id = 1; id < nSlots + 1; id++) {
      if (!m_modInfo->isActive(id)) continue;
      int nbkg = gRandom->Poisson(m_bkgLevel);
      for (int i = 0; i < nbkg; i++) {
        arichDigits.appendNew(id, gRandom->Integer(144), bitmap);
      }
    }

  }

  void ARICHDigitizerModule::endRun()
  {
  }

  void ARICHDigitizerModule::terminate()
  {

  }

} // end Belle2 namespace
