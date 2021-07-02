/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

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
      /** Is this a remote location where we want to download the files? */
      bool isRemote;
    };

    /** Enumeration of different directory layouts */
    enum class EDirectoryLayout {
      /** Flat directory containing the payloads in the form
       * `dbstore_{NAME}_rev_{REVISION}.root` */
      c_flat,
      /** Hashed directory structure containing the payloads in the form
       * `AB/{NAME}_r{REVISION}.root` where A and B are the first to characters
       * of the md5 checksum of the payload file */
      c_hashed,
    };

    /** Constructor for a given list of locations and optionally the location
     * where downloaded payloads should be cached Each location can either be a
     * path to a directory or a http/https url of a server where the payloads
     * can be downloaded (starting with `http(s)://`).
     *
     * For remote locations starting with http(s) we require that the payloads
     * follow the same layout as on the central server. That is the location
     * plus the value returned by the metadata for `payloadUrl` should point to
     * the correct file.
     *
     * For local locations we support two different layouts which will be auto
     * detected and can be mixed:
     *
     * - "flat": All payloads in one directory named `dbstore_{NAME}_rev_{REVISION}.root`
     * - "hashed": Payloads in subdirectories named `/AB/{NAME}_r{REVISION}.root`
     *   where A and B are the first two characters of the md5 checksum of the
     *   payload file.
     *
     * If cachedir is empty a default value of `$TMPDIR/basf2-conditions`
     * is assumed. Downloaded payloads will be placed in the cachedir using the
     * hashed directory structure.
     */
    explicit PayloadProvider(const std::vector<std::string>& locations, const std::string& cachedir = "", int timeout = 60);

    /** Try to find a payload, return true on success, false if it cannot be
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
    std::string getFilename(EDirectoryLayout structure, const PayloadMetadata& payload) const;
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
