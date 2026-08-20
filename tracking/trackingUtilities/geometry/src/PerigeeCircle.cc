/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/geometry/PerigeeCircle.h>

#include <tracking/trackingUtilities/geometry/PerigeeParameters.h>

#include <tracking/trackingUtilities/geometry/Circle2D.h>

#include <tracking/trackingUtilities/numerics/EForwardBackward.h>
#include <tracking/trackingUtilities/numerics/ERotation.h>
#include <tracking/trackingUtilities/numerics/Quadratic.h>
#include <tracking/trackingUtilities/numerics/SpecialFunctions.h>
#include <tracking/trackingUtilities/numerics/Angle.h>

#include <ostream>
#include <utility>
#include <cmath>

namespace Belle2 {
  namespace TrackingUtilities {
    class GeneralizedCircle;
    class Line2D;
  }
}

using namespace Belle2;
using namespace TrackingUtilities;



PerigeeCircle::PerigeeCircle()
{
  invalidate();
}

PerigeeCircle::PerigeeCircle(double curvature, const ROOT::Math::XYVector& phi0Vec, double impact)
  : m_curvature(curvature)
  , m_phi0(phi0Vec.Phi())
  , m_phi0Vec(phi0Vec)
  , m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(double curvature, double phi0, double impact)
  : m_curvature(curvature)
  , m_phi0(phi0)
  , m_phi0Vec(VectorUtil::Phi(phi0))
  , m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(const PerigeeParameters& perigeeParameters)
  : PerigeeCircle(perigeeParameters(EPerigeeParameter::c_Curv),
                  perigeeParameters(EPerigeeParameter::c_Phi0),
                  perigeeParameters(EPerigeeParameter::c_I))
{
}

PerigeeCircle::PerigeeCircle(double curvature, double phi0, const ROOT::Math::XYVector& phi0Vec, double impact)
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

PerigeeCircle PerigeeCircle::fromN(double n0, const ROOT::Math::XYVector& n12, double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n12, n3);
  return circle;
}

PerigeeCircle
PerigeeCircle::fromCenterAndRadius(const ROOT::Math::XYVector& center, double absRadius, ERotation orientation)
{
  PerigeeCircle circle;
  circle.setCenterAndRadius(center, absRadius, orientation);
  return circle;
}

ROOT::Math::XYVector PerigeeCircle::atArcLength(double arcLength) const
{
  double chi = arcLength * curvature();
  double chiHalf = chi / 2.0;

  double atX = arcLength * sinc(chi);
  double atY = arcLength * sinc(chiHalf) * sin(chiHalf) + impact();
  return VectorUtil::compose(phi0Vec(), atX, atY);
}

void PerigeeCircle::reverse()
{
  m_curvature = -m_curvature;
  m_phi0 = AngleUtil::reversed(m_phi0);
  m_phi0Vec = -m_phi0Vec;
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
  ROOT::Math::XYVector newPhi0Vec = -phi0Vec();
  double newImpact = -curvature() / denominator;
  return PerigeeCircle(newCurvature, newPhi0, newPhi0Vec, newImpact);
}

void PerigeeCircle::invalidate()
{
  m_curvature = 0.0;
  m_phi0 = NAN;
  m_phi0Vec = ROOT::Math::XYVector(0.0, 0.0);
  m_impact = 0;
}

bool PerigeeCircle::isInvalid() const
{
  return (not std::isfinite(phi0()) or not std::isfinite(curvature()) or
          not std::isfinite(impact()) or VectorUtil::isNull(phi0Vec()));
}

void PerigeeCircle::passiveMoveBy(const ROOT::Math::XYVector& by)
{
  double arcLength = arcLengthTo(by);
  m_impact = distance(by);
  m_phi0 = m_phi0 + curvature() * arcLength;
  AngleUtil::normalise(m_phi0);
  m_phi0Vec = VectorUtil::Phi(m_phi0);
}

PerigeeJacobian PerigeeCircle::passiveMoveByJacobian(const ROOT::Math::XYVector& by) const
{
  PerigeeJacobian jacobian = PerigeeUtil::identity();
  passiveMoveByJacobian(by, jacobian);
  return jacobian;
}

