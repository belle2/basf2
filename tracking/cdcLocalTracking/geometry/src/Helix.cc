/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/Helix.h"
#include "../include/HelixParameterIndex.h"


#include <tracking/cdcLocalTracking/numerics/SinEqLine.h>

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PerigeeCircle)







FloatType Helix::closestAtPerpS(const Vector3D& point) const
{
  // TODO: Introduce special case for curvatureXY == 0


  FloatType byPerpS = circleXY().lengthOnCurve(perigeeXY(), point.xy());

  // Handle z coordinate
  FloatType transformedZ0 = byPerpS * szSlope() + z0();
  FloatType deltaZ = point.z() - transformedZ0;
  // FloatType iPeriod = floor(deltaZ / zPeriod());

  // Sign ?
  //CCWInfo ccwInfo = circleXY().orientation();
  //if (ccwInfo != CCW and ccwInfo != CW) return NAN;

  //FloatType d0 = ccwInfo * circleXY().distance(point.xy());
  FloatType d0 = circleXY().distance(point.xy());

  FloatType denominator = 1 + curvatureXY() * d0;
  //B2INFO("denominator = " << denominator);
  if (denominator == 0) {
    return deltaZ / szSlope() + byPerpS;
  }

  FloatType slope = - szSlope() * szSlope() / denominator;
  FloatType intercept = - szSlope() * curvatureXY() * deltaZ / denominator;

  // B2INFO("slope = " << slope);
  // B2INFO("intercept = " << intercept);

  SinEqLine sinEqLineSolver(slope, intercept);

  Index iHalfPeriod = sinEqLineSolver.getIHalfPeriod(deltaZ / szSlope() * curvatureXY());

  // B2INFO("iHalfPeriod : " << iHalfPeriod);
  // B2INFO("Basic solution : " << sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(-1));

  // There are 4 candidate solutions
  // Note: Two of them are local maxima of the distance to the helix and could be abolished before further consideration as an optimization.
  FloatType solutions[4] = { NAN, NAN, NAN, NAN };

  solutions[0] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod - 2);
  solutions[1] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod - 1);
  solutions[2] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod);
  solutions[3] = sinEqLineSolver.computeRootLargerThanExtemumInHalfPeriod(iHalfPeriod + 1);

  FloatType distances[4] = { NAN, NAN, NAN, NAN };

  FloatType smallestDistance = NAN;
  Index iSmallestSol = 0;

  for (int iSol = 0; iSol < 4; ++iSol) {
    distances[iSol] = -2.0 * (1.0 + d0 * curvatureXY()) * cos(solutions[iSol]) + szSlope() * szSlope() * solutions[iSol] * solutions[iSol] + 2 * szSlope() * curvatureXY() * deltaZ * solutions[iSol];
    // B2INFO("Solution : " << iSol);
    // B2INFO("perpS * curvature = " << solutions[iSol]);
    // B2INFO("distance = " << distances[iSol]);

    if (not std::isnan(distances[iSol]) and not(smallestDistance < distances[iSol])) {
      smallestDistance = distances[iSol];
      iSmallestSol = iSol;
    }
    // B2INFO("smallestDistance = " << smallestDistance);

  }

  // B2INFO("Smallest solution : " << iSmallestSol);
  // B2INFO("perpS * curvature = " << solutions[iSmallestSol]);
  // B2INFO("distance = " << distances[iSmallestSol]);

  FloatType curvatureXYTimesPerpS = solutions[iSmallestSol] ;

  FloatType perpS = curvatureXYTimesPerpS / curvatureXY();

  // Correct for the periods off set before
  perpS += byPerpS;
  return perpS;

}





TMatrixD Helix::passiveMoveByJacobian(const Vector3D& by) const
{
  TMatrixD jacobian(5, 5);
  jacobian.UnitMatrix();

  // Fills the upper left 3x3 corner.
  circleXY().passiveMoveByJacobian(by.xy(), jacobian);

  FloatType curv = curvatureXY();
  FloatType m = szSlope();
  FloatType sArc = circleXY().arcLengthTo(by.xy());

  jacobian(iZ0, iCurv) = m * (jacobian(iPhi0, iCurv) - sArc) / curv;
  jacobian(iZ0, iPhi0) = m * (jacobian(iPhi0, iPhi0) - 1.) / curv;
  jacobian(iZ0, iI)    = m *  jacobian(iPhi0, iI) / curv;
  jacobian(iZ0, iSZ)   = sArc;

  return jacobian;
}





