/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/geometry/ParameterLine2D.h>

#include <tracking/trackingUtilities/geometry/VectorUtil.h>

#include <ostream>

using namespace Belle2;
using namespace TrackingUtilities;

ParameterLine2D ParameterLine2D::touchingCircles(const ROOT::Math::XYVector& fromCenter,
                                                 const double fromSignedRadius,
                                                 const ROOT::Math::XYVector& toCenter,
                                                 const double toSignedRadius)
{
  ROOT::Math::XYVector connecting = toCenter - fromCenter;

  // Normalize to the coordinate system vector, but keep the original norm
  const double norm = connecting.R();
  if (norm != 0.0) {
    connecting *= (1. / norm);
  }

  double kappa = (fromSignedRadius - toSignedRadius) / norm;
  double cokappa = sqrt(1 - kappa * kappa);

  ROOT::Math::XYVector fromPos = VectorUtil::compose(connecting, kappa * fromSignedRadius, cokappa * fromSignedRadius);
  fromPos += fromCenter;

  ROOT::Math::XYVector toPos = VectorUtil::compose(connecting, kappa * toSignedRadius, cokappa * toSignedRadius);
  toPos += toCenter;

  return ParameterLine2D::throughPoints(fromPos, toPos);
}

std::ostream& TrackingUtilities::operator<<(std::ostream& output, const ParameterLine2D& line)
{
  output << "ParameterLine2D(" << line.support() << "," << line.tangential() << ")";
  return output;
}
