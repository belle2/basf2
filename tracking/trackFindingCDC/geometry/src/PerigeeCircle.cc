/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>

#include <framework/logging/Logger.h>

#include <boost/math/tools/precision.hpp>
#include <cmath>
#include <boost/math/special_functions/sinc.hpp>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace TrackFindingCDC;




PerigeeCircle::PerigeeCircle() : GeneralizedCircle()
{
  invalidate();
}

PerigeeCircle::PerigeeCircle(const double curvature,
                             const Vector2D& tangential,
                             const double impact) :
  GeneralizedCircle(GeneralizedCircle::fromPerigeeParameters(curvature, tangential, impact)),
  m_curvature(curvature),
  m_tangentialPhi(tangential.phi()),
  m_tangential(tangential),
  m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(const double curvature,
                             const double tangentialPhi,
                             const double impact) :
  GeneralizedCircle(GeneralizedCircle::fromPerigeeParameters(curvature, tangentialPhi, impact)),
  m_curvature(curvature),
  m_tangentialPhi(tangentialPhi),
  m_tangential(Vector2D::Phi(tangentialPhi)),
  m_impact(impact)
{
}


PerigeeCircle::PerigeeCircle(const TVectorD& parameters) :
  PerigeeCircle(parameters(iCurv), parameters(iPhi0), parameters(iI))
{
}

PerigeeCircle::PerigeeCircle(const GeneralizedCircle& n0123,
                             const double curvature,
                             const double tangentialPhi,
                             const Vector2D& tangential,
                             const double impact) :
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



PerigeeCircle PerigeeCircle::fromN(const double n0,
                                   const double n1,
                                   const double n2,
                                   const double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n1, n2, n3);
  return circle;
}



PerigeeCircle PerigeeCircle::fromN(const double n0,
                                   const Vector2D& n12,
                                   const double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n12, n3);
  return circle;
}



PerigeeCircle PerigeeCircle::fromCenterAndRadius(const Vector2D& center,
                                                 const double absRadius,
                                                 const CCWInfo orientation)
{
  PerigeeCircle circle;
  circle.setCenterAndRadius(center, absRadius, orientation);
  return circle;
}


Vector2D PerigeeCircle::atArcLength(const double arcLength) const
{
  double chi = arcLength * curvature();
  double chiHalf = chi / 2.0;

  using boost::math::sinc_pi;

  double atX =  arcLength *  sinc_pi(chiHalf) * sin(chiHalf) + impact();
  double atY =  -arcLength * sinc_pi(chi);
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

  double deltaParallel = coordinateVector.unnormalizedParallelComp(delta);
  double deltaOrthogonal = coordinateVector.unnormalizedOrthogonalComp(delta);

  double halfA = fastDistance(by);
  double A = 2 * halfA;

  //B2INFO("A = " << A);
  //B2INFO("A = " << 2 * deltaOrthogonal + curvature() * delta.normSquared());

  Vector2D CB = gradient(by).orthogonal();
  //double C = CB.first();
  //double B = CB.second();

  //B2INFO("B = " << B);
  //B2INFO("C = " << C);

  double u = 1 + curvature() * impact(); //= n12().cylindricalR()

  double U = sqrt(1 + curvature() * A);

  //B2INFO("U = " << U);

  double nu = 1 + curvature() * deltaOrthogonal;

  //B2INFO("nu = " << nu);

  double xi = 1.0 / CB.normSquared();

  //B2INFO("xi = " << xi);

  double lambda = halfA / ((1 + U) * (1 + U) * U);
  double mu = 1.0 / (U * (U + 1)) + curvature() * lambda;

  //B2INFO("lambda = " << lambda);
  //B2INFO("mu = " << mu);

  double zeta = delta.normSquared();

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

double PerigeeCircle::arcLengthToCylindricalR(const double cylindricalR) const
{
  // Slight trick here
  // Since the sought point is on the helix we treat it as the perigee
  // and the origin as the point to extrapolate to.
  // We know the distance of the origin to the circle, which is just d0
  // The direct distance from the origin to the imaginary perigee is just the given cylindricalR.
  const double dr = d0();
  const double directDistance = sqrt((cylindricalR + dr) * (cylindricalR - dr) / (1 + dr * omega()));
  const double arcLength = arcLengthFactor(directDistance) * directDistance;
  return arcLength;
}
