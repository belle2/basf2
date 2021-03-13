/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Leo Piilonen, Vipin Gaur,               *
 *               Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* DQM headers. */
#include <dqm/analysis/modules/DQMHistAnalysis.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/dbobjects/KLMElectronicsMap.h>

/* ROOT headers. */
#include <TCanvas.h>
#include <TH1.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TText.h>
#include <TLine.h>

/* C++ headers. */
#include <vector>

namespace Belle2 {

  /**
   * Analysis of KLM DQM histograms.
   */
  class DQMHistAnalysisKLMModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisKLMModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisKLMModule();

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
     * Get number of processed events.
     */
    double getProcessedEvents();

    /**
     * Analyse channel hit histogram.
     * @param[in]  subdetector  Subdetector.
     * @param[in]  section      Section.
     * @param[in]  sector       Sector.
     * @param[in]  histogram    Histogram.
     * @param[in]  canvas       Canvas.
     * @param[out] latex        TLatex to draw messages.
     */
    void analyseChannelHitHistogram(
      int subdetector, int section, int sector,
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
     * Find TCanvas that matches a given name.
     * @param[in]  canvasName  Name of the desired TCanvas.
     * @param[out] TCanvas*    Matching TCanvas.
     */
    TCanvas* findCanvas(const std::string& canvasName);

    /** Number of processed events. */
    double m_ProcessedEvents;

    /** Threshold for masked channels. */
    int m_ThresholdForMasked;

    /** Threshold for hot channels. */
    int m_ThresholdForHot;

    /** Minimal number of hits for flagging. */
    int m_MinHitsForFlagging;

    /** Input parameter for minimal number of processed events for error messages. */
    double m_MinProcessedEventsForMessagesInput;

    /** Minimal number of processed events for error messages. */
    double m_MinProcessedEventsForMessages;

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

  };

}
