/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Leonardo Salinas, Swagato Banerjee,                      *
*               Michel Hernandez, Eduard De la Cruz.                     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/TauDecayMode/TauDecayModeModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <TParticlePDG.h>
#include <map>
#include <fstream>
#include <set>
using namespace std;
using namespace Belle2;

std::map<std::string, int> make_map(const std::string& file, int chg)
{
  std::string fileName;
  if (file == "") {
    if (chg < 0) {
      B2INFO("Missing input mapping file: use mp_file_minus=basf2.find_file('data/analysis/modules/TauDecayMode/map_tauminus.txt') TauDecayMode.param('file_minus', mp_file_minus)  to classify with default mapping.");
    } else {
      B2INFO("Missing input mapping file: use mp_file_plus=basf2.find_file('data/analysis/modules/TauDecayMode/map_tauplus.txt') TauDecayMode.param('file_plus', mp_file_plus)  to classify with default mapping.");
    }
  } else {
    fileName = file;
  }

  ifstream f;
  f.open(fileName);
  std::string line;
  std::string key;
  int value;
  std::map <std::string, int> map_tau;
  while (f.good()) {
    getline(f, line);
    istringstream ss(line);
    ss >> key >> value;
    map_tau[key] = value;
  }
  f.close();
  return map_tau;
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TauDecayMode)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TauDecayModeModule::TauDecayModeModule() : Module() , m_taum_no(0), m_taup_no(0), m_mmode(-2), m_pmode(-2),
  m_mprong(0), m_pprong(0), tauPair(false), numOfTauMinus(0), numOfTauPlus(0), idOfTauMinus(-1), idOfTauPlus(-1)
{
  // Set module properties
  setDescription("Module to identify generated tau pair decays, using MCParticle information."
                 "By default, each tau decay is numbered as TauolaBelle2DecayMode [Ref: BELLE2-NOTE-PH-2020-055]");
  //Parameter definition
  addParam("printmode",  m_printmode, "Printout more information from each event", std::string("default"));
  addParam("file_minus", m_file_minus, "Path for an alternative mapping for tau- decays", std::string(""));
  addParam("file_plus",  m_file_plus, "Path for an alternative mapping for tau+ decays", std::string(""));
}

//
void TauDecayModeModule::initialize()
{
  mode_decay_minus = make_map(m_file_minus, -1);
  mode_decay_plus  = make_map(m_file_plus, 1);
  m_tauDecay.registerInDataStore();
  m_event_metadata.isRequired();
}

//
void TauDecayModeModule::event()
{

  IdentifyTauPair();
  if (tauPair) {

    AnalyzeTauPairEvent();

    if (m_printmode == "missing") {
      if (m_mmode == -1) {
        B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent()
               << " Decay: tau- -> " << m_tauminusdecaymode << " Mode = " << m_mmode);
      }
      if (m_pmode == -1) {
        B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent()
               << " Decay: tau+ -> " << m_tauplusdecaymode  << " Mode = " << m_pmode);
      }
    }

    if (m_printmode == "all") {
      B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent()
             << " Decay: tau- -> " << m_tauminusdecaymode << " Mode = " << m_mmode);
      B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent()
             << " Decay: tau+ -> " << m_tauplusdecaymode  << " Mode = " << m_pmode);
    }

    //
    if (m_mmode == -1) {
      m_taum_no = m_taum_no - 1;
      m_mmode = m_taum_no;
    }
    if (m_pmode == -1) {
      m_taup_no = m_taup_no - 1;
      m_pmode = m_taup_no;
    }

    m_pprong = getProngOfDecay(*MCParticles[idOfTauPlus - 1]);
    m_mprong = getProngOfDecay(*MCParticles[idOfTauMinus - 1]);

  } else {
    m_pmode = -1;
    m_mmode = -1;
    m_pprong = -1;
    m_mprong = -1;
  }

  if (!m_tauDecay) m_tauDecay.create();
  m_tauDecay->addTauMinusIdMode(m_mmode);
  m_tauDecay->addTauPlusIdMode(m_pmode);

  m_tauDecay->addTauPlusMcProng(m_pprong);
  m_tauDecay->addTauMinusMcProng(m_mprong);

}

