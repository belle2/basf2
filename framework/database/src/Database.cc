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
#include <boost/python/overloads.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/algorithm/string.hpp>

#include <framework/database/Database.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/EnvironmentVariables.h>
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/DBStore.h>
#include <framework/database/PayloadFile.h>
#include <framework/database/ConditionsPayloadDownloader.h>

#include <TFile.h>

#include <cstdlib>
#include <iomanip>

#define CURRENT_DEFAULT_TAG "GT_gen_prod_004.47_Master-20180504-074700"

using namespace std;
using namespace Belle2;

std::unique_ptr<Database> Database::s_instance{nullptr};

std::string Database::getDefaultGlobalTags()
{
  return EnvironmentVariables::get("BELLE2_CONDB_GLOBALTAG", CURRENT_DEFAULT_TAG);
}

Database& Database::Instance()
{
  if (!s_instance) {
    DatabaseChain::createInstance(true);
    const std::vector<std::string> fallbacks = EnvironmentVariables::getList("BELLE2_CONDB_FALLBACK", {"/cvmfs/belle.cern.ch/conditions"});
    const std::vector<std::string> globalTags = EnvironmentVariables::getList("BELLE2_CONDB_GLOBALTAG", {CURRENT_DEFAULT_TAG});
    B2DEBUG(38, "Conditions database fallback options:");
    for (auto s : fallbacks) B2DEBUG(38, "  " << s);
    B2DEBUG(38, "Conditions database global tags:");
    for (auto s : globalTags) B2DEBUG(38, "  " << s);
    // OK, add fallback databases unless empty location is specified
    if (!fallbacks.empty()) {
      auto logLevel = LogConfig::c_Error;
      for (auto localdb : fallbacks) {
        if (FileSystem::isFile(FileSystem::findFile(localdb, true))) {
          // If a file name is given use it as local DB
          B2DEBUG(30, "Adding fallback database " << FileSystem::findFile(localdb));
          LocalDatabase::createInstance(FileSystem::findFile(localdb), "", true, logLevel);
        } else if (FileSystem::isDir(localdb)) {
          // If a directory is given append the database file name
          if (globalTags.empty()) {
            // Default name if no tags given: look for compile time tag.txt
            std::string fileName = FileSystem::findFile(localdb) + "/" CURRENT_DEFAULT_TAG ".txt";
            B2DEBUG(30, "Adding fallback database " << fileName);
            LocalDatabase::createInstance(fileName, "", true, logLevel);
          } else {
            // One local DB for each global tag
            for (auto tag : globalTags) {
              std::string fileName = localdb + "/" + tag + ".txt";
              if (FileSystem::isFile(fileName)) {
                B2DEBUG(30, "Adding fallback database " << fileName);
                LocalDatabase::createInstance(fileName, "", true, logLevel);
              }
            }
          }
        }
        logLevel = LogConfig::c_Debug;
      }
    }
    // and add access to the central database unless we have an empty global tag
    // in which case we disable access to the database
    if (!globalTags.empty()) {
      // add all global tags which are separated by whitespace as conditions database
      for (auto tag : globalTags) {
        B2DEBUG(30, "Adding central database for global tag " << tag);
        ConditionsDatabase::createDefaultInstance(tag, LogConfig::c_Warning);
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
      B2DEBUG(39, "Replacing DatabaseChain with DatabaseChain: ignored");
      delete database;
    } else if (chain) {
      chain->addDatabase(database);
    } else if (replacement) {
      B2DEBUG(35, "Replacing Database with DatabaseChain: adding existing database to chain");
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
  DBStore::Instance().reset(true);
}


Database::~Database() {}

std::pair<TObject*, IntervalOfValidity> Database::getData(const EventMetaData& event, const std::string& name)
{
  DBStoreEntry entry(DBStoreEntry::c_Object, name, TObject::Class(), false, true);
  DBQuery query(name, true);
  getData(event, query);
  entry.updatePayload(query.revision, query.iov, query.filename, query.checksum, event);
  return std::make_pair(entry.releaseObject(), query.iov);
}

void Database::getData(const EventMetaData& event, std::list<DBQuery>& query)
{
  for (auto& entry : query) {
    if (!getData(event, entry)) {
      entry.filename = "";
      entry.revision = 0;
      entry.checksum = "";
    }
  }
}

bool Database::storeData(std::list<DBImportQuery>& query)
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

bool Database::writePayload(const std::string& fileName, const std::string& name, const TObject* object,
                            const IntervalOfValidity* iov) const
{
  TDirectory* saveDir = gDirectory;
  TFile* file = TFile::Open(fileName.c_str(), "RECREATE");
  if (!file || !file->IsOpen()) {
    B2ERROR("Could not open payload file " << std::quoted(fileName) << " for writing.");
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

  /** Configure the network settings for the Conditions database downloads */
  boost::python::dict setConditionsNetworkSettings(boost::python::tuple args, boost::python::dict kwargs)
  {
    using namespace boost::python;
    if (len(args) > 0) {
      // keyword only function: raise typerror on non-keyword arguments
      std::string count = std::to_string(len(args)) + (len(args) > 1 ? " were" : " was");
      PyErr_SetString(PyExc_TypeError, ("set_central_database_networkparams() takes 0 "
                                        "positional arguments but " + count + " given").c_str());
      throw_error_already_set();
    }
    std::vector<std::function<void(unsigned int)>> setters = {
      &ConditionsPayloadDownloader::setConnectionTimeout,
      &ConditionsPayloadDownloader::setStalledTimeout,
      &ConditionsPayloadDownloader::setMaxRetries,
      &ConditionsPayloadDownloader::setBackoffFactor
    };
    std::vector<std::function<unsigned int()>> getters = {
      &ConditionsPayloadDownloader::getConnectionTimeout,
      &ConditionsPayloadDownloader::getStalledTimeout,
      &ConditionsPayloadDownloader::getMaxRetries,
      &ConditionsPayloadDownloader::getBackoffFactor
    };
    std::vector<std::string> names = {"connection_timeout", "stalled_timeout", "max_retries", "backoff_factor"};
    dict result;
    for (size_t i = 0; i < names.size(); ++i) {
      const std::string n = names[i];
      if (kwargs.has_key(n)) {
        setters[i](extract<unsigned int>(kwargs[n]));
        boost::python::api::delitem(kwargs, n);
      }
      result[n] = getters[i]();
    }
    if (len(kwargs) > 0) {
      std::string message = "Unrecognized keyword arguments: ";
      auto keys = kwargs.keys();
      for (int i = 0; i < len(keys); ++i) {
        if (i > 0) message += ", ";
        message += extract<std::string>(keys[i]);
      }
      PyErr_SetString(PyExc_TypeError, message.c_str());
      throw_error_already_set();
    }
    return result;
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

void Database_setCentralServerList(boost::python::list serverList)
{
  // convert list of objects to strings to pass on
  namespace py = boost::python;
  std::vector<std::string> cppServerList;
  size_t nList = py::len(serverList);
  for (size_t iList = 0; iList < nList; ++iList) {
    cppServerList.emplace_back(py::extract<std::string>(serverList[iList].attr("__str__")()));
  }
  // and now find all conditions db instances and set the list
  std::vector<Database*> databases{&Database::Instance()};
  DatabaseChain* chain = dynamic_cast<DatabaseChain*>(databases[0]);
  if (chain) {
    databases = chain->getDatabases();
  }
  for (Database* db : databases) {
    ConditionsDatabase* cond = dynamic_cast<ConditionsDatabase*>(db);
    if (cond) {
      cond->setServerList(cppServerList);
    }
  }
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

  def("get_default_global_tags", &Database::getDefaultGlobalTags, "Get the default global tags for the central database");
  def("set_experiment_name", &Database_addExperimentName, args("experiment", "name"), R"DOCSTRING(
Set a name for the given experiment number when looking up payloads in the
central database.

.. deprecated:: release-00-09-00
   This function is deprecated and any calls to it are ignored.)DOCSTRING");
  def("reset_database", &Database::reset, "Reset the database setup to have no database sources");
  def("use_database_chain", &DatabaseChain::createInstance,
      (bp::arg("resetIoVs") = true, bp::arg("loglevel") = LogConfig::c_Warning, bp::arg("invertLogging") = false),
      R"DOCSTRING(
Use a database chain: Multiple database sources are used on a first found
basis, if the payload is not found in one source try the next and so on.

If the chain is enabled calls to `use_local_database` and `use_central_database`
will add these sources to the chain instead of replacing the current source.
If there was an existing single source setup when `use_database_chain` is
called this one source is kept as the first entry in the chain. Multiple calls
to this function don't have any effect.

Parameters:
  resetIoVs (bool): A flag to indicate whether IoVs from non-primary
        databases should be set to only the current run and rechecked for the
        next run.
  loglevel (LogLevel): The severity of messages from the database
        chain, defaults to `WARNING <LogLevel.WARNING>`
  invertLogging (bool): A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        ``loglevel`` will be emitted every time a payload cannot be found. If true
        a message will be emitted if a payload is actually found.
)DOCSTRING");
  def("use_local_database", &LocalDatabase::createInstance,
      (bp::arg("filename"), bp::arg("directory")="", bp::arg("readonly")=false,
       bp::arg("loglevel")=LogConfig::c_Warning, bp::arg("invertLogging")=false),
      R"DOCSTRING(
Use a local database backend: a single file containing the payload information in plain text.

Parameters:
  filename (str): filename containing the payload information, defaults to
        "database.txt"
  directory (str): directory containing the payloads, defaults to the directory
        of the database filename
  readonly (bool): if True the database will refuse to create new payloads
  loglevel (LogLevel): The severity of messages from this backend when
        payloads cannot be found, defaults to `WARNING <LogLevel.WARNING>`
  invertLogging (bool): A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        ``loglevel`` will be emitted ever time a payload cannot be found. If
        true a message will be emitted if a payload is actually found.
)DOCSTRING");
  {
  //use_central_database has different signatures so the docstring confuses sphinx. Handcraft one complete docstring.
  docstring_options subOptions(true, false, false);

  def("use_central_database", &ConditionsDatabase::createDefaultInstance,
      (bp::arg("globalTag"), bp::arg("loglevel")=LogConfig::c_Warning, bp::arg("payloaddir")="centraldb"));
  def("use_central_database", &ConditionsDatabase::createInstance,
      (bp::arg("globalTag"), bp::arg("restBaseName"), bp::arg("payloaddir"), bp::arg("fileBaseLocal"),
       bp::arg("loglevel")=LogConfig::c_Warning, bp::arg("invertLogging")=false),
      R"DOCSTRING(use_central_database(globalTag, restBaseName=None, payloadDir="centraldb", fileBaseLocal=None, loglevel=LogLevel.WARNING, invertLogging=False)

Use the central database to obtain conditions data. Usually users should only
need to call this with one parameter which is the global tag to identify the
payloads.

>>> use_central_database("my_global_tag")

It might be useful to also specify the log level and invert the log messages
when adding an additional global tag for lookups

>>> use_central_database("my_additional_tag", loglevel=LogLevel.WARNING, invertLogging=True)

The ``payloaddir`` specifies a directory where payloads which needed to be
downloaded will be placed. This could be set to a common absolute directory for
all jobs to make sure the payloads only need to be downloaded once. The default
is to place payloads into a directory called :file:`centraldb` in the local
working directory.

Warning:
    For debugging purposes this function also allows to set the base URL for
    the REST api and the file server but these should generally not be
    modified.

Parameters:
  globalTag (str): name of the global tag to use for payload lookup
  restBaseName (str): base URL for the REST api
  fileBaseName (str): base directory to look for payloads instead of
        downloading them.
  payloaddir (str): directory where to save downloaded payloads
  loglevel (LogLevel): The LogLevel of messages from this backend when
        payloads cannot be found, defaults to `WARNING <LogLevel.WARNING>`
  invertLogging (bool): A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        ``loglevel`` will be emitted every time a payload cannot be found. If
        true a message will be emitted if a payload is actually found.
)DOCSTRING");
  }

  object f = raw_function(setConditionsNetworkSettings);
  def("set_central_database_networkparams", f);
  // for some reason we cannot directly provide the docstring on def when using
  // raw_function so let's override the docstring afterwards
  setattr(f, "__doc__",
      R"DOCSTRING(set_central_database_networkparams(**kwargs)
Set the network parameters for the central database.

You can supply any combination of keyword-only arguments defined below. The
function will return a dictionary containing the new settings.

    >>> set_central_database_networkparams(connection_timeout=5, max_retries=1)
    {'backoff_factor': 5, 'connection_timeout': 5, 'max_retries': 1, 'stalled_timeout': 60}

Warning:
    Modification of these parameters should not be needed, in rare
    circumstances this could be used to optimize access for many jobs at once
    but should only be set by experts.

Parameters:
  connection_timeout (int): timeout in seconds before connection should be
      aborted. 0 sets the timeout to the default (300s)
  stalled_timeout (int): timeout in seconds before a download should be
      aborted if the speed stays below 10 KB/s, 0 disables this timeout
  max_retries (int): maximum amount of retries if the server responded with
      an HTTP response of 500 or more. 0 disables retrying
  backoff_factor (int): backoff factor for retries in seconds. Retries are
      performed using something similar to binary backoff: For retry :math:`n`
      and a ``backoff_factor`` :math:`f` we wait for a random time chosen
      uniformely from the interval :math:`[1, (2^{n} - 1) \times f]` in
      seconds.
)DOCSTRING");
  def("set_central_serverlist", &Database_setCentralServerList, bp::arg("serverList"),
      R"DOCSTRING(
Set a list of possible servers to connect to the central database. This should
almost never be needed but can be used to test alternative servers. For example

  >>> set_central_serverlist(["http://blcdb.sdcc.bnl.gov/b2s/rest/", "http://belle2db.hep.pnnl.gov/b2s/rest/"])

would first try a server at BNL and if that fails fall back to PNNL and if that
fails to continue without a central database. This list can also be set using
``BELLE2_CONDB_SERVERLIST`` where the servers should be separated by
whitespace.

Parameters:
  serverList (list(str)): List of urls to set for all configured central databases.
)DOCSTRING");
}
