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

#include <boost/bimap.hpp>

namespace Belle2 {

  /** Simple class to encapsulate libcurl as used by the ConditionsDatabase */
  class ConditionsPayloadDownloader final {
  public:
    /** Simple struct to group all information necessary for a single payload */
    struct PayloadInfo {
      /** logical filename to the payload */
      std::string payloadUrl;
      /** base url if download is necessary */
      std::string baseUrl;
      /** digest (checksum) of the payload */
      std::string digest;
      /** the interval of validity */
      IntervalOfValidity iov;
      /** the revision of the payload */
      int revision;
    };

    /** Directory structure for local lookup directories to allow for different
     * storage layouts: all payloads in one folder, all payloads in grouped in
     * folders by their name or grouped in folders by beginning of their
     * checksum
     */
    enum EDirectoryStructure {
      /** directory contains all payloads directly */
      c_flatDirectory,
      /** directory contains all payloads as they are specified by payloadUrl
       * which might or might not include subdirectories */
      c_logicalSubdirectories,
      /** directories contains all payloads in subdirectories starting with the
       * hash values. i.e. if payload has checksum 0123456789ABCDEF it would be
       * stored in 0/12/<filename> to evenly distribute all payloads across
       * subdirectories. This is the same scheme used for git objects
       */
      c_digestSubdirectories,
    };

    /** Create a new download session */
    ConditionsPayloadDownloader(const std::string& restUrl = "http://belle2db.hep.pnnl.gov/b2s/rest/",
                                const std::string& localDir = "centraldb", int timeout = 10):
      m_restUrl(restUrl), m_timeout(timeout) { addLocalDirectory(localDir, c_flatDirectory); }
    /** Close the session */
    ~ConditionsPayloadDownloader() { finishSession(); }

    /** Add a directory to the list of directories to look for payloads before
     * downloading them.
     * @param directory path to the directory to add to the list
     * @param structure indicate how the payloads are stored in this directory
     */
    void addLocalDirectory(const std::string& directory, EDirectoryStructure structure);

    /** Start a new curl session if none is active at the moment
     * @returns true if a new session was started, false if one was active already
     */
    bool startSession();
    /** Finish an existing curl sesssion if any is active at the moment */
    void finishSession();

    /** Update the list of payloads */
    bool update(const std::string& globalTag, int experiment, int run);

    /** Check wether a payload exists */
    bool exists(const std::string& name) const
    {
      return m_payloads.find(name) != end(m_payloads);
    }

    /** Get a payload. Will return a pair consiting of the filename and the interval of validity.
     *
     * @warning this function will raise a std::out_of_bounds exception if
     * the payload doesn't exist and a std::runtime_error if the payload
     * cannot be downloaded.
     *
     * @param name name of the payload
     * @returns absolute filename of the payload and the interval of validity
     */
    std::pair<std::string, IntervalOfValidity> get(const std::string& name);

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
    static std::string getLocalName(const std::string& dir, EDirectoryStructure structure, const PayloadInfo& payload);
    /** return the filename of a payload
     * @param payload payload information to obtain the filename for
     * @returns filename if it exists or could be downloaded, empty string on error
     */
    std::string getPayload(const PayloadInfo& payload);
    /** Download the given url into a temporary file and return the filename if the digest matches
     * @param key to identify payload
     * @param url URL to download
     * @param digest known digest of the data
     * @returns temporary filename with the downloaded url, empty string on error
     */
    std::string getTemporary(const std::string& key, const std::string& url, const std::string& digest);

    /** curl session handle */
    void* m_curl{nullptr};
    /** flag to indicate whether curl has been initialized already */
    static bool s_globalInit;
    /** base url to prepend to the rest calls */
    std::string m_restUrl;
    /** local directories where we look for payloads. If we cannot find them
     * anywhere we download them and try to put them in the first entry */
    std::vector<std::pair<std::string, EDirectoryStructure>> m_localDirectories;
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
