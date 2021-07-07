/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMElectronicsMap.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMElectronicsMap::KLMElectronicsMap()
{
}

KLMElectronicsMap::~KLMElectronicsMap()
{
}

const KLMChannelNumber* KLMElectronicsMap::getDetectorChannel(
  KLMElectronicsChannel* electronicsChannel) const
{
  std::map<KLMElectronicsChannel, KLMChannelNumber>::const_iterator it;
  it = m_MapElectronicsDetector.find(*electronicsChannel);
  if (it == m_MapElectronicsDetector.end())
    return nullptr;
  return &(it->second);
}

const KLMElectronicsChannel* KLMElectronicsMap::getElectronicsChannel(
  KLMChannelNumber detectorChannel) const
{
  std::map<KLMChannelNumber, KLMElectronicsChannel>::const_iterator it;
  it = m_MapDetectorElectronics.find(detectorChannel);
  if (it == m_MapDetectorElectronics.end())
    return nullptr;
  return &(it->second);
}

void KLMElectronicsMap::addChannel(
  KLMChannelNumber detectorChannel, int copper, int slot, int lane, int axis,
  int channel)
{
  KLMElectronicsChannel electronicsChannel(copper, slot, lane, axis, channel);
  if (m_MapDetectorElectronics.find(detectorChannel) != m_MapDetectorElectronics.end()) {
    B2ERROR("Detector channel already exists in the electronics map."
            << LogVar("Detector channel", detectorChannel));
    return;
  }
  if (m_MapElectronicsDetector.find(electronicsChannel) != m_MapElectronicsDetector.end()) {
    B2ERROR("Electronics channel already exists in the electronics map."
            << LogVar("Copper", copper)
            << LogVar("Slot", slot)
            << LogVar("Lane", lane)
            << LogVar("Axis", axis)
            << LogVar("Channel", channel));
    return;
  }
  m_MapDetectorElectronics.insert(std::pair<KLMChannelNumber, KLMElectronicsChannel>(
                                    detectorChannel, electronicsChannel));
  m_MapElectronicsDetector.insert(std::pair<KLMElectronicsChannel, KLMChannelNumber>(
                                    electronicsChannel, detectorChannel));
}

