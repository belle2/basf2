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
#include <dqm/analysis/modules/DQMHistAnalysis.h>

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

    //! Constructor / Destructor
    DQMHistAnalysisInputModule();
    virtual ~DQMHistAnalysisInputModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:
    /** Memory file to hold histograms. */
    DqmMemFile* m_memory = nullptr;
    /** The name of the shared memory for the histograms. */
    std::string m_mempath;
    /** The name of the memory file (HLT or ExpressReco). */
    std::string m_memname;
    /** The shmid for the shared memory. */
    int m_shm_id;
    /** The semid for the shared memory. */
    int m_sem_id;
    /** The size of the shared memory. */
    int m_memsize;
    /** The refresh interval. */
    int m_interval;
    /** Whether automatically generate canvases for histograms. */
    bool m_autocanvas;
    /** Whether to remove empty histograms. */
    bool m_remove_empty;
    /** Whether to enable the run info to be displayed. */
    bool m_enable_run_info;
    /** The list of folders for which automatically generate canvases. */
    std::vector<std::string> m_acfolders;
    /** The list of folders which are excluded from automatically generate canvases. */
    std::vector<std::string> m_exclfolders;
    /** The canvas hold the basic DQM info. */
    TCanvas* m_c_info = nullptr;

    /** The metadata for each event. */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    /** The list of canvases for output. */
    std::map<std::string, TCanvas*> m_cs;

    /** Exp number */
    unsigned int m_expno = 0;
    /** Run number */
    unsigned int m_runno = 0;
    /** Event number */
    unsigned int m_count = 0;
  };
} // end namespace Belle2

