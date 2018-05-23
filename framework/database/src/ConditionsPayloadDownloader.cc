/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/ConditionsPayloadDownloader.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/Utils.h>
#include <framework/utilities/FileSystem.h>
#include <TRandom.h>
#include <iomanip>
#include <set>
#include <chrono>
#include <thread>

#include <curl/curl.h>
#include <cstring>

#include <TMD5.h>
//#include <openssl/evp.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem;

namespace Belle2 {
  /** struct encapsulating all the state information needed by curl */
  struct ConditionsCurlSession {
    /** curl session information */
    CURL* curl{nullptr};
    /** headers to send with every request */
    curl_slist* headers{nullptr};
    /** error buffer in case some error happens during downloading */
    char errbuf[CURL_ERROR_SIZE];
    /** last time we printed the status (in ns) */
    double lasttime{0};
  };

  /** Timeout to wait for connections in seconds */
  unsigned int ConditionsPayloadDownloader::s_connectionTimeout{60};
  /** Timeout to wait for stalled connections (<10KB/s) */
  unsigned int ConditionsPayloadDownloader::s_stalledTimeout{60};
  /** Number of retries to perform when downloading failes with HTTP response code >=500 */
  unsigned int ConditionsPayloadDownloader::s_maxRetries{3};
  /** Backoff factor for retries in seconds */
  unsigned int ConditionsPayloadDownloader::s_backoffFactor{5};
}

namespace {
  /** Callback to handle the bytes downloaded by curl.
   * See https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
   *
   * @param buffer pointer to the memory buffer of downloaded bytes
   * @param size number of elements in buffer
   * @param nmemb number of bytes per element in the buffer
   * @param userp pointer to the std::ostream instance we handed to curl
   * @returns the amount of bytes handled by this function which needs to be
   *          size*nmemb otherwise curl will abort
   */
  size_t write_function(void* buffer, size_t size, size_t nmemb, void* userp)
  {
    std::ostream& stream = *static_cast<std::ostream*>(userp);
    // size in bytes is size*nmemb so copy the correct amount and return it to curl
    try {
      stream.write(static_cast<const char*>(buffer), size * nmemb);
    } catch (std::ios_base::failure& e) {
      B2ERROR("Writing error while downloading: " << e.what());
      return 0;
    }
    return size * nmemb;
  }

