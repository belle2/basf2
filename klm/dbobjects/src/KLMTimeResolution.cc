/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMTimeResolution.h>

/* C++ headers. */
#include <limits>

using namespace Belle2;

float KLMTimeResolution::getTimeResolution(KLMChannelNumber channel) const
{
  std::map<KLMChannelNumber, float>::const_iterator it;
  it = m_TimeResolution.find(channel);
  if (it == m_TimeResolution.end())
    return std::numeric_limits<float>::quiet_NaN();
  return it->second;
}

void KLMTimeResolution::setTimeResolution(KLMChannelNumber channel, float delay)
{
  std::map<KLMChannelNumber, float>::iterator it;
  it = m_TimeResolution.find(channel);
  if (it == m_TimeResolution.end())
    m_TimeResolution.insert(std::pair<KLMChannelNumber, float>(channel, delay));
  else
    it->second = delay;
}
