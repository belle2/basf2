/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQCollector.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQCollector.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQCollector collector;
  collector.initFromConsole("b2hlt_collector - non-final event collector receiving events on the input (confirmed connection), "
                            "and forwarding them to the output (via a load-balanced connection) to clients. "
                            "Useful for stacking multiple reconstruction worker paths.", argc, argv);
  collector.main();

  B2RESULT("Program terminated.");
}