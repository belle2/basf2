/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/dqm/DqmMemFile.h>
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

#include <daq/hbasf2/utils/HLTStreamHelper.h>
#include <TH1.h>

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>


namespace po = boost::program_options;
using namespace Belle2;

int main(int argc, char* argv[])
{
  std::string outputFileName;

  po::options_description
  desc("b2hlt_create_histos - helper tool to create a single histogram raw file for testing.");
  desc.add_options()
  ("help,h", "Print this help message")
  ("output", po::value<std::string>(&outputFileName)->required(),
   "where to store the histograms to");

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

  DataStore::Instance().setInitializeActive(true);
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);

  eventMetaData.create();

  TH1F histogram("my_histogram", "Some title", 1, 0, 1);
  histogram.Fill(0.5);
  histogram.Fill(0.5);

  HLTStreamHelper streamHelper;
  auto message = streamHelper.streamHistograms();
  const auto& histogramMessage = message->getDataMessage();

  std::ofstream outfile(outputFileName, std::ofstream::binary);
  outfile.write(histogramMessage.data<char>(), histogramMessage.size());
  outfile.close();
}
