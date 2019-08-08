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
#include <framework/logging/Logger.h>
#include <klm/dataobjects/KLMElementNumbers.h>

using namespace Belle2;

KLMElementNumbers::KLMElementNumbers()
{
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMElementNumbers::~KLMElementNumbers()
{
}

const KLMElementNumbers& KLMElementNumbers::Instance()
{
  static KLMElementNumbers klmElementNumbers;
  return klmElementNumbers;
}

uint16_t KLMElementNumbers::channelNumberBKLM(
  int section, int sector, int layer, int plane, int strip) const
{
  uint16_t channel;
  channel = BKLMElementNumbers::channelNumber(
              section, sector, layer, plane, strip);
  return channel + m_BKLMOffset;
}

uint16_t KLMElementNumbers::channelNumberBKLM(int bklmChannel) const
{
  return bklmChannel + m_BKLMOffset;
}

uint16_t KLMElementNumbers::channelNumberEKLM(
  int section, int sector, int layer, int plane, int strip) const
{
  uint16_t channel;
  /*
   * Note that the default order of elements is different
   * for EKLM-specific code!
   */
  channel = m_ElementNumbersEKLM->stripNumber(
              section, layer, sector, plane, strip);
  return channel;
}

uint16_t KLMElementNumbers::channelNumberEKLM(int eklmStrip) const
{
  return eklmStrip;
}

bool KLMElementNumbers::isBKLMChannel(uint16_t channel) const
{
  return (channel >= m_BKLMOffset);
}

bool KLMElementNumbers::isEKLMChannel(uint16_t channel) const
{
  return (channel < m_BKLMOffset);
}

int KLMElementNumbers::localChannelNumberBKLM(uint16_t channel) const
{
  if (!isBKLMChannel(channel))
    B2FATAL("Cannot get BKLM local channel number for non-BKLM channel.");
  return channel - m_BKLMOffset;
}

int KLMElementNumbers::localChannelNumberEKLM(uint16_t channel) const
{
  if (!isEKLMChannel(channel))
    B2FATAL("Cannot get EKLM local channel number for non-EKLM channel.");
  return channel;
}

void KLMElementNumbers::channelNumberToElementNumbers(
  uint16_t channel, int* subdetector, int* section, int* sector, int* layer,
  int* plane, int* strip) const
{
  int localChannel;
  if (isBKLMChannel(channel)) {
    *subdetector = c_BKLM;
    localChannel = localChannelNumberBKLM(channel);
    BKLMElementNumbers::channelNumberToElementNumbers(
      localChannel, section, sector, layer, plane, strip);
  } else {
    *subdetector = c_EKLM;
    localChannel = localChannelNumberEKLM(channel);
    /*
     * Note that the default order of elements is different
     * for EKLM-specific code!
     */
    m_ElementNumbersEKLM->stripNumberToElementNumbers(
      localChannel, section, layer, sector, plane, strip);
  }
}

uint16_t KLMElementNumbers::moduleNumber(
  int subdetector, int section, int sector, int layer) const
{
  if (subdetector == c_BKLM)
    return moduleNumberBKLM(section, sector, layer);
  else
    return moduleNumberEKLM(section, sector, layer);
}

uint16_t KLMElementNumbers::moduleNumberBKLM(
  int section, int sector, int layer) const
{
  uint16_t module;
  module = BKLMElementNumbers::moduleNumber(section, sector, layer);
  return module + m_BKLMOffset;
}

uint16_t KLMElementNumbers::moduleNumberEKLM(
  int section, int sector, int layer) const
{
  uint16_t module;
  /*
   * Note that the default order of elements is different
   * for EKLM-specific code!
   */
  module = m_ElementNumbersEKLM->sectorNumber(section, layer, sector);
  return module;
}

void KLMElementNumbers::moduleNumberToElementNumbers(
  uint16_t module, int* subdetector, int* section, int* sector,
  int* layer) const
{
  int localModule;
  if (isBKLMChannel(module)) {
    *subdetector = c_BKLM;
    localModule = localChannelNumberBKLM(module);
    BKLMElementNumbers::moduleNumberToElementNumbers(
      localModule, section, sector, layer);
  } else {
    *subdetector = c_EKLM;
    localModule = localChannelNumberEKLM(module);
    /*
     * Note that the default order of elements is different
     * for EKLM-specific code!
     */
    m_ElementNumbersEKLM->sectorNumberToElementNumbers(
      localModule, section, layer, sector);
  }
}

unsigned int KLMElementNumbers::getNChannelsModule(uint16_t module) const
{
  if (isBKLMChannel(module)) {
    int localModule = localChannelNumberBKLM(module);
    int section, sector, layer;
    BKLMElementNumbers::moduleNumberToElementNumbers(
      localModule, &section, &sector, &layer);
    return BKLMElementNumbers::getNStrips(section, sector, layer, 0) +
           BKLMElementNumbers::getNStrips(section, sector, layer, 1);
  } else {
    return EKLMElementNumbers::getNStripsSector();
  }
}

uint16_t KLMElementNumbers::sectorNumberBKLM(int section, int sector) const
{
  uint16_t sect;
  sect = BKLMElementNumbers::sectorNumber(section, sector);
  return sect + m_BKLMOffset;
}

uint16_t KLMElementNumbers::sectorNumberEKLM(int section, int sector) const
{
  uint16_t sect;
  sect = m_ElementNumbersEKLM->sectorNumberKLMOrder(section, sector);
  return sect;
}
