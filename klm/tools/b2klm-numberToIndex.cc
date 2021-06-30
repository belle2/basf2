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
#include <iostream>

using namespace Belle2;

int main(int argc, char* argv[])
{
  /* Print the usage message if --help or -h are used. */
  if (argc == 1 or std::string(argv[1]) == "--help" or std::string(argv[1]) == "-h") {
    std::cout << "Usage: " << argv[0] << " [CHANNEL1] [CHANNEL2] ... [CHANNELN]\n\n"
              "   This tool converts the given channel numbers into the corresponding channel indexes.\n"
              "   A basf2 FATAL message is printed if a channel number does not exist.\n";
    return 0;
  }
  /* Print the error messages when needed. */
  int nChannels = argc - 1;
  /* cppcheck-suppress knownConditionTrueFalse */
  if (nChannels == 0) {
    B2ERROR("There are no channels to convert.");
    return 0;
  }
  /* Convert the channel number into the channel index. */
  const KLMChannelArrayIndex* channelArrayIndex = &(KLMChannelArrayIndex::Instance());
  for (int i = 1; i <= nChannels; ++i) {
    KLMChannelNumber number = std::atoi(argv[i]);
    uint16_t index = channelArrayIndex->getIndex(number);
    B2INFO("Channel number: " << number << "  ==> Channel index: " << index);
  }
  return 0;
}
