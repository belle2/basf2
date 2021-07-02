/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/muid/MuidElementNumbers.h>

/* Belle2 headers. */
#include <framework/gearbox/Const.h>

using namespace Belle2;

MuidElementNumbers::MuidElementNumbers()
{
}

MuidElementNumbers::~MuidElementNumbers()
{
}

bool MuidElementNumbers::checkExtrapolationOutcome(unsigned int outcome, int lastLayer)
{
  /* KLM volume not reached during the extrapolation. */
  if (outcome == MuidElementNumbers::c_NotReached)
    return false;
  /* Barrel stop: never in layer 14. */
  if ((outcome == MuidElementNumbers::c_StopInBarrel)
      && (lastLayer > MuidElementNumbers::getMaximalBarrelLayer() - 1))
    return false;
  /* Forward endcap stop: never in layer 13. */
  if ((outcome == MuidElementNumbers::c_StopInForwardEndcap)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1))
    return false;
  /* Barrel exit: no layers greater than 15. */
  if ((outcome == MuidElementNumbers::c_ExitBarrel)
      && (lastLayer > MuidElementNumbers::getMaximalBarrelLayer()))
    return false;
  /* Forward endcap exit: no layers greater than 14. */
  if ((outcome == MuidElementNumbers::c_ExitForwardEndcap)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer()))
    return false;
  /* Backward endcap stop: never in layer 11. */
  if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1))
    return false;
  /* Backward endcap exit: no layers greater than 12. */
  if ((outcome == MuidElementNumbers::c_ExitBackwardEndcap)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer()))
    return false;
  /* Like outcome == c_StopInForwardEndcap. */
  if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInForwardMin)
      && (outcome <= MuidElementNumbers::c_CrossBarrelStopInForwardMax)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1))
    return false;
  /* Like outcome == c_StopInBackwardEndcap. */
  if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInBackwardMin)
      && (outcome <= MuidElementNumbers::c_CrossBarrelStopInBackwardMax)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1))
    return false;
  /* Like outcome == c_ExitForwardEndcap. */
  if ((outcome >= MuidElementNumbers::c_CrossBarrelExitForwardMin)
      && (outcome <= MuidElementNumbers::c_CrossBarrelExitForwardMax)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer()))
    return false;
  /* Like outcome == c_ExitBackwardEndcap. */
  if ((outcome >= MuidElementNumbers::c_CrossBarrelExitBackwardMin)
      && (outcome <= MuidElementNumbers::c_CrossBarrelExitBackwardMax)
      && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer()))
    return false;
  return true;
}

unsigned int MuidElementNumbers::calculateExtrapolationOutcome(bool isForward, bool escaped, int lastBarrelLayer,
    int lastEndcapLayer)
{
  unsigned int outcome = MuidElementNumbers::c_NotReached;
  if ((lastBarrelLayer >= 0) || (lastEndcapLayer >= 0)) {
    /* Stop or exit in barrel. */
    if (lastEndcapLayer < 0) {
      if (escaped)
        outcome = MuidElementNumbers::c_ExitBarrel;
      else
        outcome = MuidElementNumbers::c_StopInBarrel;
    }
    /* Stop or exit in endcap. */
    else {
      if (escaped) {
        if (lastBarrelLayer < 0) { /* Exit in endcap with no barrel hits. */
          if (isForward)
            outcome = MuidElementNumbers::c_ExitForwardEndcap;
          else
            outcome = MuidElementNumbers::c_ExitBackwardEndcap;
        } else { /* Exit in endcap with barrel hits. */
          if (isForward)
            outcome = MuidElementNumbers::c_CrossBarrelExitForwardMin + lastBarrelLayer;
          else
            outcome = MuidElementNumbers::c_CrossBarrelExitBackwardMin + lastBarrelLayer;
        }
      } else {
        if (lastBarrelLayer < 0) { /* Stop in endcap with no barrel hits. */
          if (isForward)
            outcome = MuidElementNumbers::c_StopInForwardEndcap;
          else
            outcome = MuidElementNumbers::c_StopInBackwardEndcap;
        } else { /* Stop in endcap with barrel hits. */
          if (isForward)
            outcome = MuidElementNumbers::c_CrossBarrelStopInForwardMin + lastBarrelLayer;
          else
            outcome = MuidElementNumbers::c_CrossBarrelStopInBackwardMin + lastBarrelLayer;
        }
      }
    }
  }
  return outcome;
}

MuidElementNumbers::Hypothesis MuidElementNumbers::calculateHypothesisFromPDG(int pdg)
{
  /* For leptons, the sign of the PDG code is opposite to the charge. */
  if (pdg == Const::electron.getPDGCode())
    return MuidElementNumbers::c_Electron;
  if (pdg == -Const::electron.getPDGCode())
    return MuidElementNumbers::c_Positron;
  if (pdg == Const::muon.getPDGCode())
    return MuidElementNumbers::c_MuonMinus;
  if (pdg == -Const::muon.getPDGCode())
    return MuidElementNumbers::c_MuonPlus;
  /* For hadrons, the sign of the PDG code is equal to the charge. */
  if (pdg == Const::deuteron.getPDGCode())
    return MuidElementNumbers::c_Deuteron;
  if (pdg == -Const::deuteron.getPDGCode())
    return MuidElementNumbers::c_AntiDeuteron;
  if (pdg == Const::proton.getPDGCode())
    return MuidElementNumbers::c_Proton;
  if (pdg == -Const::proton.getPDGCode())
    return MuidElementNumbers::c_AntiProton;
  if (pdg == Const::pion.getPDGCode())
    return MuidElementNumbers::c_PionPlus;
  if (pdg == -Const::pion.getPDGCode())
    return MuidElementNumbers::c_PionMinus;
  if (pdg == Const::kaon.getPDGCode())
    return MuidElementNumbers::c_KaonPlus;
  if (pdg == -Const::kaon.getPDGCode())
    return MuidElementNumbers::c_KaonMinus;
  /* Only charged final state particles are supported. */
  return MuidElementNumbers::c_NotValid;
}

std::vector<int> MuidElementNumbers::getPDGVector(int charge)
{
  std::vector<int> pdgVector;
  for (const Const::ChargedStable particle : Const::chargedStableSet) {
    if ((particle == Const::electron) || (particle == Const::muon))
      pdgVector.push_back(-charge * particle.getPDGCode());
    else
      pdgVector.push_back(charge * particle.getPDGCode());
  }
  return pdgVector;
}

std::vector<int> MuidElementNumbers::getPDGVector()
{
  std::vector<int> pdgVector = getPDGVector(1);
  std::vector<int> temp = getPDGVector(-1);
  pdgVector.insert(pdgVector.end(), temp.begin(), temp.end());
  std::sort(pdgVector.begin(), pdgVector.end());
  return pdgVector;
}

int MuidElementNumbers::getLongitudinalID(int hypothesis, int outcome, int lastLayer)
{
  int id = lastLayer;
  id += (outcome << MuidElementNumbers::c_LastLayerBit);
  id += (hypothesis << (MuidElementNumbers::c_LastLayerBit + MuidElementNumbers::c_OutcomeBit));
  return id;
}

int MuidElementNumbers::getTransverseID(int hypothesis, int detector, int degreesOfFreedom)
{
  int id = degreesOfFreedom;
  id += (detector << MuidElementNumbers::c_DegreesOfFreedomBit);
  id += (hypothesis << (MuidElementNumbers::c_DegreesOfFreedomBit + MuidElementNumbers::c_DetectorBit));
  return id;
}
