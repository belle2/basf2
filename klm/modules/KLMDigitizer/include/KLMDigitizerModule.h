/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSimHit.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMElectronicsMap.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMScintillatorFEEParameters.h>
#include <klm/dbobjects/KLMScintillatorFirmware.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/rawdata/RawData.h>
#include <klm/simulation/ScintillatorFirmware.h>
#include <klm/time/KLMTime.h>

/* Basf2 headers. */
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
     * Digitization in RPCs.
     */
    void digitizeRPC();

    /**
     * Digitization in scintillators.
     */
    void digitizeScintillator();

    /**
     * Digitization in ASIC.
     */
    void digitizeAsic();

    /**
     * Check if channel is active (status is not KLMChannelStatus::c_Dead).
     * @param[in] channel Channel.
     */
    bool checkActive(KLMChannelNumber channel);

    /**
     * Efficiency correction.
     * @param[in] efficiency Efficiency.
     * @return True if the digitization passes the efficiency correction.
     */
    bool efficiencyCorrection(float efficiency);

    /** Channel status. */
    DBObjPtr<KLMChannelStatus> m_ChannelStatus;

    /** Electronics map. */
    DBObjPtr<KLMElectronicsMap> m_ElectronicsMap;

    /** Scintillator digitization parameters. */
    DBObjPtr<KLMScintillatorDigitizationParameters> m_DigPar;

    /** Scintillator FEE parameters. */
    DBObjPtr<KLMScintillatorFEEParameters> m_FEEPar;

    /** Scintillator FEE firmware version. */
    DBObjPtr<KLMScintillatorFirmware> m_ScintillatorFirmware;

    /** Strip efficiency. */
    DBObjPtr<KLMStripEfficiency> m_StripEfficiency;

    /** Time conversion. */
    KLMTime* m_Time;

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

    /** Whether to create multi-strip digits. */
    bool m_CreateMultiStripDigits;

    /** Whether to create multi-strip digits for one particular run. */
    bool m_CreateMultiStripDigitsByRun;

    /** Use debug mode in EKLM::ScintillatorSimulator or not. */
    bool m_Debug;

    /** Simulation hit map (by plane). */
    std::multimap<KLMPlaneNumber, const KLMSimHit*> m_MapPlaneSimHit;

    /** Simulation hit map (by channel). */
    std::multimap<KLMChannelNumber, const KLMSimHit*> m_MapChannelSimHit;

    /** Simulation hit map (by ASIC). */
    std::multimap<KLMElectronicsChannel, const KLMSimHit*> m_MapAsicSimHit;

    /** Digits corresponding to ASIC channels. */
    KLMDigit* m_AsicDigits[KLM::c_NChannelsAsic];

    /** Simulation hits lower bound for ASIC digit. */
    std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator
    m_AsicDigitSimHitsLowerBound[KLM::c_NChannelsAsic];

    /** Simulation hits upper bound for ASIC digit. */
    std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator
    m_AsicDigitSimHitsUpperBound[KLM::c_NChannelsAsic];

    /** FPGA fitter. */
    KLM::ScintillatorFirmware* m_Fitter;

    /** Simulation hits. */
    StoreArray<KLMSimHit> m_SimHits;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /** FPGA fits. */
    StoreArray<KLMScintillatorFirmwareFitResult> m_FPGAFits;

  };

}
