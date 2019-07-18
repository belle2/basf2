/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/utilities/ScopeGuard.h>

#include <iosfwd>
#include <memory>

namespace Belle2::Conditions {
  /** Forward declare internal curl session pointer to limit exposure to curl headers */
  struct CurlSession;

  /** Simple class to encapsulate libcurl as used by the ConditionsDatabase */
  class Downloader final {
  public:
    /** Create a new payload downloader
     */
    Downloader() = default;
    /** Destructor */
    ~Downloader();

    /** Return the default instance. There can be multiple instances without
     * problem but we provide a default one to allow for better pipelining
     * support
     */
    static Downloader& getDefaultInstance();
    /** Start a new curl session if none is active at the moment
     * @returns true if a new session was started, false if one was active already
     */
    bool startSession();
    /** Finish an existing curl sesssion if any is active at the moment */
    void finishSession();
    /** Make sure there's an active session and return a ScopeGuard object that
     * closes the session on destruction in case a new session was created;
     */
    ScopeGuard ensureSession()
    {
      bool started = startSession();
      return ScopeGuard([this, started] {if (started) finishSession();});
    }

    /** Get the timeout to wait for connections in seconds, 0 means the built in curl default */
    static unsigned int getConnectionTimeout() { return s_connectionTimeout; }
    /** Get the timeout to wait for stalled connections (<10KB/s), 0 means no timeout */
    static unsigned int getStalledTimeout() { return s_stalledTimeout; }
    /** Get the number of retries to perform when downloading failes with HTTP response code >=500, 0 means no retries*/
    static unsigned int getMaxRetries() { return s_maxRetries; }
    /** Get the backoff factor for retries in seconds */
    static unsigned int getBackoffFactor() { return s_backoffFactor; }
    /** Set the timeout to wait for connections in seconds, 0 means built in default */
    static void setConnectionTimeout(unsigned int timeout) { s_connectionTimeout = timeout; }
    /** Set the timeout to wait for stalled connections (<10KB/s), 0 disables timeout */
    static void setStalledTimeout(unsigned int timeout) { s_stalledTimeout = timeout; }
    /** Set the number of retries to perform when downloading failes with HTTP response code >=500, 0 disables retry */
    static void setMaxRetries(unsigned int retries) { s_maxRetries = retries; }
    /** Set the backoff factor for retries in seconds. Minimum is 1 and 0 will be silently converted to 1 */
    static void setBackoffFactor(unsigned int factor) { s_backoffFactor = std::max(1u, factor); }

    /** get an url and save the content to stream
     * This function raises exceptions when there are any problems
     * @warning any contents in the stream will be overwritten
     * @param url the url to download
     * @param stream the stream to save the output to
     * @param silentOnMissing if true do not emit a warning on 404 Not Found but
     *     just return false silently. Useful when checking if a file exists on the server
     * @return true on success, false on any error
     */
    bool download(const std::string& url, std::ostream& stream, bool silentOnMissing = false);

    /** check the digest of a stream
     * @param input stream to check, make sure the stream is in a valid state pointing to the correct position
     * @param checksum expected hash digest of the data
     * @return true if digest matches, false otherwise
     */
    bool verifyChecksum(std::istream& input, const std::string& checksum) { return calculateChecksum(input) == checksum; }
    /** Escape a string to make it safe to be used in web requests */
    std::string escapeString(const std::string& text);
    /** Join two strings and make sure that there is exactly one '/' between them */
    std::string joinWithSlash(const std::string& base, const std::string& second);

  private:
    /** calculate the digest/checksum on a given string.
     * @param input input stream containing the data
     * @returns the hex digest of the checksum
     */
    static std::string calculateChecksum(std::istream& input);

    /** curl session handle */
    std::unique_ptr<CurlSession> m_session;
    /** flag to indicate whether curl has been initialized already */
    static bool s_globalInit;
    /** Timeout to wait for connections in seconds */
    static unsigned int s_connectionTimeout;
    /** Timeout to wait for stalled connections (<10KB/s) */
    static unsigned int s_stalledTimeout;
    /** Number of retries to perform when downloading failes with HTTP response code >=500 */
    static unsigned int s_maxRetries;
    /** Backoff factor for retries in seconds */
    static unsigned int s_backoffFactor;
  };
} // namespace Belle2::Conditions
