/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMK0L.h>

using namespace Belle2;

ClassImp(EKLMK0L);

EKLMK0L::EKLMK0L()
{
}

EKLMK0L::~EKLMK0L()
{
}

void EKLMK0L::setHitPosition(HepGeom::Point3D<double> pos)
{
  m_hitX = pos.x();
  m_hitY = pos.y();
  m_hitZ = pos.z();
}

HepGeom::Point3D<double> EKLMK0L::getHitPosition() const
{
  return HepGeom::Point3D<double>(m_hitX, m_hitY, m_hitZ);
}

