/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/Helix.h>

#include <tracking/trackFindingCDC/numerics/SinEqLine.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

double Helix::arcLength2DToClosest(const Vector3D& point) const
{
  // TODO: Introduce special case for curvatureXY == 0


  double byArcLength2D = circleXY().arcLengthTo(point.xy());

  // Handle z coordinate
  double transformedZ0 = byArcLength2D * tanLambda() + z0();
  double deltaZ = point.z() - transformedZ0;
  // double iPeriod = floor(deltaZ / zPeriod());

  // Sign ?
  //ERotation ccwInfo = circleXY().orientation();
  //if (ccwInfo != ERotation::c_CounterClockwise and ccwInfo != ERotation::c_Clockwise) return NAN;

  //double d0 = ccwInfo * circleXY().distance(point.xy());
  double d0 = circleXY().distance(point.xy());

  double denominator = 1 + curvatureXY() * d0;
  //B2INFO("denominator = " << denominator);
  if (denominator == 0) {
    return deltaZ / tanLambda() + byArcLength2D;
  }

  double slope = - tanLambda() * tanLambda() / denominator;
  double intercept = - tanLambda() * curvatureXY() * deltaZ / denominator;

  // B2INFO("slope = " << slope);
  // B2INFO("intercept = " << intercept);

  SinEqLine sinEqLineSolver(slope, intercept);

  int iHalfPeriod = sinEqLineSolver.getIHalfPeriod(deltaZ / tanLambda() * curvatureXY());

  // B2INFO("iHalfPeriod : " << iHalfPeriod);
  // B2INFO("Basic solution : " << sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(-1));

  // There are 4 candidate solutions
  // Note: Two of them are local maxima of the distance to the helix and could be abolished before further consideration as an optimization.
  double solutions[4] = { NAN, NAN, NAN, NAN };

  solutions[0] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod - 2);
  solutions[1] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod - 1);
  solutions[2] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod);
  solutions[3] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod + 1);

  double distances[4] = { NAN, NAN, NAN, NAN };

  double smallestDistance = NAN;
  int iSmallestSol = 0;

  for (int iSol = 0; iSol < 4; ++iSol) {
    distances[iSol] = -2.0 * (1.0 + d0 * curvatureXY()) * cos(solutions[iSol]) + tanLambda() * tanLambda() * solutions[iSol] *
                      solutions[iSol] + 2 * tanLambda() * curvatureXY() * deltaZ * solutions[iSol];
    // B2INFO("Solution : " << iSol);
    // B2INFO("arcLength * curvature = " << solutions[iSol]);
    // B2INFO("distance = " << distances[iSol]);

    if (not std::isnan(distances[iSol]) and not(smallestDistance < distances[iSol])) {
      smallestDistance = distances[iSol];
      iSmallestSol = iSol;
    }
    // B2INFO("smallestDistance = " << smallestDistance);

  }

  // B2INFO("Smallest solution : " << iSmallestSol);
  // B2INFO("arcLength * curvature = " << solutions[iSmallestSol]);
  // B2INFO("distance = " << distances[iSmallestSol]);

  double curvatureXYTimesArcLength2D = solutions[iSmallestSol] ;

  double arcLength2D = curvatureXYTimesArcLength2D / curvatureXY();

  // Correct for the periods off set before
  arcLength2D += byArcLength2D;
  return arcLength2D;

}

HelixJacobian Helix::passiveMoveByJacobian(const Vector3D& by) const
{
  // Fills the upper left 3x3 corner.
  PerigeeJacobian perigeeJacobian = circleXY().passiveMoveByJacobian(by.xy());
  SZJacobian szJacobian = SZUtil::identity();
  HelixJacobian jacobian = JacobianMatrixUtil::stackBlocks(perigeeJacobian, szJacobian);

  double curv = curvatureXY();
  double m = tanLambda();
  double sArc = circleXY().arcLengthTo(by.xy());

  using namespace NHelixParameterIndices;
  jacobian(c_Z0, c_Curv) = m * (jacobian(c_Phi0, c_Curv) - sArc) / curv;
  jacobian(c_Z0, c_Phi0) = m * (jacobian(c_Phi0, c_Phi0) - 1.) / curv;
  jacobian(c_Z0, c_I)    = m *  jacobian(c_Phi0, c_I) / curv;
  jacobian(c_Z0, c_TanL) = sArc;

  return jacobian;
}
