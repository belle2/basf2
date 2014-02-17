/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCHitLookUp.h"

#include <tracking/cdcLocalTracking/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCMCHitLookUp)

namespace {
  CDCMCHitLookUp* g_mcHitLookUp = nullptr;
}



CDCMCHitLookUp::CDCMCHitLookUp()
{
}



CDCMCHitLookUp::~CDCMCHitLookUp()
{
}



CDCMCHitLookUp& CDCMCHitLookUp::getInstance()
{
  if (not g_mcHitLookUp) g_mcHitLookUp = new CDCMCHitLookUp;
  return *g_mcHitLookUp;
}



const Belle2::CDCSimHit* CDCMCHitLookUp::getSimHit(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCMap().getSimHit(ptrHit);
}



const Belle2::MCParticle* CDCMCHitLookUp::getMCParticle(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCMap().getMCParticle(ptrHit);
}



bool CDCMCHitLookUp::isReassignedSecondary(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCMap().isReassignedSecondary(ptrHit);
}



const CDCSimHit* CDCMCHitLookUp::getClosestPrimarySimHit(const CDCHit* ptrHit) const
{
  return CDCMCManager::getSimHitLookUp().getClosestPrimarySimHit(ptrHit);
}



RightLeftInfo CDCMCHitLookUp::getRLInfo(const CDCHit* ptrHit) const
{
  return CDCMCManager::getSimHitLookUp().getRLInfo(ptrHit);
}



bool CDCMCHitLookUp::isBackground(const CDCHit* ptrHit) const
{
  const CDCSimHit* ptrSimHit = getSimHit(ptrHit);
  return ptrSimHit ? ptrSimHit->getBackgroundTag() != CDCSimHit::bg_none : false;
}



ITrackType CDCMCHitLookUp::getMCTrackId(const CDCHit* ptrHit) const
{
  const MCParticle* ptrMCParticle = getMCParticle(ptrHit);
  return ptrMCParticle ? ptrMCParticle->getArrayIndex() : INVALID_ITRACK;
}



int CDCMCHitLookUp::getInTrackId(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getInTrackId(ptrHit);
}



int CDCMCHitLookUp::getInTrackSegmentId(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getInTrackSegmentId(ptrHit);
}



int CDCMCHitLookUp::getNPassedSuperLayers(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getNPassedSuperLayers(ptrHit);
}
