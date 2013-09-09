/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>
#include <framework/utilities/RegisterPythonModule.h>

#include <framework/pybasf2/Framework.h>
#include <framework/pybasf2/LogPythonInterface.h>
#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/ModuleStatistics.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/ModuleParamInfoPython.h>
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>

#include <TFile.h>
#include <TTree.h>

using namespace Belle2;
using namespace boost::python;


FileMetaData updateFileMetaData(const std::string& fileName, int id, const std::string& guid, const std::string& lfn, int logFile)
{
  // open the root file
  TFile* file = TFile::Open(fileName.c_str(), "UPDATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Failed to open the file " << fileName);
    return FileMetaData();
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
  fileMetaData->setIds(id, guid, lfn, logFile);
  if (newTree) {
    newTree->Fill();
    newTree->Write();
  } else {
    fileMetaData->Write("FileMetaData");
  }

  // update the local file catalog
  FileMetaData localMetaData = *fileMetaData;
  FileCatalog::Instance().registerFile(fileName, localMetaData);

  return *fileMetaData;
}

//-----------------------------------
//   Define the pybasf2 python module
//-----------------------------------
BOOST_PYTHON_MODULE(pybasf2)
{
  Module::exposePythonAPI();
  Path::exposePythonAPI();
  Framework::exposePythonAPI();
  ModuleParamInfoPython::exposePythonAPI();
  ModuleStatistics::exposePythonAPI();
  LogPythonInterface::exposePythonAPI();
  RandomNumbers::exposePythonAPI();
  FileMetaData::exposePythonAPI();
  def("update_file_metadata", &updateFileMetaData);
}

//register the module during library load
REGISTER_PYTHON_MODULE(pybasf2)

