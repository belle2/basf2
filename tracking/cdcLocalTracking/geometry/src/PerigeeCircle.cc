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




void PerigeeCircle::passiveMoveBy(const Vector2D& by)
{
  FloatType newCurvature = curvature();
  Vector2D newTangential = tangential(by);
  FloatType newImpact = distance(by);

  setPerigeeParameters(newCurvature, newTangential, newImpact);
}

