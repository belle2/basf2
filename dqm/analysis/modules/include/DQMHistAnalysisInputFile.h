//+
// File : DQMHistAnalysisInputFile.h
// Description : Input module for DQM Histogram analysis
//
// Author : Boqun Wang
// Date : Jun - 2018
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <daq/dqm/DqmMemFile.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TFile.h>
#include <TCanvas.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisInputFileModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisInputFileModule();

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    // Data members
  private:
    //int m_interval;
    std::string m_input_name;
    TFile* m_file;

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    std::map<std::string, TCanvas*> m_cs;
    std::vector<std::string> m_folders;

    //! Exp number, Run number
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;
  };
} // end namespace Belle2

