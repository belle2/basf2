/**************************************************************************
    * BASF2 (Belle Analysis Framework 2)                                     *
    * Copyright(C) 2020 - Belle II Collaboration                             *
    *                                                                        *
    * Author: The Belle II Collaboration                                     *
    * Contributors: Leonardo Salinas                                         *
    *                                                                        *
    * This software is provided "as is" without any warranty.                *
    **************************************************************************/

#include <TauDecayModeModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>
#include <utility>

#include <framework/logging/Logger.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixFSymfwd.h>
#include "TMath.h"
#include "TCanvas.h"
#include "TBox.h"

#include "TColor.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TDirectory.h"
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


// this part parse a string with a given separetor___________________________________________________________________________
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

std::map<string, int> make_map()
{
  const char* fileName = "/home/evilgauss/Documentos/BelleII/TauolaBBB/workdir/map_tau_v2.txt";
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


TauDecayModeModule::TauDecayModeModule() : Module()
{
  // Set module properties
  setDescription("Fill the histograms with the values of the MCParticle collection");
  //Parameter definition
  addParam("printmode",      m_printmode,      "Printout more information from each event", 0);
}
//
void TauDecayModeModule::initialize()
{

  EventNumber = 1;
  mode_decay = make_map();
  //vec_b1m, vec_b1p, vec_phi, vec_f1, vec_a1m, vec_a1p, vec_rhom, vec_rhop
}
void TauDecayModeModule::event()
{
  StoreArray<MCParticle> MCParticles;
  //

  //
  vec_em.clear(), vec_ep.clear(), vec_nue.clear(), vec_anue.clear();
  vec_mum.clear(), vec_mup.clear(), vec_numu.clear(), vec_anumu.clear();
  vec_nut.clear(), vec_anut.clear();
  vec_pim.clear(), vec_pip.clear(), vec_km.clear(), vec_kp.clear(), vec_apro.clear(), vec_pro.clear();
  vec_pi0.clear(), vec_k0s.clear(), vec_k0l.clear(), vec_gam.clear();
  vec_eta.clear(), vec_omega.clear(), vec_kstarp.clear(), vec_kstarm.clear(), vec_lambda.clear(), vec_lmb_br.clear();
  vec_kstar.clear(), vec_kstar_br.clear(), vec_etapr.clear(), vec_a0m.clear(), vec_a0p.clear();
  vec_b1m.clear(), vec_b1p.clear(), vec_phi.clear(), vec_f1.clear(), vec_a1m.clear(), vec_a1p.clear(), vec_rhom.clear(),
                vec_rhop.clear();
  vec_K0.clear(), vec_K0_br.clear(), vec_rho0.clear(), vec_f0.clear();
  //
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle& p = *MCParticles[i];
    //B2INFO("TauDecayMode::PDG: " << p.getPDG() << " p.hasStatus(2): " << p.hasStatus(2)
    //       <<" isInitial: " << p.isInitial() << " i: " << i << "  here");
    //if (p.hasStatus(2) == 1) B2INFO("i: " << i << " PDG: "<< p.getPDG() << " isInitial: " << p.isInitial() << "  here");
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




  }
  //
  if (m_printmode > 1) {
    B2INFO("TauDecayMode:: vec_nut.size()  = " << vec_nut.size());
    B2INFO("TauDecayMode:: vec_anut.size() = " << vec_anut.size());
    B2INFO("TauDecayMode:: vec_em.size()   = "  << vec_em.size());
    B2INFO("TauDecayMode:: vec_ep.size()   = "  << vec_ep.size());
    B2INFO("TauDecayMode:: vec_pim.size()  = " << vec_pim.size());
    B2INFO("TauDecayMode:: vec_pip.size()  = " << vec_pip.size());
    B2INFO("TauDecayMode:: vec_pi0.size()  = " << vec_pi0.size());
    B2INFO("TauDecayMode:: vec_gam.size()  = " << vec_gam.size());
// test for extra particles
    B2INFO("TauDecayMode:: vec_K0.size()  = " << vec_K0.size());
    B2INFO("TauDecayMode:: vec_K0_br.size()  = " << vec_K0_br.size());
    B2INFO("TauDecayMode:: vec_k0s.size()  = " << vec_k0s.size());
    B2INFO("TauDecayMode:: vec_k0l.size()  = " << vec_k0l .size());
    B2INFO("TauDecayMode:: vec_eta.size()  = " << vec_eta.size());
    B2INFO("TauDecayMode:: vec_omega.size()  = " << vec_omega.size());
    B2INFO("TauDecayMode:: vec_kstarm.size()  = " << vec_kstarm.size());
    B2INFO("TauDecayMode:: vec_kstarp.size()  = " << vec_kstarp.size());
    B2INFO("TauDecayMode:: vec_lambda.size()  = " << vec_lambda.size());
    B2INFO("TauDecayMode:: vec_lmb_br.size()  = " << vec_lmb_br.size());
    B2INFO("TauDecayMode:: vec_kstar.size()  = " << vec_kstar.size());
    B2INFO("TauDecayMode:: vec_kstar_br.size()  = " << vec_kstar_br.size());
    B2INFO("TauDecayMode:: vec_etapr.size()  = " << vec_etapr.size());
    B2INFO("TauDecayMode:: vec_a0p.size()  = " << vec_a0p.size());
    B2INFO("TauDecayMode:: vec_a0m.size()  = " << vec_a0m.size());
    B2INFO("TauDecayMode:: vec_b1m.size()  = " << vec_b1m.size());
    B2INFO("TauDecayMode:: vec_b1p.size()  = " << vec_b1p.size());
    B2INFO("TauDecayMode:: vec_phi.size()  = " << vec_phi.size());
    B2INFO("TauDecayMode:: vec_a1p.size()  = " << vec_a1p.size());
    B2INFO("TauDecayMode:: vec_a1m.size()  = " << vec_a1m.size());
    B2INFO("TauDecayMode:: vec_rhom.size()  = " << vec_rhom.size());
    B2INFO("TauDecayMode:: vec_rhop.size()  = " << vec_rhop.size());
    B2INFO("TauDecayMode:: vec_rho0.size()  = " << vec_rho0.size());
    B2INFO("TauDecayMode:: vec_f0.size()  = " << vec_f0.size());
  }
  //
  vec_dau_tauminus.clear();
  vec_dau_tauplus.clear();
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
    if (m_printmode > 2) cout << "vec_gamtmp_tauminus: i = " << i << " ii = " << ii << " E = " << p->get4Vector().E() << " EStar = " <<
                                Estar << endl;
    if (Estar > 0.1) { // tune ?
      vec_dau_tauminus.push_back(ii);
    }
  }
  //
  std::sort(vec_gamtmp_tauplus.begin(), vec_gamtmp_tauplus.end(), EnergySortInRev);
  for (unsigned int i = 0; i < vec_gamtmp_tauplus.size(); i++) {
    int ii = vec_gamtmp_tauplus[i].first;
    MCParticle* p = MCParticles[ii];
    double Estar = getEnergyTauRestFrame(p, +1);
    if (m_printmode > 2) cout << "vec_gamtmp_tauplus: i = " << i << " ii = " << ii << " E = " << p->get4Vector().E() << " EStar = " <<
                                Estar << endl;
    if (Estar > 0.1) { // tune ?
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

  //
  m_tauminusdecaymode = "";
  for (unsigned int i = 0; i < vec_dau_tauminus.size(); i++) {
    MCParticle* p = MCParticles[vec_dau_tauminus[i]];

    int pdg = p->getPDG();
    //
    if (pdg ==  16)  m_tauminusdecaymode.append(".nut");
    if (pdg == -16)  m_tauminusdecaymode.append(".anut");
    //
    if (pdg ==  11)  m_tauminusdecaymode.append(".e-");
    if (pdg == -11)  m_tauminusdecaymode.append(".e+");

    if (pdg ==  12)  m_tauminusdecaymode.append(".nue");
    if (pdg == -12)  m_tauminusdecaymode.append(".anue");

    if (pdg ==  13)  m_tauminusdecaymode.append(".mu-");
    if (pdg == -13)  m_tauminusdecaymode.append(".mu+");

    if (pdg ==  14)  m_tauminusdecaymode.append(".numu");
    if (pdg == -14)  m_tauminusdecaymode.append(".anumu");
    //
    if (pdg == -211) m_tauminusdecaymode.append(".pi-");
    if (pdg ==  211) m_tauminusdecaymode.append(".pi+");
    //
    if (pdg == -321) m_tauminusdecaymode.append(".K-");
    if (pdg ==  321) m_tauminusdecaymode.append(".K+");
    //
    if (pdg == 311) m_tauminusdecaymode.append(".K0");
    if (pdg == -311) m_tauminusdecaymode.append(".K0bar");

    if (pdg == -2212)m_tauminusdecaymode.append(".apro-");
    if (pdg ==  2212)m_tauminusdecaymode.append(".pro+");

    if (pdg == 111)  m_tauminusdecaymode.append(".pi0");
    if (pdg == 310)  m_tauminusdecaymode.append(".K0S");
    if (pdg == 130)  m_tauminusdecaymode.append(".K0L");
    if (pdg == 22)   m_tauminusdecaymode.append(".gamma");
    //
    if (pdg == 3122) m_tauminusdecaymode.append(".lambda");
    if (pdg == -3122) m_tauminusdecaymode.append(".lmbbar");
    if (pdg == 10311) m_tauminusdecaymode.append(".kstar");
    if (pdg == -10311) m_tauminusdecaymode.append(".kstarbr");
    if (pdg == 331) m_tauminusdecaymode.append(".etapr");

    //test with resonances
    if (pdg == 221) m_tauminusdecaymode.append(".|eta|");
    if (pdg == 223) m_tauminusdecaymode.append(".|omega|");
    if (pdg == 323) m_tauminusdecaymode.append(".kstarp");
    if (pdg == -323) m_tauminusdecaymode.append(".kstarm");
    if (pdg == 9000211) m_tauminusdecaymode.append(".|a0p|");
    if (pdg == -9000211) m_tauminusdecaymode.append(".|a0m|");
    if (pdg == 10213) m_tauminusdecaymode.append(".|b1p|");
    if (pdg == -10213) m_tauminusdecaymode.append(".|b1m|");
    if (pdg == 333) m_tauminusdecaymode.append(".|phi|");
    if (pdg == 20223) m_tauminusdecaymode.append(".|f1|");
    if (pdg == 20213) m_tauminusdecaymode.append(".|a1p|");
    if (pdg == -20213) m_tauminusdecaymode.append(".|a1m|");
    if (pdg == 213) m_tauminusdecaymode.append(".|rhop|");
    if (pdg == -213) m_tauminusdecaymode.append(".|rhom|");
    if (pdg == 113) m_tauminusdecaymode.append(".rho0");
    if (pdg == 9010221) m_tauminusdecaymode.append(".|f0|");
  }
//
  //
  //if (m_printmode>1) B2INFO("TauDecayMode:: PDG inside vec_dau_tauminus = " << pdg << " tauminusdecaymode = " << m_tauminusdecaymode);
  //


  if (m_printmode > 3) B2INFO("TauDecayMode:: Decay mode is =" << TauBBBmode(m_tauminusdecaymode, mode_decay));
  //m_tauminusdecaymode=m_tauminusdecaymode.erase(0,1); // remove the leading dot
  if (m_printmode > 0) B2INFO("TauDecayMode:: EventNumber = " << EventNumber << " TauMinusDecayMode: tau- -> " <<
                                m_tauminusdecaymode);
  //
  m_tauplusdecaymode = "";
  for (unsigned int i = 0; i < vec_dau_tauplus.size(); i++) {
    MCParticle* p = MCParticles[vec_dau_tauplus[i]];
    int pdg = p->getPDG();
    //
    if (pdg ==  16)   m_tauplusdecaymode.append(".nut");
    if (pdg == -16)   m_tauplusdecaymode.append(".anut");
    //
    if (pdg ==  11)   m_tauplusdecaymode.append(".e-");
    if (pdg == -11)   m_tauplusdecaymode.append(".e+");
    //
    if (pdg ==  12)   m_tauplusdecaymode.append(".nue");
    if (pdg == -12)   m_tauplusdecaymode.append(".anue");
    //
    if (pdg ==  13)   m_tauplusdecaymode.append(".mu-");
    if (pdg == -13)   m_tauplusdecaymode.append(".mu+");
    //
    if (pdg ==  14)   m_tauplusdecaymode.append(".numu");
    if (pdg == -14)   m_tauplusdecaymode.append(".anumu");
    //
    if (pdg == -211)  m_tauplusdecaymode.append(".pi-");
    if (pdg ==  211)  m_tauplusdecaymode.append(".pi+");
    //
    if (pdg == -321)  m_tauplusdecaymode.append(".K-");
    if (pdg ==  321)  m_tauplusdecaymode.append(".K+");
    //
    if (pdg == 311) m_tauplusdecaymode.append(".K0");
    if (pdg == -311) m_tauplusdecaymode.append(".K0bar");
    //
    if (pdg == -2212) m_tauplusdecaymode.append(".apro-");
    if (pdg ==  2212) m_tauplusdecaymode.append(".pro+");
    //
    if (pdg == 111)   m_tauplusdecaymode.append(".pi0");
    if (pdg == 310)   m_tauplusdecaymode.append(".K0S");
    if (pdg == 130)   m_tauplusdecaymode.append(".K0L");
    if (pdg == 22)    m_tauplusdecaymode.append(".gamma");
    //
    if (pdg == 3122) m_tauplusdecaymode.append(".lambda");
    if (pdg == -3122) m_tauplusdecaymode.append(".lmbbar");
    if (pdg == 10311) m_tauplusdecaymode.append(".kstar");
    if (pdg == -10311) m_tauplusdecaymode.append(".kstarbr");
    if (pdg == 331) m_tauplusdecaymode.append(".etapr");

    //test with resonances
    if (pdg == 221) m_tauplusdecaymode.append(".|eta|"); // intermediate resonances are implicit unless used as dictionary
    if (pdg == 223) m_tauplusdecaymode.append(".|omega|"); // intermediate resonances are implicit unless used as dictionary
    if (pdg == 323) m_tauplusdecaymode.append(".|kstarp|"); // intermediate resonances are implicit unless used as dictionary
    if (pdg == -323) m_tauplusdecaymode.append(".|kstarm|"); // intermediate resonances are implicit unless used as dictionary
    if (pdg == 9000211) m_tauplusdecaymode.append(".|a0p|");
    if (pdg == -9000211) m_tauplusdecaymode.append(".|a0m|");
    if (pdg == 10213) m_tauplusdecaymode.append(".|b1p|");
    if (pdg == -10213) m_tauplusdecaymode.append(".|b1m|");
    if (pdg == 333) m_tauplusdecaymode.append(".|phi|");
    if (pdg == 20223) m_tauplusdecaymode.append(".|f1|");
    if (pdg == 20213) m_tauplusdecaymode.append(".|a1p|");
    if (pdg == -20213) m_tauplusdecaymode.append(".|a1m|");
    if (pdg == 213) m_tauplusdecaymode.append(".|rhop|");
    if (pdg == -213) m_tauplusdecaymode.append(".|rhom|");
    if (pdg == 113) m_tauplusdecaymode.append(".|rho0|");
    if (pdg == 9010221) m_tauplusdecaymode.append(".|f0|");
    //
    //if (m_printmode>1) B2INFO("TauDecayMode:: PDG inside vec_dau_tauplus = " << pdg << "  m_tauplusdecaymode = " <<  m_tauplusdecaymode);
    //
    //if(m_printmode>3) B2INFO("TauDecayMode:: Decay mode is ="<<TauBBBmode(m_tauplusdecaymode));
  }
  m_tauplusdecaymode = m_tauplusdecaymode.erase(0, 1); // remove the leading dot
  if (m_printmode > 0) B2INFO("TauDecayMode:: EventNumber = " << EventNumber << " TauPlusDecayMode: tau+ -> " <<  m_tauplusdecaymode);
  //
  EventNumber = EventNumber + 1;
  //
}
//
void TauDecayModeModule::terminate()
{
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
  //
  StoreArray<MCParticle> MCParticles;
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
    return +1;
  } else {
    return getRecursiveMotherCharge(mother);
  }
}
//pdg(11)=e- pdg(22)=gamma pdg(111)=pi0 pdg(15)=tau-  || mother->getPDG() == 221

