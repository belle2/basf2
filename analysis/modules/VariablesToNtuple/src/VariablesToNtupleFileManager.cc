/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*               Simon Wehle                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/VariablesToNtuple/VariablesToNtupleFileManager.h>

using namespace Belle2;

/** Custom delete funktion for the TFile */
void deleter(TFile* file)
{
  B2INFO("Deleting file");
  if (file->IsOpen()) {
    file->Close();
  }
  delete file;
}

std::shared_ptr<TFile> VariablesToNtupleFileManager::getFile(std::string fileName)
{
  if (m_files.find(fileName) != m_files.end()) {
    return m_files[fileName];
  } else {
    return createRootFile(fileName);
  }
}

std::shared_ptr<TFile> VariablesToNtupleFileManager::createRootFile(std::string fileName)
{
  m_files[fileName] = std::shared_ptr<TFile>(new TFile(fileName.c_str(), "RECREATE"), deleter);
  if (!m_files[fileName]->IsOpen()) {
    B2ERROR("Could not create file \"" << fileName <<
            "\". Please set a vaild root output file name (\"fileName\" module parameter).");
    return nullptr;
  }
  return m_files[fileName];
}


