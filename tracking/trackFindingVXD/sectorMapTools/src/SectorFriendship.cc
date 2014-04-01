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

#include "tracking/trackFindingVXD/trackSegmentTools/FilterBase.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


void SectorFriendship::applySegmentFilters()
{
  for (FilterBase * aFilter : m_myFilters) {
    aFilter->checkSpacePoints(this, m_compatibilityTable);
    if (checkCombinationsAlive() == 0) { break; }
  }
}

unsigned int SectorFriendship::checkCombinationsAlive()
{
  unsigned int counter = 0;
  for (auto & aVector : m_compatibilityTable) {
    for (auto & aValue : aVector) {
      counter += aValue;
    }
  }
  return counter;
}