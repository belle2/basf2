/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisDeltaEpicsMonObjExample.h
// Description : DQM Analysis Delta+Epics+MonObj example
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

#include <TLine.h>

namespace Belle2 {
  /*! DQM Delta Histogram Test code and example */

  class DQMHistAnalysisDeltaEpicsMonObjExampleModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisDeltaEpicsMonObjExampleModule();

  private:

    /**
     * Destructor.
     */
    ~DQMHistAnalysisDeltaEpicsMonObjExampleModule();

    /**
     * Initializer.
     */
    void initialize(void) override final;

    /**
     * Called when entering a new run.
     */
    void beginRun(void) override final;

    /**
     * Called when run ends.
     */
    void endRun(void) override final;

    /**
     * This method is called for each event.
     */
    void event(void) override final;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate(void) override final;

    /**
     * Do the actual processing
     */
    void doHistAnalysis(bool forMiraBelle = false);

  private:

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! name of histogram
    std::string m_histogramName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! Final Canvas
    TCanvas* m_canvas = nullptr;

    /** Monitoring Object */
    MonitoringObject* m_monObj {};

    /** alarm limits from PV */
    double m_meanLowerAlarm{NAN};
    /** alarm limits from PV */
    double m_meanLowerWarn{NAN};
    /** alarm limits from PV */
    double m_meanUpperWarn{NAN};
    /** alarm limits from PV */
    double m_meanUpperAlarm{NAN};
    /** alarm limits from PV */
    double m_widthUpperWarn{NAN};
    /** alarm limits from PV */
    double m_widthUpperAlarm{NAN};

    /** TLine for alarm limits */
    TLine* m_meanLowerAlarmLine{nullptr};
    /** TLine for alarm limits */
    TLine* m_meanLowerWarnLine{nullptr};
    /** TLine for alarm limits */
    TLine* m_meanUpperWarnLine{nullptr};
    /** TLine for alarm limits */
    TLine* m_meanUpperAlarmLine{nullptr};

  };
} // end namespace Belle2

