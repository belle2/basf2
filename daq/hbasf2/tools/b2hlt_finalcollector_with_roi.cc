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
 * Wrapper around application implemented in ZMQFinalCollectorWithROI.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQFinalCollectorWithROI collector;
  collector.initFromConsole("b2hlt_finalcollector_with_roi - final part of the HLT event transportation receiving events from clients "
                            "(confirmed connection) and sending them out via two raw connections, e.g. to storage and ROI.", argc, argv);
  collector.main();

  B2RESULT("Program terminated.");
}