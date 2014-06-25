/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/StandardOriginCircle2D.h"

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(StandardOriginCircle2D)

Vector2D StandardOriginCircle2D::closest(const Vector2D& point) const
{

  if (fastDistance(point) == 0) return point;

  //solve the minization | point - pointOnCirlce | ^2 subjected to the on circle constraint
  //                     1.0                   * n0 +
  //                     point.x()             * n1 +
  //                     point.y()             * n2 +
  //                     point.polarRSquared() * n3 == 0

  //solved with a lagrangian muliplicator for the constraint

  Vector2D gradientAtPoint = gradient(point);
  Vector2D coordinateSystem = gradientAtPoint.unit();

  // component of closest approach orthogonal to coordinateSystem
  FloatType closestOrthogonal =  n12().cross(point) / gradientAtPoint.norm();

  // component of closest approach parallel to coordinateSystem - two solutions expected
  FloatType nOrthogonal = n12().fastOrthogonalComp(coordinateSystem);
  FloatType nParallel = n12().fastParallelComp(coordinateSystem);

  FloatType closestParallel = NAN;
  if (isLine()) {
    closestParallel = - (nOrthogonal * closestOrthogonal + n0()) / nParallel;

  } else {
    const FloatType& a = n3();
    const FloatType& b = nParallel;
    const FloatType c = n0() + nOrthogonal * closestOrthogonal + n3() * closestOrthogonal * closestOrthogonal;

    const pair<FloatType, FloatType> closestParallel12 = solveQuadraticABC(a, b, c);

    //take the solution with smaller distance to point
    const FloatType pointParallel = point.fastParallelComp(coordinateSystem);

    const FloatType criterion1 = closestParallel12.first * (closestParallel12.first - 2 * pointParallel);
    const FloatType criterion2 = closestParallel12.second * (closestParallel12.second - 2 * pointParallel);

    closestParallel = criterion1 < criterion2 ? closestParallel12.first : closestParallel12.second;

  }
  return Vector2D::compose(coordinateSystem, closestParallel, closestOrthogonal);
}




Vector2D StandardOriginCircle2D::chooseNextForwardOf(const Vector2D& start, const Vector2D& end1, const Vector2D& end2) const
{

  FloatType lengthOnCurve1 = lengthOnCurve(start, end1);
  FloatType lengthOnCurve2 = lengthOnCurve(start, end2);

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
      //in this case their is no forward intersection
      if (isLine()) {
        return Vector2D(NAN, NAN);
      } else {
        return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
      }

    }
  }

  return Vector2D(NAN, NAN); //just avoid a compiler warning

}

pair<Vector2D, Vector2D> StandardOriginCircle2D::samePolarR(const FloatType& R) const
{
  // extraploted to r
  // solve
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



FloatType StandardOriginCircle2D::lengthOnCurve(const Vector2D& from, const Vector2D& to) const
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



FloatType StandardOriginCircle2D::arcLengthFactor(const FloatType& directDistance) const
{
  FloatType x = directDistance * signedCurvature() / 2.0;

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



Vector2D StandardOriginCircle2D::samePolarR(const Vector2D& point) const
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
  const FloatType pointOrthogonal = point.fastOrthogonalComp(nUnit);

  //orthoganal component of the solution takes to sign of the orthogonal component of the point
  const FloatType samePolarROrthogonal = sign(pointOrthogonal) * sqrt(R * R - samePolarRParallel * samePolarRParallel);

  //combine parallel and orthogonal components
  return Vector2D::compose(nUnit, samePolarRParallel, samePolarROrthogonal);

}

Vector2D StandardOriginCircle2D::samePolarRForwardOf(const Vector2D& startPoint, const FloatType& polarR) const
{

  pair<Vector2D, Vector2D> candidatePoints = samePolarR(polarR);
  return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);

}




FloatType StandardOriginCircle2D::distance(const Vector2D& point) const
{

  //this is the approximated distance also used for the fit
  //can be correct for second order deviations if nessecary
  const FloatType fastD = fastDistance(point);

  if (fastD == 0.0 or isLine()) {
    //special case for unfitted state
    //and line
    return fastD;
  } else {

    const FloatType a = n3();
    const FloatType b = 1;
    const FloatType c = -fastD;

    std::pair<FloatType, FloatType> distance12 = solveQuadraticABC(a, b, c);

    //take the small solution which has always the same sign of the fastD
    return distance12.second;

  }
}



// TMatrixD StandardOriginCircle2D::passiveMoveByJacobian(const Vector2D& by) const {

//   // In this frame of reference we have d=0,  phi= + or - PI
//   Vector2D coordinateVector = tangential();

//   //Vector2D delta = perigee() - by;
//   Vector2D delta = by - perigee();

//   FloatType deltaParallel =  coordinateVector.fastParallelComp(delta);
//   FloatType deltaOrthogonal =  coordinateVector.fastOrthogonalComp(delta);

//   FloatType halfA =  fastDistance(by);
//   FloatType A = 2 * halfA;

//   B2INFO("A = " << A);
//   B2INFO("A = " << 2 * deltaOrthogonal + signedCurvature() * delta.normSquared() );

//   Vector2D CB = gradient(by).orthogonal();
//   FloatType C = CB.first();
//   FloatType B = CB.second();

//   B2INFO("B = " << B);
//   B2INFO("C = " << C);

//   FloatType u = 1 + signedCurvature() * impact(); //= n12().polarR()

//   FloatType U = sqrt(1 + signedCurvature() * A);

//   B2INFO("U = " << U);


//   FloatType nu = 1 + signedCurvature() * deltaOrthogonal;

//   B2INFO("nu = " << nu);

//   FloatType xi = 1.0 / CB.normSquared();

//   B2INFO("xi = " << xi);

//   FloatType lambda = halfA / ((1 + U) * (1 + U) * U);
//   FloatType mu = 1.0 / (U * (U + 1)) + signedCurvature() * lambda;

//   B2INFO("lambda = " << lambda);
//   B2INFO("mu = " << mu);

//   FloatType zeta = delta.normSquared();

//   B2INFO("zeta = " << zeta);

//   TMatrixD jacobian(3, 3);
//   jacobian[0][0] = 1;
//   jacobian[0][1] = 0;
//   jacobian[0][2] = 0;

//   jacobian[1][0] = xi * deltaParallel;
//   jacobian[1][1] = xi * u * nu;
//   jacobian[1][2] = -xi * signedCurvature() * signedCurvature() * deltaParallel;

//   jacobian[2][0] = mu * zeta - lambda * A;
//   jacobian[2][1] = 2 * mu * u * deltaParallel;
//   jacobian[2][2] = 2 * mu * nu;

//   return jacobian;
// }




// TMatrixD StandardOriginCircle2D::passiveMoveCovarianceBy(const Vector2D& by, const TMatrixD& perigeeCovariance) const {


//   TMatrixD jacobian(3, 3);
//   jacobian = passiveMoveByJacobian(by);

//   //result = J * V * J^T
//   TMatrixD left(3, 3);
//   left.Mult(jacobian, perigeeCovariance);

//   TMatrixD result(3, 3);
//   result.MultT(left, jacobian);
//   return result;
// }
