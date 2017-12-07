

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *
 **************************************************************************/
#ifndef KlongValidationModule_H
#define KlongValidationModule_H

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
    virtual void initialize();

    /**  beginn run   */
    virtual void beginRun();

    /** process event   */
    virtual void event();

    /** end run   */
    virtual void endRun();

    /** terminate   */
    virtual void terminate();

  protected:

  private:

    /** momentum   */
    double m_momentum;
    /** angle in z-plane  */
    double m_theta;
    /** angle in x-y   */
    double m_phi;
    /** K0L truth   */
    double m_isKl;
    /** did cluster pass selection of algorythm?  */
    bool   m_passed;
    /** cluster wrongly reconstructed as K0L?   */
    bool   m_faked;
    /** cluster reconstructed as K0L?   */
    bool   m_reconstructedAsKl;
    /** of > 0 use Klid else use trackflag as reconstruction criterion  */
    float m_KlIDCut;

    /** is beam bkg */
    bool m_isBeamBKG;
    // use TH1F histogramms to calculate efficiency
    /** efficiency in x-y plane   */
    TH1F* m_effPhi;
    /** efficiency in angle to z   */
    TH1F* m_effTheta;
    /** momentum efficiency   */
    TH1F* m_effMom;
    /** fake phi, angle in x-y   */
    TH1F* m_fakePhi;
    /**  fake theta, angle to z   */
    TH1F* m_fakeTheta;
    /**  fake momentum plot   */
    TH1F* m_fakeMom;
    /** efficiency in x-y plane   */
    TH1F* m_effPhi_Pass;
    /** efficiency in angle to z   */
    TH1F* m_effTheta_Pass;
    /** momentum efficiency   */
    TH1F* m_effMom_Pass;

    /** efficiency in x-y plane   */
    TH1F* m_Phi_all;
    /** efficiency in angle to z   */
    TH1F* m_Theta_all;
    /** momentum efficiency   */
    TH1F* m_Mom_all;
    /** momentum efficiency   */
    TH1F* m_Mom_all_plot;
    /** fake phi, angle in x-y   */
    TH1F* m_fakePhi_Pass;
    /**  fake theta, angle to z   */
    TH1F* m_fakeTheta_Pass;
    /**  fake momentum plot   */
    TH1F* m_fakeMom_Pass;
    /**  fake momentum plot   */
    TH1F* m_time;
    /**  fake momentum plot   */
    TH1F* m_trackSep;
    /**  fake momentum plot   */
    TH1F* m_energy;
    /**  fake momentum plot   */
    TH1F* m_nLayer;
    /**  innermostlayer  */
    TH1F*  m_innermostLayer;
    /**  track flag  */
    TH1F*  m_trackFlag     ;
    /**  ECL flag*/
    TH1F*  m_ECLFlag       ;    /** beambkg */
    /**mbkg  */
    TH1F*  m_bkgPhi  ;
    /** beambkg */
    TH1F*  m_bkgTheta;
    /** beambkg */
    TH1F*  m_bkgMom  ;
    /** used for roc */
    TH1F*  m_klidFake  ;
    /** used for roc */
    TH1F*  m_klidTrue  ;
    /** used for roc */
    TH1F*  m_klidAll  ;
    /** roc */
    TGraph*  m_ROC;
    /** roc */
    TGraph*  m_backRej;


    /** storearrays  */
    StoreArray<KLMCluster> m_klmClusters;
    /** storearrays  */
    StoreArray<MCParticle> m_mcParticles;

    /** bins used for the ROC plots */
    const std::vector<double> m_xbins =
    {0, 0.001, 0.01, 0.1, 0.15, 0.175, 0.2, 0.3, 0.4, 0.5, 1};

    /** output path   */
    std::string m_outPath = "nightlyKlongValidation.root";

    /** root tree etc. */
    TFile* m_f = nullptr;  //

  }; // end class
} // end namespace Belle2

#endif
