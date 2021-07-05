/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQClasses.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQAcceptor.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQAcceptor collector;
  collector.initFromConsole("b2hlt_acceptor - mimick a final sink by confirming each received input (confirmed connection).", argc,
                            argv);
  collector.main();

  B2RESULT("Program terminated.");
}