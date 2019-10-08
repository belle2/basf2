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

/* C++ headers. */
#include <cstdint>

using namespace Belle2;

void KLM::unpackRawData(
  const int* buffer, KLM::RawData* data, StoreArray<KLMDigitRaw>* klmDigitRaws,
  KLMDigitRaw** newDigitRaw, bool fillDigitRaws)
{
  uint16_t dataWords[4];
  dataWords[0] = (buffer[0] >> 16) & 0xFFFF;
  dataWords[1] =  buffer[0] & 0xFFFF;
  dataWords[2] = (buffer[1] >> 16) & 0xFFFF;
  dataWords[3] =  buffer[1] & 0xFFFF;
  data->lane = (dataWords[0] >> 8) & 0x1F;
  data->axis = (dataWords[0] >> 7) & 0x1;
  data->channel = dataWords[0] & 0x7F;
  data->ctime = dataWords[1];
  data->triggerBits = (dataWords[2] >> 11) & 0x1F;
  data->tdc = dataWords[2] & 0x7FF;
  data->charge = dataWords[3] & 0xFFF;
  if (fillDigitRaws) {
    *newDigitRaw = klmDigitRaws->appendNew(dataWords[0], dataWords[1],
                                           dataWords[2], dataWords[3]);
  }
}
