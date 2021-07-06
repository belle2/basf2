/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    auto& conf = Conditions::Configuration::getInstance();
    conf.setInitialized(false);
    DBStore::Instance().reset(true);
    Instance().m_configState = c_PreInit;
    Instance().m_metadataProvider.reset();
    Instance().m_payloadCreation.reset();
    if (not keepConfig)
      conf.reset();
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

  void Database::initialize(const EDatabaseState target)
  {
    if (m_configState >= target) return;
    auto conf = Conditions::Configuration::getInstance();

    if (m_configState == c_PreInit) {
      // first step: freeze the configuration object and determine the list of globaltags
      // this calculates if tag replay is possible and will create an error otherwise but
      // it will not do anything else than setting the final list of globaltags
      conf.setInitialized(true);
      m_globalTags = conf.getFinalListOfTags();
      // and remove duplicates, there's no need to look in the same gt multiple times
      std::set<std::string> seen;
      m_globalTags.erase(std::remove_if(m_globalTags.begin(), m_globalTags.end(),
      [&seen](const auto & tag) {
        return not seen.insert(tag).second;
      }), m_globalTags.end());
      // and also obtain usable tag states and metadata providers
      m_usableTagStates = conf.getUsableTagStates();
      m_metadataConfigurations = conf.getMetadataProviders();
      // reverse because we want to pop out elements when used
      std::reverse(m_metadataConfigurations.begin(), m_metadataConfigurations.end());
      m_configState = c_InitGlobaltagList;
    }
    // do we want to stop early?
    if (m_configState >= target) return;
    if (m_configState == c_InitGlobaltagList) {
      // setup the first working provider;
      if (m_metadataConfigurations.empty()) {
        m_metadataProvider = std::make_unique<Conditions::NullMetadataProvider>();
      } else {
        nextMetadataProvider();
      }
      // we will actually use the globaltags so print them now
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
      m_configState = c_Ready;
    }
  }

  void Database::exposePythonAPI()
  {
    // To avoid confusion between std::arg and boost::python::arg we want a shorthand namespace as well
    namespace py = boost::python;

    // Make sure the default instance is created
    Database::Instance();

    // Don't show c++ signature in python doc to keep it simple
    py::docstring_options options(true, true, false);

    // Expose our shiny configuration object
    Conditions::Configuration::exposePythonAPI();
  }
}
