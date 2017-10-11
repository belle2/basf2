/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SectorSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  bool isConnected(const VxdID& currentSensor, const VxdID& nextSensor)
  {
    const int sensorNumberDifference =
      static_cast<int>(currentSensor.getSensorNumber()) - static_cast<int>(nextSensor.getSensorNumber());
    if (abs(sensorNumberDifference) > 1) {
      return false;
    }
    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
    const VXD::SensorInfoBase& currentSensorInfo = geoCache.getSensorInfo(currentSensor);
    const VXD::SensorInfoBase& nextSensorInfo = geoCache.getSensorInfo(nextSensor);

    const Vector2D& currentCenter = Vector3D(
                                      currentSensorInfo.pointToGlobal(TVector3(-0.5 * currentSensorInfo.getWidth(),
                                          -0.5 * currentSensorInfo.getLength(),
                                          0))).xy();
    const Vector2D& nextCenter = Vector3D(nextSensorInfo.pointToGlobal(TVector3(-0.5 * nextSensorInfo.getWidth(),
                                          -0.5 * nextSensorInfo.getLength(),
                                          0))).xy();

    const double& angle = std::acos(currentCenter.dot(nextCenter) / (currentCenter.norm() * nextCenter.norm()));
    return TMath::Pi() - angle > 2;
  }
}

TrackFindingCDC::Weight
SectorSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation)
{
  const CKFToSVDState& fromState = *(relation.first);
  const CKFToSVDState& toState = *(relation.second);

  const SpacePoint* fromSpacePoint = fromState.getHit();
  const SpacePoint* toSpacePoint = toState.getHit();

  B2ASSERT("You have filled the wrong states into this!", toSpacePoint);

  if (not fromSpacePoint) {
    // We are coming from a CDC track, so we can use its position to only look for matching ladders
    // TODO: implement a better way, e.g. using
    // const RecoTrack* seed = fromState.getSeed();
    // const auto& cdcPosition = fromState.getMSoPPosition();
    return 1.0;
  }

  const VxdID& fromVXDID = fromSpacePoint->getVxdID();
  const VxdID& toVXDID = toSpacePoint->getVxdID();

  if (fromVXDID.getLayerNumber() == toVXDID.getLayerNumber()) {
    // next layer is an overlap one, so lets return all hits from the same layer, that are on a
    // ladder which is below the last added hit.
    const unsigned int fromLadderNumber = fromVXDID.getLadderNumber();
    const unsigned int maximumLadderNumber = VXD::GeoCache::getInstance().getLadders(fromVXDID).size();

    // the reason for this strange formula is the numbering scheme in the VXD.
    // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
    // then we add (PXD)/subtract(SVD) one to get to the next (overlapping) ladder and do a % N to also cope for the
    // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
    // The + maximumLadderNumber in between makes sure, we are not ending with negative numbers
    const int direction = -1;
    const unsigned int overlappingLadder =
      ((fromLadderNumber + maximumLadderNumber - 1) + direction) % maximumLadderNumber + 1;

    B2DEBUG(100, "Overlap check on " << fromLadderNumber << " using from " << overlappingLadder);
    if (toVXDID.getLadderNumber() == overlappingLadder) {
      // TODO: Also check for sensor?
      return 1.0;
    } else {
      return NAN;
    }
  }

  // next layer is not an overlap one, so we can just return all hits of the next layer
  // that are in our sensor mapping.
  // TODO: test of the lookup is too slow
  if (isConnected(fromVXDID, toVXDID)) {
    return 1.0;
  } else {
    return NAN;
  }
}