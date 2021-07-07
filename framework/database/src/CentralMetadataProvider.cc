/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/CentralMetadataProvider.h>
#include <framework/logging/Logger.h>

using json = nlohmann::json;

namespace Belle2::Conditions {
  CentralMetadataProvider::CentralMetadataProvider(std::string baseUrl, const std::set<std::string>& usableTagStates):
    MetadataProvider(usableTagStates), m_baseUrl(std::move(baseUrl))
  {
    // We want to be sure on construction that the server is working. So let's
    // just check the list of valid states
    const auto result = get("/v2/globalTagStatus");
    // check list of valid states
    auto validStates = getUsableTagStates();
    std::string invalidStates = "";
    for (const auto& info : result) {
      const std::string status = info.at("name");
      if (not validStates.erase(status)) {
        if (!invalidStates.empty()) invalidStates += ", ";
        invalidStates += status;
      }
    }
    B2INFO("Conditions Database: found working server" << LogVar("url", m_baseUrl));
    B2DEBUG(31, "Conditions Database: unusable globaltag states: " << invalidStates);
    for (const auto& status : validStates) {
      B2WARNING("Conditions Database: status marked as usable for global tags is not known to the database"
                << LogVar("status", status));

    }
  }

  json CentralMetadataProvider::get(const std::string& url)
  {
    std::stringstream stream;
    const auto fullUrl = m_downloader.joinWithSlash(m_baseUrl, url);
    m_downloader.download(fullUrl, stream);
    stream.clear();
    stream.seekg(0, std::ios::beg);
    return json::parse(stream);
  }

  std::string CentralMetadataProvider::getGlobaltagStatus(const std::string& name)
  {
    auto escaped = m_downloader.escapeString(name);
    const std::string url = "/v2/globalTag/" + escaped;
    try {
      const auto gtinfo = get(url);
      return gtinfo.at("globalTagStatus").at("name");
    } catch (std::runtime_error& e) {
      B2ERROR("Conditions Database: Cannot download information on global tag. Usually this means it "
              "doesn't exist and you misspelled the name"
              << LogVar("server url", m_baseUrl) << LogVar("globaltag", name));
    } catch (std::exception& e) {
      B2ERROR("Conditions Database: Problem determining global tag status"
              << LogVar("server url", m_baseUrl) << LogVar("globaltag", name) << LogVar("error", e.what()));
    }
    return "";
  }

  bool CentralMetadataProvider::updatePayloads(const std::string& globaltag, int exp, int run)
  {
    auto escaped = m_downloader.escapeString(globaltag);
    const std::string url =  "v2/iovPayloads/?gtName=" + escaped +
                             "&expNumber=" + std::to_string(exp) +
                             "&runNumber=" + std::to_string(run);
    try {
      const auto payloads = get(url);
      if (!payloads.is_array()) throw std::runtime_error("expected array");
      for (const auto& info : payloads) {
        if (!info.is_object()) throw std::runtime_error("excpected payload object");
        const auto& payload = info.at("payload");
        const auto& iov = info.at("payloadIov");
        addPayload(PayloadMetadata(
                     payload.at("basf2Module").at("name"),
                     globaltag,
                     payload.at("payloadUrl"),
                     payload.at("baseUrl"),
                     payload.at("checksum"),
                     iov.at("expStart"), iov.at("runStart"), iov.at("expEnd"), iov.at("runEnd"),
                     payload.at("revision")
                   ));
      }
    } catch (std::exception& e) {
      B2ERROR("Conditions Database: Problem parsing payload information."
              << LogVar("globaltag", globaltag) << LogVar("server url", m_baseUrl) << LogVar("error", e.what()));
      return false;
    }
    return true;
  }
} // Belle2::Conditions namespace
