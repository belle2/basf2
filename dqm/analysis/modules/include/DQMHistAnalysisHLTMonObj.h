/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/core/DQMHistAnalysis.h>

// RooFit forward declarations
class RooAddPdf;
class RooChebychev;
class RooGaussian;
class RooRealVar;

namespace Belle2 {

  /**
   * Creates monitoring object for HLT
   */
  class DQMHistAnalysisHLTMonObjModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisHLTMonObjModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisHLTMonObjModule();

    /**
     * Initialize the Module.
     */
    void initialize() override final;


    /**
     * End-of-run action.
     */
    void endRun() override final;

    /**
     * Termination action.
     */
    void terminate() override final;

  protected:

    TCanvas* m_c_filter = nullptr; /**<Canvas with histograms related to HLT filter*/
    TCanvas* m_c_skim = nullptr; /**<Canvas with histograms related to HLT skims*/
    TCanvas* m_c_hardware = nullptr; /**<Canvas with histograms related to HLT hardware*/
    TCanvas* m_c_l1 = nullptr; /**<Canvas with histograms related to L1*/
    TCanvas* m_c_ana_eff_shifter = nullptr; /**<Canvas with histogram related to ana_eff_shifter*/

    MonitoringObject* m_monObj = nullptr; /**< MonitoringObject to be produced by this module*/

    RooRealVar* m_KsInvMass = nullptr; /**<Invariant mass of KS for HLTPrefilter monitoring*/
    RooRealVar* m_mean1 = nullptr; /**<*Mean of first gaussian*/
    RooRealVar* m_sigma1 = nullptr; /**<*Sigma of second gaussian*/
    RooGaussian* m_gauss1 = nullptr; /**<First gaussian*/
    RooRealVar* m_mean2 = nullptr; /**<Mean of first gaussian*/
    RooRealVar* m_sigma2 = nullptr; /**<*Sigma of second gaussian*/
    RooGaussian* m_gauss2 = nullptr; /**<Second gaussian*/
    RooRealVar* m_frac = nullptr; /**<*Fraction of first gaussian in double gaussian*/
    RooAddPdf* m_double_gauss = nullptr; /**<Sum of two gaussian*/
    RooRealVar* m_slope = nullptr; /**<Slope for first order polynomial*/
    RooChebychev* m_chebpol = nullptr; /**<First order polynomial*/
    RooRealVar* m_sig = nullptr; /**<Number of Ks events from fit*/
    RooRealVar* m_bkg = nullptr; /**<Number of background from fit*/
    RooAddPdf*  m_KsPdf = nullptr; /**<Fit PDF for Ks invariant mass*/
  };

} // Belle2 namespace
