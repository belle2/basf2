/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;
using namespace TrackFindingCDC;


const CDCMCHitLookUp& CDCMCHitLookUp::getInstance()
{
  return CDCMCManager::getMCHitLookUp();
}


void CDCMCHitLookUp::fill() const
{
  return CDCMCManager::getInstance().fill();
}


const Belle2::CDCSimHit* CDCMCHitLookUp::getSimHit(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCMap().getSimHit(ptrHit);
}


const Belle2::MCParticle* CDCMCHitLookUp::getMCParticle(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCMap().getMCParticle(ptrHit);
}


const Vector2D CDCMCHitLookUp::getRefPos2D(const CDCHit* ptrHit) const
{
  if (not ptrHit) return Vector2D();
  const CDCHit& hit = *ptrHit;

  const CDCWire* ptrWire = CDCWire::getInstance(hit);

  if (not ptrWire) {
    B2WARNING("Encountered CDCHit with wire ids that do not correspond to a valid wire in the CDC)");
    return Vector2D();
  }

  const CDCWire& wire = *ptrWire;
  Vector2D refPos2D =  wire.getRefPos2D();
  return refPos2D;
}


float CDCMCHitLookUp::getRefDriftLength(const CDCHit* ptrHit) const
{

  if (not ptrHit) return NAN;
  CDCWireHit wireHit(ptrHit);
  return wireHit.getRefDriftLength();

}


const Vector3D CDCMCHitLookUp::getRecoPos3D(const CDCHit* ptrHit) const
{
  return CDCMCManager::getSimHitLookUp().getRecoPos3D(ptrHit);
}


const Vector3D CDCMCHitLookUp::getClosestPrimaryRecoPos3D(const CDCHit* ptrHit) const
{
  return CDCMCManager::getSimHitLookUp().getClosestPrimaryRecoPos3D(ptrHit);
}


bool CDCMCHitLookUp::isReassignedSecondary(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCMap().isReassignedSecondary(ptrHit);
}



const CDCSimHit* CDCMCHitLookUp::getClosestPrimarySimHit(const CDCHit* ptrHit) const
{
  return CDCMCManager::getSimHitLookUp().getClosestPrimarySimHit(ptrHit);
}


ERightLeft CDCMCHitLookUp::getRLInfo(const CDCHit* ptrHit) const
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


Index CDCMCHitLookUp::getNLoops(const CDCHit* ptrHit) const
{
  return CDCMCManager::getMCTrackStore().getNLoops(ptrHit);
}
