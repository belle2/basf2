/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMChannelData.h>

using namespace Belle2;

EKLMChannelData::EKLMChannelData()
{
  m_Active = false;
}

EKLMChannelData::~EKLMChannelData()
{
}

void EKLMChannelData::setActive(bool active)
{
  m_Active = active;
}

bool EKLMChannelData::getActive() const
{
  return m_Active;
}

