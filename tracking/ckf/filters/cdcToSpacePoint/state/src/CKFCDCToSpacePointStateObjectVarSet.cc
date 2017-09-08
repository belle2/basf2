/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateObjectVarSet.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  template<class APredicate>
  double meanOver(const BaseCKFCDCToSpacePointStateObjectFilter::Object* state, const APredicate& t)
  {
    double sum = 0;
    unsigned int numberOfHits = 0;

    state->walk([&](const auto & walkState) {
      const SpacePoint* spacePoint = walkState->getHit();
      if (not spacePoint) {
        return;
      }
      for (auto& cluster : spacePoint->getRelationsTo<SVDCluster>()) {
        numberOfHits++;
        sum += t(cluster);
      }
    });

    return sum / numberOfHits;
  }

  template<class APredicate>
  double minOver(const BaseCKFCDCToSpacePointStateObjectFilter::Object* state, const APredicate& t)
  {
    double minimalValue = NAN;

    state->walk([&](const auto & walkState) {
      const SpacePoint* spacePoint = walkState->getHit();
      if (not spacePoint) {
        return;
      }
      for (auto& cluster : spacePoint->getRelationsTo<SVDCluster>()) {
        double currentValue = t(cluster);
        if (std::isnan(minimalValue)) {
          minimalValue = currentValue;
        } else {
          minimalValue = std::min(currentValue, minimalValue);
        }
      }
    });

    return minimalValue;
  }

  template<class APredicate>
  double stdOver(const BaseCKFCDCToSpacePointStateObjectFilter::Object* state, const APredicate& t)
  {
    double sum = 0;
    double sumSquared = 0;
    unsigned int numberOfHits = 0;

    state->walk([&](const auto & walkState) {
      const SpacePoint* spacePoint = walkState->getHit();
      if (not spacePoint) {
        return;
      }
      for (auto& cluster : spacePoint->getRelationsTo<SVDCluster>()) {
        numberOfHits++;
        const auto& currentValue =  t(cluster);
        sum += currentValue;
        sumSquared += currentValue;
      }
    });

    return std::sqrt((sumSquared - sum / numberOfHits) / numberOfHits - 1);
  }
}

