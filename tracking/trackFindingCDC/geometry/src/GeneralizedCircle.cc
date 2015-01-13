/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/GeneralizedCircle.h"

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>

#include <boost/math/special_functions/sinc.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(GeneralizedCircle)



GeneralizedCircle::GeneralizedCircle() : m_n3(0.0), m_n12(0.0, 0.0), m_n0(0.0) { ; }



GeneralizedCircle::GeneralizedCircle(const FloatType& n0, const FloatType& n1,
                                     const FloatType& n2, const FloatType& n3) :
  m_n3(n3), m_n12(n1, n2), m_n0(n0)
{
  normalize();
}



GeneralizedCircle::GeneralizedCircle(const FloatType& n0,
                                     const Vector2D& n12,
                                     const FloatType& n3):
  m_n3(n3), m_n12(n12), m_n0(n0)
{
  normalize();
}



GeneralizedCircle::GeneralizedCircle(const Line2D& n012):
  m_n3(0.0), m_n12(n012.n12()), m_n0(n012.n0())
{
  normalize();
}


GeneralizedCircle::GeneralizedCircle(const Circle2D& circle):
  m_n3(1.0 / 2.0 / circle.radius()),
  m_n12(- circle.center().x() * (m_n3 * 2.0)  ,
        - circle.center().y() * (m_n3 * 2.0)),
  m_n0((circle.center().normSquared() - circle.radiusSquared()) * m_n3)
{ ; }


GeneralizedCircle GeneralizedCircle::fromCenterAndRadius(const Vector2D& center,
                                                         const FloatType& absRadius,
                                                         const CCWInfo& orientation)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setCenterAndRadius(center, absRadius, orientation);
  return generalizedCircle;
}

GeneralizedCircle GeneralizedCircle::fromPerigeeParameters(const FloatType& curvature,
                                                           const Vector2D& tangential,
                                                           const FloatType& impact)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setPerigeeParameters(curvature, tangential, impact);
  return generalizedCircle;
}

GeneralizedCircle GeneralizedCircle::fromPerigeeParameters(const FloatType& curvature,
                                                           const FloatType& tangentialPhi,
                                                           const FloatType& impact)
{
  GeneralizedCircle generalizedCircle;
  generalizedCircle.setPerigeeParameters(curvature, tangentialPhi, impact);
  return generalizedCircle;
}






void GeneralizedCircle::setCenterAndRadius(const Vector2D& center,
                                           const FloatType& absRadius,
                                           const CCWInfo& orientation)
{
  FloatType curvature = orientation / fabs(absRadius);
  setN0((center.normSquared() - absRadius * absRadius) * curvature / 2.0);
  setN1(-center.x() * curvature);
  setN2(-center.y() * curvature);
  setN3(curvature / 2.0);
  normalize(); // the call to normalize should be superfluous
}



void GeneralizedCircle::setPerigeeParameters(const FloatType& curvature,
                                             const Vector2D& tangential,
                                             const FloatType& impact)
{
  FloatType n0 = impact * (impact * curvature / 2.0 + 1.0);
  Vector2D n12 = -tangential.orthogonal() * (1 + curvature * impact);
  FloatType n3 = curvature / 2.0;
  setN(n0, n12, n3);
}





Vector2D GeneralizedCircle::closest(const Vector2D& point) const
{

  if (fastDistance(point) == 0) return point;

  //solve the minization | point - pointOnCirlce | ^2 subjected to the on circle constraint
  //                     1.0                   * m_n0 +
  //                     point.x()             * m_n1 +
  //                     point.y()             * m_n2 +
  //                     point.polarRSquared() * m_n3 == 0

  //solved with a lagrangian muliplicator for the constraint

  Vector2D gradientAtPoint = gradient(point);
  Vector2D coordinateSystem = gradientAtPoint.unit();

  // component of closest approach orthogonal to coordinateSystem
  FloatType closestOrthogonal =  n12().cross(point) / gradientAtPoint.norm();

  // component of closest approach parallel to coordinateSystem - two solutions expected
  FloatType nOrthogonal = n12().unnormalizedOrthogonalComp(coordinateSystem);
  FloatType nParallel = n12().unnormalizedParallelComp(coordinateSystem);

  FloatType closestParallel = 0.0;
  if (isLine()) {
    closestParallel = - (nOrthogonal * closestOrthogonal + n0()) / nParallel;

  } else {
    const FloatType& a = n3();
    const FloatType& b = nParallel;
    const FloatType c = n0() + nOrthogonal * closestOrthogonal + n3() * closestOrthogonal * closestOrthogonal;

    const pair<FloatType, FloatType> closestParallel12 = solveQuadraticABC(a, b, c);

    //take the solution with smaller distance to point
    const FloatType pointParallel = point.unnormalizedParallelComp(coordinateSystem);

    const FloatType criterion1 = closestParallel12.first * (closestParallel12.first - 2 * pointParallel);
    const FloatType criterion2 = closestParallel12.second * (closestParallel12.second - 2 * pointParallel);

    closestParallel = criterion1 < criterion2 ? closestParallel12.first : closestParallel12.second;

  }
  return Vector2D::compose(coordinateSystem, closestParallel, closestOrthogonal);
}

