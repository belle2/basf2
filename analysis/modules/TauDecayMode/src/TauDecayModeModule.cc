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


#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>
#include <utility>

#include <framework/logging/Logger.h>
#include <TLorentzVector.h>
#include <TMatrixFSymfwd.h>
#include "TMath.h"

#include "TKey.h"
#include "TObject.h"

#include <iostream>
#include <vector>
#include <algorithm>
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


TauDecayModeModule::TauDecayModeModule() : Module() , EventNumber(1), nop(0) , taum_no(0), taup_no(0), m_pmode(-2), m_mmode(-2),
  m_pprong(0), m_mprong(0), tauPair(false), numOfTauPlus(0), numOfTauMinus(0), idOfTauPlus(-1), idOfTauMinus(-1), pdg_extra(0)
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
  // nop = 0 ;
  // taum_no = 0;
  // taup_no = 0;
  // EventNumber = 1;
  mode_decay = make_map(m_file);
  m_tauDecay.registerInDataStore();
  if (m_particle != "") {
    std::vector<std::string> extra = parseString(m_particle, "=");
    pdg_extra = atoi(extra[0].c_str());
    name = extra[1];
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

  for (int i = 0; i < MCParticles.getEntries(); i++) {

    MCParticle& p = *MCParticles[i];
    if (!p.hasStatus(MCParticle::c_PrimaryParticle)) continue;
    //
    if (p.getPDG() ==  11 && p.isInitial() == 0)  vec_em.push_back(i);
    if (p.getPDG() == -11 && p.isInitial() == 0)  vec_ep.push_back(i);
    //
    if (p.getPDG() ==  12)  vec_nue.push_back(i);
    if (p.getPDG() == -12)  vec_anue.push_back(i);
    //
    if (p.getPDG() ==  13)  vec_mum.push_back(i);
    if (p.getPDG() == -13)  vec_mup.push_back(i);
    //
    if (p.getPDG() ==  14)  vec_numu.push_back(i);
    if (p.getPDG() == -14)  vec_anumu.push_back(i);
    //
    if (p.getPDG() ==  16)  vec_nut.push_back(i);
    if (p.getPDG() == -16)  vec_anut.push_back(i);
    //
    if (p.getPDG() == -211) vec_pim.push_back(i);
    if (p.getPDG() ==  211) vec_pip.push_back(i);
    //
    if (p.getPDG() == -321) vec_km.push_back(i);
    if (p.getPDG() ==  321) vec_kp.push_back(i);
    //
    if (p.getPDG() == -2212)vec_apro.push_back(i);
    if (p.getPDG() ==  2212)vec_pro.push_back(i);
    //
    if (p.getPDG() ==  111) vec_pi0.push_back(i);
    if (p.getPDG() ==  310) vec_k0s.push_back(i);
    if (p.getPDG() ==  130) vec_k0l.push_back(i);
    if (p.getPDG() ==  22)  vec_gam.push_back(i);
    if (p.getPDG() ==  311) vec_K0.push_back(i);
    if (p.getPDG() ==  -311) vec_K0_br.push_back(i);
    //
    if (p.getPDG() ==  221) vec_eta.push_back(i);
    if (p.getPDG() ==  223) vec_omega.push_back(i);
    if (p.getPDG() ==  323) vec_kstarp.push_back(i);
    if (p.getPDG() == -323) vec_kstarm.push_back(i);
    // add particles
    if (p.getPDG() == 3122) vec_lambda.push_back(i);
    if (p.getPDG() == -3122) vec_lmb_br.push_back(i);
    if (p.getPDG() == 10311) vec_kstar.push_back(i);
    if (p.getPDG() == -10311) vec_kstar_br.push_back(i);
    if (p.getPDG() == 331) vec_etapr.push_back(i);
    if (p.getPDG() == 9000111) vec_a0.push_back(i);
    if (p.getPDG() == 9000211) vec_a0p.push_back(i);
    if (p.getPDG() == -9000211) vec_a0m.push_back(i);
    if (p.getPDG() == -10213) vec_b1m.push_back(i);
    if (p.getPDG() == 10213) vec_b1p.push_back(i);
    if (p.getPDG() == 333) vec_phi.push_back(i);
    if (p.getPDG() == 20223) vec_f1.push_back(i);
    if (p.getPDG() == 20213) vec_a1p.push_back(i);
    if (p.getPDG() == -20213) vec_a1m.push_back(i);
    if (p.getPDG() == -213) vec_rhom.push_back(i);
    if (p.getPDG() == 213) vec_rhop.push_back(i);
    if (p.getPDG() == 113) vec_rho0.push_back(i);
    if (p.getPDG() == 9010221) vec_f0.push_back(i);
    if (m_particle != "" && p.getPDG() == pdg_extra) vec_extra.push_back(i);
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
  for (unsigned int i = 0; i < vec_nue.size(); i++) {
    int ii = vec_nue[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_anue.size(); i++) {
    int ii = vec_anue[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_mum.size(); i++) {
    int ii = vec_mum[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_mup.size(); i++) {
    int ii = vec_mup[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_numu.size(); i++) {
    int ii = vec_numu[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_anumu.size(); i++) {
    int ii = vec_anumu[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_nut.size(); i++) {
    int ii = vec_nut[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_anut.size(); i++) {
    int ii = vec_anut[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_pim.size(); i++) {
    int ii = vec_pim[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_pip.size(); i++) {
    int ii = vec_pip[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_km.size(); i++) {
    int ii = vec_km[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_kp.size(); i++) {
    int ii = vec_kp[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_apro.size(); i++) {
    int ii = vec_apro[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_pro.size(); i++) {
    int ii = vec_pro[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_pi0.size(); i++) {
    int ii = vec_pi0[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_k0s.size(); i++) {
    int ii = vec_k0s[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_k0l.size(); i++) {
    int ii = vec_k0l[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
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
  //
  for (unsigned int i = 0; i < vec_eta.size(); i++) {
    int ii = vec_eta[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_omega.size(); i++) {
    int ii = vec_omega[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_kstarp.size(); i++) {
    int ii = vec_kstarp[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_kstarm.size(); i++) {
    int ii = vec_kstarm[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //extra particles
  for (unsigned int i = 0; i < vec_lambda.size(); i++) {
    int ii = vec_lambda[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_lmb_br.size(); i++) {
    int ii = vec_lmb_br[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_kstar.size(); i++) {
    int ii = vec_kstar[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_kstar_br.size(); i++) {
    int ii = vec_kstar_br[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_etapr.size(); i++) {
    int ii = vec_etapr[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //

  for (unsigned int i = 0; i < vec_a0.size(); i++) {
    int ii = vec_a0[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_a0p.size(); i++) {
    int ii = vec_a0p[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_a0m.size(); i++) {
    int ii = vec_a0m[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_b1m.size(); i++) {
    int ii = vec_b1m[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_b1p.size(); i++) {
    int ii = vec_b1p[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_phi.size(); i++) {
    int ii = vec_phi[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_f1.size(); i++) {
    int ii = vec_f1[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_a1p.size(); i++) {
    int ii = vec_a1p[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  //
  for (unsigned int i = 0; i < vec_a1m.size(); i++) {
    int ii = vec_a1m[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_rhom.size(); i++) {
    int ii = vec_rhom[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //
  for (unsigned int i = 0; i < vec_rhop.size(); i++) {
    int ii = vec_rhop[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  for (unsigned int i = 0; i < vec_K0.size(); i++) {
    int ii = vec_K0[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  for (unsigned int i = 0; i < vec_K0_br.size(); i++) {
    int ii = vec_K0_br[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  for (unsigned int i = 0; i < vec_rho0.size(); i++) {
    int ii = vec_rho0[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }
  for (unsigned int i = 0; i < vec_f0.size(); i++) {
    int ii = vec_f0[i];
    int chg = getRecursiveMotherCharge(MCParticles[ii]);
    if (chg < 0) vec_dau_tauminus.push_back(ii);
    if (chg > 0) vec_dau_tauplus.push_back(ii);
  }

  //make decay string for t-
  m_tauminusdecaymode = "";
  for (unsigned int i = 0; i < vec_dau_tauminus.size(); i++) {
    MCParticle* p = MCParticles[vec_dau_tauminus[i]];

    int pdg = p->getPDG();
    //
    if (m_particle != "" && pdg == pdg_extra) m_tauminusdecaymode.append("." + name);
    //
    if (pdg ==  16)  m_tauminusdecaymode.append(".nu_tau");
    if (pdg == -16)  m_tauminusdecaymode.append(".anti-nu_tau");
    //
    if (pdg ==  11)  m_tauminusdecaymode.append(".e-");
    if (pdg == -11)  m_tauminusdecaymode.append(".e+");

    if (pdg ==  12)  m_tauminusdecaymode.append(".nu_e");
    if (pdg == -12)  m_tauminusdecaymode.append(".anti-nu_e");

    if (pdg ==  13)  m_tauminusdecaymode.append(".mu-");
    if (pdg == -13)  m_tauminusdecaymode.append(".mu+");

    if (pdg ==  14)  m_tauminusdecaymode.append(".nu_mu");
    if (pdg == -14)  m_tauminusdecaymode.append(".anti-nu_mu");
    //
    if (pdg == -211) m_tauminusdecaymode.append(".pi-");
    if (pdg ==  211) m_tauminusdecaymode.append(".pi+");
    //
    if (pdg == -321) m_tauminusdecaymode.append(".K-");
    if (pdg ==  321) m_tauminusdecaymode.append(".K+");
    //
    if (pdg == 311) m_tauminusdecaymode.append(".K0");
    if (pdg == -311) m_tauminusdecaymode.append(".anti-K0");

    if (pdg == -2212)m_tauminusdecaymode.append(".anti-p-");
    if (pdg ==  2212)m_tauminusdecaymode.append(".p+");

    if (pdg == 111)  m_tauminusdecaymode.append(".pi0");
    if (pdg == 310)  m_tauminusdecaymode.append(".K_S0");
    if (pdg == 130)  m_tauminusdecaymode.append(".K_L0");
    if (pdg == 22)   m_tauminusdecaymode.append(".gamma");
    //
    if (pdg == 3122) m_tauminusdecaymode.append(".Lambda0");
    if (pdg == -3122) m_tauminusdecaymode.append(".anti-Lambda0");
    if (pdg == 10311) m_tauminusdecaymode.append(".K_0*0");
    if (pdg == -10311) m_tauminusdecaymode.append(".anti-K_0*0");
    if (pdg == 331) m_tauminusdecaymode.append(".eta'");


    if (pdg == 221) m_tauminusdecaymode.append(".eta");
    if (pdg == 223) m_tauminusdecaymode.append(".omega");
    if (pdg == 323) m_tauminusdecaymode.append(".K*+");
    if (pdg == -323) m_tauminusdecaymode.append(".K*-");
    if (pdg == 9000111) m_tauminusdecaymode.append(".a00");
    if (pdg == 9000211) m_tauminusdecaymode.append(".a_0+");
    if (pdg == -9000211) m_tauminusdecaymode.append(".a_0-");
    if (pdg == 10213) m_tauminusdecaymode.append(".b_1+");
    if (pdg == -10213) m_tauminusdecaymode.append(".b_1-");
    if (pdg == 333) m_tauminusdecaymode.append(".phi");
    if (pdg == 20223) m_tauminusdecaymode.append(".f_1");
    if (pdg == 20213) m_tauminusdecaymode.append(".a_1+");
    if (pdg == -20213) m_tauminusdecaymode.append(".a_1-");
    if (pdg == 213) m_tauminusdecaymode.append(".rho+");
    if (pdg == -213) m_tauminusdecaymode.append(".rho-");
    if (pdg == 113) m_tauminusdecaymode.append(".rho0");
    if (pdg == 9010221) m_tauminusdecaymode.append(".f_0");
  }

  if (m_printmode == "missing") {

    if (TauBBBmode(m_tauminusdecaymode) == -1) {
      B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauminusdecaymode));
      B2INFO("TauDecayMode:: EventNumber = " << EventNumber << " TauMinusDecayMode: tau- -> " << m_tauminusdecaymode);

    }
  }

  if (m_printmode == "all") {
    B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauminusdecaymode));
    B2INFO("TauDecayMode:: EventNumber = " << EventNumber << " TauMinusDecayMode: tau- -> " <<
           m_tauminusdecaymode);
  }


  m_tauplusdecaymode = "";
  for (unsigned int i = 0; i < vec_dau_tauplus.size(); i++) {
    MCParticle* p = MCParticles[vec_dau_tauplus[i]];
    int pdg = p->getPDG();
    //
    if (m_particle != "" && pdg == pdg_extra) m_tauplusdecaymode.append("." + name);
    //
    if (pdg ==  16)  m_tauplusdecaymode.append(".nu_tau");
    if (pdg == -16)  m_tauplusdecaymode.append(".anti-nu_tau");
    //
    if (pdg ==  11)  m_tauplusdecaymode.append(".e-");
    if (pdg == -11)  m_tauplusdecaymode.append(".e+");

    if (pdg ==  12)  m_tauplusdecaymode.append(".nu_e");
    if (pdg == -12)  m_tauplusdecaymode.append(".anti-nu_e");

    if (pdg ==  13)  m_tauplusdecaymode.append(".mu-");
    if (pdg == -13)  m_tauplusdecaymode.append(".mu+");

    if (pdg ==  14)  m_tauplusdecaymode.append(".nu_mu");
    if (pdg == -14)  m_tauplusdecaymode.append(".anti-nu_mu");
    //
    if (pdg == -211) m_tauplusdecaymode.append(".pi-");
    if (pdg ==  211) m_tauplusdecaymode.append(".pi+");
    //
    if (pdg == -321) m_tauplusdecaymode.append(".K-");
    if (pdg ==  321) m_tauplusdecaymode.append(".K+");
    //
    if (pdg == 311) m_tauplusdecaymode.append(".K0");
    if (pdg == -311) m_tauplusdecaymode.append(".anti-K0");

    if (pdg == -2212)m_tauplusdecaymode.append(".anti-p-");
    if (pdg ==  2212)m_tauplusdecaymode.append(".p+");

    if (pdg == 111)  m_tauplusdecaymode.append(".pi0");
    if (pdg == 310)  m_tauplusdecaymode.append(".K_S0");
    if (pdg == 130)  m_tauplusdecaymode.append(".K_L0");
    if (pdg == 22)   m_tauplusdecaymode.append(".gamma");
    //
    if (pdg == 3122) m_tauplusdecaymode.append(".Lambda0");
    if (pdg == -3122) m_tauplusdecaymode.append(".anti-Lambda0");
    if (pdg == 10311) m_tauplusdecaymode.append(".K_0*0");
    if (pdg == -10311) m_tauplusdecaymode.append(".anti-K_0*0");
    if (pdg == 331) m_tauplusdecaymode.append(".eta'");


    if (pdg == 221) m_tauplusdecaymode.append(".eta");
    if (pdg == 223) m_tauplusdecaymode.append(".omega");
    if (pdg == 323) m_tauplusdecaymode.append(".K*+");
    if (pdg == -323) m_tauplusdecaymode.append(".K*-");
    if (pdg == 9000111) m_tauplusdecaymode.append(".a00");
    if (pdg == 9000211) m_tauplusdecaymode.append(".a_0+");
    if (pdg == -9000211) m_tauplusdecaymode.append(".a_0-");
    if (pdg == 10213) m_tauplusdecaymode.append(".b_1+");
    if (pdg == -10213) m_tauplusdecaymode.append(".b_1-");
    if (pdg == 333) m_tauplusdecaymode.append(".phi");
    if (pdg == 20223) m_tauplusdecaymode.append(".f_1");
    if (pdg == 20213) m_tauplusdecaymode.append(".a_1+");
    if (pdg == -20213) m_tauplusdecaymode.append(".a_1-");
    if (pdg == 213) m_tauplusdecaymode.append(".rho+");
    if (pdg == -213) m_tauplusdecaymode.append(".rho-");
    if (pdg == 113) m_tauplusdecaymode.append(".rho0");
    if (pdg == 9010221) m_tauplusdecaymode.append(".f_0");
    //


  }
  if (m_printmode == "missing") {
    if (TauBBBmode(m_tauplusdecaymode) == -1) {
      B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauplusdecaymode));
      B2INFO("TauDecayMode:: EventNumber = " << EventNumber << " TauMinusDecayMode: tau+ -> " << m_tauplusdecaymode);
    }
  }

  if (m_printmode == "all") {
    B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauplusdecaymode));
    B2INFO("TauDecayMode:: EventNumber = " << EventNumber << " TauPlusDecayMode: tau+ -> " <<
           m_tauplusdecaymode);
  }
  //

  m_mmode = TauBBBmode(m_tauminusdecaymode);
  m_pmode = TauBBBmode(m_tauplusdecaymode);
  if (m_mmode == -1) {
    taum_no = taum_no - 1;
    m_mmode = taum_no;
  }
  if (m_pmode == -1) {
    taup_no = taup_no - 1;
    m_pmode = taup_no;
  }

  m_tauDecay->addTauMinusIdMode(m_mmode);
  m_tauDecay->addTauPlusIdMode(m_pmode);

  m_tauDecay->addTauPlusMcProng(m_pprong);
  m_tauDecay->addTauMinusMcProng(m_mprong);

  EventNumber = EventNumber + 1;
  //
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
  //= mode_decay.begin()
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

