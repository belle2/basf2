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
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMScintillatorFEEParameters.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/simulation/ScintillatorFirmware.h>
#include <klm/time/KLMTime.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dbobjects/HardwareClockSettings.h>

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
    ~KLMDigitizerModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

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
     * Check scintillator FEE parameters for channel-specific simulation.
     */
    void checkScintillatorFEEParameters();

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

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /** Scintillator digitization parameters. */
    DBObjPtr<KLMScintillatorDigitizationParameters> m_DigPar;

    /** Scintillator FEE parameters. */
    DBObjPtr<KLMScintillatorFEEParameters> m_FEEPar;

    /** Strip efficiency. */
    DBObjPtr<KLMStripEfficiency> m_StripEfficiency;

    /** Hardware clock settings. */
    DBObjPtr<HardwareClockSettings> m_HardwareClockSettings;

    /** Time conversion. */
    KLMTime m_Time;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Simulation mode. */
    std::string m_SimulationMode;

    /** Whether the simulation is channel-specific. */
    bool m_ChannelSpecificSimulation;

    /** Initial digitization time in CTIME periods. */
    int m_DigitizationInitialTime;

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
