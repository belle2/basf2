/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/SZLine.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Vector2D SZLine::intersection(const SZLine& szLine) const
{
  const double s = -(z0() - szLine.z0()) / (tanLambda() - szLine.tanLambda());
  return Vector2D(s, map(s));
}
