/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/PayloadProvider.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>

namespace fs = boost::filesystem;

namespace Belle2::Conditions {
  PayloadProvider::PayloadProvider(const std::vector<std::string>& locations, const std::string& cacheDir, int timeout): m_timeout{timeout}
  {
    // check whether we have a chache directory ... otherwise use default
    if (!cacheDir.empty()) {
      m_cacheDir = {fs::absolute(cacheDir).string(), false};
    } else {
      m_cacheDir = {fs::absolute(fs::temp_directory_path() / "basf2-conditions").string(), false};
    }
    m_locations.reserve(locations.size() + 2); //cache location + all configured + central server
    // always look in cache directory first
    m_locations.emplace_back(m_cacheDir);
    B2DEBUG(33, "Added payload cache location" << LogVar("path", m_cacheDir.base));
    // and then in the other directories as specified
    for (auto path : locations) {
      boost::algorithm::trim(path);
      if (path.empty()) {
        B2FATAL("Found empty payload location in configuration. "
                "Please make sure that the conditions database settings are correct");
      }
      bool remote = false;
      if (auto pos = path.find("://"); pos != std::string::npos) {
        //found a protocol: if file remove, otherwise keep as is and set as remote ...
        auto protocol = path.substr(0, pos);
        boost::algorithm::to_lower(protocol);
        if (protocol == "file") {
          path = path.substr(pos + 3);
        } else if (protocol == "http" or protocol == "https") {
          remote = true;
        } else {
          B2ERROR("Unknown protocol, only supported protocols for payload download are file, http, https" << LogVar("protocol", protocol));
          continue;
        }
      }
      // Also make sure files are absolute
      if (!remote) path = fs::absolute(path).string();
      // And then add it to the list
      B2DEBUG(33, "Added payload search location" << LogVar(remote ? "url" : "path", path));
      m_locations.emplace_back(PayloadLocation{path, remote});
    }
    // and as as last resort always go to the central server
    m_locations.emplace_back(PayloadLocation{"", true});
  }

  bool PayloadProvider::find(PayloadMetadata& metadata)
  {
    // Check all locations for the file ... but dispatch to the correct member function
    return std::any_of(m_locations.begin(), m_locations.end(), [this, &metadata](const auto & loc) {
      return loc.isRemote ? getRemoteFile(loc, metadata) : getLocalFile(loc, metadata);
    });
  }

  bool PayloadProvider::getLocalFile(const PayloadLocation& loc, PayloadMetadata& metadata) const
  {
    // look in both flat and hashed directory structures.
    for (EDirectoryLayout structure : {EDirectoryLayout::c_hashed, EDirectoryLayout::c_flat}) {
      auto fullPath = fs::path(loc.base) / getFilename(structure, metadata);
      // No such file? nothing to do
      if (!fs::exists(fullPath)) continue;
      // Otherwise check the md5
      B2DEBUG(36, "Checking checksum for payload file" << LogVar("name", metadata.name) << LogVar("local dir", loc.base)
              << LogVar("revision", metadata.revision) << LogVar("filename", fullPath) << LogVar("checksum", metadata.checksum));
      const auto actual = FileSystem::calculateMD5(fullPath.string());
      if (actual == metadata.checksum) {
        metadata.filename = fullPath.string();
        B2DEBUG(37, "Found matching payload file");
        return true;
      }
      B2DEBUG(37, "Checksum doesn't match, continue with next");
    }
    return false;
  }

