/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/geometry/GeneralizedCircle.h>

#include <tracking/trackingUtilities/geometry/Circle2D.h>
#include <tracking/trackingUtilities/geometry/Line2D.h>

#include <tracking/trackingUtilities/numerics/EForwardBackward.h>
#include <tracking/trackingUtilities/numerics/ERotation.h>

#include <tracking/trackingUtilities/numerics/SpecialFunctions.h>
#include <tracking/trackingUtilities/numerics/Quadratic.h>

#include <ostream>
#include <cmath>

using namespace Belle2;
using namespace TrackingUtilities;

GeneralizedCircle::GeneralizedCircle(const double n0,
                                     const double n1,
                                     const double n2,
                                     const double n3)
  : m_n3(n3)
  , m_n12(n1, n2)
  , m_n0(n0)
{
  normalize();
}

GeneralizedCircle::GeneralizedCircle(const double n0, const ROOT::Math::XYVector& n12, const double n3)
  : m_n3(n3)
  , m_n12(n12)
  , m_n0(n0)
{
  normalize();
}

GeneralizedCircle::GeneralizedCircle(const Line2D& n012)
  : m_n3(0.0)
  , m_n12(n012.n12())
  , m_n0(n012.n0())
{
  normalize();
}

GeneralizedCircle::GeneralizedCircle(const Circle2D& circle)
  : m_n3(1.0 / 2.0 / circle.radius())
  , m_n12(-circle.center().x() * (m_n3 * 2.0), -circle.center().y() * (m_n3 * 2.0))
  , m_n0((circle.center().Mag2() - circle.radiusSquared()) * m_n3)
{
}

GeneralizedCircle GeneralizedCircle::fromCenterAndRadius(const ROOT::Math::XYVector& center,
                                                         const double absRadius,
                                                         const ERotation orientation)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setCenterAndRadius(center, absRadius, orientation);
  return generalizedCircle;
}

GeneralizedCircle GeneralizedCircle::fromPerigeeParameters(const double curvature,
                                                           const ROOT::Math::XYVector& tangential,
                                                           const double impact)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setPerigeeParameters(curvature, tangential, impact);
  return generalizedCircle;
}

GeneralizedCircle GeneralizedCircle::fromPerigeeParameters(const double curvature,
                                                           const double tangentialPhi,
                                                           const double impact)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setPerigeeParameters(curvature, tangentialPhi, impact);
  return generalizedCircle;
}

void GeneralizedCircle::setCenterAndRadius(const ROOT::Math::XYVector& center,
                                           const double absRadius,
                                           const ERotation orientation)
{
  double curvature = static_cast<double>(orientation) / fabs(absRadius);
  setN0((center.Mag2() - absRadius * absRadius) * curvature / 2.0);
  setN1(-center.x() * curvature);
  setN2(-center.y() * curvature);
  setN3(curvature / 2.0);
  normalize(); // the call to normalize should be superfluous
}

void GeneralizedCircle::setPerigeeParameters(const double curvature,
                                             const ROOT::Math::XYVector& tangential,
                                             const double impact)
{
  double loc_n0 = impact * (impact * curvature / 2.0 + 1.0);
  ROOT::Math::XYVector loc_n12 = -VectorUtil::Orthogonal(tangential) * (1 + curvature * impact);
  double loc_n3 = curvature / 2.0;
  setN(loc_n0, loc_n12, loc_n3);
}

ROOT::Math::XYVector GeneralizedCircle::closest(const ROOT::Math::XYVector& point) const
{
  if (fastDistance(point) == 0) return point;

  // solve the minimization | point - pointOnCirlce | ^2 subjected to the on circle constraint
  //                       1.0                   * m_n0 +
  //                       point.x()             * m_n1 +
  //                       point.y()             * m_n2 +
  //                       point.cylindricalRSquared() * m_n3 == 0
  // solved with a Lagrangian multiplicator for the constraint

  ROOT::Math::XYVector gradientAtPoint = gradient(point);
  ROOT::Math::XYVector coordinateSystem = VectorUtil::unit(gradientAtPoint);

  // component of closest approach orthogonal to coordinateSystem
  double closestOrthogonal = VectorUtil::Cross(n12(), point) / gradientAtPoint.R();

  // component of closest approach parallel to coordinateSystem - two solutions expected
  double nOrthogonal = VectorUtil::unnormalizedOrthogonalComp(n12(), coordinateSystem);
  double nParallel = VectorUtil::unnormalizedParallelComp(n12(), coordinateSystem);

  double closestParallel = 0.0;
  if (isLine()) {
    closestParallel = -(nOrthogonal * closestOrthogonal + n0()) / nParallel;

  } else {
    const double a = n3();
    const double b = nParallel;
    const double c = n0() + (nOrthogonal + n3() * closestOrthogonal) * closestOrthogonal;

    const std::pair<double, double> closestParallel12 = solveQuadraticABC(a, b, c);

    // take the solution with smaller distance to point
    const double pointParallel = VectorUtil::unnormalizedParallelComp(point, coordinateSystem);

    const double criterion1 =
      closestParallel12.first * (closestParallel12.first - 2 * pointParallel);
    const double criterion2 =
      closestParallel12.second * (closestParallel12.second - 2 * pointParallel);

    closestParallel = criterion1 < criterion2 ? closestParallel12.first : closestParallel12.second;
  }
  return VectorUtil::compose(coordinateSystem, closestParallel, closestOrthogonal);
}

