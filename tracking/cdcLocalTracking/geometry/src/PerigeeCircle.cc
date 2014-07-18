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
#include <boost/math/special_functions/sinc.hpp>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PerigeeCircle)



PerigeeCircle::PerigeeCircle() : GeneralizedCircle()
{
  setNull();
}


PerigeeCircle::PerigeeCircle(const GeneralizedCircle& n0123,
                             const FloatType& curvature,
                             const FloatType& tangentialPhi,
                             const Vector2D& tangential,
                             const FloatType& impact) :
  GeneralizedCircle(n0123),
  m_curvature(curvature),
  m_tangentialPhi(tangentialPhi),
  m_tangential(tangential),
  m_impact(impact)
{
  /// Nothing to do here
}



PerigeeCircle::PerigeeCircle(const Line2D& n012) :
  GeneralizedCircle(n012)
{
  receivePerigeeParameters();
}



PerigeeCircle::PerigeeCircle(const GeneralizedCircle& n0123) :
  GeneralizedCircle(n0123)
{
  receivePerigeeParameters();
}



PerigeeCircle::PerigeeCircle(const Circle2D& circle)
{
  setCenterAndRadius(circle.center(), circle.absRadius(), circle.orientation());
}



PerigeeCircle PerigeeCircle::fromN(const FloatType& n0,
                                   const FloatType& n1,
                                   const FloatType& n2,
                                   const FloatType& n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n1, n2, n3);
  return circle;
}



PerigeeCircle PerigeeCircle::fromN(const FloatType& n0,
                                   const Vector2D& n12,
                                   const FloatType& n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n12, n3);
  return circle;
}



PerigeeCircle PerigeeCircle::fromCenterAndRadius(const Vector2D& center,
                                                 const FloatType& absRadius,
                                                 const CCWInfo& orientation)
{
  PerigeeCircle circle;
  circle.setCenterAndRadius(center, absRadius, orientation);
  return circle;
}



PerigeeCircle PerigeeCircle::fromPerigeeParameters(const FloatType& curvature,
                                                   const Vector2D& tangential,
                                                   const FloatType& impact)
{
  PerigeeCircle circle;
  circle.setPerigeeParameters(curvature, tangential.unit(), impact);
  return circle;
}



PerigeeCircle PerigeeCircle::fromPerigeeParameters(const FloatType& curvature,
                                                   const FloatType& tangentialPhi,
                                                   const FloatType& impact)
{
  PerigeeCircle circle;
  circle.setPerigeeParameters(curvature, tangentialPhi, impact);
  return circle;
}



Vector2D PerigeeCircle::atPerpS(const FloatType& perpS) const
{
  FloatType atX =  perpS * cosc(perpS * curvature()) + impact();
  FloatType atY =  -perpS * sinc_pi(perpS * curvature());
  return Vector2D::compose(-n12().unit(), atX, atY);
}



TMatrixD PerigeeCircle::passiveMoveByJacobian(const Vector2D& by) const
{
  TMatrixD jacobian(3, 3);
  passiveMoveByJacobian(by, jacobian);
  return jacobian;
}



void PerigeeCircle::passiveMoveByJacobian(const Vector2D& by, TMatrixD& jacobian) const
{

  // In this frame of reference we have d=0,  phi= + or - PI
  Vector2D coordinateVector = tangential();

  //Vector2D delta = perigee() - by;
  Vector2D delta = by - perigee();

  FloatType deltaParallel = coordinateVector.fastParallelComp(delta);
  FloatType deltaOrthogonal = coordinateVector.fastOrthogonalComp(delta);

  FloatType halfA = fastDistance(by);
  FloatType A = 2 * halfA;

  //B2INFO("A = " << A);
  //B2INFO("A = " << 2 * deltaOrthogonal + curvature() * delta.normSquared());

  Vector2D CB = gradient(by).orthogonal();
  //FloatType C = CB.first();
  //FloatType B = CB.second();

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

  jacobian(iCurv, iCurv) = 1;
  jacobian(iCurv, iPhi0) = 0;
  jacobian(iCurv, iI) = 0;

  jacobian(iPhi0, iCurv) = xi * deltaParallel;
  jacobian(iPhi0, iPhi0) = xi * u * nu;
  jacobian(iPhi0, iI) = -xi * curvature() * curvature() * deltaParallel;

  jacobian(iI, iCurv) = mu * zeta - lambda * A;
  jacobian(iI, iPhi0) = 2 * mu * u * deltaParallel;
  jacobian(iI, iI) = 2 * mu * nu;

}
