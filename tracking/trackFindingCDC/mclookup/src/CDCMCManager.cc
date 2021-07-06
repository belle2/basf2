/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment2DLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCSegment3DLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  CDCMCMap& getMCMapInstance()
  {
    static CDCMCMap mcMap;
    return mcMap;
  }

  CDCMCTrackStore& getMCTrackStoreInstance()
  {
    static CDCMCTrackStore mcTrackStore;
    return mcTrackStore;
  }

  CDCSimHitLookUp& getSimHitLookUpInstance()
  {
    static CDCSimHitLookUp simHitLookUp;
    return simHitLookUp;
  }

  CDCMCHitLookUp& getMCHitLookUpInstance()
  {
    static CDCMCHitLookUp mcHitLookUp;
    return mcHitLookUp;
  }

  CDCMCSegment2DLookUp& getMCSegment2DLookUpInstance()
  {
    static CDCMCSegment2DLookUp mcSegment2DLookUp;
    return mcSegment2DLookUp;
  }

  CDCMCSegment3DLookUp& getMCSegment3DLookUpInstance()
  {
    static CDCMCSegment3DLookUp mcSegment3DLookUp;
    return mcSegment3DLookUp;
  }

  CDCMCTrackLookUp& getMCTrackLookUpInstance()
  {
    static CDCMCTrackLookUp mcTrackLookUp;
    return mcTrackLookUp;
  }
}

CDCMCManager& CDCMCManager::getInstance()
{
  static CDCMCManager mcManager;
  return mcManager;
}

const CDCMCMap& CDCMCManager::getMCMap()
{
  return ::getMCMapInstance();
}

const CDCMCTrackStore& CDCMCManager::getMCTrackStore()
{
  return ::getMCTrackStoreInstance();
}

const CDCSimHitLookUp& CDCMCManager::getSimHitLookUp()
{
  return ::getSimHitLookUpInstance();
}

const CDCMCHitLookUp& CDCMCManager::getMCHitLookUp()
{
  return ::getMCHitLookUpInstance();
}

const CDCMCSegment2DLookUp& CDCMCManager::getMCSegment2DLookUp()
{
  return ::getMCSegment2DLookUpInstance();
}

const CDCMCSegment3DLookUp& CDCMCManager::getMCSegment3DLookUp()
{
  return ::getMCSegment3DLookUpInstance();
}

const CDCMCTrackLookUp& CDCMCManager::getMCTrackLookUp()
{
  return ::getMCTrackLookUpInstance();
}

void CDCMCManager::requireTruthInformation()
{
  StoreArray <CDCSimHit> cdcSimHits;
  cdcSimHits.isRequired();
  StoreArray <MCParticle> mcParticles;
  mcParticles.isRequired();
}

void CDCMCManager::clear()
{
  ::getMCTrackLookUpInstance().clear();
  ::getMCSegment3DLookUpInstance().clear();
  ::getMCSegment2DLookUpInstance().clear();
  //::getMCHitLookUpInstance().clear(); // Currently has no clear

  ::getMCTrackStoreInstance().clear();
  ::getSimHitLookUpInstance().clear();
  ::getMCMapInstance().clear();
}

void CDCMCManager::fill()
{
  StoreObjPtr<EventMetaData> storedEventMetaData;

  if (storedEventMetaData.isValid()) {
    if (m_eventMetaData == *storedEventMetaData) {
      // Instance has already been filled with the current event
      return;
    }
  }

  clear();

  ::getMCMapInstance().fill();
  const CDCMCMap* ptrMCMap = &getMCMap();
  ::getSimHitLookUpInstance().fill(ptrMCMap);

  const CDCSimHitLookUp* ptrSimHitLookUp = &getSimHitLookUp();
  ::getMCTrackStoreInstance().fill(ptrMCMap, ptrSimHitLookUp);

  if (storedEventMetaData.isValid()) {
    //after filling store the event numbers
    m_eventMetaData = *storedEventMetaData;
  }
}
