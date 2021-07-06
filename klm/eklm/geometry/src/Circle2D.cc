/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
