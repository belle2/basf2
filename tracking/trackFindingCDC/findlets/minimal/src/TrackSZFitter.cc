/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackSZFitter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCAxialStereoFusion.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/geometry/UncertainHelix.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackSZFitter::getDescription()
{
  return "Use an SZFitter to create the 3D trajectory out of the 2D one.";
}

void TrackSZFitter::apply(std::vector<CDCTrack>& tracks)
{
  // Postprocess each track (=fit)
  for (CDCTrack& track : tracks) {
    const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

    track.shiftToPositiveArcLengths2D();
    track.sortByArcLength2D();

    CDCTrajectory2D originalTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();
    Vector2D localOrigin = originalTrajectory2D.getLocalOrigin();

    const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
    CDCTrajectory3D preliminaryTrajectory3D{originalTrajectory2D, szTrajectory};
    track.setStartTrajectory3D(preliminaryTrajectory3D);

    // Estimate a better covariance matrix
    CDCAxialStereoFusion axialStereoFusionFitter;
    CDCSegment2D axialSegment2D;
    CDCSegment2D stereoSegment2D;
    for (const CDCRecoHit3D& recoHit3D : track) {
      if (recoHit3D.isAxial()) {
        axialSegment2D.push_back(recoHit3D.getRecoHit2D());
      } else {
        stereoSegment2D.push_back(recoHit3D.getRecoHit2D());
      }
      if ((axialSegment2D.size() > 6) and (stereoSegment2D.size() > 6)) break;
    }

    if (not((axialSegment2D.size() > 6) and (stereoSegment2D.size() > 6))) continue;

    CDCTrajectory3D trajectory3D =
      axialStereoFusionFitter.reconstructFuseTrajectories(axialSegment2D, stereoSegment2D, preliminaryTrajectory3D);
    trajectory3D.setLocalOrigin({localOrigin, 0});

    // Copy only the covariance matrix, chi2 and ndf over for a conservative introduction for the moment.
    UncertainHelix preliminaryUncertainHelix = preliminaryTrajectory3D.getLocalHelix();
    UncertainHelix uncertainHelix = trajectory3D.getLocalHelix();

    preliminaryUncertainHelix.setHelixCovariance(uncertainHelix.helixCovariance());
    preliminaryUncertainHelix.setChi2(uncertainHelix.chi2());
    preliminaryUncertainHelix.setNDF(uncertainHelix.ndf());
    preliminaryTrajectory3D.setLocalHelix(preliminaryUncertainHelix);

    track.setStartTrajectory3D(preliminaryTrajectory3D);
  }
}
