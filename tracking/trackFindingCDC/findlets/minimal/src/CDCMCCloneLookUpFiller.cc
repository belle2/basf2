/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/CDCMCCloneLookUpFiller.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string CDCMCCloneLookUpFiller::getDescription()
{
  return "Fill CDCTracks into CDCMCCloneLookUp singleton, which stores lookup table if track is clone";
}

/// Signal the beginning of a new event
void CDCMCCloneLookUpFiller::beginEvent()
{
  CDCMCCloneLookUp::getInstance().clear();
}

void CDCMCCloneLookUpFiller::apply(std::vector<CDCTrack>& cdcTracks)
{
  CDCMCCloneLookUp::getInstance().fill(cdcTracks);
}
