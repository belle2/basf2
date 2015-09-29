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
#include <eklm/geometry/Circle2D.h>
#include <eklm/geometry/Line2D.h>

using namespace Belle2;

EKLM::Line2D::Line2D(double x, double y, double vecx, double vecy)
{
  m_Point.setX(x);
  m_Point.setY(y);
  m_Point.setZ(0);
  m_Vector.setX(vecx);
  m_Vector.setY(vecy);
  m_Vector.setZ(0);
}

EKLM::Line2D::~Line2D()
{
}

int EKLM::Line2D::
findIntersection(const Circle2D& circle,
                 HepGeom::Point3D<double> intersections[2]) const
{
  double t[2];
  return findIntersection(circle, intersections, t);
}

/*
 * Equation: (m_Point.x() + m_Vector.x() * t - circle.getCenter().x())^2 +
 * (m_Point.y() + m_Vector.y() * t - circle.getCenter().y())^2 =
 * circle.getRadius()^2
 */
int EKLM::Line2D::
findIntersection(const Circle2D& circle,
                 HepGeom::Point3D<double> intersections[2],
                 double t[2]) const
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
    intersections[0].setY(m_Point.y() + m_Vector.y() * t[1]);
    intersections[0].setZ(0);
    return 1;
  }
  t[0] = (-b - sqrt(d)) / (2.0 * a);
  t[1] = (-b + sqrt(d)) / (2.0 * a);
  for (i = 0; i < 2; i++) {
    intersections[i].setX(m_Point.x() + m_Vector.x() * t[i]);
    intersections[i].setY(m_Point.y() + m_Vector.y() * t[i]);
    intersections[i].setZ(0);
  }
  return 2;
}

