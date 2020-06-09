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
    virtual ~DQMHistAnalysisKLMModule();

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

  protected:

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
     * Process histogram containing the number of hits in plane.
     * @param[in] histName  Histogram name.
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

    /** Electronics map. */
    DBObjPtr<KLMElectronicsMap> m_ElectronicsMap;

    /** Threshold for masked channels. */
    int m_ThresholdForMasked;

    /** Threshold for hot channels. */
    int m_ThresholdForHot;

    /** Minimal number of hits for flagging. */
    int m_MinHitsForFlagging;

    /** Vector of dead barrel modules. */
    std::vector<uint16_t> m_DeadBarrelModules;

    /** Vector of dead endcap modules. */
    std::vector<uint16_t> m_DeadEndcapModules;

    /** Vector of new channels to be masked. */
    std::vector<uint16_t> m_NewMaskedChannels;

    /** Vector of channels already masked. */
    std::vector<uint16_t> m_MaskedChannels;

    /** KLM channel array index. */
    const KLMChannelArrayIndex* m_ChannelArrayIndex;

    /** KLM sector array index. */
    const KLMSectorArrayIndex* m_SectorArrayIndex;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

    /** EKLM strip number within a layer. */
    TCanvas* m_eklmStripLayer[
      EKLMElementNumbers::getMaximalLayerGlobalNumber()];

    /** TLine for boundary in plane histograms. */
    TLine m_PlaneLine;

    /** TText for names in plane histograms. */
    TText m_PlaneText;

  };

}
