/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/EConditionsDirectoryStructure.h>
#include <framework/database/PayloadMetadata.h>
#include <framework/database/Downloader.h>
#include <framework/utilities/FileSystem.h>

#include <vector>
#include <string>
#include <unordered_map>

namespace Belle2::Conditions {
  /** Class to find payload files in a list of locations */
  class PayloadProvider {
  public:
    /** Simple struct to represent a lookup location */
    struct PayloadLocation {
      /** base path or uri */
      std::string base;
      /** How are payloads structured in this location */
      EConditionsDirectoryStructure structure;
      /** Is this a remote location where we want to download the files? */
      bool isRemote;
    };

    /** Constructor for a given list of locations and optionally the location
     * where downloaded payloads should be cached Each location can either be a
     * path to a directory or a http/https url of a server where the payloads
     * can be downloaded (starting with `http(s)://`).
     *
     * In addition each path can have an option after a question mark. Possible
     * values are
     * - "flat": All payloads in one directory
     * - "digest": Payloads in subdirectories /A/BC/ where A,B and C are the
     *   first three characters of the checksum of the payload file
     * - "logical": As given in the payloadUrl of the metadata with
     *   subdirectories for the payload name
     *
     * If cachedir is empty a default value of `$TMPDIR/basf2-conditions?digest`
     * is assumed.
     */
    PayloadProvider(const std::vector<std::string>& locations, const std::string& cachedir = "", int timeout = 60);

    /** try to find a payload, return true on success, false if it cannot be
     * found.
     *
     * This will go through all configured payloads locations and if it can find
     * the payload will set the filename member of the metadata instance and
     * return true.
     */
    bool find(PayloadMetadata& meta);
  private:
    /** Look for a payload in the local directory location, set the filename
     * member of the metadata instance and return true on succes */
    bool getLocalFile(const PayloadLocation& loc, PayloadMetadata& meta) const;
    /** Look for a payload on a remote server and download if possible, set the
     * filename member of the metadata instance and return true on succes */
    bool getRemoteFile(const PayloadLocation& loc, PayloadMetadata& meta);
    /** Try to download url into a temporary file, if successful set the
     * filename member of the metadata and return true. Otherwise return false.
     * If silentOnMissing is true a 404 error will not be treated as worthy of a
     * message */
    bool getTemporaryFile(const std::string& url, PayloadMetadata& meta, bool silentOnMissing);
    /** Return the filename of a payload to look for given a directory structure and some metadata */
    std::string getFilename(EConditionsDirectoryStructure structure, const PayloadMetadata& payload) const;
    /** List of configured lookup locations: The first one will always be the
     * cache directory and the last one will always be fallback url included in
     * the payload metadata */
    std::vector<PayloadLocation> m_locations;
    /** Location of the cache directory where/how we want to store downloaded payloads */
    PayloadLocation m_cacheDir;
    /** Instance to the database file downloading instance */
    Downloader& m_downloader{Downloader::getDefaultInstance()};
    /** Map of all active temporary files we downloaded and keep around until they can be closed */
    std::unordered_map<std::string, std::unique_ptr<FileSystem::TemporaryFile>> m_temporaryFiles;
    /** Timeout to wait for a write look when trying to download payloads */
    int m_timeout;
  };
} // Belle2::Conditions namespace
