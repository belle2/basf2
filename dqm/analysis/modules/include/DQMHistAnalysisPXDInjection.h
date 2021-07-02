/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDInjection.h
// Description : DQM module, which gives occupancies in time after injection
// PXD specific per module plots without EPICS export
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

    //! List of PXD sensors
    std::vector<VxdID> m_sensors;
    //! Canvases per sensor for LER
    std::map <VxdID, TCanvas*>    m_cInjectionLERPXDMod;
    //! Canvases per sensor for HER
    std::map <VxdID, TCanvas*> m_cInjectionHERPXDMod;
    //! Histogram per sensor for LER
    std::map <VxdID, TH1F*> m_hInjectionLERPXDMod;
    //! Histogram per sensor for HER
    std::map <VxdID, TH1F*> m_hInjectionHERPXDMod;
    //! Canvases per sensor for LER normalized
    std::map <VxdID, TCanvas*> m_cInjectionLERPXDModNorm;
    //! Canvases per sensor for HER normalized
    std::map <VxdID, TCanvas*> m_cInjectionHERPXDModNorm;
    //! Histogram per sensor for LER normalized
    std::map <VxdID, TH1F*> m_hInjectionLERPXDModNorm;
    //! Histogram per sensor for HER normalized
    std::map <VxdID, TH1F*>     m_hInjectionHERPXDModNorm;

  };
} // end namespace Belle2

