/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/LocalMetadataProvider.h>

#include <framework/logging/Logger.h>

namespace Belle2::Conditions {

  LocalMetadataProvider::LocalMetadataProvider(std::string filename, const std::set<std::string>& usableTagStates):
    MetadataProvider(usableTagStates), m_filename{std::move(filename)}, m_connection{m_filename},
    m_globaltagStatus{m_connection, "SELECT globalTagStatus FROM globaltags WHERE globalTagName=:globaltag", true},
    m_selectPayloads{m_connection, R"SQL(
        SELECT
          payloadName, globalTagName, payloadUrl, baseUrl, checksum,
          firstExp, firstRun, finalExp, finalRun, revision
        FROM iov_payloads
        WHERE globalTagName=:globaltag AND
          ((firstExp==:exp AND firstRun<=:run) OR firstExp<:exp) AND
          (finalExp<0 OR (finalRun<0 AND finalExp>=:exp) OR finalExp>:exp OR (finalExp==:exp AND finalRun>=:run));
      )SQL", true}
  {}

  std::string LocalMetadataProvider::getGlobaltagStatus(const std::string& globaltag)
  {
    try {
      if (m_globaltagStatus.execute(globaltag).step()) {
        return m_globaltagStatus.getRow();
      }
      B2ERROR("Local Database: Global tag does not exist"
              << LogVar("database", m_filename) << LogVar("globaltag", globaltag));
    } catch (std::exception& e) {
      B2ERROR("Local Database: Error checking globaltag"
              << LogVar("database", m_filename) << LogVar("globaltag", globaltag) << LogVar("error", e.what()));
    }
    return "";
  }

  bool LocalMetadataProvider::updatePayloads(const std::string& globaltag, int exp, int run)
  {
    try {
      for (auto && row : m_selectPayloads.execute(globaltag, exp, run)) {
        addPayload(std::move(row), "Local Database");
      }
    } catch (std::exception& e) {
      B2ERROR("Local Database: Error obtaining payloads"
              << LogVar("database", m_filename) << LogVar("globaltag", globaltag) << LogVar("error", e.what()));
      return false;
    }
    return true;
  }
} // Belle2::Conditions namespace
