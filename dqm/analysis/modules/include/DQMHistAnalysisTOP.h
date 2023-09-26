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
#include <TH2.h>
#include <TLine.h>
#include <TPaveText.h>
#include <THStack.h>
#include <TLegend.h>
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
     * Alarm states enumerator
     */
    enum EAlarmStates {
      c_Gray = 0,    // undefined
      c_Green = 1,   // good
      c_Yellow = 2,  // warning
      c_Red = 3      // alarm
    };

    /**
     * Updates canvas of window_vs_slot w/ alarming
     */
    void updateWindowVsSlotCanvas();

    /**
     * Updates canvas of event desynchronization monitor w/ alarming
     */
    void updateEventMonitorCanvas();

    /**
     * Makes a plot of dead and hot channel fractions per slot
     * @return fraction of active channels per slot
     */
    const TH1F* makeDeadAndHotFractionsPlot();


    /**
     * Make plots of dead-and-hot-channel corrected photon yields and BG rates per slot.
     * @param activeFraction fraction of active channels per slot
     */
    void makePhotonYieldsAndBGRatePlots(const TH1F* activeFraction);

    /**
     * Makes a plot of fractions of bad hits per boardstack
     */
    void makeJunkFractionPlot();

    /**
     * Sets z-axis range of 2D histograms
     * @param name the name of histograms
     * @param scale scale factor for the histogram z-axis range (maximum = average * scale)
     */
    void setZAxisRange(const std::string& name, double scale);

    /**
     * Makes background subtracted time distribution plot
     * @param name the name of the histogram
     */
    void makeBGSubtractedTimimgPlot(const std::string& name);

    /**
     * Sets MiraBelle variables from the histogram with bins corresponding to slot numbers.
     * @param variableName variable name
     * @param histogram histogram with bins corresponding to slot numbers
     */
    void setMiraBelleVariables(const std::string& variableName, const TH1* histogram);

    /**
     * Returns alarm state.
     * @param value value to translate to color
     * @param alarmLevels alarm levels
     * @param bigRed on true red color for large values, else red color for small values
     * @return alarm state (to be used as an index of m_alarmColors array)
     */
    int getAlarmState(double value, const std::vector<double>& alarmLevels, bool bigRed = true) const;

    /**
     * Converts alarm state to color
     * @param alarmState alarm state
     * @return alarm color
     */
    int getAlarmColor(unsigned alarmState) const
    {
      if (alarmState < m_alarmColors.size()) return m_alarmColors[alarmState];
      return m_alarmColors[c_Gray];
    }

    /**
     * Sets alarm lines.
     * @param alarmLevels alarm levels
     * @param xmin minimal x
     * @param xmax maximal x
     * @param alarmLines alarm lines [out]
     * @param bigRed on true red color for large values, else red color for small values
     */
    void setAlarmLines(const std::vector<double>& alarmLevels, double xmin, double xmax, std::vector<TLine*>& alarmLines,
                       bool bigRed = true);

    /**
     * Sets all alarm lines.
     */
    void setAlarmLines();

    /**
     * Returns histogram mean by excluding bins with zero content
     * @param h 2D histogram
     * @return mean
     */
    double getMean(const TH2* h);

    /**
     * Calculates and sets epics variables
     */
    void setEpicsVariables();

    /**
     * Updates limits defined by module parameters using EpicsPVs.
     * This includes rawTimingBand, alarmLevels and excluded boardstacks.
     */
    void updateLimits();

    // module parameters

    std::vector<int> m_rawTimingBand = {215, 235}; /**< lower and upper bin of a band denoting good windows */
    std::vector<double> m_rawTimingAlarmLevels = {0.002, 0.02}; /**< alarm levels for fraction of windows outside the band */
    std::vector<double> m_eventMonitorAlarmLevels = {1e-4, 2e-3}; /**< alarm levels for fraction of desynchronized digits */
    std::vector<double> m_badHitsAlarmLevels = {0.05, 0.25}; /**< alarm levels for the fraction of junk hits */
    std::vector<double> m_deadChannelsAlarmLevels = {0.1, 0.35}; /**< alarm levels for the fraction of dead + hot channels */
    std::vector<double> m_backgroundAlarmLevels = {5.0, 10.0}; /**< alarm levels for background rates [MHz/PMT] */
    std::vector<double> m_photonYieldsAlarmLevels = {15.0, 25.0}; /**< alarm levels for the number of photons per track */
    std::vector<std::string> m_excludedBoardstacks; /**< list of boarstacks to be excluded from alarming */
    std::string  m_pvPrefix;  /**< Epics PV prefix */

    // other

    std::vector<int> m_alarmColors = {kGray, kGreen, kYellow, kRed}; /**< alarm colors */
    std::vector<bool> m_includedBoardstacks; /**< boardstacks included in alarming */

    bool m_IsNullRun = false; /**< Run type flag for null runs. */

    TH1D* m_windowFractions = nullptr; /**< fraction of windows outside the band denoting good windows, per slot */
    double m_totalWindowFraction = 0;  /**< total fraction of windows outside the band */

    TH1D* m_photonYields = nullptr; /**< photon yields per slot (corrected for active channels) */
    TCanvas* m_c_photonYields = nullptr; /**< Canvas: photon yields per slot */

    TH1D* m_backgroundRates = nullptr; /**< background rates per slot */
    TCanvas* m_c_backgroundRates = nullptr; /**< Canvas: background rates per slot */

    TH1F* m_hotFraction = nullptr; /**< fraction of hot channels per slot */
    TH1F* m_deadFraction = nullptr; /**< fraction of dead channels per slot */
    TH1F* m_activeFraction = nullptr; /**< fraction of active channels per slot */
    THStack* m_stack = nullptr;  /**< stack for drawing dead, hot and active channel fractions */
    TLegend* m_legend = nullptr; /**< legend for dead and hot channels */
    TCanvas* m_c_deadAndHot = nullptr; /**< Canvas: fractin of dead and hot channels */

    TH1F* m_junkFraction = nullptr; /**< fraction of bad hits per boardstack */
    TCanvas* m_c_junkFraction = nullptr; /**< Canvas: fraction of bad hits per boardstack */

    std::vector<TLine*> m_rawTimingBandLines; /**< lines denoting a band of good windows */
    std::vector<TLine*> m_verticalLines; /**< vertical lines splitting slots */
    std::vector<TLine*> m_badHitsAlarmLines; /**< lines representing alarm levels */
    std::vector<TLine*> m_deadChannelsAlarmLines; /**< lines representing alarm levels */
    std::vector<TLine*> m_backgroundAlarmLines; /**< lines representing alarm levels */
    std::vector<TLine*> m_photonYieldsAlarmLines; /**< lines representing alarm levels */

    TPaveText* m_text1 = nullptr; /**< text to be written to window_vs_slot */
    TPaveText* m_text2 = nullptr; /**< text to be written to event desynchonization monitor */
    TPaveText* m_text3 = nullptr; /**< text to be written to background rates */

    MonitoringObject* m_monObj = nullptr; /**< MiraBelle monitoring object */

  };
} // end namespace Belle2

