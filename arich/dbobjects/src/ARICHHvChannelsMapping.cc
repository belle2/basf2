/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHHvChannelsMapping.h>
#include <iostream>

using namespace std;
using namespace Belle2;

ARICHHvChannelsMapping::ARICHHvChannelsMapping()
{
}

std::vector<int> ARICHHvChannelsMapping::getChannelValues(int connectionID, int pinID) const
{

  vector<int> channel{ -2, -2, -2};
  for (auto& i : m_channel2type) {
    if (((i.second)[0] == connectionID) && ((i.second)[1] == pinID)) channel = i.first;
  }
  return channel;
}

int ARICHHvChannelsMapping::getPinID(std::vector<int> channel) const
{
  std::vector<int> pinProp = m_channel2type.find(channel)->second;
  return pinProp[1];
}

int ARICHHvChannelsMapping::getConnectionID(std::vector<int> channel) const
{
  std::vector<int> pinProp = m_channel2type.find(channel)->second;
  return pinProp[0];
}

int ARICHHvChannelsMapping::getCrate(int connectionID, int pinID) const
{
  return (ARICHHvChannelsMapping::getChannelValues(connectionID, pinID))[0];
}

int ARICHHvChannelsMapping::getSlot(int connectionID, int pinID) const
{
  return (ARICHHvChannelsMapping::getChannelValues(connectionID, pinID))[1];
}

int ARICHHvChannelsMapping::getChannel(int connectionID, int pinID) const
{
  return (ARICHHvChannelsMapping::getChannelValues(connectionID, pinID))[2];
}

void ARICHHvChannelsMapping::addMapping(int crate, int slot, int channelID, int connectionID, int pinID)
{

  if (crate > 6 || crate < 3) { B2WARNING("ARICHHvChannelsMapping::addMapping: Crate ID number " << crate << " not valid!"); }
  if (slot > 12 || slot < 0 || slot % 2 != 0) { B2WARNING("ARICHHvChannelsMapping::addMapping: Slot ID number " << slot << " not valid!"); }

  std::vector<int> pinProp{connectionID, pinID};
  std::vector<int> channelMap{crate, slot, channelID};
  m_channel2type.insert(std::pair<std::vector<int>, std::vector<int>>(channelMap, pinProp));

}

