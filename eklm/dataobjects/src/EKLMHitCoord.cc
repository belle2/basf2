/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitCoord.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(EKLMHitCoord);

EKLMHitCoord::EKLMHitCoord()
{
  m_localX = -1;
  m_localY = -1;
  m_localZ = -1;
}

EKLMHitCoord::~EKLMHitCoord()
{
}

HepGeom::Point3D<double> EKLMHitCoord::getLocalPosition() const
{
  return HepGeom::Point3D<double>(m_localX, m_localY, m_localZ);
}

void EKLMHitCoord::setLocalPosition(HepGeom::Point3D<double> lpos)
{
  m_localX = lpos.x();
  m_localY = lpos.y();
  m_localZ = lpos.z();
}

