/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/cdcToSpacePoint/state/SimpleCKFCDCToSpacePointStateObjectFilter.h>
#include <tracking/ckf/findlets/cdcToSVDSpacePoint/CDCToSVDSpacePointMatcher.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  template <class ARhs, class ALhs>
  decltype(ARhs() % ALhs()) mod(ARhs a, ALhs b)
  {
    return (a % b + b) % b;
  }
}

void SimpleCKFCDCToSpacePointStateObjectFilter::exposeParameters(ModuleParamList* moduleParamList,
    const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumHelixChi2XYZ"),
                                m_param_maximumHelixChi2XYZ, "", m_param_maximumHelixChi2XYZ);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumChi2XY"),
                                m_param_maximumChi2XY, "", m_param_maximumChi2XY);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumChi2"),
                                m_param_maximumChi2, "", m_param_maximumChi2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "hitJumpingUpTo"), m_param_hitJumpingUpTo,
                                "", m_param_hitJumpingUpTo);
}

void SimpleCKFCDCToSpacePointStateObjectFilter::initialize()
{
  BaseCKFCDCToSpacePointStateObjectFilter::initialize();

  B2ASSERT("You need to provide exactly 6 maximal norms (for six layers).", m_param_maximumHelixChi2XYZ.size() == 6);
  B2ASSERT("You need to provide exactly 6 maximal norms (for six layers).", m_param_maximumChi2XY.size() == 6);
  B2ASSERT("You need to provide exactly 6 maximal norms (for six layers).", m_param_maximumChi2.size() == 6);
}

void SimpleCKFCDCToSpacePointStateObjectFilter::beginRun()
{
  m_cachedBField = TrackFindingCDC::CDCBFieldUtil::getBFieldZ();
}

