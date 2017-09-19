/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/GeneralizedCircle.h>

#include <tracking/trackFindingCDC/geometry/Circle2D.h>
#include <tracking/trackFindingCDC/geometry/Line2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/EForwardBackward.h>
#include <tracking/trackFindingCDC/numerics/ERotation.h>

#include <tracking/trackFindingCDC/numerics/SpecialFunctions.h>
#include <tracking/trackFindingCDC/numerics/Quadratic.h>

#include <ostream>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

GeneralizedCircle::GeneralizedCircle()
  : m_n3(0.0)
  , m_n12(0.0, 0.0)
  , m_n0(0.0)
{
}

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

GeneralizedCircle::GeneralizedCircle(const double n0, const Vector2D& n12, const double n3)
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
  , m_n0((circle.center().normSquared() - circle.radiusSquared()) * m_n3)
{
}

GeneralizedCircle GeneralizedCircle::fromCenterAndRadius(const Vector2D& center,
                                                         const double absRadius,
                                                         const ERotation orientation)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setCenterAndRadius(center, absRadius, orientation);
  return generalizedCircle;
}

GeneralizedCircle GeneralizedCircle::fromPerigeeParameters(const double curvature,
                                                           const Vector2D& tangential,
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

void GeneralizedCircle::setCenterAndRadius(const Vector2D& center,
                                           const double absRadius,
                                           const ERotation orientation)
{
  double curvature = orientation / fabs(absRadius);
  setN0((center.normSquared() - absRadius * absRadius) * curvature / 2.0);
  setN1(-center.x() * curvature);
  setN2(-center.y() * curvature);
  setN3(curvature / 2.0);
  normalize(); // the call to normalize should be superfluous
}

void GeneralizedCircle::setPerigeeParameters(const double curvature,
                                             const Vector2D& tangential,
                                             const double impact)
{
  double n0 = impact * (impact * curvature / 2.0 + 1.0);
  Vector2D n12 = -tangential.orthogonal() * (1 + curvature * impact);
  double n3 = curvature / 2.0;
  setN(n0, n12, n3);
}

Vector2D GeneralizedCircle::closest(const Vector2D& point) const
{
  if (fastDistance(point) == 0) return point;

  // solve the minization | point - pointOnCirlce | ^2 subjected to the on circle constraint
  //                       1.0                   * m_n0 +
  //                       point.x()             * m_n1 +
  //                       point.y()             * m_n2 +
  //                       point.cylindricalRSquared() * m_n3 == 0
  // solved with a lagrangian muliplicator for the constraint

  Vector2D gradientAtPoint = gradient(point);
  Vector2D coordinateSystem = gradientAtPoint.unit();

  // component of closest approach orthogonal to coordinateSystem
  double closestOrthogonal = n12().cross(point) / gradientAtPoint.norm();

  // component of closest approach parallel to coordinateSystem - two solutions expected
  double nOrthogonal = n12().unnormalizedOrthogonalComp(coordinateSystem);
  double nParallel = n12().unnormalizedParallelComp(coordinateSystem);

  double closestParallel = 0.0;
  if (isLine()) {
    closestParallel = -(nOrthogonal * closestOrthogonal + n0()) / nParallel;

  } else {
    const double a = n3();
    const double b = nParallel;
    const double c = n0() + (nOrthogonal + n3() * closestOrthogonal) * closestOrthogonal;

    const std::pair<double, double> closestParallel12 = solveQuadraticABC(a, b, c);

    // take the solution with smaller distance to point
    const double pointParallel = point.unnormalizedParallelComp(coordinateSystem);

    const double criterion1 =
      closestParallel12.first * (closestParallel12.first - 2 * pointParallel);
    const double criterion2 =
      closestParallel12.second * (closestParallel12.second - 2 * pointParallel);

    closestParallel = criterion1 < criterion2 ? closestParallel12.first : closestParallel12.second;
  }
  return Vector2D::compose(coordinateSystem, closestParallel, closestOrthogonal);
}

Vector2D GeneralizedCircle::perigee() const
{
  Vector2D result(n12());
  result.setCylindricalR(-impact());
  return result;
}

Vector2D GeneralizedCircle::apogee() const
{
  Vector2D result(n12());
  result.setCylindricalR(-impact() - 2 * radius());
  return result;
}

Vector2D GeneralizedCircle::chooseNextForwardOf(const Vector2D& start,
                                                const Vector2D& end1,
                                                const Vector2D& end2) const
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
        return Vector2D(NAN, NAN);
      } else {
        return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
      }
    }
  }
  return Vector2D(NAN, NAN); // just avoid a compiler warning
}

