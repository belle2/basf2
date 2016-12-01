/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python/def.hpp>

#include <framework/database/ConditionsDatabase.h>
#include <framework/database/ConditionsPayloadDownloader.h>
#include <framework/database/DatabaseChain.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <boost/filesystem.hpp>
#include <TClonesArray.h>
#include <TFile.h>
#include <TTree.h>

#include <cstdio>
#include <fstream>
#include <sys/file.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

void ConditionsDatabase::createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel logLevel,
                                               const std::string& payloadDir)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, payloadDir);
  database->setLogLevel(logLevel);
  Database::setInstance(database);
}

void ConditionsDatabase::createInstance(const std::string& globalTag, const std::string& restBaseName,
                                        const std::string& fileBaseName,
                                        const std::string& fileBaseLocal, LogConfig::ELogLevel logLevel)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, fileBaseLocal);
  database->setRESTBase(restBaseName);
  database->addLocalDirectory(fileBaseName, EConditionsDirectoryStructure::c_logicalSubdirectories);
  database->setLogLevel(logLevel);
  Database::setInstance(database);
}


ConditionsDatabase::ConditionsDatabase(const std::string& globalTag, const std::string& payloadDir): m_globalTag(globalTag),
  m_currentExperiment(-1), m_currentRun(0)
{
  if (payloadDir.empty()) {
    m_payloadDir = fs::absolute(fs::current_path()).string();
  } else {
    m_payloadDir = fs::absolute(fs::path(payloadDir)).string();
  }
  m_downloader.reset(new ConditionsPayloadDownloader(m_payloadDir));
}

ConditionsDatabase::~ConditionsDatabase() {}


pair<TObject*, IntervalOfValidity> ConditionsDatabase::getData(const EventMetaData& event, const string& package,
    const std::string& module)
{
  //create session to reuse connection if there is none
  ConditionsPayloadDownloader::SessionGuard session(*m_downloader);

  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  if ((m_currentExperiment != event.getExperiment()) || (m_currentRun != event.getRun())) {
    m_currentExperiment = event.getExperiment();
    m_currentRun = event.getRun();
    m_downloader->update(m_globalTag, m_currentExperiment, m_currentRun);
  }

  if (!m_downloader->exists(module)) {
    B2LOG(m_logLevel, 0, "No payload " << module << " found in the conditions database for global tag "
          << m_globalTag << ".");
    return result;
  }

  const auto& info = m_downloader->get(module);

  if (info.filename.empty()) {
    B2ERROR("Failed to get " << module << " from conditions database.");
    return result;
  }

  result.first = readPayload(info.filename, module);
  if (!result.first) return result;

  result.second = info.iov;

  // Update database local cache file but only if payload is found in m_payloadDir
  if (fs::absolute(fs::path(info.filename)).parent_path() != fs::path(m_payloadDir)) {
    return result;
  }

  std::stringstream buffer;
  buffer << package << "/" << module << " " << info.revision << " " << result.second;
  std::string entry = buffer.str();
  std::string cacheFile = m_payloadDir + "/dbcache.txt";

  FileSystem::Lock lock(cacheFile);
  if (lock.lock()) {
    std::ifstream checkCache(cacheFile);
    std::string line;
    bool found = false;
    while (getline(checkCache, line)) {
      if (entry.compare(line) == 0) {
        found = true;
        break;
      }
    }
    checkCache.close();
    if (!found) {
      std::ofstream updateCache(cacheFile, std::ios::app);
      if (updateCache.is_open()) {
        updateCache << entry << endl;
      }
      updateCache.close();
    }
  }

  return result;
}

bool ConditionsDatabase::storeData(__attribute((unused)) const std::string& package,
                                   __attribute((unused)) const std::string& module,
                                   __attribute((unused)) TObject* object,
                                   __attribute((unused)) const IntervalOfValidity& iov)
{
  return false; // not implemented yet
}

bool ConditionsDatabase::addPayload(__attribute((unused)) const std::string& package,
                                    __attribute((unused)) const std::string& module,
                                    __attribute((unused)) const std::string& fileName,
                                    __attribute((unused)) const IntervalOfValidity&)
{
  return false; // not implemented yet
}

bool ConditionsDatabase::addExperimentName(int experiment, const std::string& name)
{
  return m_downloader->addExperimentName(experiment, name);
}

void ConditionsDatabase::setRESTBase(const std::string& restBase)
{
  m_downloader->setRESTBase(restBase);
}
void ConditionsDatabase::addLocalDirectory(const std::string& localDir, EConditionsDirectoryStructure structure)
{
  m_downloader->addLocalDirectory(localDir, structure);
}
