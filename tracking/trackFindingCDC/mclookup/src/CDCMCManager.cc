/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/datastore/RelationVector.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCMCManager& CDCMCManager::getInstance()
{
  static CDCMCManager mcManager;
  return mcManager;
}


void CDCMCManager::requireTruthInformation()
{
  StoreArray <CDCSimHit>::required();
  StoreArray <MCParticle>::required();
}

void CDCMCManager::clear()
{
  m_mcTrackLookUp.clear();
  m_mcSegmentLookUp.clear();
  //m_mcHitLookUp.clear(); // Currently has no clear

  m_mcTrackStore.clear();
  m_simHitLookUp.clear();
  m_mcMap.clear();
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

  m_mcMap.fill();
  const CDCMCMap* ptrMCMap = &m_mcMap;
  m_simHitLookUp.fill(ptrMCMap);

  const CDCSimHitLookUp* ptrSimHitLookUp = &m_simHitLookUp;
  m_mcTrackStore.fill(ptrMCMap, ptrSimHitLookUp);

  if (storedEventMetaData.isValid()) {
    //after filling store the event numbers
    m_eventMetaData = *storedEventMetaData;
  }
}
