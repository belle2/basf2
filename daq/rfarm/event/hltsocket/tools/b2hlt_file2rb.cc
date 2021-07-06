/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rfarm/manager/RFFlowStat.h>
#include <framework/pcore/RingBuffer.h>
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <daq/rfarm/event/hltsocket/HLTFile.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/EvtMessage.h>

#include <boost/program_options.hpp>
#include <iostream>

#define MAXEVTSIZE 80000000

using namespace Belle2;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  std::string ringBufferName;
  std::string shmName;
  unsigned int shmID;
  bool raw;
  bool repeat;
  std::string file_name;

  po::options_description desc("b2hlt_file2rb FILE-NAME RING-BUFFER-NAME SHM-NAME SHM-ID");
  desc.add_options()
  ("help,h", "Print this help message")
  ("ring-buffer-name,r", po::value<std::string>(&ringBufferName)->required(), "name of the ring buffer")
  ("file-name,f", po::value<std::string>(&file_name)->required(), "file name to write to")
  ("shm-name,n", po::value<std::string>(&shmName)->required(), "name of the shm for flow output")
  ("shm-id,i", po::value<unsigned int>(&shmID)->required(), "id in the shm for flow output")
  ("raw", po::bool_switch(&raw)->default_value(false), "send and receive raw data instead of event buffers")
  ("repeat", po::bool_switch(&repeat)->default_value(false), "repeat after the file is finished");


  po::positional_options_description p;
  p.add("file-name", 1).add("ring-buffer-name", 1).add("shm-name", 1).add("shm-id", 1);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
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

  // TODO: delete or not?
  RingBuffer* ringBuffer = new RingBuffer(ringBufferName.c_str());
  RFFlowStat flow((char*)shmName.c_str(), shmID, ringBuffer);
  int* buffer = new int[MAXEVTSIZE];

  HLTMainLoop mainLoop;

  HLTFile file;
  int size;
  int nevt = 0;
  bool terminate = false;

  if (not file.open(file_name, raw, "r")) {
    B2ERROR("Can not open file");
    terminate = true;
  }

  while (mainLoop.isRunning() and not terminate) {
    // Read from socket
    if (raw) {
      size = file.get_wordbuf(buffer, MAXEVTSIZE);
    } else {
      size = file.get(reinterpret_cast<char*>(buffer), MAXEVTSIZE);
    }
    // Error checking socket
    if (size == 0) {
      if (repeat) {
        file.open(file_name, raw, "r");
        continue;
      } else {
        B2RESULT("Reached end of file");
        break;
      }
    } else if (size < 0) {
      if (mainLoop.isRunning()) {
        B2ERROR("Error in receiving the event! Aborting.");
      }
      // This is fine if we are terminating anyways
      break;
    }
    B2ASSERT("Size is negative! This should be handled above. Not good!", size > 0);

    // Terminate messages make us terminate
    if (not raw) {
      EvtMessage message(reinterpret_cast<char*>(buffer));
      if (message.type() == MSG_TERMINATE) {
        B2RESULT("Having received terminate message");
        terminate = true;
      }
    }

    // Monitoring
    flow.log(size * sizeof(int));

    // Write to ring buffer
    const int returnValue = mainLoop.writeToRingBufferWaiting(ringBuffer, buffer, size);
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
      B2RESULT("b2hlt_file2rb event number: " << nevt);
    }
  }

  B2RESULT("Program terminated.");
}
