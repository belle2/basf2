/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* DQM headers. */
#include <dqm/core/DQMHistAnalysis.h>

/* Basf2 headers. */
#include <framework/database/DBObjPtr.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/dbobjects/KLMElectronicsMap.h>

/* ROOT headers. */
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TLine.h>
#include <TText.h>

/* C++ headers. */
#include <vector>

namespace Belle2 {

  /**
   * Analysis of KLM DQM histograms.
   */
  class DQMHistAnalysisKLMModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisKLMModule();


    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override final;

  private:

    /**
     * Get number of processed events.
     */
    double getProcessedEvents();

    /**
     * Analyse channel hit histogram.
     * @param[in]  subdetector  Subdetector.
     * @param[in]  section      Section.
     * @param[in]  sector       Sector.
     * @param[in]  index        Histogram Index.
     * @param[in]  histogram    Histogram.
     * @param[in]  canvas       Canvas.
     * @param[out] latex        TLatex to draw messages.
     */
    void analyseChannelHitHistogram(
      int subdetector, int section, int sector, int index,
      TH1* histogram, TCanvas* canvas, TLatex& latex);

    /**
     * Process spatial 2D hits histograms for endcap.
     * @param[in] section   Section.
     * @param[in] histogram Histogram.
     * @param[in] canvas    Canvas.
     */
    void processSpatial2DHitEndcapHistogram(
      uint16_t section, TH2F* histogram, TCanvas* canvas);

    /**
     * Process histogram containing the hit times.
     * @param[in]  histName  Histogram name.
     */
    void processTimeHistogram(const std::string& histName);

    /**
     * Process histogram containing the number of hits in plane.
     * @param[in]  histName  Histogram name.
     * @param[out] latex     TLatex to draw messages.
     */
    void processPlaneHistogram(const std::string& histName, TLatex& latex);


    /**
     * Fill histogram containing masked channels per sector.
     * @param[in] histName  Histogram name.
     */
    void fillMaskedChannelsHistogram(const std::string& histName);

    /**
     * Scales and draws desired delta histogram for current canvas.
     * @param[in] delta  Delta histogram.
     * @param[in] histogram  Base histogram (for normalization).
     * @param[in] canvas  Canvas with delta histogram.
     */
    void deltaDrawer(TH1* delta, TH1* histogram, TCanvas* canvas);

    /** Number of processed events. */
    double m_ProcessedEvents;

    /** Threshold for masked channels. */
    int m_ThresholdForMasked;

    /** Threshold for hot channels. */
    int m_ThresholdForHot;

    /** Threshold for log scale. */
    int m_ThresholdForLog;

    /** Minimal number of hits for flagging. */
    int m_MinHitsForFlagging;

    /** Input parameter for minimal number of processed events for error messages. */
    double m_MinProcessedEventsForMessagesInput;

    /** Minimal number of processed events for error messages. */
    double m_MinProcessedEventsForMessages;

    /** Minimal number of entries for delta histogram update. */
    double m_minEntries;

    /** Name of histogram directory */
    std::string m_histogramDirectoryName;

    /** Reference Histogram Root file name */
    std::string m_refFileName;

    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

    /** Vector of dead barrel modules. */
    std::vector<uint16_t> m_DeadBarrelModules;

    /** Vector of dead endcap modules. */
    std::vector<uint16_t> m_DeadEndcapModules;

    /** Vector of masked channels. */
    std::vector<uint16_t> m_MaskedChannels;

    /** TLine for background region in 2d hits histograms. */
    TLine m_2DHitsLine;

    /** TLine for boundary in plane histograms. */
    TLine m_PlaneLine;

    /** TText for names in plane histograms. */
    TText m_PlaneText;

    /** Run type flag for null runs. */
    bool m_IsNullRun;

    /** KLM channel array index. */
    const KLMChannelArrayIndex* m_ChannelArrayIndex;

    /** KLM sector array index. */
    const KLMSectorArrayIndex* m_SectorArrayIndex;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_EklmElementNumbers;

    /** Electronics map. */
    DBObjPtr<KLMElectronicsMap> m_ElectronicsMap;

    /** Monitoring object. */
    MonitoringObject* m_monObj {};

  };

}
