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
#include <framework/database/LocalDatabase.h>
#include <framework/database/ConditionsDatabase.h>
#include <framework/database/DatabaseChain.h>
#include <framework/database/DBStore.h>
#include <framework/database/PayloadFile.h>

#include <framework/database/PayloadProvider.h>
#include <framework/database/MetadataProvider.h>
#include <framework/database/LocalMetadataProvider.h>
#include <framework/database/CentralMetadataProvider.h>
#include <framework/database/Configuration.h>

#include <cstdlib>

namespace Belle2 {

  Database& Database::Instance()
  {
    static Database instance;
    return instance;
  }

  Database::~Database() = default;

  void Database::reset(bool keepConfig)
  {
    DBStore::Instance().reset(true);
    Instance().m_metadataProvider.reset();
    Instance().m_payloadCreation.reset();
    if (not keepConfig)
      Conditions::Configuration::getInstance().reset();
  }

  ScopeGuard Database::createScopedUpdateSession()
  {
    // make sure we reread testing text files in case they got updated
    for (auto& testing : m_testingPayloads) {
      testing.reset();
    }
    // and return a downloader session guard for the downloader we use
    return Conditions::Downloader::getDefaultInstance().ensureSession();
  }

  std::pair<TObject*, IntervalOfValidity> Database::getData(const EventMetaData& event, const std::string& name)
  {
    DBStoreEntry entry(DBStoreEntry::c_Object, name, TObject::Class(), false, true);
    std::vector<DBQuery> query{DBQuery{name, true}};
    getData(event, query);
    entry.updatePayload(query[0].revision, query[0].iov, query[0].filename, query[0].checksum, event);
    return std::make_pair(entry.releaseObject(), query[0].iov);
  }

  bool Database::getData(const EventMetaData& event, std::vector<DBQuery>& query)
  {
    // initialize lazily ...
    if (!m_metadataProvider) initialize();
    // So first go over the requested payloads once, reset the info and check for any
    // testing payloads we might want to use
    const size_t testingPayloads = std::count_if(query.begin(), query.end(), [this, &event](auto & payload) {
      // make sure the queries are "reset" to invalid revision and no filename before we start looking
      payload.filename = "";
      payload.revision = 0;
      // and now look in all testing payload providers if any.
      for (auto& tmp : m_testingPayloads) {
        if (tmp.get(event, payload)) return true;
      }
      return false;
    });
    // if we already found all just return here
    if (testingPayloads == query.size()) return true;
    // nooow, lets look for proper payloads;
    try {
      m_metadataProvider->getPayloads(event.getExperiment(), event.getRun(), query);
    } catch (std::exception&) {
      // something went wrong with the metadata update ... so let's try next provider
      B2ERROR("Conditions data: Problem with payload metadata, trying to fall back to next provider ...");
      nextMetadataProvider();
      return getData(event, query);
    }
    // and if we could find the metadata lets also locate the payloads ...
    const size_t payloadsLocated = std::count_if(query.begin(), query.end(), [this](auto & payload) {
      // make sure we don't overwrite local payloads or otherwise already valid filenames;
      if (!payload.filename.empty()) return true;
      // but don't check for payloads we could not find. But this is only a
      // problem if they are required so report success for not required
      // payloads
      if (payload.revision == 0) return not payload.required;
      // and locate the payload.
      if (not m_payloadProvider->find(payload)) {
        // if that fails lets let the user know: Even for optional payloads, if
        // we know the metadata but cannot find the file something is fishy and
        // should be reported.
        auto loglevel = payload.required ? LogConfig::c_Error : LogConfig::c_Warning;
        B2LOG(loglevel, 0, "Conditions data: Could not find file for payload"
              << LogVar("name", payload.name) << LogVar("revision", payload.revision)
              << LogVar("checksum", payload.checksum) << LogVar("globaltag", payload.globaltag));
        return not payload.required;
      }
      return true;
    });
    // did we find all payloads?
    return payloadsLocated == query.size();
  }

  bool Database::storeData(std::list<DBImportQuery>& query)
  {
    return std::all_of(query.begin(), query.end(), [this](const auto & import) {
      return storeData(import.name, import.object, import.iov);
    });
  }

