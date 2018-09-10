/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
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


void DatabaseChain::createInstance(bool resetIoVs, LogConfig::ELogLevel logLevel, bool invertLogging)
{
  DatabaseChain* database = new DatabaseChain(resetIoVs);
  database->setLogLevel(logLevel, invertLogging);
  Database::setInstance(database);
}

DatabaseChain::~DatabaseChain()
{
  for (auto* database : m_databases) delete database;
  m_databases.clear();
}


void DatabaseChain::addDatabase(Database* database)
{
  if (database == s_instance.get()) {
    B2ERROR("Cannot add main database instance.");
  } else if (database == this) {
    B2ERROR("Cannot add DatabaseChain to itself.");
  } else {
    m_databases.insert(m_databases.begin(), database);
  }
}


bool DatabaseChain::getData(const EventMetaData& event, DBQuery& query)
{
  // loop over database backends and try to find a matching payload
  for (unsigned int i = 0; i < m_databases.size(); i++) {
    if (m_databases[i]->getData(event, query)) {
      if (m_resetIoVs && (i > 0)) {
        query.iov = IntervalOfValidity(event.getExperiment(), event.getRun(), event.getExperiment(), event.getRun());
      }
      if (m_invertLogging)
        B2LOG(m_logLevel, 35, "Obtained " << query.name << " from database chain. IoV=" << query.iov);
      return true;
    }
  }

  if (!m_invertLogging)
    B2LOG(m_logLevel, 35, "Failed to get " << query.name << " from database chain. "
          "No matching entry for experiment/run " << event.getExperiment() << "/" << event.getRun() << " found.");
  return false;
}

bool DatabaseChain::storeData(const std::string& name, TObject* object,
                              const IntervalOfValidity& iov)
{
  if (m_databases.empty()) {
    B2ERROR("No database backend available.");
    return false;
  }

  return m_databases[0]->storeData(name, object, iov);
}

bool DatabaseChain::addPayload(const std::string& name, const std::string& fileName,
                               const IntervalOfValidity& iov)
{
  if (m_databases.empty()) {
    B2ERROR("No database backend available.");
    return false;
  }

  return m_databases[0]->addPayload(name, fileName, iov);
}
