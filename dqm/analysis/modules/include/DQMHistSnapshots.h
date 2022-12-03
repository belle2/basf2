/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// EPICS
#ifdef _BELLE2_EPICS
#include "cadef.h"
// #include "dbDefs.h"
// #include "epicsString.h"
// #include "cantProceed.h"
#endif

#include <dqm/core/DQMHistAnalysis.h>
#include <TH1.h>
#include <TCanvas.h>
#include <time.h>

namespace Belle2 {
  /** Class for generating snapshots for histograms. */

  class DQMHistSnapshotsModule : public DQMHistAnalysisModule {

    /**
     * The struct for the snapshots.
     */
    typedef struct {
      /** The histogram for snapshot. */
      TH1* histo;
      /** The canvas for the histogram to be showed. */
      TCanvas* canvas;
      /** Whether the histogram is not updated for a long time. */
      int stale;
    } SSNODE;

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistSnapshotsModule();

    /**
     * Destructor.
     */
    ~DQMHistSnapshotsModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

    /**
     * Find a snapshot by the histogram's name.
     * @param a The name of the hisotgram in the snapshot.
     * @return The node struct for the snapshot, or nullptr if not found.
     */
    SSNODE* find_snapshot(TString a);

    // Data members
  private:
    /** List of snapshots. */
    std::vector<SSNODE*> m_ssnode;
    /** Interval between checks in second. */
    int m_check_interval;
    /** The time for the last check. 0 for the first check. */
    time_t m_last_check = 0;

  };
} // end namespace Belle2

