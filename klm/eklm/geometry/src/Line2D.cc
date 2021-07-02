/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/Line2D.h>

/* KLM headers. */
#include <klm/eklm/geometry/Circle2D.h>

using namespace Belle2;

EKLM::Line2D::Line2D(double x, double y, double vecx, double vecy) :
  m_Point(x, y, 0),
  m_Vector(vecx, vecy, 0)
{
}

EKLM::Line2D::~Line2D()
{
}

int EKLM::Line2D::
findIntersection(const Line2D& line,
                 HepGeom::Point3D<double>* intersection) const
{
  double t[2];
  return findIntersection(line, intersection, t);
}

int EKLM::Line2D::
findIntersection(const Circle2D& circle,
                 HepGeom::Point3D<double> intersections[2]) const
{
  double t[2], angles[2];
  return findIntersection(circle, intersections, t, angles);
}

int EKLM::Line2D::
findIntersection(const Arc2D& arc,
                 HepGeom::Point3D<double> intersections[2]) const
{
  int i, n;
  double t[2], angles[2];
  bool condition[2];
  n = findIntersection(arc, intersections, t, angles);
  for (i = 0; i < n; i++)
    condition[i] = arc.angleWithinRange(angles[i]);
  return selectIntersections(intersections, condition, n);
}

/*
 * System of equations:
 * m_Point.x() + m_Vector.x() * t[0] =
 * line.getInitialPoint().x() + line.getVector().x() * t[1],
 * m_Point.y() + m_Vector.y() * t[0] =
 * line.getInitialPoint().y() + line.getVector().y() * t[1].
 * Other form:
 * a1 * t[0] + b1 * t[1] + c1 = 0, a2 * t[0] + b2 * t[1] + c2 = 0.
 */
int EKLM::Line2D::
findIntersection(const Line2D& line,
                 HepGeom::Point3D<double>* intersection, double t[2]) const
{
  double a1, a2, b1, b2, c1, c2, d, dt[2];
  a1 = m_Vector.x();
  a2 = m_Vector.y();
  b1 = -line.getVector().x();
  b2 = -line.getVector().y();
  c1 = m_Point.x() - line.getInitialPoint().x();
  c2 = m_Point.y() - line.getInitialPoint().y();
  d = a1 * b2 - a2 * b1;
  /* Same line (d == 0, dt[i] == 0) considered as no intersection! */
  if (d == 0)
    return 0;
  dt[0] = c1 * b2 - c2 * b1;
  dt[1] = a1 * c2 - a2 * c1;
  t[0] = -dt[0] / d;
  t[1] = -dt[1] / d;
  intersection->setX(m_Point.x() + m_Vector.x() * t[0]);
  intersection->setY(m_Point.y() + m_Vector.y() * t[0]);
  intersection->setZ(0);
  return 1;
}

/*
 * Equation: (m_Point.x() + m_Vector.x() * t - circle.getCenter().x())^2 +
 * (m_Point.y() + m_Vector.y() * t - circle.getCenter().y())^2 =
 * circle.getRadius()^2
 */
int EKLM::Line2D::
findIntersection(const Circle2D& circle,
                 HepGeom::Point3D<double> intersections[2],
                 double t[2], double angles[2]) const
{
  int i;
  double a, b, c, d, x0, y0;
  const HepGeom::Point3D<double>& circleCenter = circle.getCenter();
  x0 = m_Point.x() - circleCenter.x();
  y0 = m_Point.y() - circleCenter.y();
  a = m_Vector.x() * m_Vector.x() + m_Vector.y() * m_Vector.y();
  b = 2.0 * (m_Vector.x() * x0 + m_Vector.y() * y0);
  c = x0 * x0 + y0 * y0 - circle.getRadius() * circle.getRadius();
  d = b * b - 4 * a * c;
  if (d < 0)
    return 0;
  if (d == 0) {
    t[0] = -b / (2.0 * a);
    intersections[0].setX(m_Point.x() + m_Vector.x() * t[0]);
    intersections[0].setY(m_Point.y() + m_Vector.y() * t[0]);
    intersections[0].setZ(0);
    angles[0] = atan2(intersections[0].y() - circleCenter.y(),
                      intersections[0].x() - circleCenter.x());
    return 1;
  }
  t[0] = (-b - sqrt(d)) / (2.0 * a);
  t[1] = (-b + sqrt(d)) / (2.0 * a);
  for (i = 0; i < 2; i++) {
    intersections[i].setX(m_Point.x() + m_Vector.x() * t[i]);
    intersections[i].setY(m_Point.y() + m_Vector.y() * t[i]);
    intersections[i].setZ(0);
    angles[i] = atan2(intersections[i].y() - circleCenter.y(),
                      intersections[i].x() - circleCenter.x());
  }
  return 2;
}

int EKLM::Line2D::selectIntersections(HepGeom::Point3D<double>* intersections,
                                      bool* condition, int n) const
{
  int i, j;
  j = 0;
  for (i = 0; i < n; i++) {
    if (condition[i]) {
      if (i != j)
        intersections[j] = intersections[i];
      j++;
    }
  }
  return j;
}

