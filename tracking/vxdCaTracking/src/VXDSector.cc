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
#include "../include/FullSecID.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Tracking;

// VXDSector::VXDSector(unsigned int secID) {
//  FullSecID aSecInt = FullSecID(secID);
//  m_sectorID = aSecInt;
// }

void VXDSector::addHit(VXDTFHit* newSpacePoint) { m_hits.push_back(newSpacePoint); }

void VXDSector::addFriend(int newSector)
{
  m_friends.push_back(newSector);
//   SectorFriends aFriend = SectorFriends(newSector, m_sectorID);
  m_friendMap.insert(make_pair(newSector, SectorFriends(newSector, m_sectorID)));
} // should be called only at the beginning of a new run

void VXDSector::addCutoff(int cutOffType, unsigned int friendName, pair<double, double> values)
{
  FriendMap::iterator mapIter = m_friendMap.find(friendName);
  if (mapIter == m_friendMap.end()) {
    m_friends.push_back(friendName);
//     SectorFriends aFriend = SectorFriends(friendName, m_sectorID);
    m_friendMap.insert(make_pair(friendName, SectorFriends(friendName, m_sectorID)));
    mapIter = m_friendMap.find(friendName);
  }
  mapIter->second.addValuePair(cutOffType, values);
}


/** getter **/
const vector<int> VXDSector::getSupportedCutoffs(unsigned int aFriend)
{
  FriendMap::iterator mapIter = m_friendMap.find(aFriend);
  if (mapIter == m_friendMap.end()) {
    FullSecID aFullSecID = FullSecID(aFriend);
    B2FATAL("VXDSector::getSupportedCudoffs - friend int/string" << aFriend << "/" << aFullSecID.getFullSecString() << " not found!"); // includes a total break, therefore no return needed
  }
  vector<int> supportedCutoffs;
  mapIter->second.getSupportedCutoffs(supportedCutoffs);
  return supportedCutoffs;
}

Cutoff* VXDSector::getCutoff(int cutOffType, unsigned int aFriend)
{
  FriendMap::iterator mapIter = m_friendMap.find(aFriend);
  if (mapIter == m_friendMap.end()) { // not found
    return NULL;
  }
  return mapIter->second.getCutOff(cutOffType);
}
