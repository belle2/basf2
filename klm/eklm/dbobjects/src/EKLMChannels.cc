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
#include <klm/eklm/dbobjects/EKLMChannels.h>

using namespace Belle2;

EKLMChannels::EKLMChannels()
{
}

EKLMChannels::~EKLMChannels()
{
}

void EKLMChannels::setChannelData(uint16_t strip, EKLMChannelData* dat)
{
  std::map<uint16_t, EKLMChannelData>::iterator it;
  it = m_Channels.find(strip);
  if (it == m_Channels.end())
    m_Channels.insert(std::pair<uint16_t, EKLMChannelData>(strip, *dat));
  else
    it->second = *dat;
}

const EKLMChannelData* EKLMChannels::getChannelData(uint16_t strip) const
{
  std::map<uint16_t, EKLMChannelData>::const_iterator it;
  it = m_Channels.find(strip);
  if (it == m_Channels.end())
    return nullptr;
  return &(it->second);
}
