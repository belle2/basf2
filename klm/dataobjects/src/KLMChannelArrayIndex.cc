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
#include <framework/logging/Logger.h>
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

KLMChannelArrayIndex::KLMChannelArrayIndex()
{
  uint16_t index = 0;
  KLMChannelIndex klmChannels;
  m_NChannels = 0;
  for (KLMChannelIndex& klmChannel : klmChannels) {
    uint16_t number = klmChannel.getKLMChannelNumber();
    m_MapNumberIndex.insert(std::pair<uint16_t, uint16_t>(number, index));
    m_MapIndexNumber.insert(std::pair<uint16_t, uint16_t>(index, number));
    index++;
    m_NChannels++;
  }
}

KLMChannelArrayIndex::~KLMChannelArrayIndex()
{
}

const KLMChannelArrayIndex& KLMChannelArrayIndex::Instance()
{
  static KLMChannelArrayIndex klmChannelArrayIndex;
  return klmChannelArrayIndex;
}

uint16_t KLMChannelArrayIndex::getIndex(uint16_t number) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapNumberIndex.find(number);
  if (it == m_MapNumberIndex.end())
    B2FATAL("Incorrect KLM channel number.");
  return it->second;
}

uint16_t KLMChannelArrayIndex::getNumber(uint16_t index) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapIndexNumber.find(index);
  if (it == m_MapNumberIndex.end())
    B2FATAL("Incorrect KLM channel index.");
  return it->second;
}
