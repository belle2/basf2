//+
// File : DQMHistAnalysisEventT0.h
// Description : module for DQM histogram analysis of trigger jitter
//
// Author : Giulia Casarosa (PI),
// Date : 20181128
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TFile.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TF1.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisEventT0Module : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisEventT0Module();
    virtual ~DQMHistAnalysisEventT0Module();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void terminate() override;

    //parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */

  private:

    int m_nEntriesMin = 100; /**< minimum number of entries to process the histogram*/
    /** process the EventT0 distribution
     * fitting with two gaussians
     * filling the MonitoringObject
     * @param h EventT0 histogram
     * @param fitf fit function
     * @param tag to distinguish results
     * @return false if the histogram is not found or the fit is not converged
     **/
    bool processHistogram(TH1* h, TF1* fitf, TString tag);

    /** double gaussian fitting function for the jitter distribution*/
    static double fDoubleGaus(double* x, double* par);

    TCanvas* m_cHadronECLTRG = nullptr; /**< TOP EventT0 for Hadron ECLTRG canvas */
    TCanvas* m_cBhabhaECLTRG = nullptr; /**< TOP EventT0 for Bhabha ECLTRG canvas */
    TCanvas* m_cMumuECLTRG = nullptr; /**< TOP EventT0 for Mumu ECLTRG canvas */
    TCanvas* m_cHadronCDCTRG = nullptr; /**< TOP EventT0 for Hadron CDCTRG canvas */
    TCanvas* m_cBhabhaCDCTRG = nullptr; /**< TOP EventT0 for Bhabha CDCTRG canvas */
    TCanvas* m_cMumuCDCTRG = nullptr; /**< TOP EventT0 for Mumu CDCTRG canvas */

    MonitoringObject* m_monObj = NULL; /**< MonitoringObject to be produced by this module*/
  };
} // end namespace Belle2

