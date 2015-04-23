/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/AllAxialStereoSegmentPairNeighborChooser.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


NeighborWeight AllAxialStereoSegmentPairNeighborChooser::operator()(const CDCAxialStereoSegmentPair&,
    const CDCAxialStereoSegmentPair& toPair)
{
  return  -toPair.getStartSegment()->size();
}
