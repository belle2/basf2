/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMElementNumbers.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMElementNumbers::KLMElementNumbers() :
  m_eklmElementNumbers(&(EKLMElementNumbers::Instance()))
{
}

KLMElementNumbers::~KLMElementNumbers()
{
}

const KLMElementNumbers& KLMElementNumbers::Instance()
{
  static KLMElementNumbers klmElementNumbers;
  return klmElementNumbers;
}

uint16_t KLMElementNumbers::channelNumber(
  int subdetector, int section, int sector, int layer, int plane,
  int strip) const
{
  switch (subdetector) {
    case c_BKLM:
      return channelNumberBKLM(section, sector, layer, plane, strip);
    case c_EKLM:
      return channelNumberEKLM(section, sector, layer, plane, strip);
  }
  B2FATAL("Incorrect subdetector number: " << subdetector);
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
  channel = m_eklmElementNumbers->stripNumber(
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
    m_eklmElementNumbers->stripNumberToElementNumbers(
      localChannel, section, layer, sector, plane, strip);
  }
}

uint16_t KLMElementNumbers::planeNumberBKLM(
  int section, int sector, int layer, int plane) const
{
  uint16_t planeGlobal;
  planeGlobal = BKLMElementNumbers::planeNumber(section, sector, layer, plane);
  return planeGlobal + m_BKLMOffset;
}

uint16_t KLMElementNumbers::planeNumberEKLM(
  int section, int sector, int layer, int plane) const
{
  uint16_t planeGlobal;
  /*
   * Note that the default order of elements is different
   * for EKLM-specific code!
   */
  planeGlobal = m_eklmElementNumbers->planeNumber(
                  section, layer, sector, plane);
  return planeGlobal;
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
  module = m_eklmElementNumbers->sectorNumber(section, layer, sector);
  return module;
}

uint16_t KLMElementNumbers::moduleNumberByChannel(uint16_t channel) const
{
  int subdetector, section, sector, layer, plane, strip;
  channelNumberToElementNumbers(channel, &subdetector, &section, &sector,
                                &layer, &plane, &strip);
  return moduleNumber(subdetector, section, sector, layer);
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
    m_eklmElementNumbers->sectorNumberToElementNumbers(
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
  sect = m_eklmElementNumbers->sectorNumberKLMOrder(section, sector);
  return sect;
}

int KLMElementNumbers::getExtrapolationLayer(int subdetector, int layer) const
{
  if (subdetector == c_BKLM)
    return layer;
  else
    return BKLMElementNumbers::getMaximalLayerNumber() + layer;
}

int KLMElementNumbers::getMinimalPlaneNumber(int subdetector) const
{
  if (subdetector == c_BKLM)
    return 0;
  else
    return 1;
}

std::string KLMElementNumbers::getSectorDAQName(int subdetector, int section, int sector) const
{
  std::string name;
  if (subdetector == c_BKLM) {
    BKLMElementNumbers::checkSector(sector);
    if (section == BKLMElementNumbers::c_BackwardSection)
      name = "BB" + std::to_string(sector - 1);
    if (section == BKLMElementNumbers::c_ForwardSection)
      name = "BF" + std::to_string(sector - 1);
  }
  if (subdetector == c_EKLM) {
    m_eklmElementNumbers->checkSector(sector);
    if (section == EKLMElementNumbers::c_BackwardSection)
      name = "EB" + std::to_string(sector - 1);
    if (section == EKLMElementNumbers::c_ForwardSection)
      name = "EF" + std::to_string(sector - 1);
  }
  if (name.empty())
    B2FATAL("Invalid KLM sector."
            << LogVar("Subdetector", subdetector)
            << LogVar("Section", section)
            << LogVar("Sector", sector));
  return name;
}