int TauDecayModeModule::getRecursiveMother(const MCParticle* p)
{
  const MCParticle* mother = p->getMother();
  if (mother->getPDG() == 15 || mother->getPDG() == -15) {
    return 1;
  }

  else if (mother->getPDG() == 221) {
    if (getRecursiveMother(mother) == 9000211 || getRecursiveMother(mother) == -9000211) {
      return 10;
    } else if (getRecursiveMother(mother) == 20223) {
      return 11;
    } else if (getRecursiveMother(mother) == 9000111) {
      return 12;
    } else {
      return 2;
    }
  } else if (mother->getPDG() == 223) {
    if (getRecursiveMother(mother) == 10213 || getRecursiveMother(mother) == -10213) {
      return 13;
    } else if (getRecursiveMother(mother) == 9000211 || getRecursiveMother(mother) == -9000211) {
      return 14;
    } else {
      return 3;
    }


  } else if (mother->getPDG() == 9000211 || mother->getPDG() == -9000211) {
    return 4;

  } else if (mother->getPDG() == 10213 || mother->getPDG() == 10213) {
    return 5;

  } else if (mother->getPDG() == 9010221) {
    return 6;

  } else if (mother->getPDG() == 213 || mother->getPDG() == -213) {
    if (getRecursiveMother(mother) == 10213 || getRecursiveMother(mother) == -10213) {
      return 15;
    } else {
      return 7;
    }
  } else if (mother->getPDG() == 333) {
    return 8;
  } else if (mother->getPDG() == 20223) {
    return 9;
  } else {
    return getRecursiveMother(mother);
  }
}
//a1=20213, b1=10213 , eta=221, omega=223, ro+=213, f1=20223 , a0+=9000211 , a0=9000111



int TauDecayModeModule::TauBBBmode(string state, map<string, int> tau_map)
{
  string dem = ".";
  std::vector<std::string> x = parseString(state, dem);
  int r = x.size();
  int i;
  map<string, int>::iterator itr = mode_decay.begin();
  for (itr =  mode_decay.begin(); itr !=  mode_decay.end(); ++itr) {
    string mode = itr-> first;
    std::vector<std::string> y = parseString(itr -> first, dem);
    int b = y.size();
    std::set<std::string> const uniques(x.begin(), x.end());
    x.assign(uniques.begin(), uniques.end());
    int count = 0;
    int nein = 0;
    for (i = 0 ; i != x.size(); ++i) {
      int pos = 0;
      int index;

      if ((index = mode.find(x[i], pos)) == string::npos) {
        nein = nein + 1;
      } else {
        while ((index = mode.find(x[i], pos)) != string::npos) {
          if (x[i] != "") {count = count + 1;}

          if ((index = mode.find(x[i], pos)) == string::npos) {
            break;
          }
          pos = index + 1;
        }
      }
    }
    if ((b == r) & (b - 1 == count) & (nein == 0)) {
      return itr -> second;
      break;
    }
    // else{
    //     continue;
    // }
  }
  return 0;

}