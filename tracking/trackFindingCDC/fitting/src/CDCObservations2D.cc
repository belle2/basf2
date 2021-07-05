/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/fitting/EigenObservationMatrix.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitTriple.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

double CDCObservations2D::getPseudoDriftLengthVariance(const CDCWireHit& wireHit)
{
  return getPseudoDriftLengthVariance(wireHit.getRefDriftLength(),
                                      wireHit.getRefDriftLengthVariance());
}

std::size_t
CDCObservations2D::fill(double x, double y, double signedRadius, double weight)
{
  if (std::isnan(x)) return 0;
  if (std::isnan(y)) return 0;

  if (std::isnan(signedRadius)) {
    B2WARNING("Signed radius is nan. Skipping observation");
    return 0;
  }

  if (std::isnan(weight)) {
    B2WARNING("Weight is nan. Skipping observation");
    return 0;
  }

  m_observations.push_back(x);
  m_observations.push_back(y);
  m_observations.push_back(signedRadius);
  m_observations.push_back(weight);
  return 1;
}

std::size_t
CDCObservations2D::fill(const Vector2D& pos2D, double signedRadius, double weight)
{
  return fill(pos2D.x(), pos2D.y(), signedRadius, weight);
}

std::size_t CDCObservations2D::append(const CDCWireHit& wireHit, ERightLeft rlInfo)
{
  const Vector2D& wireRefPos2D = wireHit.getRefPos2D();

  double signedDriftLength = 0;
  if (m_fitPos == EFitPos::c_RLDriftCircle and isValid(rlInfo)) {
    signedDriftLength = rlInfo * wireHit.getRefDriftLength();
  } else {
    signedDriftLength = 0;
  }

  double variance = 1;
  if (m_fitVariance == EFitVariance::c_Unit) {
    variance = 1;
  } else if (m_fitVariance == EFitVariance::c_Nominal) {
    variance = CDCWireHit::c_simpleDriftLengthVariance;
  } else if (m_fitVariance == EFitVariance::c_DriftLength) {
    const double driftLength = wireHit.getRefDriftLength();
    variance = fabs(driftLength);
  } else if (m_fitVariance == EFitVariance::c_Pseudo) {
    variance = getPseudoDriftLengthVariance(wireHit);
  } else if (m_fitVariance == EFitVariance::c_Proper) {
    if (abs(rlInfo) != 1) {
      variance = getPseudoDriftLengthVariance(wireHit);
    } else {
      variance = wireHit.getRefDriftLengthVariance();
    }
  }
  return fill(wireRefPos2D, signedDriftLength, 1 / variance);
}

std::size_t CDCObservations2D::append(const CDCWireHit* wireHit, ERightLeft rlInfo)
{
  if (wireHit) {
    return append(*(wireHit), rlInfo);
  } else {
    return 0;
  }
}

std::size_t CDCObservations2D::append(const CDCRLWireHit& rlWireHit)
{
  const ERightLeft rlInfo = rlWireHit.getRLInfo();

  const double driftLength = rlWireHit.getRefDriftLength();
  const double driftLengthVariance = rlWireHit.getRefDriftLengthVariance();

  const Vector2D& wireRefPos2D = rlWireHit.getRefPos2D();

  double signedDriftLength = 0;
  if (m_fitPos == EFitPos::c_RLDriftCircle and isValid(rlInfo)) {
    signedDriftLength = rlInfo * driftLength;
  } else {
    signedDriftLength = 0;
  }

  double variance = 1;
  if (m_fitVariance == EFitVariance::c_Unit) {
    variance = 1;
  } else if (m_fitVariance == EFitVariance::c_Nominal) {
    variance = CDCWireHit::c_simpleDriftLengthVariance;
  } else if (m_fitVariance == EFitVariance::c_DriftLength) {
    variance = fabs(driftLength);
  } else if (m_fitVariance == EFitVariance::c_Pseudo) {
    variance = getPseudoDriftLengthVariance(driftLength, driftLengthVariance);
  } else if (m_fitVariance == EFitVariance::c_Proper) {
    if (abs(rlInfo) != 1) {
      variance = getPseudoDriftLengthVariance(driftLength, driftLengthVariance);
    } else {
      variance = driftLengthVariance;
    }
  }

  return fill(wireRefPos2D, signedDriftLength, 1 / variance);
}

