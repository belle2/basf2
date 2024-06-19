/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistDeltaHisto.h
// Description :
//-

#pragma once

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <dqm/core/DQMHistAnalysis.h>
#include <TH1.h>
#include <TCanvas.h>
#include <time.h>
#include <map>
#include <queue>
#include <vector>

namespace Belle2 {
  /** Class for generating snapshots for histograms. */

  class DQMHistDeltaHistoModule final : public DQMHistAnalysisModule {

    /**
     * The struct for the snapshots.
     */
    typedef struct {
      /** The histogram for snapshot. */
      TH1* histo;
      /** The histogram for snapshot. */
      TH1* diff_histo;
      /** Whether the histogram is not updated for a long time. */
      time_t time_modified;
    } SSNODE;

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistDeltaHistoModule();

    /**
     * Destructor.
     */
    ~DQMHistDeltaHistoModule();

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

    /**
     * Clear content of SSNODE
     * @param n Pointer to the SSNODE
     */
    void clear_node(SSNODE* n);

    // Data members
  private:
    /** Names of the histograms that should be monitored */
    std::vector<std::string> m_monitoredHistos;
    /** Map of histogram names to queues of monitoring objects */
    std::map<std::string, std::queue<SSNODE*> > m_histosQueues;
    /** Interval between checks in second. */
    int m_interval;
    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_evtMetaDataPtr;

  };
} // end namespace Belle2

