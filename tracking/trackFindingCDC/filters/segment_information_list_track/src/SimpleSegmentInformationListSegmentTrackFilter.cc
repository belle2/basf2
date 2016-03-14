/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segment_information_list_track/SimpleSegmentInformationListTrackFilter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CellWeight SimpleSegmentInformationListTrackFilter::operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>&
    testPair)
{
  const std::vector<SegmentInformation*>& train = testPair.first;
  const CDCTrack* track = testPair.second;

  if (train.size() == 0) return NOT_A_CELL;

  const CDCTrajectory2D& trajectory2D = track->getStartTrajectory3D().getTrajectory2D();

  CDCObservations2D observations;

  bool isAxialSegment = train.front()->getSegment()->getStereoType() == AXIAL;

  for (const CDCRecoHit3D& recoHit : *track) {
    if (isAxialSegment) {
      observations.append(recoHit.getWireHit().getRefPos2D());
    } else {
      double s = recoHit.getPerpS();
      double z = recoHit.getRecoZ();
      observations.append(s, z);
    }
  }

  for (SegmentInformation* segmentInformation : train) {
    for (const CDCRecoHit2D& recoHit : * (segmentInformation->getSegment())) {
      if (isAxialSegment) {
        observations.append(recoHit.getRecoPos2D());
      } else {
        CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
        CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
        double s = recoHit3D.getPerpS();
        double z = recoHit3D.getRecoZ();
        observations.append(s, z);
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
