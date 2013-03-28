/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/VXDSector.h"
#include "../include/Cutoff.h"
#include "../include/SectorFriends.h"
#include "../include/VXDTFHit.h"
#include "../include/VXDSegmentCell.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


void VXDSector::addHit(VXDTFHit* newSpacePoint) { m_hits.push_back(newSpacePoint); }

void VXDSector::addFriend(string newSector)
{
  m_friends.push_back(newSector);
  SectorFriends aFriend = SectorFriends(newSector, m_sectorID);
  m_friendMap.insert(make_pair(newSector, aFriend));
} // should be called only at the beginning of a new run

void VXDSector::addCutoff(string cutOffType, string friendName, pair<double, double> values)
{
  FriendMap::iterator mapIter = m_friendMap.find(friendName);
  if (mapIter == m_friendMap.end()) {
    m_friends.push_back(friendName);
    SectorFriends aFriend = SectorFriends(friendName, m_sectorID);
    m_friendMap.insert(make_pair(friendName, aFriend));
    mapIter = m_friendMap.find(friendName);
  }
  mapIter->second.addValuePair(cutOffType, values);
}

void VXDSector::addInnerSegmentCell(VXDSegmentCell* newSegment) { m_innerSegmentCells.push_back(newSegment); }

void VXDSector::addOuterSegmentCell(VXDSegmentCell* newSegment) { m_outerSegmentCells.push_back(newSegment); }

/** getter **/
const vector<string> VXDSector::getSupportedCutoffs(string aFriend)
{
  FriendMap::iterator mapIter;
  mapIter = m_friendMap.find(aFriend);
  if (mapIter == m_friendMap.end()) {
    B2FATAL("VXDSector::getSupportedCudoffs - friend " << aFriend << " not found!"); // includes a total break, therefore no return needed
  }
  vector<string> cutoffs = mapIter->second.getSupportedCutoffs();
  return cutoffs;
}

Cutoff* VXDSector::getCutoff(string cutOffType, string aFriend)
{
  FriendMap::iterator mapIter;
  mapIter = m_friendMap.find(aFriend);
  if (mapIter == m_friendMap.end()) {
    /*B2DEBUG(50,"friend " << aFriend << " not found!");*/
    return NULL;
  }
  Cutoff* aCutOffPtr = mapIter->second.getCutOff(cutOffType);
  return aCutOffPtr;
}

void VXDSector::resetSector() { m_hits.clear(); m_innerSegmentCells.clear(); m_outerSegmentCells.clear(); } // should be called at the end of each event.
