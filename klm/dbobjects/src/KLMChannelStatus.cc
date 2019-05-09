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
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMChannelStatus::KLMChannelStatus()
{
}

KLMChannelStatus::~KLMChannelStatus()
{
}

enum KLMChannelStatus::ChannelStatus
KLMChannelStatus::getChannelStatus(uint16_t channel) const {
  std::map<uint16_t, enum ChannelStatus>::const_iterator it;
  it = m_ChannelStatus.find(channel);
  if (it == m_ChannelStatus.end())
    return c_Unknown;
  return it->second;
}

void KLMChannelStatus::setChannelStatus(uint16_t channel,
                                        enum ChannelStatus status)
{
  std::map<uint16_t, enum ChannelStatus>::iterator it;
  it = m_ChannelStatus.find(channel);
  if (it == m_ChannelStatus.end()) {
    m_ChannelStatus.insert(
      std::pair<uint16_t, enum ChannelStatus>(channel, status));
  } else {
    it->second = status;
  }
}

void KLMChannelStatus::setStatusAllChannels(enum ChannelStatus status)
{
  const EKLM::ElementNumbersSingleton* eklmElementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  const KLMElementNumbers* elementNumbers =
    &(KLMElementNumbers::Instance());
  for (int isForward = 0; isForward < 2; isForward++) {
    for (int sector = 1; sector < 9; sector++) {
      for (int layer = 1; layer < 16; layer++) {
        for (int plane = 0; plane < 2; plane++) {
          int nStrips = BKLMElementNumbers::getNStrips(
                          isForward, sector, layer, plane);
          for (int strip = 1; strip <= nStrips; strip++) {
            uint16_t channel = elementNumbers->channelNumberBKLM(
                                 isForward, sector, layer, plane, strip);
            setChannelStatus(channel, status);
          }
        }
      }
    }
  }
  int endcap, layer, sector, plane, strip;
  int nEndcaps, nLayers[2], nSectors, nPlanes, nStrips;
  nEndcaps = eklmElementNumbers->getMaximalEndcapNumber();
  nLayers[0] = eklmElementNumbers->getMaximalDetectorLayerNumber(1);
  nLayers[1] = eklmElementNumbers->getMaximalDetectorLayerNumber(2);
  nSectors = eklmElementNumbers->getMaximalSectorNumber();
  nPlanes = eklmElementNumbers->getMaximalPlaneNumber();
  nStrips = eklmElementNumbers->getMaximalStripNumber();
  for (endcap = 1; endcap <= nEndcaps; endcap++) {
    for (layer = 1; layer <= nLayers[endcap - 1]; layer++) {
      for (sector = 1; sector <= nSectors; sector++) {
        for (plane = 1; plane <= nPlanes; plane++) {
          for (strip = 1; strip <= nStrips; strip++) {
            uint16_t channel = elementNumbers->channelNumberEKLM(
                                 endcap, layer, sector, plane, strip);
            setChannelStatus(channel, status);
          }
        }
      }
    }
  }
}

int KLMChannelStatus::getActiveStripsEKLMSector(int sectorGlobal) const
{
  int active;
  int nPlanes, nStrips;
  int endcap, layer, sector, plane, strip;
  const EKLM::ElementNumbersSingleton* eklmElementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  const KLMElementNumbers* elementNumbers =
    &(KLMElementNumbers::Instance());
  nPlanes = eklmElementNumbers->getMaximalPlaneNumber();
  nStrips = eklmElementNumbers->getMaximalStripNumber();
  eklmElementNumbers->sectorNumberToElementNumbers(
    sectorGlobal, &endcap, &layer, &sector);
  active = 0;
  for (plane = 1; plane <= nPlanes; ++plane) {
    for (strip = 1; strip <= nStrips; ++strip) {
      uint16_t channel = elementNumbers->channelNumberEKLM(
                           endcap, layer, sector, plane, strip);
      enum ChannelStatus status = getChannelStatus(channel);
      if (status == c_Unknown)
        B2FATAL("Incomplete KLM channel data.");
      if (status != c_Dead)
        active++;
    }
  }
  return active;
}
