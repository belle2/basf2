/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMTimeCableDelay.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMTimeCableDelay::KLMTimeCableDelay()
{
}

KLMTimeCableDelay::~KLMTimeCableDelay()
{
}

void KLMTimeCableDelay::setTimeDelay(KLMChannelNumber channel, float delay)
{
  std::map<KLMChannelNumber, float>::iterator it;
  it = m_timeDelay.find(channel);
  if (it == m_timeDelay.end())
    m_timeDelay.insert(std::pair<KLMChannelNumber, float>(channel, delay));
  else
    it->second = delay;
}

float KLMTimeCableDelay::getTimeDelay(KLMChannelNumber channel) const
{
  std::map<KLMChannelNumber, float>::const_iterator it;
  it = m_timeDelay.find(channel);
  if (it == m_timeDelay.end())
    return std::numeric_limits<float>::quiet_NaN();
  return it->second;
}

void KLMTimeCableDelay::clearTimeDelay()
{
  m_timeDelay.clear();
}