  bool Database::storeData(const std::string& name, TObject* obj, const IntervalOfValidity& iov)
  {
    if (!m_payloadCreation) initialize();
    auto result = m_payloadCreation->storeData(name, obj, iov);
    // we added payloads, make sure we reread testing files on next try
    if (result) {
      for (auto& testing : m_testingPayloads) {
        testing.reset();
      }
    }
    return result;
  }

  std::string Database::getGlobalTags()
  {
    return boost::algorithm::join(m_globalTags, ",");
  }

  void Database::nextMetadataProvider()
  {
    if (m_metadataConfigurations.empty()) {
      B2FATAL("Conditions data: No more metadata providers available");
    }
    auto provider = m_metadataConfigurations.back();
    m_metadataConfigurations.pop_back();
    bool remote{false};
    if (auto pos = provider.find("://"); pos != std::string::npos) {
      // found a protocol: if file remove, otherwise keep as is and set as remote ...
      auto protocol = provider.substr(0, pos);
      boost::algorithm::to_lower(protocol);
      if (protocol == "file") {
        provider = provider.substr(pos + 3);
      } else if (protocol == "http" or protocol == "https") {
        remote = true;
      } else {
        B2FATAL("Conditions data: Unknown metadata protocol, only supported protocols for payload metadata are file, http, https"
                << LogVar("protocol", protocol));
      }
    }
    try {
      if (remote) {
        m_metadataProvider = std::make_unique<Conditions::CentralMetadataProvider>(provider, m_usableTagStates);
      } else {
        m_metadataProvider = std::make_unique<Conditions::LocalMetadataProvider>(provider, m_usableTagStates);
      }
    } catch (std::exception& e) {
      B2WARNING("Conditions data: Metadata provider not usable, trying next one ..."
                << LogVar("provider", provider) << LogVar("error", e.what()));
      return nextMetadataProvider();
    }
    // and check the tags are useable
    if (!m_metadataProvider->setTags(m_globalTags)) {
      B2FATAL("Conditions data: Problems with globaltag configuration, cannot continue");
    }
  }

  void Database::initialize()
  {
    auto conf = Conditions::Configuration::getInstance();
    m_globalTags = conf.getFinalListOfTags();
    m_usableTagStates = conf.getUsableTagStates();
    m_metadataConfigurations = conf.getMetadataProviders();
    // reverse because we want to pop out elements when used
    std::reverse(m_metadataConfigurations.begin(), m_metadataConfigurations.end());
    // and setup the first working provider;
    if (m_metadataConfigurations.empty()) {
      m_metadataProvider = std::make_unique<Conditions::NullMetadataProvider>();
    } else {
      nextMetadataProvider();
    }
    if (!m_globalTags.empty()) {
      // Globaltags are useable so print out the final list we're gonna use
      if (m_globalTags.size() == 1) {
        B2INFO("Conditions data: configured globaltag is " << m_globalTags[0]);
      } else {
        B2INFO("Conditions data: configured globaltags (highest priority first) are " << boost::algorithm::join(m_globalTags, ", "));
      }
    }
    // Configure payload location/download
    m_payloadProvider = std::make_unique<Conditions::PayloadProvider>(
                          conf.getPayloadLocations(),
                          conf.getDownloadCacheDirectory(),
                          conf.getDownloadLockTimeout()
                        );
    // Also we need to be able to create payloads ...
    m_payloadCreation = std::make_unique<Conditions::TestingPayloadStorage>(conf.getNewPayloadLocation());
    // And maaaybe we want to use testing payloads
    m_testingPayloads.clear();
    for (const auto& path : conf.getTestingPayloadLocations()) {
      B2INFO("Conditions data: configured to use testing payloads" << LogVar("location", path));
      m_testingPayloads.emplace_back(path);
    }
    // If so, warn again ... because
    if (not m_testingPayloads.empty()) {
      B2WARNING(R"(Conditions data: configured to look for temporary tesing payloads from one or more local folders.

  This will lead to non-reproducible results and is strictly only for testing purposes.
  It is NOT ALLOWED for official productions or data analysis and any results obtained like this WILL NOT BE PUBLISHED.
)";);
    }
  }

// we know LocalDatabase/ConditionsDatabase/DatabaseChain are deprecated, we don't want the warnings when compiling the service itself ...
#ifdef __INTEL_COMPILER
#pragma warning (disable:1478) //[[deprecated]]
#pragma warning (disable:1786) //[[deprecated("message")]]
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

