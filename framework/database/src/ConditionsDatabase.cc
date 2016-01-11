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

#include <framework/conditions/ConditionsService.h>
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

void ConditionsDatabase::createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel logLevel)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, "centraldb");
  ConditionsService::getInstance()->setFILEbaselocal("centraldb/");
  database->setLogLevel(logLevel);
  Database::setInstance(database);
}

void ConditionsDatabase::createInstance(const std::string& globalTag, const std::string& restBaseName,
                                        const std::string& fileBaseName,
                                        const std::string& fileBaseLocal, LogConfig::ELogLevel logLevel)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, fileBaseLocal);
  ConditionsService::getInstance()->setRESTbasename(restBaseName);
  ConditionsService::getInstance()->setFILEbasename(fileBaseName);
  ConditionsService::getInstance()->setFILEbaselocal(fileBaseLocal);
  database->setLogLevel(logLevel);
  Database::setInstance(database);
}


ConditionsDatabase::ConditionsDatabase(const std::string& globalTag, const std::string& payloadDir): m_globalTag(globalTag),
  m_payloadDir(payloadDir), m_currentExperiment(-1), m_currentRun(0)
{
  if (m_payloadDir.empty()) {
    m_payloadDir = fs::absolute(fs::current_path()).string();
  }
}


pair<TObject*, IntervalOfValidity> ConditionsDatabase::getData(const EventMetaData& event, const string& package,
    const std::string& module)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  if ((m_currentExperiment != event.getExperiment()) || (m_currentRun != event.getRun())) {
    m_currentExperiment = event.getExperiment();
    m_currentRun = event.getRun();
    ConditionsService::getInstance()->getPayloads(m_globalTag, std::to_string(m_currentExperiment), std::to_string(m_currentRun));
  }

  if (!ConditionsService::getInstance()->payloadExists(package + module)) {
    B2LOG(m_logLevel, 0, "No payload " << package << "/" << module << " found in the conditions database for global tag " << m_globalTag
          << ".");
    return result;
  }

  if (!fs::exists(m_payloadDir)) {
    fs::create_directories(m_payloadDir);
  }

  std::string filename = ConditionsService::getInstance()->getPayloadFileURL(package, module);
  if (filename.empty()) {
    B2ERROR("Failed to get " << package << "/" << module << " from conditions database.");
    return result;
  }

  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open(filename.c_str());
  saveDir->cd();
  if (!file || !file->IsOpen()) {
    B2ERROR("Could not open payload file " << file << ".");
    delete file;
    return result;
  }

  result.first = file->Get(module.c_str());
  delete file;
  if (!result.first) {
    B2ERROR("Failed to get " << package << "/" << module  << " from database. Object not found in payload file.");
    return result;
  }

  conditionsPayload paylodInfo = ConditionsService::getInstance()->getPayloadInfo(package, module);
  result.second = IntervalOfValidity(stoi(paylodInfo.expInitial), stoi(paylodInfo.runInitial), stoi(paylodInfo.expFinal),
                                     stoi(paylodInfo.runFinal));

  // Update database local cache file
  std::string revision = filename.substr(filename.find_last_of("_") + 1);
  revision = revision.substr(0, revision.find_last_of("."));
  std::stringstream buffer;
  buffer << package << "/" << module << " " << revision << " " << result.second;
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

bool ConditionsDatabase::storeData(const std::string& package, const std::string& module, TObject* object, IntervalOfValidity& iov)
{
  std::string fileName = fs::unique_path().string();
  if (writePayload(fileName, module, object, &iov)) {
    ConditionsService::getInstance()->writePayloadFile(fileName, package, module);
  }

  remove("payload.root");

  return true;
}
