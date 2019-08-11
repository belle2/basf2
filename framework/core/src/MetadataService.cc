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
#include <framework/core/ProcessStatistics.h>
#include <framework/core/Environment.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/gearbox/Unit.h>
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

static auto basf2StartTime = Utils::getClock();

void MetadataService::addBasf2Status(const std::string& message)
{
  m_json["basf2_status"]["elapsed_walltime_sec"] = (Utils::getClock() - basf2StartTime) / Unit::s;
  m_json["basf2_status"]["resident_memory_mb"] = Utils::getRssMemoryKB() / 1024;
  static StoreObjPtr<ProcessStatistics> processStatistics;
  if (processStatistics.isValid()) {
    const auto& stats = processStatistics->getGlobal();
    m_json["basf2_status"]["runs_processed"] = stats.getCalls(ModuleStatistics::EStatisticCounters::c_BeginRun);
    m_json["basf2_status"]["events_processed"] = stats.getCalls();
  }
  if ((m_json["basf2_status"].count("total_events") == 0) || (m_json["basf2_status"]["total_events"] == 0)) {
    m_json["basf2_status"]["total_events"] = Environment::Instance().getNumberOfEvents();
  }
  m_json["basf2_status"]["fatals"] = LogSystem::Instance().getMessageCounter(LogConfig::c_Fatal);
  m_json["basf2_status"]["errors"] = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  m_json["basf2_status"]["warnings"] = LogSystem::Instance().getMessageCounter(LogConfig::c_Warning);
  m_json["basf2_status"]["finished"] = false;
  m_json["basf2_status"]["message"] = message;

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
