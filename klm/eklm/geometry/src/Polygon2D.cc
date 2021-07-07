/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/Polygon2D.h>

/* Belle 2 headers. */
#include <framework/core/RandomNumbers.h>

using namespace Belle2;

EKLM::Polygon2D::Polygon2D(const HepGeom::Point3D<double>* points, int n) :
  m_nPoints(n)
{
  int i;
  m_LineSegments = new LineSegment2D*[n];
  for (i = 0; i < n; i++) {
    if (i < n - 1)
      m_LineSegments[i] = new LineSegment2D(points[i], points[i + 1]);
    else
      m_LineSegments[i] = new LineSegment2D(points[i], points[0]);
  }
}

EKLM::Polygon2D::~Polygon2D()
{
  int i;
  for (i = 0; i < m_nPoints; i++)
    delete m_LineSegments[i];
  delete[] m_LineSegments;
}

bool EKLM::Polygon2D::pointInside(const HepGeom::Point3D<double>& point) const
{
  int i, n, nIntersections;
  double angle, t[2];
  HepGeom::Point3D<double> intersection;
new_line: {
    nIntersections = 0;
    angle = gRandom->Uniform(0., 2.0 * M_PI);
    Line2D line(point.x(), point.y(), cos(angle), sin(angle));
    for (i = 0; i < m_nPoints; i++) {
      n = m_LineSegments[i]->Line2D::findIntersection(line, &intersection, t);
      if (n == 0)
        continue;
      /* Line goes through vertex, need to generate another line. */
      if (t[0] == 0 || t[0] == 1)
        goto new_line;
      if (t[0] > 0 && t[0] < 1 && t[1] > 0)
        nIntersections++;
    }
  }
  if (nIntersections % 2 == 1)
    return true;
  return false;
}

bool EKLM::Polygon2D::hasIntersection(const LineSegment2D& lineSegment) const
{
  int i;
  HepGeom::Point3D<double> intersection;
  for (i = 0; i < m_nPoints; i++) {
    if (m_LineSegments[i]->findIntersection(lineSegment, &intersection) > 0)
      return true;
  }
  if (pointInside(lineSegment.getInitialPoint()))
    return true;
  return false;
}

bool EKLM::Polygon2D::hasIntersection(const Arc2D& arc) const
{
  int i;
  HepGeom::Point3D<double> intersections[2];
  for (i = 0; i < m_nPoints; i++) {
    if (m_LineSegments[i]->findIntersection(arc, intersections) > 0)
      return true;
  }
  if (pointInside(arc.getInitialPoint()))
    return true;
  return false;
}

bool EKLM::Polygon2D::hasIntersection(const Polygon2D& polygon) const
{
  int i;
  for (i = 0; i < m_nPoints; i++) {
    if (polygon.hasIntersection(*m_LineSegments[i]))
      return true;
  }
  if (pointInside(polygon.getLineSegments()[0]->getInitialPoint()) ||
      polygon.pointInside(m_LineSegments[0]->getInitialPoint()))
    return true;
  return false;
}