  /** Callback to show download progress. This function is called by curl when downloading things
   * See https://curl.haxx.se/libcurl/c/CURLOPT_XFERINFOFUNCTION.html
   *
   * @param clientp pointer to a progress_status struct we handed to curl
   * @param dltotal total number of bytes to download, can be 0 if unknown
   * @param dlnow number of bytes already downloaded
   * @param ultotal total number of bytes to upload, can be 0 if unknown
   * @param ulnow number of bytes already uploaded
   * @returns 0 otherwise curl aborts the download
   */
  int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                        __attribute((unused)) curl_off_t ultotal, __attribute((unused)) curl_off_t ulnow)
  {
    // nothing to show ...
    if (dlnow == 0) return 0;
    // otherwise print number of transferred bytes
    Belle2::ConditionsCurlSession& status = *static_cast<Belle2::ConditionsCurlSession*>(clientp);
    double time = Belle2::Utils::getClock();
    // make sure we don't print the status too often
    if (status.lasttime != 0 && (time - status.lasttime) / Belle2::Unit::ms < 200) {
      return 0;
    }
    status.lasttime = time;
    if (dltotal > 0) {
      B2DEBUG(300, "curl:= " << dlnow << " / " << dltotal << " bytes transferred");
    } else {
      B2DEBUG(300, "curl:= " << dlnow << " bytes transferred");
    }
    return 0;
  }

  /** callback function to show debug output when downloading things.
   * See https://curl.haxx.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html
   *
   * @param handle pointer to the curl session
   * @param type type of the data
   * @param data buffer to the data
   * @param size size of the data buffer
   * @param userptr optional additional information passed along by curl
   * @returns 0
   */
  int debug_callback(__attribute((unused)) CURL* handle, curl_infotype type, char* data, size_t size,
                     __attribute((unused)) void* userptr)
  {
    std::string prefix = "curl:";
    int level = 500;
    if (type == CURLINFO_TEXT) { prefix += "*"; level = 200; }
    else if (type == CURLINFO_HEADER_OUT) prefix += ">";
    else if (type == CURLINFO_HEADER_IN) prefix += "<";
    else return 0;
    std::string message(data, size);
    boost::trim(message);
    if (!message.empty()) B2DEBUG(level, prefix << " " <<  message);
    return 0;
  }

  /** Join two strings and make sure that there is exactly one '/' between them */
  std::string urljoin(const std::string& base, const std::string& rest)
  {
    return boost::trim_right_copy_if(base, boost::is_any_of("/")) + "/" + boost::trim_left_copy_if(rest, boost::is_any_of("/"));
  }

  /** Small helper to get a value from environment or fall back to a default
   * @param envName name of the environment variable to look for
   * @param fallback value to return in case environment variable is not set
   * @return whitespace trimmed value of the environment variable if set, otherwise the fallback value
   */
  std::string getFromEnvironment(const std::string& envName, const std::string& fallback)
  {
    char* envValue = std::getenv(envName.c_str());
    if (envValue != nullptr) {
      std::string val(envValue);
      boost::trim(val);
      return envValue;
    }
    return fallback;
  }

  /** Return a string with the software version to send as software version */
  std::string getUserAgent()
  {
    return "BASF2/" + getFromEnvironment("BELLE2_RELEASE", "unknown");
  }
}

namespace Belle2 {
  /* We only want to initialize curl once */
  bool ConditionsPayloadDownloader::s_globalInit{false};

  /* add directory but make sure it's absolute */
  void ConditionsPayloadDownloader::addLocalDirectory(const std::string& directory, EConditionsDirectoryStructure structure)
  {
    m_localDirectories.emplace_back(fs::absolute(fs::path(directory)).string(), structure);
  }