void TauDecayModeModule::AnalyzeTauPairEvent()
{
  // Clear local vectors
  vec_nut.clear();
  vec_anut.clear();
  vec_numu.clear();
  vec_anumu.clear();
  vec_nue.clear();
  vec_anue.clear();
  vec_em.clear();
  vec_ep.clear();
  vec_mum.clear();
  vec_mup.clear();
  vec_pim.clear();
  vec_pip.clear();
  vec_km.clear();
  vec_kp.clear();
  vec_apro.clear();
  vec_pro.clear();
  vec_pi0.clear();
  vec_k0s.clear();
  vec_k0l.clear();
  vec_eta.clear();
  vec_omega.clear();
  vec_etapr.clear();
  vec_phi.clear();
  vec_rhom.clear();
  vec_rhop.clear();
  vec_rho0.clear();
  vec_kstarm.clear();
  vec_kstarp.clear();
  vec_kstar0.clear();
  vec_kstar0_br.clear();
  vec_a1m.clear();
  vec_a1p.clear();
  vec_a00_980.clear();
  vec_a0m_980.clear();
  vec_a0p_980.clear();
  vec_a00_1450.clear();
  vec_a0m_1450.clear();
  vec_a0p_1450.clear();
  vec_b1m.clear();
  vec_b1p.clear();
  vec_f1.clear();
  vec_f0.clear();
  vec_lambda.clear();
  vec_lmb_br.clear();
  vec_alpha.clear();
  vec_gam.clear();
  //
  map<int, std::vector<int>> map_vec;
  //
  map_vec[11] = vec_em;
  map_vec[-11] = vec_ep;
  map_vec[12] = vec_nue;
  map_vec[-12] = vec_anue;
  map_vec[13] = vec_mum;
  map_vec[-13] = vec_mup;
  map_vec[14] = vec_numu;
  map_vec[-14] = vec_anumu;
  map_vec[16] = vec_nut;
  map_vec[-16] = vec_anut;
  map_vec[-211] = vec_pim;
  map_vec[211] = vec_pip;
  map_vec[-321] = vec_km;
  map_vec[321] = vec_kp;
  map_vec[-2212] = vec_apro;
  map_vec[2212] = vec_pro;
  map_vec[111] = vec_pi0;
  map_vec[310] = vec_k0s;
  map_vec[130] = vec_k0l;
  map_vec[221] = vec_eta;
  map_vec[223] = vec_omega;
  map_vec[331] = vec_etapr;
  map_vec[333] = vec_phi;
  map_vec[-213] = vec_rhom;
  map_vec[213] = vec_rhop;
  map_vec[113] = vec_rho0;
  map_vec[-323] = vec_kstarm;
  map_vec[323] = vec_kstarp;
  map_vec[313] = vec_kstar0;
  map_vec[-313] = vec_kstar0_br;
  map_vec[-20213] = vec_a1m;
  map_vec[20213] = vec_a1p;
  map_vec[-9000211] = vec_a0m_980;
  map_vec[9000211]  = vec_a0p_980;
  map_vec[9000111]  = vec_a00_980;
  map_vec[-10211] = vec_a0m_1450;
  map_vec[10211]  = vec_a0p_1450;
  map_vec[10111]  = vec_a00_1450;
  map_vec[-10213] = vec_b1m;
  map_vec[10213] = vec_b1p;
  map_vec[20223] = vec_f1;
  map_vec[9010221] = vec_f0;
  map_vec[3122] = vec_lambda;
  map_vec[-3122] = vec_lmb_br;
  map_vec[94144] = vec_alpha;
  map_vec[22] = vec_gam;

  bool elecFirst = true;
  bool muonFirst = true;

  bool isPiPizGamTauMinusFirst = true;
  bool isPiPizGamTauPlusFirst = true;

  bool isLFVTauMinus2BodyDecayFirst = true;
  bool isLFVTauPlus2BodyDecayFirst = true;

  bool isChargedRhoFromTauMinusFirst = true;
  bool isChargedRhoFromTauPlusFirst = true;

  bool isChargedA1FromTauMinusFirst = true;
  bool isChargedA1FromTauPlusFirst = true;

  bool isEtaPPGFromTauMinusFirst = true;
  bool isEtaPPGFromTauPlusFirst = true;

  bool isOmegaPizGamFromTauMinusFirst = true;
  bool isOmegaPizGamFromTauPlusFirst = true;

  bool isEtaPizPizPizFromTauMinusFirst = true;
  bool isEtaPizPizPizFromTauPlusFirst = true;

  m_isEtaPizPizPizFromTauMinus = false;
  m_isEtaPizPizPizFromTauPlus = false;

  bool isOmegaPimPipFromTauMinusFirst = true;
  bool isOmegaPimPipFromTauPlusFirst = true;

  m_isOmegaPimPipFromTauMinus = false;
  m_isOmegaPimPipFromTauPlus = false;

  // Loop of MCParticles
  for (int i = 0; i < MCParticles.getEntries(); i++) {

    MCParticle& p = *MCParticles[i];

    int pdgid = p.getPDG();

    if (pdgid == -130) pdgid = 130; // Strange feature in TauolaBelle2

    if (!p.hasStatus(MCParticle::c_PrimaryParticle))
      continue; // only consider particles coming from generator, e.g. discard particles added by Geant4
    if (p.isInitial()) continue; // pick e-  e+, but not from the incoming beams

    if (pdgid == 11 && elecFirst)  {
      elecFirst = false;
      const MCParticle* mother = p.getMother();
      const vector<MCParticle*> daughters = mother->getDaughters();
      int nElMinus = 0;
      int nElPlus = 0;
      stringstream elec_ss;
      for (MCParticle* d : daughters) {
        if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
        elec_ss <<  d->getPDG() << " ";
        if (d->getPDG() == 11) nElMinus++;
        if (d->getPDG() == -11) nElPlus++;
      }
      if (nElMinus == 1 && nElPlus == 1) { // use this information in getRecursiveMotherCharge
        B2DEBUG(1, "Mother of elec pair is = " << mother->getPDG() << " which has daughters : " << elec_ss.str());
      }
    }

    if (pdgid == 13 && muonFirst)  {
      muonFirst = false;
      const MCParticle* mother = p.getMother();
      const vector<MCParticle*> daughters = mother->getDaughters();
      int nMuMinus = 0;
      int nMuPlus = 0;
      stringstream muon_ss;
      for (MCParticle* d : daughters) {
        if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
        muon_ss <<  d->getPDG() << " ";
        if (d->getPDG() == 13) nMuMinus++;
        if (d->getPDG() == -13) nMuPlus++;
      }
      if (nMuMinus == 1 && nMuPlus == 1) { // use this information in getRecursiveMotherCharge
        B2DEBUG(1, "Mother of muon pair is = " << mother->getPDG() << " which has daughters : " << muon_ss.str());
      }
    }

    // Special treatment for photons
    bool accept_photon = false;
    if (pdgid == 22)  {
      const MCParticle* mother = p.getMother();
      int mothid = abs(mother->getPDG());

      // check if the gamma comes from final state charged particles {e, mu, pi, K, p, b_1}
      bool isRadiationfromFinalStateChargedParticle = false;
      if (mothid == 11 ||
          mothid == 13 ||
          mothid == 211 ||
          mothid == 321 ||
          mothid == 2212 ||
          mothid == 10213) {
        isRadiationfromFinalStateChargedParticle = true;
      }

      // check if the gamma from ChargedRho
      bool isRadiationFromChargedRho = false;
      if (mothid == 213) {
        int chg = getRecursiveMotherCharge(mother);
        if (chg < 0 && isChargedRhoFromTauMinusFirst) {
          isChargedRhoFromTauMinusFirst = false;
          isRadiationFromChargedRho = true;
        }
        if (chg > 0 && isChargedRhoFromTauPlusFirst) {
          isChargedRhoFromTauPlusFirst = false;
          isRadiationFromChargedRho = true;
        }
      }

      // check if the gamma from ChargedA1
      bool isRadiationFromChargedA1 = false;
      if (mothid == 20213) {
        int chg = getRecursiveMotherCharge(mother);
        if (chg < 0 && isChargedA1FromTauMinusFirst) {
          isChargedA1FromTauMinusFirst = false;
          isRadiationFromChargedA1 = true;
        }
        if (chg > 0 && isChargedA1FromTauPlusFirst) {
          isChargedA1FromTauPlusFirst = false;
          isRadiationFromChargedA1 = true;
        }
      }

      // check if the gamma comes from intermediate W+- boson
      bool isRadiationfromIntermediateWBoson = false;
      if (mothid == 24) isRadiationfromIntermediateWBoson = true;

      // check if it is a tau- -> pi- omega nu, omega -> pi0 gamma decay
      // Note: TauolaBelle2 generator treats this a coherant production
      // e.g. includes omega in the form factor but not as an explicit final state particle
      bool isPiPizGam = false;
      if (isRadiationfromIntermediateWBoson) {
        if (p.get4Vector().E() > 0.050) { // 50 MeV threshold
          const vector<MCParticle*> daughters = mother->getDaughters();
          int nPiSisters = 0;
          int nPizSisters = 0;
          int nTotSisters = 0;
          int nOtherSisters = 0;
          for (MCParticle* d : daughters) {
            if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
            nTotSisters++;
            if (abs(d->getPDG()) == 211) {
              nPiSisters++;
            } else if (abs(d->getPDG()) == 111) {
              nPizSisters++;
            } else if (abs(d->getPDG()) != 22) {
              nOtherSisters++;
            }
          }
          // allow final state radiation from the tau lepton, but ignore information on number of photons for decay mode classifiction
          if (nTotSisters >= 3 && nPiSisters == 1 && nPizSisters == 1 && nOtherSisters == 0) {
            int chg = getRecursiveMotherCharge(mother);
            if (chg < 0 && isPiPizGamTauMinusFirst) {
              isPiPizGamTauMinusFirst = false;
              isPiPizGam = true;
            }
            if (chg > 0 && isPiPizGamTauPlusFirst) {
              isPiPizGamTauPlusFirst = false;
              isPiPizGam = true;
            }
          }
        }
      }

      // check if the gamma comes from tau
      bool isRadiationfromTau = false;
      if (mothid == 15) isRadiationfromTau = true;

      // check if the gamma comes from 2 body LFV decays of tau, e.g. tau- -> e-/mu- gamma with arbitary number of extra photon radiations from PHOTOS/FSR
      bool isLFVTau2BodyDecay = false;
      if (isRadiationfromTau) {
        bool hasNeutrinoAsSister = false;
        int numChargedSister = 0;
        int numNeutralNonNeutrinoNonPhotonSister = 0;
        const vector<MCParticle*> daughters = mother->getDaughters();
        for (MCParticle* d : daughters) {
          if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
          hasNeutrinoAsSister = find(begin(Neutrinos), end(Neutrinos), abs(d->getPDG())) != end(Neutrinos);
          if (hasNeutrinoAsSister) break;
        }
        if (!hasNeutrinoAsSister) {
          for (MCParticle* d : daughters) {
            if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
            bool isChargedFinalState = find(begin(finalStatePDGs), end(finalStatePDGs), abs(d->getPDG())) != end(finalStatePDGs);
            if (isChargedFinalState) {
              numChargedSister++;
            } else if (d->getPDG() != 22) {
              numNeutralNonNeutrinoNonPhotonSister++;
            }
          }
          if (numChargedSister == 1 && numNeutralNonNeutrinoNonPhotonSister == 0) {
            if (mother->getPDG() == 15 && isLFVTauMinus2BodyDecayFirst) {
              isLFVTauMinus2BodyDecayFirst = false;
              isLFVTau2BodyDecay = true;
            }
            if (mother->getPDG() == -15 && isLFVTauPlus2BodyDecayFirst) {
              isLFVTauPlus2BodyDecayFirst = false;
              isLFVTau2BodyDecay = true;
            }
          }
        }
        B2DEBUG(1, "hasNeutrinoAsSister = " << hasNeutrinoAsSister
                << " numChargedSister = " << numChargedSister
                << " numNeutralNonNeutrinoNonPhotonSister = " << numNeutralNonNeutrinoNonPhotonSister
                << " isLFVTau2BodyDecay = " << isLFVTau2BodyDecay);
      }

      bool isPi0GG = false;
      bool isEtaGG = false;
      bool isEtpGG = false;
      bool isPi0GEE = false;
      bool isEtaPPG = false;
      bool isOmPizG = false;
      if (mothid == 111 || mothid == 221 || mothid == 331 || mothid == 223) {
        const vector<MCParticle*> daughters = mother->getDaughters();
        int numberofTotalDaughters = 0;
        int numberOfPhotonDaughters = 0;
        int numberOfElectronDaughters = 0;
        int numberOfPionDaughters = 0;
        int numberOfPizDaughters = 0;
        for (MCParticle* d : daughters) {
          if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
          numberofTotalDaughters ++;
          if (abs(d->getPDG()) == 22) numberOfPhotonDaughters++;
          if (abs(d->getPDG()) == 11) numberOfElectronDaughters++;
          if (abs(d->getPDG()) == 211) numberOfPionDaughters++;
          if (abs(d->getPDG()) == 111) numberOfPizDaughters++;
        }
        if (numberofTotalDaughters == 2 && numberOfPhotonDaughters == 2) {
          if (mothid == 111) isPi0GG = true;
          if (mothid == 221) isEtaGG = true;
          if (mothid == 331) isEtpGG = true;
        }
        if (numberofTotalDaughters >= 3 && numberOfPhotonDaughters >= 1 && numberOfElectronDaughters == 2 && numberOfPionDaughters == 0
            && numberOfPizDaughters == 0) {
          if (mothid == 111) isPi0GEE = true;
        }
        if (numberofTotalDaughters >= 3 && numberOfPhotonDaughters >= 1 && numberOfPizDaughters == 0 && numberOfPionDaughters == 2) {
          if (mothid == 221) {
            int chg = getRecursiveMotherCharge(mother);
            if (chg < 0 && isEtaPPGFromTauMinusFirst)  {
              isEtaPPGFromTauMinusFirst = false;
              isEtaPPG = true;
            }
            if (chg > 0 && isEtaPPGFromTauPlusFirst)  {
              isEtaPPGFromTauPlusFirst = false;
              isEtaPPG = true;
            }
          }
        }
        if (numberofTotalDaughters >= 2 && numberOfPhotonDaughters >= 1 && numberOfPizDaughters == 1 && numberOfPionDaughters == 0) {
          if (mothid == 223) {
            int chg = getRecursiveMotherCharge(mother);
            if (chg < 0 && isOmegaPizGamFromTauMinusFirst) {
              isOmegaPizGamFromTauMinusFirst = false;
              isOmPizG = true;
            }
            if (chg > 0 && isOmegaPizGamFromTauPlusFirst) {
              isOmegaPizGamFromTauPlusFirst = false;
              isOmPizG = true;
            }
          }
        }
      }

      B2DEBUG(1, "isRadiationfromFinalStateChargedParticle = " << isRadiationfromFinalStateChargedParticle);
      B2DEBUG(1, "isRadiationFromChargedRho = " << isRadiationFromChargedRho);
      B2DEBUG(1, "isRadiationFromChargedA1 = " << isRadiationFromChargedA1);
      B2DEBUG(1, "isRadiationfromIntermediateWBoson = " << isRadiationfromIntermediateWBoson << " isPiPizGam = " << isPiPizGam);
      B2DEBUG(1, "isRadiationfromTau = " << isRadiationfromTau << " isLFVTau2BodyDecay = " << isLFVTau2BodyDecay);
      B2DEBUG(1, "isPi0GG = " << isPi0GG << " isEtaGG = " << isEtaGG << " isEtpGG = " << isEtpGG << " isPi0GEE = " << isPi0GEE);
      B2DEBUG(1, "isEtaPPG = " << isEtaPPG << " isOmPizG = " << isOmPizG);

      // accept photons if (isRadiationfromFinalStateChargedParticle is false) or
      // if (isRadiationfromIntermediateWBoson is false) or
      // if (isRadiationfromTau is true) {gamma is from 2 body LFV decays of tau} or
      // if the radiation is coming from any other decay [except pi0 -> gamma gamma, eta-> gamma gamma, eta' -> gamma gamma]
      if (isRadiationfromFinalStateChargedParticle) {
      } else if (isRadiationFromChargedRho) {
        accept_photon = true;
      } else if (isRadiationFromChargedA1) {
        accept_photon = true;
      } else if (isRadiationfromIntermediateWBoson) {
        if (isPiPizGam) {
          accept_photon = true;
        }
      } else if (isRadiationfromTau) { // accept one photon from tau -> (charged particle) + gamma [no neutrinos]
        if (isLFVTau2BodyDecay) {
          accept_photon = true;
        }
      } else if (isPi0GG) {
      } else if (isEtaGG) {
      } else if (isEtpGG) {
      } else if (isPi0GEE) {
      } else if (isEtaPPG) {
        accept_photon = true;
      } else if (isOmPizG) {
        accept_photon = true;
      }
    }

    // Without further analysis, it is NOT possible to separate
    // tau- -> pi- 2pi0 eta (-> pi- pi+ pi0) nu decays [Mode 39] from
    // tau- -> 2pi- pi+ eta (-> pi0 pi0 pi0) nu decays [Mode 42]

    if (pdgid == 111 && (isEtaPizPizPizFromTauMinusFirst || isEtaPizPizPizFromTauPlusFirst)) {
      const MCParticle* mother = p.getMother();
      if (mother->getPDG() == 221) { // eta -> pi0 pi0 pi0
        const vector<MCParticle*> daughters = mother->getDaughters();
        int nPizSisters = 0;
        for (MCParticle* d : daughters) {
          if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
          if (d->getPDG() == 111) nPizSisters++;
        }
        B2DEBUG(1, "nPizSisters = " << nPizSisters);
        if (nPizSisters == 3) {
          int chg = getRecursiveMotherCharge(mother);
          if (chg < 0 && isEtaPizPizPizFromTauMinusFirst) {
            isEtaPizPizPizFromTauMinusFirst = false;
            m_isEtaPizPizPizFromTauMinus = true;
          }
          if (chg > 0 && isEtaPizPizPizFromTauPlusFirst) {
            isEtaPizPizPizFromTauPlusFirst = false;
            m_isEtaPizPizPizFromTauPlus = true;
          }
        }
      }
    }
    B2DEBUG(1,
            "isEtaPizPizPizFromTauMinusFirst = " << isEtaPizPizPizFromTauMinusFirst << " "
            "m_isEtaPizPizPizFromTauMinus = "    << m_isEtaPizPizPizFromTauMinus    << " "
            "isEtaPizPizPizFromTauPlusFirst = "  << isEtaPizPizPizFromTauPlusFirst  << " "
            "m_isEtaPizPizPizFromTauPlus = "     << m_isEtaPizPizPizFromTauPlus
           );

    // Without further analysis, it is NOT possible to separate
    // tau- ->  pi-     2pi0 omega (-> pi- pi+)     nu decays [Mode 49] from
    // tau- -> 2pi- pi+ 2pi0                        nu decays [Mode 66]
    // Note: TauolaBelle2 treats Mode 66 is coherant production,
    // e.g. includes omega in the form factor but not as an explicit final state particle.
    // Note2: omega is explicitly included in following mode:
    // tau- ->  pi-      pi0 omega (-> pi- pi+ pi0) nu decays [Mode 130]
    // which is where Mode 49 is mapped to if there is no omega->pi-pi+ decay

    // Same confusion can also happen for
    // tau- -> pi- pi0 omega (-> pi- pi+)     nu decays [Mode 131]
    // tau- -> pi-     omega (-> pi- pi+ pi0) nu decays [Mode 236]

    if (pdgid == -211 && (isOmegaPimPipFromTauMinusFirst || isOmegaPimPipFromTauPlusFirst)) {
      const MCParticle* mother = p.getMother();
      if (mother->getPDG() == 223) { // omega -> pi- pi+
        const vector<MCParticle*> daughters = mother->getDaughters();
        int nOmegaDaughters = 0;
        int nPimSisters = 0;
        int nPipSisters = 0;
        int nPizSisters = 0;
        for (MCParticle* d : daughters) {
          if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
          nOmegaDaughters++;
          if (d->getPDG() == -211) nPimSisters++;
          if (d->getPDG() ==  211) nPipSisters++;
          if (d->getPDG() ==  111) nPizSisters++;
        }
        B2DEBUG(1,
                "nOmegaDaughters = " << nOmegaDaughters << " "
                "nPimSisters = " << nPimSisters << " "
                "nPipSisters = " << nPipSisters << " "
                "nPizSisters = " << nPizSisters);
        if (nOmegaDaughters >= 2 && nPimSisters == 1 && nPipSisters == 1 && nPizSisters == 0) { // allow omega->pi-pi+gama
          int chg = getRecursiveMotherCharge(mother);
          if (chg < 0 && isOmegaPimPipFromTauMinusFirst) {
            isOmegaPimPipFromTauMinusFirst = false;
            m_isOmegaPimPipFromTauMinus = true;
          }
          if (chg > 0 && isOmegaPimPipFromTauPlusFirst) {
            isOmegaPimPipFromTauPlusFirst = false;
            m_isOmegaPimPipFromTauPlus = true;
          }
        }
      }
    }
    B2DEBUG(1,
            "isOmegaPimPipFromTauMinusFirst = " << isOmegaPimPipFromTauMinusFirst << " "
            "m_isOmegaPimPipFromTauMinus = "    << m_isOmegaPimPipFromTauMinus    << " "
            "isOmegaPimPipFromTauPlusFirst = "  << isOmegaPimPipFromTauPlusFirst  << " "
            "m_isOmegaPimPipFromTauPlus = "     << m_isEtaPizPizPizFromTauPlus
           );

    // Fill up all the vector of particles
    map<int, std::vector<int>>::iterator ite ;
    for (ite =  map_vec.begin(); ite !=  map_vec.end(); ++ite) {
      if (pdgid == ite->first) {
        if (pdgid == 22) {
          if (accept_photon) {
            B2DEBUG(1, "Photon accepted");
            ite-> second.push_back(i);
          }
        } else {
          ite-> second.push_back(i);
        }
        break;
      }
    }

  } // End of loop over MCParticles

  vec_dau_tauminus.clear();
  vec_dau_tauplus.clear();

  map<int, std::vector<int>>::iterator itr ;
  for (itr =  map_vec.begin(); itr !=  map_vec.end(); ++itr) {
    for (unsigned int i = 0; i < itr-> second.size(); i++) {
      int ii = itr-> second[i];
      int chg = getRecursiveMotherCharge(MCParticles[ii]);
      if (chg < 0) vec_dau_tauminus.push_back(ii);
      if (chg > 0) vec_dau_tauplus.push_back(ii);
    }
  }

  EvtGenDatabasePDG* databasePDG = EvtGenDatabasePDG::Instance();
  std::string pdgname;

  //make decay string for tau-
  m_tauminusdecaymode = "";
  for (unsigned iorder = 0; iorder < 46; ++iorder) {
    int ii = OrderedList[iorder];
    //
    for (unsigned int i = 0; i < vec_dau_tauminus.size(); i++) {
      MCParticle* p = MCParticles[vec_dau_tauminus[i]];
      int pdg = p->getPDG();
      if (pdg == -130) pdg = 130; // Strange Feature in TauolaBelle2
      if (pdg != ii) continue;
      m_tauminusdecaymode.append(".");
      if (pdg == 94144) {
        pdgname = "alpha";
      } else {
        pdgname = databasePDG->GetParticle(pdg)->GetName();
      }
      m_tauminusdecaymode.append(pdgname);
    }
  }
  //
  m_mmode = TauolaBelle2DecayMode(m_tauminusdecaymode, -1);

  //make decay string for tau+
  m_tauplusdecaymode = "";
  for (unsigned iorder = 0; iorder < 46; ++iorder) {
    int ii = OrderedList[iorder];
    //
    for (unsigned int i = 0; i < vec_dau_tauplus.size(); i++) {
      MCParticle* p = MCParticles[vec_dau_tauplus[i]];
      int pdg = p->getPDG();
      if (pdg == -130) pdg = 130; // Strange Feature in TauolaBelle2
      if (pdg != ii) continue;
      m_tauplusdecaymode.append(".");
      if (pdg == 94144) {
        pdgname = "alpha";
      } else {
        pdgname = databasePDG->GetParticle(pdg)->GetName();
      }
      m_tauplusdecaymode.append(pdgname);
    }
  }
  //
  m_pmode = TauolaBelle2DecayMode(m_tauplusdecaymode, 1);

}

