/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisEventT0TriggerJitter.h
// Description : module for DQM histogram analysis of trigger jitter
//-

#pragma once

#include <dqm/core/DQMHistAnalysis.h>
#include <TEfficiency.h>

namespace Belle2 {

  /** Class definition */
  class DQMHistAnalysisEventT0TriggerJitterModule final : public DQMHistAnalysisModule {

    // Public functions
  public:

    /** Constructor */
    DQMHistAnalysisEventT0TriggerJitterModule();
    /** Destructor */
    ~DQMHistAnalysisEventT0TriggerJitterModule();

    /** create TCanvas and MonitoringObject */
    void initialize() override final;

    /** clear TCanvas */
    void beginRun() override final;

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

    void analyseECLTRGEventT0Distributions(); /**< Analyse the ECLTRG EventT0 distributions*/
    void analyseCDCTRGEventT0Distributions(); /**< Analyse the CDCTRG EventT0 distributions*/
    void analyseTOPTRGEventT0Distributions(); /**< Analyse the TOPTRG EventT0 distributions*/
    void initializeCanvases(); /**< Initialise canvases */
    void clearCanvases(); /**< Initialise canvases */
    void printCanvases(); /**< Print canvases if required */
    void deleteCanvases(); /**< Delete canvases */

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/

    // ECLTRG jitter canvases
    TCanvas* m_cECLTimeHadronsECLTRG = nullptr; /**< Canvas for ECL time ECLTRG jitter hadrons */
    TCanvas* m_cECLTimeBhaBhaECLTRG  = nullptr; /**< Canvas for ECL time ECLTRG jitter bhabhas */
    TCanvas* m_cECLTimeMuMuECLTRG    = nullptr; /**< Canvas for ECL time ECLTRG jitter mumu */
    TCanvas* m_cCDCTimeHadronsECLTRG = nullptr; /**< Canvas for CDC time ECLTRG jitter hadrons */
    TCanvas* m_cCDCTimeBhaBhaECLTRG  = nullptr; /**< Canvas for CDC time ECLTRG jitter bhabhas */
    TCanvas* m_cCDCTimeMuMuECLTRG    = nullptr; /**< Canvas for CDC time ECLTRG jitter mumu */
    TCanvas* m_cTOPTimeHadronsECLTRG = nullptr; /**< Canvas for TOP time ECLTRG jitter hadrons */
    TCanvas* m_cTOPTimeBhaBhaECLTRG  = nullptr; /**< Canvas for TOP time ECLTRG jitter bhabhas */
    TCanvas* m_cTOPTimeMuMuECLTRG    = nullptr; /**< Canvas for TOP time ECLTRG jitter mumu */
    TCanvas* m_cSVDTimeHadronsECLTRG = nullptr; /**< Canvas for SVD time ECLTRG jitter hadrons */
    TCanvas* m_cSVDTimeBhaBhaECLTRG  = nullptr; /**< Canvas for SVD time ECLTRG jitter bhabhas */
    TCanvas* m_cSVDTimeMuMuECLTRG    = nullptr; /**< Canvas for SVD time ECLTRG jitter mumu */

    // CDCTRG jitter canvases
    TCanvas* m_cECLTimeHadronsCDCTRG = nullptr; /**< Canvas for ECL time CDCTRG jitter hadrons */
    TCanvas* m_cECLTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for ECL time CDCTRG jitter bhabhas */
    TCanvas* m_cECLTimeMuMuCDCTRG    = nullptr; /**< Canvas for ECL time CDCTRG jitter mumu */
    TCanvas* m_cCDCTimeHadronsCDCTRG = nullptr; /**< Canvas for CDC time CDCTRG jitter hadrons */
    TCanvas* m_cCDCTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for CDC time CDCTRG jitter bhabhas */
    TCanvas* m_cCDCTimeMuMuCDCTRG    = nullptr; /**< Canvas for CDC time CDCTRG jitter mumu */
    TCanvas* m_cTOPTimeHadronsCDCTRG = nullptr; /**< Canvas for TOP time CDCTRG jitter hadrons */
    TCanvas* m_cTOPTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for TOP time CDCTRG jitter bhabhas */
    TCanvas* m_cTOPTimeMuMuCDCTRG    = nullptr; /**< Canvas for TOP time CDCTRG jitter mumu */
    TCanvas* m_cSVDTimeHadronsCDCTRG = nullptr; /**< Canvas for SVD time CDCTRG jitter hadrons */
    TCanvas* m_cSVDTimeBhaBhaCDCTRG  = nullptr; /**< Canvas for SVD time CDCTRG jitter bhabhas */
    TCanvas* m_cSVDTimeMuMuCDCTRG    = nullptr; /**< Canvas for SVD time CDCTRG jitter mumu */

    // TOPTRG jitter canvases
    TCanvas* m_cECLTimeHadronsTOPTRG = nullptr; /**< Canvas for ECL time TOPTRG jitter hadrons */
    TCanvas* m_cECLTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for ECL time TOPTRG jitter bhabhas */
    TCanvas* m_cECLTimeMuMuTOPTRG    = nullptr; /**< Canvas for ECL time TOPTRG jitter mumu */
    TCanvas* m_cCDCTimeHadronsTOPTRG = nullptr; /**< Canvas for CDC time TOPTRG jitter hadrons */
    TCanvas* m_cCDCTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for CDC time TOPTRG jitter bhabhas */
    TCanvas* m_cCDCTimeMuMuTOPTRG    = nullptr; /**< Canvas for CDC time TOPTRG jitter mumu */
    TCanvas* m_cTOPTimeHadronsTOPTRG = nullptr; /**< Canvas for TOP time TOPTRG jitter hadrons */
    TCanvas* m_cTOPTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for TOP time TOPTRG jitter bhabhas */
    TCanvas* m_cTOPTimeMuMuTOPTRG    = nullptr; /**< Canvas for TOP time TOPTRG jitter mumu */
    TCanvas* m_cSVDTimeHadronsTOPTRG = nullptr; /**< Canvas for SVD time TOPTRG jitter hadrons */
    TCanvas* m_cSVDTimeBhaBhaTOPTRG  = nullptr; /**< Canvas for SVD time TOPTRG jitter bhabhas */
    TCanvas* m_cSVDTimeMuMuTOPTRG    = nullptr; /**< Canvas for SVD time TOPTRG jitter mumu */
  };
} // end namespace Belle2

