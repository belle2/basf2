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

EKLM::Circle2D::Circle2D(double x, double y, double radius) :
  m_Center(x, y, 0),
  m_Radius(radius)
{
}

EKLM::Circle2D::~Circle2D()
{
}