//
int TauDecayModeModule::TauolaBelle2DecayMode(std::string state, int chg)
{
  std::map<std::string, int> mode_decay = (chg < 0) ? mode_decay_minus : mode_decay_plus;
  map<std::string, int>::iterator itr ;
  for (itr =  mode_decay.begin(); itr !=  mode_decay.end(); ++itr) {
    if (state == itr-> first) {
      int mode = itr-> second;
      if (mode == 39) {
        if (chg < 0 && m_isEtaPizPizPizFromTauMinus) mode = 42;
        if (chg > 0 && m_isEtaPizPizPizFromTauPlus)  mode = 42;
      }
      if (mode == 49) {
        if (chg < 0 && !m_isOmegaPimPipFromTauMinus) mode = 130;
        if (chg > 0 && !m_isOmegaPimPipFromTauPlus)  mode = 130;
      }
      if (mode == 131) {
        if (chg < 0 && !m_isOmegaPimPipFromTauMinus) mode = 236;
        if (chg > 0 && !m_isOmegaPimPipFromTauPlus)  mode = 236;
      }
      if (mode == 247) {
        if (chg < 0 && !m_isEtaPizPizPizFromTauMinus) mode = 15;
        if (chg > 0 && !m_isEtaPizPizPizFromTauPlus)  mode = 15;
      }
      return mode;
    }
  }
  return -1;
}

