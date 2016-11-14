/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentInformationListTrack/SimpleSegmentInformationListTrackFilter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight SimpleSegmentInformationListTrackFilter::operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>&
                                                           testPair)
{
  const std::vector<SegmentInformation*>& train = testPair.first;
  const CDCTrack* track = testPair.second;

  if (train.size() == 0) return NAN;

  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

  CDCObservations2D observations;

  bool isAxialSegment = train.front()->getSegment()->getStereoKind() == EStereoKind::c_Axial;

  for (const CDCRecoHit3D& recoHit : *track) {
    if (isAxialSegment) {
      observations.fill(recoHit.getWireHit().getRefPos2D());
    } else {
      double s = recoHit.getArcLength2D();
      double z = recoHit.getRecoZ();
      observations.fill(s, z);
    }
  }

  for (SegmentInformation* segmentInformation : train) {
    for (const CDCRecoHit2D& recoHit : * (segmentInformation->getSegment())) {
      if (isAxialSegment) {
        observations.fill(recoHit.getRecoPos2D());
      } else {
        const CDCRLWireHit& rlWireHit = recoHit.getRLWireHit();
        CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
        double s = recoHit3D.getArcLength2D();
        double z = recoHit3D.getRecoZ();
        observations.fill(s, z);
      }
    }
  }

  if (isAxialSegment) {
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    CDCTrajectory2D fittedTrajectory = fitter.fit(observations);
    return TMath::Prob(fittedTrajectory.getChi2(), fittedTrajectory.getNDF());
  } else {
    const CDCSZFitter& fitter = CDCSZFitter::getFitter();
    CDCTrajectorySZ fittedTrajectory = fitter.fit(observations);
    return TMath::Prob(fittedTrajectory.getChi2(), fittedTrajectory.getNDF());
  }
}
