/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQCollector.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQProxyCollector.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQProxyCollector collector;
  collector.initFromConsole("b2hlt_proxycollector - special, non-final event collector receiving events on the input (confirmed connection), "
                            "and forwards them to the output (also via a confirmed connection!). Useful for hierarchical structures.", argc, argv);
  collector.main();

  B2RESULT("Program terminated.");
}