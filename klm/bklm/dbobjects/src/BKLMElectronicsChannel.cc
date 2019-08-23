/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/bklm/dbobjects/BKLMElectronicsChannel.h>

using namespace Belle2;

bool BKLMElectronicsChannel::operator<(
  const BKLMElectronicsChannel& channel) const
{
  if (m_Copper < channel.m_Copper)
    return true;
  else if (m_Copper > channel.m_Copper)
    return false;
  if (m_Slot < channel.m_Slot)
    return true;
  else if (m_Slot > channel.m_Slot)
    return false;
  if (m_Lane < channel.m_Lane)
    return true;
  else if (m_Lane > channel.m_Lane)
    return false;
  if (m_Axis < channel.m_Axis)
    return true;
  else if (m_Axis > channel.m_Axis)
    return false;
  return m_Channel < channel.m_Channel;
}

