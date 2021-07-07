/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQHistogramServer.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQHistogramToRawOutput.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQHistogramToRawServer histogramServer;
  histogramServer.initFromConsole("b2hlt_rawhistoserver - final histogram server receiving DQM histograms from clients (confirmed connection) "
                                  "merging them periodically and send them out to a legacy histogram server.", argc, argv);
  histogramServer.main();

  B2RESULT("Program terminated.");
}