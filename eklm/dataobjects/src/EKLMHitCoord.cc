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
}

EKLMHitCoord::~EKLMHitCoord()
{
}

HepGeom::Point3D<double> EKLMHitCoord::getGlobalPosition()
{
  return HepGeom::Point3D<double>(m_globalX, m_globalY, m_globalZ);
}

void EKLMHitCoord::setGlobalPosition(HepGeom::Point3D<double> gpos)
{
  m_globalX = gpos.x();
  m_globalY = gpos.y();
  m_globalZ = gpos.z();
}

HepGeom::Point3D<double> EKLMHitCoord::getLocalPosition()
{
  return HepGeom::Point3D<double>(m_localX, m_localY, m_localZ);
}

void EKLMHitCoord::setLocalPosition(HepGeom::Point3D<double> lpos)
{
  m_localX = lpos.x();
  m_localY = lpos.y();
  m_localZ = lpos.z();
}

const TVector3* EKLMHitCoord::getPosition() const
{
  return new TVector3(m_localX, m_localY, m_localZ);
}

const TVector3 EKLMHitCoord::getGlobalPosition() const
{
  return TVector3(m_localX, m_localY, m_localZ);
}

