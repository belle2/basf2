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

/* Belle2 headers. */
#include <klm/bklm/dataobjects/BKLMDigit.h>
#include <klm/bklm/dataobjects/BKLMSimHit.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMSimHit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/dbobjects/EKLMChannels.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/dbobjects/KLMTimeConversion.h>
#include <klm/simulation/ScintillatorFirmware.h>

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
    const EKLM::ElementNumbersSingleton* m_eklmElementNumbers;

    /** Simulation mode. */
    std::string m_SimulationMode;

    /** Whether the simulation is channel-specific. */
    bool m_ChannelSpecificSimulation;

    /** Initial digitization time. */
    double m_DigitizationInitialTime;

    /** Save FPGA fit data (KLMScintillatorFirmwareFitResult). */
    bool m_SaveFPGAFit;

    /** Use debug mode in EKLM::ScintillatorSimulator or not. */
    bool m_Debug;

    /** Simulation hit map for BKLM. */
    std::multimap<uint16_t, BKLMSimHit*> m_bklmSimHitChannelMap;

    /** Simulation hit map for EKLM. */
    std::multimap<uint16_t, EKLMSimHit*> m_eklmSimHitChannelMap;

    /** FPGA fitter. */
    KLM::ScintillatorFirmware* m_Fitter;

    /** BKLM simulation hits. */
    StoreArray<BKLMSimHit> m_bklmSimHits;

    /** EKLM simulation hits. */
    StoreArray<EKLMSimHit> m_eklmSimHits;

    /** BKLM digits. */
    StoreArray<BKLMDigit> m_bklmDigits;

    /** EKLM digits. */
    StoreArray<EKLMDigit> m_eklmDigits;

    /** FPGA fits. */
    StoreArray<KLMScintillatorFirmwareFitResult> m_FPGAFits;

  };

}
