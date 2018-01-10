/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Igal Jaegle                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <simulation/background/BeamBGTypes.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <string>

// SimHits
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>

// BEAST SimHits
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>
#include <beast/claw/dataobjects/ClawSimHit.h>
#include <beast/claws/dataobjects/CLAWSSimHit.h>
#include <beast/fangs/dataobjects/FANGSSimHit.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/he3tube/dataobjects/He3tubeSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorSimHit.h>
#include <beast/bgo/dataobjects/BgoSimHit.h>
#include <beast/csi/dataobjects/CsiSimHit.h>


namespace Belle2 {

  /**
   * A module that sets m_backgroundTag variable in SimHits (see SimHitBase.h).
   * @return true if at least one SimHit store array has entries.
   */
  class BeamBkgTagSetterModule : public Module {

  public:

    /**
     * Constructor
     */
    BeamBkgTagSetterModule();

    /**
     * Destructor
     */
    virtual ~BeamBkgTagSetterModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    std::string m_backgroundType; /**< BG type */
    double m_realTime; /**< real time that corresponds to beam background sample */
    int m_phase; /**< Phase that corresponds to beam background sample */
    std::string m_specialFor; /**< ordinary or special file for ECL, PXD */

    background::BeamBGTypes m_bgTypes;  /**< defined BG types */
    SimHitBase::BG_TAG m_backgroundTag; /**< background tag to set (from BG type) */
    BackgroundMetaData::EFileType m_fileType; /**< file type to set */

    StoreArray<PXDSimHit> m_pxdSimHits; /**< PXD simulated hits */
    StoreArray<SVDSimHit> m_svdSimHits; /**< SVD simulated hits */
    StoreArray<CDCSimHit> m_cdcSimHits; /**< CDC simulated hits */
    StoreArray<TOPSimHit> m_topSimHits; /**< TOP simulated hits */
    StoreArray<ARICHSimHit> m_arichSimHits; /**< ARICH simulated hits */
    StoreArray<ECLSimHit> m_eclSimHits; /**< ECL simulated hits */
    StoreArray<ECLHit> m_eclHits; /**< ECL simulated hits (short version) */
    StoreArray<BKLMSimHit> m_bklmSimHits; /**< BKLM simulated hits */
    StoreArray<EKLMSimHit> m_eklmSimHits; /**< EKLM simulated hits */

    StoreArray<BeamabortSimHit> m_diaSimHits; /**< beast simulated hits */
    StoreArray<CLAWSSimHit> m_clw2SimHits;  /**< beast simulated hits */
    StoreArray<ClawSimHit> m_clw1SimHits;  /**< beast simulated hits */
    StoreArray<FANGSSimHit> m_fngSimHits; /**< beast simulated hits */
    StoreArray<PlumeSimHit> m_plmSimHits; /**< beast simulated hits */
    StoreArray<PindiodeSimHit> m_pinSimHits; /**< beast simulated hits */
    StoreArray<He3tubeSimHit> m_he3SimHits; /**< beast simulated hits */
    StoreArray<MicrotpcSimHit> m_tpcSimHits; /**< beast simulated hits */
    StoreArray<QcsmonitorSimHit> m_sciSimHits; /**< beast simulated hits */
    StoreArray<BgoSimHit> m_bgoSimHits; /**< beast simulated hits */
    StoreArray<CsiSimHit> m_csiSimHits; /**< beast simulated hits */

    /**
     * functions that set background tag in SimHits
     * @param simHits a reference to DataStore SimHits
     * @return number of tagged SimHit entries
     */
    template<class SIMHIT>
    int setBackgroundTag(StoreArray<SIMHIT>& simHits)
    {
      if (!simHits.isValid()) return 0;

      int numEntries = simHits.getEntries();
      for (int i = 0; i < numEntries; i++) {
        SIMHIT* simHit = simHits[i];
        simHit->setBackgroundTag(m_backgroundTag);
      }
      return numEntries;
    }


  };

} // Belle2 namespace

