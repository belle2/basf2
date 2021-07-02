/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/dataobjects/FileMetaData.h>

#include <TFile.h>
#include <TTree.h>

#include <boost/program_options.hpp>

#include <csignal>
#include <string>
#include <iostream>

using namespace std;
using namespace Belle2;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  //remove SIGPIPE handler set by ROOT which sometimes caused infinite loops
  //See https://savannah.cern.ch/bugs/?97991
  //default action is to abort
  if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) {
    B2FATAL("Cannot remove SIGPIPE signal handler");
  }

  std::string filename;
  po::options_description description("Options");
  description.add_options()
  ("filename", po::value<std::string>(&filename), "belle 2 mdst file");
  po::variables_map vm;
  po::positional_options_description p;
  p.add("filename", -1);

  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(description).positional(p).run();
    po::store(parsed, vm);

    if (vm.count("help")) {
      std::cout << description << std::endl;
      return 1;
    }
    po::notify(vm);
  } catch (po::error& err) {
    std::cerr << "Error: " << err.what() << "\n";
    return 1;
  }

  FileMetaData metaData;
  FileMetaData* metaDataPtr = &metaData;

  //Check for file option
  TFile* file = TFile::Open(filename.c_str(), "READ");
  if (!file || !file->IsOpen()) {
    B2ERROR("Couldn't open file " << filename);
    return 1;
  }
  TTree* tree = (TTree*) file->Get("persistent");
  if (!tree) {
    B2ERROR("No tree persistent found in " << filename);
    return 1;
  }
  TBranch* branch = tree->GetBranch("FileMetaData");
  if (!branch) {
    B2ERROR("No meta data found in " << filename);
    return 1;
  }

  branch->SetAddress(&metaDataPtr);
  tree->GetEntry(0);
  std::cout << metaDataPtr->getNEvents() << std::endl;

  return 0;
}

