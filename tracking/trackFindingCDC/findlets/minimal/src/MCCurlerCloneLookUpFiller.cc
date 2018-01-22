/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/MCCurlerCloneLookUpFiller.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCCurlerCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/dataobjects/RecoTrack.h>



using namespace Belle2;
using namespace TrackFindingCDC;

std::string MCCurlerCloneLookUpFiller::getDescription()
{
  return "Fill CDCTracks into CDCMCCurlerCloneLookUp singleton, which stores lookup table if track is clone from curler loops";
}

/// Signal the beginning of a new event
void MCCurlerCloneLookUpFiller::beginEvent()
{
  CDCMCCurlerCloneLookUp::getInstance().clear();
}

void MCCurlerCloneLookUpFiller::apply(std::vector<CDCTrack>& cdcTracks)
{
  CDCMCCurlerCloneLookUp::getInstance().fill(cdcTracks);
}
