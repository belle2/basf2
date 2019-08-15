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
#include <klm/eklm/geometry/LineSegment2D.h>

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
findIntersection(const Line2D& line,
                 HepGeom::Point3D<double>* intersection) const
{
  int n;
  double t[2];
  bool condition;
  n = Line2D::findIntersection(line, intersection, t);
  if (n == 0)
    return 0;
  condition = tWithinRange(t[0]);
  return selectIntersections(intersection, &condition, 1);
}


int EKLM::LineSegment2D::
findIntersection(const LineSegment2D& lineSegment,
                 HepGeom::Point3D<double>* intersection) const
{
  int n;
  double t[2];
  bool condition;
  n = Line2D::findIntersection(lineSegment, intersection, t);
  if (n == 0)
    return 0;
  condition = tWithinRange(t[0]) && lineSegment.tWithinRange(t[1]);
  return selectIntersections(intersection, &condition, 1);
}

int EKLM::LineSegment2D::
findIntersection(const Circle2D& circle,
                 HepGeom::Point3D<double> intersections[2]) const
{
  int i, n;
  double t[2], angles[2];
  bool condition[2];
  n = Line2D::findIntersection(circle, intersections, t, angles);
  for (i = 0; i < n; i++)
    condition[i] = tWithinRange(t[i]);
  return selectIntersections(intersections, condition, n);
}

int EKLM::LineSegment2D::
findIntersection(const Arc2D& arc,
                 HepGeom::Point3D<double> intersections[2]) const
{
  int i, n;
  double t[2], angles[2];
  bool condition[2];
  n = Line2D::findIntersection(arc, intersections, t, angles);
  for (i = 0; i < n; i++)
    condition[i] = tWithinRange(t[i]) && arc.angleWithinRange(angles[i]);
  return selectIntersections(intersections, condition, n);
}

bool EKLM::LineSegment2D::tWithinRange(double t) const
{
  return (t >= 0 && t <= 1);
}

