/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/Downloader.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/Utils.h>
#include <framework/utilities/EnvironmentVariables.h>

#include <curl/curl.h>
#include <TMD5.h>
#include <TRandom.h>

#include <boost/algorithm/string.hpp>

#include <chrono>
#include <memory>
#include <thread>

namespace Belle2::Conditions {
  /** struct encapsulating all the state information needed by curl */
  struct CurlSession {
    /** curl session information */
    CURL* curl{nullptr};
    /** headers to send with every request */
    curl_slist* headers{nullptr};
    /** error buffer in case some error happens during downloading */
    char errbuf[CURL_ERROR_SIZE];
    /** last time we printed the status (in ns) */
    double lasttime{0};
  };

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
      // size in bytes is size*nmemb so copy the correct amount and return it to curl
      try {
        std::ostream& stream = *static_cast<std::ostream*>(userp);
        stream.write(static_cast<const char*>(buffer), size * nmemb);
      } catch (std::ios_base::failure& e) {
        B2ERROR("Writing error while downloading: " << e.code().message() << '(' << e.code().value() << ')');
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
      CurlSession& status = *static_cast<CurlSession*>(clientp);
      double time = Utils::getClock();
      // make sure we don't print the status too often
      if (status.lasttime != 0 && (time - status.lasttime) / Unit::ms < 200) {
        return 0;
      }
      status.lasttime = time;
      if (dltotal > 0) {
        B2DEBUG(39, "curl:= " << dlnow << " / " << dltotal << " bytes transferred");
      } else {
        B2DEBUG(39, "curl:= " << dlnow << " bytes transferred");
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
    int debug_callback([[maybe_unused]] CURL* handle, curl_infotype type, char* data, size_t size,
                       [[maybe_unused]] void* userptr)
    {
      std::string prefix = "curl:";
      // Choose loglevel: if type is CURLINFO_TEXT the messages are general
      // informations about what curl is doing. The more detailed information
      // about incoming/outgoing headers is a bit less important so give it a
      // higher log level.
      int level = 39;
      if (type == CURLINFO_TEXT) { prefix += "*"; level = 38; }
      else if (type == CURLINFO_HEADER_OUT) prefix += ">";
      else if (type == CURLINFO_HEADER_IN) prefix += "<";
      else return 0;
      // Convert char* data to a string and strip whitespace ...
      std::string message(data, size);
      boost::trim(message);
      // And log if there's something left
      if (!message.empty()) B2DEBUG(level, prefix << " " <<  message);
      return 0;
    }

    /** Return a string with the software version to send as software version */
    std::string getUserAgent()
    {
      return "BASF2/" + ::Belle2::EnvironmentVariables::get("BELLE2_RELEASE", "unknown");
    }
  }
  /* We only want to initialize curl once */
  bool Downloader::s_globalInit{false};

  Downloader& Downloader::getDefaultInstance()
  {
    static Downloader instance;
    return instance;
  }

  Downloader::~Downloader() { finishSession(); }

  std::string Downloader::escapeString(const std::string& text)
  {
    //make sure we have an active curl session ...
    auto session = ensureSession();
    char* escaped = curl_easy_escape(m_session->curl, text.c_str(), text.size());
    if (!escaped) {
      throw std::runtime_error("Could not escape string");
    }
    std::string escapedStr{escaped};
    curl_free(escaped);
    return escapedStr;
  }

  /** Join two strings and make sure that there is exactly one '/' between them */
  std::string Downloader::joinWithSlash(const std::string& base, const std::string& rest)
  {
    return boost::trim_right_copy_if(base, boost::is_any_of("/")) + "/" +
           boost::trim_left_copy_if(rest, boost::is_any_of("/"));
  }

  bool Downloader::startSession()
  {
    // start a curl session but if there is already one return false
    if (m_session) return false;
    // make sure curl is initialized correctly
    if (!s_globalInit) {
      curl_global_init(CURL_GLOBAL_ALL);
      s_globalInit = true;
    }
    // create the curl session
    m_session = std::make_unique<CurlSession>();
    m_session->curl = curl_easy_init();
    if (!m_session->curl) {
      B2FATAL("Cannot initialize libcurl");
    }
    m_session->headers = curl_slist_append(nullptr, "Accept: application/json");
    curl_easy_setopt(m_session->curl, CURLOPT_HTTPHEADER, m_session->headers);
    curl_easy_setopt(m_session->curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(m_session->curl, CURLOPT_CONNECTTIMEOUT, m_connectionTimeout);
    curl_easy_setopt(m_session->curl, CURLOPT_LOW_SPEED_LIMIT, 10 * 1024); //10 kB/s
    curl_easy_setopt(m_session->curl, CURLOPT_LOW_SPEED_TIME, m_stalledTimeout);
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
    if (EnvironmentVariables::isSet("BELLE2_CONDB_PROXY")) {
      const std::string proxy = EnvironmentVariables::get("BELLE2_CONDB_PROXY");
      curl_easy_setopt(m_session->curl, CURLOPT_PROXY, proxy.c_str());
    }
    curl_easy_setopt(m_session->curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(m_session->curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_session->curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(m_session->curl, CURLOPT_TCP_FASTOPEN, 0L);
    curl_easy_setopt(m_session->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_session->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_session->curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
    // Don't cache DNS entries, ask the system every time we need to connect ...
    curl_easy_setopt(m_session->curl, CURLOPT_DNS_CACHE_TIMEOUT, 0L);
    // and shuffle the addresses so we try a different node, otherwise we might
    // always get the same address due to system caching and RFC 3484
    curl_easy_setopt(m_session->curl, CURLOPT_DNS_SHUFFLE_ADDRESSES, 1L);
    auto version = getUserAgent();
    curl_easy_setopt(m_session->curl, CURLOPT_USERAGENT, version.c_str());
    return true;
  }

  void Downloader::finishSession()
  {
    // if there's a session clean it ...
    if (m_session) {
      curl_easy_cleanup(m_session->curl);
      curl_slist_free_all(m_session->headers);
      m_session.reset();
    }
  }

  std::string Downloader::calculateChecksum(std::istream& input)
  {
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

  void Downloader::setConnectionTimeout(unsigned int timeout)
  {
    m_connectionTimeout = timeout;
    if (m_session) {
      curl_easy_setopt(m_session->curl, CURLOPT_CONNECTTIMEOUT, m_connectionTimeout);
    }
  }

  void Downloader::setStalledTimeout(unsigned int timeout)
  {
    m_stalledTimeout = timeout;
    if (m_session) {
      curl_easy_setopt(m_session->curl, CURLOPT_LOW_SPEED_TIME, m_stalledTimeout);
    }
  }

  bool Downloader::download(const std::string& url, std::ostream& buffer, bool silentOnMissing)
  {
    //make sure we have an active curl session ...
    auto session = ensureSession();
    B2DEBUG(37, "Download started ..." << LogVar("url", url));
    // we might need to try a few times in case of HTTP>=500
    for (unsigned int retry{1};; ++retry) {
      //rewind the stream to the beginning
      buffer.clear();
      buffer.seekp(0, std::ios::beg);
      if (!buffer.good()) {
        throw std::runtime_error("cannot write to stream");
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
      // and check for errors which occurred during download ...
      if (res != CURLE_OK) {
        size_t len = strlen(m_session->errbuf);
        const std::string error = len ? m_session->errbuf : curl_easy_strerror(res);
        if (m_maxRetries > 0 && res == CURLE_HTTP_RETURNED_ERROR) {
          if (retry <= m_maxRetries) {
            // we treat everything below 500 as permanent error with the request,
            // only retry on 500.
            long responseCode{0};
            curl_easy_getinfo(m_session->curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if (responseCode >= 500) {
              // use exponential backoff but don't restrict to exact slots like
              // Ethernet, just use a random wait time between 1s and maxDelay =
              // 2^(retry)-1 * backoffFactor
              double maxDelay = (std::pow(2, retry) - 1) * m_backoffFactor;
              double seconds = gRandom->Uniform(1., maxDelay);
              B2WARNING("Could not download url, retrying ..."
                        << LogVar("url", url) << LogVar("error", error)
                        << LogVar("try", retry) << LogVar("waiting time", seconds));
              std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1e3)));
              continue;
            }
            if (responseCode == 404 and silentOnMissing) return false;
          }
        }
        throw std::runtime_error(error);
      }
      break;
    }
    // all fine
    B2DEBUG(37, "Download finished successfully." << LogVar("url", url));
    return true;
  }
} // namespace Belle2::Conditions