Vector2D GeneralizedCircle::perigee() const
{
  Vector2D result(n12());
  result.setPolarR(-impact());
  return result;
}

Vector2D GeneralizedCircle::chooseNextForwardOf(const Vector2D& start, const Vector2D& end1, const Vector2D& end2) const
{

  FloatType lengthOnCurve1 = arcLengthBetween(start, end1);
  FloatType lengthOnCurve2 = arcLengthBetween(start, end2);

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

      //both lengths on curve have a negative sign
      //the candidate with the lesser length on curve wins because of the discontinuaty
      //unless the generalized circle is a line
      //in this case their is no forward intersection with the same polar radius
      if (isLine()) {
        return Vector2D(NAN, NAN);
      } else {
        return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
      }

    }
  }

  return Vector2D(NAN, NAN); //just avoid a compiler warning

}

pair<Vector2D, Vector2D> GeneralizedCircle::samePolarR(const FloatType& R) const
{
  //extraploted to r
  //solve
  // n0 + n1*x + n2*y + n3*r*r == 0
  // and r = R
  //search for x and y

  //solve the equation in a coordinate system parallel and orthogonal to the reduced circle center
  const Vector2D nUnit = n12().unit();

  // parallel component
  const FloatType samePolarRParallel = -(n0() + n3() * R * R) / n12().norm();

  //orthogonal component
  const FloatType samePolarROrthogonal = sqrt(R * R - samePolarRParallel * samePolarRParallel);

  /// Two versions in this case
  Vector2D samePolarR1 = Vector2D::compose(nUnit, samePolarRParallel, samePolarROrthogonal);
  Vector2D samePolarR2 = Vector2D::compose(nUnit, samePolarRParallel, -samePolarROrthogonal);

  pair<Vector2D, Vector2D> result(samePolarR1, samePolarR2);
  return result;

}


Vector2D GeneralizedCircle::samePolarR(const Vector2D& point) const
{
  const FloatType R = point.norm();

  //extraploted to r
  //solve
  // reducedCircleParameters(0) + reducedCircleParameters(1)*x + reducedCircleParameters(2)*y + reducedCircleParameters(3)*r*r == 0
  // and r = R
  //search for x and y

  //solve the equation in a coordinate system parallel and orthogonal to the reduced circle center
  const Vector2D nUnit = n12().unit();

  // parallel component
  const FloatType samePolarRParallel = -(n0() + n3() * R * R) / n12().norm();

  //orthogonal component
  const FloatType pointOrthogonal = point.unnormalizedOrthogonalComp(nUnit);

  //orthoganal component of the solution takes to sign of the orthogonal component of the point
  const FloatType samePolarROrthogonal = sign(pointOrthogonal) * sqrt(R * R - samePolarRParallel * samePolarRParallel);

  //combine parallel and orthogonal components
  return Vector2D::compose(nUnit, samePolarRParallel, samePolarROrthogonal);

}

Vector2D GeneralizedCircle::samePolarRForwardOf(const Vector2D& startPoint, const FloatType& polarR) const
{

  pair<Vector2D, Vector2D> candidatePoints = samePolarR(polarR);
  return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);

}



FloatType GeneralizedCircle::arcLengthBetween(const Vector2D& from, const Vector2D& to) const
{
  ForwardBackwardInfo lengthSign = isForwardOrBackwardOf(from, to);
  if (lengthSign == INVALID_INFO) return NAN;

  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == UNKNOWN_INFO) lengthSign = 1;

  Vector2D closestAtFrom = closest(from);
  Vector2D closestAtTo = closest(to);
  FloatType directDistance = closestAtFrom.distance(closestAtTo);

  return lengthSign * arcLengthFactor(directDistance) * directDistance;
}



