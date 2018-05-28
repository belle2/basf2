/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
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
#include <iomanip>
#include <sys/file.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

void ConditionsDatabase::createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel logLevel,
                                               const std::string& payloadDir)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, payloadDir);
  database->setLogLevel(logLevel);
  database->addLocalDirectory("/cvmfs/belle.cern.ch/conditions", EConditionsDirectoryStructure::c_flatDirectory);
  Database::setInstance(database);
}

void ConditionsDatabase::createInstance(const std::string& globalTag, const std::string& restBaseName,
                                        const std::string& fileBaseName,
                                        const std::string& fileBaseLocal, LogConfig::ELogLevel logLevel,
                                        bool invertLogging)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, fileBaseLocal);
  database->setRESTBase(restBaseName);
  database->addLocalDirectory(fileBaseName, EConditionsDirectoryStructure::c_logicalSubdirectories);
  database->addLocalDirectory("/cvmfs/belle.cern.ch/conditions", EConditionsDirectoryStructure::c_flatDirectory);
  database->setLogLevel(logLevel, invertLogging);
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


bool ConditionsDatabase::getData(const EventMetaData& event, DBQuery& query)
{
  //create session to reuse connection if there is none
  ConditionsPayloadDownloader::SessionGuard session(*m_downloader);

  if ((m_currentExperiment != event.getExperiment()) || (m_currentRun != event.getRun())) {
    m_currentExperiment = event.getExperiment();
    m_currentRun = event.getRun();
    m_downloader->update(m_globalTag, m_currentExperiment, m_currentRun);
  }

  if (!m_downloader->exists(query.name)) {
    if (!m_invertLogging)
      B2LOG(m_logLevel, 35, "No entry " << std::quoted(query.name) << " found for this run in the conditions "
            "database for global tag " << std::quoted(m_globalTag) << ".");
    return false;
  }

  const auto& info = m_downloader->get(query.name);

  if (info.filename.empty()) {
    // error raised already ...
    return false;
  }

  query.filename = info.filename;
  query.revision = info.revision;
  query.checksum = info.digest;

  if (m_invertLogging)
    B2LOG(m_logLevel, 35, "Payload " << std::quoted(query.name) << " found in the conditions database for global tag "
          << std::quoted(m_globalTag) << ". IoV=" << info.iov);

  // Update database local cache file but only if payload is found in m_payloadDir
  if (fs::absolute(fs::path(info.filename)).parent_path() != fs::path(m_payloadDir)) {
    return true;
  }

  std::stringstream buffer;
  buffer << "dbstore/" << query.name << " " << info.revision << " " << query.iov;
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

  return true;
}

bool ConditionsDatabase::storeData(__attribute((unused)) const std::string& name,
                                   __attribute((unused)) TObject* object,
                                   __attribute((unused)) const IntervalOfValidity& iov)
{
  return false; // not implemented yet
}

bool ConditionsDatabase::addPayload(__attribute((unused)) const std::string& name,
                                    __attribute((unused)) const std::string& fileName,
                                    __attribute((unused)) const IntervalOfValidity&)
{
  return false; // not implemented yet
}

void ConditionsDatabase::setRESTBase(const std::string& restBase)
{
  m_downloader->setRESTBase(restBase);
}
void ConditionsDatabase::addLocalDirectory(const std::string& localDir, EConditionsDirectoryStructure structure)
{
  m_downloader->addLocalDirectory(localDir, structure);
}
void ConditionsDatabase::setServerList(const std::vector<std::string>& serverList)
{
  m_downloader->setServerList(serverList);
}
