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
