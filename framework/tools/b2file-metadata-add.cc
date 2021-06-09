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
  std::string fileName;
  std::string lfn;
  std::vector<std::string> dataDescriptions;
  // define command line options
  prog::options_description options("Options");
  options.add_options()
  ("help,h", "print all available options")
  ("file", prog::value<string>(&fileName), "file name")
  ("lfn,l", prog::value<string>(&lfn), "logical file name")
  ("description,d", prog::value<std::vector<std::string>>(&dataDescriptions),
   "data description to set of the form key=value. If the argument does not contain an equal sign it's interpeted as a key to delete from the dataDescriptions")
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
    cout << "Add/edit LFN in given file or modify the data description stored in the file, also update file catalog.\n\n";
    cout << options << endl;
    return 0;
  }

  // check parameters
  if (!varMap.count("file")) {
    B2ERROR("The filename is missing.");
    return 1;
  }
  if (!varMap.count("lfn") && !varMap.count("description")) {
    B2ERROR("Neither lfn nor data descriptions to be modified, nothing to do");
    return 1;
  }

  // open the root file
  gErrorIgnoreLevel = kError;
  TFile* file = TFile::Open(fileName.c_str(), "UPDATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Failed to open the file " << fileName);
    return 1;
  }

  // read the FileMetaData object or create a new one if it doesn't exist
  FileMetaData* fileMetaData = nullptr;
  auto* tree = dynamic_cast<TTree*>(file->Get("persistent"));
  TTree* newTree = nullptr;
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

  if (!fileMetaData) {
    B2ERROR("Failed to load FileMetaData from file " << fileName);
    return 1;
  }

  // remember old lfn in case this file was registered in file catalog
  const std::string oldLFN = fileMetaData->getLfn();

  // update the IDs and write the updated FileMetaData to the file
  if (varMap.count("lfn")) fileMetaData->setLfn(lfn);
  if (!dataDescriptions.empty()) {
    for (const auto& keyvalue : dataDescriptions) {
      size_t pos = keyvalue.find('=');
      if (pos == std::string::npos) {
        // no '=' in -d argument, assume deletion
        fileMetaData->removeDataDescription(keyvalue);
      } else {
        const std::string key = keyvalue.substr(0, pos);
        const std::string value = keyvalue.substr(pos + 1);
        fileMetaData->setDataDescription(key, value);
      }
    }
  }
  if (newTree) {
    newTree->Fill();
    newTree->Write(newTree->GetName(), TObject::kWriteDelete);
  } else {
    fileMetaData->Write("FileMetaData");
  }

  // properly close file
  file->Close();

  // update the local file catalog but only *if* the file was already registered
  std::string oldPFN = oldLFN;
  FileMetaData localMetaData;
  if (FileCatalog::Instance().getMetaData(oldPFN, localMetaData)) {
    localMetaData  = *fileMetaData;
    FileCatalog::Instance().registerFile(fileName, localMetaData, oldLFN);
  }
  return 0;
}
