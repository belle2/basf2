//+
// File : DQMHistAnalysisPXDFits.h
// Description : An example module for DQM histogram analysis
//
// Author : Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-


#pragma once

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisPXDEffModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPXDEffModule();
    virtual ~DQMHistAnalysisPXDEffModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    int m_u_bins;
    int m_v_bins;
    std::string m_histogramDirectoryName;

    VXD::GeoCache& m_vxdGeometry;

    //IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //todo canvas for these? only when option selected?
    //should these be printed out?
    //Individual efficiency for each module
    std::map<VxdID, TH2D*> m_hEffModules;
    std::map<VxdID, TCanvas*> m_cEffModules;

    //Make four summary plots for each module type
    TH2D* m_hEffIF;
    TH2D* m_hEffIB;
    TH2D* m_hEffOF;
    TH2D* m_hEffOB;
    TCanvas* m_cEffIF;
    TCanvas* m_cEffIB;
    TCanvas* m_cEffOF;
    TCanvas* m_cEffOB;

    //One bin for each module in the geometry
    TH1D* m_hEffAll;
    TCanvas* m_cEffAll;

  };
} // end namespace Belle2