//
int TauDecayModeModule::getRecursiveMotherCharge(const MCParticle* p)
{
  const MCParticle* mother = p->getMother();
  if (mother == nullptr) {
    return 0;
  } else if (abs(p->getPDG()) == 11 && mother->getPDG() == 111) { // pi0 -> e-e+ gamma
    return 0;
  } else if (abs(p->getPDG()) == 11 && mother->getPDG() == 221) { // eta -> e-e+ gamma
    return 0;
  } else if (abs(p->getPDG()) == 11 && mother->getPDG() == 113) { // rho0 -> e-e+
    return 0;
  } else if (abs(p->getPDG()) == 11 && mother->getPDG() == 223) { // omega -> e- e+
    return 0;
  } else if (abs(p->getPDG()) == 11 && mother->getPDG() == 333) { // phi -> e- e+
    return 0;
  } else if (abs(p->getPDG()) == 13 && mother->getPDG() == 221) { // eta -> mu-mu+ gamma
    return 0;
  } else if (abs(p->getPDG()) == 13 && mother->getPDG() == 113) { // rho0 -> mu-mu+
    return 0;
  } else if (abs(p->getPDG()) == 13 && mother->getPDG() == 333) { // phi -> mu-mu+
    return 0;
  } else if (mother->getPDG() == 15) {
    return -1;
  } else if (mother->getPDG() == -15) {
    return 1;
  } else {
    return getRecursiveMotherCharge(mother);
  }
}

