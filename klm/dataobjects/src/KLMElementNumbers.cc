/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMElementNumbers.h>

using namespace Belle2;

KLMElementNumbers::KLMElementNumbers()
{
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMElementNumbers::~KLMElementNumbers()
{
}

uint16_t KLMElementNumbers::channelNumberBKLM(
  int forward, int sector, int layer, int plane, int strip) const
{
  uint16_t channel;
  channel = BKLMElementNumbers::channelNumber(
              forward, sector, layer, plane, strip);
  return channel + 0x8000;
}

uint16_t KLMElementNumbers::channelNumberEKLM(
  int endcap, int layer, int sector, int plane, int strip) const
{
  uint16_t channel;
  channel = m_ElementNumbersEKLM->stripNumber(
              endcap, layer, sector, plane, strip);
  return channel;
}
