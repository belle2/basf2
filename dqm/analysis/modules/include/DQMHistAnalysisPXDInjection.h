//+
// File : DQMHistAnalysisPXDInjection.h
// Description : DQM module, which gives occupancies in time after injection
// PXD specific per module plots without EPICS export
//
// Author: Björn Spruck
// Date : Now
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/geometry/GeoCache.h>

#include <TH1.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD occupancy after Injection */

  class DQMHistAnalysisPXDInjectionModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDInjectionModule();

  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;

    //! PXD Hits
    TH1F* m_hInjectionLERPXD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionLERPXD = nullptr;
    //! PXD Hits
    TH1F* m_hInjectionHERPXD = nullptr;
    //! Final Canvas
    TCanvas* m_cInjectionHERPXD = nullptr;

    std::vector<VxdID> m_sensors;
    std::map <VxdID, TCanvas*>    m_cInjectionLERPXDMod;
    std::map <VxdID, TCanvas*> m_cInjectionHERPXDMod;
    std::map <VxdID, TH1F*> m_hInjectionLERPXDMod;
    std::map <VxdID, TH1F*> m_hInjectionHERPXDMod;
    std::map <VxdID, TCanvas*> m_cInjectionLERPXDModNorm;
    std::map <VxdID, TCanvas*> m_cInjectionHERPXDModNorm;
    std::map <VxdID, TH1F*> m_hInjectionLERPXDModNorm;
    std::map <VxdID, TH1F*>     m_hInjectionHERPXDModNorm;

  };
} // end namespace Belle2

