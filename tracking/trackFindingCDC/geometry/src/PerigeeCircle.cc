/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>

#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <tracking/trackFindingCDC/geometry/Circle2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/Quadratic.h>
#include <tracking/trackFindingCDC/numerics/SpecialFunctions.h>
#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <ostream>
#include <utility>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    class GeneralizedCircle;
    class Line2D;
  }
}

using namespace Belle2;
using namespace TrackFindingCDC;



PerigeeCircle::PerigeeCircle()
{
  invalidate();
}

PerigeeCircle::PerigeeCircle(double curvature, const Vector2D& phi0Vec, double impact)
  : m_curvature(curvature)
  , m_phi0(phi0Vec.phi())
  , m_phi0Vec(phi0Vec)
  , m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(double curvature, double phi0, double impact)
  : m_curvature(curvature)
  , m_phi0(phi0)
  , m_phi0Vec(Vector2D::Phi(phi0))
  , m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(const PerigeeParameters& perigeeParameters)
  : PerigeeCircle(perigeeParameters(EPerigeeParameter::c_Curv),
                  perigeeParameters(EPerigeeParameter::c_Phi0),
                  perigeeParameters(EPerigeeParameter::c_I))
{
}

PerigeeCircle::PerigeeCircle(double curvature, double phi0, const Vector2D& phi0Vec, double impact)
  : m_curvature(curvature)
  , m_phi0(phi0)
  , m_phi0Vec(phi0Vec)
  , m_impact(impact)
{
  /// Nothing to do here
}

PerigeeCircle::PerigeeCircle(const Line2D& n012)
{
  setN(n012);
}

PerigeeCircle::PerigeeCircle(const GeneralizedCircle& n0123)
{
  setN(n0123);
}

PerigeeCircle::PerigeeCircle(const Circle2D& circle)
{
  setCenterAndRadius(circle.center(), circle.absRadius(), circle.orientation());
}

PerigeeCircle PerigeeCircle::fromN(double n0, double n1, double n2, double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n1, n2, n3);
  return circle;
}

PerigeeCircle PerigeeCircle::fromN(double n0, const Vector2D& n12, double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n12, n3);
  return circle;
}

PerigeeCircle
PerigeeCircle::fromCenterAndRadius(const Vector2D& center, double absRadius, ERotation orientation)
{
  PerigeeCircle circle;
  circle.setCenterAndRadius(center, absRadius, orientation);
  return circle;
}

Vector2D PerigeeCircle::atArcLength(double arcLength) const
{
  double chi = arcLength * curvature();
  double chiHalf = chi / 2.0;

  double atX = arcLength * sinc(chi);
  double atY = arcLength * sinc(chiHalf) * sin(chiHalf) + impact();
  return Vector2D::compose(phi0Vec(), atX, atY);
}

void PerigeeCircle::reverse()
{
  m_curvature = -m_curvature;
  m_phi0 = AngleUtil::reversed(m_phi0);
  m_phi0Vec.reverse();
  m_impact = -m_impact;
}

PerigeeCircle PerigeeCircle::reversed() const
{
  return PerigeeCircle(-m_curvature, AngleUtil::reversed(m_phi0), -m_phi0Vec, -m_impact);
}

void PerigeeCircle::conformalTransform()
{
  double denominator = 2 + curvature() * impact();
  std::swap(m_impact, m_curvature);
  m_curvature *= denominator;
  m_impact /= denominator;
  // Also properly fixing the orientation to the opposite.
  reverse();
}

PerigeeCircle PerigeeCircle::conformalTransformed() const
{
  double denominator = 2 + curvature() * impact();
  // Properly fixing the orientation to the opposite by the minus signs
  double newCurvature = -impact() * denominator;
  double newPhi0 = AngleUtil::reversed(phi0());
  Vector2D newPhi0Vec = -phi0Vec();
  double newImpact = -curvature() / denominator;
  return PerigeeCircle(newCurvature, newPhi0, newPhi0Vec, newImpact);
}

void PerigeeCircle::invalidate()
{
  m_curvature = 0.0;
  m_phi0 = NAN;
  m_phi0Vec = Vector2D(0.0, 0.0);
  m_impact = 0;
}

