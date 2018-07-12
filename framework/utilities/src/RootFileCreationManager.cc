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
#include <iomanip>

namespace {
  /** Custom delete funktion for the TFile */
  void TFile_Deleter(TFile* file)
  {
    B2DEBUG(30, "Closing ROOT file");
    if (file->IsOpen()) {
      file->Close();
    }
    delete file;
  }
}

namespace Belle2 {
  std::shared_ptr<TFile> RootFileCreationManager::getFile(std::string fileName, bool ignoreErrors)
  {
    auto ptr = m_files[fileName].lock();
    if (!ptr) {
      ptr = std::shared_ptr<TFile>(TFile::Open(fileName.c_str(), "RECREATE"), TFile_Deleter);
      m_files[fileName] = ptr;
    }
    if (!ptr->IsOpen()) {
      if (!ignoreErrors) B2ERROR("Could not create file " << std::quoted(fileName));
      return nullptr;
    }
    return ptr;
  }

  RootFileCreationManager& RootFileCreationManager::getInstance()
  {
    static RootFileCreationManager instance;
    return instance;
  }
}
