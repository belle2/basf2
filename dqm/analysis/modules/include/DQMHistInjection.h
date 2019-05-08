//+
// File : DQMHistInjection.h
// Description : DQM module, which gives occupancies in time after injection
//
// Author: Björn Spruck
// Date : Now
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TH1.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Efficiency */

  class DQMHistInjectionModule : public DQMHistAnalysisModule {

#ifdef _BELLE2_EPICS
    typedef struct {
      chid    mychid;
      TH1*  histo = {};
      std::vector <double> data;
    } MYNODE;

#endif

    // Public functions
  public:

    //! Constructor
    DQMHistInjectionModule();
    //! Destructor
    ~DQMHistInjectionModule();
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

    void cleanPVs(void);

    //! PXD Hits
    TH1F* m_hInjectionLERPXD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERPXD = nullptr;
    //! PXD Occ norm
    TH1F* m_hInjectionLERPXDOcc = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERPXDOcc = nullptr;
    //! ECL Hits
    TH1F* m_hInjectionLERECL = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERECL = nullptr;
    //! PXD Hits
    TH1F* m_hInjectionHERPXD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERPXD = nullptr;
    //! PXD Occ norm
    TH1F* m_hInjectionHERPXDOcc = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERPXDOcc = nullptr;
    //! ECL Hits
    TH1F* m_hInjectionHERECL = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERECL = nullptr;

#ifdef _BELLE2_EPICS
    //! EPICS PVs
    std::vector <MYNODE>  m_nodes;
#endif
  };
} // end namespace Belle2