//
void TauDecayModeModule::IdentifyTauPair()
{
  numOfTauPlus = 0;
  numOfTauMinus = 0;
  idOfTauPlus = 0;
  idOfTauMinus = 0;
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle& p = *MCParticles[i];

    if (p.getStatus() == 1 && p.getPDG() == 15) {
      numOfTauMinus++;
      idOfTauMinus = p.getIndex();
    }
    if (p.getStatus() == 1 && p.getPDG() == -15) {
      numOfTauPlus++;
      idOfTauPlus = p.getIndex();
    }
  }
  if (numOfTauPlus == 1 && numOfTauMinus == 1) {
    tauPair = true;
  } else tauPair = false;
}

//
int TauDecayModeModule::getProngOfDecay(const MCParticle& p)
{
  int ret = 0;
  const vector<MCParticle*> daughters = p.getDaughters();
  if (daughters.empty()) return ret;
  for (MCParticle* d : daughters) {
    if (!d->hasStatus(MCParticle::c_PrimaryParticle)) continue;
    // TODO: Improve how to identify a final state particle.
    bool isChargedFinalState = find(begin(finalStatePDGs),
                                    end(finalStatePDGs),
                                    abs(d->getPDG())) != end(finalStatePDGs);
    if (isChargedFinalState) ret++;
    else ret += getProngOfDecay(*d);
  }
  return ret;
}
