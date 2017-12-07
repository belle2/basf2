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
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/DBStore.h>
#include <framework/database/PayloadFile.h>
#include <framework/database/ConditionsPayloadDownloader.h>

#include <TFile.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>

#define CURRENT_DEFAULT_TAG "GT_gen_prod_004.11_Master-20171213-230000"

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

std::string Database::getDefaultGlobalTags()
{
  return getFromEnvironment("BELLE2_CONDB_GLOBALTAG", CURRENT_DEFAULT_TAG);
}

Database& Database::Instance()
{
  if (!s_instance) {
    DatabaseChain::createInstance(true);
    const std::string fallback = getFromEnvironment("BELLE2_CONDB_FALLBACK", "/cvmfs/belle.cern.ch/conditions");
    const std::string globalTag = getDefaultGlobalTags();
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    std::vector<string> tags;
    boost::split(tags, globalTag, boost::is_any_of(" \t\n\r"));
    // OK, add fallback databases unless empty location is specified
    if (!fallback.empty()) {
      auto logLevel = LogConfig::c_Error;
      for (auto localdb : tokenizer(fallback, boost::char_separator<char> {" \t\n\r"})) {
        if (FileSystem::isFile(FileSystem::findFile(localdb, true))) {
          // If a file name is given use it as local DB
          B2DEBUG(10, "Adding fallback database " << FileSystem::findFile(localdb));
          LocalDatabase::createInstance(FileSystem::findFile(localdb), "", true, logLevel);
        } else if (FileSystem::isDir(localdb)) {
          // If a directory is given append the database file name
          if (globalTag.empty()) {
            // Default name if no tags given: look for compile time tag.txt
            std::string fileName = FileSystem::findFile(localdb) + "/" CURRENT_DEFAULT_TAG ".txt";
            B2DEBUG(10, "Adding fallback database " << fileName);
            LocalDatabase::createInstance(fileName, "", true, logLevel);
          } else {
            // One local DB for each global tag
            for (auto tag : tags) {
              std::string fileName = localdb + "/" + tag + ".txt";
              if (FileSystem::isFile(fileName)) {
                B2DEBUG(10, "Adding fallback database " << fileName);
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
    if (!globalTag.empty()) {
      // add all global tags which are separated by whitespace as conditions database
      for (auto tag : tags) {
        B2DEBUG(10, "Adding central database for global tag " << tag);
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

:param bool resetIoVs: A flag to indicate whether IoVs from non-primary
        databases should be set to only the current run and rechecked for the
        next run.
:param LogLevel loglevel: The severity of messages from the database
        chain, defaults to `WARNING <LogLevel.WARNING>`
:param bool invertLogging: A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        ``loglevel`` will be emitted every time a payload cannot be found. If true
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
:param LogLevel loglevel: The severity of messages from this backend when
        payloads cannot be found, defaults to `WARNING <LogLevel.WARNING>`
:param bool invertLogging: A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        ``loglevel`` will be emitted ever time a payload cannot be found. If true
        a message will be emitted if a payload is actually found.
)DOCSTRING");
  {
  //use_central_database has different signatures so the docstring confuses sphinx. Handcraft one complete docstring.
  docstring_options options(true, false, false);

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

.. warning::

    For debugging purposes this function also allows to set the base URL for
    the REST api and the file server but these should generally not be
    modified.


:param str globalTag: name of the global tag to use for payload lookup
:param str restBaseName: base URL for the REST api
:param str fileBaseName: base URL for the payload download
:param str payloaddir: directory where to save downloaded payloads
:param LogLevel loglevel: The LogLevel of messages from this backend when
        payloads cannot be found, defaults to `WARNING <LogLevel.WARNING>`
:param bool invertLogging: A flag to indicate whether logging of obtained
        payloads should be inverted. If False a log message with level
        ``loglevel`` will be emitted every time a payload cannot be found. If true
        a message will be emitted if a payload is actually found.
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

.. warning::

    Modification of these parameters should not be needed, in rare
    circumstances this could be used to optimize access for many jobs at once
    but should only be set by experts.

:param int connection_timeout: timeout in seconds before connection should be
    aborted. 0 sets the timeout to the default (300s)
:param int stalled_timeout: timeout in seconds before a download should be
    aborted if the speed stays below 10 KB/s, 0 disables this timeout
:param int max_retries: maximum amount of retries if the server responded with
    an HTTP response of 500 or more. 0 disables retrying
:param int backoff_factor: backoff factor for retries in seconds. Retries are
   performed using something similar to binary backoff: For retry :math:`n` and
   a ``backoff_factor`` :math:`f` we wait for a random time chosen uniformely
   from the interval :math:`[1, (2^{n} - 1) \times f]` in seconds.
)DOCSTRING");
}
