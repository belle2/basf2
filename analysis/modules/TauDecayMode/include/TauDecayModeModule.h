/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
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
    /** Mapping for tau- decays */
    std::map<std::string, int> mode_decay_minus;
    /** Mapping for tau+ decays */
    std::map<std::string, int> mode_decay_plus;
    /** Variable name for the decay mode of the tau- */
    std::string m_tauminusdecaymode;
    /** Variable name for the decay mode of the tau+ */
    std::string m_tauplusdecaymode;

    /** ID of the decay channel of tau- */
    Int_t m_mmode;
    /** ID of the decay channel of tau+ */
    Int_t m_pmode;

    /** Prong of the decay channel of tau- */
    Int_t m_mprong;
    /** Prong of the decay channel of tau+ */
    Int_t m_pprong;

    /** Boolean variable used to identify tau event */
    bool tauPair;
    /** Number of tau- in the event */
    int numOfTauMinus;
    /** Number of tau+ in the event */
    int numOfTauPlus;
    /** Index of the generated tau- */
    int idOfTauMinus;
    /** Index of the generated tau+ */
    int idOfTauPlus;
    /** Alternative mapping for tau- */
    std::string m_file_minus;
    /** Alternative mapping for tau+ */
    std::string m_file_plus;

    /** PDG codes accepted as charged final state particles in generation: {e, mu, pi, K, p} */
    static constexpr int finalStatePDGs[5] = { 11, 13, 211, 321, 2212 };

    /** PDG codes of neutrinos in final state particles in generation: {nu_e, nu_mu, mu_tau} */
    static constexpr int Neutrinos[3] = { 12, 14, 16 };

    /** PDG codes of ORDERED particles */
    static constexpr int OrderedList[46] = {
      16, -16, 14, -14, 12, -12, // neutrinos
      11, -11, 13, -13, -211, 211, -321, 321, -2212, 2212, // charged final state particles
      111, 310, 130, 221, 223, 331, 333, //neutral mesons
      -213, 213, 113, //rho
      -323, 323, 313, -313, //Kstar
      -20213, 20213, //a1
      -9000211, 9000211, 9000111, -10211, 10211, 10111, //a0
      -10213, 10213, //b1
      20223, 9010221, //f1,f0
      3122, -3122, //lambda
      94144,//alpha
      22//gamma
    };

    /** Flag for eta->pi0pi0pi0 decays from tau- */
    bool m_isEtaPizPizPizFromTauMinus;
    /** Flag for eta->pi0pi0pi0 decays from tau+ */
    bool m_isEtaPizPizPizFromTauPlus;

    /** Flag for omega->pi-pi+ decays from tau- */
    bool m_isOmegaPimPipFromTauMinus;
    /** Flag for omega->pi-pi+ decays from tau+ */
    bool m_isOmegaPimPipFromTauPlus;

    //
    std::vector<int> vec_nut;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_anut;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_numu;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_anumu;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_nue; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_anue; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_em;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_ep;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_mum;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_mup;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pim;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pip;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_km;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kp;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_apro;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pro;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_pi0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_k0s;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_k0l;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_eta;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_omega;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_etapr;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_phi;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_rhom;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_rhop;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_rho0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstarm;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstarp;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstar0; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_kstar0_br; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a1m;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a1p;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a00_980;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0m_980;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0p_980;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a00_1450;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0m_1450;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_a0p_1450;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_b1m;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_b1p; /**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_f1;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_f0;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_lambda;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_lmb_br;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_alpha;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_gam;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_dau_tauminus;/**< Variable name of the vector where particles identified in the event are stored */
    std::vector<int> vec_dau_tauplus;/**< Variable name of the vector where particles identified in the event are stored */

    /** Analyze a generated tau pair event */
    void AnalyzeTauPairEvent();
    /** Classifies the decays of the event and assigns a decay mode */
    int TauolaBelle2DecayMode(const std::string& s, int chg);
    /** Identifies particles coming from tau decays */
    int getRecursiveMotherCharge(const MCParticle* mc);
    /** Identifies if the event is a generated tau pair */
    void IdentifyTauPair();
    /** Identifies the number of charged final state particles in the decay*/
    int getProngOfDecay(const MCParticle& mc);

  };

}
