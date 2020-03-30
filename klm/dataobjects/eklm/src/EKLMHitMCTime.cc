/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMHitMCTime.h>

using namespace Belle2;

EKLMHitMCTime::EKLMHitMCTime()
{
  m_MCTime = -1;
}

EKLMHitMCTime::~EKLMHitMCTime()
{
}

void EKLMHitMCTime::setMCTime(float t)
{
  m_MCTime = t;
}

float EKLMHitMCTime::getMCTime() const
{
  return m_MCTime;
}