  bool PayloadProvider::getRemoteFile(const PayloadLocation& loc, PayloadMetadata& metadata)
  {
    // we want to download payloads in a hashed directory structure to keep amount of payloads per directory to a managable level
    const auto local = fs::path(m_cacheDir.base) / getFilename(EDirectoryLayout::c_hashed, metadata);
    // empty location: use the central server supplied baseUrl from payload metadata
    const bool fallback = loc.base.empty();
    const auto base = fallback ? metadata.baseUrl : loc.base;
    // but we assume that on servers we have logical directory structure: value of payloadUrl is taken without modification
    const auto url = m_downloader.joinWithSlash(base, metadata.payloadUrl);
    // If anything fails we might want to go to temporary file and this happens at multiple places so lets use exception for that
    try {
      // now we need to make the directories to the file
      try {
        // Make sure that we create directories writable for all users
        auto oldUmask = umask(0);
        ScopeGuard umaskGuard([oldUmask] {umask(oldUmask);});
        fs::create_directories(local.parent_path());
      } catch (fs::filesystem_error& e) {
        B2WARNING("Cannot create local payload directory" << LogVar("directory", local.parent_path())
                  << LogVar("error", e.code().message()));
        throw; //rethrow existing exception
      }
      // ok, directory exists, now we need a write lock on the file to avoid race conditions
      Belle2::FileSystem::Lock writelock(local.string());
      B2DEBUG(37, "Attempting to lock payload file for writing ..." << LogVar("filename", local));
      // if we cannot get one the folder/file might be write protected or
      // download by another process takes to long.  So let's download into
      // temporary file.
      if (!writelock.lock(m_timeout, true)) {
        throw std::runtime_error("write lock failed");
      }
      // Ok we have the write lock, check if we can open the file for writing which should be a guaranteed success but who knows
      // so bail if that fails.
      std::fstream localStream(local.string().c_str(), std::ios::binary | std::ios::in | std::ios::out);
      B2DEBUG(37, "Got write lock, check for file access ...");
      if (!localStream.good()) {
        B2ERROR("Cannot open file for writing" << LogVar("filename", local) << LogVar("error", strerror(errno)));
        throw std::runtime_error("cannot open file for writing????");
      }
      // and make sure it's readable for all
      boost::filesystem::permissions(local, boost::filesystem::all_all &
                                     ~(boost::filesystem::owner_exe | boost::filesystem::group_exe | boost::filesystem::others_exe));
      // File is open. Someone might have downloaded the file
      // while we waited, check md5sum again.
      B2DEBUG(37, "Ok, check digest in case another process downloaded already...");
      if (not m_downloader.verifyChecksum(localStream, metadata.checksum)) {
        // we have lock and it's broken so download the file
        B2DEBUG(37, "Still not good, download now ...");
        try {
          if (not m_downloader.download(url, localStream, not fallback)) {
            // if this returns false we have a 404: no need to try again
            B2DEBUG(37, "Payload not found ... trying next source");
            return false;
          }
          // ok, download was fine, check checksum again
          if (not m_downloader.verifyChecksum(localStream, metadata.checksum)) {
            B2WARNING("Conditions Database: checksum mismatch after download. Trying once more in a temporary file"
                      << LogVar("name", metadata.name) << LogVar("revision", metadata.revision) << LogVar("filename", local.string()));
            throw std::runtime_error("checksum mismatch");
          }
        } catch (std::exception& e) {
          B2ERROR("Conditions Database: failure downloading url" << LogVar("url", url) << LogVar("error", e.what()));
          throw; // rethrow existing exception
        }
        B2DEBUG(37, "Download of payload successful");
      }
      // found in cache or downloaded, fine
      metadata.filename = local.string();
      return true;
    } catch (std::exception&) {
      // errors are already shown, just try again
      return getTemporaryFile(url, metadata, not fallback);
    }
  }

  std::string PayloadProvider::getFilename(EDirectoryLayout structure,
                                           const PayloadMetadata& payload) const
  {
    fs::path path("");
    switch (structure) {
      case EDirectoryLayout::c_hashed:
        path /= payload.checksum.substr(0, 2);
        path /= payload.name + "_r" + std::to_string(payload.revision) + ".root";
        break;
      case EDirectoryLayout::c_flat:
        path /= "dbstore_" + payload.name + "_rev_" + std::to_string(payload.revision) + ".root";
        break;
    };
    return path.string();
  }

  bool PayloadProvider::getTemporaryFile(const std::string& url, PayloadMetadata& metadata, bool silentOnMissing)
  {
    if (auto && it = m_temporaryFiles.find(metadata.checksum); it != m_temporaryFiles.end()) {
      metadata.filename = it->second->getName();
      return true;
    }
    const auto openmode = std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
    std::unique_ptr<FileSystem::TemporaryFile> tmpfile(new FileSystem::TemporaryFile(openmode));
    B2DEBUG(37, "Trying to download into temporary file " << tmpfile->getName());
    try {
      if (not m_downloader.download(url, *tmpfile, silentOnMissing)) return false;
      if (not m_downloader.verifyChecksum(*tmpfile, metadata.checksum)) return false;
    } catch (std::exception& e) {
      B2ERROR("Conditions Database: failure downloading url" << LogVar("url", url) << LogVar("error", e.what()));
      return false;
    }
    metadata.filename = tmpfile->getName();
    m_temporaryFiles[metadata.checksum] = std::move(tmpfile);
    return true;
  }
}
