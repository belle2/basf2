/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMSimulationParameters.h>

using namespace Belle2;

EKLMSimulationParameters::EKLMSimulationParameters()
{
  m_HitTimeThreshold = 0;
}

EKLMSimulationParameters::~EKLMSimulationParameters()
{
}

float EKLMSimulationParameters::getHitTimeThreshold() const
{
  return m_HitTimeThreshold;
}

void EKLMSimulationParameters::setHitTimeThreshold(float threshold)
{
  m_HitTimeThreshold = threshold;
}

