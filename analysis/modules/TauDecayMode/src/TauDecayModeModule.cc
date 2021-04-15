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

#include <analysis/modules/TauDecayMode/TauDecayModeModule.h>


#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>


#include <framework/logging/Logger.h>
#include <TLorentzVector.h>
#include <TMatrixFSymfwd.h>
#include "TMath.h"

#include "TKey.h"
#include "TObject.h"

#include <framework/particledb/EvtGenDatabasePDG.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <TParticlePDG.h>
#include <map>
#include <fstream>
#include <string>
using namespace std;
using namespace Belle2;

// this part parse a string with a given separetor [utility code recycled from the past]______________________________________
std::vector<std::string> parseString(std::string str, std::string sep)
{
  std::vector<std::string> parsed;
  int pos = 0;
  bool first = true;
  if (str.size() == 0) return parsed;
  if (str.find(sep) == std::string::npos) {
    parsed.push_back(str);
    return parsed;
  }
  //
  while (true) {
    int newPos = str.find(sep, pos);
    if (str.find(sep, pos) == std::string::npos) {
      if (!first) parsed.push_back(str.substr(pos, newPos - pos));
      break;
    }
    std::string sub = str.substr(pos, newPos - pos);
    parsed.push_back(sub);
    pos = newPos + 1;
    first = false;
  }
  return parsed;
}

std::map<string, int> make_map(const std::string& file)
{
  std::string fileName;
  if (file == "") {
    B2INFO("This is a log message" << LogVar("text",
                                             "Missing input mapping file , use mp_file=basf2.find_file('data/analysis/modules/TauDecayMode/map_tau_vf.txt') TauDecayMode.param('file', mp_file)  for classify with the default mapping."));

  } else {fileName = file;}

  ifstream f;
  f.open(fileName);
  string line;
  string key;
  double value;
  std::map <string, int> map_tau;
  while (f.good()) {
    getline(f, line);
    istringstream ss(line);
    ss >> key >> value;
    map_tau[key] = value;
  }
  f.close();
  return map_tau;
}

// Driver function to sort the vector elements by
// second element of pair in descending order
bool EnergySortInRev(const std::pair<int, double>& a,
                     const std::pair<int, double>& b)
{
  return (a.second > b.second);
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TauDecayMode)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


TauDecayModeModule::TauDecayModeModule() : Module() , m_taum_no(0), m_taup_no(0), m_pmode(-2), m_mmode(-2),
  m_pprong(0), m_mprong(0), tauPair(false), numOfTauPlus(0), numOfTauMinus(0), idOfTauPlus(-1), idOfTauMinus(-1), m_pdg_extra(0)
{
  // Set module properties
  setDescription("Module to identify generated tau pair decays, using MCParticle information. Each tau lepton decay channel "
                 "is by default numbered following the order of TauolaBBB");
  //Parameter definition
  addParam("printmode",      m_printmode,      "Printout more information from each event", std::string("default"));
  addParam("file", m_file, "path for an alternative mapping", std::string(""));
  addParam("particle", m_particle , "Add a particle with his pdg for clasification", std::string(""));
}



