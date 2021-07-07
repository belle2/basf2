/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

#include <framework/database/MetadataProvider.h>
#include <framework/utilities/sqlite.h>

namespace Belle2::Conditions {
  /** Class to obtain metadata of all valid payloads from a local SQLite file
   * instead of the central server.
   *
   * This is intended as a fallback in case the official server is not
   * reachable or a way to download the conditions for use without internet
   * connection.
   *
   * In contrast to a LocalDatabase this class just provides local access to
   * payload information previously downloaded from the central server.
   * There's no method nor is it intended to put custom payloads for testing
   * into such a downloaded file. It is always assumed that the content in these
   * files is identical to the central database server
   *
   * The requirements for the sqlite file are
   *
   * - there needs to be a table/view which allows to check for the global tag status
   *   \code{.sql}
       SELECT globalTagStatus FROM globaltags WHERE globalTagName=?
       \endcode
   * - there needs to be a table/view which allows to check for payloads like so
   *   \code{.sql}
       SELECT
         payloadName, globalTagName, payloadUrl, baseUrl, checksum,
         firstExp, firstRun, finalExp, finalRun, revision
       FROM iov_payloads
       WHERE globalTagName=:globaltag AND firstExp<=:exp AND firstRun<=:run AND
         (finalExp<0 OR (finalRun<0 AND finalExp>=:exp) OR (finalExp>=:exp AND finalRun>=:run))
       \endcode
   *
   */
  class LocalMetadataProvider: public MetadataProvider {
  public:
    /** Construct with the name of the sqlite file */
    LocalMetadataProvider(std::string filename, const std::set<std::string>& usableTagStates);
    /** copy constructor */
    LocalMetadataProvider(const LocalMetadataProvider&) = delete;
    /** move constructor */
    LocalMetadataProvider(LocalMetadataProvider&&) = delete;
    /** assignment operator */
    LocalMetadataProvider& operator=(const LocalMetadataProvider&) = delete;
    /** move assignment operator */
    LocalMetadataProvider& operator=(LocalMetadataProvider&&) = delete;
    /** default destructor */
    virtual ~LocalMetadataProvider() = default;

    /** Return the status of the given globaltag
     *
     * @param globaltag name of the globaltag
     * @returns the string representation of the global tag status (like 'OPEN',
     *     'PUBLISHED', 'INVALID') or an empty string if the tag does not exist.
     */
    std::string getGlobaltagStatus(const std::string& globaltag) override;

    /** Update the list of existing payloads from a given globaltag, exp and run combination.
     * Calls addPayload() for each payload it finds
     * @returns true on success, false on failure
     */
    bool updatePayloads(const std::string& globaltag, int exp, int run) override;
  private:
    /** Filename for the sqlite database file */
    std::string m_filename;
    /** SQLite connection object */
    sqlite::Connection m_connection;
    /** SQLite statement to return the status of a global tag */
    sqlite::SimpleStatement<std::string> m_globaltagStatus;
    /** SQLite statement to return all payloads for a given globaltag, exp, run */
    sqlite::ObjectStatement<PayloadMetadata,
           std::string, std::string, std::string, std::string, std::string,
           int, int, int, int, int> m_selectPayloads;
  };

} // namespace Belle2::Conditions
