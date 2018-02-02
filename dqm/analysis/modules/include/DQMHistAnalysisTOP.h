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

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisTOPModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisTOPModule();
    virtual ~DQMHistAnalysisTOPModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Data members
  private:
    TCanvas* m_c_particleHitsMean = nullptr;
    TCanvas* m_c_particleHitsRMS = nullptr;
    TCanvas* m_c_otherHitsMean = nullptr;
    TCanvas* m_c_otherHitsRMS = nullptr;

    TH1F* m_h_particleHitsMean = nullptr;
    TH1F* m_h_particleHitsRMS = nullptr;
    TH1F* m_h_otherHitsMean = nullptr;
    TH1F* m_h_otherHitsRMS = nullptr;

  };
} // end namespace Belle2

