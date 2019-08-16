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
      if (iov.contains(event) and info.revision < revision) {
        info.revision = revision;
        info.globaltag = "temp://" + m_absoluteFilename;
        info.baseUrl = "";
        info.payloadUrl = "";
        info.filename = payloadFilename(m_payloadDir, info.name, info.revision);
        info.iov = iov;
        if (!FileSystem::fileExists(info.filename)) {
          B2FATAL("Could not find payload file specified in testing payload storage" << LogVar("storage filen", m_filename)
                  << LogVar("name", info.name) << LogVar("local revision", info.revision)
                  << LogVar("payload filename", info.filename));
        }
        info.checksum = FileSystem::calculateMD5(info.filename);
        found = true;
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
        std::string revisionStr;
        IntervalOfValidity iov;
        try {
          std::stringstream(line) >> name >> revisionStr >> iov;
        } catch (std::runtime_error& e) {
          throw std::runtime_error("line must be of the form 'dbstore/<payloadname> <revision> <firstExp>,<firstRun>,<finalExp>,<finalRun>'");
        }
        int revision{ -1};
        try {
          revision = stoi(revisionStr);
        } catch (std::invalid_argument& e) {
          throw std::runtime_error("revision must be an integer");
        }
        // parse name
        size_t pos = name.find('/');
        if (pos == std::string::npos) {
          throw std::runtime_error("payload name must be of the form dbstore/<payloadname>");
        }
        std::string module = name.substr(pos + 1, name.length());
        // and add to map of payloads
        B2DEBUG(39, "Found testing payload" << LogVar("storage file", m_filename) << LogVar("name", module) << LogVar("revision",
                revision) << LogVar("iov", iov));
        m_payloads[module].emplace_back(revision, iov);
      }
    } catch (std::exception& e) {
      B2FATAL("Problem reading testing payloads storage" << LogVar("storage file", m_filename)
              << LogVar("line", lineno) << LogVar("error", e.what()));
    }
  }

  std::string TestingPayloadStorage::payloadFilename(const std::string& path, const std::string& name,
                                                     int revision)
  {
    std::stringstream result;
    if (!path.empty()) result << path << '/';
    result << "dbstore_" << name << "_rev_" << revision << ".root";
    return result.str();
  }

  bool TestingPayloadStorage::storeData(const std::string& name, TObject* object, const IntervalOfValidity& iov)
  {
    return store(name, iov, [this, &object, &name](const std::string & filename) {
      return writePayload(filename, name, object);
    });
  }

  bool TestingPayloadStorage::storePayload(const std::string& name, const std::string& fileName, const IntervalOfValidity& iov)
  {
    // resolve all symbolic links to make sure we point to the real file
    boost::filesystem::path resolved = boost::filesystem::canonical(fileName);
    if (not boost::filesystem::is_regular_file(resolved)) {
      B2ERROR("Problem creating testing payload: Given payload storage file doesn't exist" << LogVar("storage file", fileName));
      return false;
    }
    return store(name, iov, [&resolved](const std::string & destination) {
      // copy payload file to payload directory and rename it to follow the file name convention
      boost::filesystem::copy(resolved, destination);
      return true;
    });
  }

  bool TestingPayloadStorage::store(const std::string& name, const IntervalOfValidity& iov,
                                    const std::function<bool(const std::string&)>& writer)
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
    // get lock for write access to database file
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

    // Find the next free revision number
    for (int revision = 1; revision < INT_MAX; ++revision) {
      auto filename = payloadFilename(m_payloadDir, name, revision);
      // FIXME: This could be a race condition, we check for existence and then
      // create which could fail if two processes try this at the same time and
      // thus overwrite the files of each other. We could instead check if an
      // `open(filename.c_str(), O_CREAT|O_EXCL)` is successful in which the
      // file is ours however I'm a bit sceptical if this will work on SL6. But
      // since we locked the database file and no longer support the payloads be
      // in a different directory then the text files this almost fine. However
      // there could still be multiple text files in the same directory so still
      // a slight chance for race conditions. Or locking could just not work on
      // some file systems, for example misconfigured NFS
      if (FileSystem::fileExists(filename)) continue;
      // free revision found, try to save
      if (!writer(filename)) return false;
      // Ok, now we need to add it to the database file
      file << "dbstore/" << name << " " << revision << " " << iov << std::endl;
      B2DEBUG(32, "Storing testing payload" << LogVar("storage file", m_filename) << LogVar("name", name)
              << LogVar("local revision", revision) << LogVar("iov", iov));
      // And make sure we reread the file on next request to payloads
      m_initialized = false;
      return true;
    }
    B2ERROR("Could not find a suitable revision to create payload" << LogVar("storage file", m_filename) << LogVar("name", name));
    return false;
  }

  bool TestingPayloadStorage::writePayload(const std::string& fileName, const std::string& name, const TObject* object)
  {
    // Save the current gDirectory
    TDirectory::TContext saveDir;
    // And create the file ...
    std::unique_ptr<TFile> file{TFile::Open(fileName.c_str(), "RECREATE")};
    if (!file || !file->IsOpen()) {
      B2ERROR("Could not open payload file for writing." << LogVar("filename", m_filename));
      return false;
    }
    // Write the payload and maybe the iov
    object->Write(name.c_str(), TObject::kSingleKey);
    // Done, let's go
    file->Close();
    return true;
  }

} // Belle2::Conditions namespace
