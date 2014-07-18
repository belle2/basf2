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

  jacobian(iZ0, iCurv) = m * (jacobian(iPhi0, iCurv) - sArc) / curv;
  jacobian(iZ0, iPhi0) = m * (jacobian(iPhi0, iPhi0) - 1.) / curv;
  jacobian(iZ0, iI)    = m *  jacobian(iPhi0, iI) / curv;
  jacobian(iZ0, iSZ)   = sArc;

  return jacobian;
}