std::size_t CDCObservations2D::append(const CDCRLWireHitPair& rlWireHitPair)
{
  return append(rlWireHitPair.getFromRLWireHit()) + append(rlWireHitPair.getToRLWireHit());
}

std::size_t CDCObservations2D::append(const CDCRLWireHitTriple& rlWireHitTriple)
{
  return append(rlWireHitTriple.getStartRLWireHit()) +
         append(rlWireHitTriple.getMiddleRLWireHit()) + append(rlWireHitTriple.getEndRLWireHit());
}

std::size_t CDCObservations2D::append(const CDCFacet& facet)
{
  if (m_fitPos == EFitPos::c_RecoPos) {
    return append(facet.getStartRecoHit2D()) + append(facet.getMiddleRecoHit2D()) +
           append(facet.getEndRecoHit2D());
  } else {
    const CDCRLWireHitTriple& rlWireHitTriple = facet;
    return append(rlWireHitTriple);
  }
}

std::size_t CDCObservations2D::append(const CDCRecoHit2D& recoHit2D)
{
  Vector2D fitPos2D;
  double signedDriftLength = 0;
  if (m_fitPos == EFitPos::c_RecoPos) {
    fitPos2D = recoHit2D.getRecoPos2D();
    signedDriftLength = 0;

    // Fall back to the rl circle in case position is not setup
    if (fitPos2D.hasNAN()) {
      fitPos2D = recoHit2D.getWire().getRefPos2D();
      signedDriftLength = recoHit2D.getSignedRefDriftLength();
    }

  } else if (m_fitPos == EFitPos::c_RLDriftCircle) {
    fitPos2D = recoHit2D.getWire().getRefPos2D();
    signedDriftLength = recoHit2D.getSignedRefDriftLength();
  } else if (m_fitPos == EFitPos::c_WirePos) {
    fitPos2D = recoHit2D.getWire().getRefPos2D();
    signedDriftLength = 0;
  }

  const double driftLength = recoHit2D.getRefDriftLength();
  const ERightLeft rlInfo = recoHit2D.getRLInfo();

  double variance = recoHit2D.getRefDriftLengthVariance();
  if (m_fitVariance == EFitVariance::c_Unit) {
    variance = 1;
  } else if (m_fitVariance == EFitVariance::c_Nominal) {
    variance = CDCWireHit::c_simpleDriftLengthVariance;
  } else if (m_fitVariance == EFitVariance::c_DriftLength) {
    variance = std::fabs(driftLength);
  } else if (m_fitVariance == EFitVariance::c_Pseudo or abs(rlInfo) != 1) {
    // Fall back to the pseudo variance if the rl information is not known
    variance = getPseudoDriftLengthVariance(driftLength, variance);
  } else if (m_fitVariance == EFitVariance::c_Proper) {
    variance = recoHit2D.getRefDriftLengthVariance();
  }
  return fill(fitPos2D, signedDriftLength, 1 / variance);
}

std::size_t CDCObservations2D::append(const CDCRecoHit3D& recoHit3D)
{
  Vector2D fitPos2D = recoHit3D.getRecoPos2D();
  double signedDriftLength = 0;
  if (m_fitPos == EFitPos::c_RecoPos) {
    fitPos2D = recoHit3D.getRecoPos2D();
    signedDriftLength = 0;
  } else if (m_fitPos == EFitPos::c_RLDriftCircle) {
    fitPos2D = recoHit3D.getRecoWirePos2D();
    signedDriftLength = recoHit3D.getSignedRecoDriftLength();
  } else if (m_fitPos == EFitPos::c_WirePos) {
    fitPos2D = recoHit3D.getRecoWirePos2D();
    signedDriftLength = 0;
  }

  const double driftLength = std::fabs(recoHit3D.getSignedRecoDriftLength());
  const ERightLeft rlInfo = recoHit3D.getRLInfo();

  double variance = recoHit3D.getRecoDriftLengthVariance();
  if (m_fitVariance == EFitVariance::c_Unit) {
    variance = 1;
  } else if (m_fitVariance == EFitVariance::c_Nominal) {
    variance = CDCWireHit::c_simpleDriftLengthVariance;
  } else if (m_fitVariance == EFitVariance::c_DriftLength) {
    variance = std::fabs(driftLength);
  } else if (m_fitVariance == EFitVariance::c_Pseudo or abs(rlInfo) != 1) {
    // Fall back to the pseudo variance if the rl information is not known
    variance = getPseudoDriftLengthVariance(driftLength, variance);
  } else if (m_fitVariance == EFitVariance::c_Proper) {
    variance = recoHit3D.getRecoDriftLengthVariance();
  }
  return fill(fitPos2D, signedDriftLength, 1 / variance);
}

