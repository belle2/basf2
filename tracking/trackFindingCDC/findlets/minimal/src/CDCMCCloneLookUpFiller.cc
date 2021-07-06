/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
