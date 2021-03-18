/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Leonardo Salinas, Swagato Banerjee                       *
*               Michel Hernandez, Eduard De la Cruz.                     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/VariableManager/Utility.h>
#include <TVector3.h>
#include "TFile.h"
#include "TBox.h"
#include "TDirectory.h"
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <string>
#include <analysis/dataobjects/TauPairDecay.h>
namespace Belle2 {

  /** Module to classify tau decay events  according to a mapping given by the user or
  *with a default mapping based on
  *the TauolaBelle2 decay list
  */
  class TauDecayModeModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TauDecayModeModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;


  private:

    /** pointer to tau pair decay objects */
    StoreObjPtr<TauPairDecay> m_tauDecay;

    int EventNumber; /**< event number */
    int taum_no;     /**< number of tau- unclassified events */
    int taup_no;     /**< number of tau+ unclassified events */

    /** StoreArray of MCParticles */
    StoreArray<MCParticle> MCParticles;
    /** Mapping for the decays */
    std::map<std::string, int> mode_decay;
    /** Variable name for the decay mode of the negative tau */
    std::string m_tauminusdecaymode;
    /** Variable name for the decay mode of the positive tau */
    std::string m_tauplusdecaymode;

    /** ID of the decay channel of positive tau*/
    Int_t m_pmode;
    /** ID of the decay channel of negative tau*/
    Int_t m_mmode;
    /** Prong of the decay channel of positive tau */
    Int_t m_pprong;
    /** Prong of the decay channel of negative tau*/
    Int_t m_mprong;

    /** Boolean variable used to identify tau event */
    bool tauPair;
    /** Number of positive tau leptons in the event */
    int numOfTauPlus;
    /** Number of negative tau leptons in the event */
    int numOfTauMinus;
    /** Index of the generated positive tau */
    int idOfTauPlus;
    /** Index of the generated negative tau */
    int idOfTauMinus;
    /** Alternative mapping */
    std::string m_file;
    /** Extra particle  */
    std::string m_particle;
    /** Variable name for an extra pdg number in the classification */
    int pdg_extra;
    /** Variable name for an extra particle in the classification */
    std::string name;

    /** PDG codes accepted as charged final state particles in generation: {e, mu, pi, K, p} */
    const int finalStatePDGs[5] = { 11, 13, 211, 321, 2212 };

    std::vector<int> vec_em, vec_ep, vec_nue,
        vec_anue; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_mum, vec_mup, vec_numu,
        vec_anumu;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_nut, vec_anut;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pim, vec_pip, vec_km, vec_kp, vec_apro,
        vec_pro;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pi0, vec_k0s, vec_k0l,
        vec_gam;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_eta, vec_omega, vec_kstarp,
        vec_kstarm;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_lambda, vec_lmb_br, vec_kstar, vec_kstar_br,
        vec_etapr;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0, vec_a0p, vec_a0m, vec_b1m, vec_b1p, vec_phi, vec_f1, vec_a1m, vec_a1p, vec_rhom,
        vec_rhop ;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_K0, vec_K0_br,  vec_rho0,
        vec_f0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_dau_tauminus;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_dau_tauplus;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_extra;/**< Variable name of the vector where particles identified in the event are stored */

    /** Identifies if the event is a generated tau pair */
    void IdentifyTauPair();
    /** Get the energy of the tau in the rest frame */
    double getEnergyTauRestFrame(const MCParticle* mc, const int ichg);
    /** Identifies particles coming from tau decays */
    int getRecursiveMotherCharge(const MCParticle* mc);
    /** Classifies the decays of the event and assigns a decay mode */
    int TauBBBmode(std::string s);
    /** Identifies the number of charged final state particles in the decay*/
    int getProngOfDecay(const MCParticle& mc);

  protected:
    //* Parameter passed by the user to indicated the informationt to be printed */
    std::string m_printmode;

  };

}
