/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/HSFCentralMetadataProvider.h>
#include <framework/database/MetadataProvider.h>
#include <framework/database/Downloader.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

using json = nlohmann::json;

namespace Belle2::Conditions {

  HSFCentralMetadataProvider::HSFCentralMetadataProvider(std::string baseUrl, const std::set<std::string>& usableTagStates):
    MetadataProvider(usableTagStates), m_baseUrl(std::move(baseUrl))
  {
    // We want to be sure on construction that the server is working. So let's
    // just check the list of valid states
    const auto result = get("/gtstatus");
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
    B2DEBUG(31, "Infinite IoV value is set to " << m_maxIoV);
    printInfoMessage(m_baseUrl);
    B2DEBUG(31, "Conditions Database: unusable globaltag states: " << invalidStates);
    for (const auto& status : validStates) {
      B2DEBUG(31, "Conditions Database: status marked as usable for global tags is not known to the database"
              << LogVar("status", status));

    }
    const auto payloadUrl = get("user_settings/CDB_PAYLOAD_BASE_URL");
    m_payloadBaseUrl = payloadUrl.at("CDB_PAYLOAD_BASE_URL");
    B2DEBUG(31, "Fetched payload location" << LogVar("URL", m_payloadBaseUrl));
  }

  json HSFCentralMetadataProvider::get(const std::string& url)
  {
    std::stringstream stream;
    const auto fullUrl = m_downloader.joinWithSlash(m_baseUrl, url);
    m_downloader.download(fullUrl, stream);
    stream.clear();
    stream.seekg(0, std::ios::beg);
    return json::parse(stream);
  }

  std::string HSFCentralMetadataProvider::getGlobaltagStatus(const std::string& name)
  {
    auto escaped = m_downloader.escapeString(name);
    const std::string url = "/globalTag/" + escaped;
    try {
      const auto gtinfo = get(url);
      return gtinfo.at("status").at("name");
    } catch (std::runtime_error& e) {
      B2WARNING("Conditions Database: Cannot download information on global tag. "
                "Either the database is overloaded or the selected globaltag doesn't exist"
                << LogVar("server url", m_baseUrl) << LogVar("globaltag", name) << LogVar("query", url));
    } catch (std::exception& e) {
      B2WARNING("Conditions Database: Problem determining global tag status"
                << LogVar("server url", m_baseUrl) << LogVar("globaltag", name) << LogVar("query", url) << LogVar("error", e.what()));
    }
    return "";
  }

  bool HSFCentralMetadataProvider::updatePayloads(const std::string& globaltag, int exp, int run)
  {
    auto escaped = m_downloader.escapeString(globaltag);
    const std::string url =  "payloadiovs/?gtName=" + escaped +
                             "&majorIOV=" + std::to_string(exp) +
                             "&minorIOV=" + std::to_string(run) +
                             "&shape=dict";
    try {
      const auto payloads = get(url);
      if (!payloads.is_array()) throw std::runtime_error("expected array");
      for (const auto& payload : payloads) {
        if (!payload.is_object()) throw std::runtime_error("expected payload object");

        const long long int experimentHighFromServer = payload.at("major_iov_end");
        const long long int runHighFromServer = payload.at("minor_iov_end");

        // The infinite IoV is represented by m_maxIoV; in basf2 we replace it for -1 for historical reasons
        const long long int experimentHigh = (experimentHighFromServer == m_maxIoV) ? -1 : experimentHighFromServer;
        const long long int runHigh = (runHighFromServer == m_maxIoV) ? -1 : runHighFromServer;

        // Check if the current (exp, run) falls into the payload IoV:
        // if yes, let's keep the payload, otherwise skip it.
        const IntervalOfValidity iov{payload.at("major_iov"), payload.at("minor_iov"), static_cast<int>(experimentHigh), static_cast<int>(runHigh)};
        if (iov.contains(exp, run)) {
          addPayload(PayloadMetadata(
                       payload.at("payload_type_name"),
                       globaltag,
                       payload.at("payload_url"),
                       m_payloadBaseUrl,
                       payload.at("checksum"),
                       iov.getExperimentLow(),
                       iov.getRunLow(),
                       iov.getExperimentHigh(),
                       iov.getRunHigh(),
                       convertString<unsigned long int>(payload.at("revision"))
                     ));
          B2DEBUG(31, "Conditions Database: added payload from new central server"
                  << LogVar("Payload type name", payload.at("payload_type_name"))
                  << LogVar("url", payload.at("payload_url"))
                  << LogVar("checksum", payload.at("checksum")));
        }
      }

      B2DEBUG(31, "Conditions Database: fetched " << payloads.size() << " payloads for globaltag"
              << LogVar("globaltag", globaltag) << LogVar("exp", exp) << LogVar("run", run));
    } catch (std::exception& e) {
      B2WARNING("Conditions Database: Problem while fetching the list of payloads"
                << LogVar("globaltag", globaltag) << LogVar("server url", m_baseUrl) << LogVar("query", url) << LogVar("error", e.what()));
      return false;
    }
    return true;
  }

} // Belle2::Conditions namespace
