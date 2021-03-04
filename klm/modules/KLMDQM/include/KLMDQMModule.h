/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Vipin Gaur, Leo Piilonen                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMHit1d.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>

/* Belle 2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawKLM.h>

/* ROOT headers. */
#include <TH1F.h>

namespace Belle2 {

  /**
   * KLM data quality monitor module.
   */
  class KLMDQMModule : public HistoModule {

  public:

    /**
     * Constructor.
     */
    KLMDQMModule();

    /**
     * Destructor
     */
    ~KLMDQMModule();

    /**
     * Definition of the histograms.
     */
    void defineHisto() override;

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

    /** Bins for the trigger bits historgrams. */
    enum TriggerBitsBin {

      /** 0x8. */
      c_0x8 = 1,

      /** 0x4. */
      c_0x4 = 2,

      /** 0x2. */
      c_0x2 = 3,

      /** 0x1. */
      c_0x1 = 4,

    };

    /** Number of channel hit histograms per sector for BKLM. */
    const int m_ChannelHitHistogramsBKLM = 2;

    /** Number of channel hit histograms per sector for EKLM. */
    const int m_ChannelHitHistogramsEKLM = 3;

    /** Directory for KLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryName;

    /** Directory for EKLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryNameEKLM;

    /** Directory for BKLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryNameBKLM;

    /** KLM DAQ inclusion. */
    TH1F* m_DAQInclusion;

    /** Time: BKLM RPCs. */
    TH1F* m_TimeRPC;

    /** Time: BKLM scintillators. */
    TH1F* m_TimeScintillatorBKLM;

    /** Time: EKLM scintillators. */
    TH1F* m_TimeScintillatorEKLM;

    /** Plane occupancy: BKLM, phi readout. */
    TH1F* m_PlaneBKLMPhi;

    /** Plane occupancy: BKLM, z readout. */
    TH1F* m_PlaneBKLMZ;

    /** Plane occupancy: EKLM. */
    TH1F* m_PlaneEKLM;

    /** Number of hits per channel. */
    TH1F** m_ChannelHits[
      EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() +
      BKLMElementNumbers::getMaximalSectorGlobalNumber()] = {nullptr};

    /** Masked channels per sector. */
    TH1F* m_MaskedChannelsPerSector;

    /** Number of digits: whole KLM. */
    TH1F* m_DigitsKLM;

    /** Number of digits: BKLM RPCs. */
    TH1F* m_DigitsRPC;

    /** Number of digits: BKLM scintillators. */
    TH1F* m_DigitsScintillatorBKLM;

    /** Number of digits: EKLM scintillators. */
    TH1F* m_DigitsScintillatorEKLM;

    /** Trigger bits: BKLM scintillators. */
    TH1F* m_TriggerBitsBKLM;

    /** Trigger bits: EKLM scintillators. */
    TH1F* m_TriggerBitsEKLM;

    /** Number of KLM Digits after LER injection. */
    TH1F* m_KlmDigitsAfterLERInj;

    /** Histogram to be used for normalization of occupancy after LER injection. */
    TH1F* m_TriggersLERInj;

    /** Number of KLM Digits after LER injection. */
    TH1F* m_KlmDigitsAfterHERInj;

    /** Histogram to be used for normalization of occupancy after HER injection. */
    TH1F* m_TriggersHERInj;

    /** KLM channel array index. */
    const KLMChannelArrayIndex* m_ChannelArrayIndex;

    /** KLM sector array index. */
    const KLMSectorArrayIndex* m_SectorArrayIndex;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** Raw FTSW. */
    StoreArray<RawFTSW> m_RawFtsws;

    /** Raw KLM. */
    StoreArray<RawKLM> m_RawKlms;

    /** KLM digits. */
    StoreArray<KLMDigit> m_Digits;

    /** BKLM 1d hits. */
    StoreArray<BKLMHit1d> m_BklmHit1ds;

    /** BKLM 2d hits. */
    StoreArray<BKLMHit2d> m_BklmHit2ds;

  };

}
