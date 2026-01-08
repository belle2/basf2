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


/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * Analysis of histograms from DetectorOccupancyMoudle
   */
  class DQMHistAnalysisDetectorOccupancyModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisDetectorOccupancyModule();

    /** Destructor. */
    ~DQMHistAnalysisDetectorOccupancyModule();

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


  private:

    /**
     * Process histogram containing the efficiencies.
     * @param[in]  totalHist      Occupancy Plot with all triggers
     * @param[in]  trgHist        Occupancy Plot with background triggers
     * @param[in]  backRatio      ratio variable address in backward region
     * @param[in]  backErrRatio   ratio uncertainty address in backward region
     * @param[in]  forwardRatio      ratio variable address in backward region
     * @param[in]  forwardErrRatio   ratio uncertainty address in backward region
     */
    void KLMOccupancyProcessing(TH1* totalHist, TH1* trgHist,
                                Double_t& backRatio, Double_t& backErrRatio,
                                Double_t& forwardRatio, Double_t& forwardErrRatio);

    /**
     * Process histogram containing the efficiencies.
     * @param[in]  totalHist  Occupancy Plot with all triggers
     * @param[in]  trgHist    Occupancy Plot with background triggers
     * @param[in]  bin1    First bin for Integral argument
     * @param[in]  bin2    Second bin for argument
     * @param[in]  ratio    ratio variable address used to save value
     * @param[in]  ratioErr    ratio error variable address used to save value
     */
    void HistToRatio(TH1* totalHist,  TH1* trgHist, int bin1, int bin2, Double_t& ratio, Double_t& ratioErr);


    /** Run type flag for null runs. */
    bool m_IsPhysicsRun = false;

    /** Run type flag for null runs. */
    bool m_IsNullRun;

    /** KLM Monitoring object. */
    MonitoringObject* m_klmMonObj {};

    /** Name of histogram directory */
    std::string m_histogramDirectoryName;


    /** Tag suffix based on injection veto */
    const std::string m_tag[2] = {"OUT", "IN"};

    /** Name of histogram suffix based on tag */
    const std::string m_title[2] = {"[Outside Active Veto Window]", "[Inside Active Veto Window]"};

  };//class definition end

}//namespace end
