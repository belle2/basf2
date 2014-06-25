/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/UncertainPerigeeCircle.h"

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(UncertainPerigeeCircle)

TMatrixD UncertainPerigeeCircle::passiveMoveByJacobian(const Vector2D& by) const
{

  // In this frame of reference we have d=0,  phi= + or - PI
  Vector2D coordinateVector = tangential();

  //Vector2D delta = perigee() - by;
  Vector2D delta = by - perigee();

  FloatType deltaParallel =  coordinateVector.fastParallelComp(delta);
  FloatType deltaOrthogonal =  coordinateVector.fastOrthogonalComp(delta);

  FloatType halfA =  fastDistance(by);
  FloatType A = 2 * halfA;

  //B2INFO("A = " << A);
  //B2INFO("A = " << 2 * deltaOrthogonal + curvature() * delta.normSquared());

  Vector2D CB = gradient(by).orthogonal();
  FloatType C = CB.first();
  FloatType B = CB.second();

  //B2INFO("B = " << B);
  //B2INFO("C = " << C);

  FloatType u = 1 + curvature() * impact(); //= n12().polarR()

  FloatType U = sqrt(1 + curvature() * A);

  //B2INFO("U = " << U);


  FloatType nu = 1 + curvature() * deltaOrthogonal;

  //B2INFO("nu = " << nu);

  FloatType xi = 1.0 / CB.normSquared();

  //B2INFO("xi = " << xi);

  FloatType lambda = halfA / ((1 + U) * (1 + U) * U);
  FloatType mu = 1.0 / (U * (U + 1)) + curvature() * lambda;

  //B2INFO("lambda = " << lambda);
  //B2INFO("mu = " << mu);

  FloatType zeta = delta.normSquared();

  //B2INFO("zeta = " << zeta);

  TMatrixD jacobian(3, 3);
  jacobian[0][0] = 1;
  jacobian[0][1] = 0;
  jacobian[0][2] = 0;

  jacobian[1][0] = xi * deltaParallel;
  jacobian[1][1] = xi * u * nu;
  jacobian[1][2] = -xi * curvature() * curvature() * deltaParallel;

  jacobian[2][0] = mu * zeta - lambda * A;
  jacobian[2][1] = 2 * mu * u * deltaParallel;
  jacobian[2][2] = 2 * mu * nu;

  return jacobian;
}




TMatrixD UncertainPerigeeCircle::passiveMovedCovarianceBy(const Vector2D& by, const TMatrixD& perigeeCovariance) const
{

  TMatrixD jacobian(3, 3);
  jacobian = passiveMoveByJacobian(by);

  //result = J * V * J^T
  TMatrixD left(3, 3);
  left.Mult(jacobian, perigeeCovariance);

  TMatrixD result(3, 3);
  result.MultT(left, jacobian);
  return result;
}