FloatType GeneralizedCircle::arcLengthTo(const Vector2D& to) const
{
  const Vector2D from = perigee();

  ForwardBackwardInfo lengthSign = isForwardOrBackwardOf(from, to);
  if (lengthSign == INVALID_INFO) return NAN;

  // Handling the rare case that from and to correspond to opposing points on the circle
  if (lengthSign == UNKNOWN_INFO) lengthSign = 1;

  const Vector2D& closestAtFrom = from;
  Vector2D closestAtTo = closest(to);
  FloatType directDistance = closestAtFrom.distance(closestAtTo);

  return lengthSign * arcLengthFactor(directDistance) * directDistance;

}



FloatType GeneralizedCircle::arcLengthFactor(const FloatType& directDistance) const
{
  FloatType x = directDistance * curvature() / 2.0;

  // Implementation of asin(x)/x
  // Inspired by BOOST's sinc
  BOOST_MATH_STD_USING;

  FloatType const taylor_n_bound = tools::forth_root_epsilon<FloatType>();

  if (abs(x) >= taylor_n_bound) {
    return  asin(x) / x;

  } else {
    // approximation by taylor series in x at 0 up to order 0
    FloatType result = 1.0;

    FloatType const taylor_0_bound = tools::epsilon<FloatType>();
    if (abs(x) >= taylor_0_bound) {
      FloatType x2 = x * x;
      // approximation by taylor series in x at 0 up to order 2
      result += x2 / 6.0;

      FloatType const taylor_2_bound = tools::root_epsilon<FloatType>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        result += x2 * x2 * (3.0 / 40.0);

      }
    }
    return result;
  }
}





FloatType GeneralizedCircle::distance(const Vector2D& point) const
{
  //this is the approximated distance also used for the fit
  //can be correct for second order deviations if nessecary
  const FloatType fastD = fastDistance(point);
  return distance(fastD);
}





FloatType GeneralizedCircle::distance(const FloatType& fastDistance) const
{
  if (fastDistance == 0.0 or isLine()) {
    //special case for unfitted state
    //and line
    return fastDistance;
  } else {

    const FloatType a = n3();
    const FloatType b = 1;
    const FloatType c = -fastDistance;

    std::pair<FloatType, FloatType> distance12 = solveQuadraticABC(a, b, c);

    //take the small solution which has always the same sign of the fastDistance
    return distance12.second;

  }
}



std::pair<Vector2D, Vector2D> GeneralizedCircle::intersections(const GeneralizedCircle& generalizedCircle) const
{

  const FloatType& m0 = generalizedCircle.n0();
  const Vector2D& m12 = generalizedCircle.n12();
  const FloatType& m3 = generalizedCircle.n3();

  const FloatType& n0 = this->n0();
  const Vector2D& n12 = this->n12();
  const FloatType& n3 = this->n3();


  Vector2D unitC = n12 * m3 - m12 * n3;
  FloatType absC = unitC.normalize();

  FloatType xParallel = (m0 * n3 - m3 * n0) / absC;

  // Use symmetric solution and use all input parameters
  Vector2D mn12 = n12 + m12;
  FloatType mn12Parallel = unitC.unnormalizedParallelComp(mn12);
  FloatType mn12Orthogonal = unitC.unnormalizedOrthogonalComp(mn12);

  FloatType a = m3 + n3;
  FloatType b = mn12Orthogonal;
  FloatType c = (a * xParallel + mn12Parallel) * xParallel + m0 + n0;

  pair<FloatType, FloatType> xOrthogonal = solveQuadraticABC(a, b, c);

  return make_pair(Vector2D::compose(unitC, xParallel, xOrthogonal.first),
                   Vector2D::compose(unitC, xParallel, xOrthogonal.second));
}


Vector2D GeneralizedCircle::atArcLength(const FloatType& arcLength) const
{
  FloatType chi = arcLength * curvature();
  FloatType chiHalf = chi / 2.0;

  using boost::math::sinc_pi;

  FloatType atX =  arcLength *  sinc_pi(chiHalf) * sin(chiHalf) + impact();
  FloatType atY =  -arcLength * sinc_pi(chi);
  return Vector2D::compose(-n12().unit(), atX, atY);
}
