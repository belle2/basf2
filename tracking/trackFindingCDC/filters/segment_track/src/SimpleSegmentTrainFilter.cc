/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrainFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackFinderOutputCombining;

CellWeight SimpleSegmentTrainFilter::operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>& testPair)
{
  CellWeight extracted = Super::operator()(testPair);

  const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

  // TODO

  // Means: yes
  return 1.0;
}
