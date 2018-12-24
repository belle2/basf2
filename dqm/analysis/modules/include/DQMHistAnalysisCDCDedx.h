/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jitendra Kumar                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TLine.h>
#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisCDCDedxModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisCDCDedxModule();
    virtual ~DQMHistAnalysisCDCDedxModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    void computedEdxMeanSigma();
    void computedEdxBandPlot();

    //! Data members
  private:

    TCanvas* c_CDCdedxMean = nullptr;
    TCanvas* c_CDCdedxSigma = nullptr;

    TH1F* h_CDCdedxMean = nullptr;
    TH1F* h_CDCdedxSigma = nullptr;

    TF1* f_fGaus = nullptr; // only one fit function

    TLine* tLine = nullptr;

    std::string runstatus;
    std::string runnumber;

    double dedxmean;
    double dedxsigma;


  };
} // end namespace Belle2

