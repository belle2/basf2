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
#include <framework/dataobjects/FileMetaData.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/Utils.h>
#include <fstream>

using namespace Belle2;

MetadataService::MetadataService(): m_basf2StartTime{Utils::getClock()}
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
  if (m_fileName.empty()) return;
  if (!FileSystem::isFile(fileName)) return;

  nlohmann::json file_json = {{"type", "RootOutput"}, {"filename", fileName}};

  if (metaData) {
    file_json["metadata"] = nlohmann::json::parse(metaData->getJsonStr());
  }

  try {
    std::string check = Utils::getCommandOutput("b2file-check", {"--json", fileName});
    file_json.merge_patch(nlohmann::json::parse(check));
  } catch (...) {}

  file_json["checksums"]["md5"] = FileSystem::calculateMD5(fileName);
  file_json["checksums"]["adler32"] = FileSystem::calculateAdler32(fileName);
  // no sha256 yet

  m_json["output_files"].push_back(file_json);

  writeJson();
}

void MetadataService::addRootNtupleFile(const std::string& fileName)
{
  if (m_fileName.empty()) return;
  if (!FileSystem::isFile(fileName)) return;

  nlohmann::json file_json = {{"type", "RootNtuple"}, {"filename", fileName}};

  // no metadata and no check

  file_json["checksums"]["md5"] = FileSystem::calculateMD5(fileName);
  file_json["checksums"]["adler32"] = FileSystem::calculateAdler32(fileName);
  // no sha256 yet

  m_json["output_files"].push_back(file_json);

  writeJson();
}

void MetadataService::addBasf2Status(const std::string& message)
{
  if (m_fileName.empty()) return;
  auto& status = m_json["basf2_status"];
  status["elapsed_walltime_sec"] = (Utils::getClock() - m_basf2StartTime) / Unit::s;
  status["resident_memory_mb"] = Utils::getRssMemoryKB() / 1024;
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  if (processStatistics.isValid()) {
    const auto& stats = processStatistics->getGlobal();
    status["runs_processed"] = int(stats.getCalls(ModuleStatistics::EStatisticCounters::c_BeginRun));
    status["events_processed"] = int(stats.getCalls());
  }
  if ((status.count("total_events") == 0) || (status["total_events"] == 0)) {
    status["total_events"] = Environment::Instance().getNumberOfEvents();
  }
  status["fatals"] = LogSystem::Instance().getMessageCounter(LogConfig::c_Fatal);
  status["errors"] = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  status["warnings"] = LogSystem::Instance().getMessageCounter(LogConfig::c_Warning);
  status["finished"] = false;
  status["message"] = message;

  writeJson();
}

void MetadataService::addModuleCallsCount()
{
  if (m_fileName.empty()) return;

  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);

  if (processStatistics.isValid()) {

    std::vector<ModuleStatistics> modulesSortedByIndex(processStatistics->getAll());
    sort(modulesSortedByIndex.begin(), modulesSortedByIndex.end(), [](const ModuleStatistics & a, const ModuleStatistics & b) { return a.getIndex() < b.getIndex(); });

    for (const ModuleStatistics& stats : modulesSortedByIndex) {
      m_json["modules_calls"][stats.getName()] = int(stats.getCalls(ModuleStatistics::EStatisticCounters::c_Event));
    }
  }

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
