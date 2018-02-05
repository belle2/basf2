//+
// File : DQMHistAnalysisPXDEff.h
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//
// Modified to efficiency by Uwe Gebauer
// Based on work from: Tomoyuki Konno, Tokyo Metroplitan Univerisity
// Date : 25  - Dec - 2015 ; first commit
//-


#pragma once

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>


namespace Belle2 {

  class DQMHistAnalysisPXDEffModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPXDEffModule();
  private:
    ~DQMHistAnalysisPXDEffModule() override final;

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    // Data members
    int m_u_bins;
    int m_v_bins;
    std::string m_histogramDirectoryName;
    bool m_singleHists;

    //IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //IDs of only the sensors of layer 1/2, to iterate the summary histograms
    std::vector<VxdID> m_PXDLayer1;
    std::vector<VxdID> m_PXDLayer2;

    //Individual efficiency for each module
    std::map<VxdID, TH2D*> m_hEffModules;
    std::map<VxdID, TCanvas*> m_cEffModules;

    //Make four summary plots for each module type
    std::map<std::string, TH2D*> m_hEffMerge;
    std::map<std::string, TCanvas*> m_cEffMerge;

    //One bin for each module in the geometry, one histogram for each layer
    TH1D* m_hEffAll1;
    TCanvas* m_cEffAll1;
    TH1D* m_hEffAll2;
    TCanvas* m_cEffAll2;

    TH1* findHistLocal(TString& a);

  };
} // end namespace Belle2
