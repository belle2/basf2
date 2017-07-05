/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
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

#include <boost/program_options.hpp>

#include <string>
#include <iostream>

using namespace std;
using namespace Belle2;
namespace prog = boost::program_options;

int main(int argc, char* argv[])
{
  // define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("file", prog::value<string>(), "file name")
  ("lfn,l", prog::value<string>(), "logical file name")
  ;

  prog::positional_options_description posOptDesc;
  posOptDesc.add("file", -1);

  prog::variables_map varMap;
  prog::store(prog::command_line_parser(argc, argv).
              options(options).positional(posOptDesc).run(), varMap);
  prog::notify(varMap);

  // check for help option
  if (varMap.count("help")) {
    cout << "Usage: " << argv[0] << " [OPTIONS] [FILE]\n";
    cout << "Add/edit LFN in given file, also update file catalog.\n\n";
    cout << options << endl;
    return 0;
  }

  // check parameters
  for (auto param : {"file", "lfn"}) {
    if (!varMap.count(param)) {
      B2ERROR("The " << param << " parameter is missing.");
      return 1;
    }
  }

  // read parameters
  string fileName = varMap["file"].as<string>();
  string lfn = varMap["lfn"].as<string>();

  // open the root file
  gErrorIgnoreLevel = kError;
  TFile* file = TFile::Open(fileName.c_str(), "UPDATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Failed to open the file " << fileName);
    return 1;
  }

  // read the FileMetaData object or create a new one if it doesn't exist
  FileMetaData* fileMetaData = 0;
  TTree* tree = (TTree*) file->Get("persistent");
  TTree* newTree = 0;
  if (!tree) {
    fileMetaData = dynamic_cast<FileMetaData*>(file->Get("FileMetaData"));
    if (!fileMetaData) {
      B2WARNING("Failed to get persistent tree in the file " << fileName);
      tree = new TTree("persistent", "persistent");
      fileMetaData = new FileMetaData;
      tree->Branch("FileMetaData", &fileMetaData);
      newTree = tree;
    }
  } else {
    tree->SetBranchAddress("FileMetaData", &fileMetaData);
    newTree = tree->CloneTree(0);
    tree->GetEntry(0);
  }

  // update the IDs and write the updated FileMetaData to the file
  fileMetaData->setLfn(lfn);
  if (newTree) {
    newTree->Fill();
    newTree->Write(newTree->GetName(), TObject::kWriteDelete);
  } else {
    fileMetaData->Write("FileMetaData");
  }

  // properly close file
  file->Close();

  // update the local file catalog
  FileMetaData localMetaData = *fileMetaData;
  FileCatalog::Instance().registerFile(fileName, localMetaData);

  return 0;
}

