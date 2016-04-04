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

#include <framework/database/DatabaseChain.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;


void DatabaseChain::createInstance(bool resetIoVs, LogConfig::ELogLevel logLevel)
{
  DatabaseChain* database = new DatabaseChain(resetIoVs);
  database->setLogLevel(logLevel);
  Database::setInstance(database);
}

DatabaseChain::~DatabaseChain()
{
  for (auto* database : m_databases) delete database;
  m_databases.clear();
}


void DatabaseChain::addDatabase(Database* database)
{
  if (database == s_instance) {
    B2ERROR("Cannot add main database instance.");
  } else if (database == this) {
    B2ERROR("Cannot add DatabaseChain to itself.");
  } else {
    m_databases.insert(m_databases.begin(), database);
  }
}


pair<TObject*, IntervalOfValidity> DatabaseChain::getData(const EventMetaData& event, const string& package,
                                                          const std::string& module)
{
  pair<TObject*, IntervalOfValidity> result;
  result.first = 0;

  // loop over database backends and try to find a matching payload
  for (unsigned int i = 0; i < m_databases.size(); i++) {
    result = m_databases[i]->getData(event, package, module);
    if (result.first) {
      if (m_resetIoVs && (i > 0)) {
        result.second = IntervalOfValidity(event.getExperiment(), event.getRun(), event.getExperiment(), event.getRun());
      }
      return result;
    }
  }

  B2LOG(m_logLevel, 0, "Failed to get " << package << "/" << module << " from database chain. No matching entry for experiment/run "
        << event.getExperiment() << "/" <<
        event.getRun() << " found.");
  return result;
}

bool DatabaseChain::storeData(const std::string& package, const std::string& module, TObject* object, IntervalOfValidity& iov)
{
  if (m_databases.empty()) {
    B2ERROR("No database backend available.");
    return false;
  }

  return m_databases[0]->storeData(package, module, object, iov);
}

bool DatabaseChain::addPayload(const std::string& package, const std::string& module, const std::string& fileName,
                               IntervalOfValidity& iov)
{
  if (m_databases.empty()) {
    B2ERROR("No database backend available.");
    return false;
  }

  return m_databases[0]->addPayload(package, module, fileName, iov);
}
