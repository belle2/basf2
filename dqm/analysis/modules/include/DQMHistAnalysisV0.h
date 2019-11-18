//+
// File : DQMHistAnalysisV0.h
// Description : Overlay plotting for V0
//
// Author : Bryan Fulsom (PNNL), B Spruck
// Date : 2019-01-17
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TH1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisV0Module : public DQMHistAnalysisModule {

    typedef std::map<std::string, TH1*> HistList;


    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisV0Module();

  private:

    TCanvas* m_c_xvsy[32] = {nullptr};
    TCanvas* m_c_xvsz = nullptr;
    std::vector<TList*> contLevelXY;
    TList* contLevelXZ;

    std::string m_OverlayPath = "";

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void event() override;

  };
} // end namespace Belle2

