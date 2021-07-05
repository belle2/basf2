/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <daq/rfarm/event/hltsocket/HLTFile.h>
#include <daq/rfarm/event/hltsocket/HLTSocket.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/EvtMessage.h>

#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>

#define MAXEVTSIZE 80000000

using namespace Belle2;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  std::string ringBufferName;
  unsigned int port;
  std::string sourceHost;
  bool raw;
  bool repeat;
  std::string file_name;

  po::options_description desc("b2hlt_file2socket FILE-NAME PORT SHM-NAME SHM-ID");
  desc.add_options()
  ("help,h", "Print this help message")
  ("port,p", po::value<unsigned int>(&port)->required(), "port number to connect or listen to")
  ("file-name,f", po::value<std::string>(&file_name)->required(), "file name to write to")
  ("raw", po::bool_switch(&raw)->default_value(false), "send and receive raw data instead of event buffers")
  ("repeat", po::bool_switch(&repeat)->default_value(false), "repeat after the file is finished");


  po::positional_options_description p;
  p.add("file-name", 1).add("port", 1);

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

  int* buffer = new int[MAXEVTSIZE];

  HLTMainLoop mainLoop;

  HLTSocket socket;
  HLTFile file;
  int size;
  int returnValue;
  int nevt = 0;
  bool terminate = false;

  if (not file.open(file_name, raw, "r")) {
    B2ERROR("Can not open file");
    terminate = true;
  }


  auto start = std::chrono::steady_clock::now();
  while (mainLoop.isRunning() and not terminate) {
    // Connect socket if needed
    if (not socket.initialized()) {
      if (vm.count("connect-to")) {
        if (not socket.connect(sourceHost, port, mainLoop)) {
          B2ERROR("Could not reconnnect!");
          break;
        }
      } else {
        if (not socket.accept(port)) {
          B2ERROR("Could not reconnect!");
          break;
        }
      }
      B2RESULT("Connected.");
      start = std::chrono::steady_clock::now();
    }

    // Read from file
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

    if (raw) {
      returnValue = socket.put_wordbuf(buffer, size);
    } else {
      EvtMessage message(reinterpret_cast<char*>(buffer));
      returnValue = socket.put(message.buffer(), message.size());
      // Terminate messages make us terminate
      if (message.type() == MSG_TERMINATE) {
        B2RESULT("Having received terminate message");
        terminate = true;
      }
    }
    if (returnValue == 0) {
      B2ERROR("Error in sending the event! Reconnecting.");
      socket.deinitialize();
      continue;
    } else if (returnValue < 0) {
      if (mainLoop.isRunning()) {
        B2ERROR("Error in sending the event! Aborting.");
      }
      // This is fine if we are terminating anyways
      break;
    }
    B2ASSERT("Written size is negative! This should be handled above. Not good!", returnValue > 0);

    // Logging
    nevt++;
    if (nevt % 5000 == 0) {
      auto current = std::chrono::steady_clock::now();
      double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(current - start).count();
      B2RESULT("b2hlt_file2socket event number: " << nevt << " with a rate of " << 5000 / elapsed << " Hz");
      start = std::chrono::steady_clock::now();
    }
  }

  B2RESULT("Program terminated.");
}
