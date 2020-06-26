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
#include <klm/dbobjects/KLMChannelStatus.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>

/* Belle 2 headers. */
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
  KLMChannelIndex klmChannels;
  for (KLMChannelIndex& klmChannel : klmChannels)
    setChannelStatus(klmChannel.getKLMChannelNumber(), status);
}

int KLMChannelStatus::getActiveStripsInModule(uint16_t module) const
{
  int active;
  int subdetector, section, layer, sector;
  const KLMElementNumbers* elementNumbers =
    &(KLMElementNumbers::Instance());
  elementNumbers->moduleNumberToElementNumbers(
    module, &subdetector, &section, &sector, &layer);
  KLMChannelIndex klmModule(subdetector, section, sector, layer, 1, 1);
  klmModule.setIndexLevel(KLMChannelIndex::c_IndexLevelLayer);
  KLMChannelIndex klmNextModule(klmModule);
  ++klmNextModule;
  KLMChannelIndex klmChannel(klmModule);
  klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
  active = 0;
  for (; klmChannel != klmNextModule; ++klmChannel) {
    uint16_t channel = klmChannel.getKLMChannelNumber();
    ChannelStatus status = getChannelStatus(channel);
    if (status == c_Unknown)
      B2FATAL("Incomplete KLM channel data.");
    if (status != c_Dead)
      active++;
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