ROOT::Math::XYVector GeneralizedCircle::perigee() const
{
  ROOT::Math::XYVector result(n12());
  result *= (-impact() / result.R());
  return result;
}

ROOT::Math::XYVector GeneralizedCircle::apogee() const
{
  ROOT::Math::XYVector result(n12());
  result *= ((-impact() - 2 * radius()) / result.R());
  return result;
}

ROOT::Math::XYVector GeneralizedCircle::chooseNextForwardOf(const ROOT::Math::XYVector& start,
                                                            const ROOT::Math::XYVector& end1,
                                                            const ROOT::Math::XYVector& end2) const
{
  double lengthOnCurve1 = arcLengthBetween(start, end1);
  double lengthOnCurve2 = arcLengthBetween(start, end2);

  if (lengthOnCurve1 >= 0.0) {

    if (lengthOnCurve2 >= 0.0) {
      return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
    } else {
      return end1;
    }
  } else {

    if (lengthOnCurve2 >= 0.0) {
      return end2;
    } else {

      // both lengths on curve have a negative sign
      // the candidate with the lesser length on curve wins because of the discontinuaty
      // unless the generalized circle is a line
      // in this case their is no forward intersection with the same cylindrical radius
      if (isLine()) {
        return ROOT::Math::XYVector(NAN, NAN);
      } else {
        return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
      }
    }
  }
  return ROOT::Math::XYVector(NAN, NAN); // just avoid a compiler warning
}

std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector> GeneralizedCircle::atCylindricalR(const double cylindricalR) const
{
  // extraploted to r
  // solve
  //  n0 + n1*x + n2*y + n3*r*r == 0
  //  and r = cylindricalR
  // search for x and y

  // solve the equation in a coordinate system parallel and orthogonal to the reduced circle center
  const ROOT::Math::XYVector nUnit = VectorUtil::unit(n12());

  // parallel component
  const double sameCylindricalRParallel = -(n0() + n3() * square(cylindricalR)) / n12().R();

  // orthogonal component
  const double sameCylindricalROrthogonal = sqrt(square(cylindricalR) - square(sameCylindricalRParallel));

  /// Two versions in this case
  ROOT::Math::XYVector sameCylindricalR1 =
    VectorUtil::compose(nUnit, sameCylindricalRParallel, -sameCylindricalROrthogonal);

  ROOT::Math::XYVector sameCylindricalR2 =
    VectorUtil::compose(nUnit, sameCylindricalRParallel, sameCylindricalROrthogonal);

  std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector> result(sameCylindricalR1, sameCylindricalR2);
  return result;
}

ROOT::Math::XYVector GeneralizedCircle::atCylindricalRForwardOf(const ROOT::Math::XYVector& startPoint,
    const double cylindricalR) const
{
  std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector> candidatePoints = atCylindricalR(cylindricalR);
  return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);
}

double GeneralizedCircle::arcLengthBetween(const ROOT::Math::XYVector& from, const ROOT::Math::XYVector& to) const
{
  EForwardBackward lengthSign = isForwardOrBackwardOf(from, to);
  if (not NForwardBackward::isValid(lengthSign)) return NAN;

  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == EForwardBackward::c_Unknown) lengthSign = EForwardBackward::c_Forward;

  ROOT::Math::XYVector closestAtFrom = closest(from);
  ROOT::Math::XYVector closestAtTo = closest(to);
  double directDistance = VectorUtil::Distance(closestAtFrom, closestAtTo);

  return static_cast<double>(lengthSign) * arcLengthFactor(directDistance) * directDistance;
}

