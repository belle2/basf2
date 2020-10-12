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

#include "tracking/dataobjects/FullSecID.h"
#include <boost/format.hpp> // formatting output

using namespace std;
using namespace Belle2;

void Sector::segmentMaker()
{
  for (SectorFriendship* aFriend : m_myFriends) {
    aFriend->applySegmentFilters();
  }
}


std::string Sector::printSector()
{
  string isSectorActive = "no", useDistance = "no";

  if (m_myActiveSector != nullptr) isSectorActive = "yes";
  if (m_useDistance4sort == true) useDistance = "yes";

  uint nFriends = m_myFriends.size();
  FullSecID myID(m_sectorID);

  return (boost::format("Sector %1% got activated %2% and has %3% friends. Using distance for sorting: %4%, while having distance of %5%cm to origin.\n")
          % myID % isSectorActive % nFriends % useDistance % m_distance2Origin).str();
}
