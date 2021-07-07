/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMHitGlobalCoord.h>

using namespace Belle2;

EKLMHitGlobalCoord::EKLMHitGlobalCoord() :
  m_globalX(-1),
  m_globalY(-1),
  m_globalZ(-1)
{
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
