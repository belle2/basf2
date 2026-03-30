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
    printInfoMessage(m_baseUrl);
    B2DEBUG(31, "Conditions Database: unusable globaltag states: " << invalidStates);
    for (const auto& status : validStates) {
      B2DEBUG(31, "Conditions Database: status marked as usable for global tags is not known to the database"
              << LogVar("status", status));

    }
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
                             "&minorIOV=" + std::to_string(run);
    try {
      const auto payloads = get(url);
      if (!payloads.is_array()) throw std::runtime_error("expected array");
      for (const auto& row : payloads) {
        if (!row.is_array()) throw std::runtime_error("expected row array");
        // Column indices from nopayloaddb SQL query:
        // 0: payload_type_name, 1: payload_url, 2: checksum, 3: size,
        // 4: major_iov, 5: minor_iov, 6: major_iov_end, 7: minor_iov_end
        // Also: check if the current (exp, run) falls into the payload IoV:
        // if yes, let's keep the payload, otherwise skip it.
        const IntervalOfValidity iov{row.at(4).get<int>(), row.at(5).get<int>(), row.at(6).get<int>(), row.at(7).get<int>()};
        if (iov.contains(exp, run)) {
          addPayload(PayloadMetadata(
                       row.at(0).get<std::string>(),  // payload_type_name (module name)
                       globaltag,
                       row.at(1).get<std::string>(),  // payload_url
                       "http://belle2db-files.sdcc.bnl.gov/",   // baseUrl (harcoded for now)
                       row.at(2).get<std::string>(),  // checksum
                       iov.getExperimentLow(),        // major_iov (expStart)
                       iov.getRunLow(),               // minor_iov (runStart)
                       iov.getExperimentHigh(),       // major_iov_end (expEnd)
                       iov.getRunHigh(),              // minor_iov_end (runEnd)
                       1                              // revision (not provided, use default)
                     ));
          B2INFO("Conditions Database: added payload from new central server"
                 << LogVar("module", row.at(0).get<std::string>())
                 << LogVar("url", row.at(1).get<std::string>())
                 << LogVar("checksum", row.at(2).get<std::string>()));
        }
      }

      B2INFO("Conditions Database: fetched " << payloads.size() << " payloads for globaltag"
             << LogVar("globaltag", globaltag) << LogVar("exp", exp) << LogVar("run", run));
    } catch (std::exception& e) {
      B2WARNING("Conditions Database: Problem while fetching the list of payloads"
                << LogVar("globaltag", globaltag) << LogVar("server url", m_baseUrl) << LogVar("query", url) << LogVar("error", e.what()));
      return false;
    }
    return true;
  }

} // Belle2::Conditions namespace
