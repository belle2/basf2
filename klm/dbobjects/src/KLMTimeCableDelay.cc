/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

void KLMTimeCableDelay::setTimeShift(uint16_t strip, double par)
{
  std::map<uint16_t, double>::iterator it;
  it = m_timeShift.find(strip);
  if (it == m_timeShift.end())
    m_timeShift.insert(std::pair<uint16_t, double>(strip, par));
  else
    B2WARNING("TimeCalibration para for the strip already exists.");
}

double KLMTimeCableDelay::getTimeShift(uint16_t strip) const
{
  std::map<uint16_t, double>::const_iterator it;
  it = m_timeShift.find(strip);
  if (it == m_timeShift.end())
    return 0.0;
  return it->second;
}

void KLMTimeCableDelay::cleanTimeShift()
{
  m_timeShift.clear();
}
