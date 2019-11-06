/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMPlaneArrayIndex.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMPlaneArrayIndex::KLMPlaneArrayIndex()
{
  uint16_t index = 0;
  KLMChannelIndex klmPlanes(KLMChannelIndex::c_IndexLevelPlane);
  m_NPlanes = 0;
  for (KLMChannelIndex& klmPlane : klmPlanes) {
    uint16_t number = klmPlane.getKLMPlaneNumber();
    m_MapNumberIndex.insert(std::pair<uint16_t, uint16_t>(number, index));
    m_MapIndexNumber.insert(std::pair<uint16_t, uint16_t>(index, number));
    index++;
    m_NPlanes++;
  }
}

KLMPlaneArrayIndex::~KLMPlaneArrayIndex()
{
}

const KLMPlaneArrayIndex& KLMPlaneArrayIndex::Instance()
{
  static KLMPlaneArrayIndex klmPlaneArrayIndex;
  return klmPlaneArrayIndex;
}

uint16_t KLMPlaneArrayIndex::getIndex(uint16_t number) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapNumberIndex.find(number);
  if (it == m_MapNumberIndex.end())
    B2FATAL("Incorrect KLM plane number.");
  return it->second;
}

uint16_t KLMPlaneArrayIndex::getNumber(uint16_t index) const
{
  std::map<uint16_t, uint16_t>::const_iterator it =
    m_MapIndexNumber.find(index);
  if (it == m_MapIndexNumber.end())
    B2FATAL("Incorrect KLM plane index.");
  return it->second;
}
