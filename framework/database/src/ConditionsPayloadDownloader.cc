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
#include <iostream>
#include <set>

#include <curl/curl.h>
#include <cstring>

#include <TMD5.h>
//#include <openssl/evp.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

namespace fs = boost::filesystem;

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
    stream.write(static_cast<const char*>(buffer), size * nmemb);
    if (!stream.good()) {
      B2ERROR("Writing error while downloading...");
      return 0;
    }
    return size * nmemb;
  }

  /** simple struct to contain the status we need to print progress messages */
  struct progress_status {
    /** pointer to the url we're downloading */
    const std::string* url {nullptr};
    /** last time we printed the status (in ns) */
    double lasttime{0};
  };

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
    progress_status& status = *static_cast<progress_status*>(clientp);
    double time = Belle2::Utils::getClock();
    // make sure we don't print the status too often
    if (status.lasttime != 0 && (time - status.lasttime) / Belle2::Unit::ms < 200) {
      return 0;
    }
    status.lasttime = time;
    if (dltotal > 0) {
      B2DEBUG(50, "Downloading " << *status.url << ": " << dlnow << " / " << dltotal << " bytes transferred");
    } else {
      B2DEBUG(50, "Downloading " << *status.url << ": " << dlnow << " bytes transferred");
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
    B2DEBUG(level, prefix << " " <<  message);
    return 0;
  }

  /** Simple class to make sure that the curl session is closed correctly.
   * When this class is instantiated with a downloader instance it will make
   * sure that there is an active curl session. If a session is created (i.e.
   * there was no session already) it will also end the session once this
   * object goes out of scope
   */
  class SessionGuard final {
  public:
    /** Constructor which makes sure there is an active session */
    SessionGuard(Belle2::ConditionsPayloadDownloader& instance): m_instance(instance), m_createdSession(instance.startSession()) {}
    /** disable move construction */
    SessionGuard(SessionGuard&&) = delete;
    /** disable move assignment */
    SessionGuard&   operator= (SessionGuard&&) = delete;
    /** disable copy construction */
    SessionGuard(const SessionGuard&) = delete;
    /** disable assignment */
    SessionGuard& operator= (const SessionGuard&) = delete;
    /** close session if it was created by this instance */
    ~SessionGuard() { if (m_createdSession) m_instance.finishSession(); }
  private:
    /** reference to the Downloader containing the session */
    Belle2::ConditionsPayloadDownloader& m_instance;
    /** indicates whether we created a session so we will also close it */
    bool m_createdSession;
  };
}

namespace Belle2 {
  /* We only want to initialize curl once */
  bool ConditionsPayloadDownloader::s_globalInit{false};

