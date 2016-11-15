/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>

#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentVarSet.h>
#include <tracking/trackFindingCDC/filters/stereoSegments/StereoSegmentTruthVarSet.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class SimpleStereoSegmentFilter : public Filter<std::pair<const CDCSegment3D*, const CDCTrack*>> {
    public:
      Weight operator()(const std::pair<const CDCSegment3D*, const CDCTrack*>& pairToTest) override
      {
        const CDCSegment3D& segment3D = *pairToTest.first;
        const CDCTrack& track = *pairToTest.second;

        const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
        const CDCTrajectorySZ& trajectorySZ = track.getStartTrajectory3D().getTrajectorySZ();
        const bool isCurler = trajectory2D.isCurler();
        const double radius = trajectory2D.getLocalCircle()->absRadius();

        const ISuperLayer segmentSuperLayer = segment3D.getISuperLayer();

        const unsigned int numberOfHitsInSameSuperLayer = std::count_if(track.begin(),
        track.end(), [&segmentSuperLayer](const CDCRecoHit3D & recoHit) -> bool {
          return recoHit.getISuperLayer() == segmentSuperLayer;
        });

        if (numberOfHitsInSameSuperLayer > 0 and not isCurler) {
          return NAN;
        }

        // Count the number of hits with reconstruction position out of the CDC
        unsigned int numberOfHitsOutOfCDC = 0;
        unsigned int numberOfHitsOnWrongSide = 0;
        double sumDistanceToTrack = 0;

        std::vector<double> arcLength2DList(segment3D.size());

        for (const CDCRecoHit3D& recoHit3D : segment3D.items()) {
          const CDCWire& wire = recoHit3D.getWire();
          const Vector3D& reconstructedPosition = recoHit3D.getRecoPos3D();

          if (not wire.isInCellZBounds(reconstructedPosition)) {
            numberOfHitsOutOfCDC++;
          }

          const double arcLength2D = recoHit3D.getArcLength2D();
          if (arcLength2D < 0 and not isCurler) {
            numberOfHitsOnWrongSide++;
            arcLength2DList.push_back(arcLength2D + 2 * TMath::Pi() * radius);
            sumDistanceToTrack += trajectorySZ.getZDist(arcLength2D + 2 * TMath::Pi() * radius, reconstructedPosition.z());
          } else {
            arcLength2DList.push_back(arcLength2D);
            sumDistanceToTrack += trajectorySZ.getZDist(arcLength2D, reconstructedPosition.z());
          }
        }

        std::sort(arcLength2DList.begin(), arcLength2DList.end());

        const double minimumArcLength2D = arcLength2DList.front();
        const double maximumArcLength2D = arcLength2DList.back();

        size_t numberOfHitsInSameRegion = std::count_if(track.begin(), track.end(), [&](const CDCRecoHit3D & recoHit) -> bool {
          return recoHit.getArcLength2D() < maximumArcLength2D and recoHit.getArcLength2D() > minimumArcLength2D;
        });

        if (numberOfHitsInSameRegion > 1) {
          return NAN;
        }

        if (numberOfHitsOutOfCDC > 1) {
          return NAN;
        }

        if (numberOfHitsOnWrongSide > 1) {
          return NAN;
        }

        if (maximumArcLength2D > track.back().getArcLength2D()) {
          return NAN;
        }

        return 1.0;
      }
    };
  }
}
