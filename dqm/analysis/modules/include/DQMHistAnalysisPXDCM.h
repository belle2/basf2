//+
// File : DQMHistAnalysisPXDCM.h
// Description : DAQM Analysis for PXD Common Modes
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <vector>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Common Modes */

  class DQMHistAnalysisPXDCMModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDCMModule();
    //! Destructor
    ~DQMHistAnalysisPXDCMModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void terminate(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! histogram covering all modules
    TH2F* m_hCommonMode = nullptr;
    //! Final Canvas
    TCanvas* m_cCommonMode = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_line1 = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_line2 = nullptr;
    //! Line in the Canvas to guide the eye
    TLine* m_line3 = nullptr;

#ifdef _BELLE2_EPICS
    //! epics PVs
    std::vector <chid> mychid;
#endif
  };
} // end namespace Belle2

