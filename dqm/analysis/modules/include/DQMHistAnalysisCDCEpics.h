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
#include <TEllipse.h>
#include <numeric>
#include <iostream>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/CDCGeometryParConstants.h>

namespace Belle2 {


  /**
   * const CDC numbers for layers, boards and super layers
   */
  constexpr unsigned kNumLayers = c_maxNFieldLayers; /**< Total number of CDC layers. */
  constexpr unsigned kNumBoards = c_nBoards;/**< Total number of CDC Boards. */
  constexpr std::array<int, 9> slindex = { 8, 14, 20, 26, 32, 38, 44, 50, 56 }; /**< Index (layer number) of the 9 super-layers in the CDC. */

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

    /**
     * Populate the efficiency histograms
     */
    void fillEffiTH2(TH2F* hist, TH2F* attached, TH2F* expected, TH2F* efficiency) ;

  protected:

    //Canvas for DQM analysis IR plots
    TCanvas* c_histmd_ladc = nullptr; /**< canvas for adc layer median */
    TH1F* m_histmd_ladc = nullptr; /**< for above*/

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
    TH2F* m_hist_attach_eff[3] = {nullptr}; /**< for above*/
    TH2Poly* m_hist_attach_eff_Poly[3] = {nullptr}; /**< for above*/
    TH1F* m_hist_wire_attach_eff_1d = nullptr; /**< for above*/
    double lbinEdges[kNumLayers] = {0.0}; /**< vector for radius edge 56*/

    TLine* m_line_ladc  = nullptr; /**< line for lower ADC window */
    TLine* m_line_hadc  = nullptr; /**< line for higher ADC window */
    TLine* m_line_ltdc  = nullptr; /**< line for lower TDC window */
    TLine* m_line_htdc  = nullptr; /**< line for higher TDC window */

    std::string m_name_dir = ""; /**< histogram dir*/
    std::string m_name_refdir = ""; /**< reference histogram dir*/
    std::string m_name_pvpfx = ""; /**< Prefix of PVs */
    std::string m_fname_refphi = ""; /**< reference file of phi histogram */
    std::string m_hname_ladc = ""; /**< Layer ADC histogram names*/
    std::string m_hname_badc = ""; /**< Board ADC histogram names*/
    std::string m_hname_btdc = ""; /**< Board TDC histogram names*/
    std::string m_hname_idxphi = ""; /**< Phi Inedx histogram names*/
    std::string m_hname_effphi = ""; /**< Phi Eff histogram names*/
    std::string m_hname_hitsphi = ""; /**< Phi Hits histogram names*/
    std::string m_histoTrackingWireEff = ""; /**< Wire Eff histogram names*/
    bool  m_doTH2PolyTrackingWireEff = false; /**< If true, creates TH2Poly instead of TH2F for TrackingWireEff Histos */
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
    double m_phialarm;/**< alarm thershold for phi differences */
    double m_phiwarn;/**< warn thershold for phi differences */
    std::vector<TLine*> m_lines;/**< number of CDC layer lines */

    TH1D* m_hists_lADC[kNumLayers]; /**< ADC histograms with track associated hits for each board (0-299) */
    TH1D* m_hists_bADC[kNumBoards]; /**< ADC histograms with track associated hits for each board (0-299) */
    TH1D* m_hists_bTDC[kNumBoards]; /**< TDC histograms with track associated hits for each board (0-299) */
  };
} // Belle2 namespace