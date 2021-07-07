/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <TFile.h>
#include <string>
#include <TH1F.h>
#include <TGraph.h>

namespace Belle2 {

  /** Module used by the validation server to generate root files for the validation. Calculates fake rates and efficiencies.*/
  class KlongValidationModule : public Module {

  public:

    /** Constructor   */
    KlongValidationModule();

    /** Destructor   */
    virtual ~KlongValidationModule();

    /** initialize   */
    virtual void initialize() override;

    /**  beginn run   */
    virtual void beginRun() override;

    /** process event   */
    virtual void event() override;

    /** end run   */
    virtual void endRun() override;

    /** terminate   */
    virtual void terminate() override;

  protected:

  private:

    /** momentum   */
    double m_momentum{ -1.};
    /** angle in z-plane  */
    double m_theta{ -1.};
    /** angle in x-y   */
    double m_phi{ -1.};
    /** K0L truth   */
    double m_isKl{ -1.};
    /** did cluster pass selection of algorythm?  */
    bool   m_passed{false};
    /** cluster wrongly reconstructed as K0L?   */
    bool   m_faked{false};
    /** cluster reconstructed as K0L?   */
    bool   m_reconstructedAsKl{false};
    /** of > 0 use Klid else use trackflag as reconstruction criterion  */
    float m_KlIdCut{ -1};

    /** is beam bkg */
    bool m_isBeamBKG{false};
    // use TH1F histogramms to calculate efficiency
    /** efficiency in x-y plane   */
    TH1F* m_effPhi{nullptr};
    /** efficiency in angle to z   */
    TH1F* m_effTheta{nullptr};
    /** momentum efficiency   */
    TH1F* m_effMom{nullptr};
    /** fake phi, angle in x-y   */
    TH1F* m_fakePhi{nullptr};
    /**  fake theta, angle to z   */
    TH1F* m_fakeTheta{nullptr};
    /**  fake momentum plot   */
    TH1F* m_fakeMom{nullptr};
    /** efficiency in x-y plane   */
    TH1F* m_Phi_Pass{nullptr};
    /** efficiency in angle to z   */
    TH1F* m_Theta_Pass{nullptr};
    /** momentum efficiency   */
    TH1F* m_Mom_Pass{nullptr};

    /** efficiency in x-y plane   */
    TH1F* m_Phi_all{nullptr};
    /** efficiency in angle to z   */
    TH1F* m_Theta_all{nullptr};
    /** momentum efficiency   */
    TH1F* m_Mom_all{nullptr};
    /** momentum efficiency   */
    TH1F* m_Mom_all_plot{nullptr};
    /** fake phi, angle in x-y   */
    TH1F* m_fakePhi_Pass{nullptr};
    /**  fake theta, angle to z   */
    TH1F* m_fakeTheta_Pass{nullptr};
    /**  fake momentum plot   */
    TH1F* m_fakeMom_Pass{nullptr};
    /**  cluster timing plot   */
    TH1F* m_time{nullptr};
    /**  track separation distance plot   */
    TH1F* m_trackSep{nullptr};
    /**  energy plot   */
    TH1F* m_energy{nullptr};
    /**  layer count plot   */
    TH1F* m_nLayer{nullptr};
    /**  innermostlayer  */
    TH1F*  m_innermostLayer{nullptr};
    /**  track flag  */
    TH1F*  m_trackFlag{nullptr};
    /**  ECL flag*/
    TH1F*  m_ECLFlag{nullptr};
    /** beambkg  */
    TH1F*  m_bkgPhi{nullptr};
    /** beambkg */
    TH1F*  m_bkgTheta{nullptr};
    /** beambkg */
    TH1F*  m_bkgMom{nullptr};
    /** used for roc */
    TH1F*  m_klidFake{nullptr};
    /** used for roc */
    TH1F*  m_klidTrue{nullptr};
    /** used for roc */
    TH1F*  m_klidAll{nullptr};
    /** roc */
    TGraph*  m_ROC{nullptr};
    /** background rejection */
    TGraph*  m_backRej{nullptr};


    /** storearrays  */
    StoreArray<KLMCluster> m_klmClusters;
    /** storearrays  */
    StoreArray<MCParticle> m_mcParticles;

    /** bins used for the ROC plots */
    const std::vector<double> m_xbins =
    {0, 0.001, 0.01, 0.1, 0.15, 0.175, 0.2, 0.3, 0.4, 0.5, 1};

    /** output file name */
    std::string m_outputName = "K_long_full_validation_sample.root";

    /** root tree etc. */
    TFile* m_f = nullptr;

  }; // end class

} // end namespace Belle2

