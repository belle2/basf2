/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/vtx/filters/states/VTXStateVarSet.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Helper function to calculate the mean of a given function over all states in the list
  template<class APredicate>
  double meanOver(const std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>>& states, const APredicate& t)
  {
    double sum = 0;
    unsigned int numberOfHits = 0;

    for (const CKFToVTXState* walkState : states) {
      const SpacePoint* spacePoint = walkState->getHit();
      if (not spacePoint) {
        continue;
      }
      for (auto& cluster : spacePoint->getRelationsTo<VTXCluster>()) {
        numberOfHits++;
        sum += t(cluster);
      }
    }

    return sum / numberOfHits;
  }

  /// Helper function to calculate the min of a given function over all states in the list
  template<class APredicate>
  double minOver(const std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>>& states, const APredicate& t)
  {
    double minimalValue = NAN;

    for (const CKFToVTXState* walkState : states) {
      const SpacePoint* spacePoint = walkState->getHit();
      if (not spacePoint) {
        continue;
      }
      for (auto& cluster : spacePoint->getRelationsTo<VTXCluster>()) {
        double currentValue = t(cluster);
        if (std::isnan(minimalValue)) {
          minimalValue = currentValue;
        } else {
          minimalValue = std::min(currentValue, minimalValue);
        }
      }
    }

    return minimalValue;
  }

  /// Helper function to calculate the standard deviation of a given function over all states in the list
  template<class APredicate>
  double stdOver(const std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>>& states, const APredicate& t)
  {
    double sum = 0;
    double sumSquared = 0;
    unsigned int numberOfHits = 0;

    for (const CKFToVTXState* walkState : states) {
      const SpacePoint* spacePoint = walkState->getHit();
      if (not spacePoint) {
        continue;
      }
      for (auto& cluster : spacePoint->getRelationsTo<VTXCluster>()) {
        numberOfHits++;
        const auto& currentValue =  t(cluster);
        sum += currentValue;
        sumSquared += currentValue * currentValue;
      }
    }

    return std::sqrt((sumSquared - sum * sum / numberOfHits) / (numberOfHits - 1));
  }
}

bool VTXStateVarSet::extract(const BaseVTXStateFilter::Object* pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>>& previousStates = pair->first;
  CKFToVTXState* state = pair->second;

  const RecoTrack* cdcTrack = previousStates.front()->getSeed();
  B2ASSERT("Path without seed?", cdcTrack);

  const SpacePoint* spacePoint = state->getHit();
  B2ASSERT("Path without hit?", spacePoint);

  std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>> allStates = previousStates;
  allStates.emplace_back(state, 0);

  const std::vector<CDCHit*>& cdcHits = cdcTrack->getSortedCDCHitList();

  var<named("seed_cdc_hits")>() = cdcHits.size();
  var<named("seed_lowest_cdc_layer")>() = cdcHits.front()->getICLayer();

  genfit::MeasuredStateOnPlane firstMeasurement;
  if (state->mSoPSet()) {
    firstMeasurement = state->getMeasuredStateOnPlane();
  } else {
    B2ASSERT("Previous state was not fitted?", previousStates.back()->mSoPSet());
    firstMeasurement = previousStates.back()->getMeasuredStateOnPlane();
  }

  Vector3D position = Vector3D(firstMeasurement.getPos());
  Vector3D momentum = Vector3D(firstMeasurement.getMom());

  const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed());

  const Vector3D& hitPosition = static_cast<Vector3D>(spacePoint->getPosition());

  const double arcLength = trajectory.calcArcLength2D(hitPosition);
  const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
  const double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);

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

  // TODO: as long there are no SNR values available, these values can't be used
  //       include them when SNR for the VTX is available
//   const auto calculateSNR = [](const auto & s) {
//     return s.getSNR();
//   };
  const auto calculateChargeSizeRatio = [](const auto & s) {
    return s.getCharge() / s.getSize();
  };

  if (spacePoint->getType() == VXD::SensorInfoBase::SensorType::VTX) {
    const auto& clusters = spacePoint->getRelationsTo<VTXCluster>();

    const VTXCluster* cluster = clusters[0];

    var<named("cluster_charge")>() = calculateCharge(*cluster);
//     var<named("mean_rest_cluster_charge")>() = meanOver(allStates, calculateCharge);
//     var<named("min_rest_cluster_charge")>() = minOver(allStates, calculateCharge);
//     var<named("std_rest_cluster_charge")>() = stdOver(allStates, calculateCharge);

    var<named("cluster_seed_charge")>() = calculateSeedCharge(*cluster);
//     var<named("mean_rest_cluster_seed_charge")>() = meanOver(allStates, calculateSeedCharge);
//     var<named("min_rest_cluster_seed_charge")>() = minOver(allStates, calculateSeedCharge);
//     var<named("std_rest_cluster_seed_charge")>() = stdOver(allStates, calculateSeedCharge);

    var<named("cluster_size")>() = calculateSize(*cluster);
//     var<named("mean_rest_cluster_size")>() = meanOver(allStates, calculateSize);
//     var<named("min_rest_cluster_size")>() = minOver(allStates, calculateSize);
//     var<named("std_rest_cluster_size")>() = stdOver(allStates, calculateSize);

    // TODO: as long there are no SNR values available, these values can't be used
    //       include them when SNR for the VTX is available
    // var<named("cluster_snr")>() = calculateSNR(*firstCluster);
    // var<named("mean_rest_cluster_snr")>() = meanOver(allStates, calculateSNR);
    // var<named("min_rest_cluster_snr")>() = minOver(allStates, calculateSNR);
    // var<named("std_rest_cluster_snr")>() = stdOver(allStates, calculateSNR);

    var<named("cluster_charge_over_size")>() = calculateChargeSizeRatio(*cluster);
//     var<named("mean_rest_cluster_charge_over_size")>() = meanOver(allStates, calculateChargeSizeRatio);
//     var<named("min_rest_cluster_charge_over_size")>() = minOver(allStates, calculateChargeSizeRatio);
//     var<named("std_rest_cluster_charge_over_size")>() = stdOver(allStates, calculateChargeSizeRatio);

  }

  std::vector<const SpacePoint*> spacePoints;
  for (const CKFToVTXState* walkState : allStates) {
    const SpacePoint* sp = walkState->getHit();
    if (sp) {
      spacePoints.push_back(sp);
    }
  }

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
