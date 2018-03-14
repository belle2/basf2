/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/AllTrackRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllTrackRelationFilter::operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack)
{
  return fromTrack.getAutomatonCell().getCellWeight() + toTrack.getAutomatonCell().getCellWeight();
}
