/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TauDecayMarker/TauDecayMarkerModule.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

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

TauDecayMarkerModule::TauDecayMarkerModule() : Module(), tauPair(false), numOfTauPlus(0), numOfTauMinus(0), idOfTauPlus(-1),
  idOfTauMinus(-1), m_pmode(-2), m_mmode(-2), m_pprong(0), m_mprong(0)
{
  // Set module properties
  setDescription("Module to identify generated tau pair decays, using MCParticle information. Each tau lepton decay channel "
                 "is numbered following the order in the default KKMC decay table. Using this module, "
                 "the channel number will be stored in the variables `tauPlusMCMode`, and `tauMinusMCMode`. "
                 "Further details and usage can be found at `TauDecayMCModes`. ");
  //Parameter definition
  addParam("printDecayInfo", m_printDecayInfo, "Print information of the tau pair decay from MC.", false);
}

void TauDecayMarkerModule::initialize()
{
  m_tauDecay.registerInDataStore();

}

void TauDecayMarkerModule::event()
{
  if (!m_tauDecay) m_tauDecay.create();

  IdentifyTauPair();
  if (tauPair) {
    m_pmode = getDecayChannelOfTau(+1) % 100;
    m_mmode = getDecayChannelOfTau(-1) % 100;

    m_pprong = getProngOfDecay(*m_MCParticles[idOfTauPlus - 1]);
    m_mprong = getProngOfDecay(*m_MCParticles[idOfTauMinus - 1]);

    if (m_printDecayInfo) {
      B2INFO("Decay ID: " << m_pmode << " (tau+), " << m_mmode << " (tau-)." <<
             " Topology: " << m_pprong << "-" << m_mprong << " prong");
    }

  } else {
    m_pmode = -1;
    m_mmode = -1;
  }

  m_tauDecay->addTauPlusIdMode(m_pmode);
  m_tauDecay->addTauMinusIdMode(m_mmode);

  m_tauDecay->addTauPlusMcProng(m_pprong);
  m_tauDecay->addTauMinusMcProng(m_mprong);

}

void TauDecayMarkerModule::IdentifyTauPair()
{
  numOfTauPlus = 0;
  numOfTauMinus = 0;
  idOfTauPlus = 0;
  idOfTauMinus = 0;
  for (int i = 0; i < m_MCParticles.getEntries(); i++) {
    MCParticle& p = *m_MCParticles[i];

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

int TauDecayMarkerModule::getNumDaughterOfTau(int s, int id, int sign)
{
  if (s == 0 || !tauPair) return -1;
  int tauid = idOfTauMinus;
  if (s > 0) tauid = idOfTauPlus;
  int ret = 0;
  const MCParticle& p = *m_MCParticles[tauid - 1];

  if (id == 0) {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *m_MCParticles[i - 1];
      if (abs(d.getPDG()) == 24)
        ret += d.getLastDaughter() - d.getFirstDaughter() + 1;
      else ret++;
    }
  } else {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *m_MCParticles[i - 1];
      int pdg = d.getPDG();
      if (pdg == id || (sign == 0 && abs(pdg) == abs(id))) ret++;
      if (abs(pdg) == 24) {
        for (int j = d.getFirstDaughter(); j <= d.getLastDaughter(); ++j) {
          MCParticle& e = *m_MCParticles[j - 1];
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
  if (s == 0 || !tauPair) return -1;
  int tauid = idOfTauMinus;
  if (s > 0) tauid = idOfTauPlus;
  int ret = 0;
  const MCParticle& p = *m_MCParticles[tauid - 1];

  if (id == 0) {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *m_MCParticles[i - 1];
      if (abs(d.getPDG()) == 24) {
        for (int j = d.getFirstDaughter(); j <= d.getLastDaughter(); ++j) {
          MCParticle& e = *m_MCParticles[j - 1];
          if (e.getPDG() != Const::photon.getPDGCode()) ret++;
        }
      } else if (d.getPDG() != Const::photon.getPDGCode()) ret++;
    }
  } else {
    for (int i = p.getFirstDaughter(); i <= p.getLastDaughter(); ++i) {
      MCParticle& d = *m_MCParticles[i - 1];
      int pdg = d.getPDG();
      if (abs(pdg) == 24) {
        for (int j = d.getFirstDaughter(); j <= d.getLastDaughter(); ++j) {
          MCParticle& e = *m_MCParticles[j - 1];
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
  if (tauPair && s != 0) {
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


int TauDecayMarkerModule::getProngOfDecay(const MCParticle& p)
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

