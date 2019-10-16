/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/Circle2D.h>

using namespace Belle2;

EKLM::Circle2D::Circle2D(double x, double y, double radius)
{
  m_Center.setX(x);
  m_Center.setY(y);
  m_Center.setZ(0);
  m_Radius = radius;
}

EKLM::Circle2D::~Circle2D()
{
}

const HepGeom::Point3D<double>& EKLM::Circle2D::getCenter() const
{
  return m_Center;
}

double EKLM::Circle2D::getRadius() const
{
  return m_Radius;
}

