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
#include <string>

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
     * Remakes plots of 2D hit distributions.
     * @param name the name of histograms
     * @param canvases list of canvases
     * @param scale scale factor for the histogram z-axis range (maximum = average * scale)
     */
    void remake2DHistograms(const std::string& name, const std::vector<TCanvas*>& canvases, double scale);

    /**
     * Sets MiraBelle variables from the histogram with bins corresponding to slot numbers.
     * @param variableName variable name
     * @param histogram histogram with bins corresponding to slot numbers
     */
    void setMiraBelleVariables(const std::string& variableName, const TH1* histogram);

    /**
     * Returns alarm color.
     * @param value value to translate to color
     * @param alarmBorders alarm borders
     * @return root color index
     */
    int getAlarmColor(double value, const std::vector<double>& alarmBorders) const;

    // module parameters

    std::vector<int> m_rawTimingBand = {215, 235}; /**< lower and upper bin of a band denoting good windows */
    std::vector<double> m_rawTimingAlarmBorders = {0.001, 0.01}; /**< alarm borders for fraction of windows outside the band */
    std::vector<double> m_eventMonitorAlarmBorders = {0.00001, 0.0001}; /**< alarm borders for fraction of desynchronized digits */
    std::vector<double> m_badHitsAlarmBorders = {0.05, 0.25}; /**< alarm borders for the fraction of junk hits */
    std::vector<double> m_deadChannelsAlarmBorders = {0.05, 0.25}; /**< alarm borders for the fraction of dead + hot channels */

    // other

    TH1* m_RunType = nullptr; /**< Histogram from DQMInfo with run type. */
    TString m_RunTypeString; /**< String with run type. */
    bool m_IsNullRun = false; /**< Run type flag for null runs. */

    TH1D* m_windowFractions = nullptr; /**< fraction of windows outside the band denoting good windows, per slot */
    double m_totalWindowFraction = 0;  /**< total fraction of windows outside the band */

    TH1D* m_photonYields = nullptr; /**< photon yields per slot (corrected for active channels) */
    TCanvas* m_c_photonYields = nullptr; /**< Canvas: photon yields per slot */

    TH1D* m_backgroundRates = nullptr; /**< background rates per slot */
    TCanvas* m_c_backgroundRates = nullptr; /**< Canvas: background rates per slot */

    TH1F* m_hotFraction = nullptr; /**< fractin of hot channels per slot */
    TH1F* m_deadFraction = nullptr; /**< fractin of dead channels per slot */
    TH1F* m_activeFraction = nullptr; /**< fractin of active channels per slot */
    TCanvas* m_c_deadAndHot = nullptr; /**< Canvas: fractin of dead and hot channels */

    TH1F* m_junkFraction = nullptr; /**< fraction of bad hits per boardstack */
    TCanvas* m_c_junkFraction = nullptr; /**< Canvas: fraction of bad hits per boardstack */

    std::vector<TCanvas*> m_c_good_hits_xy; /**< Canvases for good hits (pixel representation) */
    std::vector<TCanvas*> m_c_bad_hits_xy;  /**< Canvases for bad hits (pixel representation) */
    std::vector<TCanvas*> m_c_good_hits_asics; /**< Canvases for good hits (asic representation) */
    std::vector<TCanvas*> m_c_bad_hits_asics;  /**< Canvases for bad hits (asic representation) */

    TLine* m_line1 = nullptr; /**< line denoting lower bound of good windows */
    TLine* m_line2 = nullptr; /**< line denoting upper bound of good windows */
    TPaveText* m_text1 = nullptr; /**< text to be written to window_vs_slot */
    TPaveText* m_text2 = nullptr; /**< text to be written to event desynchonization monitor */
    std::vector<TLine*> m_verticalLines; /**< vertical lines splitting slots */
    std::vector<TLine*> m_badHitsAlarmLines; /**< lines representing alarm borders */
    std::vector<TLine*> m_deadChannelsAlarmLines; /**< lines representing alarm borders */

    MonitoringObject* m_monObj = nullptr; /**< MiraBelle monitoring object */

  };
} // end namespace Belle2

