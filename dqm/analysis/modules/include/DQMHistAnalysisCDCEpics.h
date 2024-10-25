/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <dqm/core/DQMHistAnalysis.h>

#include <TROOT.h>
#include <TLine.h>
#include <TH2Poly.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TLine.h>
#include <numeric>
#include <iostream>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisCDCEpicsModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisCDCEpicsModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisCDCEpicsModule();

    /**
     * Initialize the Module.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * intra-run actions (EPICC PVs).
     */
    void event() override final;

    /**
     * End-of-run action.
     */
    void endRun() override final;

    /**
     * Termination action.
     */
    void terminate() override final;

    /**
     * get histogram styles
     */
    void getHistStyle(TH1F*& htemp, std::string label, double max) const
    {
      gStyle->SetOptStat("ne");
      if (strcmp(label.data(), "adc") == 0)htemp->GetYaxis()->SetRangeUser(max * 0.25, max * 2.25);
      else if (strcmp(label.data(), "tdc") == 0)htemp->GetYaxis()->SetRangeUser(max * 0.90, max * 1.10);
      htemp->Sumw2(0);
    };

    /**
     * Get median of given histogram
     */
    float getHistMedian(TH1D* h) const;

    /**
     * Convenient function to create a TH2Poly based on CDC geometry
     */
    TH2Poly* createEffiTH2Poly(const TString& name, const TString& title) ;

    /**
     * Populate the efficiency histograms
     */
    void fillEffiTH2Poly(TH2F* hist, TH2Poly* attached, TH2Poly* expected, TH2Poly* efficiency) ;


  protected:

    //Canvas for DQM analysis IR plots
    TCanvas* c_hist_adc = nullptr; /**< canvas for adc board median */
    TH1F* m_hist_adc = nullptr; /**< for above*/

    TCanvas* c_hist_tdc = nullptr; /**< canvas for tdc board median */
    TH1F* m_hist_tdc = nullptr; /**< for above */

    TCanvas* c_hist_crphi = nullptr; /**< canvas for control shifter phi */
    TH1D* m_hist_crphi = nullptr; /**< for above*/

    TCanvas* c_hist_hitsphi = nullptr; /**< expert canvas for hits vs phi */

    TCanvas* c_hist_effphi = nullptr; /**< canvas for tracking efficiency */
    TH1D* m_hist_effphi = nullptr; /**< for above*/

    TCanvas* c_hist_skimphi[8] = {nullptr}; /**< canvas for various phi distribution */
    TH1D* m_hist_skimphi[8] = {nullptr}; /**< for above*/

    TCanvas* c_hist_attach_eff[4] = {nullptr}; /**< canvas for layer efficiency */
    TH2Poly* m_hist_attach_eff[3] = {nullptr}; /**< for above*/
    TH1F* m_hist_wire_attach_eff_1d = nullptr; /**< for above*/

    TLine* m_line_ladc  = nullptr; /**< line for lower ADC window */
    TLine* m_line_hadc  = nullptr; /**< line for higher ADC window */
    TLine* m_line_ltdc  = nullptr; /**< line for lower TDC window */
    TLine* m_line_htdc  = nullptr; /**< line for higher TDC window */

    std::string m_histoDir = ""; /**< histogram dir of CDC DQMs */
    std::string m_histoADC = ""; /**< ADC histogram names of CDC DQMs */
    std::string m_histoTDC = ""; /**< TDC histogram names of CDC DQMs */
    std::string m_histoPhiIndex = ""; /**< Phi Inedx histogram names of CDC DQMs */
    std::string m_histoPhiEff = ""; /**< Phi Eff histogram names of CDC DQMs */
    std::string m_histoHitsPhi = ""; /**< Phi Hits histogram names of CDC DQMs */
    std::string m_histoTrackingWireEff = ""; /**< Wire Eff histogram names of CDC DQMs */
    std::string m_pvPrefix = ""; /**< Prefix of PVs */
    std::string m_refDir = ""; /**< reference histogram dir of CDC DQMs */
    std::string m_refNamePhi = ""; /**< reference histogram of phi */
    double m_firstEffBoundary = 0.08; /**< The first boundary of the efficiency range */
    double m_secondEffBoundary = 0.72; /**< The second boundary of the efficiency range */

    MonitoringObject* m_monObj = nullptr; /**< monitoring object */

    TFile* m_fileRefPhi = nullptr; /**< reference histogram file point */
    TH2F* m_histref_phiindex = nullptr; /**< for above*/
    TH1D* m_hist_refphi = nullptr; /**< for above*/

    int m_minevt;/**< min events for single intra-run point */
    double m_minadc;/**< min adc median thershold accepted */
    double m_maxadc;/**< max adc median thershold accepted */
    double m_mintdc;/**< min tdc median thershold accepted */
    double m_maxtdc;/**< max tdc median thershold accepted */
    double m_phistop;/**< stop thershold for phi differences */
    double m_phialarm;/**< alram thershold for phi differences */
    double m_phiwarn;/**< warn thershold for phi differences */

    TH1D* m_hADCs[300]; /**< ADC histograms with track associated hits for each board (0-299) */
    TH1D* m_hTDCs[300]; /**< TDC histograms with track associated hits for each board (0-299) */
  };
} // Belle2 namespace
