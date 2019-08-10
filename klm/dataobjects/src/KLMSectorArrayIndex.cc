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
#include <klm/dataobjects/KLMSectorArrayIndex.h>
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

KLMSectorArrayIndex::KLMSectorArrayIndex()
{
  uint16_t index = 0;
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  m_NSectors = 0;
  for (KLMChannelIndex& klmSector : klmSectors) {
    uint16_t number = klmSector.getKLMSectorNumber();
    m_MapNumberIndex.insert(std::pair<uint16_t, uint16_t>(number, index));
    m_MapIndexNumber.insert(std::pair<uint16_t, uint16_t>(index, number));
    index++;
    m_NSectors++;
  }
}

KLMSectorArrayIndex::~KLMSectorArrayIndex()
{
}

const KLMSectorArrayIndex& KLMSectorArrayIndex::Instance()
{
  static KLMSectorArrayIndex klmSectorArrayIndex;
  return klmSectorArrayIndex;
}

uint16_t KLMSectorArrayIndex::getIndex(uint16_t number) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapNumberIndex.find(number);
  if (it == m_MapNumberIndex.end())
    B2FATAL("Incorrect KLM sector number.");
  return it->second;
}

uint16_t KLMSectorArrayIndex::getNumber(uint16_t index) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapIndexNumber.find(index);
  if (it == m_MapIndexNumber.end())
    B2FATAL("Incorrect KLM sector index.");
  return it->second;
}
