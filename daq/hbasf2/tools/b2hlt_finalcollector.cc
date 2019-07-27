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
 * Wrapper around application implemented in ZMQFinalCollector.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQFinalCollector collector;
  collector.initFromConsole("b2hlt_finalcollector - final part of the HLT event transportation receiving events from clients "
                            "(confirmed connection) and sending them out via a raw connection, e.g. to storage.", argc, argv);
  collector.main();

  B2RESULT("Program terminated.");
}