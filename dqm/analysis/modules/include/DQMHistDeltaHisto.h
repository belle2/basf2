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

  class DQMHistDeltaHistoModule : public DQMHistAnalysisModule {

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
    virtual ~DQMHistDeltaHistoModule();

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

    /**
     * Clear content of SSNODE
     * @param n Pointer to the SSNODE
     */
    void clear_node(SSNODE* n);

    // Data members
  private:
    /** Names of the histograms that should be monitored */
    std::vector<std::string> m_monitored_histos;
    /** Map of histogram names to queues of monitoring objects */
    std::map<std::string, std::queue<SSNODE*> > m_histos_queues;
    /** Interval between checks in second. */
    int m_interval;
    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_evtMetaDataPtr;

  };
} // end namespace Belle2

