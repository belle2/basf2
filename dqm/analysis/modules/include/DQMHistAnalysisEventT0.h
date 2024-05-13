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
#include <TEfficiency.h>

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

    /** Event */
    void event() override final;

    /** fit the histograms */
    void endRun() override final;

    /** delete pointers */
    void terminate() override final;

  private:

    /** prefix to be added to canvas name when saved as pdf */
    std::string m_prefixCanvas;
    /** if true print the pdf of the canvases */
    bool m_printCanvas;
    /** minimum number of entries to process the histogram */
    uint m_nEntriesMin = 100;
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

    TCanvas* m_cTOPTimeForECLTRG = nullptr; /**< TOP EventT0 for ECLTRG plots canvas */
    TCanvas* m_cTOPTimeForCDCTRG = nullptr; /**< TOP EventT0 for CDCTRG plots canvas */
    TCanvas* m_cSVDTimeForECLTRG = nullptr; /**< SVD EventT0 for ECLTRG plots canvas */
    TCanvas* m_cSVDTimeForCDCTRG = nullptr; /**< SVD EventT0 for CDCTRG plots canvas */

    TPad* m_topPad1ECLTRG = nullptr; /**< pad for TOP time ECLTRG hadrons */
    TPad* m_topPad2ECLTRG = nullptr; /**< pad for TOP time ECLTRG bhabhas */
    TPad* m_topPad3ECLTRG = nullptr; /**< pad for TOP time ECLTRG mumu */

    TPad* m_topPad1CDCTRG = nullptr; /**< pad for TOP time CDCTRG hadrons */
    TPad* m_topPad2CDCTRG = nullptr; /**< pad for TOP time CDCTRG bhabhas */
    TPad* m_topPad3CDCTRG = nullptr; /**< pad for TOP time CDCTRG mumu */

    TPad* m_svdPad1ECLTRG = nullptr; /**< pad for SVD time ECLTRG hadrons */
    TPad* m_svdPad2ECLTRG = nullptr; /**< pad for SVD time ECLTRG bhabhas */
    TPad* m_svdPad3ECLTRG = nullptr; /**< pad for SVD time ECLTRG mumu */

    TPad* m_svdPad1CDCTRG = nullptr; /**< pad for SVD time CDCTRG hadrons */
    TPad* m_svdPad2CDCTRG = nullptr; /**< pad for SVD time CDCTRG bhabhas */
    TPad* m_svdPad3CDCTRG = nullptr; /**< pad for SVD time CDCTRG mumu */

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/

    TCanvas* m_cT0FractionsForHadrons = nullptr; /**< EventT0 fractions plots canvas for hadron events */
    TCanvas* m_cT0FractionsForBhaBhas = nullptr; /**< EventT0 fractions plots canvas for BhaBha events */
    TCanvas* m_cT0FractionsForMuMus   = nullptr; /**< EventT0 fractions plots canvas for MuMu events */

    TCanvas* m_cHadronECLTRG = nullptr; /**< pad for time fractions for ECLTRG hadrons */
    TCanvas* m_cHadronCDCTRG = nullptr; /**< pad for time fractions for CDCTRG hadrons */
    TCanvas* m_cHadronTOPTRG = nullptr; /**< pad for time fractions for TOPTRG hadrons */

    TCanvas* m_cBhaBhaECLTRG = nullptr; /**< pad for time fractions for ECLTRG bhabhas */
    TCanvas* m_cBhaBhaCDCTRG = nullptr; /**< pad for time fractions for CDCTRG bhabhas */
    TCanvas* m_cBhaBhaTOPTRG = nullptr; /**< pad for time fractions for TOPTRG bhabhas */

    TCanvas* m_cMuMuECLTRG = nullptr; /**< pad for time fractions for ECLTRG mumu */
    TCanvas* m_cMuMuCDCTRG = nullptr; /**< pad for time fractions for CDCTRG mumu */
    TCanvas* m_cMuMuTOPTRG = nullptr; /**< pad for time fractions for TOPTRG mumu */

    /// EventT0 algorithms for which to calculate fractions of abundance
    const char* c_eventT0Algorithms[6] = {"ECL", "SVD", "CDC HitBased", "CDC FullGrid #chi^{2}", "TOP", "Any"};
    /// Fraction of events with EventT0 from a given algorithm, HLT hadronic events, L1 time by ECL trigger
    TEfficiency* m_eAlgorithmSourceFractionsHadronL1ECLTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT hadronic events, L1 time by CDC trigger
    TEfficiency* m_eAlgorithmSourceFractionsHadronL1CDCTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT hadronic events, L1 time by TOP trigger
    TEfficiency* m_eAlgorithmSourceFractionsHadronL1TOPTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT bhabha events, L1 time by ECL trigger
    TEfficiency* m_eAlgorithmSourceFractionsBhaBhaL1ECLTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT bhabha events, L1 time by CDC trigger
    TEfficiency* m_eAlgorithmSourceFractionsBhaBhaL1CDCTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT bhabha events, L1 time by TOP trigger
    TEfficiency* m_eAlgorithmSourceFractionsBhaBhaL1TOPTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT mumu events, L1 time by ECL trigger
    TEfficiency* m_eAlgorithmSourceFractionsMuMuL1ECLTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT mumu events, L1 time by CDC trigger
    TEfficiency* m_eAlgorithmSourceFractionsMuMuL1CDCTRG = nullptr;
    /// Fraction of events with EventT0 from a given algorithm, HLT mumu events, L1 time by TOP trigger
    TEfficiency* m_eAlgorithmSourceFractionsMuMuL1TOPTRG = nullptr;
    /// Fill the TEfficiency plots
    bool FillEfficiencyHistogram(const std::string& histname, TEfficiency* eff);
  };
} // end namespace Belle2