  bool ConditionsPayloadDownloader::startSession()
  {
    // start a curl session but if there is already one return false
    if (m_session) return false;
    // make sure curl is initialized correctly
    if (!s_globalInit) {
      curl_global_init(CURL_GLOBAL_ALL);
      s_globalInit = true;
    }
    // create the curl session
    m_session.reset(new ConditionsCurlSession);
    m_session->curl = curl_easy_init();
    if (!m_session->curl) {
      B2FATAL("Cannot intialize libcurl");
    }
    m_session->headers = curl_slist_append(nullptr, "Accept: application/json");
    curl_easy_setopt(m_session->curl, CURLOPT_HTTPHEADER, m_session->headers);
    curl_easy_setopt(m_session->curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(m_session->curl, CURLOPT_CONNECTTIMEOUT, s_connectionTimeout);
    curl_easy_setopt(m_session->curl, CURLOPT_LOW_SPEED_LIMIT, 10 * 1024); //10 kB/s
    curl_easy_setopt(m_session->curl, CURLOPT_LOW_SPEED_TIME, s_stalledTimeout);
    curl_easy_setopt(m_session->curl, CURLOPT_WRITEFUNCTION, write_function);
    curl_easy_setopt(m_session->curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(m_session->curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(m_session->curl, CURLOPT_DEBUGFUNCTION, debug_callback);
    curl_easy_setopt(m_session->curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(m_session->curl, CURLOPT_XFERINFODATA, m_session.get());
    curl_easy_setopt(m_session->curl, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(m_session->curl, CURLOPT_ERRORBUFFER, m_session->errbuf);
    // enable transparent compression support
    curl_easy_setopt(m_session->curl, CURLOPT_ACCEPT_ENCODING, "");
    // Set proxy if defined
    const char* proxy = std::getenv("BELLE2_CONDB_PROXY");
    if (proxy) curl_easy_setopt(m_session->curl, CURLOPT_PROXY, proxy);
    curl_easy_setopt(m_session->curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(m_session->curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_session->curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(m_session->curl, CURLOPT_TCP_FASTOPEN, 0L);
    curl_easy_setopt(m_session->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_session->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_session->curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
    auto version = getUserAgent();
    curl_easy_setopt(m_session->curl, CURLOPT_USERAGENT, version.c_str());
    return true;
  }

  void ConditionsPayloadDownloader::finishSession()
  {
    // if there's a session clean it ...
    if (m_session) {
      curl_easy_cleanup(m_session->curl);
      curl_slist_free_all(m_session->headers);
      m_session.reset();
    }
  }

  ConditionsPayloadDownloader::ConditionsPayloadDownloader(
    const std::string& localDir,  const std::string& restUrl, int timeout):
    m_restUrl(restUrl), m_timeout(timeout)
  {
    addLocalDirectory(localDir, EConditionsDirectoryStructure::c_flatDirectory);
    // Allow overriding with an environment variable where the servers are
    // given separated by space
    std::string serverList = getFromEnvironment("BELLE2_CONDB_SERVERLIST", "");
    if (restUrl.empty() && !serverList.empty()) {
      boost::split(m_serverList, serverList, boost::is_any_of(" \t\n\r"));
      B2INFO("Setting Conditions Database servers from Environment:");
      int i{0};
      for (const auto& s : m_serverList) B2INFO("  " << ++i << ". " << s);
    }
  }

  ConditionsPayloadDownloader::~ConditionsPayloadDownloader()
  {
    finishSession();
  }

  bool ConditionsPayloadDownloader::update(const std::string& globalTag, int experiment, int run)
  {
    //make sure we have an active curl session needed for escaping the tag name ...
    SessionGuard session(*this);

    // update all the payload info from the central database
    // so first clear existing information
    m_payloads.clear();
    // build up the request url
    // make sure the tag name doesn't contain special characters
    char* escapedTag = curl_easy_escape(m_session->curl, globalTag.c_str(), globalTag.size());
    if (!escapedTag) {
      B2ERROR("Could not encode the global tag name: " << globalTag);
      return false;
    }
    std::string escapedTagStr = std::string(escapedTag);
    curl_free(escapedTag);
    std::string restUrl = m_restUrl;
    if (m_restUrl.empty()) {
      if (m_serverList.empty()) {
        B2WARNING("No Working Database Server configured, giving up");
        return false;
      }
      restUrl = m_serverList.front();
      m_serverList.erase(m_serverList.begin());
    }
    const std::string url = urljoin(restUrl, "v2/iovPayloads/?gtName=" + escapedTagStr + "&expNumber=" +
                                    std::to_string(experiment) + "&runNumber=" + std::to_string(run));
    // and perform the request
    std::stringstream payloads;
    if (!download(url, payloads)) {
      // Download of payload information failed, try somewhere else if we don't
      // have a custom URL. We do this recursively to keep code changes
      // minimal. As we always pop an element this will finish eventually and
      // the amount of servers should never be too big.
      if (m_restUrl.empty()) {
        B2WARNING("Failed to obtain payload information from " << restUrl << ", trying next server");
        return update(globalTag, experiment, run);
      }
      // but for custom url we behave as before
      B2WARNING("Could not get list of payloads from database");
      return false;
    } else if (m_restUrl.empty()) {
      // found a working server, let's stick to this one
      m_restUrl = restUrl;
    }
    // and if that was successful we parse the returned json by resetting the stringstream to its beginning
    payloads.clear();
    payloads.seekg(0, std::ios::beg);
    boost::property_tree::ptree pt;
    std::set<std::string> duplicates;
    try {
      // and load it into a boost property_tree
      boost::property_tree::read_json(payloads, pt);

      // and loop over all elements and extract the necessary information
      for (auto& iov : pt.get_child("")) {
        PayloadInfo payloadInfo;
        auto payload = iov.second.get_child("payload");
        auto payloadIov = iov.second.get_child("payloadIov");
        B2DEBUG(500, "Parsing payload with id " << payload.get("payloadId", ""));
        std::string name = payload.get<std::string>("basf2Module.name");
        payloadInfo.digest = payload.get<std::string>("checksum");
        payloadInfo.payloadUrl = payload.get<std::string>("payloadUrl");
        payloadInfo.revision = payload.get<int>("revision", 0);
        payloadInfo.baseUrl = payload.get<std::string>("baseUrl");
        int firstExp = payloadIov.get<int>("expStart");
        int firstRun = payloadIov.get<int>("runStart");
        int finalExp = payloadIov.get<int>("expEnd");
        int finalRun = payloadIov.get<int>("runEnd");
        payloadInfo.iov = IntervalOfValidity(firstExp, firstRun, finalExp, finalRun);

        // make sure we have all fields filled
        if (name.empty() || payloadInfo.payloadUrl.empty() || payloadInfo.digest.empty() || payloadInfo.baseUrl.empty()) {
          B2WARNING("ConditionsDB Payload not parsed correctly: empty name, url or checksum");
        } else {
          // and update or replace the payloadinfo in the map
          auto payloadIter = m_payloads.find(name);
          if (payloadIter != m_payloads.end()) {
            int keep = std::max(payloadIter->second.revision, payloadInfo.revision);
            int drop = std::min(payloadIter->second.revision, payloadInfo.revision);
            if (payloadIter->second.revision < payloadInfo.revision) {
              payloadIter->second = payloadInfo;
            }
            B2DEBUG(200, "Found duplicate payload key " << name << " while parsing conditions payloads. "
                    "Discarding revision " << drop << " and using revision " << keep);
            duplicates.insert(name);
          } else {
            B2DEBUG(200, "Found payload '" << name << "' at " << payloadInfo.payloadUrl << " and checksum "
                    << payloadInfo.digest << ". iov: " << payloadInfo.iov);
            m_payloads[name] = payloadInfo;
          }
        }
      }
    } catch (boost::property_tree::ptree_error& e) {
      // Any problems parsing the reply? clean up and bail
      B2WARNING("Cannot parse database payload information: " << e.what());
      m_payloads.clear();
      return false;
    }
    // report on duplicate payloads because that should not be ...
    // FIXME: commented out until we can delete iovs ...
    //if (!duplicates.empty()) {
    //  B2INFO("Found more then one payload for the following keys: " << boost::algorithm::join(duplicates, ", "));
    //}
    return true;
  }

  const ConditionsPayloadDownloader::PayloadInfo& ConditionsPayloadDownloader::get(const std::string& name)
  {
    // assume the payload exists, if not an exception is thrown which we let bubble up
    PayloadInfo& info = m_payloads.at(name);
    if (info.filename.empty()) {
      // and download the filename if necessary
      info.filename = getPayloadFile(info);
    }
    // download failed, raise an error
    if (info.filename.empty()) {
      B2ERROR("Failure obtaining payload file for entry " << std::quoted(name) << " from central database");
    }
    // otherwise return filename and iov
    return info;
  }

  std::string ConditionsPayloadDownloader::calculateDigest(std::istream& input)
  {
    /*** openssl 1.1 version ...
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5());
    B2INFO(EVP_MD_block_size(EVP_md5()))
      char buffer[4096];
    while(input.good()){
      input.read(buffer, 4096);
      if(input.gcount()==0) break;
      EVP_DigestUpdate(&ctx, buffer, input.gcount());
    }

    EVP_DigestFinal_ex((unsigned char*)buffer, &md5);
    std::stringstream digest;
    // this does not work yet, need exactly 2 hex chars
    for(size_t i=0; i<MD5_DIGEST_LENGTH; ++i){
      digest << std::hex << (int)buffer[i];
    }

    return digest.str();
    ***/

    // rewind stream
    input.clear();
    input.seekg(0, std::ios::beg);
    // and calculate md5 checksum by feeding it blockwise to the TMD5 update
    TMD5 md5;
    char buffer[4096];
    while (input.good()) {
      input.read(buffer, 4096);
      if (input.gcount() == 0) break;
      md5.Update((unsigned char*)buffer, input.gcount());
    }
    // finalize and return output
    md5.Final();
    return md5.AsString();
  }

  std::string ConditionsPayloadDownloader::getLocalName(const std::string& dir, EConditionsDirectoryStructure structure,
                                                        const PayloadInfo& payload)
  {
    fs::path path(dir);
    switch (structure) {
      case  EConditionsDirectoryStructure::c_logicalSubdirectories:
        path /= fs::path(payload.payloadUrl);
        break;
      case EConditionsDirectoryStructure::c_digestSubdirectories:
        path /= payload.digest.substr(0, 1);
        path /= payload.digest.substr(1, 2);
        // intentional fall through to get flat name in addition ...
        [[fallthrough]];
      case EConditionsDirectoryStructure::c_flatDirectory:
        path /= fs::path(payload.payloadUrl).filename();
        break;
    };
    return path.lexically_normal().string();
  }

  std::string ConditionsPayloadDownloader::getPayloadFile(const PayloadInfo& payload)
  {
    //check if we already have downloaded this payload before into a temporary file
    auto it = m_tempfiles.find(payload.payloadUrl);
    if (it != end(m_tempfiles)) {
      return it->second->getName();
    }
    //ok look in all local directories
    for (const auto& dir : m_localDirectories) {
      const std::string localFile = getLocalName(dir.first, dir.second, payload);
      if (fs::exists(localFile)) {
        B2DEBUG(200, "Checking checksum for " << localFile);
        std::ifstream localStream(localFile.c_str(), std::ios::binary);
        if (checkDigest(localStream, payload.digest)) {
          B2DEBUG(200, "found matching payload: " << localFile);
          return localFile;
        }
        B2DEBUG(200, "Check failed, need to download, continue with next");
      }
    }
    // ok all local directories failed, try to download the file into the first
    // local directory
    const auto& dir = m_localDirectories[0];
    const std::string url = urljoin(payload.baseUrl, payload.payloadUrl);
    const std::string localFile = getLocalName(dir.first, dir.second, payload);
    // now we need to make the directories to the file
    try {
      fs::create_directories(fs::path(localFile).parent_path());
    } catch (fs::filesystem_error& e) {
      B2WARNING("Cannot create local payload directory " << fs::path(localFile).parent_path());
      return getTemporary(payload.payloadUrl, url, payload.digest);
    }

    // ok, directory exists, now we need a write lock on the file to avoid race conditions
    Belle2::FileSystem::Lock writelock(localFile);
    B2DEBUG(200, "Attempting to lock payload file " << localFile << " for writing ...");
    // if we cannot get one the folder/file might be write protected or
    // download by another process takes to long.  So let's download into
    // temporary file.
    if (!writelock.lock(m_timeout, true)) return getTemporary(payload.payloadUrl, url, payload.digest);
    // Ok we have the write lock, check if we can open the file for writing
    std::fstream localStream(localFile.c_str(), std::ios::binary | std::ios::in | std::ios::out);
    B2DEBUG(200, "Got write lock, check for file access ...");
    if (!localStream.good()) {
      B2WARNING("Cannot open " << std::quoted(localFile) << " for writing");
    }
    // File is open. Someone might have downloaded the file
    // while we waited, check md5sum again.
    B2DEBUG(200, "Ok, check digest in case another process downloaded already...");
    if (checkDigest(localStream, payload.digest)) return localFile;
    // we have lock and it's broken so download the file
    B2DEBUG(200, "Still not good, download now ...");
    if (!downloadAndCheck(url, localStream, payload.digest)) {
      B2INFO("Download for payload " <<  std::quoted(payload.payloadUrl) << " failed, trying one last time into temporary file");
      return getTemporary(payload.payloadUrl, url, payload.digest);
    }
    B2DEBUG(200, "Download of payload successful");
    return localFile;
  }

  bool ConditionsPayloadDownloader::download(const std::string& url, std::ostream& buffer)
  {
    //make sure we have an active curl session ...
    SessionGuard session(*this);
    B2DEBUG(200, "Download of " << url << " started ...");
    // we might need to try a few times in case of HTTP>=500
    for (unsigned int retry{1};; ++retry) {
      //rewind the stream to the beginning
      buffer.clear();
      buffer.seekp(0, std::ios::beg);
      if (!buffer.good()) {
        B2ERROR("Cannot write to stream when downloading " << url);
        return false;
      }
      // Set the exception flags to notify us of any problem during writing
      auto oldExceptionMask = buffer.exceptions();
      buffer.exceptions(std::ios::failbit | std::ios::badbit);
      // build the request ...
      CURLcode res{CURLE_FAILED_INIT};
      // and set all the curl options
      curl_easy_setopt(m_session->curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(m_session->curl, CURLOPT_WRITEDATA, &buffer);
      // perform the request ...
      res = curl_easy_perform(m_session->curl);
      // flush output
      buffer.exceptions(oldExceptionMask);
      buffer.flush();
      // and check for errors which occured during download ...
      if (res != CURLE_OK) {
        size_t len = strlen(m_session->errbuf);
        if (len) {
          B2WARNING("Could not get " << std::quoted(url) << ": " << m_session->errbuf);
        } else {
          B2WARNING("Could not get " << std::quoted(url) << ": " << curl_easy_strerror(res));
        }
        if (s_maxRetries > 0) {
          if (retry > s_maxRetries) {
            B2WARNING("Failed " << retry << " times, giving up");
          } else if (res == CURLE_HTTP_RETURNED_ERROR) {
            // we treat everything below 500 as permanent error with the request,
            // only retry on 500.
            long responseCode{0};
            curl_easy_getinfo(m_session->curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if (responseCode >= 500) {
              // use exponential backoff but don't restrict to exact slots like
              // Ethernet, just use a random wait time between 1s and maxDelay =
              // 2^(retry)-1 * backoffFactor
              double maxDelay = (std::pow(2, retry) - 1) * s_backoffFactor;
              double seconds = gRandom->Uniform(1., maxDelay);
              B2INFO("Waiting " << std::setprecision(2) << seconds << " seconds before retrying download ...");
              std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1e3)));
              continue;
            }
          }
        }
        return false;
      }
      break;
    }
    // all fine
    B2DEBUG(200, "Download of " << std::quoted(url) << " finished succesfully.");
    return true;
  }

  bool ConditionsPayloadDownloader::downloadAndCheck(const std::string& url, std::iostream& stream, const std::string& digest)
  {
    if (!download(url, stream)) return false;
    const std::string actual = calculateDigest(stream);
    if (actual != digest) {
      B2WARNING("Checksum mismatch for " << std::quoted(url) << ": " << "should be " <<  digest << " but found " << actual);
      return false;
    }
    return true;
  }

  std::string ConditionsPayloadDownloader::getTemporary(const std::string& key, const std::string& url, const std::string& digest)
  {
    const auto openmode = std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
    std::unique_ptr<FileSystem::TemporaryFile> tmpfile(new FileSystem::TemporaryFile(openmode));
    B2DEBUG(200, "Trying to download into temporary file " << tmpfile->getName());
    if (downloadAndCheck(url, *tmpfile, digest)) {
      m_tempfiles[key] = std::move(tmpfile);
      return m_tempfiles[key]->getName();
    }
    return "";
  }
}
