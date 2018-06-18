/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Michel Villanueva                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/TauDecayMarker/TauDecayMarkerModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TauDecayMarker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TauDecayMarkerModule::TauDecayMarkerModule() : Module()
{
  // Set module properties
  setDescription("Module to identify and label generated tau decays channels, using MCParticle information..");
}

void TauDecayMarkerModule::initialize()
{
  StoreObjPtr<TauPairDecay> tauDecay;
  tauDecay.registerInDataStore();

}

void TauDecayMarkerModule::event()
{
  StoreObjPtr<TauPairDecay> tauDecay;
  if (!tauDecay) tauDecay.create();

  IdentifyTauPair();
  if (tau_pair) {
    m_pmode = getDecayChannelOfTau(+1) % 100;
    m_mmode = getDecayChannelOfTau(-1) % 100;
  } else {
    m_pmode = -1;
    m_mmode = -1;
  }

  tauDecay->addTauPlusIdMode(m_pmode);
  tauDecay->addTauMinusIdMode(m_mmode);

}

void TauDecayMarkerModule::IdentifyTauPair()
{
  StoreArray<MCParticle> MCParticles;
  no_of_tau_plus = 0;
  no_of_tau_minus = 0;
  id_of_tau_plus = 0;
  id_of_tau_minus = 0;
  TLorentzVector P4p, P4m;
  for (int i = 0; i < MCParticles.getEntries(); i++) {
    MCParticle& p = *MCParticles[i];

    if (p.getStatus() == 1 && p.getPDG() == 15) {
      no_of_tau_minus++;
      id_of_tau_minus = p.getIndex();
      P4m = p.get4Vector();
    }
    if (p.getStatus() == 1 && p.getPDG() == -15) {
      no_of_tau_plus++;
      id_of_tau_plus = p.getIndex();
      P4p = p.get4Vector();
    }

  }
  if (no_of_tau_plus == 1 && no_of_tau_minus == 1) {
    tau_pair = true;
  } else tau_pair = false;
}

int TauDecayMarkerModule::getNumDaughterOfTau(int s, int id, int sign)
{
  if (s == 0 || !tau_pair) return -1;
  int tauid = id_of_tau_minus;
  if (s > 0) tauid = id_of_tau_plus;
  int ret = 0;
  StoreArray<MCParticle> MCParticles;
  MCParticle& p = *MCParticles[tauid - 1];

  if (id == 0) {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *MCParticles[i - 1];
      if (abs(d.getPDG()) == 24)
        ret += d.getLastDaughter() - d.getFirstDaughter() + 1;
      else ret++;
    }
  } else {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *MCParticles[i - 1];
      int pdg = d.getPDG();
      if (pdg == id || (sign == 0 && abs(pdg) == abs(id))) ret++;
      if (abs(pdg) == 24) {
        for (int j = d.getFirstDaughter(); j <= d.getLastDaughter(); ++j) {
          MCParticle& e = *MCParticles[j - 1];
          int pdg2 = e.getPDG();
          if (pdg2 == id ||
              (sign == 0)) ret++;
        }
      }
    }
  }
  return ret;
}

int TauDecayMarkerModule::getNumDaughterOfTauExceptGamma(int s, int id, int sign)
{
  if (s == 0 || !tau_pair) return -1;
  int tauid = id_of_tau_minus;
  if (s > 0) tauid = id_of_tau_plus;
  int ret = 0;
  StoreArray<MCParticle> MCParticles;
  MCParticle& p = *MCParticles[tauid - 1];

  if (id == 0) {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *MCParticles[i - 1];
      if (abs(d.getPDG()) == 24) {
        for (int j = d.getFirstDaughter(); j <= d.getLastDaughter(); ++j) {
          MCParticle& e = *MCParticles[j - 1];
          if (e.getPDG() != 22) ret++;
        }
      } else if (d.getPDG() != 22) ret++;
    }
  } else {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *MCParticles[i - 1];
      int pdg = d.getPDG();
      if (abs(pdg) == 24) {
        for (int j = d.getFirstDaughter(); j <= d.getLastDaughter(); ++j) {
          MCParticle& e = *MCParticles[j - 1];
          int pdg2 = e.getPDG();
          if (pdg2 == id ||
              (sign == 0 && abs(pdg2) == abs(id))) ret++;
        }
      } else if (pdg == id || (sign == 0 && abs(pdg) == abs(id))) ret++;
    }
  }
  return ret;
}

int TauDecayMarkerModule::getDecayChannelOfTau(int s)
{
  int ret = 0;
  if (tau_pair && s != 0) {
    if (
      getNumDaughterOfTauExceptGamma(s, -s * (-12), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (11), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 1 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-14), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (13), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 2 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 2
    ) ret = 3 + (getNumDaughterOfTau(s, 0, 1) - 2) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-213), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 2
    ) ret = 4 + (getNumDaughterOfTau(s, 0, 1) - 2) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-20213), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 2
    ) ret = 5 + (getNumDaughterOfTau(s, 0, 1) - 2) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 2
    ) ret = 6 + (getNumDaughterOfTau(s, 0, 1) - 2) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-323), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 2
    ) ret = 7 + (getNumDaughterOfTau(s, 0, 1) - 2) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 8 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 3 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 9 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 6
    ) ret = 10 + (getNumDaughterOfTau(s, 0, 1) - 6) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 3 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 6
    ) ret = 11 + (getNumDaughterOfTau(s, 0, 1) - 6) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 3 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 7
    ) ret = 12 + (getNumDaughterOfTau(s, 0, 1) - 7) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 3 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 7
    ) ret = 13 + (getNumDaughterOfTau(s, 0, 1) - 7) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 14 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 15 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 2 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 2) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 15 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 16 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 16 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 17 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 18 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 19 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 19 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 221, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 20 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTau(s, 22, 1) >= 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 21 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 22 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 22 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 4 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 6
    ) ret = 23 + (getNumDaughterOfTau(s, 0, 1) - 6) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 223, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 4
    ) ret = 24 + (getNumDaughterOfTau(s, 0, 1) - 4) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 221, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 25 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 221, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 26 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 221, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 27 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-323), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 221, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 28 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 29 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 3 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 30 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 31 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 31 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 32 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 32 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 33 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 34 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 2 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 2) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 34 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 223, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 35 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 223, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 36 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-11), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-12), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (11), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 37 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 20223, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 38 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 223, 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 3
    ) ret = 39 + (getNumDaughterOfTau(s, 0, 1) - 3) * 1000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 40 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (211), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 40 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 41 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 111, 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 41 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;
    else if (
      getNumDaughterOfTauExceptGamma(s, -s * (-10313), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 42 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000;
    else if (
      (getNumDaughterOfTauExceptGamma(s, 130, 0)
       + getNumDaughterOfTauExceptGamma(s, 310, 0) == 1 ||
       getNumDaughterOfTauExceptGamma(s, 311, 0) == 1) &&
      getNumDaughterOfTauExceptGamma(s, -s * (-211), 1) == 2 &&
      getNumDaughterOfTauExceptGamma(s, -s * (16), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, -s * (321), 1) == 1 &&
      getNumDaughterOfTauExceptGamma(s, 0, 1) == 5
    ) ret = 42 + (getNumDaughterOfTau(s, 0, 1) - 5) * 1000
              + getNumDaughterOfTauExceptGamma(s, 310, 0) * 10000
              + getNumDaughterOfTauExceptGamma(s, 130, 0) * 100000;

  }
  return ret;
}
