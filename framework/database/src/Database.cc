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
#include <boost/python/overloads.hpp>

#include <framework/database/Database.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/DBStore.h>

#include <TFile.h>

using namespace std;
using namespace Belle2;


Database* Database::s_instance = 0;

Database& Database::Instance()
{
  if (!s_instance) {
    DatabaseChain::createInstance(true);
    LocalDatabase::createInstance(FileSystem::findFile("data/framework/database.txt"), "", LogConfig::c_Error);
    ConditionsDatabase::createDefaultInstance("production", LogConfig::c_Warning);
    LocalDatabase::createInstance("localdb/database.txt", "", LogConfig::c_Debug);
  }
  return *s_instance;
}

void Database::setInstance(Database* database)
{
  if (s_instance) {
    DatabaseChain* chain = dynamic_cast<DatabaseChain*>(s_instance);
    DatabaseChain* replacement = dynamic_cast<DatabaseChain*>(database);
    if (replacement && chain) {
      B2DEBUG(200, "Replacing DatabaseChain with DatabaseChain: ignored");
      delete database;
    } else if (chain) {
      chain->addDatabase(database);
    } else if (replacement) {
      B2DEBUG(200, "Replacing Database with DatabaseChain: adding existing database to chain");
      replacement->addDatabase(s_instance);
      s_instance = replacement;
    } else {
      B2WARNING("The already created database instance is replaced by a new instance.");
      delete s_instance;
      s_instance = database;
      DBStore::Instance().reset();
    }
  } else {
    s_instance = database;
  }
}

void Database::reset()
{
  delete s_instance;
  s_instance = 0;
  DBStore::Instance().reset();
}


Database::~Database()
{
  if (s_instance == this) s_instance = 0;
}


void Database::getData(const EventMetaData& event, std::list<DBQuery>& query)
{
  for (auto& entry : query) {
    auto objectIov = getData(event, entry.package, entry.module);
    entry.object = objectIov.first;
    entry.iov = objectIov.second;
  }
}

bool Database::storeData(std::list<DBQuery>& query)
{
  bool result = true;
  for (auto& entry : query) {
    result = result && storeData(entry.package, entry.module, entry.object, entry.iov);
  }
  return result;
}


std::string Database::payloadFileName(const std::string& path, const std::string& package, const std::string& module,
                                      int revision) const
{
  std::string result = package + "_" + module;
  if (revision > 0) result += "_rev_" + std::to_string(revision);
  result += ".root";
  if (!path.empty()) result = path + "/" + result;
  return result;
}

TObject* Database::readPayload(const std::string& fileName, const std::string& module) const
{
  TObject* result = 0;

  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open(fileName.c_str());
  saveDir->cd();
  if (!file || !file->IsOpen()) {
    B2ERROR("Could not open payload file " << fileName << ".");
    delete file;
    return result;
  }

  result = file->Get(module.c_str());
  delete file;
  if (!result) {
    B2ERROR("Failed to get " << module  << " from payload file" << fileName << ".");
  }

  return result;
}

bool Database::writePayload(const std::string& fileName, const std::string& module, const TObject* object,
                            const IntervalOfValidity* iov) const
{
  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open(fileName.c_str(), "RECREATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Could not open payload file " << fileName << ".");
    delete file;
    saveDir->cd();
    return false;
  }

  object->Write(module.c_str(), TObject::kSingleKey);
  if (iov) file->WriteObject(iov, "IoV");

  file->Close();
  delete file;
  saveDir->cd();

  return true;
}

#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
BOOST_PYTHON_FUNCTION_OVERLOADS(chain_createInstance_overloads, DatabaseChain::createInstance, 0, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(local_createInstance_overloads, LocalDatabase::createInstance, 0, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(condition_createDefaultInstance_overloads, ConditionsDatabase::createDefaultInstance, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(condition_createInstance_overloads, ConditionsDatabase::createInstance, 4, 5);
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

void Database::exposePythonAPI()
{
  using namespace boost::python;
  // make sure the default instance is created
  Database::Instance();

  def("reset_database", &Database::reset);
  def("use_database_chain", &DatabaseChain::createInstance, chain_createInstance_overloads());
  def("use_local_database", &LocalDatabase::createInstance, local_createInstance_overloads());
  def("use_central_database", &ConditionsDatabase::createDefaultInstance, condition_createDefaultInstance_overloads());
  def("use_central_database", &ConditionsDatabase::createInstance, condition_createInstance_overloads());
}
