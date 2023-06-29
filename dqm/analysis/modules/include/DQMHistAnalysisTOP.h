/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TString.h>
#include <TLine.h>
#include <TPaveText.h>
#include <vector>

namespace Belle2 {
  /**
   * Class for TOP histogram analysis.
   */

  class DQMHistAnalysisTOPModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisTOPModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisTOPModule();

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
     * Remakes plots of 2D hit distributions
     * @param name the name of histograms
     * @param canvases list of canvases
     * @scale scale scale factor for the histogram range (maximum = average * scale)
     */
    void remake2DHistograms(const TString& name, const std::vector<TCanvas*>& canvases, double scale);

    /**
     * Sets MiraBelle variables from the histogram where each bin corresponds to a slot number
     * @param variableName variable name
     * @param histogram histogram with bins corresponding to slot numbers
     */
    void setMiraBelleVariables(const TString& variableName, const TH1* histogram);

    /**
     * Returns alarm colors
     * @param value value to translate to color
     * @param alarmBorders alarm borders
     * @return root color index
     */
    int getAlarmColor(double value, const std::vector<double>& alarmBorders) const;

    // module parameters

    std::vector<int> m_rawTimingBand = {215, 235}; /**< lower and upper bin of a band */
    std::vector<double> m_rawTimingAlarmBorders = {0.001, 0.01}; /**< alarm borders for raw timing */
    std::vector<double> m_eventMonitorAlarmBorders = {0.00001, 0.0001}; /**< alarm borders for event desynchronization monitor */
    std::vector<double> m_badHitsAlarmBorders = {0.05, 0.25}; /**< alarm borders for the fraction of junk hits */
    std::vector<double> m_deadChannelsAlarmBorders = {0.05, 0.25}; /**< alarm borders for the fraction of dead + hot channels */

    // other

    TH1* m_RunType = nullptr; /**< Histogram from DQMInfo with run type. */
    TString m_RunTypeString; /**< String with run type. */
    bool m_IsNullRun = false; /**< Run type flag for null runs. */

    TH1D* m_windowFractions = nullptr; /**< fraction of windows outside the band per slot */
    double m_totalWindowFraction = 0;  /**< total fraction of windows outside the band */

    std::vector<TCanvas*> m_c_good_hits_xy; /**< Canvases for good hits in pixels */
    std::vector<TCanvas*> m_c_bad_hits_xy;  /**< Canvases for bad hits in pixels */
    std::vector<TCanvas*> m_c_good_hits_asics; /**< Canvases for good hits in asics */
    std::vector<TCanvas*> m_c_bad_hits_asics;  /**< Canvases for bad hits in asics */

    TH1D* m_photonYields = nullptr; /**< photon yields per slot (corrected for active channels) */
    TCanvas* m_c_photonYields = nullptr; /**< Canvas: photon yields per slot */

    TH1D* m_backgroundRates = nullptr; /**< background rates per slot */
    TCanvas* m_c_backgroundRates = nullptr; /**< Canvas: background rates per slot */

    TH1F* m_hotFraction = nullptr; /**< Histogram: fractin of hot channels per slot */
    TH1F* m_deadFraction = nullptr; /**< Histogram: fractin of dead channels per slot */
    TH1F* m_activeFraction = nullptr; /**< Histogram: fractin of active channels per slot */
    TCanvas* m_c_deadAndHot = nullptr; /**< Canvas: fractin of dead and hot channels */

    TLine* m_line1 = nullptr; /**< The line for the upper bound of the nornal window. */
    TLine* m_line2 = nullptr; /**< The line for the lower bound of the nornal window. */
    TPaveText* m_text1 = nullptr; /**< The text for the conditions of the nornal window. */

    MonitoringObject* m_monObj = nullptr; /**< MiraBelle monitoring object. */

  };
} // end namespace Belle2