double GeneralizedCircle::arcLengthTo(const ROOT::Math::XYVector& to) const
{
  const ROOT::Math::XYVector from = perigee();

  EForwardBackward lengthSign = isForwardOrBackwardOf(from, to);
  if (not NForwardBackward::isValid(lengthSign)) return NAN;

  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == EForwardBackward::c_Unknown) lengthSign = EForwardBackward::c_Forward;

  const ROOT::Math::XYVector& closestAtFrom = from;
  ROOT::Math::XYVector closestAtTo = closest(to);
  double directDistance = VectorUtil::Distance(closestAtFrom, closestAtTo);

  return static_cast<double>(lengthSign) * arcLengthFactor(directDistance) * directDistance;
}

double GeneralizedCircle::arcLengthToCylindricalR(const double cylindricalR) const
{
  // Slight trick here
  // Since the sought point is on the helix we treat it as the perigee
  // and the origin as the point to extrapolate to.
  // We know the distance of the origin to the circle, which is just d0
  // The direct distance from the origin to the imaginary perigee is just the given cylindricalR.
  const double dr = d0();
  const double directDistance =
    sqrt((cylindricalR + dr) * (cylindricalR - dr) / (1 + dr * omega()));
  const double arcLength = arcLengthFactor(directDistance) * directDistance;
  return arcLength;
}

double GeneralizedCircle::arcLengthFactor(const double directDistance, const double curvature)
{
  double x = directDistance * curvature / 2.0;
  return asinc(x);
}

double GeneralizedCircle::distance(const ROOT::Math::XYVector& point) const
{
  const double fastD = fastDistance(point);
  return distance(fastD);
}

double GeneralizedCircle::distance(const double fastDistance) const
{
  if (fastDistance == 0.0 or isLine()) {
    // special case for unfitted state
    // and line
    return fastDistance;
  } else {

    const double a = n3();
    const double b = 1;
    const double c = -fastDistance;

    std::pair<double, double> distance12 = solveQuadraticABC(a, b, c);

    // take the small solution which has always the same sign of the fastDistance
    return distance12.second;
  }
}

std::pair<ROOT::Math::XYVector, ROOT::Math::XYVector>
GeneralizedCircle::intersections(const GeneralizedCircle& generalizedCircle) const
{
  const double m0 = generalizedCircle.n0();
  const ROOT::Math::XYVector& m12 = generalizedCircle.n12();
  const double m3 = generalizedCircle.n3();

  const double loc_n0 = this->n0();
  const ROOT::Math::XYVector& loc_n12 = this->n12();
  const double loc_n3 = this->n3();

  ROOT::Math::XYVector unitC = loc_n12 * m3 - m12 * loc_n3;
  double absC = unitC.R();
  if (absC != 0.0) {
    unitC *= (1. / absC);
  }

  double xParallel = (m0 * loc_n3 - m3 * loc_n0) / absC;

  // Use symmetric solution and use all input parameters
  ROOT::Math::XYVector mn12 = loc_n12 + m12;
  double mn12Parallel = VectorUtil::unnormalizedParallelComp(unitC, mn12);
  double mn12Orthogonal = VectorUtil::unnormalizedOrthogonalComp(unitC, mn12);

  double a = m3 + loc_n3;
  double b = mn12Orthogonal;
  double c = (a * xParallel + mn12Parallel) * xParallel + m0 + loc_n0;

  std::pair<double, double> xOrthogonal = solveQuadraticABC(a, b, c);

  return std::make_pair(VectorUtil::compose(unitC, xParallel, xOrthogonal.first),
                        VectorUtil::compose(unitC, xParallel, xOrthogonal.second));
}

ROOT::Math::XYVector GeneralizedCircle::atArcLength(const double arcLength) const
{
  double chi = arcLength * curvature();
  double chiHalf = chi / 2.0;

  double atX = arcLength * sinc(chiHalf) * sin(chiHalf) + impact();
  double atY = -arcLength * sinc(chi);
  return VectorUtil::compose(-VectorUtil::unit(n12()), atX, atY);
}

std::ostream& TrackingUtilities::operator<<(std::ostream& output, const GeneralizedCircle& circle)
{
  if (circle.isLine()) {
    output << "Line support point = " << circle.perigee();
    return output;
  } else {
    output << "CircleCenter = " << circle.center() << ", Radius = " << circle.absRadius();
    return output;
  }
}
