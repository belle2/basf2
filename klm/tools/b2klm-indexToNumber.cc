/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
              "   This tool converts the given channel indexes into the corresponding channel numbers.\n"
              "   A basf2 FATAL message is printed if a channel index does not exist.\n";
    return 0;
  }
  /* Print the error messages when needed. */
  int nChannels = argc - 1;
  /* cppcheck-suppress knownConditionTrueFalse */
  if (nChannels == 0) {
    B2ERROR("There are no channels to convert.");
    return 0;
  }
  /* Convert the channel index into the channel number. */
  const KLMChannelArrayIndex* channelArrayIndex = &(KLMChannelArrayIndex::Instance());
  for (int i = 1; i <= nChannels; ++i) {
    uint16_t index = std::atoi(argv[i]);
    KLMChannelNumber number = channelArrayIndex->getNumber(index);
    B2INFO("Channel index: " << index << "  ==> Channel number: " << number);
  }
  return 0;
}
