/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Vipin Gaur                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TH1F.h>
#include <TH2F.h>

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

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
    std::string m_outputDigitsName;

    /** Name of BKLMHit store array. */
    std::string m_outputHitsName;

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

    /** Sector number. */
    TH1F* m_Sector;

    /** Strip number within a layer. */
    TH1F** m_StripLayer;

    /** Histogram: number of hits per layer. */
    TH1F* h_layerHits;

    /** Histogram: Lowest 16 bits of the B2TT CTIME signal. */
    TH1F* h_ctime;

    /** Histogram: Reconstructed pulse height. */
    TH1F* h_eDep;

    /** Histogram: Reconstructed number MPPC pixels. */
    TH1F* h_nPixel;

    /** Histogram: Detector-module identifier. */
    TH1F* h_moduleID;

    /** Histogram: z-measuring strip numbers of the 2D hit. */
    TH1F* h_zStrips;

    /** Histogram: Phi strip number of muon hit. */
    TH1F* h_phiStrip;

    /** Histogram: Sector number of muon hit. */
    TH1F* h_sector;

    /** Histogram: Layer number of muon hit. */
    TH1F* h_layer;

    /** Histogram: Distance from z axis in transverse plane of muon hit. */
    TH1F* h_rBKLMHit2ds;

    /** Histogram: Axial position of muon hit. */
    TH1F* h_zBKLMHit2ds;

    /** Histogram: Position projected into transverse plane of muon hit. */
    TH2F* h_yvsxBKLMHit2ds;

    /** Histogram: Position projected into x-z plane of muon hit. */
    TH2F* h_xvszBKLMHit2ds;

    /** Histogram: Position projected into y-z plane of muon hit. */
    TH2F* h_yvszBKLMHit2ds;

  };

}
