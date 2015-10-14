/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackSegmentTools/AlwaysYesFilter.h"

#include "tracking/trackFindingVXD/sectorMapTools/SectorFriendship.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

void AlwaysYesFilter::checkSpacePoints(const SectorFriendship* thisFriendship, CompatibilityTable& compatibilityTable)
{
  for (auto& aVector : compatibilityTable) {
    for (auto& aValue : aVector) {
      B2DEBUG(1000, "there are currently " << thisFriendship->checkCombinationsAlive() << " alive");
      aValue++;
    }
  }
}