bool PerigeeCircle::isInvalid() const
{
  return (not std::isfinite(phi0()) or not std::isfinite(curvature()) or
          not std::isfinite(impact()) or phi0Vec().isNull());
}

void PerigeeCircle::passiveMoveBy(const Vector2D& by)
{
  double arcLength = arcLengthTo(by);
  m_impact = distance(by);
  m_phi0 = m_phi0 + curvature() * arcLength;
  AngleUtil::normalise(m_phi0);
  m_phi0Vec = Vector2D::Phi(m_phi0);
}

PerigeeJacobian PerigeeCircle::passiveMoveByJacobian(const Vector2D& by) const
{
  PerigeeJacobian jacobian = PerigeeUtil::identity();
  passiveMoveByJacobian(by, jacobian);
  return jacobian;
}

void PerigeeCircle::passiveMoveByJacobian(const Vector2D& by, PerigeeJacobian& jacobian) const
{
  Vector2D deltaVec = by - perigee();
  double delta = deltaVec.norm();
  double deltaParallel = phi0Vec().dot(deltaVec);
  // double deltaOrthogonal = phi0Vec().cross(deltaVec);
  // double zeta = deltaVec.normSquared();

  Vector2D UVec = gradient(by);
  double U = UVec.norm();
  double USquared = UVec.normSquared();
  double UOrthogonal = phi0Vec().cross(UVec);
  // double UParallel = phi0Vec().dot(UVec);

  // Vector2D CB = gradient(by).orthogonal();
  // double U = sqrt(1 + curvature() * A);
  // double xi = 1.0 / CB.normSquared();
  // double nu = 1 - curvature() * deltaOrthogonal;
  // double mu = 1.0 / (U * (U + 1)) + curvature() * lambda;
  // double mu = 1.0 / U / 2.0;
  // double nu = -UOrthogonal;
  // double xi = 1 / USquared;

  // double halfA = fastDistance(by);
  // double A = 2 * halfA;
  // double lambda = halfA / ((1 + U) * (1 + U) * U);
  double dr = distance(by);

  // Vector2D uVec = gradient(Vector2D(0.0, 0.0));
  // double u = uVec.norm();
  double u = 1 + curvature() * impact(); //= n12().cylindricalR()

  using namespace NPerigeeParameterIndices;
  jacobian(c_Curv, c_Curv) = 1;
  jacobian(c_Curv, c_Phi0) = 0;
  jacobian(c_Curv, c_I) = 0;

  jacobian(c_Phi0, c_Curv) = deltaParallel / USquared;
  jacobian(c_Phi0, c_Phi0) = -u * UOrthogonal / USquared;
  jacobian(c_Phi0, c_I) = -curvature() * curvature() * deltaParallel / USquared;

  jacobian(c_I, c_Curv) = (delta - dr) * (delta + dr) / U / 2;
  jacobian(c_I, c_Phi0) = u * deltaParallel / U;
  jacobian(c_I, c_I) = -UOrthogonal / U;
}

double PerigeeCircle::arcLengthTo(const Vector2D& point) const
{
  Vector2D closestToPoint = closest(point);
  double secantLength = perigee().distance(closestToPoint);
  double deltaParallel = phi0Vec().dot(point);
  return copysign(arcLengthAtSecantLength(secantLength), deltaParallel);
}

double PerigeeCircle::arcLengthBetween(const Vector2D& from, const Vector2D& to) const
{
  EForwardBackward lengthSign = isForwardOrBackwardOf(from, to);
  if (not NForwardBackward::isValid(lengthSign)) return NAN;
  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == EForwardBackward::c_Unknown) lengthSign = EForwardBackward::c_Forward;
  Vector2D closestAtFrom = closest(from);
  Vector2D closestAtTo = closest(to);
  double secantLength = closestAtFrom.distance(closestAtTo);
  return lengthSign * arcLengthAtSecantLength(secantLength);
}

