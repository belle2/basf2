/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/dataobjects/EKLMHitGlobalCoord.h>

using namespace Belle2;

EKLMHitGlobalCoord::EKLMHitGlobalCoord()
{
  m_globalX = -1;
  m_globalY = -1;
  m_globalZ = -1;
}

EKLMHitGlobalCoord::~EKLMHitGlobalCoord()
{
}

void EKLMHitGlobalCoord::setPosition(float x, float y, float z)
{
  m_globalX = x;
  m_globalY = y;
  m_globalZ = z;
}

void EKLMHitGlobalCoord::setPosition(const TVector3& pos)
{
  m_globalX = pos.X();
  m_globalY = pos.Y();
  m_globalZ = pos.Z();
}

float EKLMHitGlobalCoord::getPositionX() const
{
  return m_globalX;
}

float EKLMHitGlobalCoord::getPositionY() const
{
  return m_globalY;
}

float EKLMHitGlobalCoord::getPositionZ() const
{
  return m_globalZ;
}

TVector3 EKLMHitGlobalCoord::getPosition() const
{
  return TVector3(m_globalX, m_globalY, m_globalZ);
}

