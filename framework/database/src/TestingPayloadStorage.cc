/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/TestingPayloadStorage.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/utilities/ScopeGuard.h>

#include <TDirectory.h>
#include <TFile.h>
#include <TObject.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem;

namespace Belle2::Conditions {
  TestingPayloadStorage::TestingPayloadStorage(const std::string& filename):
    m_filename{filename}, m_absoluteFilename{fs::absolute(m_filename).string()},
    m_payloadDir{fs::path(m_absoluteFilename).parent_path().string()}
  {}

  bool TestingPayloadStorage::get(const EventMetaData& event, PayloadMetadata& info)
  {
    // Read database file on first access
    if (!m_initialized) read();
    m_initialized = true;
    // And then let's look for a payload with the name
    auto it = m_payloads.find(info.name);
    if (it == m_payloads.end()) return false;
    bool found{false};
    // and check all of them for what is the highest revision containing the event
    for (const auto& [revision, iov] : it->second) {
      if (iov.contains(event)) {
        info.revision = 0;
        info.globaltag = "temp://" + m_absoluteFilename;
        info.baseUrl = "";
        info.payloadUrl = "";
        info.filename = payloadFilename(m_payloadDir, info.name, revision);
        info.iov = iov;
        if (!FileSystem::fileExists(info.filename)) {
          B2FATAL("Could not find payload file specified in testing payload storage" << LogVar("storage filen", m_filename)
                  << LogVar("name", info.name) << LogVar("local revision", info.revision)
                  << LogVar("payload filename", info.filename));
        }
        info.checksum = FileSystem::calculateMD5(info.filename);
        found = true;
        break;
      }
    }
    if (found) {
      B2WARNING("Conditions: Temporary testing payload used for processing" << LogVar("storage file", m_filename)
                << LogVar("name", info.name) << LogVar("revision", info.revision) << LogVar("iov", info.iov));
    }
    // Did we find something? did we?
    return found;
  }

  void TestingPayloadStorage::read()
  {
    m_payloads.clear();
    if (!fs::is_regular_file(m_absoluteFilename)) {
      B2WARNING("Given testing payload storage file doesn't exist or is not a regular file" << LogVar("storage file", m_filename));
      return;
    }
    // read and parse the database content
    std::ifstream file(m_absoluteFilename.c_str());
    if (!file.is_open()) {
      B2FATAL("Opening of testing payload storage file failed" << LogVar("storage file", m_filename) << LogVar("error", strerror(errno)));
    }
    int lineno{0};
    try {
      while (!file.eof()) {
        // read the file line by line
        std::string line;
        std::getline(file, line);
        ++lineno;
        // and remove comments from the line
        size_t commentChar = line.find('#');
        if (commentChar != std::string::npos) {
          line = line.substr(0, commentChar);
        }
        // trim whitespace on each side
        boost::algorithm::trim(line);
        // if nothing is left skip the line
        if (line.empty()) continue;
        // otherwise read name, revision and iov from the line
        std::string name;
        std::string revision;
        IntervalOfValidity iov;
        try {
          std::stringstream(line) >> name >> revision >> iov;
        } catch (std::runtime_error& e) {
          throw std::runtime_error("line must be of the form 'dbstore/<payloadname> <revision> <firstExp>,<firstRun>,<finalExp>,<finalRun>'");
        }
        // parse name
        size_t pos = name.find('/');
        if (pos == std::string::npos) {
          throw std::runtime_error("payload name must be of the form dbstore/<payloadname>");
        }
        std::string module = name.substr(pos + 1, name.length());
        // and add to map of payloads
        B2DEBUG(39, "Found testing payload" << LogVar("storage file", m_filename) << LogVar("name", module)
                << LogVar("revision/md5", revision) << LogVar("iov", iov));
        m_payloads[module].emplace_back(revision, iov);
      }
    } catch (std::exception& e) {
      B2FATAL("Problem reading testing payloads storage" << LogVar("storage file", m_filename)
              << LogVar("line", lineno) << LogVar("error", e.what()));
    }
    // and reverse all the payloads so the last ones in the file have highest priority
    for (auto& [name, payloads] : m_payloads) {
      std::reverse(payloads.begin(), payloads.end());
    }
  }

  std::string TestingPayloadStorage::payloadFilename(const std::string& path, const std::string& name,
                                                     const std::string& revision)
  {
    std::stringstream result;
    if (!path.empty()) result << path << '/';
    result << "dbstore_" << name << "_rev_" << revision << ".root";
    return result.str();
  }

  bool TestingPayloadStorage::storeData(const std::string& name, TObject* object, const IntervalOfValidity& iov)
  {
    return store(name, iov, "", [this, &object, &name](const std::string & filename) {
      return writePayload(filename, name, object);
    });
  }

