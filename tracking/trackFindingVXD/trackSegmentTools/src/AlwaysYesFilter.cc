/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
