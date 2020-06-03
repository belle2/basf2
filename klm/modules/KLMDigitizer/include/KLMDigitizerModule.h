/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMSimHit.h>
#include <klm/dataobjects/eklm/EKLMSimHit.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/eklm/EKLMChannels.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/dbobjects/KLMTimeConversion.h>
#include <klm/simulation/ScintillatorFirmware.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  /**
   * KLM digitization module.
   */
  class KLMDigitizerModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMDigitizerModule();

    /**
     * Destructor
     */
    virtual ~KLMDigitizerModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

  private:

    /**
     * Efficiency determination mode.
     */
    enum EfficiencyMode {

      /** Strip. */
      c_Strip,

      /** Plane. */
      c_Plane,

    };

    /**
     * Check channel parameters for channel-specific simulation.
     */
    void checkChannelParameters();

    /**
     * Digitization in BKLM.
     */
    void digitizeBKLM();

    /**
     * Digitization in EKLM.
     */
    void digitizeEKLM();

    /**
     * Check if channel is active (status is not KLMChannelStatus::c_Dead).
     * @param[in] channel Channel.
     */
    bool checkActive(uint16_t channel);

    /**
     * Efficiency correction.
     * @param[in] efficiency Efficiency.
     * @return True if the digitization passes the efficiency correction.
     */
    bool efficiencyCorrection(float efficiency);

    /** Digitization parameters. */
    DBObjPtr<KLMScintillatorDigitizationParameters> m_DigPar;

    /** Time conversion. */
    DBObjPtr<KLMTimeConversion> m_TimeConversion;

    /** Channel data. */
    DBObjPtr<EKLMChannels> m_Channels;

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /** Strip efficiency. */
    DBObjPtr<KLMStripEfficiency> m_StripEfficiency;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** Simulation mode. */
    std::string m_SimulationMode;

    /** Whether the simulation is channel-specific. */
    bool m_ChannelSpecificSimulation;

    /** Initial digitization time. */
    double m_DigitizationInitialTime;

    /** Save FPGA fit data (KLMScintillatorFirmwareFitResult). */
    bool m_SaveFPGAFit;

    /** Efficiency determination mode ("Strip" or "Plane"). */
    std::string m_Efficiency;

    /** Efficiency determination mode (converted from the string parameter). */
    EfficiencyMode m_EfficiencyMode;

    /** Use debug mode in EKLM::ScintillatorSimulator or not. */
    bool m_Debug;

    /** Simulation hit map for BKLM (by channel). */
    std::multimap<uint16_t, const BKLMSimHit*> m_bklmSimHitChannelMap;

    /** Simulation hit map for BKLM (by plane). */
    std::multimap<uint16_t, const BKLMSimHit*> m_bklmSimHitPlaneMap;

    /** Simulation hit map for EKLM (by channel). */
    std::multimap<uint16_t, const EKLMSimHit*> m_eklmSimHitChannelMap;

    /** Simulation hit map for EKLM (by plane). */
    std::multimap<uint16_t, const EKLMSimHit*> m_eklmSimHitPlaneMap;

    /** FPGA fitter. */
    KLM::ScintillatorFirmware* m_Fitter;

    /** BKLM simulation hits. */
    StoreArray<BKLMSimHit> m_bklmSimHits;

    /** EKLM simulation hits. */
    StoreArray<EKLMSimHit> m_eklmSimHits;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /** FPGA fits. */
    StoreArray<KLMScintillatorFirmwareFitResult> m_FPGAFits;

  };

}
