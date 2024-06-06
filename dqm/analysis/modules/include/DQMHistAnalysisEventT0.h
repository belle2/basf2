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

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/

    TCanvas* m_cECLTimeHadronsECLTRG = nullptr; /**< Canvas for SVD time ECLTRG time jitter hadrons */
    TCanvas* m_cECLTimeBhaBhaECLTRG  = nullptr; /**< Canvas for SVD time ECLTRG time jitter bhabhas */
    TCanvas* m_cECLTimeMuMuECLTRG    = nullptr; /**< Canvas for SVD time ECLTRG time jitter mumu */
    TCanvas* m_cCDCTimeHadronsECLTRG = nullptr; /**< Canvas for SVD time ECLTRG time jitter hadrons */
    TCanvas* m_cCDCTimeBhaBhaECLTRG  = nullptr; /**< Canvas for SVD time ECLTRG time jitter bhabhas */
    TCanvas* m_cCDCTimeMuMuECLTRG    = nullptr; /**< Canvas for SVD time ECLTRG time jitter mumu */
    TCanvas* m_cTOPTimeHadronsECLTRG = nullptr; /**< Canvas for TOP time ECLTRG time jitter hadrons */
    TCanvas* m_cTOPTimeBhaBhaECLTRG  = nullptr; /**< Canvas for TOP time ECLTRG time jitter bhabhas */
    TCanvas* m_cTOPTimeMuMuECLTRG    = nullptr; /**< Canvas for TOP time ECLTRG time jitter mumu */
    TCanvas* m_cSVDTimeHadronsECLTRG = nullptr; /**< Canvas for SVD time ECLTRG time jitter hadrons */
    TCanvas* m_cSVDTimeBhaBhaECLTRG  = nullptr; /**< Canvas for SVD time ECLTRG time jitter bhabhas */
    TCanvas* m_cSVDTimeMuMuECLTRG    = nullptr; /**< Canvas for SVD time ECLTRG time jitter mumu */


    TCanvas* m_cECLTimeHadronsCDCTRG = nullptr; /**< Canvas for SVD time CDCTRG time jitter hadrons */
    TCanvas* m_cECLTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for SVD time CDCTRG time jitter bhabhas */
    TCanvas* m_cECLTimeMuMuCDCTRG    = nullptr; /**< Canvas for SVD time CDCTRG time jitter mumu */
    TCanvas* m_cCDCTimeHadronsCDCTRG = nullptr; /**< Canvas for SVD time CDCTRG time jitter hadrons */
    TCanvas* m_cCDCTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for SVD time CDCTRG time jitter bhabhas */
    TCanvas* m_cCDCTimeMuMuCDCTRG    = nullptr; /**< Canvas for SVD time CDCTRG time jitter mumu */
    TCanvas* m_cTOPTimeHadronsCDCTRG = nullptr; /**< Canvas for TOP time CDCTRG time jitter hadrons */
    TCanvas* m_cTOPTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for TOP time CDCTRG time jitter bhabhas */
    TCanvas* m_cTOPTimeMuMuCDCTRG    = nullptr; /**< Canvas for TOP time CDCTRG time jitter mumu */
    TCanvas* m_cSVDTimeHadronsCDCTRG = nullptr; /**< Canvas for SVD time CDCTRG time jitter hadrons */
    TCanvas* m_cSVDTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for SVD time CDCTRG time jitter bhabhas */
    TCanvas* m_cSVDTimeMuMuCDCTRG    = nullptr; /**< Canvas for SVD time CDCTRG time jitter mumu */

    TCanvas* m_cTOPTimeHadronsTOPTRG = nullptr; /**< Canvas for TOP time TOPTRG time jitter hadrons */
    TCanvas* m_cTOPTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for TOP time TOPTRG time jitter bhabhas */
    TCanvas* m_cTOPTimeMuMuTOPTRG    = nullptr; /**< Canvas for TOP time TOPTRG time jitter mumu */
    TCanvas* m_cECLTimeHadronsTOPTRG = nullptr; /**< Canvas for SVD time TOPTRG time jitter hadrons */
    TCanvas* m_cECLTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for SVD time TOPTRG time jitter bhabhas */
    TCanvas* m_cECLTimeMuMuTOPTRG    = nullptr; /**< Canvas for SVD time TOPTRG time jitter mumu */
    TCanvas* m_cSVDTimeHadronsTOPTRG = nullptr; /**< Canvas for SVD time TOPTRG time jitter hadrons */
    TCanvas* m_cSVDTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for SVD time TOPTRG time jitter bhabhas */
    TCanvas* m_cSVDTimeMuMuTOPTRG    = nullptr; /**< Canvas for SVD time TOPTRG time jitter mumu */
    TCanvas* m_cCDCTimeHadronsTOPTRG = nullptr; /**< Canvas for SVD time TOPTRG time jitter hadrons */
    TCanvas* m_cCDCTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for SVD time TOPTRG time jitter bhabhas */
    TCanvas* m_cCDCTimeMuMuTOPTRG    = nullptr; /**< Canvas for SVD time TOPTRG time jitter mumu */


    TCanvas* m_cT0FractionsHadronECLTRG = nullptr; /**< Canvas for time fractions for ECLTRG hadrons */
    TCanvas* m_cT0FractionsHadronCDCTRG = nullptr; /**< Canvas for time fractions for CDCTRG hadrons */
    TCanvas* m_cT0FractionsHadronTOPTRG = nullptr; /**< Canvas for time fractions for TOPTRG hadrons */

    TCanvas* m_cT0FractionsBhaBhaECLTRG = nullptr; /**< Canvas for time fractions for ECLTRG bhabhas */
    TCanvas* m_cT0FractionsBhaBhaCDCTRG = nullptr; /**< Canvas for time fractions for CDCTRG bhabhas */
    TCanvas* m_cT0FractionsBhaBhaTOPTRG = nullptr; /**< Canvas for time fractions for TOPTRG bhabhas */

    TCanvas* m_cT0FractionsMuMuECLTRG = nullptr; /**< Canvas for time fractions for ECLTRG mumu */
    TCanvas* m_cT0FractionsMuMuCDCTRG = nullptr; /**< Canvas for time fractions for CDCTRG mumu */
    TCanvas* m_cT0FractionsMuMuTOPTRG = nullptr; /**< Canvas for time fractions for TOPTRG mumu */

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

