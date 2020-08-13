/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQClasses.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/**
 * HLT-ZMQ Tool as part of the data transportation suite.
 * Wrapper around application implemented in ZMQReadySender.
 * For a description of the functionality see there.
 */
int main(int argc, char* argv[])
{
  ZMQReadySender readySender;
  readySender.initFromConsole("b2hlt_readysender - mimick a final sink by sending a ready message on every received event/message (load-balanced connection).",
                              argc, argv);
  readySender.main();

  B2RESULT("Program terminated.");
}