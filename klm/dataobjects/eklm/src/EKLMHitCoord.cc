/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMHitCoord.h>

using namespace Belle2;

EKLMHitCoord::EKLMHitCoord() :
  m_localX(-1),
  m_localY(-1),
  m_localZ(-1)
{
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
