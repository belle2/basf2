/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEventT0.h
// Description : module for DQM histogram analysis of trigger jitter
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>

namespace Belle2 {

  /** Class definition */
  class DQMHistAnalysisEventT0Module final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /** Constructor */
    DQMHistAnalysisEventT0Module();
    /** Destructor */
    ~DQMHistAnalysisEventT0Module();

    /** create TCanvas and MonitoringObject */
    void initialize() override final;

    /** clear TCanvas */
    void beginRun() override final;

    /** fit the histograms */
    void endRun() override final;

    /** delete pointers */
    void terminate() override final;

    std::string m_prefixCanvas; /**< prefix to be added to canvas name when saved as pdf*/
    bool m_printCanvas; /**< if true print the pdf of the canvases */

  private:

    int m_nEntriesMin = 100; /**< minimum number of entries to process the histogram*/
    /** process the EventT0 distribution
     * fitting with two gaussians
     * filling the MonitoringObject
     * @param h EventT0 histogram
     * @param tag to distinguish results
     * @return false if the histogram is not found or the fit is not converged
     **/
    bool processHistogram(TH1* h, TString tag);

    /** double gaussian fitting function for the jitter distribution
     * @param x Data used to fit double gaussians
     * @param par Normalization + fraction + double gaussian parameters (mu, sigma)
    */
    static double fDoubleGaus(double* x, double* par);

    TCanvas* m_cECLTRG = nullptr; /**< TOP EventT0 for ECLTRG plots canvas */
    TCanvas* m_cCDCTRG = nullptr; /**< CDC EventT0 for Hadron CDCTRG plots canvas */
    TCanvas* m_cSVDECLTRG = nullptr; /**< SVD EventT0 for Hadron ECLTRG plots canvas */
    TCanvas* m_cSVDCDCTRG = nullptr; /**< SVD EventT0 for Hadron CDCTRG plots canvas */

    TPad* m_pad1ECLTRG = nullptr; /**< pad for ECLTRG hadrons */
    TPad* m_pad2ECLTRG = nullptr; /**< pad for ECLTRG bhabhas */
    TPad* m_pad3ECLTRG = nullptr; /**< pad for ECLTRG mumuss */

    TPad* m_pad1CDCTRG = nullptr; /**< pad for CDCTRG hadrons */
    TPad* m_pad2CDCTRG = nullptr; /**< pad for CDCTRG bhabhas */
    TPad* m_pad3CDCTRG = nullptr; /**< pad for CDCTRG mumus */

    TPad* m_svdPad1ECLTRG = nullptr; /**< SVD pad for ECLTRG hadrons */
    TPad* m_svdPad2ECLTRG = nullptr; /**< SVD pad for ECLTRG bhabhas */
    TPad* m_svdPad3ECLTRG = nullptr; /**< SVD pad for ECLTRG mumus */

    TPad* m_svdPad1CDCTRG = nullptr; /**< SVD pad for CDCTRG hadrons */
    TPad* m_svdPad2CDCTRG = nullptr; /**< SVD pad for CDCTRG bhabhas */
    TPad* m_svdPad3CDCTRG = nullptr; /**< SVD pad for CDCTRG mumus */

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/
  };
} // end namespace Belle2