  bool ConditionsPayloadDownloader::startSession()
  {
    // start a curl session but if there is already one return false
    if (m_curl) return false;
    // make sure curl is initialized correctly
    if (!s_globalInit) {
      curl_global_init(CURL_GLOBAL_ALL);
      s_globalInit = true;
    }
    // create the curl session
    m_curl = curl_easy_init();
    if (!m_curl) {
      B2FATAL("Cannot intialize libcurl");
    }
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, debug_callback);
    return true;
  }

  void ConditionsPayloadDownloader::finishSession()
  {
    // if there's a session clean it ...
    if (m_curl) {
      curl_easy_cleanup(m_curl);
      m_curl = nullptr;
    }
  }

  bool ConditionsPayloadDownloader::update(const std::string& globalTag, int experiment, int run)
  {
    // update all the payload info from the central database
    // so first clear existing information
    m_payloads.clear();
    // build up the request url
    std::string experiment_str = std::to_string(experiment);
    const std::string url = m_restURL + "iovPayloads/?gtName=" + globalTag + "&expName=" + experiment_str + "&runName=" +
                            std::to_string(run);
    // and perform the request
    std::stringstream payloads;
    if (!download(url, payloads)) {
      B2WARNING("Could not get list of payloads from database");
      return false;
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
        B2DEBUG(100, "Parsing payload with id " << payload.get("payloadId", ""));
        //payloadInfo.package = payload.get<std::string>("basf2Module.basf2Package.name");
        std::string name = payload.get<std::string>("basf2Module.name");
        payloadInfo.digest = payload.get<std::string>("checksum");
        payloadInfo.url = m_baseURL + payload.get<std::string>("payloadUrl");
        int firstExp =  payloadIov.get<int>("initialRunId.experiment.name");
        int firstRun =  payloadIov.get<int>("initialRunId.name");
        int finalExp =  payloadIov.get<int>("finalRunId.experiment.name");
        int finalRun =  payloadIov.get<int>("finalRunId.name");
        payloadInfo.iov = IntervalOfValidity(firstExp, firstRun, finalExp, finalRun);
        payloadInfo.revision = payload.get<int>("revision", 0);

        // make sure we have all fields filled
        if (name.empty() || payloadInfo.url.empty() || payloadInfo.digest.empty()) {
          B2WARNING("ConditionsService::parse_payload Payload not parsed correctly: empty name, filename or checksum");
        } else {
          // and update or replace the payloadinfo in the map
          auto payloadIter = m_payloads.find(name);
          if (payloadIter != m_payloads.end()) {
            int keep = std::max(payloadIter->second.revision, payloadInfo.revision);
            int drop = std::min(payloadIter->second.revision, payloadInfo.revision);
            if (payloadIter->second.revision < payloadInfo.revision) {
              payloadIter->second = payloadInfo;
            }
            B2DEBUG(10, "Found duplicate payload key " << name << " while parsing conditions payloads. "
                    "Discarding revision " << drop << " and using revision " << keep);
            duplicates.insert(name);
          } else {
            B2DEBUG(100, "Found payload '" << name << "' at URL " << payloadInfo.url << " and checksum: " << payloadInfo.digest << ". iov: " <<
                    payloadInfo.iov);
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
    if (!duplicates.empty()) {
      B2INFO("Found more then one payload for the following keys: " << boost::algorithm::join(duplicates, ", "));
    }
    return true;
  }

  std::pair<std::string, IntervalOfValidity> ConditionsPayloadDownloader::get(const std::string& name)
  {
    // assume the payload exists, if not an exception is thrown which we let bubble up
    const PayloadInfo& info = m_payloads.at(name);
    // and download the filename if necessary
    std::string filename = getPayload(info);
    // download failed, raise an error
    if (filename.empty()) {
      B2ERROR("Failure obtaining payload '" << "' from central database");
      throw std::runtime_error("Could not download payload");
    }
    // otherwise return filename and iov
    return std::make_pair(filename, info.iov);
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

  std::string ConditionsPayloadDownloader::getPayload(const PayloadInfo& payload)
  {
    const std::string local_file = (fs::path(m_outputDir) / fs::path(payload.url).filename()).string();
    if (fs::exists(local_file)) {
      Belle2::FileSystem::Lock readlock(local_file, true);
      B2DEBUG(200, "Attempting to lock payload file for reading ...");
      if (!readlock.lock(m_timeout, true)) return getTemporary(payload.url, payload.digest);
      B2DEBUG(200, "Got read lock, check digest ...");
      std::ifstream local_stream(local_file.c_str(), std::ios::binary);
      //Ok we have read lock, check md5 and if it's fine return the filename
      if (checkDigest(local_stream, payload.digest)) return local_file;
      B2DEBUG(200, "Check failed, need to download");
      // otherwie let's release the file lock so that other processes can
      // modify the file.
    }
    // ok, try to download the file
    {
      // for this we need a write lock
      Belle2::FileSystem::Lock writelock(local_file);
      // if we cannot get one the folder/file might be write protected or
      // download by another process takes to long. So let's download into
      // temporary file.
      B2DEBUG(200, "Attempting to lock payload file for writing ...");
      if (!writelock.lock(m_timeout, true)) return getTemporary(payload.url, payload.digest);
      std::fstream local_stream(local_file.c_str(), std::ios::binary | std::ios::in | std::ios::out);
      B2DEBUG(200, "Got write lock, check for file access ...");
      if (!local_stream.good()) {
        B2WARNING("Cannot open " << local_file << " for writing");
      }
      B2DEBUG(200, "Ok, check digest again ...");
      // ok we have write lock. Someone might have downloaded the file
      // while we waited, check md5sum again.
      if (checkDigest(local_stream, payload.digest)) return local_file;
      // we have lock and it's broken so download the file
      B2DEBUG(200, "Still not good, download now ...");
      if (!downloadAndCheck(payload.url, local_stream, payload.digest)) return getTemporary(payload.url, payload.digest);
      B2DEBUG(200, "Download of payload successful");
      return local_file;
    }
  }

  bool ConditionsPayloadDownloader::download(const std::string& url, std::ostream& buffer)
  {
    //make sure we have an active curl session ...
    SessionGuard session(*this);
    B2DEBUG(50, "Download of " << url << " started ...");
    //rewind the stream to the beginning
    buffer.clear();
    buffer.seekp(0, std::ios::beg);
    if (!buffer.good()) {
      B2ERROR("Cannot write to stream when downloading " << url);
      return false;
    }
    // build the request ...
    CURLcode res{CURLE_FAILED_INIT};
    char errbuf[CURL_ERROR_SIZE];
    curl_slist* headers{curl_slist_append(nullptr, "Accept: application/json")};
    progress_status status;
    status.url = &url;
    // and set all the curl options
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_function);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, &status);
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);
    // perform the request ...
    res = curl_easy_perform(m_curl);
    // free the headers
    curl_slist_free_all(headers);
    // flush output
    buffer.flush();
    // and check for errors which occured during download ...
    if (res != CURLE_OK) {
      size_t len = strlen(errbuf);
      if (len) {
        B2WARNING("Could not get '" << url << "': " << errbuf);
      } else {
        B2WARNING("Could not get '" << url << "': " << curl_easy_strerror(res));
      }
      return false;
    }
    // all fine
    B2DEBUG(50, "Download of " << url << " finished succesfully.");
    return true;
  }

  bool ConditionsPayloadDownloader::downloadAndCheck(const std::string& url, std::iostream& stream, const std::string& digest)
  {
    if (!download(url, stream)) return false;
    const std::string actual = calculateDigest(stream);
    if (actual != digest) {
      B2WARNING("Checksum mismatch for " << url << ": " << "should be " <<  digest << " but found " << actual);
      return false;
    }
    return true;
  }

  std::string ConditionsPayloadDownloader::getTemporary(const std::string& url, const std::string& digest)
  {
    const auto openmode = std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
    std::unique_ptr<FileSystem::TemporaryFile> tmpfile(new FileSystem::TemporaryFile(openmode));
    B2DEBUG(100, "Trying to download into temporary file " << tmpfile->getName() << " ... uhh, eeeh, da isses ja!");
    if (downloadAndCheck(url, *tmpfile, digest)) {
      m_tempfiles.emplace_back(std::move(tmpfile));
      return m_tempfiles.back()->getName();
    }
    return "";
  }
}
