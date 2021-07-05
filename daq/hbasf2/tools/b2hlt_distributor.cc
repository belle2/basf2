/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQDistributor.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQDistributor.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQDistributor distributor;
  distributor.initFromConsole("b2hlt_distributor - first step in the HLT data transportation. Receives events via a "
                              "raw connection (e.g. from event builder) and sends them via a load-balanced connection to connected clients.",
                              argc, argv);
  distributor.main();

  B2RESULT("Program terminated.");
}