  void Database::exposePythonAPI()
  {
    // to avoid confusion between std::arg and boost::python::arg we want a shorthand namespace as well
    namespace py = boost::python;
    // make sure the default instance is created
    Database::Instance();

    //don't show c++ signature in python doc to keep it simple
    py::docstring_options options(true, true, false);

    py::def("reset_database", &Database::reset, (py::arg("keep_config") = false),
        R"DOC(Reset the database setup to have no database sources

.. deprecated:: release-04-00-00
   Please use `basf2.conditions` for all configuration of the conditions database)DOC");
    py::def("use_database_chain", &DatabaseChain::createInstance,
        (py::arg("resetIoVs") = true, py::arg("loglevel") = LogConfig::c_Warning, py::arg("invertLogging") = false),
        R"DOCSTRING(
Use a database chain. This function used to be necessary to enable usage of
multiple globaltags but has been deprecated.

.. deprecated:: release-04-00-00
   This function is no longer needed and is just kept for compatibility.
   In its current state it does nothing and ignores all arguments.
   Please use `basf2.conditions` for all configuration of the conditions database
)DOCSTRING");
  py::def("use_local_database", &LocalDatabase::createInstance,
      (py::arg("filename"), py::arg("directory")="", py::arg("readonly")=false,
       py::arg("loglevel")=LogConfig::c_Warning, py::arg("invertLogging")=false),
      R"DOCSTRING(
Use a local database backend: a single file containing the payload information in plain text.

Parameters:
  filename (str): filename containing the payload information, defaults to
        "database.txt". This file needs to exist.
  directory (str): directory containing the payloads, defaults to the directory
        of the database filename. This parameter doesn't have any effect anymore:
        payloads **must** be in the same directory as the text file. If this parameter
        is not empty and doesn't point to the same directory a error is raised.
  readonly (bool): if True the database will refuse to create new payloads.
        This parameter doesn't have any effect anymore. Local databases are always readonly
  loglevel (LogLevel): The severity of messages from this backend when
        payloads cannot be found. This parameter doesn't have any effect anymore.
  invertLogging (bool): A flag to indicate whether logging of obtained
        payloads should be inverted. This parameter doesn't have any effect anymore.

.. deprecated:: release-04-00-00
   Most of the parameters don't have any effect anymore and are kept for compatibility
   Please use `basf2.conditions` for all configuration of the conditions database.
)DOCSTRING");
  {
  //use_central_database has different signatures so the docstring confuses sphinx. Handcraft one complete docstring.
  py::docstring_options subOptions(true, false, false);

  py::def("use_central_database", &ConditionsDatabase::createDefaultInstance,
      (py::arg("globalTag"), py::arg("loglevel")=LogConfig::c_Warning, py::arg("payloaddir")="centraldb"));
  py::def("use_central_database", &ConditionsDatabase::createInstance,
      (py::arg("globalTag"), py::arg("restBaseName"), py::arg("payloaddir"), py::arg("fileBaseLocal"),
       py::arg("loglevel")=LogConfig::c_Warning, py::arg("invertLogging")=false),
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
  restBaseName (str): base URL for the REST api. This parameter is no longer supported,
      any value other than an empty string will lead to an error.
  fileBaseName (str): base directory to look for payloads instead of
      downloading them. This parameter is no longer supported,
      any value other than an empty string will lead to an error.
  payloaddir (str): directory where to save downloaded payloads.
  loglevel (LogLevel): The LogLevel of messages from this backend when
      payloads cannot be found. This parameter is no longer supported and is ignored.
  invertLogging (bool): A flag to indicate whether logging of obtained
      payloads should be inverted. This parameter is no longer supported and is ignored.

.. deprecated:: release-04-00-00
   Most of the parameters don't have any effect anymore and are kept for compatibility
   Please use `basf2.conditions` for all configuration of the conditions database.
)DOCSTRING");
  }
    // and expose our shiny configuration object
    Conditions::Configuration::exposePythonAPI();
  }
}
