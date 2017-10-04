/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/ParameterLine2D.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

ParameterLine2D ParameterLine2D::touchingCircles(const Vector2D& fromCenter,
                                                 const double fromSignedRadius,
                                                 const Vector2D& toCenter,
                                                 const double toSignedRadius)
{
  Vector2D connecting = toCenter - fromCenter;

  // Normalize to the coordinate system vector, but keep the original norm
  const double norm = connecting.normalize();

  double kappa = (fromSignedRadius - toSignedRadius) / norm;
  double cokappa = sqrt(1 - kappa * kappa);

  Vector2D fromPos = Vector2D(connecting, kappa * fromSignedRadius, cokappa * fromSignedRadius);
  fromPos += fromCenter;

  Vector2D toPos = Vector2D(connecting, kappa * toSignedRadius, cokappa * toSignedRadius);
  toPos += toCenter;

  return ParameterLine2D::throughPoints(fromPos, toPos);
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const ParameterLine2D& line)
{
  output << "ParameterLine2D(" << line.support() << "," << line.tangential() << ")";
  return output;
}
