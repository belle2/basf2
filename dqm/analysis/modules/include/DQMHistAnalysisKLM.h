/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Leo Piilonen                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TROOT.h>
#include <TClass.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TText.h>
#include <TLine.h>

/* Belle2 headers. */
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>

#define NSECTORFB 16

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
     * Analyse strip number histogram.
     * @param[in]  layerGlobal Global layer number.
     * @param[in]  histogram   Histogram.
     * @param[out] latex       TLatex to draw messages.
     */
    void analyseStripLayerHistogram(int layerGlobal, TH1* histogram,
                                    TLatex& latex);

    /**
     * Process one BKLM sector+layer histogram.
     * @param[in]  histName  Histogram name.
     */
    void processBKLMSectorLayerHistogram(const std::string& histName);

    /**
     * Find TCanvas that matches a given name.
     * @param[in]  canvasName   name of the desired TCanvas.
     * @param[out] TCanvas*     matching TCanvas.
     */
    TCanvas* findCanvas(const std::string& canvasName);

    /** Electronics map. */
    DBObjPtr<EKLMElectronicsMap> m_ElectronicsMap;

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_ElementNumbers;

    /** EKLM strip number within a layer. */
    TCanvas* m_eklmStripLayer[
      EKLMElementNumbers::getMaximalLayerGlobalNumber()];

    /** TLine for BKLM sector boundary in histogram. */
    TLine* m_sectorLine[NSECTORFB];

    /** TText for BKLM sector name in histogram. */
    TText* m_sectorText[NSECTORFB];

  };

}