Weight SimpleCKFCDCToSpacePointStateObjectFilter::operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState)
{
  const SpacePoint* spacePoint = currentState.getHit();

  unsigned int numberOfHoles = 0;

  currentState.walk([&numberOfHoles](const BaseCKFCDCToSpacePointStateObjectFilter::Object * walkObject) {
    if (not walkObject->getHit() and not isOnOverlapLayer(*walkObject)) {
      numberOfHoles++;
    }
  });

  // Allow layers to have no hit
  // TODO: do only allow this in some cases, where it is reasonable to have no hit
  if (not spacePoint) {
    if (isOnOverlapLayer(currentState) or numberOfHoles <= m_param_hitJumpingUpTo) {
      return 1;
    } else {
      return NAN;
    }
  }

  if (spacePoint->getType() == VXD::SensorInfoBase::PXD) {
    if (not isOnOverlapLayer(currentState)) {
      if (spacePoint->getVxdID().getLayerNumber() == 2) {
        static std::map<unsigned int, std::vector<unsigned int>> ladderMapping = {
          {1, {1,  2,  3}},
          {2, {3,  4}},
          {3, {4,  5,  6}},
          {4, {6,  7,  8}},
          {5, {8,  9}},
          {6, {9,  10, 11, 12}},
          {7, {11, 12, 1}},
        };


        RecoTrack* seed = currentState.getSeedRecoTrack();
        if (not seed->getSVDHitList().empty()) {
          SVDCluster* firstSVDHit = seed->getSortedSVDHitList()[1];
          const VxdID& currentID = spacePoint->getVxdID();
          const VxdID& lastID = firstSVDHit->getSensorID();

          const unsigned int lastLadder = lastID.getLadderNumber();
          const unsigned int currentLadder = currentID.getLadderNumber();

          if (lastID.getLayerNumber() == 3) {
            if (not TrackFindingCDC::is_in(currentLadder, ladderMapping.at(lastLadder))) {
              return NAN;
            }
          }
        }

      } else {
        const auto* overlappingParent = currentState.getParent();
        if (overlappingParent) {
          const auto* lastLayerParent = overlappingParent->getParent();
          if (lastLayerParent and lastLayerParent->getHit()) {
            static std::map<unsigned int, std::vector<unsigned int>> ladderMapping = {
              {1,  {1}},
              {2,  {1, 2}},
              {3,  {2, 3}},
              {4,  {3}},
              {5,  {3, 4}},
              {6,  {4, 5}},
              {7,  {5}},
              {8,  {5, 6}},
              {9,  {6, 7}},
              {10, {7}},
              {11, {7, 8}},
              {12, {8, 1}},
            };

            const VxdID& currentID = spacePoint->getVxdID();
            const VxdID& lastID = lastLayerParent->getHit()->getVxdID();

            const unsigned int lastLadder = lastID.getLadderNumber();
            const unsigned int currentLadder = currentID.getLadderNumber();

            if (not TrackFindingCDC::is_in(currentLadder, ladderMapping.at(lastLadder))) {
              return NAN;
            }
          }
        }
      }
    }
  } else {
    // Check the distance (in ladders and sensors) to the last hit and only allow those, which point "more or less" to
    // the origin
    if (not isOnOverlapLayer(currentState)) {
      const auto* overlappingParent = currentState.getParent();
      if (overlappingParent) {
        const auto* lastLayerParent = overlappingParent->getParent();
        if (lastLayerParent and lastLayerParent->getHit()) {
          const VxdID& currentID = spacePoint->getVxdID();
          const VxdID& lastID = lastLayerParent->getHit()->getVxdID();

          const int deltaSensor = lastID.getSensorNumber() - currentID.getSensorNumber();
          const int deltaLadder = mod(lastID.getLadderNumber() - currentID.getLadderNumber(),
                                      CDCToSVDSpacePointMatcher::maximumLadderNumbers[currentID.getLayerNumber()]);

          if ((deltaSensor != 0 and deltaSensor != 1) or (deltaLadder > 5)) {
            return NAN;
          }
        }
      }
    }
  }

  const Vector3D position(currentState.getMSoPPosition());
  const Vector3D hitPosition(currentState.getHit()->getPosition());

  const double sameHemisphere = fabs(position.phi() - hitPosition.phi()) < TMath::PiOver2();

  if (sameHemisphere != 1) {
    return NAN;
  }

  const TMatrixDSym& cov = currentState.getMSoPCovariance();
  const double layer = extractGeometryLayer(currentState);

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    // Filter 1
    const RecoTrack* cdcTrack = currentState.getSeedRecoTrack();

    const Vector3D momentum(currentState.getMSoPMomentum());
    const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed(), m_cachedBField);

    const double arcLength = trajectory.calcArcLength2D(hitPosition);
    const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
    const double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);
    const Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
    const Vector3D differenceHelix = trackPositionAtHit - hitPosition;

    const double helix_chi2_xyz = (differenceHelix.x() * differenceHelix.x() / sqrt(cov(0, 0)) +
                                   differenceHelix.y() * differenceHelix.y() / sqrt(cov(1, 1)) +
                                   differenceHelix.z() * differenceHelix.z() / sqrt(cov(2, 2)));

    if (helix_chi2_xyz > m_param_maximumHelixChi2XYZ[layer]) {
      return NAN;
    } else {
      return helix_chi2_xyz;
    }
  } else if (not currentState.isFitted()) {
    // Filter 2
    const Vector3D& difference = position - hitPosition;

    const double chi2_xy = (difference.x() * difference.x() / sqrt(cov(0, 0)) +
                            difference.y() * difference.y() / sqrt(cov(1, 1)));
    const double chi2_xyz = chi2_xy + difference.z() * difference.z() / sqrt(cov(2, 2));

    if (chi2_xy > m_param_maximumChi2XY[layer]) {
      return NAN;
    } else {
      return chi2_xyz;
    }
  } else {
    // Filter 3
    const double chi2 = currentState.getChi2();
    if (chi2 > m_param_maximumChi2[layer]) {
      return NAN;
    } else {
      return chi2;
    }
  }
}
