/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/random/RandomSeedRestoreModule.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/core/Framework.h>

#include <TFile.h>
#include <TTree.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RandomSeedRestore)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RandomSeedRestoreModule::RandomSeedRestoreModule() : Module()
{
  // Set module properties
  setDescription("Restores the random number generator seed from a file.");

  // Parameter definition
  addParam("FileName", m_fileName, "The file that contains the random number seed.");
}


void RandomSeedRestoreModule::initialize()
{
  // Open the file and get the FileMetaData branch
  TFile* file = TFile::Open(m_fileName.c_str());
  if (!file or !file->IsOpen()) {
    B2ERROR("The file " << m_fileName << " could not be opened.");
    return;
  }
  TTree* tree = static_cast<TTree*>(file->Get("persistent"));
  if (!tree) {
    B2ERROR("The file " << m_fileName << " does not contain a persistent tree.");
    return;
  }
  TBranch* branch = tree->GetBranch("FileMetaData");
  if (!branch) {
    B2ERROR("The file " << m_fileName << " does not contain a FileMetaData object.");
    return;
  }

  // Read the FileMetaData object
  FileMetaData* fileMetaData = 0;
  branch->SetAddress(&fileMetaData);
  branch->GetEntry(0);
  if (!fileMetaData) {
    B2ERROR("Reading the FileMetaData from file " << m_fileName << " failed.");
    return;
  }

  // Restore the random number generator
  *static_cast<TRandom3*>(gRandom) = fileMetaData->getRandom();
  Framework::resetInitialRandom(fileMetaData->getRandomSeed());
  B2INFO("The random number seed was restored from the file " << m_fileName << ".");

  // Clean up
  delete file;
}


