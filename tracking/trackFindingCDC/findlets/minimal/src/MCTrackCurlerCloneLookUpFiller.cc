/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/MCTrackCurlerCloneLookUpFiller.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackCurlerCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/dataobjects/RecoTrack.h>



using namespace Belle2;
using namespace TrackFindingCDC;

std::string MCTrackCurlerCloneLookUpFiller::getDescription()
{
  return "Fill CDCTracks into CDCMCTrackCurlerCloneLookUp singleton, which stores lookup table if track is clone from curler loops";
}

void MCTrackCurlerCloneLookUpFiller::initialize()
{
  Super::initialize();
}

void MCTrackCurlerCloneLookUpFiller::apply(std::vector<CDCTrack>& cdcTracks)
{
  CDCMCTrackCurlerCloneLookUp::getInstance().fill(cdcTracks);
}
