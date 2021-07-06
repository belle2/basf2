/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>

#include <TFile.h>
#include <TTree.h>
#include <TError.h>

#include <boost/program_options.hpp>
#include <string>
#include <iostream>

using namespace std;
using namespace Belle2;
namespace prog = boost::program_options;

int main(int argc, char* argv[])
{
  // Define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("file,f", prog::value<string>(), "local file name")
  ;

  prog::positional_options_description posOptDesc;
  posOptDesc.add("file", -1);

  prog::variables_map varMap;
  prog::store(prog::command_line_parser(argc, argv).
              options(options).positional(posOptDesc).run(), varMap);
  prog::notify(varMap);

  // Check for help option
  if (varMap.count("help")) {
    cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    cout << "Add the given file to the file catalog (e.g. so it can be found as a parent file).\n\n";
    cout << options << endl;
    return 0;
  }

  // Check for file option
  if (!varMap.count("file")) {
    B2ERROR("No file name given");
    return 1;
  }
  string fileName = varMap["file"].as<string>();

  // Open file and read metadata
  gErrorIgnoreLevel = kError;
  TFile* file = TFile::Open(fileName.c_str(), "READ");
  if (!file || !file->IsOpen()) {
    B2ERROR("Couldn't open file " << fileName);
    return 1;
  }
  auto* tree = (TTree*) file->Get("persistent");
  if (!tree) {
    B2ERROR("No tree persistent found in " << fileName);
    return 1;
  }
  TBranch* branch = tree->GetBranch("FileMetaData");
  if (!branch) {
    B2ERROR("No meta data found in " << fileName);
    return 1;
  }
  FileMetaData* metaData = nullptr;
  branch->SetAddress(&metaData);
  tree->GetEntry(0);

  // Register file in metadata catalog
  if (!FileCatalog::Instance().registerFile(fileName, *metaData)) {
    B2ERROR("Registration of file " << fileName << " failed");
    return 1;
  }

  return 0;
}

