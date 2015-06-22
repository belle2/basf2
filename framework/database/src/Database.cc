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

#include <framework/database/Database.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>

using namespace std;
using namespace Belle2;


Database* Database::s_instance = 0;

Database& Database::Instance()
{
  if (!s_instance) LocalDatabase::createInstance();
  return *s_instance;
}


void Database::getData(const EventMetaData& event, std::list<DBQuery>& query)
{
  for (auto& entry : query) {
    auto objectIov = getData(event, entry.name);
    entry.object = objectIov.first;
    entry.iov = objectIov.second;
  }
}

bool Database::storeData(std::list<DBQuery>& query)
{
  bool result = true;
  for (auto& entry : query) {
    result = result && storeData(entry.name, entry.object, entry.iov);
  }
  return result;
}


void Database::exposePythonAPI()
{
  using namespace boost::python;

  def("set_global_tag", &Database::setGlobalTag);
  def("use_local_database", &LocalDatabase::createInstance);
  def("use_central_database", &ConditionsDatabase::createDefaultInstance);
  def("use_central_database", &ConditionsDatabase::createInstance);
}
