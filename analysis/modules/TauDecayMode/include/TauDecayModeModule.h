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
    //! Terminate at the end of job
    virtual void terminate();

  private:

    /** pointer to tau pair decay objects */
    StoreObjPtr<TauPairDecay> m_tauDecay;

    int EventNumber;
    int nop;
    int taum_no;
    int taup_no;

    /** StoreArray of MCParticles */
    StoreArray<MCParticle> MCParticles;

    std::map<std::string, int> mode_decay;

    std::string m_tauminusdecaymode;
    std::string m_tauplusdecaymode;

    Int_t m_pmode;
    /** ID of the decay channel of negative tau*/
    Int_t m_mmode;
    /** Prong of the decay channel of positive tau */
    Int_t m_pprong;
    /** Prong of the decay channel of negative tau*/
    Int_t m_mprong;

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

    int pdg_extra;

    std::string name;

    /** PDG codes accepted as charged final state particles in generation: {e, mu, pi, K, p} */
    const int finalStatePDGs[5] = { 11, 13, 211, 321, 2212 };

    std::vector<int> vec_em, vec_ep, vec_nue, vec_anue;
    std::vector<int> vec_mum, vec_mup, vec_numu, vec_anumu;
    std::vector<int> vec_nut, vec_anut;
    std::vector<int> vec_pim, vec_pip, vec_km, vec_kp, vec_apro, vec_pro;
    std::vector<int> vec_pi0, vec_k0s, vec_k0l, vec_gam;
    std::vector<int> vec_eta, vec_omega, vec_kstarp, vec_kstarm;
    std::vector<int> vec_lambda, vec_lmb_br, vec_kstar, vec_kstar_br, vec_etapr;//new particle vector
    std::vector<int> vec_a0, vec_a0p, vec_a0m, vec_b1m, vec_b1p, vec_phi, vec_f1, vec_a1m, vec_a1p, vec_rhom, vec_rhop ;
    std::vector<int> vec_K0, vec_K0_br,  vec_rho0, vec_f0;
    std::vector<int> vec_dau_tauminus;
    std::vector<int> vec_dau_tauplus;
    std::vector<int> vec_extra;

    /** Identifies if the event is a generated tau pair */
    void IdentifyTauPair();
    double getEnergyTauRestFrame(const MCParticle* mc, const int ichg);
    int getRecursiveMotherCharge(const MCParticle* mc);
    int TauBBBmode(std::string s, std::map<std::string, int> tau_map);
    int getProngOfDecay(const MCParticle& mc);

  protected:

    int m_printmode;

  };

}
