/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHBiasChannelsMapping.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

ARICHBiasChannelsMapping::ARICHBiasChannelsMapping()
{
}

int ARICHBiasChannelsMapping::getPinID(std::vector<int> channel) const
{
  std::tuple<int, int, int, std::string> pinProp = m_channel2type.find(channel)->second;
  return std::get<0>(pinProp);
}
int ARICHBiasChannelsMapping::getConnectionID(std::vector<int> channel) const
{
  std::tuple<int, int, int, std::string> pinProp = m_channel2type.find(channel)->second;
  return std::get<1>(pinProp);
}
int ARICHBiasChannelsMapping::getInnerID(std::vector<int> channel) const
{
  std::tuple<int, int, int, std::string> pinProp = m_channel2type.find(channel)->second;
  return std::get<2>(pinProp);
}
std::string ARICHBiasChannelsMapping::getType(std::vector<int> channel) const
{
  std::tuple<int, int, int, std::string> pinProp = m_channel2type.find(channel)->second;
  return std::get<3>(pinProp);
}


std::tuple<int, int, std::string> ARICHBiasChannelsMapping::getInnerConnection(std::vector<int> channel) const
{
  // connection ID, inner ID, type

  std::tuple<int, int, int, std::string> pinProp = m_channel2type.find(channel)->second;
  return std::make_tuple(std::get<1>(pinProp), std::get<2>(pinProp), std::get<3>(pinProp));
}

int ARICHBiasChannelsMapping::getChannelID(int crate, int slot, int connectionID, int innerID, const std::string& type) const
{
  if (innerID > 9 || innerID < -2 || innerID == 0) { B2WARNING("ARICHBiasChannelsMapping::getChannelID: Inner ID " << innerID << " not valid!"); }

  vector<int> channel{ -2, -2, -2};
  for (auto& i : m_channel2type) {
    if ((std::get<1>(i.second) == connectionID) && (std::get<2>(i.second) == innerID) && (std::get<3>(i.second) == type)
        && ((i.first)[0] == crate) && ((i.first)[1] == slot)) channel = i.first;
  }
  return channel[2];
}
std::vector<int> ARICHBiasChannelsMapping::getChannelValues(int connectionID, int innerID, const std::string& type) const
{
  if (innerID > 9 || innerID < -2 || innerID == 0) { B2WARNING("ARICHBiasChannelsMapping::getChannelID: Inner ID " << innerID << " not valid!"); }

  vector<int> channel{ -2, -2, -2};
  for (auto& i : m_channel2type) {
    if ((std::get<1>(i.second) == connectionID) && (std::get<2>(i.second) == innerID)
        && (std::get<3>(i.second) == type)) channel = i.first;
  }
  return channel;
}


void ARICHBiasChannelsMapping::addMapping(int crate, int slot, int channelID, int pinID, int connectionID, int innerID,
                                          const std::string& type)
{

  if (channelID > 47 || channelID < 0) { B2WARNING("ARICHBiasChannelsMapping::addMapping: Channel ID number " << channelID << " not valid!"); }
  if (pinID > 46 || pinID < -2 || pinID == 0) { B2WARNING("ARICHBiasChannelsMapping::addMapping: Pin ID number " << pinID << " not valid!"); }
  if (innerID > 9 || innerID < -2 || innerID == 0) { B2WARNING("ARICHBiasChannelsMapping::addMapping: Inner ID " << innerID << " not valid!"); }

  auto pinProp = std::make_tuple(pinID, connectionID, innerID, type);
  std::vector<int> channelMap{crate, slot, channelID};
  m_channel2type.insert(std::pair<std::vector<int>, std::tuple<int, int, int, std::string>>(channelMap, pinProp));

}

void ARICHBiasChannelsMapping::print()
{

  for (int crate = 0; crate < 3 ; crate++) {
    for (int slot = 0; slot < 15 ; slot++) {
      for (int channelID = 0; channelID < N_BIASCHANNELS; channelID++) {
        std::vector<int> channel{crate, slot, channelID};
        cout << " Channel " << channelID << " (crate " << crate << ", slot " << slot << "): pinID " << getPinID(
               channel) << ", inner pin " << std::get<0>(getInnerConnection(channel)) << ", inner type " << std::get<1>(getInnerConnection(
                     channel)) << endl;
      }
    }
  }

}

