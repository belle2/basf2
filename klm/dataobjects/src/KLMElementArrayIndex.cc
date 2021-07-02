/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMElementArrayIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMElementArrayIndex::KLMElementArrayIndex(
  enum KLMChannelIndex::IndexLevel indexLevel)
{
  uint16_t index = 0;
  KLMChannelIndex klmElements(indexLevel);
  m_NElements = 0;
  for (KLMChannelIndex& klmElement : klmElements) {
    uint16_t number;
    switch (indexLevel) {
      case KLMChannelIndex::c_IndexLevelStrip:
        number = klmElement.getKLMChannelNumber();
        break;
      case KLMChannelIndex::c_IndexLevelPlane:
        number = klmElement.getKLMPlaneNumber();
        break;
      case KLMChannelIndex::c_IndexLevelLayer:
        number = klmElement.getKLMModuleNumber();
        break;
      case KLMChannelIndex::c_IndexLevelSector:
        number = klmElement.getKLMSectorNumber();
        break;
      default:
        B2FATAL("Unsupported index level in KLMElementArrayIndex.");
    }
    m_MapNumberIndex.insert(std::pair<uint16_t, uint16_t>(number, index));
    m_MapIndexNumber.insert(std::pair<uint16_t, uint16_t>(index, number));
    index++;
    m_NElements++;
  }
}

KLMElementArrayIndex::~KLMElementArrayIndex()
{
}

uint16_t KLMElementArrayIndex::getIndex(uint16_t number) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapNumberIndex.find(number);
  if (it == m_MapNumberIndex.end())
    B2FATAL("Incorrect KLM element number.");
  return it->second;
}

uint16_t KLMElementArrayIndex::getNumber(uint16_t index) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapIndexNumber.find(index);
  if (it == m_MapIndexNumber.end())
    B2FATAL("Incorrect KLM element index.");
  return it->second;
}
