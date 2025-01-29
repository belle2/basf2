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
#include <map>

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
     * Updates canvas of number of good hits per event w/ alarming (injection BG)
     */
    void updateNGoodHitsCanvas();

    /**
     * Updates canvas of event T0 w/ alarming
     */
    void updateEventT0Canvas();

    /**
     * Updates canvas of bunch offset w/ alarming
     */
    void updateBunchOffsetCanvas();

    /**
     * Updates canvas of timing plot w/ alarming
     */
    void updateTimingCanvas();

    /**
     * Checks if histograms are defined in the same way (nbins, xmin, xmax)
     * @param h1 first histogram
     * @param h2 second histogram
     * @return true if (nbins, xmin, xmax) match
     */
    bool sameHistDefinition(TH1* h1, TH1* h2);

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
     * Makes a plot of fractions of junk hits per boardstack
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
     * @param trackHits histogram used to scale background in case it is available
     * @param slot slot number
     */
    void makeBGSubtractedTimingPlot(const std::string& name, const TH2F* trackHits, int slot);

    /**
     * Makes plots of the number of PMT hits per event
     */
    void makePMTHitRatesPlots();

    /**
     * Makes projections of injection BG plots
     */
    void makeInjectionBGPlots();

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
     * Converts alarm state to official status (see EStatus of the base class)
     * @param alarmState alarm state
     * @return alarm status
     */
    int getOffcialAlarmStatus(unsigned alarmState) const
    {
      if (alarmState < m_officialStates.size()) return m_officialStates[alarmState];
      return c_StatusDefault;
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
     * Returns cut levels for dead and hot channels
     * @param h pixel or channel distribution of hits (1D or 2D histogram)
     * @return cut levels (first = dead, second = hot)
     */
    std::pair<double, double> getDeadAndHotCuts(const TH1* h);

    /**
     * Calculates and sets epics variables
     */
    void setEpicsVariables();

    /**
     * Updates limits defined by module parameters using EpicsPVs.
     * This includes asicWindowsBand, alarmLevels and excluded boardstacks.
     */
    void updateLimits();

    /**
     * Sets flags for boardstacks to be included in alarming
     * @param excludedBoardstacks list of boarstacks to be excluded from alarming
     */
    void setIncludedBoardstacks(const std::vector<std::string>& excludedBoardstacks);

    // module parameters

    std::vector<int> m_asicWindowsBand = {215, 235}; /**< lower and upper bin of a band denoting good windows */
    std::vector<double> m_asicWindowsAlarmLevels = {0.002, 0.02}; /**< alarm levels for fraction of windows outside the band */
    std::vector<double> m_eventMonitorAlarmLevels = {1e-4, 2e-3}; /**< alarm levels for fraction of desynchronized digits */
    std::vector<double> m_junkHitsAlarmLevels = {0.05, 0.25}; /**< alarm levels for the fraction of junk hits */
    std::vector<double> m_deadChannelsAlarmLevels = {0.1, 0.35}; /**< alarm levels for the fraction of dead + hot channels */
    std::vector<double> m_backgroundAlarmLevels = {5.0, 10.0}; /**< alarm levels for background rates [MHz/PMT] */
    std::vector<double> m_photonYieldsAlarmLevels = {15.0, 25.0}; /**< alarm levels for the number of photons per track */
    std::vector<std::string> m_excludedBoardstacks; /**< list of boarstacks to be excluded from alarming */
    std::string  m_pvPrefix;  /**< Epics PV prefix */

    std::vector<double> m_injectionBGAlarmLevels = {5, 10}; /**< alarm levels for injection background (in % of events) */
    std::vector<double> m_timingAlarmLevels = {0.15, 0.30}; /**< alarm levels for time distribution (fraction of area difference) */
    std::vector<double> m_eventT0MeanAlarmLevels = {8, 20}; /**< alarm levels for mean of event T0 [ns] */
    std::vector<double> m_eventT0RmsAlarmLevels = {10, 20}; /**< alarm levels for r.m.s. of event T0 [ ns] */
    std::vector<double> m_offsetMeanAlarmLevels = {0.2, 0.5}; /**< alarm levels for mean of bunch offset [ns] */
    std::vector<double> m_offsetRmsAlarmLevels = {0.25, 0.50}; /**< alarm levels for r.m.s. of bunch offset [ns] */

    // other

    std::vector<int> m_alarmColors = {c_ColorTooFew, c_ColorGood, c_ColorWarning, c_ColorError}; /**< alarm colors (see base class) */
    std::vector<int> m_officialStates = {c_StatusTooFew, c_StatusGood, c_StatusWarning, c_StatusError}; /**< official alarm states */
    std::vector<bool> m_includedBoardstacks; /**< boardstacks included in alarming */
    std::map<std::string, int> m_bsmap;  /**< a map of boardstack names to ID's */
    int m_alarmStateOverall = 0; /**< overall alarm state of histograms to be sent by EpicsPV */
    double m_averageRate = 0; /**< average BG rate (to pass to EpicsPV) */

    bool m_IsNullRun = false; /**< Run type flag for null runs. */
    std::string m_runType; /**< Run type */
    double m_numEvents = 0; /**< number of events processed with TOPDQM module */

    TH1D* m_photonYields = nullptr; /**< photon yields per slot */
    TH1D* m_backgroundRates = nullptr; /**< background rates per slot */
    TCanvas* m_c_photonYields = nullptr; /**< Canvas: photon yields per slot */
    TCanvas* m_c_backgroundRates = nullptr; /**< Canvas: background rates per slot */

    TH1F* m_hotFraction = nullptr; /**< fraction of hot channels per slot (included boardstacks only) */
    TH1F* m_deadFraction = nullptr; /**< fraction of dead channels per slot (included boardstacks only) */
    TH1F* m_excludedFraction = nullptr; /**< fraction of dead and hot channels per slot in excluded boardstacks only */
    TH1F* m_activeFraction = nullptr; /**< fraction of active channels per slot */
    THStack* m_stack = nullptr;  /**< stack for drawing dead, hot and active channel fractions */
    TLegend* m_legend = nullptr; /**< legend for dead and hot channels */
    TCanvas* m_c_deadAndHot = nullptr; /**< Canvas: fractin of dead and hot channels */

    TH1F* m_junkFraction = nullptr; /**< fraction of junk hits per boardstack */
    TH1F* m_excludedBSHisto = nullptr; /**< histogram to show excluded boardstacks on junk fraction plot */
    TCanvas* m_c_junkFraction = nullptr; /**< Canvas: fraction of junk hits per boardstack */

    std::vector<TH1F*> m_pmtHitRates; /**< histograms of PMT hits per event (index = slot - 1) */
    std::vector<TCanvas*> m_c_pmtHitRates; /**< Canvases of PMT hits per event (index = slot - 1) */

    std::map<std::string, TCanvas*> m_c_injBGs; /**< Canvases for projections of injection BG histograms */
    std::map<std::string, TProfile*> m_profiles; /**< profiles of injection BG */
    std::map<std::string, TH1D*> m_projections; /**< projections of injection BG */

    std::vector<TLine*> m_asicWindowsBandLines; /**< lines denoting a band of good windows */
    std::vector<TLine*> m_verticalLines; /**< vertical lines splitting slots */
    std::vector<TLine*> m_junkHitsAlarmLines; /**< lines representing alarm levels */
    std::vector<TLine*> m_deadChannelsAlarmLines; /**< lines representing alarm levels */
    std::vector<TLine*> m_backgroundAlarmLines; /**< lines representing alarm levels */
    std::vector<TLine*> m_photonYieldsAlarmLines; /**< lines representing alarm levels */
    TLine* m_injBGCutLine = nullptr; /**< a line denoting the cut on the number of hits for injection BG counting */

    TPaveText* m_text1 = nullptr; /**< text to be written to window_vs_slot */
    TPaveText* m_text2 = nullptr; /**< text to be written to event desynchonization monitor */
    TPaveText* m_text3 = nullptr; /**< text to be written to background rates */
    TPaveText* m_text4 = nullptr; /**< text to be written to number of good hits per event */

    std::map<std::string, double> m_mirabelleVariables; /**< variables for MiraBelle */
    MonitoringObject* m_monObj = nullptr; /**< MiraBelle monitoring object */

  };
} // end namespace Belle2