  bool TestingPayloadStorage::storePayload(const std::string& name, const std::string& fileName, const IntervalOfValidity& iov)
  {
    // resolve all symbolic links to make sure we point to the real file
    fs::path resolved = fs::canonical(fileName);
    if (not fs::is_regular_file(resolved)) {
      B2ERROR("Problem creating testing payload: Given payload storage file doesn't exist" << LogVar("storage file", fileName));
      return false;
    }
    return store(name, iov, resolved.string(), [&resolved](const std::string & destination) {
      // copy payload file to payload directory and rename it to follow the file name convention
      fs::copy_file(resolved, destination, fs::copy_option::overwrite_if_exists);
      return true;
    });
  }

  bool TestingPayloadStorage::store(const std::string& name, const IntervalOfValidity& iov,
                                    const std::string& source, const std::function<bool(const std::string&)>& writer)
  {
    if (iov.empty()) {
      B2ERROR("IoV is empty, refusing to store object in testing payload storage"
              "Please provide a valid experiment/run range for the data, for example "
              "using IntervalOfValidity::always() to store data which is always valid"
              << LogVar("name", name));
      return false;
    }

    if (!fs::exists(m_payloadDir)) {
      fs::create_directories(m_payloadDir);
    }

    // create a temporary file if we don't have a source file yet
    fs::path sourcefile{source};
    if (source.empty()) {
      while (true) {
        sourcefile = fs::path(m_payloadDir) / fs::unique_path();
        auto fd = open(sourcefile.c_str(), O_CREAT | O_EXCL);
        if (fd >= 0) {
          close(fd);
          break;
        }
        if (errno != EEXIST && errno != EINTR) {
          B2ERROR("Cannot create payload file:" << strerror(errno));
          return false;
        }
        B2DEBUG(35, "first try to create tempfile failed, trying again");
      }
      if (!writer(sourcefile.string())) return false;
    }
    // If we created a temporary file we want to delete it again so we'd like to
    // use a scope guard to do so. However we need it in this scope so we need
    // to create one in any case and release it if we didn't create a temporary
    // file
    ScopeGuard delete_srcfile([&sourcefile] {fs::remove(sourcefile);});
    if (!source.empty()) delete_srcfile.release();

    std::string md5 = FileSystem::calculateMD5(sourcefile.string());

    // Ok, now we have the file and it's md5 sum so let's get a write lock to the database file
    // to avoid race conditions when creating files and writing the info in the text file.
    FileSystem::Lock lock(m_absoluteFilename);
    if (!lock.lock()) {
      B2ERROR("Locking of testing payload storage file failed, cannot create payload"
              << LogVar("storage file", m_filename));
      return false;
    }
    std::ofstream file(m_absoluteFilename.c_str(), std::ios::app);
    if (!file.is_open()) {
      B2ERROR("Could not open testing payload storage file for writing" << LogVar("storage file", m_filename));
    }

    // So let's try renaming our temporary payload file to final destination
    // We start with a 5 digit hash and expand if there's a collision
    std::string revision;
    bool found = false;
    for (int i = 6; i <= 32; ++i) {
      revision = md5.substr(0, i);
      auto filename = payloadFilename(m_payloadDir, name, revision);
      if (FileSystem::fileExists(filename)) {
        if (md5 != FileSystem::calculateMD5(filename)) continue;
      } else {
        if (source.empty()) {
          fs::rename(sourcefile, filename);
          delete_srcfile.release();
        } else {
          fs::copy_file(source, filename, fs::copy_option::overwrite_if_exists);
        }
      }
      found = true;
      break;
    }
    if (!found) {
      B2ERROR("Cannot create payload file: checksum mistmatch for existing files");
      return false;
    }
    // Ok, add to the text file
    file << "dbstore/" << name << " " << revision << " " << iov << std::endl;
    B2DEBUG(32, "Storing testing payload" << LogVar("storage file", m_filename) << LogVar("name", name)
            << LogVar("local revision", revision) << LogVar("iov", iov));
    // And make sure we reread the file on next request to payloads
    m_initialized = false;
    return true;
  }

  bool TestingPayloadStorage::writePayload(const std::string& fileName, const std::string& name, const TObject* object)
  {
    // Save the current gDirectory
    TDirectory::TContext saveDir;
    // And create a reproducible TFile: one that has the same checksum every time it's created as long as the content is the same
    std::unique_ptr<TFile> file{TFile::Open((fileName + "?reproducible=PayloadFile").c_str(), "RECREATE")};
    if (!file || !file->IsOpen()) {
      B2ERROR("Could not open payload file for writing." << LogVar("filename", m_filename));
      return false;
    }
    // Write the payload
    object->Write(name.c_str(), TObject::kSingleKey);
    // Done, let's go
    file->Close();
    return true;
  }

} // Belle2::Conditions namespace