bool CKFCDCToSpacePointStateObjectVarSet::extract(const BaseCKFCDCToSpacePointStateObjectFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getHit();

  // TODO: Do not dismiss spacePoint = 0 cases!
  if (not cdcTrack or not spacePoint) {
    return false;
  }

  if (not cdcTrack->wasFitSuccessful()) {
    return false;
  }

  const auto& firstMeasurement = result->getMeasuredStateOnPlane();
  Vector3D position = Vector3D(firstMeasurement.getPos());
  Vector3D momentum = Vector3D(firstMeasurement.getMom());

  const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

  const auto& hitPosition = Vector3D(spacePoint->getPosition());

  const double arcLength = trajectory.calcArcLength2D(hitPosition);
  const auto& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
  const auto& trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

  Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);

  const auto& sensorInfo = spacePoint->getVxdID();

  var<named("ladder")>() = sensorInfo.getLadderNumber();
  var<named("sensor")>() = sensorInfo.getSensorNumber();
  var<named("segment")>() = sensorInfo.getSegmentNumber();
  var<named("id")>() = sensorInfo.getID();

  var<named("last_layer")>() = 0;
  var<named("last_ladder")>() = 0;
  var<named("last_sensor")>() = 0;
  var<named("last_segment")>() = 0;
  var<named("last_id")>() = 0;

  const auto* parent = result->getParent();
  if (parent) {
    // skip the overlap layers
    parent = parent->getParent();
    if (parent) {
      const auto* parentSpacePoint = parent->getHit();
      if (parentSpacePoint) {
        const auto& parentSensorInfo = parentSpacePoint->getVxdID();

        var<named("last_layer")>() = parentSensorInfo.getLayerNumber();
        var<named("last_ladder")>() = parentSensorInfo.getLadderNumber();
        var<named("last_sensor")>() = parentSensorInfo.getSensorNumber();
        var<named("last_segment")>() = parentSensorInfo.getSegmentNumber();
        var<named("last_id")>() = parentSensorInfo.getID();
      }
    }
  }

  if (spacePoint->getType() == VXD::SensorInfoBase::SensorType::SVD) {
    const auto& clusters = spacePoint->getRelationsTo<SVDCluster>();

    B2ASSERT("Must be related to exactly 2 clusters", clusters.size() == 2);
    const SVDCluster* firstCluster = clusters[0];
    const SVDCluster* secondCluster = clusters[1];

    var<named("cluster_1_charge")>() = firstCluster->getCharge();
    var<named("cluster_2_charge")>() = secondCluster->getCharge();
    var<named("mean_rest_cluster_charge")>() = meanOver(result, [](const auto & s) {return s.getCharge();});
    var<named("min_rest_cluster_charge")>() = minOver(result, [](const auto & s) {return s.getCharge();});
    var<named("std_rest_cluster_charge")>() = stdOver(result, [](const auto & s) {return s.getCharge();});
    var<named("cluster_1_seed_charge")>() = firstCluster->getSeedCharge();
    var<named("cluster_2_seed_charge")>() = secondCluster->getSeedCharge();
    var<named("mean_rest_cluster_seed_charge")>() = meanOver(result, [](const auto & s) {return s.getSeedCharge();});;
    var<named("min_rest_cluster_seed_charge")>() = minOver(result, [](const auto & s) {return s.getSeedCharge();});;
    var<named("std_rest_cluster_seed_charge")>() = stdOver(result, [](const auto & s) {return s.getSeedCharge();});;
    var<named("cluster_1_size")>() = firstCluster->getSize();
    var<named("cluster_2_size")>() = secondCluster->getSize();
    var<named("mean_rest_cluster_size")>() = meanOver(result, [](const auto & s) {return s.getSize();});;
    var<named("min_rest_cluster_size")>() = meanOver(result, [](const auto & s) {return s.getSize();});;
    var<named("std_rest_cluster_size")>() = stdOver(result, [](const auto & s) {return s.getSize();});;
    var<named("cluster_1_snr")>() = firstCluster->getSNR();
    var<named("cluster_2_snr")>() = secondCluster->getSNR();
    var<named("mean_rest_cluster_snr")>() = meanOver(result, [](const auto & s) {return s.getSNR();});;
    var<named("min_rest_cluster_snr")>() = minOver(result, [](const auto & s) {return s.getSNR();});;
    var<named("std_rest_cluster_snr")>() = stdOver(result, [](const auto & s) {return s.getSNR();});;
    var<named("cluster_1_charge_over_size")>() = firstCluster->getCharge() / firstCluster->getSize();
    var<named("cluster_2_charge_over_size")>() = secondCluster->getCharge() / secondCluster->getSize();
    var<named("mean_rest_cluster_charge_over_size")>() = meanOver(result, [](const auto & s) {return s.getCharge() / s.getSize();});;
    var<named("min_rest_cluster_charge_over_size")>() = minOver(result, [](const auto & s) {return s.getCharge() / s.getSize();});;
    var<named("std_rest_cluster_charge_over_size")>() = stdOver(result, [](const auto & s) {return s.getCharge() / s.getSize();});;
  }

  std::vector<const SpacePoint*> spacePoints;
  result->walk([&](const auto * walkState) {
    const SpacePoint* sp = walkState->getHit();
    if (sp) {
      spacePoints.push_back(sp);
    }
  });

  if (spacePoints.size() >= 3) {
    var<named("quality_index_triplet")>() = m_qualityTriplet.estimateQuality(spacePoints);
    var<named("quality_index_circle")>() = m_qualityCircle.estimateQuality(spacePoints);
    var<named("quality_index_helix")>() = m_qualityHelix.estimateQuality(spacePoints);
  } else {
    var<named("quality_index_triplet")>() = 0;
    var<named("quality_index_circle")>() = 0;
    var<named("quality_index_helix")>() = 0;
  }

  return true;
}
