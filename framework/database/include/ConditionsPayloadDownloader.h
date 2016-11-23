/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <iosfwd>
#include <memory>
#include <vector>
#include <map>
#include <framework/utilities/FileSystem.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/EConditionsDirectoryStructure.h>

#include <boost/bimap.hpp>

namespace Belle2 {
  /** Forward declare internal curl session pointer to limit exposure to curl headers
   */
  struct ConditionsCurlSession;

  /** Simple class to encapsulate libcurl as used by the ConditionsDatabase */
  class ConditionsPayloadDownloader final {
  public:

    /** Simple struct to group all information necessary for a single payload */
    struct PayloadInfo {
      /** full filename to the payload file */
      std::string filename{""};
      /** logical filename to the payload */
      std::string payloadUrl{""};
      /** base url if download is necessary */
      std::string baseUrl{""};
      /** digest (checksum) of the payload */
      std::string digest{""};
      /** the interval of validity */
      IntervalOfValidity iov;
      /** the revision of the payload */
      int revision{ -1};
    };

    /** Simple class to make sure that the curl session is closed correctly.
     * When this class is instantiated with a downloader instance it will make
     * sure that there is an active curl session. If a session is created (i.e.
     * there was no session already) it will also end the session once this
     * object goes out of scope
     */
    class SessionGuard final {
    public:
      /** Constructor which makes sure there is an active session */
      SessionGuard(ConditionsPayloadDownloader& instance): m_instance(instance), m_createdSession(instance.startSession()) {}
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
      ConditionsPayloadDownloader& m_instance;
      /** indicates whether we created a session so we will also close it */
      bool m_createdSession;
    };

    /** Create a new payload downloader
     * @param restURL base url for rest requests
     * @param localDir output directory for payloads that need to be downloaded
     * @param timeout timeout to wait for lock on file when another process is
     * already downloading a payload. After that time the payload will be
     * loaded into a temporary file.
     */
    ConditionsPayloadDownloader(const std::string& localDir = "centraldb",
                                const std::string& restUrl = "http://belle2db.hep.pnnl.gov/b2s/rest/",
                                int timeout = 10);
    /** Destructor */
    ~ConditionsPayloadDownloader();

    /** Set the base of the url used for REST requests to the central server */
    void setRESTBase(const std::string& restUrl) { m_restUrl = restUrl; }

    /** Add a directory to the list of directories to look for payloads before
     * downloading them.
     * @param directory path to the directory to add to the list
     * @param structure indicate how the payloads are stored in this directory
     */
    void addLocalDirectory(const std::string& directory, EConditionsDirectoryStructure structure);

    /** Start a new curl session if none is active at the moment
     * @returns true if a new session was started, false if one was active already
     */
    bool startSession();

    /** Finish an existing curl sesssion if any is active at the moment */
    void finishSession();

    /** Update the list of payloads
     * @returns true if successful, false on any error
     */
    bool update(const std::string& globalTag, int experiment, int run);

    /** Check wether a payload exists */
    bool exists(const std::string& name) const
    {
      return m_payloads.find(name) != std::end(m_payloads);
    }

    /** Get a payload. Will return a reference to the PayloadInfo containing all
     * the information about the payload.
     *
     * @warning this function will raise a std::out_of_bounds exception if
     * the payload doesn't exist. If the payload cannot be downloaded or found
     * locally the filename will be empty
     *
     * @param name name of the payload
     * @returns absolute filename of the payload and the interval of validity
     */
    const PayloadInfo& get(const std::string& name);

    /** set a mapping from experiment name to experiment number.
     * The experiment numbers and names need to be unique as we have to
     * transform the mapping in both directions. So a experiment number cannot
     * have multiple names different experiment numbers cannot have the same
     * name.
     *
     * @param experiment experiment number as used in the EventMetaData
     * @param name       name of that experiment in the ConditionsDB
     * @return           true if the mapping could be added, false if there's a
     *                   conflict with an existing entry.
     */
    bool addExperimentName(int experiment, const std::string& name);

  private:

    /** get an url and save the content to stream
     * @param url the url to download
     * @param stream the stream to save the output to
     * @returns true on success, false on any error
     */
    bool download(const std::string& url, std::ostream& stream);

    /** check the digest of a stream
     * @param input stream to check, make sure the stream is in a valid state pointing to the correct position
     * @param digest expected hash digest of the data
     * @returns true if digest matches, false otherwise
     */
    bool checkDigest(std::istream& input, const std::string& digest)
    {
      return calculateDigest(input) == digest;
    }

    /** download an url to a stream and check the digest against a known value.
     * @warning any contents in the stream will be overwritten
     * @param url url to download
     * @param stream the stream to save the output to
     * @param digest the known digest of the downloaded data
     * @returns true on success, false on any error
     */
    bool downloadAndCheck(const std::string& url, std::iostream& stream, const std::string& digest);

    /** calculate the digest/checksum on a given string.
     * @param input input stream containing the data
     * @returns the hex digest of the checksum
     */
    static std::string calculateDigest(std::istream& input);

    /** obtain the filename of a payload in a given directory entry
     * @param dir directory where the payload should be
     * @param stucture storage structure of the directory
     * @param payload payload information
     * @returns filename of the payload honoring the selected directory structure
     */
    static std::string getLocalName(const std::string& dir, EConditionsDirectoryStructure structure,
                                    const PayloadInfo& payload);

    /** return the filename of a payload
     * @param payload payload information to obtain the filename for
     * @returns filename if it exists or could be downloaded, empty string on error
     */
    std::string getPayloadFile(const PayloadInfo& payload);

    /** Download the given url into a temporary file and return the filename if the digest matches
     * @param key to identify payload
     * @param url URL to download
     * @param digest known digest of the data
     * @returns temporary filename with the downloaded url, empty string on error
     */
    std::string getTemporary(const std::string& key, const std::string& url, const std::string& digest);

    /** curl session handle */
    std::unique_ptr<ConditionsCurlSession> m_session;
    /** flag to indicate whether curl has been initialized already */
    static bool s_globalInit;
    /** base url to prepend to the rest calls */
    std::string m_restUrl;
    /** local directories where we look for payloads. If we cannot find them
     * anywhere we download them and try to put them in the first entry */
    std::vector<std::pair<std::string, EConditionsDirectoryStructure>> m_localDirectories;
    /** number of seconds to wait for obtaining a lock */
    int m_timeout{10};
    /** Map of all payloads we downloaded to a temporary file */
    std::map<std::string, std::unique_ptr<FileSystem::TemporaryFile>> m_tempfiles;
    /** Map of all existing payloads */
    std::map<std::string, PayloadInfo> m_payloads;
    /** bidirectional mapping from experiment number to name in the central
     * database, only used for v1 api, will go away soon */
    boost::bimap<int, std::string> m_mapping;
  };
}
