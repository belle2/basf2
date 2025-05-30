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
     * @param[in]  subdetector  Subdetector (E or B-KLM).
     * @param[in]  section      Section (Forward or Backward).
     * @param[in]  sector       Sector.
     * @param[in]  index        Histogram Index (The j'th histogram of a given sector).
     * @param[in]  histogram    Histogram.
     * @param[in]  delta        Delta Histogram.
     * @param[in]  canvas       Canvas.
     * @param[out] latex        TLatex to draw messages.
     */
    void analyseChannelHitHistogram(
      int subdetector, int section, int sector, int index,
      TH1* histogram, TH1* delta, TCanvas* canvas, TLatex& latex);

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
     * @param[in]  histogram Histogram.
     * @param[out] latex     TLatex to draw messages.
     */
    void processPlaneHistogram(const std::string& histName, TLatex* latex, TH1* histogram);

    /**
     * Process histogram containing the number of hits in plane.
     * @param[in]  histName  Histogram name.
     * @param[out] latex     TLatex to draw messages.
     */
    // Overloaded function for backward compatibility (TLatex reference)
    // so that the above function can be used with or without TLatex for FE histograms.
    void processPlaneHistogram(const std::string& histName, TLatex& latex);

    /**
    * Helper function to update the canvas status based on dead modules.
    * @param[in] canvas Canvas of interest.
    * @param[in] deadModules List of dead modules.
    * @param[in] latex LaTeX object to render text annotations.
    * @param[out] message_counter Counter for how many messages are drawn.
    * @param[out] xAlarm X-position for the alarm messages.
    * @param[out] yAlarm Y-position for the alarm messages.
    */
    void updateCanvasStatus(
      TCanvas* canvas, const std::vector<KLMModuleNumber>& deadModules,
      TLatex* latex, int& message_counter, double xAlarm, double yAlarm);

    /**
     * Process histogram containing the efficiencies.
     * @param[in]  feHist  Histogram itself.
     * @param[in]  denominator Denominator for efficiency hist.
     * @param[in]  numerator Numerator for efficiency hist.
     * @param[in]  canvas Canvas of interest.
     */
    void processFEHistogram(TH1* feHist,  TH1* denominator, TH1* numerator, TCanvas* canvas);

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

    /** Message Threshold for expert pots */
    int m_MessageThreshold;

    /** Input parameter for minimal number of processed events for error messages. */
    double m_MinProcessedEventsForMessagesInput;

    /** Minimal number of processed events for error messages. */
    double m_MinProcessedEventsForMessages;

    /** Minimal number of entries for delta histogram update. */
    double m_minEntries;

    /** Name of histogram directory */
    std::string m_histogramDirectoryName;

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

    /** Histogram for BKLM plane events fraction w/ FE. */
    TH1* m_fe_bklm_ratio = nullptr;

    /** Canvas for BKLM plane events fraction w/ FE.  */
    TCanvas* m_c_fe_bklm_ratio = nullptr;

    /** Histogram for EKLM plane events fraction w/ FE. */
    TH1* m_fe_eklm_ratio = nullptr;

    /** Canvas for EKLM plane events fraction w/ FE. */
    TCanvas* m_c_fe_eklm_ratio = nullptr;

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
