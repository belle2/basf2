/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/MCCloneLookUpFiller.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/dataobjects/RecoTrack.h>



using namespace Belle2;
using namespace TrackFindingCDC;

std::string MCCloneLookUpFiller::getDescription()
{
  return "Fill CDCTracks into CDCMCCloneLookUp singleton, which stores lookup table if track is clone from curler loops";
}

/// Signal the beginning of a new event
void MCCloneLookUpFiller::beginEvent()
{
  CDCMCCloneLookUp::getInstance().clear();
}

void MCCloneLookUpFiller::apply(std::vector<CDCTrack>& cdcTracks)
{
  CDCMCCloneLookUp::getInstance().fill(cdcTracks);
}
