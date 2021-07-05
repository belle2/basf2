/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
