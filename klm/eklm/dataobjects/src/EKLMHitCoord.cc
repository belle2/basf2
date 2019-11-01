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
#include <klm/eklm/dataobjects/EKLMHitCoord.h>

using namespace Belle2;

EKLMHitCoord::EKLMHitCoord()
{
  m_localX = -1;
  m_localY = -1;
  m_localZ = -1;
}

EKLMHitCoord::~EKLMHitCoord()
{
}

void EKLMHitCoord::setLocalPosition(float x, float y, float z)
{
  m_localX = x;
  m_localY = y;
  m_localZ = z;
}

float EKLMHitCoord::getLocalPositionX() const
{
  return m_localX;
}

float EKLMHitCoord::getLocalPositionY() const
{
  return m_localY;
}

float EKLMHitCoord::getLocalPositionZ() const
{
  return m_localZ;
}

TVector3 EKLMHitCoord::getLocalPosition() const
{
  return TVector3(m_localX, m_localY, m_localZ);
}