void PerigeeCircle::passiveMoveByJacobian(const ROOT::Math::XYVector& by, PerigeeJacobian& jacobian) const
{
  ROOT::Math::XYVector deltaVec = by - perigee();
  double delta = deltaVec.R();
  double deltaParallel = phi0Vec().Dot(deltaVec);
  // double deltaOrthogonal = VectorUtil::Cross(phi0Vec(), deltaVec);
  // double zeta = deltaVec.Mag2();

  ROOT::Math::XYVector UVec = gradient(by);
  double U = UVec.R();
  double USquared = UVec.Mag2();
  double UOrthogonal = VectorUtil::Cross(phi0Vec(), UVec);
  // double UParallel = phi0Vec().Dot(UVec);

  // ROOT::Math::XYVector CB = VectorUtil::Orthogonal(gradient(by));
  // double U = sqrt(1 + curvature() * A);
  // double xi = 1.0 / CB.Mag2();
  // double nu = 1 - curvature() * deltaOrthogonal;
  // double mu = 1.0 / (U * (U + 1)) + curvature() * lambda;
  // double mu = 1.0 / U / 2.0;
  // double nu = -UOrthogonal;
  // double xi = 1 / USquared;

  // double halfA = fastDistance(by);
  // double A = 2 * halfA;
  // double lambda = halfA / ((1 + U) * (1 + U) * U);
  double dr = distance(by);

  // ROOT::Math::XYVector uVec = gradient(ROOT::Math::XYVector(0.0, 0.0));
  // double u = uVec.R();
  double u = 1 + curvature() * impact(); //= n12().R()

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

double PerigeeCircle::arcLengthTo(const ROOT::Math::XYVector& point) const
{
  ROOT::Math::XYVector closestToPoint = closest(point);
  double secantLength = VectorUtil::Distance(perigee(), closestToPoint);
  double deltaParallel = phi0Vec().Dot(point);
  return copysign(arcLengthAtSecantLength(secantLength), deltaParallel);
}

double PerigeeCircle::arcLengthBetween(const ROOT::Math::XYVector& from, const ROOT::Math::XYVector& to) const
{
  EForwardBackward lengthSign = isForwardOrBackwardOf(from, to);
  if (not NForwardBackward::isValid(lengthSign)) return NAN;
  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == EForwardBackward::c_Unknown) lengthSign = EForwardBackward::c_Forward;
  ROOT::Math::XYVector closestAtFrom = closest(from);
  ROOT::Math::XYVector closestAtTo = closest(to);
  double secantLength = VectorUtil::Distance(closestAtFrom, closestAtTo);
  return static_cast<double>(lengthSign) * arcLengthAtSecantLength(secantLength);
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

std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector> PerigeeCircle::atCylindricalR(const double cylindricalR) const
{
  const double u = (1 + curvature() * impact());
  const double orthogonal = ((square(impact()) + square(cylindricalR)) * curvature() / 2.0 + impact()) / u;
  const double parallel = sqrt(square(cylindricalR) - square(orthogonal));
  ROOT::Math::XYVector atCylindricalR1 = VectorUtil::compose(phi0Vec(), -parallel, orthogonal);
  ROOT::Math::XYVector atCylindricalR2 = VectorUtil::compose(phi0Vec(), parallel, orthogonal);
  std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector> result(atCylindricalR1, atCylindricalR2);
  return result;
}

ROOT::Math::XYVector PerigeeCircle::atCylindricalRForwardOf(const ROOT::Math::XYVector& startPoint,
                                                            const double cylindricalR) const
{
  std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector> candidatePoints = atCylindricalR(cylindricalR);
  return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);
}

ROOT::Math::XYVector PerigeeCircle::chooseNextForwardOf(const ROOT::Math::XYVector& start,
                                                        const ROOT::Math::XYVector& end1,
                                                        const ROOT::Math::XYVector& end2) const
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
    return ROOT::Math::XYVector(NAN, NAN);
  }
}

ROOT::Math::XYVector PerigeeCircle::closest(const ROOT::Math::XYVector& point) const
{
  return point - normal(point) * distance(point);
}

double PerigeeCircle::distance(double fastDistance) const
{
  double A = 2 * fastDistance;
  double U = std::sqrt(1 + A * curvature());
  return A / (1.0 + U);
}

double PerigeeCircle::fastDistance(const ROOT::Math::XYVector& point) const
{
  ROOT::Math::XYVector delta = point - perigee();
  double deltaOrthogonal = VectorUtil::Cross(phi0Vec(), delta);
  return -deltaOrthogonal + curvature() * delta.Mag2() / 2;
}

void PerigeeCircle::setCenterAndRadius(const ROOT::Math::XYVector& center,
                                       double absRadius,
                                       ERotation orientation)
{
  m_curvature = static_cast<double>(orientation) / std::fabs(absRadius);
  m_phi0Vec = VectorUtil::Orthogonal(center, NRotation::reversed(orientation));
  if (m_phi0Vec.R() != 0.0) {
    m_phi0Vec *= (1. / m_phi0Vec.R());
  }
  m_phi0 = m_phi0Vec.Phi();
  m_impact = (center.R() - std::fabs(absRadius)) * static_cast<double>(orientation);
}

void PerigeeCircle::setN(double n0, const ROOT::Math::XYVector& n12, double n3)
{
  double normalization = sqrt(n12.Mag2() - 4 * n0 * n3);
  m_curvature = 2 * n3 / normalization;
  m_phi0Vec = VectorUtil::Orthogonal(n12);
  if (m_phi0Vec.R() != 0.0) {
    m_phi0Vec *= (1. / m_phi0Vec.R());
  }
  m_phi0 = m_phi0Vec.Phi();
  m_impact = distance(n0 / normalization); // Uses the new curvature
}

std::ostream& TrackingUtilities::operator<<(std::ostream& output, const PerigeeCircle& circle)
{
  return output << "PerigeeCircle("
         << "curvature=" << circle.curvature() << ","
         << "phi0=" << circle.phi0() << ","
         << "impact=" << circle.impact() << ")";
}
