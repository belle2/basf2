/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/Line2D.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Vector2D Line2D::intersection(const Line2D& line) const
{

  double determinant = n12().cross(line.n12());
  Vector2D result(-n0() * line.n2() + line.n0() * n2(), n0() * line.n1() - line.n0() * n1());

  result /= determinant;
  return result;
}
