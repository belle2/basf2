//+
// File : DQMHistAnalysisV0.h
// Description : An example module for DQM histogram analysis
//
// Author : Bryan Fulsom (PNNL)
// Date : 2019-01-17
//-

#pragma once

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TFile.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TH1.h>
#include <TImage.h>
#include <TPad.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisV0Module : public DQMHistAnalysisModule {

    typedef std::map<std::string, TH1*> HistList;


    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisV0Module();
    virtual ~DQMHistAnalysisV0Module();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    //parameters

    //! Parameters accesible from basf2 scripts
    //  protected:

    //! Data members
  private:

    TCanvas* m_c_xvsy[32] = {nullptr};
    TCanvas* m_c_xvsz = nullptr;
    TImage* m_img[32] = {nullptr};
    TImage* m_img_xz = nullptr;
    TPad* p, *pxz;
    std::string m_OverlayPath = "";

  };
} // end namespace Belle2

