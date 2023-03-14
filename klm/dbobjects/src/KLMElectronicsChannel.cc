/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMElectronicsChannel.h>

/* KLM headers. */
#include <klm/rawdata/RawData.h>

using namespace Belle2;

bool KLMElectronicsChannel::operator<(
  const KLMElectronicsChannel& channel) const
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

KLMElectronicsChannel KLMElectronicsChannel::getAsic() const
{
  int asic = (m_Channel - 1) / KLM::c_NChannelsAsic;
  return KLMElectronicsChannel(m_Copper, m_Slot, m_Lane, m_Axis, asic);
}
