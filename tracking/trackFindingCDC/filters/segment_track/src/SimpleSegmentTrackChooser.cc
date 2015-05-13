/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_track/SimpleSegmentTrackChooser.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CellWeight SimpleSegmentTrackChooser::operator()(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>& testPair)
{
  const CDCRecoSegment2D* segment = testPair.first;
  const CDCTrack* track = testPair.second;

  const CDCRecoHit2D& front = segment->front();
  const CDCRecoHit2D& back = segment->back();

  // Calculate distance
  const CDCTrajectory2D& trajectory = track->getStartTrajectory3D().getTrajectory2D();

  if (segment->getStereoType() != AXIAL) {
    if (fabs(trajectory.getDist2D(front.getWireHit().getRefPos2D())) > 10
        or fabs(trajectory.getDist2D(back.getWireHit().getRefPos2D())) > 10)  {
      B2DEBUG(120, "Hits too far away: " << trajectory.getDist2D(front.getRecoPos2D()))
      return NOT_A_CELL;
    }
  } else {
    if (fabs(trajectory.getDist2D(front.getWireHit().getRefPos2D())) > 2
        or fabs(trajectory.getDist2D(back.getWireHit().getRefPos2D())) > 2)  {
      B2DEBUG(120, "Hits too far away: " << trajectory.getDist2D(front.getRecoPos2D()))
      return NOT_A_CELL;
    }
  }

  Vector3D frontRecoPos3D = front.reconstruct3D(trajectory);
  Vector3D backRecoPos3D = back.reconstruct3D(trajectory);

  if (segment->getStereoType() != AXIAL) {
    double forwardZ = front.getWire().getSkewLine().forwardZ();
    double backwardZ = front.getWire().getSkewLine().backwardZ();

    if (frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ) {
      B2DEBUG(120, "Segment out of CDC after reconstruction.")
      return NOT_A_CELL;
    }
  }


  unsigned int hitsInSameRegion = 0;
  for (const CDCRecoHit3D& recoHit : track->items()) {
    if (recoHit.getISuperLayer() == segment->getISuperLayer()) {
      hitsInSameRegion++;
    }
  }

  if (hitsInSameRegion > 5) {
    B2DEBUG(110, "Too many hits in the same region: " << hitsInSameRegion)
    return NOT_A_CELL;
  } else {
    B2DEBUG(110, "Hits in the region: " << hitsInSameRegion << " while hits in segment: " <<
            segment->size())
  }

  // Means: yes
  return 1.0;
}
