/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/CosmicsTrackMerger.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CosmicsTrackMerger::CosmicsTrackMerger()
{
}

std::string CosmicsTrackMerger::getDescription()
{
  return "Merge cosmics tracks based on different filter criteria.";
}

void CosmicsTrackMerger::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
}

void CosmicsTrackMerger::apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks)
{

}
