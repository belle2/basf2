/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCManager.h"

#include <framework/datastore/RelationVector.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  CDCMCManager* g_mcManager = nullptr;
}

CDCMCManager::CDCMCManager() : m_eventMetaData(-999, -999, -999)
{
}

CDCMCManager::~CDCMCManager()
{
}



CDCMCManager& CDCMCManager::getInstance()
{
  if (not g_mcManager) g_mcManager = new CDCMCManager;
  return *g_mcManager;
}



void CDCMCManager::clear()
{
  m_mcMap.clear();
  m_mcTrackStore.clear();
  m_simHitLookUp.clear();

  //m_mcHitLookUp.clear();
  m_mcSegmentLookUp.clear();
}

void CDCMCManager::fill()
{

  StoreObjPtr<EventMetaData> storedEventMetaData;

  if (storedEventMetaData.isValid()) {
    if (m_eventMetaData == *storedEventMetaData) {
      //Instance has already been filled with the current event
      return;
    }
  }

  clear();

  m_mcMap.fill();
  const CDCMCMap* ptrMCMap = &m_mcMap;
  m_mcTrackStore.fill(ptrMCMap);
  m_simHitLookUp.fill(ptrMCMap);

  if (storedEventMetaData.isValid()) {
    //after filling store the event numbers
    m_eventMetaData = *storedEventMetaData;
  }
}



