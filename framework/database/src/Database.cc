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
#include <boost/python/docstring_options.hpp>
#include <boost/algorithm/string.hpp>

#include <framework/database/Database.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/DBStore.h>
#include <framework/database/PayloadFile.h>

#include <TFile.h>

using namespace std;
using namespace Belle2;


Database* Database::s_instance = 0;

Database& Database::Instance()
{
  if (!s_instance) {
    DatabaseChain::createInstance(true);
    LocalDatabase::createInstance(FileSystem::findFile("data/framework/database.txt"), "", true, LogConfig::c_Error);
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
      Database* old = s_instance;
      s_instance = replacement;
      replacement->addDatabase(old);
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

  if (module.find(".") != std::string::npos) {
    return new PayloadFile(fileName);
  }

  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open(fileName.c_str());
  saveDir->cd();
  if (!file || !file->IsOpen()) {
    B2ERROR("Could not open payload file " << fileName << " for reading.");
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
    B2ERROR("Could not open payload file " << fileName << " for writing.");
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
BOOST_PYTHON_FUNCTION_OVERLOADS(local_createInstance_overloads, LocalDatabase::createInstance, 0, 4);
BOOST_PYTHON_FUNCTION_OVERLOADS(condition_createDefaultInstance_overloads, ConditionsDatabase::createDefaultInstance, 1, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(condition_createInstance_overloads, ConditionsDatabase::createInstance, 4, 5);
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

namespace {
  void Database_addExperimentName(int experiment, const std::string& name)
  {
    B2INFO("Map Experiment " << experiment << " to '" << name << "'");
    std::vector<Database*> databases{&Database::Instance()};
    DatabaseChain* chain = dynamic_cast<DatabaseChain*>(databases[0]);
    if (chain) {
      databases = chain->getDatabases();
    }
    bool found{false};
    for (Database* db : databases) {
      ConditionsDatabase* cond = dynamic_cast<ConditionsDatabase*>(db);
      if (cond) {
        cond->addExperimentName(experiment, name);
        found = true;
      }
    }
    if (!found) {
      B2WARNING("No central database configured, experiment name ignored");
    }
  }
}

std::string Database::getGlobalTag()
{
  std::vector<Database*> databases{&Database::Instance()};
  std::vector<std::string> tags;
  DatabaseChain* chain = dynamic_cast<DatabaseChain*>(databases[0]);
  if (chain) {
    databases = chain->getDatabases();
  }
  for (Database* db : databases) {
    ConditionsDatabase* cond = dynamic_cast<ConditionsDatabase*>(db);
    if (cond) {
      std::string tag = cond->getGlobalTag();
      if (std::find(tags.begin(), tags.end(), tag) == tags.end()) {
        tags.emplace_back(tag);
      }
    }
  }
  if (tags.empty()) return "";
  return boost::algorithm::join(tags, ",");
}

void Database::exposePythonAPI()
{
  using namespace boost::python;
  // make sure the default instance is created
  Database::Instance();

  //don't show c++ signature in python doc to keep it simple
  docstring_options options(true, true, false);

  def("set_experiment_name", &Database_addExperimentName, args("experiment", "name"), R"DOCSTRING(
Set a name for the given experiment number when looking up payloads in the
central database. A central database needs to be set up before using `use_central_database`.

The experiment and numbers and names need to be unique because the mapping
needs to be performed in both directions so different experiment numbers cannot
be mapped to the same name or vice versa.

Example:
    >>> set_experiment_name(4, "BELLE_exp4")

:param int experiment: Experiment number from EventMetaData
:param str name: name of the experiment when looking up payloads
)DOCSTRING");
  def("reset_database", &Database::reset, "Reset the database setup to have no database sources");
  def("use_database_chain", &DatabaseChain::createInstance, chain_createInstance_overloads(args("resetIoVs", "loglevel"), R"DOCSTRING(
Use a database chain: Multiple database sources are used on a first found
basis: If the payload is not found in one source try the next and so on.

If the chain is enabled calls to use_local_database and use_central_database
will add these sources to the chain instead of replacing the current source.
If there was an exisiting single source setup the one source is kept in the
chain. If there was already a DatabaseChain nothing changes.

:param bool resetIoVs: A flag to indicate whether IoVs from non-primary
        databases should be set to only the current run and rechecked for the
        next run.
:param basf2.LogLevel loglevel: The LogLevel of messages from the database
        chain, defaults to LogLevel.WARNING
)DOCSTRING"));
  def("use_local_database", &LocalDatabase::createInstance, local_createInstance_overloads(args("filename", "directory", "readonly", "loglevel"), R"DOCSTRING(
Use a local database backend: a single file containing the payload information in plain text.

:param str filename: filename containing the payload information, defaults to
        "database.txt"
:param str directory: directory containing the payloads, defaults to current
        directory
:param bool readonly: if True the database will refuse to create new payloads
:param basf2.LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to LogLevel.WARNING
)DOCSTRING"));
  def("use_central_database", &ConditionsDatabase::createDefaultInstance, condition_createDefaultInstance_overloads(
              args("globalTag", "loglevel"), R"DOCSTRING(
Use the central database via REST api and the default connection parameters.

:param str globalTag: name of the global tag to use for payload lookup
:param basf2.LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to LogLevel.WARNING
)DOCSTRING"));
  def("use_central_database", &ConditionsDatabase::createInstance, condition_createInstance_overloads(
              args("globalTag", "restBaseName", "fileBaseName", "fileBaseLocal", "loglevel"), R"DOCSTRING(
Use the central database via REST api and the custom connection parameters.
This version should only used by experts to debug the conditions database or
use a different database.

:param str globalTag: name of the global tag to use for payload lookup
:param restBaseName: base URL for the REST api
:param fileBaseName: base URL for the payload download
:param fileBaseLocal: directory where to save downloaded payloads
:param basf2.LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to LogLevel.WARNING
)DOCSTRING"));
}
