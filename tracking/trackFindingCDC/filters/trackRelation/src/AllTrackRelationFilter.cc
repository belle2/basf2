/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/trackRelation/AllTrackRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllTrackRelationFilter::operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack)
{
  return fromTrack.getAutomatonCell().getCellWeight() + toTrack.getAutomatonCell().getCellWeight();
}
