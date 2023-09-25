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
#include <TH2F.h>
#include <TH1F.h>
#include <TStyle.h>
#include <TBox.h>
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
    void getHistStyle(TH1F*& htemp, int color, int style) const
    {
      gStyle->SetOptStat("ne");
      htemp->SetMarkerColor(color);
      htemp->SetMarkerStyle(style);
      htemp->SetMarkerSize(0.70);
      htemp->Sumw2(0);
    };

    /**
     * Get median of given histogram
     */
    float getHistMedian(TH1D* h) const;


  protected:

    //Canvas for DQM analysis IR plots
    TCanvas* m_cBoards = nullptr; /**< main panel */
    TBox* m_boxadc  = nullptr; /**< box for normal ADC window */
    TBox* m_boxtdc  = nullptr; /**< box for normal ADC window */

    std::string m_histoDir = ""; /**< histogram dir of CDC DQMs */
    std::string m_histoADC = ""; /**< ADC histogram names of CDC DQMs */
    std::string m_histoTDC = ""; /**< TDC histogram names of CDC DQMs */
    std::string m_pvPrefix = ""; /**< Prefix of PVs */

    int m_minevt;/**< min events for single intra-run point */
    double m_minadc;/**< min adc median thershold accepted */
    double m_maxadc;/**< max adc median thershold accepted */
    double m_mintdc;/**< min tdc median thershold accepted */
    double m_maxtdc;/**< max tdc median thershold accepted */

    TH2F* delta_adc = nullptr; /**< 2D histograms (Card vs ADC) from track associated hits (Intra run)*/
    TH1F* hist_adc = nullptr; /**< same as above but for Integrated run*/

    TH2F* delta_tdc = nullptr; /**< 2D histograms (Card vs ADC) from track associated hits (Intra run)*/
    TH1F* hist_tdc = nullptr; /**< same as above but for Integrated run*/

    TH1D* m_hADCs[300]; /**< ADC histograms with track associated hits for each board (0-299) */
    TH1D* m_hTDCs[300]; /**< TDC histograms with track associated hits for each board (0-299) */
  };
} // Belle2 namespace
