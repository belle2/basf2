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
#include "../include/CDCMCManager.h"

#include <tracking/cdcLocalTracking/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCMCHitLookUp)



CDCMCHitLookUp::CDCMCHitLookUp()
{
}



CDCMCHitLookUp::~CDCMCHitLookUp()
{
}



const CDCMCHitLookUp& CDCMCHitLookUp::getInstance()
{
  return CDCMCManager::getMCHitLookUp();
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



Index CDCMCHitLookUp::getInTrackId(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getInTrackId(ptrHit);
}



Index CDCMCHitLookUp::getInTrackSegmentId(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getInTrackSegmentId(ptrHit);
}



Index CDCMCHitLookUp::getNPassedSuperLayers(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getNPassedSuperLayers(ptrHit);
}
