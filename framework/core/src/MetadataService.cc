/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/MetadataService.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/Utils.h>
#include <fstream>

#include <iostream>

using namespace Belle2;


MetadataService::MetadataService()
{
  m_json["basf2_apiversion"] = 1;
}

MetadataService& MetadataService::Instance()
{
  static MetadataService instance;
  return instance;
}

void MetadataService::addRootOutputFile(const std::string& fileName, const FileMetaData* metaData)
{
  if (!FileSystem::isFile(fileName)) return;

  nlohmann::json file_json = {{"type", "RootOutput"}, {"filename", fileName}};

  if (metaData) {
    file_json["metadata"] = metaData->getJson();
  }

  try {
    std::string check = Utils::getCommandOutput("b2file-check --json " + fileName);
    file_json.merge_patch(nlohmann::json::parse(check));
  } catch (...) {}

  file_json["checksums"]["md5"] = FileSystem::calculateMD5(fileName);
  // no sha256 yet

  m_json["output_files"].push_back(file_json);

  writeJson();
}

void MetadataService::addRootNtupleFile(const std::string& fileName)
{
  if (!FileSystem::isFile(fileName)) return;

  nlohmann::json file_json = {{"type", "RootNtuple"}, {"filename", fileName}};

  // no metadata and no check

  file_json["checksums"]["md5"] = FileSystem::calculateMD5(fileName);
  // no sha256 yet

  m_json["output_files"].push_back(file_json);

  writeJson();
}

void MetadataService::finishBasf2(bool success)
{
  m_json["basf2_status"]["finished"] = true;
  m_json["basf2_status"]["success"] = success;
  m_json["basf2_status"]["message"] = "finished successfully";

  writeJson();
}

void MetadataService::writeJson()
{
  if (m_fileName.empty()) return;
  std::ofstream jsonFile(m_fileName.c_str());
  jsonFile << m_json.dump(2) << std::endl;
}
