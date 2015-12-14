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
#include <eklm/geometry/Polygon2D.h>

using namespace Belle2;

EKLM::Polygon2D::Polygon2D(const HepGeom::Point3D<double>* points, int n)
{
  int i;
  m_nPoints = n;
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

bool EKLM::Polygon2D::hasIntersection(const LineSegment2D& lineSegment) const
{
  int i;
  HepGeom::Point3D<double> intersection;
  for (i = 0; i < m_nPoints; i++) {
    if (m_LineSegments[i]->findIntersection(lineSegment, &intersection) > 0)
      return true;
  }
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
  return false;
}

bool EKLM::Polygon2D::hasIntersection(const Polygon2D& polygon) const
{
  int i;
  for (i = 0; i < m_nPoints; i++) {
    if (polygon.hasIntersection(*m_LineSegments[i]))
      return true;
  }
  return false;
}

