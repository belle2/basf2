/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PerigeeCircle.h"

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PerigeeCircle)

// Vector2D PerigeeCircle::closest(const Vector2D& point) const
// {

//   if (fastDistance(point) == 0) return point;

//   //solve the minization | point - pointOnCirlce | ^2 subjected to the on circle constraint
//   //                     1.0                   * n0 +
//   //                     point.x()             * n1 +
//   //                     point.y()             * n2 +
//   //                     point.polarRSquared() * n3 == 0

//   //solved with a lagrangian muliplicator for the constraint

//   Vector2D gradientAtPoint = gradient(point);
//   Vector2D coordinateSystem = gradientAtPoint.unit();

//   // component of closest approach orthogonal to coordinateSystem
//   FloatType closestOrthogonal =  n12().cross(point) / gradientAtPoint.norm();

//   // component of closest approach parallel to coordinateSystem - two solutions expected
//   FloatType nOrthogonal = n12().fastOrthogonalComp(coordinateSystem);
//   FloatType nParallel = n12().fastParallelComp(coordinateSystem);

//   FloatType closestParallel = NAN;
//   if (isLine()) {
//     closestParallel = - (nOrthogonal * closestOrthogonal + n0()) / nParallel;

//   } else {
//     const FloatType& a = n3();
//     const FloatType& b = nParallel;
//     const FloatType c = n0() + nOrthogonal * closestOrthogonal + n3() * closestOrthogonal * closestOrthogonal;

//     const pair<FloatType, FloatType> closestParallel12 = solveQuadraticABC(a, b, c);

//     //take the solution with smaller distance to point
//     const FloatType pointParallel = point.fastParallelComp(coordinateSystem);

//     const FloatType criterion1 = closestParallel12.first * (closestParallel12.first - 2 * pointParallel);
//     const FloatType criterion2 = closestParallel12.second * (closestParallel12.second - 2 * pointParallel);

//     closestParallel = criterion1 < criterion2 ? closestParallel12.first : closestParallel12.second;

//   }
//   return Vector2D::compose(coordinateSystem, closestParallel, closestOrthogonal);
// }




// Vector2D PerigeeCircle::chooseNextForwardOf(const Vector2D& start, const Vector2D& end1, const Vector2D& end2) const
// {

//   FloatType lengthOnCurve1 = lengthOnCurve(start, end1);
//   FloatType lengthOnCurve2 = lengthOnCurve(start, end2);

//   if (lengthOnCurve1 >= 0.0) {

//     if (lengthOnCurve2 >= 0.0) {
//       return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
//     } else {
//       return end1;
//     }
//   } else {

//     if (lengthOnCurve2 >= 0.0) {
//       return end2;
//     } else {

//       //both lengths on curve have a negative sign
//       //the candidate with the lesser length on curve wins because of the discontinuaty
//       //unless the generalized circle is a line
//       //in this case their is no forward intersection
//       if (isLine()) {
//         return Vector2D(NAN, NAN);
//       } else {
//         return lengthOnCurve1 < lengthOnCurve2 ? end1 : end2;
//       }

//     }
//   }

//   return Vector2D(NAN, NAN); //just avoid a compiler warning

// }

// pair<Vector2D, Vector2D> PerigeeCircle::samePolarR(const FloatType& R) const
// {
//   // extraploted to r
//   // solve
//   // n0 + n1*x + n2*y + n3*r*r == 0
//   // and r = R
//   //search for x and y

//   //solve the equation in a coordinate system parallel and orthogonal to the reduced circle center
//   const Vector2D nUnit = n12().unit();

//   // parallel component
//   const FloatType samePolarRParallel = -(n0() + n3() * R * R) / n12().norm();

//   //orthogonal component
//   const FloatType samePolarROrthogonal = sqrt(R * R - samePolarRParallel * samePolarRParallel);

//   /// Two versions in this case
//   Vector2D samePolarR1 = Vector2D::compose(nUnit, samePolarRParallel, samePolarROrthogonal);
//   Vector2D samePolarR2 = Vector2D::compose(nUnit, samePolarRParallel, -samePolarROrthogonal);

//   pair<Vector2D, Vector2D> result(samePolarR1, samePolarR2);
//   return result;

// }



// FloatType PerigeeCircle::lengthOnCurve(const Vector2D& from, const Vector2D& to) const
// {
//   ForwardBackwardInfo lengthSign = isForwardOrBackwardOf(from, to);
//   if (lengthSign == INVALID_INFO) return NAN;

//   // Handling the rare case that from and to correspond to opposing points on the circle
//   if (lengthSign == UNKNOWN_INFO) lengthSign = 1;

//   Vector2D closestAtFrom = closest(from);
//   Vector2D closestAtTo = closest(to);
//   FloatType directDistance = closestAtFrom.distance(closestAtTo);

//   return lengthSign * m_standardOriginCircle.arcLengthFactor(directDistance) * directDistance;
// }



// Vector2D PerigeeCircle::samePolarR(const Vector2D& point) const
// {

//   const FloatType R = point.norm();

//   //extraploted to r
//   //solve
//   // reducedCircleParameters(0) + reducedCircleParameters(1)*x + reducedCircleParameters(2)*y + reducedCircleParameters(3)*r*r == 0
//   // and r = R
//   //search for x and y

//   //solve the equation in a coordinate system parallel and orthogonal to the reduced circle center
//   const Vector2D nUnit = n12().unit();

//   // parallel component
//   const FloatType samePolarRParallel = -(n0() + n3() * R * R) / n12().norm();

//   //orthogonal component
//   const FloatType pointOrthogonal = point.fastOrthogonalComp(nUnit);

//   //orthoganal component of the solution takes to sign of the orthogonal component of the point
//   const FloatType samePolarROrthogonal = sign(pointOrthogonal) * sqrt(R * R - samePolarRParallel * samePolarRParallel);

//   //combine parallel and orthogonal components
//   return Vector2D::compose(nUnit, samePolarRParallel, samePolarROrthogonal);

// }

// Vector2D PerigeeCircle::samePolarRForwardOf(const Vector2D& startPoint, const FloatType& polarR) const
// {
//   pair<Vector2D, Vector2D> candidatePoints = samePolarR(polarR);
//   return chooseNextForwardOf(startPoint, candidatePoints.first, candidatePoints.second);
// }



// FloatType PerigeeCircle::distance(const Vector2D& point) const
// {
//   const FloatType fastD = fastDistance(point);

//   if (fastD == 0.0 or isLine()) {
//     //special case for unfitted state
//     //and line
//     return fastD;
//   } else {

//     const FloatType a = n3();
//     const FloatType b = 1;
//     const FloatType c = -fastD;

//     std::pair<FloatType, FloatType> distance12 = solveQuadraticABC(a, b, c);

//     //take the small solution which has always the same sign of the fastD
//     return distance12.second;

//   }
// }



void PerigeeCircle::passiveMoveBy(const Vector2D& by)
{
  FloatType newCurvature = curvature();
  Vector2D newTangential = tangential(by);
  FloatType newImpact = distance(by);

  setPerigeeParameters(newCurvature, newTangential, newImpact);
}