//
void TauDecayModeModule::initialize()
{
  mode_decay = make_map(m_file);
  m_tauDecay.registerInDataStore();
  m_event_metadata.isRequired();
  if (m_particle != "") {
    std::vector<std::string> extra = parseString(m_particle, "=");
    m_pdg_extra = atoi(extra[0].c_str());
    m_name = extra[1];
  }

}
void TauDecayModeModule::event()
{

  IdentifyTauPair();
  if (tauPair) {
    m_pprong = getProngOfDecay(*MCParticles[idOfTauPlus - 1]);
    m_mprong = getProngOfDecay(*MCParticles[idOfTauMinus - 1]);
  }


  if (!m_tauDecay) m_tauDecay.create();
  //
  vec_em.clear(), vec_ep.clear(), vec_nue.clear(), vec_anue.clear();
  vec_mum.clear(), vec_mup.clear(), vec_numu.clear(), vec_anumu.clear();
  vec_nut.clear(), vec_anut.clear();
  vec_pim.clear(), vec_pip.clear(), vec_km.clear(), vec_kp.clear(), vec_apro.clear(), vec_pro.clear();
  vec_pi0.clear(), vec_k0s.clear(), vec_k0l.clear(), vec_gam.clear();
  vec_eta.clear(), vec_omega.clear(), vec_kstarp.clear(), vec_kstarm.clear(), vec_lambda.clear(), vec_lmb_br.clear();
  vec_kstar.clear(), vec_kstar_br.clear(), vec_etapr.clear(), vec_a0m.clear(), vec_a0p.clear(), vec_a0.clear();
  vec_b1m.clear(), vec_b1p.clear(), vec_phi.clear(), vec_f1.clear(), vec_a1m.clear(), vec_a1p.clear(),
                vec_rhom.clear(), vec_rhop.clear();
  vec_K0.clear(), vec_K0_br.clear(), vec_rho0.clear(), vec_f0.clear();
  vec_extra.clear();
  //
  map<int, std::vector<int>> map_vec;

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
  map_vec[311] = vec_K0;
  map_vec[-311] = vec_K0_br;
  map_vec[221] = vec_eta;
  map_vec[223] = vec_omega;
  map_vec[323] = vec_kstarp;
  map_vec[-323] = vec_kstarm;
  map_vec[3122] = vec_lambda;
  map_vec[-3122] = vec_lmb_br;
  map_vec[10311] = vec_kstar;
  map_vec[-10311] = vec_kstar_br;
  map_vec[331] = vec_etapr;
  map_vec[9000111] = vec_a0;
  map_vec[9000211] = vec_a0p;
  map_vec[-9000211] = vec_a0m;
  map_vec[-10213] = vec_b1m;
  map_vec[10213] = vec_b1p;
  map_vec[333] = vec_phi;
  map_vec[20223] = vec_f1;
  map_vec[20213] = vec_a1p;
  map_vec[-20213] = vec_a1m;
  map_vec[-213] = vec_rhom;
  map_vec[213] = vec_rhop;
  map_vec[113] = vec_rho0;
  map_vec[9010221] = vec_f0;



  for (int i = 0; i < MCParticles.getEntries(); i++) {

    MCParticle& p = *MCParticles[i];
    if (!p.hasStatus(MCParticle::c_PrimaryParticle)) continue;
    //
    if (p.getPDG() ==  11 && p.isInitial() == 0)  vec_em.push_back(i);
    if (p.getPDG() == -11 && p.isInitial() == 0)  vec_ep.push_back(i);
    if (p.getPDG() ==  22)  vec_gam.push_back(i);
    //
    map<int, std::vector<int>>::iterator ite ;
    for (ite =  map_vec.begin(); ite !=  map_vec.end(); ++ite) {
      if (p.getPDG() == ite-> first) ite-> second.push_back(i);
    }

    if (m_particle != "" && p.getPDG() == m_pdg_extra) vec_extra.push_back(i);
  }

  //
  vec_dau_tauminus.clear();
  vec_dau_tauplus.clear();
  //
  if (m_particle != "") {
    for (unsigned int i = 0; i < vec_extra.size(); i++) {
      int ii = vec_extra[i];
      int chg = getRecursiveMotherCharge(MCParticles[ii]);
      if (chg < 0) vec_dau_tauminus.push_back(ii);
      if (chg > 0) vec_dau_tauplus.push_back(ii);
    }
  }
  //
  for (unsigned int i = 0; i < vec_em.size(); i++) {
    int ii = vec_em[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_ep.size(); i++) {
    int ii = vec_ep[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  map<int, std::vector<int>>::iterator itr ;
  for (itr =  map_vec.begin(); itr !=  map_vec.end(); ++itr) {
    for (unsigned int i = 0; i < itr-> second.size(); i++) {
      int ii = itr-> second[i];
      int chg = getRecursiveMotherCharge(MCParticles[ii]);
      if (chg < 0) vec_dau_tauminus.push_back(ii);
      if (chg > 0) vec_dau_tauplus.push_back(ii);
    }
  }

  std::vector< std::pair<int, double> > vec_gamtmp_tauminus;
  std::vector< std::pair<int, double> > vec_gamtmp_tauplus;
  for (unsigned int i = 0; i < vec_gam.size(); i++) {
    int ii = vec_gam[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_gamtmp_tauminus.push_back(make_pair(ii, MCParticles[ii]->get4Vector().E()));
    if (chg > 0) vec_gamtmp_tauplus.push_back(make_pair(ii, MCParticles[ii]->get4Vector().E()));
  }
  //
  std::sort(vec_gamtmp_tauminus.begin(), vec_gamtmp_tauminus.end(), EnergySortInRev);
  for (unsigned int i = 0; i < vec_gamtmp_tauminus.size(); i++) {
    int ii = vec_gamtmp_tauminus[i].first;
    MCParticle* p = MCParticles[ii];
    double Estar = getEnergyTauRestFrame(p, -1);
    if (Estar > 0.00) { // tune ?
      vec_dau_tauminus.push_back(ii);
    }
  }
  //
  std::sort(vec_gamtmp_tauplus.begin(), vec_gamtmp_tauplus.end(), EnergySortInRev);
  for (unsigned int i = 0; i < vec_gamtmp_tauplus.size(); i++) {
    int ii = vec_gamtmp_tauplus[i].first;
    MCParticle* p = MCParticles[ii];
    double Estar = getEnergyTauRestFrame(p, +1);
    if (Estar > 0.00) { // tune ?
      vec_dau_tauplus.push_back(ii);
    }
  }

  EvtGenDatabasePDG* databasePDG = EvtGenDatabasePDG::Instance();
  //make decay string for t-
  m_tauminusdecaymode = "";
  for (unsigned int i = 0; i < vec_dau_tauminus.size(); i++) {
    MCParticle* p = MCParticles[vec_dau_tauminus[i]];

    int pdg = p->getPDG();

    //
    if (m_particle != "" && pdg == m_pdg_extra) m_tauminusdecaymode.append("." + m_name);
    //
    m_tauminusdecaymode.append(".");
    m_tauminusdecaymode.append(databasePDG->GetParticle(pdg)->GetName());

  }

  if (m_printmode == "missing") {

    if (TauBBBmode(m_tauminusdecaymode) == -1) {
      B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauminusdecaymode));
      B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent() << " TauMinusDecayMode: tau- -> " << m_tauminusdecaymode);

    }
  }

  if (m_printmode == "all") {
    B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauminusdecaymode));
    B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent() << " TauMinusDecayMode: tau- -> " <<
           m_tauminusdecaymode);
  }


  m_tauplusdecaymode = "";
  for (unsigned int i = 0; i < vec_dau_tauplus.size(); i++) {
    MCParticle* p = MCParticles[vec_dau_tauplus[i]];
    int pdg = p->getPDG();
    //
    if (m_particle != "" && pdg == m_pdg_extra) m_tauplusdecaymode.append("." + m_name);


    m_tauplusdecaymode.append(".");
    m_tauplusdecaymode.append(databasePDG->GetParticle(pdg)->GetName());



  }
  if (m_printmode == "missing") {
    if (TauBBBmode(m_tauplusdecaymode) == -1) {
      B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauplusdecaymode));
      B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent() << " TauMinusDecayMode: tau+ -> " << m_tauplusdecaymode);
    }
  }

  if (m_printmode == "all") {
    B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauplusdecaymode));
    B2INFO("TauDecayMode:: EventNumber = " << m_event_metadata->getEvent() << " TauPlusDecayMode: tau+ -> " <<
           m_tauplusdecaymode);
  }
  //

  m_mmode = TauBBBmode(m_tauminusdecaymode);
  m_pmode = TauBBBmode(m_tauplusdecaymode);
  if (m_mmode == -1) {
    m_taum_no = m_taum_no - 1;
    m_mmode = m_taum_no;
  }
  if (m_pmode == -1) {
    m_taup_no = m_taup_no - 1;
    m_pmode = m_taup_no;
  }

  m_tauDecay->addTauMinusIdMode(m_mmode);
  m_tauDecay->addTauPlusIdMode(m_pmode);

  m_tauDecay->addTauPlusMcProng(m_pprong);
  m_tauDecay->addTauMinusMcProng(m_mprong);


}


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
double TauDecayModeModule::getEnergyTauRestFrame(const MCParticle* p, const int ichg)
{
  //
  if (p == nullptr) {
    return 0.0;
  }
  //
  TLorentzVector mom_4vecLAB(0.0, 0.0, 0.0, 0.0);
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle* m = MCParticles[i];
    if (m->getStatus() == 1 && m->getPDG() == -15 * ichg) {
      mom_4vecLAB = m->get4Vector();
      break;
    }
  }
  if (mom_4vecLAB.E() == 0) return 0.0;
  //
  TLorentzVector part_4vecLAB = p->get4Vector();
  TVector3 boostMom = mom_4vecLAB.BoostVector();
  part_4vecLAB.Boost(-boostMom);
  return part_4vecLAB.E();
}
//
int TauDecayModeModule::getRecursiveMotherCharge(const MCParticle* p)
{
  const MCParticle* mother = p->getMother();
  if (mother == nullptr) {
    return 0;
  } else if (p->getPDG() == 22 && (mother->getPDG() == 111)) {
    return 0;
  } else if (abs(p->getPDG()) == 11 && mother->getPDG() == 111) {
    return 0;
  } else if (mother->getPDG() == 15) {
    return -1;
  } else if (mother->getPDG() == -15) {
    return 1;
  } else {
    return getRecursiveMotherCharge(mother);
  }
}




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


