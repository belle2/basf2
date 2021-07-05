/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/dqm/DqmMemFile.h>
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <framework/logging/Logger.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <thread>
#include <chrono>


namespace po = boost::program_options;
using namespace Belle2;

int main(int argc, char* argv[])
{
  std::string dqmFileName;

  po::options_description
  desc("b2hlt_read_histos - helper tool to check the content of a DQM shared memory file repeatedly (every 5s).");
  desc.add_options()
  ("help,h", "Print this help message")
  ("dqmFileName", po::value<std::string>(&dqmFileName)->required(),
   "where to read the shared memory from");

  po::positional_options_description p;

  po::variables_map vm;
  try {
    po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  } catch (std::exception& e) {
    B2FATAL(e.what());
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    exit(1);
  }

  try {
    po::notify(vm);
  } catch (std::exception& e) {
    B2FATAL(e.what());
  }

  Belle2::DqmMemFile file(dqmFileName, "read");
  HLTMainLoop mainLoop;
  while (mainLoop.isRunning()) {

    auto memFile = file.LoadMemFile();
    memFile->GetListOfKeys()->Print();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5s);
  }
}