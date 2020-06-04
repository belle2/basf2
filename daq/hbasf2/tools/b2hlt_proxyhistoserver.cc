/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQHistogramServer.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQHistogramToZMQServer.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQHistogramToZMQServer histogramServer;
  histogramServer.initFromConsole("b2hlt_proxyhistoserver - non-final histogram server receiving DQM histograms on the input (confirmed connection), "
                                  "merging them and sending the result to the output (also a confirmed connection). Useful for hierarchical structures.",
                                  argc, argv);
  histogramServer.main();

  B2RESULT("Program terminated.");
}