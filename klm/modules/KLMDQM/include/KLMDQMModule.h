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
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

/* Belle 2 headers. */
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

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
    virtual ~KLMDQMModule();

    /**
     * Definition of the histograms.
     */
    virtual void defineHisto() override;

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
     * Define EKLM histograms.
     */
    void defineHistoEKLM();

    /**
     * Define BKLM histograms.
     */
    void defineHistoBKLM();

    /** Directory for KLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryName;

    /** Directory for EKLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryNameEKLM;

    /** Directory for BKLM DQM histograms in ROOT file. */
    std::string m_HistogramDirectoryNameBKLM;

    /** name of BKLMDigit store array. */
    std::string m_inputDigitsName;

    /** Name of BKLMHit2d store array. */
    std::string m_inputHitsName2d;

    /** Name of BKLMHit1d store array. */
    std::string m_inputHitsName1d;

    /** KLM channel array index. */
    const KLMChannelArrayIndex* m_ChannelArrayIndex;

    /** KLM sector array index. */
    const KLMSectorArrayIndex* m_SectorArrayIndex;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_Elements;

    /** Digits. */
    StoreArray<EKLMDigit> m_Digits;

    /** Time: BKLM RPCs. */
    TH1F* m_TimeRPC;

    /** Time: BKLM scintillators. */
    TH1F* m_TimeScintillatorBKLM;

    /** Time: EKLM scintillators. */
    TH1F* m_TimeScintillatorEKLM;

    /** Number of hits per channel. */
    TH1F** m_ChannelHits[
      EKLMElementNumbers::getMaximalSectorGlobalNumberKLMOrder() +
      BKLMElementNumbers::getMaximalSectorGlobalNumber()] = {nullptr};

    /** Number of channel hit histograms per sector for BKLM. */
    const int m_ChannelHitHistogramsBKLM = 2;

    /** Number of channel hit histograms per sector for EKLM. */
    const int m_ChannelHitHistogramsEKLM = 3;

    /** Sector number. */
    TH1F* m_eklmSector;

    /** Axial position of muon hit. */
    TH1F* m_bklmHit2dsZ;

    /** Sector and layer number occupancy for phi-readout hits */
    TH1F* m_bklmSectorLayerPhi;

    /** Sector and layer number occupancy for Z-readout hits */
    TH1F* m_bklmSectorLayerZ;

    /** Number of BKLM Digits. */
    TH1F* m_bklmDigitsN;

  };

}
