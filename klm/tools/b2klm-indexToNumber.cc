/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMChannelArrayIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <cstdlib>

using namespace Belle2;

int main(int argc, char* argv[])
{
  int nChannels = argc - 1;
  if (nChannels == 0) {
    B2ERROR("There are no channels to convert.");
    return 0;
  }
  const KLMChannelArrayIndex* channelArrayIndex = &(KLMChannelArrayIndex::Instance());
  for (int i = 1; i <= nChannels; ++i) {
    uint16_t index = std::atoi(argv[i]);
    uint16_t number = channelArrayIndex->getNumber(index);
    B2INFO("Channel index: " << index << "  ==> Channel number: " << number);
  }
  return 0;
}
