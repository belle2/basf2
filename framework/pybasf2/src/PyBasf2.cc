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
#include <framework/pybasf2/ProcessStatisticsPython.h>
#include <framework/core/Module.h>
#include <framework/core/Path.h>
#include <framework/core/PyObjROOTUtils.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/ModuleParamInfoPython.h>
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/database/Database.h>
#include <framework/io/RootFileInfo.h>

#include <TFile.h>
#include <TTree.h>

using namespace Belle2;
using namespace boost::python;


FileMetaData updateFileMetaData(const std::string& fileName, const std::string& lfn)
{
  // open the root file
  TFile* file = TFile::Open(fileName.c_str(), "UPDATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Failed to open the file " << fileName);
    return FileMetaData();
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
    return FileMetaData();
  }

  // update the IDs and write the updated FileMetaData to the file
  const std::string oldLFN = fileMetaData->getLfn();
  fileMetaData->setLfn(lfn);
  if (newTree) {
    newTree->Fill();
    newTree->Write();
  } else {
    fileMetaData->Write("FileMetaData");
  }

  // update the local file catalog but only *if* the file was already registered
  std::string oldPFN = oldLFN;
  FileMetaData localMetaData;
  if (FileCatalog::Instance().getMetaData(oldPFN, localMetaData)) {
    localMetaData  = *fileMetaData;
    FileCatalog::Instance().registerFile(fileName, localMetaData, oldLFN);
  }
  return *fileMetaData;
}

object getFileMetadata(const std::string& filename)
{
  RootIOUtilities::RootFileInfo fileInfo(filename);
  return createROOTObjectPyCopy(fileInfo.getFileMetaData());
}

//-----------------------------------
//   Define the pybasf2 python module
//-----------------------------------
BOOST_PYTHON_MODULE(pybasf2)
{
  Module::exposePythonAPI();
  ModuleCondition::exposePythonAPI();
  Path::exposePythonAPI();
  Framework::exposePythonAPI();
  ModuleParamInfoPython::exposePythonAPI();
  ProcessStatisticsPython::exposePythonAPI();
  LogPythonInterface::exposePythonAPI();
  RandomNumbers::exposePythonAPI();
  Database::exposePythonAPI();
  FileMetaData::exposePythonAPI();

  //don't show c++ signature in python doc to keep it simple
  docstring_options options(true, true, false);
  def("update_file_metadata", &updateFileMetaData);
  def("get_file_metadata", &getFileMetadata, R"DOC(
Return the FileMetaData object for the given output file.
)DOC");
}

//register the module during library load
REGISTER_PYTHON_MODULE(pybasf2)
