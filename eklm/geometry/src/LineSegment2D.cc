/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/geometry/LineSegment2D.h>

using namespace Belle2;

EKLM::LineSegment2D::LineSegment2D(const HepGeom::Point3D<double>& point1,
                                   const HepGeom::Point3D<double>& point2) :
  Line2D(point1.x(), point1.y(),
         point2.x() - point1.x(), point2.y() - point1.y())
{
}

EKLM::LineSegment2D::~LineSegment2D()
{
}

int EKLM::LineSegment2D::
findIntersection(const Circle2D& circle,
                 HepGeom::Point3D<double> intersections[2]) const
{
  int n;
  double t[2];
  n = Line2D::findIntersection(circle, intersections, t);
  if (n == 2) {
    if (tWithinSegment(t[0])) {
      if (tWithinSegment(t[1]))
        return 2;
      else
        return 1;
    } else {
      if (tWithinSegment(t[1])) {
        intersections[0] = intersections[1];
        t[0] = t[1];
        return 1;
      } else
        return 0;
    }
  }
  if (n == 1) {
    if (tWithinSegment(t[0]))
      return 1;
    else
      return 0;
  }
  return 0;
}

bool EKLM::LineSegment2D::tWithinSegment(double t) const
{
  return (t >= 0 && t <= 1);
}

