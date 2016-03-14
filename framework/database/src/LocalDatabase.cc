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

#include <framework/database/LocalDatabase.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <sys/file.h>

using namespace std;
using namespace Belle2;
namespace fs = boost::filesystem;

void LocalDatabase::createInstance(const std::string& fileName, const std::string& payloadDir, LogConfig::ELogLevel logLevel)
{
  LocalDatabase* database = new LocalDatabase(fileName, payloadDir);
  database->setLogLevel(logLevel);
  Database::setInstance(database);
}

LocalDatabase::LocalDatabase(const std::string& fileName,
                             const std::string& payloadDir): m_fileName(fs::absolute(fileName).string()), m_payloadDir(payloadDir)
{
  if (m_payloadDir.empty()) {
    m_payloadDir = fs::path(m_fileName).parent_path().string();
  }
  if (!fs::exists(m_payloadDir)) {
    fs::create_directories(m_payloadDir);
  }
  readDatabase();
}


bool LocalDatabase::readDatabase()
{
  m_database.clear();

  if (!fs::exists(m_fileName)) return true;

  // get lock for read access to database file
  FileSystem::Lock lock(m_fileName);
  if (!lock.lock()) {
    B2ERROR("Locking of database file " << m_fileName << " failed.");
    return false;
  }

  // read and parse the database content
  std::ifstream file(m_fileName.c_str());
  if (!file.is_open()) {
    B2ERROR("Opening of database file " << m_fileName << " failed.");
    return false;
  }
  try {
    while (!file.eof()) {
      string name;
      int revision;
      IntervalOfValidity iov;
      file >> name >> revision >> iov;
      int pos = name.find("/");
      string package = name.substr(0, pos);
      string module = name.substr(pos + 1, name.length());
      m_database[package][module].push_back(make_pair(revision, iov));
    }
  } catch (std::exception& e) {
    B2ERROR("Errors occured while reading " << m_fileName << ". (Error details: " << e.what() << ")");
    return false;
  }

  return true;
}

pair<TObject*, IntervalOfValidity> LocalDatabase::tryDefault(const std::string& package, const std::string& module)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  std::string defaultName = payloadFileName(m_payloadDir, package, module, 0);
  if (FileSystem::fileExists(defaultName)) {
    result.first = readPayload(defaultName, module.c_str());
    if (!result.first) return result;
    result.second = IntervalOfValidity(0, -1, -1, -1);
    return result;
  }

  B2LOG(m_logLevel, 0, "Failed to get " << package << "/" << module << " from local database " << m_fileName << ".");
  return result;
}

pair<TObject*, IntervalOfValidity> LocalDatabase::getData(const EventMetaData& event, const string& package,
                                                          const std::string& module)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  // find the entry for package and module in the maps
  const auto& packageEntry = m_database.find(package);
  if (packageEntry == m_database.end()) return tryDefault(package, module);
  const auto& moduleEntry = packageEntry->second.find(module);
  if (moduleEntry == packageEntry->second.end()) return tryDefault(package, module);

  // find the payload whose IoV contains the current event and load it
  for (auto& entry : moduleEntry->second) {
    if (entry.second.contains(event)) {
      int revision = entry.first;
      result.first = readPayload(payloadFileName(m_payloadDir, package, module, revision), module.c_str());
      if (!result.first) return result;
      result.second = entry.second;
      return result;
    }
  }

  B2LOG(m_logLevel, 0, "Failed to get " << package << "/" << module << " from local database " << m_fileName <<
        ". No matching entry for experiment/run " << event.getExperiment() << "/" << event.getRun() << " found.");
  return result;
}


bool LocalDatabase::storeData(const std::string& package, const std::string& module, TObject* object, IntervalOfValidity& iov)
{
  // get lock for write access to database file
  FileSystem::Lock lock(m_fileName);
  if (!lock.lock()) {
    B2ERROR("Locking of database file " << m_fileName << " failed.");
    return false;
  }

  // get revision number
  int revision = 1;
  while (FileSystem::fileExists(payloadFileName(m_payloadDir, package, module, revision))) revision++;

  // write payload file
  if (!writePayload(payloadFileName(m_payloadDir, package, module, revision), module, object, &iov)) return false;

  // add to database and update database file
  m_database[package][module].push_back(make_pair(revision, iov));
  std::ofstream file(m_fileName.c_str(), std::ios::app);
  if (!file.is_open()) return false;
  file << package << "/" << module << " " << revision << " " << iov << endl;

  return true;
}
