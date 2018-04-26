/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZERMODULE_H
#define EKLMDIGITIZERMODULE_H

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMSim2Hit.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMDigitizationParameters.h>
#include <eklm/dbobjects/EKLMTimeConversion.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/FPGAFitter.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * Module EKLMDigitizationModule.
   * @details
   * Simple module for reading EKLM hits.
   */
  class EKLMDigitizerModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMDigitizerModule();

    /**
     * Destructor
     */
    virtual ~EKLMDigitizerModule();

    /**
     * Initializer.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * This method is called for each event.
     */
    virtual void event();

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate();

  private:

    /**
     * Read hits from the store, sort sim hits and fill m_HitStripMap.
     */
    void readAndSortSimHits();

    /**
     * Create EKLMSim2Hits from EKLMSimHits using boost:graph mechanism.
     */
    void makeSim2Hits();

    /**
     * Merge hits from the same strip. Create EKLMDigits.
     */
    void mergeSimHitsToStripHits();

    /** Digitization parameters. */
    DBObjPtr<EKLMDigitizationParameters> m_DigPar;

    /** Time conversion. */
    DBObjPtr<EKLMTimeConversion> m_TimeConversion;

    /** Channel data. */
    DBObjPtr<EKLMChannels> m_Channels;

    /** Initial digitization time. */
    double m_DigitizationInitialTime;

    /** Save FPGA fit data (EKLMFPGAFit). */
    bool m_SaveFPGAFit;

    /** Use debug mode in EKLM::FiberAndElectronics or not. */
    bool m_Debug;

    /** Create EKLMSim2Hits? */
    bool m_CreateSim2Hits;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Map for EKLMSimHit sorting according sensitive volumes. */
    std::multimap<int, EKLMSimHit*> m_SimHitVolumeMap;

    /** FPGA fitter. */
    EKLM::FPGAFitter* m_Fitter;

    /** Simulation hits. */
    StoreArray<EKLMSimHit> m_SimHits;

    /** Partly merged simulation hits (not created by default). */
    StoreArray<EKLMSim2Hit> m_Sim2Hits;

    /** Digits. */
    StoreArray<EKLMDigit> m_Digits;

    /** FPGA fits. */
    StoreArray<EKLMFPGAFit> m_FPGAFits;

  };

}

#endif

