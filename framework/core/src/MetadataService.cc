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
#include <fstream>
#include <boost/process.hpp>
#include <TMD5.h>

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
    boost::process::ipstream out;
    boost::process::system(boost::process::search_path("b2file-check"), "--json", fileName, boost::process::std_out > out);
    nlohmann::json checks;
    out >> checks;
    file_json.merge_patch(checks);
  } catch (...) {}

  file_json["checksums"]["md5"] = TMD5::FileChecksum(fileName.c_str())->AsString();
  // no sha256 yet

  m_json["output_files"].push_back(file_json);

  writeJson();
}

void MetadataService::finishBasf2(bool success)
{
  m_json["basf2_status"]["finished"] = true;
  m_json["basf2_status"]["success"] = success;

  writeJson();
}

void MetadataService::writeJson()
{
  if (m_fileName.empty()) return;
  std::ofstream jsonFile(m_fileName.c_str());
  jsonFile << m_json.dump(2) << std::endl;
}
