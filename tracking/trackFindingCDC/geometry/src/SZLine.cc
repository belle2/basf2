/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/SZLine.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Vector2D SZLine::intersection(const SZLine& szLine) const
{
  const double s = -(z0() - szLine.z0()) / (tanLambda() - szLine.tanLambda());
  return Vector2D(s, map(s));
}
