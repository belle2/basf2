/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>

#include <TFile.h>
#include <TTree.h>
#include <TError.h>

#include <sys/signal.h>

#include <boost/program_options.hpp>

#include <string>
#include <iostream>

using namespace std;
using namespace Belle2;
namespace prog = boost::program_options;

int main(int argc, char* argv[])
{
  //remove SIGPIPE handler set by ROOT which sometimes caused infinite loops
  //See https://savannah.cern.ch/bugs/?97991
  //default action is to abort
  if (signal(SIGPIPE, SIG_DFL) == SIG_ERR) {
    B2FATAL("Cannot remove SIGPIPE signal handler");
  }

  // Define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("file,f", prog::value<string>(), "local file name")
  ("lfn,l", prog::value<string>(), "logical file name")
  ("all,a", "print all information")
  ("json", "print machine-readable information in JSON format. Implies --all and --steering.")
  ("steering,s", "print steering file contents")
  ;

  prog::positional_options_description posOptDesc;
  posOptDesc.add("file", -1);

  prog::variables_map varMap;
  prog::store(prog::command_line_parser(argc, argv).
              options(options).positional(posOptDesc).run(), varMap);
  prog::notify(varMap);

  //Check for help option
  if (varMap.count("help") or argc == 1) {
    cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    cout << options << endl;
    return 0;
  }

  FileMetaData metaData;
  FileMetaData* metaDataPtr = &metaData;

  //Check for file option
  if (varMap.count("file")) {
    gErrorIgnoreLevel = kError;
    string fileName = varMap["file"].as<string>();
    TFile* file = TFile::Open(fileName.c_str(), "READ");
    if (!file || !file->IsOpen()) {
      B2ERROR("Couldn't open file " << fileName);
      return 1;
    }
    TTree* tree = (TTree*) file->Get("persistent");
    if (!tree) {
      B2ERROR("No tree persistent found in " << fileName);
      return 1;
    }
    TBranch* branch = tree->GetBranch("FileMetaData");
    if (!branch) {
      B2ERROR("No meta data found in " << fileName);
      return 1;
    }
    metaDataPtr = 0;
    branch->SetAddress(&metaDataPtr);
    tree->GetEntry(0);

  } else if (varMap.count("lfn")) {
    std::string lfn = varMap["lfn"].as<string>();
    if (!FileCatalog::Instance().getMetaData(lfn, metaData)) {
      B2ERROR("No meta data found in file catalog for LFN " << varMap["lfn"].as<int>());
      return 1;
    }

  } else {
    B2ERROR("Please specify either a file name, a unique ID, or a LFN.");
    return 1;
  }

  const char* option = "";
  if (varMap.count("json")) option = "json";
  else if (varMap.count("all")) option = "all";
  metaDataPtr->Print(option);
  if (string(option) != "json" and varMap.count("steering")) metaDataPtr->Print("steering");

  return 0;
}

