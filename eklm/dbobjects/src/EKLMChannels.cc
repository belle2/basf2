/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMChannels::EKLMChannels()
{
}

EKLMChannels::~EKLMChannels()
{
}

void EKLMChannels::setChannelData(uint16_t strip, EKLMChannelData* dat)
{
  std::map<uint16_t, EKLMChannelData>::iterator it;
  it = m_Channels.find(strip);
  if (it == m_Channels.end())
    m_Channels.insert(std::pair<uint16_t, EKLMChannelData>(strip, *dat));
  else
    it->second = *dat;
}

const EKLMChannelData* EKLMChannels::getChannelData(uint16_t strip) const
{
  std::map<uint16_t, EKLMChannelData>::const_iterator it;
  it = m_Channels.find(strip);
  if (it == m_Channels.end())
    return nullptr;
  return &(it->second);
}

int EKLMChannels::getActiveStripsSector(int sectorGlobal) const
{
  int active;
  int nPlanes, nStrips;
  int endcap, layer, sector, plane, strip, stripGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  nPlanes = elementNumbers->getMaximalPlaneNumber();
  nStrips = elementNumbers->getMaximalStripNumber();
  elementNumbers->sectorNumberToElementNumbers(
    sectorGlobal, &endcap, &layer, &sector);
  active = 0;
  for (plane = 1; plane <= nPlanes; ++plane) {
    for (strip = 1; strip <= nStrips; ++strip) {
      stripGlobal = elementNumbers->stripNumber(
                      endcap, layer, sector, plane, strip);
      const EKLMChannelData* channelData = getChannelData(stripGlobal);
      if (channelData == nullptr)
        B2FATAL("Incomplete EKLM channel data.");
      if (channelData->getActive())
        active++;
    }
  }
  return active;
}
