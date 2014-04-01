/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/sectorMapTools/Sector.h"
#include "tracking/trackFindingVXD/sectorMapTools/SectorFriendship.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

ClassImp(Sector);

void Sector::segmentMaker()
{
  for (SectorFriendship * aFriend : m_myFriends) {
    aFriend->applySegmentFilters();
  }
}