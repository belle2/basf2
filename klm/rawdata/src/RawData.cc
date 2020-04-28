/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/rawdata/RawData.h>

/* KLM headers. */
#include <klm/dataobjects/KLMDigitRaw.h>

/* C++ headers. */
#include <cstdint>

using namespace Belle2;

void KLM::unpackRawData(
  int copper, int slot, const int* buffer, KLM::RawData* data,
  StoreArray<KLMDigitRaw>* klmDigitRaws,
  KLMDigitRaw** newDigitRaw, bool fillDigitRaws)
{
  uint16_t dataWords[4];
  dataWords[0] = (buffer[0] >> 16) & 0xFFFF;
  dataWords[1] =  buffer[0] & 0xFFFF;
  dataWords[2] = (buffer[1] >> 16) & 0xFFFF;
  dataWords[3] =  buffer[1] & 0xFFFF;
  data->lane = RawData::unpackLane(dataWords[0]);
  data->axis = RawData::unpackAxis(dataWords[0]);
  data->channel = RawData::unpackChannel(dataWords[0]);
  data->ctime = RawData::unpackCtime(dataWords[1]);
  data->triggerBits = RawData::unpackTriggerBits(dataWords[2]);
  data->tdc = RawData::unpackTdc(dataWords[2]);
  data->charge =  RawData::unpackCharge(dataWords[3]);
  if (fillDigitRaws) {
    *newDigitRaw = klmDigitRaws->appendNew(copper, slot,
                                           dataWords[0], dataWords[1],
                                           dataWords[2], dataWords[3]);
  }
}
