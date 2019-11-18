/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/sectorMapTools/SectorFriendship.h"
#include "tracking/trackFindingVXD/sectorMapTools/ActivatedSector.h"

#include "tracking/trackFindingVXD/trackSegmentTools/FilterBase.h"

using namespace std;
using namespace Belle2;

void SectorFriendship::prepareCompatibilityTable()
{
  m_compatibilityTable.clear();

  uint nHitsOnMainSector = m_mainSector->getMyActiveSector()->size();
  uint nHitsOnFriendSector = m_friendSector->getMyActiveSector()->size();
  std::vector<CompatibilityValue> compatibility(nHitsOnFriendSector,
                                                0);  // TODO: concept for compatibilityTable needed. shall the filters check for a threshold value or something else?
  for (uint i = 0 ; i < nHitsOnMainSector; ++i) {
    m_compatibilityTable.push_back(compatibility);
  }
}


void SectorFriendship::applySegmentFilters()
{
  prepareCompatibilityTable();

  for (FilterBase* aFilter : m_myFilters) {
    aFilter->checkSpacePoints(this, m_compatibilityTable);
    if (checkCombinationsAlive() == 0) { break; }
  }
}

unsigned int SectorFriendship::checkCombinationsAlive() const
{
  unsigned int counter = 0;
  for (const auto& aVector : m_compatibilityTable) {
    for (const auto& aValue : aVector) {
      counter += aValue;
    }
  }
  return counter;
}

