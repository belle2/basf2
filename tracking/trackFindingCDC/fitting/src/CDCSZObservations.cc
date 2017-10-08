/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>

#include <tracking/trackFindingCDC/fitting/EigenObservationMatrix.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::size_t CDCSZObservations::fill(double s, double z, double weight)
{
  if (std::isnan(s)) return 0;
  if (std::isnan(z)) return 0;
  if (std::isnan(weight)) {
    B2WARNING("Weight is nan. Skipping observation");
    return 0;
  }

  m_szObservations.push_back(s);
  m_szObservations.push_back(z);
  m_szObservations.push_back(weight);
  return 1;
}

std::size_t CDCSZObservations::append(const CDCRecoHit3D& recoHit3D)
{
  if (m_onlyStereo and recoHit3D.isAxial()) return 0;

  const double s = recoHit3D.getArcLength2D();
  const double z = recoHit3D.getRecoPos3D().z();

  double weight = 1.0;
  if (m_fitVariance == EFitVariance::c_Unit) {
    weight = 1;
  } else {
    // Translate the drift length uncertainty to a uncertainty in z
    // by the taking the projected wire vector part parallel to the displacement
    // as a proportionality factor to the z direction.
    const CDCWire& wire = recoHit3D.getWire();
    const Vector3D& wireVector = wire.getWireVector();
    const Vector2D disp2D = recoHit3D.getRecoDisp2D();
    const double driftlengthVariance = recoHit3D.getRecoDriftLengthVariance();

    double dispNorm = disp2D.norm();

    double zeta = 1.0;
    if (dispNorm == 0.0) {
      zeta = wireVector.xy().norm() / wireVector.z();
    } else {
      zeta = wireVector.xy().dot(disp2D) / wireVector.z() / dispNorm;
    }

    weight = zeta * zeta / driftlengthVariance;
  }

  size_t appendedHit = fill(s, z, weight);
  return appendedHit;
}

std::size_t CDCSZObservations::appendRange(const std::vector<CDCRecoHit3D>& recoHit3Ds)
{
  std::size_t nAppendedHits = 0;
  for (const CDCRecoHit3D& recoHit3D : recoHit3Ds) {
    nAppendedHits += append(recoHit3D);
  }
  return nAppendedHits;
}

std::size_t CDCSZObservations::appendRange(const CDCSegment3D& segment3D)
{
  const std::vector<CDCRecoHit3D> recoHit3Ds = segment3D;
  return this->appendRange(recoHit3Ds);
}

std::size_t CDCSZObservations::appendRange(const CDCTrack& track)
{
  const std::vector<CDCRecoHit3D> recoHit3Ds = track;
  return this->appendRange(recoHit3Ds);
}

Vector2D CDCSZObservations::getCentralPoint() const
{
  std::size_t n = size();
  if (n == 0) return Vector2D(NAN, NAN);
  std::size_t i = n / 2;

  if (isEven(n)) {
    // For even number of observations use the middle one with the bigger distance from IP
    Vector2D center1(getS(i), getZ(i));
    Vector2D center2(getS(i - 1), getZ(i - 1));
    return center1.normSquared() > center2.normSquared() ? center1 : center2;
  } else {
    Vector2D center1(getS(i), getZ(i));
    return center1;
  }
}

void CDCSZObservations::passiveMoveBy(const Vector2D& origin)
{
  Eigen::Matrix<double, 1, 2> eigenOrigin(origin.x(), origin.y());
  EigenObservationMatrix eigenObservations = getEigenObservationMatrix(this);
  eigenObservations.leftCols<2>().rowwise() -= eigenOrigin;
}

Vector2D CDCSZObservations::centralize()
{
  // Pick an observation at the center
  Vector2D centralPoint = getCentralPoint();
  passiveMoveBy(centralPoint);
  return centralPoint;
}