std::pair<Vector2D, Vector2D> GeneralizedCircle::atCylindricalR(const double cylindricalR) const
{
  // extraploted to r
  // solve
  //  n0 + n1*x + n2*y + n3*r*r == 0
  //  and r = cylindricalR
  // search for x and y

  // solve the equation in a coordinate system parallel and orthogonal to the reduced circle center
  const Vector2D nUnit = n12().unit();

  // parallel component
  const double sameCylindricalRParallel = -(n0() + n3() * square(cylindricalR)) / n12().norm();

  // orthogonal component
  const double sameCylindricalROrthogonal = sqrt(square(cylindricalR) - square(sameCylindricalRParallel));

  /// Two versions in this case
  Vector2D sameCylindricalR1 =
    Vector2D::compose(nUnit, sameCylindricalRParallel, -sameCylindricalROrthogonal);

  Vector2D sameCylindricalR2 =
    Vector2D::compose(nUnit, sameCylindricalRParallel, sameCylindricalROrthogonal);

  std::pair<Vector2D, Vector2D> result(sameCylindricalR1, sameCylindricalR2);
  return result;
}

Vector2D GeneralizedCircle::atCylindricalRForwardOf(const Vector2D& startPoint,
                                                    const double cylindricalR) const
{
  std::pair<Vector2D, Vector2D> candidatePoints = atCylindricalR(cylindricalR);
  return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);
}

double GeneralizedCircle::arcLengthBetween(const Vector2D& from, const Vector2D& to) const
{
  EForwardBackward lengthSign = isForwardOrBackwardOf(from, to);
  if (not NForwardBackward::isValid(lengthSign)) return NAN;

  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == EForwardBackward::c_Unknown) lengthSign = EForwardBackward::c_Forward;

  Vector2D closestAtFrom = closest(from);
  Vector2D closestAtTo = closest(to);
  double directDistance = closestAtFrom.distance(closestAtTo);

  return lengthSign * arcLengthFactor(directDistance) * directDistance;
}

double GeneralizedCircle::arcLengthTo(const Vector2D& to) const
{
  const Vector2D from = perigee();

  EForwardBackward lengthSign = isForwardOrBackwardOf(from, to);
  if (not NForwardBackward::isValid(lengthSign)) return NAN;

  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == EForwardBackward::c_Unknown) lengthSign = EForwardBackward::c_Forward;

  const Vector2D& closestAtFrom = from;
  Vector2D closestAtTo = closest(to);
  double directDistance = closestAtFrom.distance(closestAtTo);

  return lengthSign * arcLengthFactor(directDistance) * directDistance;
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

double GeneralizedCircle::distance(const Vector2D& point) const
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

std::pair<Vector2D, Vector2D>
GeneralizedCircle::intersections(const GeneralizedCircle& generalizedCircle) const
{
  const double m0 = generalizedCircle.n0();
  const Vector2D& m12 = generalizedCircle.n12();
  const double m3 = generalizedCircle.n3();

  const double n0 = this->n0();
  const Vector2D& n12 = this->n12();
  const double n3 = this->n3();

  Vector2D unitC = n12 * m3 - m12 * n3;
  double absC = unitC.normalize();

  double xParallel = (m0 * n3 - m3 * n0) / absC;

  // Use symmetric solution and use all input parameters
  Vector2D mn12 = n12 + m12;
  double mn12Parallel = unitC.unnormalizedParallelComp(mn12);
  double mn12Orthogonal = unitC.unnormalizedOrthogonalComp(mn12);

  double a = m3 + n3;
  double b = mn12Orthogonal;
  double c = (a * xParallel + mn12Parallel) * xParallel + m0 + n0;

  std::pair<double, double> xOrthogonal = solveQuadraticABC(a, b, c);

  return std::make_pair(Vector2D::compose(unitC, xParallel, xOrthogonal.first),
                        Vector2D::compose(unitC, xParallel, xOrthogonal.second));
}

Vector2D GeneralizedCircle::atArcLength(const double arcLength) const
{
  double chi = arcLength * curvature();
  double chiHalf = chi / 2.0;

  double atX = arcLength * sinc(chiHalf) * sin(chiHalf) + impact();
  double atY = -arcLength * sinc(chi);
  return Vector2D::compose(-n12().unit(), atX, atY);
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const GeneralizedCircle& circle)
{
  if (circle.isLine()) {
    output << "Line support point = " << circle.perigee();
    return output;
  } else {
    output << "CircleCenter = " << circle.center() << ", Radius = " << circle.absRadius();
    return output;
  }
}
