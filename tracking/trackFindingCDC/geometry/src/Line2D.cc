/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
