/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/states/CKFState.h>
#include <tracking/ckf/filters/cdcToSpacePoint/state/CKFCDCToSpacePointStateVarSet.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Helper function to calculate the mean of a given function over all states in the list
  template<class APredicate>
  double meanOver(const BaseCKFCDCToSpacePointStateFilter::Object* state, const APredicate& t)
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

  /// Helper function to calculate the min of a given function over all states in the list
  template<class APredicate>
  double minOver(const BaseCKFCDCToSpacePointStateFilter::Object* state, const APredicate& t)
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

  /// Helper function to calculate the std of a given function over all states in the list
  template<class APredicate>
  double stdOver(const BaseCKFCDCToSpacePointStateFilter::Object* state, const APredicate& t)
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

bool CKFCDCToSpacePointStateVarSet::extract(const BaseCKFCDCToSpacePointStateFilter::Object* result)
{
  RecoTrack* cdcTrack = result->getSeedRecoTrack();
  const SpacePoint* spacePoint = result->getHit();

  const auto& cdcHits = cdcTrack->getSortedCDCHitList();
  const auto& svdHits = cdcTrack->getSortedSVDHitList();
  var<named("seed_cdc_hits")>() = cdcHits.size();
  var<named("seed_svd_hits")>() = svdHits.size();
  if (svdHits.empty()) {
    var<named("seed_lowest_svd_layer")>() = NAN;
  } else {
    var<named("seed_lowest_svd_layer")>() = svdHits.front()->getSensorID().getLayerNumber();
  }
  if (cdcHits.empty()) {
    var<named("seed_lowest_cdc_layer")>() = NAN;
  } else {
    var<named("seed_lowest_cdc_layer")>() = cdcHits.front()->getICLayer();
  }

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

  const auto calculateCharge = [](const auto & s) {
    return s.getCharge();
  };
  const auto calculateSeedCharge = [](const auto & s) {
    return s.getSeedCharge();
  };
  const auto calculateSize = [](const auto & s) {
    return s.getSize();
  };
  const auto calculateSNR = [](const auto & s) {
    return s.getSNR();
  };
  const auto calculateChargeSizeRatio = [](const auto & s) {
    return s.getCharge() / s.getSize();
  };

  if (spacePoint->getType() == VXD::SensorInfoBase::SensorType::SVD) {
    const auto& clusters = spacePoint->getRelationsTo<SVDCluster>();

    B2ASSERT("Must be related to exactly 2 clusters", clusters.size() == 2);
    const SVDCluster* firstCluster = clusters[0];
    const SVDCluster* secondCluster = clusters[1];

    var<named("cluster_1_charge")>() = calculateCharge(*firstCluster);
    var<named("cluster_2_charge")>() = calculateCharge(*secondCluster);
    var<named("mean_rest_cluster_charge")>() = meanOver(result, calculateCharge);
    var<named("min_rest_cluster_charge")>() = minOver(result, calculateCharge);
    var<named("std_rest_cluster_charge")>() = stdOver(result, calculateCharge);

    var<named("cluster_1_seed_charge")>() = calculateSeedCharge(*firstCluster);
    var<named("cluster_2_seed_charge")>() = calculateSeedCharge(*secondCluster);
    var<named("mean_rest_cluster_seed_charge")>() = meanOver(result, calculateSeedCharge);
    var<named("min_rest_cluster_seed_charge")>() = minOver(result, calculateSeedCharge);
    var<named("std_rest_cluster_seed_charge")>() = stdOver(result, calculateSeedCharge);

    var<named("cluster_1_size")>() = calculateSize(*firstCluster);
    var<named("cluster_2_size")>() = calculateSize(*secondCluster);
    var<named("mean_rest_cluster_size")>() = meanOver(result, calculateSize);
    var<named("min_rest_cluster_size")>() = meanOver(result, calculateSize);
    var<named("std_rest_cluster_size")>() = stdOver(result, calculateSize);

    var<named("cluster_1_snr")>() = calculateSNR(*firstCluster);
    var<named("cluster_2_snr")>() = calculateSNR(*secondCluster);
    var<named("mean_rest_cluster_snr")>() = meanOver(result, calculateSNR);
    var<named("min_rest_cluster_snr")>() = minOver(result, calculateSNR);
    var<named("std_rest_cluster_snr")>() = stdOver(result, calculateSNR);

    var<named("cluster_1_charge_over_size")>() = calculateChargeSizeRatio(*firstCluster);
    var<named("cluster_2_charge_over_size")>() = calculateChargeSizeRatio(*secondCluster);
    var<named("mean_rest_cluster_charge_over_size")>() = meanOver(result, calculateChargeSizeRatio);
    var<named("min_rest_cluster_charge_over_size")>() = minOver(result, calculateChargeSizeRatio);
    var<named("std_rest_cluster_charge_over_size")>() = stdOver(result, calculateChargeSizeRatio);
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