std::size_t CDCObservations2D::appendRange(const CDCSegment2D& segment2D)
{
  std::size_t nAppendedHits = 0;
  for (const CDCRecoHit2D& recoHit2D : segment2D) {
    nAppendedHits += append(recoHit2D);
  }
  return nAppendedHits;
}

std::size_t CDCObservations2D::appendRange(const CDCSegment3D& segment3D)
{
  std::size_t nAppendedHits = 0;
  for (const CDCRecoHit3D& recoHit3D : segment3D) {
    nAppendedHits += append(recoHit3D);
  }
  return nAppendedHits;
}

std::size_t CDCObservations2D::appendRange(const CDCAxialSegmentPair& axialSegmentPair)
{
  std::size_t nAppendedHits = 0;
  const CDCSegment2D* ptrStartSegment2D = axialSegmentPair.getStartSegment();
  if (ptrStartSegment2D) {
    const CDCSegment2D& startSegment2D = *ptrStartSegment2D;
    nAppendedHits += appendRange(startSegment2D);
  }

  const CDCSegment2D* ptrEndSegment2D = axialSegmentPair.getEndSegment();
  if (ptrEndSegment2D) {
    const CDCSegment2D& endSegment2D = *ptrEndSegment2D;
    nAppendedHits += appendRange(endSegment2D);
  }
  return nAppendedHits;
}

std::size_t CDCObservations2D::appendRange(const CDCTrack& track)
{
  std::size_t nAppendedHits = 0;
  for (const CDCRecoHit3D& recoHit3D : track) {
    nAppendedHits += append(recoHit3D);
  }
  return nAppendedHits;
}

std::size_t CDCObservations2D::appendRange(const std::vector<const CDCWire*>& wires)
{
  std::size_t nAppendedHits = 0;
  for (const CDCWire* ptrWire : wires) {
    if (not ptrWire) continue;
    const CDCWire& wire = *ptrWire;
    const Vector2D& wirePos = wire.getRefPos2D();
    const double driftLength = 0.0;
    const double weight = 1.0;
    nAppendedHits += fill(wirePos, driftLength, weight);
  }
  return nAppendedHits;
}

std::size_t CDCObservations2D::appendRange(const CDCWireHitSegment& wireHits)
{
  std::size_t nAppendedHits = 0;
  for (const CDCWireHit* ptrWireHit : wireHits) {
    if (not ptrWireHit) continue;
    const CDCWireHit& wireHit = *ptrWireHit;
    nAppendedHits += append(wireHit);
  }
  return nAppendedHits;
}

double CDCObservations2D::getTotalPerpS(const CDCTrajectory2D& trajectory2D) const
{
  return trajectory2D.calcArcLength2DBetween(getFrontPos2D(), getBackPos2D());
}

std::size_t CDCObservations2D::getNObservationsWithDriftRadius() const
{
  std::size_t result = 0;
  Index nObservations = size();

  for (Index iObservation = 0; iObservation < nObservations; ++iObservation) {
    const double driftLength = getDriftLength(iObservation);
    bool hasDriftLength = (driftLength != 0.0);
    result += hasDriftLength ? 1 : 0;
  }

  return result;
}

Vector2D CDCObservations2D::getCentralPoint() const
{
  std::size_t n = size();
  if (n == 0) return Vector2D(NAN, NAN);
  std::size_t i = n / 2;

  if (isEven(n)) {
    // For even number of observations use the middle one with the bigger distance from IP
    Vector2D center1(getX(i), getY(i));
    Vector2D center2(getX(i - 1), getY(i - 1));
    return center1.normSquared() > center2.normSquared() ? center1 : center2;
  } else {
    Vector2D center1(getX(i), getY(i));
    return center1;
  }
}

void CDCObservations2D::passiveMoveBy(const Vector2D& origin)
{
  Eigen::Matrix<double, 1, 2> eigenOrigin(origin.x(), origin.y());
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(this);
  eigenObservations.leftCols<2>().rowwise() -= eigenOrigin;
}

Vector2D CDCObservations2D::centralize()
{
  // Pick an observation at the center
  Vector2D centralPoint = getCentralPoint();
  passiveMoveBy(centralPoint);
  return centralPoint;
}