double PerigeeCircle::arcLengthToCylindricalR(double cylindricalR) const
{
  // Slight trick here
  // Since the sought point is on the helix we treat it as the perigee
  // and the origin as the point to extrapolate to.
  // We know the distance of the origin to the circle, which is just d0
  // The direct distance from the origin to the imaginary perigee is just the given cylindricalR.
  return arcLengthAtDeltaLength(cylindricalR, impact());
}

double PerigeeCircle::arcLengthAtDeltaLength(double delta, double dr) const
{
  const double secantLength = sqrt((delta + dr) * (delta - dr) / (1 + dr * curvature()));
  const double arcLength = arcLengthAtSecantLength(secantLength);
  return arcLength;
}

double PerigeeCircle::arcLengthAtSecantLength(double secantLength) const
{
  double x = secantLength * curvature() / 2.0;
  double arcLengthFactor = asinc(x);
  return secantLength * arcLengthFactor;
}

std::pair<Vector2D, Vector2D> PerigeeCircle::atCylindricalR(const double cylindricalR) const
{
  const double u = (1 + curvature() * impact());
  const double orthogonal = ((square(impact()) + square(cylindricalR)) * curvature() / 2.0 + impact()) / u;
  const double parallel = sqrt(square(cylindricalR) - square(orthogonal));
  Vector2D atCylindricalR1 = Vector2D::compose(phi0Vec(), -parallel, orthogonal);
  Vector2D atCylindricalR2 = Vector2D::compose(phi0Vec(), parallel, orthogonal);
  std::pair<Vector2D, Vector2D> result(atCylindricalR1, atCylindricalR2);
  return result;
}

Vector2D PerigeeCircle::atCylindricalRForwardOf(const Vector2D& startPoint,
                                                const double cylindricalR) const
{
  std::pair<Vector2D, Vector2D> candidatePoints = atCylindricalR(cylindricalR);
  return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);
}

Vector2D PerigeeCircle::chooseNextForwardOf(const Vector2D& start,
                                            const Vector2D& end1,
                                            const Vector2D& end2) const
{
  double arcLength1 = arcLengthBetween(start, end1);
  double arcLength2 = arcLengthBetween(start, end2);
  if (arcLength1 < 0) arcLength1 += arcLengthPeriod();
  if (arcLength2 < 0) arcLength2 += arcLengthPeriod();
  if (fmin(arcLength1, arcLength2) == arcLength1) {
    return end1;
  } else if (fmin(arcLength1, arcLength2) == arcLength2) {
    return end2;
  } else {
    return Vector2D(NAN, NAN);
  }
}

Vector2D PerigeeCircle::closest(const Vector2D& point) const
{
  return point - normal(point) * distance(point);
}

double PerigeeCircle::distance(double fastDistance) const
{
  double A = 2 * fastDistance;
  double U = std::sqrt(1 + A * curvature());
  return A / (1.0 + U);
}

double PerigeeCircle::fastDistance(const Vector2D& point) const
{
  Vector2D delta = point - perigee();
  double deltaOrthogonal = phi0Vec().cross(delta);
  return -deltaOrthogonal + curvature() * delta.normSquared() / 2;
}

void PerigeeCircle::setCenterAndRadius(const Vector2D& center,
                                       double absRadius,
                                       ERotation orientation)
{
  m_curvature = orientation / std::fabs(absRadius);
  m_phi0Vec = center.orthogonal(NRotation::reversed(orientation));
  m_phi0Vec.normalize();
  m_phi0 = m_phi0Vec.phi();
  m_impact = (center.norm() - std::fabs(absRadius)) * orientation;
}

void PerigeeCircle::setN(double n0, const Vector2D& n12, double n3)
{
  double normalization = sqrt(n12.normSquared() - 4 * n0 * n3);
  m_curvature = 2 * n3 / normalization;
  m_phi0Vec = n12.orthogonal();
  m_phi0Vec.normalize();
  m_phi0 = m_phi0Vec.phi();
  m_impact = distance(n0 / normalization); // Uses the new curvature
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const PerigeeCircle& circle)
{
  return output << "PerigeeCircle("
         << "curvature=" << circle.curvature() << ","
         << "phi0=" << circle.phi0() << ","
         << "impact=" << circle.impact() << ")";
}
