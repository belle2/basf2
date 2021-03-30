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


  protected:

    std::string m_printmode; /**< Parameter passed by the user to indicated the informationt to be printed */

  private:

    /** pointer to tau pair decay objects */
    StoreObjPtr<TauPairDecay> m_tauDecay;

    StoreObjPtr<EventMetaData> m_event_metadata; /**< event number */
    int m_taum_no;     /**< number of tau- unclassified events */
    int m_taup_no;     /**< number of tau+ unclassified events */

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
    int m_pdg_extra;
    /** Variable name for an extra particle in the classification */
    std::string m_name;

    /** PDG codes accepted as charged final state particles in generation: {e, mu, pi, K, p} */
    const int finalStatePDGs[5] = { 11, 13, 211, 321, 2212 };

    std::vector<int> vec_em;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_ep;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_nue; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int>vec_anue; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_mum;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_mup;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_numu;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_anumu;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_nut;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_anut;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pim;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pip;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_km;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kp;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_apro;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pro;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pi0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_k0s;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_k0l;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_gam;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_eta;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_omega;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstarp;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstarm;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_lambda;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_lmb_br;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstar; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int>vec_kstar_br; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int>vec_etapr;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0p;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0m;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_b1m;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_b1p; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int>vec_phi;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_f1;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a1m;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a1p;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_rhom;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_rhop;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_K0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_K0_br;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int>  vec_rho0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_f0;/**< Variable name of the vector where particles identified in the event are stored */

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



  };

}
