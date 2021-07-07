/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/RingBuffer.h>
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/EvtMessage.h>

#include <boost/program_options.hpp>
#include <iostream>

#define MAXEVTSIZE 80000000
#define RBUFSIZE  10000000


using namespace Belle2;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  std::vector<std::string> inputRingBufferNames;
  std::vector<std::string> outputRingBufferNames;

  po::options_description desc("b2hlt_rb2rb");
  desc.add_options()
  ("help,h", "Print this help message")
  ("input-ring-buffer-name,r", po::value<std::vector<std::string>>(&inputRingBufferNames)->required(),
   "name of the input ring buffers")
  ("output-ring-buffer-name,r", po::value<std::vector<std::string>>(&outputRingBufferNames)->required(),
   "name of the output ring buffers");

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  } catch (std::exception& e) {
    B2FATAL(e.what());
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  try {
    po::notify(vm);
  } catch (std::exception& e) {
    B2FATAL(e.what());
  }

  if (outputRingBufferNames.empty() or inputRingBufferNames.empty()) {
    B2FATAL("Need at least one output and input ring buffer!");
  }

  // TODO: delete or not?
  std::vector<RingBuffer*> inputRingBuffers;
  for (const std::string& bufferName : inputRingBufferNames) {
    inputRingBuffers.push_back(new RingBuffer(bufferName.c_str(), RBUFSIZE));
    inputRingBuffers.back()->dump_db();
  }

  std::vector<RingBuffer*> outputRingBuffers;
  for (const std::string& bufferName : outputRingBufferNames) {
    outputRingBuffers.push_back(new RingBuffer(bufferName.c_str(), RBUFSIZE));
    outputRingBuffers.back()->dump_db();
  }

  HLTMainLoop mainLoop;

  int* buffer = new int[MAXEVTSIZE];
  int nevt = 0;

  auto inputRingBuffer = inputRingBuffers.begin();
  auto outputRingBuffer = outputRingBuffers.begin();

  while (mainLoop.isRunning()) {
    // Read from ring buffer
    const int size = mainLoop.readFromRingBufferWaiting(*inputRingBuffer, buffer);
    // Error checking ring buffer
    if (size <= 0) {
      if (mainLoop.isRunning()) {
        B2ERROR("Writing to the ring buffer failed!");
      }
      // This is fine if we are terminating anyways
      break;
    }
    B2ASSERT("Size is negative! This should be handled above. Not good!", size > 0);

    // Write to ring buffer
    const int returnValue = mainLoop.writeToRingBufferWaiting(*outputRingBuffer, buffer, size);
    // Error check ring buffer
    if (returnValue <= 0) {
      if (mainLoop.isRunning()) {
        B2ERROR("Writing to the ring buffer failed!");
      }
      // This is fine if we are terminating anyways
      break;
    }
    B2ASSERT("Written size is negative! This should be handled above. Not good!", returnValue > 0);

    // Logging
    nevt++;
    if (nevt % 5000 == 0) {
      B2RESULT("b2hlt_rb2socket event number: " << nevt);
    }

    inputRingBuffer++;
    if (inputRingBuffer == inputRingBuffers.end()) {
      inputRingBuffer = inputRingBuffers.begin();
    }
    outputRingBuffer++;
    if (outputRingBuffer == outputRingBuffers.end()) {
      outputRingBuffer = outputRingBuffers.begin();
    }
  }

  B2RESULT("Program terminated.");
}
