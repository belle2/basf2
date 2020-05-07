/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/eklm/EKLMChannelData.h>

using namespace Belle2;

EKLMChannelData::EKLMChannelData()
{
  m_Active = false;
  m_Pedestal = 0;
  m_PhotoelectronAmplitude = 0;
  m_Threshold = 0;
  m_Voltage = 0;
  m_AdjustmentVoltage = 0;
  m_LookbackTime = 0;
  m_LookbackWindowWidth = 0;
}

EKLMChannelData::~EKLMChannelData()
{
}
