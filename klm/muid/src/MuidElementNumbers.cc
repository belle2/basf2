/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/muid/MuidElementNumbers.h>

using namespace Belle2;

MuidElementNumbers::MuidElementNumbers()
{
}

MuidElementNumbers::~MuidElementNumbers()
{
}

bool MuidElementNumbers::checkExtrapolationOutcome(int outcome, int lastLayer)
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

int MuidElementNumbers::calculateExtrapolationOutcome(bool isForward, bool escaped, int lastBarrelLayer, int lastEndcapLayer)
{
  int outcome = MuidElementNumbers::c_NotReached;
  if ((lastBarrelLayer >= 0) || (lastEndcapLayer >= 0)) {
    /* Stop or exit in barrel. */
    if (lastEndcapLayer < 0)
      outcome = escaped ? MuidElementNumbers::c_ExitBarrel : MuidElementNumbers::c_StopInBarrel;
    /* Stop or exit in endcap. */
    else {
      if (escaped) {
        if (lastBarrelLayer < 0) /* Exit in endcap with no barrel hits. */
          outcome = isForward ? MuidElementNumbers::c_ExitForwardEndcap :
                    MuidElementNumbers::c_ExitBackwardEndcap;
        else /* Exit in endcap with barrel hits. */
          outcome = (isForward ? MuidElementNumbers::c_CrossBarrelExitForwardMin : MuidElementNumbers::c_CrossBarrelExitBackwardMin) +
                    lastBarrelLayer;
      } else {
        if (lastBarrelLayer < 0) /* Stop in endcap with no barrel hits. */
          outcome = isForward ? MuidElementNumbers::c_StopInForwardEndcap :
                    MuidElementNumbers::c_StopInBackwardEndcap;
        else /* Stop in endcap with barrel hits. */
          outcome = (isForward ? MuidElementNumbers::c_CrossBarrelStopInForwardMin : MuidElementNumbers::c_CrossBarrelStopInBackwardMin) +
                    lastBarrelLayer;
      }
    }
  }
  return outcome;
}
