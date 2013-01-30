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
#include <eklm/dataobjects/EKLMHitGlobalCoord.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(EKLMHitGlobalCoord);

EKLMHitGlobalCoord::EKLMHitGlobalCoord()
{
}

EKLMHitGlobalCoord::~EKLMHitGlobalCoord()
{
}

HepGeom::Point3D<double> EKLMHitGlobalCoord::getGlobalPosition() const
{
  return HepGeom::Point3D<double>(m_globalX, m_globalY, m_globalZ);
}

void EKLMHitGlobalCoord::setGlobalPosition(HepGeom::Point3D<double> gpos)
{
  m_globalX = gpos.x();
  m_globalY = gpos.y();
  m_globalZ = gpos.z();
}

TVector3 EKLMHitGlobalCoord::getPosition() const
{
  return TVector3(m_globalX, m_globalY, m_globalZ);
}

