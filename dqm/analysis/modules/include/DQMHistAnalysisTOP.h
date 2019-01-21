/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TString.h>
#include <TLine.h>
#include <TPaveText.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisTOPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisTOPModule();
    virtual ~DQMHistAnalysisTOPModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    TH1* find_histo_in_canvas(TString);
    TCanvas* find_canvas(TString);
    //! Data members
  private:
    TCanvas* m_c_goodHitsMean = nullptr;
    TCanvas* m_c_goodHitsRMS = nullptr;
    TCanvas* m_c_badHitsMean = nullptr;
    TCanvas* m_c_badHitsRMS = nullptr;

    TH1F* m_h_goodHitsMean = nullptr;
    TH1F* m_h_goodHitsRMS = nullptr;
    TH1F* m_h_badHitsMean = nullptr;
    TH1F* m_h_badHitsRMS = nullptr;

    TLine* m_line1 = nullptr;
    TLine* m_line2 = nullptr;
    TPaveText* m_text1 = nullptr;
  };
} // end namespace Belle2

