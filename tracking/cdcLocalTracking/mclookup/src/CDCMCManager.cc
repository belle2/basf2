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
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/mclookup/CDCMCMap.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

namespace {
  CDCMCManager* g_mcManager = nullptr;
}

CDCMCManager::CDCMCManager() : m_mcMap(), m_mcTrackStore(), m_simHitLookUp()
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

}

void CDCMCManager::fill()
{

  m_mcMap.fill();
  const CDCMCMap* ptrMCMap = &m_mcMap;
  m_mcTrackStore.fill(ptrMCMap);
  m_simHitLookUp.fill(ptrMCMap);

}



