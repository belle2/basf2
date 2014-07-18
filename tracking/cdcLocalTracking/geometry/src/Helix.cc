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
#include "../include/CovarianceMatrixIndices.h"

using namespace std;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PerigeeCircle)

TMatrixD Helix::passiveMoveByJacobian(const Vector3D& by) const
{
  TMatrixD jacobian(5, 5);
  jacobian.UnitMatrix();

  // Fills the upper left 3x3 corner.
  circleXY().passiveMoveByJacobian(by.xy(), jacobian);

  FloatType curv = curvatureXY();
  FloatType m = szSlope();
  FloatType sArc = circleXY().arcLengthTo(by.xy());

  jacobian(4, 0) = m * (jacobian(1, 0) - sArc) / curv;
  jacobian(4, 1) = m * (jacobian(1, 1) - 1.) / curv;
  jacobian(4, 2) = m * jacobian(1, 2) / curv;
  jacobian(4, 3) = sArc;

  return jacobian;
}

