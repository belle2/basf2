/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMTimeResolution.h>

/* C++ headers. */
#include <limits>

using namespace Belle2;

float KLMTimeResolution::getTimeResolution(uint16_t channel) const
{
  std::map<uint16_t, float>::const_iterator it;
  it = m_TimeResolution.find(channel);
  if (it == m_TimeResolution.end())
    return std::numeric_limits<float>::quiet_NaN();
  return it->second;
}

void KLMTimeResolution::setTimeResolution(uint16_t channel, float delay)
{
  std::map<uint16_t, float>::iterator it;
  it = m_TimeResolution.find(channel);
  if (it == m_TimeResolution.end())
    m_TimeResolution.insert(std::pair<uint16_t, float>(channel, delay));
  else
    it->second = delay;
}
