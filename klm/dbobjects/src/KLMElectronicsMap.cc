/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

const uint16_t* KLMElectronicsMap::getDetectorChannel(
  KLMElectronicsChannel* electronicsChannel) const
{
  std::map<KLMElectronicsChannel, uint16_t>::const_iterator it;
  it = m_MapElectronicsDetector.find(*electronicsChannel);
  if (it == m_MapElectronicsDetector.end())
    return nullptr;
  return &(it->second);
}

const KLMElectronicsChannel* KLMElectronicsMap::getElectronicsChannel(
  uint16_t detectorChannel) const
{
  std::map<uint16_t, KLMElectronicsChannel>::const_iterator it;
  it = m_MapDetectorElectronics.find(detectorChannel);
  if (it == m_MapDetectorElectronics.end())
    return nullptr;
  return &(it->second);
}

void KLMElectronicsMap::addChannel(
  uint16_t detectorChannel, int copper, int slot, int lane, int axis,
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
  m_MapDetectorElectronics.insert(std::pair<uint16_t, KLMElectronicsChannel>(
                                    detectorChannel, electronicsChannel));
  m_MapElectronicsDetector.insert(std::pair<KLMElectronicsChannel, uint16_t>(
                                    electronicsChannel, detectorChannel));
}

