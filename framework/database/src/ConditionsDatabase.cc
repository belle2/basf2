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

void ConditionsDatabase::createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel logLevel,
                                               const std::string& payloadDir)
{
  ConditionsDatabase* database = new ConditionsDatabase(globalTag, payloadDir);
  ConditionsService::getInstance()->setFILEbaselocal(payloadDir + "/");
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
  m_currentExperiment(-1), m_currentRun(0)
{
  if (payloadDir.empty()) {
    m_payloadDir = fs::absolute(fs::current_path()).string();
  } else {
    m_payloadDir = fs::absolute(fs::path(payloadDir)).string();
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
    std::string experimentName = std::to_string(m_currentExperiment);
    // check if we have a name defined for the current experiment number
    auto it = m_mapping.left.find(m_currentExperiment);
    if (it != m_mapping.left.end()) {
      experimentName = it->second;
    }
    ConditionsService::getInstance()->getPayloads(m_globalTag, experimentName, std::to_string(m_currentRun));
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

  result.first = readPayload(filename, module);
  if (!result.first) return result;

  conditionsPayload payloadInfo = ConditionsService::getInstance()->getPayloadInfo(package, module);
  // we possibly need to convert the experiment names back to numbers so we
  // have to check for both if a name is defined and if so just use the number
  // associated, otherwise try to convert to int. We need this two times so
  // let's use a small lambda to save some copy pasta
  auto getExpNumber = [&](const std::string & exp) {
    auto it = m_mapping.right.find(exp);
    if (it != m_mapping.right.end()) return it->second;
    try {
      return stoi(exp);
    } catch (std::invalid_argument& e) {
      B2FATAL("Invalid experiment number: '" << exp << "'. Please define the experiment name using 'set_experiment_name'");
    }
  };
  result.second = IntervalOfValidity(getExpNumber(payloadInfo.expInitial), stoi(payloadInfo.runInitial),
                                     getExpNumber(payloadInfo.expFinal), stoi(payloadInfo.runFinal));

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

bool ConditionsDatabase::storeData(const std::string& package, const std::string& module, TObject* object,
                                   const IntervalOfValidity& iov)
{
  std::string fileName = fs::unique_path().string();
  if (writePayload(fileName, module, object, &iov)) {
    ConditionsService::getInstance()->writePayloadFile(fileName, package, module);
  }

  remove(fileName.c_str());

  return true;
}

bool ConditionsDatabase::addPayload(const std::string& package, const std::string& module, const std::string& fileName,
                                    const IntervalOfValidity&)
{
  ConditionsService::getInstance()->writePayloadFile(fileName, package, module);
  return true;
}

bool ConditionsDatabase::addExperimentName(int experiment, const std::string& name)
{
  auto it = m_mapping.insert(boost::bimap<int, std::string>::value_type(experiment, name));
  if (!it.second) {
    B2ERROR("Cannot set experiment name " << experiment << "->'" << name << "': conflict with "
            << it.first->left << "->'" << it.first->right << "'");
  }
  return it.second;
}
