/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/rawdata/RawData.h>

/* KLM headers. */
#include <klm/dataobjects/KLMDigitRaw.h>

/* C++ headers. */
#include <cstdint>

using namespace Belle2::KLM;

RawData::RawData(
  int copper, int slot, const int* buffer,
  StoreArray<KLMDigitRaw>* klmDigitRaws,
  KLMDigitRaw** newDigitRaw, bool fillDigitRaws)
{
  uint16_t dataWords[4];
  dataWords[0] = (buffer[0] >> 16) & 0xFFFF;
  dataWords[1] =  buffer[0] & 0xFFFF;
  dataWords[2] = (buffer[1] >> 16) & 0xFFFF;
  dataWords[3] =  buffer[1] & 0xFFFF;
  m_Type = unpackType(dataWords[0]);
  m_Lane = unpackLane(dataWords[0]);
  m_Axis = unpackAxis(dataWords[0]);
  m_Channel = unpackChannel(dataWords[0]);
  m_CTime = unpackCTime(dataWords[1]);
  m_TriggerBits = unpackTriggerBits(dataWords[2]);
  m_TDC = unpackTDC(dataWords[2]);
  m_Charge = unpackCharge(dataWords[3]);
  m_FE = unpackFE(dataWords[3]);
  if (fillDigitRaws) {
    *newDigitRaw = klmDigitRaws->appendNew(copper, slot,
                                           dataWords[0], dataWords[1],
                                           dataWords[2], dataWords[3]);
  }
}

RawData::~RawData()
{
}

void RawData::getChannelGroups(std::vector<ChannelGroup>& channelGroups) const
{
  ChannelGroup group;
  if (multipleStripHit()) {
    int asic = (m_Channel - 1) / c_NChannelsAsic;
    int channelBase = c_NChannelsAsic * asic;
    channelGroups.clear();
    if (m_Type == 0x4) { // for old scintillator-readout firmware with bug
      if ((m_TriggerBits & 0xF) != 0) {
        group.firstChannel = channelBase + 1;
        group.lastChannel = channelBase + c_NChannelsAsic;
        channelGroups.push_back(group);
      }
    } else if ((m_Type == 0x5) && (m_FE != 0)) { // for new scintillator-readout firmware
      group.firstChannel = m_Channel;
      group.lastChannel = 0;
      channelGroups.push_back(group);
    } else if ((m_Type == 0x5) && (m_FE == 0)) { // for new scintillator-readout firmware
      if ((m_TriggerBits & 0x1) != 0) {
        group.firstChannel = channelBase + 1;
        group.lastChannel = channelBase + 4;
        channelGroups.push_back(group);
      }
      if ((m_TriggerBits & 0x2) != 0) {
        group.firstChannel = channelBase + 5;
        group.lastChannel = channelBase + 8;
        channelGroups.push_back(group);
      }
      if ((m_TriggerBits & 0x4) != 0) {
        group.firstChannel = channelBase + 9;
        group.lastChannel = channelBase + 12;
        channelGroups.push_back(group);
      }
      if ((m_TriggerBits & 0x8) != 0) {
        group.firstChannel = channelBase + 13;
        group.lastChannel = channelBase + c_NChannelsAsic;
        channelGroups.push_back(group);
      }
    }
  } else { // for RPC strip or isolated scintillator
    group.firstChannel = m_Channel;
    group.lastChannel = 0;
    channelGroups.push_back(group);
  }
}
