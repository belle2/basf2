/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *               Simon Wehle                                              *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/RootFileCreationManager.h>
#include <framework/logging/Logger.h>
#include <framework/core/MetadataService.h>
#include <iomanip>

namespace {
  /** Custom delete function for the TFile */
  void TFile_Deleter(TFile* file)
  {
    if (!file) return;
    B2DEBUG(30, "Closing ROOT file");
    if (file->IsOpen()) {
      file->Close();
      Belle2::MetadataService::Instance().addRootNtupleFile(file->GetName());
    }
    delete file;
  }
}

namespace Belle2 {
  std::shared_ptr<TFile> RootFileCreationManager::getFile(std::string fileName, bool ignoreErrors)
  {
    std::shared_ptr<TFile> ptr = m_files[fileName].lock();
    if (!ptr) {
      // make sure stupid gDirectory is not modified ... that means that users
      // have to ptr->cd() but plays nice with other modules which might not
      // set their cd() correctly.
      TDirectory::TContext gDirectoryGuard;
      // Create shared ptr for the file which will correctly close it when the last user disconnects.
      ptr = std::shared_ptr<TFile>(TFile::Open(fileName.c_str(), "RECREATE"), TFile_Deleter);
      // Check if the file is actually open ... otherwise no use in returning it
      if (!ptr || !ptr->IsOpen()) {
        if (!ignoreErrors) B2ERROR("Could not create file " << std::quoted(fileName));
        return nullptr;
      }
      //remember this ...
      m_files[fileName] = ptr;
    }
    return ptr;
  }

  RootFileCreationManager& RootFileCreationManager::getInstance()
  {
    static RootFileCreationManager instance;
    return instance;
  }
}
