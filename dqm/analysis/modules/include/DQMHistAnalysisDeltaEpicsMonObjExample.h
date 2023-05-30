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

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Delta Histogram Test code and example */

  class DQMHistAnalysisDeltaEpicsMonObjExampleModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisDeltaEpicsMonObjExampleModule();

    /**
     * Destructor.
     */
    ~DQMHistAnalysisDeltaEpicsMonObjExampleModule();

  private:

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
     * Do the actal processing
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

    /** flag if to export to EPICS */
    bool m_useEpics;

#ifdef _BELLE2_EPICS
    //! vector for EPICS PVs
    std::vector <chid> mychid;
#endif
  };
} // end namespace Belle2

