/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
KLMChannelStatus::getChannelStatus(KLMChannelNumber channel) const {
  std::map<KLMChannelNumber, enum ChannelStatus>::const_iterator it;
  it = m_ChannelStatus.find(channel);
  if (it == m_ChannelStatus.end())
    return c_Unknown;
  return it->second;
}

void KLMChannelStatus::setChannelStatus(KLMChannelNumber channel,
                                        enum ChannelStatus status)
{
  std::map<KLMChannelNumber, enum ChannelStatus>::iterator it;
  it = m_ChannelStatus.find(channel);
  if (it == m_ChannelStatus.end()) {
    m_ChannelStatus.insert(
      std::pair<KLMChannelNumber, enum ChannelStatus>(channel, status));
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

int KLMChannelStatus::getActiveStripsInModule(KLMChannelNumber module) const
{
  int active;
  int subdetector, section, sector, layer;
  const KLMElementNumbers* elementNumbers =
    &(KLMElementNumbers::Instance());
  elementNumbers->moduleNumberToElementNumbers(
    module, &subdetector, &section, &sector, &layer);
  KLMChannelIndex klmModule(subdetector, section, sector, layer, 1, 1);
  /* Plane number is 0-based for BKLM. */
  if (subdetector == KLMElementNumbers::c_BKLM) {
    KLMChannelIndex klmModuleTemp(subdetector, section, sector, layer, 0, 1);
    klmModule = klmModuleTemp;
  }
  klmModule.setIndexLevel(KLMChannelIndex::c_IndexLevelLayer);
  KLMChannelIndex klmNextModule(klmModule);
  ++klmNextModule;
  KLMChannelIndex klmChannel(klmModule);
  klmChannel.setIndexLevel(KLMChannelIndex::c_IndexLevelStrip);
  active = 0;
  for (; klmChannel != klmNextModule; ++klmChannel) {
    KLMChannelNumber channel = klmChannel.getKLMChannelNumber();
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
  std::map<KLMChannelNumber, enum ChannelStatus>::iterator it, it2;
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
  std::map<KLMChannelNumber, enum ChannelStatus>::iterator it, it2;
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
