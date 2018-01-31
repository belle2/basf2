/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMTimeConversion.h>

using namespace Belle2;

EKLMTimeConversion::EKLMTimeConversion()
{
  m_TDCPeriod = 0;
  m_TimeOffset = 0;
}

EKLMTimeConversion::~EKLMTimeConversion()
{
}

void EKLMTimeConversion::setTDCFrequency(double frequency)
{
  m_TDCPeriod = 1.0 / frequency;
}

void EKLMTimeConversion::setTimeOffset(double offset)
{
  m_TimeOffset = offset;
}
