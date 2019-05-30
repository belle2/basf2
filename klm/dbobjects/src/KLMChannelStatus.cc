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
#include <klm/dataobjects/BKLMChannelIndex.h>
#include <klm/dataobjects/EKLMChannelIndex.h>
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
  BKLMChannelIndex bklmChannels;
  for (BKLMChannelIndex& bklmChannel : bklmChannels)
    setChannelStatus(bklmChannel.getKLMChannelNumber(), status);
  EKLMChannelIndex eklmChannels;
  for (EKLMChannelIndex& eklmChannel : eklmChannels)
    setChannelStatus(eklmChannel.getKLMChannelNumber(), status);
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

bool KLMChannelStatus::operator==(KLMChannelStatus& status)
{
  if (this->m_ChannelStatus.size() != status.m_ChannelStatus.size())
    return false;
  std::map<uint16_t, enum ChannelStatus>::iterator it, it2;
  it = this->m_ChannelStatus.begin();
  it2 = status.m_ChannelStatus.begin();
  while (it != this->m_ChannelStatus.end()) {
    if (it->first != it2->first)
      return false;
    if (it->second != it2->second)
      return false;
    ++it;
    ++it2;
  }
  return true;
}

unsigned int KLMChannelStatus::newNormalChannels(KLMChannelStatus& status)
{
  unsigned int channels = 0;
  if (this->m_ChannelStatus.size() != status.m_ChannelStatus.size())
    return 0;
  std::map<uint16_t, enum ChannelStatus>::iterator it, it2;
  it = this->m_ChannelStatus.begin();
  it2 = status.m_ChannelStatus.begin();
  while (it != this->m_ChannelStatus.end()) {
    if (it->first != it2->first)
      return 0;
    if ((it->second == c_Normal) && (it2->second != c_Normal))
      ++channels;
    ++it;
    ++it2;
  }
  return channels;
}
