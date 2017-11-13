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

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>
#include <tracking/trackFindingCDC/geometry/SZLine.h>
#include <tracking/trackFindingCDC/geometry/SZParameters.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <boost/math/tools/minima.hpp>

#include <utility>
#include <algorithm>
#include <limits>
#include <ostream>
#include <cstdint>

using namespace Belle2;
using namespace TrackFindingCDC;

double Helix::arcLength2DToClosest(const Vector3D& point, bool firstPeriod) const
{
  // The point may happen to lie in the center of the helix.
  double d0 = circleXY().distance(point.xy());
  double denominator = 1 + curvatureXY() * d0;
  if (denominator == 0) {
    // When this happens we can optimise the z distance for the closest point
    double arcLength2D = (point.z() - z0()) / tanLambda();
    if (not std::isfinite(arcLength2D)) {
      return 0.0;
    } else {
      return arcLength2D;
    }
  }

  // First approximation optimising the xy distance.
  double arcLength2D = circleXY().arcLengthTo(point.xy());
  // In case the helix is a flat circle with no extend into z this is the actual solution
  if (tanLambda() == 0) {
    return arcLength2D;
  }

  double deltaZ = point.z() - szLine().map(arcLength2D);
  if (not firstPeriod) {
    if (fabs(deltaZ) > zPeriod() / 2) {
      double newDeltaZ = std::remainder(deltaZ, zPeriod());
      double nPeriodShift = (deltaZ - newDeltaZ) / zPeriod();
      arcLength2D += nPeriodShift * perimeterXY();
      deltaZ = newDeltaZ;
    }
  }

  using boost::math::tools::brent_find_minima;

  auto distance3D = [this, &point](const double & s) -> double {
    Vector3D pos = atArcLength2D(s);
    return pos.distance(point);
  };

  double searchWidth = std::fmin(perimeterXY(), 2 * deltaZ / tanLambda());

  double lowerS = arcLength2D - searchWidth;
  double upperS = arcLength2D + searchWidth;

  int bits = std::numeric_limits<double>::digits;
  boost::uintmax_t nMaxIter = 100;

  std::pair<double, double> sBounds = brent_find_minima(distance3D, lowerS, upperS, bits, nMaxIter);

  // Stopped before iterations were exhausted?
  // bool converged = nMaxIter > 0;

  double firstDistance = distance3D(sBounds.first);
  double secondDistance = distance3D(sBounds.second);
  if (firstDistance < secondDistance) {
    return sBounds.first;
  } else {
    return sBounds.second;
  }
}

HelixJacobian Helix::passiveMoveByJacobian(const Vector3D& by) const
{
  // Fills the upper left 3x3 corner.
  PerigeeJacobian perigeeJacobian = circleXY().passiveMoveByJacobian(by.xy());
  SZJacobian szJacobian = SZUtil::identity();
  HelixJacobian jacobian = HelixUtil::stackBlocks(perigeeJacobian, szJacobian);

  double curv = curvatureXY();
  double tanL = tanLambda();
  double sArc = circleXY().arcLengthTo(by.xy());

  using namespace NHelixParameterIndices;
  jacobian(c_Z0, c_Curv) = tanL * (jacobian(c_Phi0, c_Curv) - sArc) / curv;
  jacobian(c_Z0, c_Phi0) = tanL * (jacobian(c_Phi0, c_Phi0) - 1.) / curv;
  jacobian(c_Z0, c_I) = tanL * jacobian(c_Phi0, c_I) / curv;
  jacobian(c_Z0, c_TanL) = sArc;

  return jacobian;
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const Helix& helix)
{
  return output << "Helix("
         << "curv=" << helix.curvatureXY() << ","
         << "phi0=" << helix.phi0() << ","
         << "impact=" << helix.impactXY() << ","
         << "tanL=" << helix.tanLambda() << ","
         << "z0=" << helix.z0() << ")";
}
