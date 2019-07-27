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
 * Wrapper around application implemented in ZMQHistogramToFileServer.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQHistogramToFileServer histogramServer;
  histogramServer.initFromConsole("b2hlt_finalhistoserver - final histogram server receiving DQM histograms from clients (confirmed connection) "
                                  "merging them periodically and storing them to a ROOT file as well as a shared memory file (optionally).", argc, argv);
  histogramServer.main();

  B2RESULT("Program terminated.");
}