//
int TauDecayModeModule::TauBBBmode(string state)
{
  string dem = ".";
  std::vector<std::string> x = parseString(state, dem);
  int r = x.size();
  int i;
  map<string, int>::iterator itr ;
  for (itr =  mode_decay.begin(); itr !=  mode_decay.end(); ++itr) {
    string mode = itr-> first;
    std::vector<std::string> y = parseString(itr -> first, dem);
    int b = y.size();
    int j;
    int val = 0;
    for (j = 0; j != b; ++j) {
      if (state.find(y[j]) == string::npos) {
        val = val + 1;
        break;
      }
    }
    std::set<std::string> const uniques(x.begin(), x.end());
    x.assign(uniques.begin(), uniques.end());
    int count = 0;
    int nein = 0;
    for (i = 0 ; i != (int)x.size(); ++i) {
      int pos = 0;
      int index;

      if ((index = mode.find(x[i], pos)) == (int)string::npos) {
        nein = nein + 1;
      } else {
        while ((index = mode.find(x[i], pos)) != (int)string::npos) {
          if (x[i] != "") {count = count + 1;}

          if ((index = mode.find(x[i], pos)) == (int)string::npos) {
            break;
          }
          pos = index + 1;
        }
      }
    }
    if ((b == r) & (b - 1 == count) & (nein == 0) & (val == 0)) {
      int tau_mode = itr-> second;
      return tau_mode;
    }

  }
  return -1;

}

