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
#include <dqm/analysis/modules/DQMHistAnalysis.h>
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

    //! Constructor / Destructor
    DQMHistDeltaHistoModule();
    virtual ~DQMHistDeltaHistoModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;
    /**
     * Find canvas by name
     * @param s Name of the canvas
     * @return The pointer to the canvas, or nullptr if not found.
     */
    TCanvas* find_canvas(TString s);
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

