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

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>

using namespace std;
using namespace Belle2;

namespace {
  /** Small helper to get a value from environment or fall back to a default
   * @param envName name of the environment variable to look for
   * @param fallback value to return in case environment variable is not set
   * @return whitespace trimmed value of the environment variable if set, otherwise the fallback value
   */
  std::string getFromEnvironment(const std::string& envName, const std::string& fallback)
  {
    char* envValue = std::getenv(envName.c_str());
    if (envValue != nullptr) {
      std::string val(envValue);
      boost::trim(val);
      return envValue;
    }
    return fallback;
  }
}

std::unique_ptr<Database> Database::s_instance{nullptr};

Database& Database::Instance()
{
  if (!s_instance) {
    DatabaseChain::createInstance(true);
    const std::string fallbackFilename = getFromEnvironment("BELLE2_CONDB_FALLBACK", "data/framework/database.txt");
    const std::string globalTag = getFromEnvironment("BELLE2_CONDB_GLOBALTAG", "production");
    // OK, add a fallback database unless empty location is specified
    if (!fallbackFilename.empty()) {
      LocalDatabase::createInstance(FileSystem::findFile(fallbackFilename), "", true, LogConfig::c_Error);
    }
    // and add access to the central database unless we have an empty global tag
    // in which case we disable access to the database
    if (!globalTag.empty()) {
      typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
      // add all global tags which are separated by whitespace as conditions database
      for (auto token : tokenizer(globalTag, boost::char_separator<char> {" \t\n\r"})) {
        B2DEBUG(100, "Adding central database for global tag " << token);
        ConditionsDatabase::createDefaultInstance(token, LogConfig::c_Warning);
      }
    }
    LocalDatabase::createInstance("localdb/database.txt", "", false, LogConfig::c_Warning, true);
  }
  return *s_instance;
}

void Database::setInstance(Database* database)
{
  if (s_instance) {
    DatabaseChain* chain = dynamic_cast<DatabaseChain*>(s_instance.get());
    DatabaseChain* replacement = dynamic_cast<DatabaseChain*>(database);
    if (replacement && chain) {
      B2DEBUG(200, "Replacing DatabaseChain with DatabaseChain: ignored");
      delete database;
    } else if (chain) {
      chain->addDatabase(database);
    } else if (replacement) {
      B2DEBUG(200, "Replacing Database with DatabaseChain: adding existing database to chain");
      Database* old = s_instance.release();
      s_instance.reset(replacement);
      replacement->addDatabase(old);
    } else {
      B2WARNING("The already created database instance is replaced by a new instance.");
      s_instance.reset(database);
      DBStore::Instance().reset();
    }
  } else {
    s_instance.reset(database);
  }
}

void Database::reset()
{
  s_instance.reset();
  DBStore::Instance().reset();
}


Database::~Database() {}


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


std::string Database::payloadFileName(const std::string& path, const std::string& name,
                                      int revision) const
{
  std::string result = "dbstore_" + name;
  if (revision > 0) result += "_rev_" + std::to_string(revision);
  result += ".root";
  if (!path.empty()) result = path + "/" + result;
  return result;
}

TObject* Database::readPayload(const std::string& fileName, const std::string& name) const
{
  TObject* result = 0;

  if (name.find(".") != std::string::npos) {
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

  result = file->Get(name.c_str());
  delete file;
  if (!result) {
    B2ERROR("Failed to get " << name << " from payload file" << fileName << ".");
  }

  return result;
}

bool Database::writePayload(const std::string& fileName, const std::string& name, const TObject* object,
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

  object->Write(name.c_str(), TObject::kSingleKey);
  if (iov) file->WriteObject(iov, "IoV");

  file->Close();
  delete file;
  saveDir->cd();

  return true;
}

namespace {
  void Database_addExperimentName(int, const std::string&)
  {
    B2WARNING("set_experiment_name is deprecated: setting experiment names is no longer possible and will be ignored");
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
  // to avoid confusion between std::arg and boost::python::arg we want a shorthand namespace as well
  namespace bp = boost::python;
  // make sure the default instance is created
  Database::Instance();

  //don't show c++ signature in python doc to keep it simple
  docstring_options options(true, true, false);

  def("set_experiment_name", &Database_addExperimentName, args("experiment", "name"), R"DOCSTRING(
Set a name for the given experiment number when looking up payloads in the
central database. Thisf function is deprecated and any calls to it are ignored.)DOCSTRING");
  def("reset_database", &Database::reset, "Reset the database setup to have no database sources");
  def("use_database_chain", &DatabaseChain::createInstance,
      (bp::arg("resetIoVs") = true, bp::arg("loglevel") = LogConfig::c_Warning, bp::arg("invertLogging") = false),
      R"DOCSTRING(
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
:param bool invertLogging: A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        `loglevel` will be emitted everytime a payload cannot be found. If true
        a message will be emitted if a payload is actually found.
)DOCSTRING");
  def("use_local_database", &LocalDatabase::createInstance,
      (bp::arg("filename"), bp::arg("directory")="", bp::arg("readonly")=false,
       bp::arg("loglevel")=LogConfig::c_Warning, bp::arg("invertLogging")=false),
      R"DOCSTRING(
Use a local database backend: a single file containing the payload information in plain text.

:param str filename: filename containing the payload information, defaults to
        "database.txt"
:param str directory: directory containing the payloads, defaults to current
        directory
:param bool readonly: if True the database will refuse to create new payloads
:param basf2.LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to LogLevel.WARNING
:param bool invertLogging: A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        `loglevel` will be emitted everytime a payload cannot be found. If true
        a message will be emitted if a payload is actually found.
)DOCSTRING");
  def("use_central_database", &ConditionsDatabase::createDefaultInstance,
      (bp::arg("globalTag"), bp::arg("loglevel")=LogConfig::c_Warning, bp::arg("payloaddir")="centraldb"),
      R"DOCSTRING(
Use the central database via REST api and the default connection parameters.

:param str globalTag: name of the global tag to use for payload lookup
:param basf2.LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to LogLevel.WARNING
:param str payloaddir: directory where to save downloaded payloads
)DOCSTRING");
  def("use_central_database", &ConditionsDatabase::createInstance,
      (bp::arg("globalTag"), bp::arg("restBaseName"), bp::arg("payloaddir"), bp::arg("fileBaseLocal"),
       bp::arg("loglevel")=LogConfig::c_Warning, bp::arg("invertLogging")=false),
      R"DOCSTRING(
Use the central database via REST api and the custom connection parameters.
This version should only used by experts to debug the conditions database or
use a different database.

:param str globalTag: name of the global tag to use for payload lookup
:param str restBaseName: base URL for the REST api
:param str fileBaseName: base URL for the payload download
:param str payloaddir: directory where to save downloaded payloads
:param basf2.LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to LogLevel.WARNING
:param bool invertLogging: A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        `loglevel` will be emitted everytime a payload cannot be found. If true
        a message will be emitted if a payload is actually found.
)DOCSTRING");
}
