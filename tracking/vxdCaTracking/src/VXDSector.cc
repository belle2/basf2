/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/VXDSector.h"
#include "tracking/vxdCaTracking/Cutoff.h"
#include "tracking/vxdCaTracking/SectorFriends.h"
// #include "tracking/vxdCaTracking/VXDTFHit.h"
// #include "tracking/vxdCaTracking/VXDSegmentCell.h"
#include "tracking/dataobjects/FullSecID.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;



void VXDSector::addCutoff(int cutOffType, unsigned int friendName, pair<double, double> values)
{
  FriendMap::iterator mapIter = m_friendMap.find(friendName);
  if (mapIter == m_friendMap.end()) {
    addFriend(friendName);
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
    B2FATAL("VXDSector::getSupportedCudoffs - friend int/string" << aFriend << "/" << aFullSecID <<
            " not found!"); // includes a total break, therefore no return needed
  }
  vector<int> supportedCutoffs;
  mapIter->second.getSupportedCutoffs(supportedCutoffs);
  return supportedCutoffs;
}
