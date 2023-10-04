/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisInput.h
// Description : Input module for DQM Histogram analysis
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <daq/dqm/DqmMemFile.h>
#include <dqm/core/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TKey.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisInputModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisInputModule();

    /**
     * Destructor.
     */
    virtual ~DQMHistAnalysisInputModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;
    /** The map of histogram names to canvas pointers for output. */
    std::map<std::string, TCanvas*> m_cs;

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

    // Data members
  private:
    /** Memory file to hold histograms. */
    DqmMemFile* m_memory = nullptr;
    /** The name of the shared memory for the histograms. */
    std::string m_mempath;
    /** The name of the memory file (HLT or ExpressReco). */
    std::string m_memname;
    /** The owner of the memory file (HLT or ExpressReco). */
    std::string m_username;
    /** The shmid for the shared memory. */
    int m_shm_id{-1};
    /** The semid for the shared memory. */
    int m_sem_id{-1};
    /** The size of the shared memory. */
//    int m_memsize; // this is not used in code!!! it is hard coded in DqmMemFile to 4*128MB
    /** The refresh interval. */
    int m_interval;
    /** Whether to remove empty histograms. */
    bool m_remove_empty;
    /** Whether to enable the run info to be displayed. */
    bool m_enable_run_info;
    /** The canvas hold the basic DQM info. */
    TCanvas* m_c_info = nullptr;

    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** Exp number */
    unsigned int m_expno = 0;
    /** Run number */
    unsigned int m_runno = 0;
    /** Event number */
    unsigned int m_count = 0;
  };
} // end namespace Belle2

