/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/relations/SectorPXDPairFilter.h>
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
    const int layerNumberDifference =
      static_cast<int>(currentSensor.getLayerNumber()) - static_cast<int>(nextSensor.getLayerNumber());

    if ((abs(sensorNumberDifference) > 1 and layerNumberDifference == 1) or (abs(sensorNumberDifference) > 2)) {
      return false;
    }
    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
    const VXD::SensorInfoBase& currentSensorInfo = geoCache.getSensorInfo(currentSensor);
    const VXD::SensorInfoBase& nextSensorInfo = geoCache.getSensorInfo(nextSensor);

    static const TVector3 origin;

    const Vector2D currentCenter = Vector3D(currentSensorInfo.pointToGlobal(origin)).xy();
    const Vector2D nextCenter = Vector3D(nextSensorInfo.pointToGlobal(origin)).xy();

    const double angle = currentCenter.angleWith(nextCenter);
    return TMath::Pi() - angle > 2;
  }
}

TrackFindingCDC::Weight
SectorPXDPairFilter::operator()(const std::pair<const CKFToPXDState*, const CKFToPXDState*>& relation)
{
  const CKFToPXDState& fromState = *(relation.first);
  const CKFToPXDState& toState = *(relation.second);

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
    // TODO: Also check for sensors?
    return 1.0;
  }

  // next layer is not an overlap one, so we can just return all hits of the next layer
  // that are in our sensor mapping.
  // TODO: test of the lookup is too slow
  if (isConnected(fromVXDID, toVXDID)) {
    return 1.0;
  }

  return NAN;
}