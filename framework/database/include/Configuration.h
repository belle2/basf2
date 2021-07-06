/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/utilities/EnvironmentVariables.h>
#include <framework/dataobjects/FileMetaData.h>

#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>

#include <variant>
#include <set>

namespace Belle2::Conditions {
  /** Wrapper class for a list of strings to either be held in a std::vector or
   * in a python list. It's basically a std::variant with some convenince
   * members to convert between the two representations.
   *
   * This is necessary as python lists can only exist after Py_Initialize() and
   * before Py_Finalize() and since we want this configuration class to work also
   * in command line tools we need to be able to use std::vector unless python
   * functions are called.
   */
  class CppOrPyList {
  public:
    /** Return the python list version. Convert if necessary */
    boost::python::list& ensurePy();
    /** Return the C++ vector version. Convert if necessary */
    std::vector<std::string>& ensureCpp();
    /** Append an element to whatever representation we currently have */
    void append(const std::string& element);
    /** Prepend an element to whatever representation we currently have */
    void prepend(const std::string& element);
    /** shallow copy all elements of the source object into the python representation.
     * Also converts to python representation */
    void shallowCopy(const boost::python::object& source);
  private:
    /** Store either a std::vector or a python list of strings */
    std::variant<std::vector<std::string>, boost::python::list> m_value;
  };

  /** Class to enable configuration of the conditions database access in C++ and
   * python. This class contains all settings relevant for the usage of the
   * conditions database.
   *
   * Mostly this class is just a container for user settings without much
   * intelligence. The only thing it does is construct the final list of
   * globaltags to be used from input data and user settings, maybe via a user
   * callback, in getFinalListOfTags().
   *
   * the exposePythonAPI() function contains detailed documentation to most of
   * the members in this class for the python interface and details all of the
   * settings.
   */
  class Configuration {
  public:
    /** Initialize default values */
    Configuration();
    /** Get a reference to the instance which will be used when the Database is initialized */
    static Configuration& getInstance();
    /** Reset to default values */
    void reset();

    /** @name Configuration of globaltags
     *
     * These members are responsible to configure the list of user globaltags which
     * will be used in addition or instead of the base globaltags.
     */
    ///@{

    /** Append a globaltag */
    void appendGlobalTag(const std::string& globalTag) { ensureEditable(); m_globalTags.append(globalTag); }
    /** preprend a globaltag */
    void prependGlobalTag(const std::string& globalTag) { ensureEditable(); m_globalTags.prepend(globalTag); }
    /** Set the list of globaltags */
    void setGlobalTags(const std::vector<std::string>& list) { ensureEditable(); m_globalTags.ensureCpp() = list; }
    /** Set the list of globaltags from python */
    void setGlobalTagsPy(const boost::python::list& globalTags) { ensureEditable(); m_globalTags.shallowCopy(globalTags); }
    /** Get the list of user globaltags */
    std::vector<std::string> getGlobalTags() { return m_globalTags.ensureCpp(); }
    /** Get the list of user globaltags as python version */
    boost::python::list getGlobalTagsPy() { return m_globalTags.ensurePy(); }

    /** Get the std::vector of default globaltags */
    std::vector<std::string> getDefaultGlobalTags() const;
    /** Get the tuple of default globaltags as python version */
    boost::python::tuple getDefaultGlobalTagsPy() const;

    /** To be called by input modules with the tags to be added from input
     * files.
     *
     * This method is to be called by input modules which know the list of
     * globaltags to be used but cannot provide FileMetaData instances for these
     * input files.
     *
     * In case an empty list is put in this means global tag replay is not
     * possible for these input files and the user will need to manually supply
     * all tags and use the override flag to disable any attempt at tag replay.
     *
     * \sa setInputMetadata()
     */
    void setInputGlobaltags(const std::vector<std::string>& inputTags)
    {
      ensureEditable();
      m_inputGlobaltags = inputTags;
    }

    /** To be called by input modules with the list of all input FileMetaData
     *
     * This method is to be called by input modules which have a full list of
     * FileMetaData instances for all input files. It will then check if all
     * instances have a compatible globaltag setting and call `setInputGlobalTags`
     * accordingly.
     *
     * \sa setInputGlobaltags()
     */
    void setInputMetadata(const std::vector<FileMetaData>& inputMetadata);

    /** Get the base globaltags to be used in addition to user globaltags.
     *
     * - If no input file is present, the result of getBaseTags() is identical
     *   to the list returned by getDefaultGlobalTags()
     * - If one ore more input files are present, the input module is
     *   responsible to call setInputGlobalTags(). If setInputGlobalTags() has
     *   been called then getBaseTags() returns the list of globaltags by
     *   splitting the value passed from the input modules at the commas.
     */
    std::vector<std::string> getBaseTags() const;

    /** Enable globaltag override: If this is called once than overrideEnabled()
     * will return true and getFinalListOfTags() will just return
     * getGlobalTags() */
    void overrideGlobalTags() { ensureEditable(); m_overrideEnabled = true; }
    /** Enable globaltag override and set the list of user globaltags in one go */
    void overrideGlobalTagsPy(const boost::python::list& globalTags);
    /** Check if override is enabled by previous calls to overrideGlobalTags() */
    bool overrideEnabled() const { return m_overrideEnabled; }

    /** Disable global tag replay.
     *
     * This restores the previous behavior of just using the default globaltag
     * and is just a shortcut for calling overrideGlobalTags() and appending
     * each tag in getDefaultGlobalTags() tho the list of globaltags using
     * appendGlobalTag()
     */
    void disableGlobalTagReplay()
    {
      overrideGlobalTags();
      for (const auto& tag : getDefaultGlobalTags()) appendGlobalTag(tag);
    }

    /** Get the final list of globaltags to be used for processing.
     *
     * - if overrideEnabled() is false then processing will use getGlobalTags()
     *   and getBaseTags() (in this order) as the list of globaltags to be used
     *   for processing. if getBaseTags() is empty an error will be raised and
     *   processing will be aborted.
     * - if overrideEnabled() is true (by calling overrideGlobalTags()) then
     *   only the list from getGlobaltags() will be used for processing.
     *
     * \see setGlobaltagCallbackPy() to set a callback function from python to
     * further customize this behavior
     *
     * \warning This function is only to be called by the conditions database
     * service when processing starts. Calling this function from user code
     * might abort processing if called at the wrong time or without properly
     * initializing the settings.
     */
    std::vector<std::string> getFinalListOfTags();

    ///@}

    /** @name Testing Payload Configuration These members control where to look
     * for temporary testing payloads. Each entry in the list of locations
     * should be a filename of a textfile containing payload information. All
     * payload files need to be in the same directory as the text file.
     *
     * Entries are highest priority first: Payloads found by earlier entries will
     * take precedence over later entries. Payloads found in these text files.
     * take precedence over payloads from globaltags.
     *
     * \warning This causes non reproducible results and is **only for testing**
     * purposes.
     */
    ///@{

    /** Add a local text file with testing payloads */
    void appendTestingPayloadLocation(const std::string& filename) { ensureEditable(); m_testingPayloadLocations.append(filename); }
    /** Prepend a local text file with testing payloads to the list */
    void prependTestingPayloadLocation(const std::string& filename) { ensureEditable(); m_testingPayloadLocations.prepend(filename); }
    /** Set the list of local text files to look for testing payloads */
    void setTestingPayloadLocations(const std::vector<std::string>& list) { ensureEditable(); m_testingPayloadLocations.ensureCpp() = list;}
    /** Set the list of text files containing test payloads in python */
    void setTestingPayloadLocationsPy(const boost::python::list& list) { ensureEditable(); m_testingPayloadLocations.shallowCopy(list); }
    /** Get the list of testing payload locations */
    std::vector<std::string> getTestingPayloadLocations() { return m_testingPayloadLocations.ensureCpp(); }
    /** Get the list of text files containing test payloads in python */
    boost::python::list getTestingPayloadLocationsPy() { return m_testingPayloadLocations.ensurePy(); }

    ///@}

    /** @name Configure Metadata providers
     *
     * These members are used to configure metadata providers: Where to look for
     * payload information given the list of globaltags.
     *
     * - Each entry in the list should be an URI or filename to a central REST
     *   server or a sqlite file containing a previously downloaded dump.
     * - The first entry in the list is used unless there is a problem reaching
     *   the server/reading the file in which case we use the next entry in the
     *   list as failover.
     */
    ///@{

    /** Append a metadata provider to the list */
    void appendMetadataProvider(const std::string& provider) { ensureEditable(); m_metadataProviders.append(provider); }
    /** Prepend a metadata provider to the list */
    void prependMetadataProvider(const std::string& provider) { ensureEditable(); m_metadataProviders.prepend(provider); }
    /** Set the list of metadata providers */
    void setMetadataProviders(const std::vector<std::string>& list) { ensureEditable(); m_metadataProviders.ensureCpp() = list; }
    /** Set the list of metadata providers in python */
    void setMetadataProvidersPy(const boost::python::list& list) { ensureEditable(); m_metadataProviders.shallowCopy(list); }
    /** Get the list of metadata providers */
    std::vector<std::string> getMetadataProviders() { return m_metadataProviders.ensureCpp(); }
    /** Get the list of metadata providers in python */
    boost::python::list getMetadataProvidersPy() { return m_metadataProviders.ensurePy(); }
    ///@}

    /** @name Payload Location Configuration
     *
     * Configure where to look for payload files. This should be a list of
     * directories and or http(s) urls where the files containing the payload
     * content can be found. Each location will be tried in turn and if the
     * payload is not found in any of them we try to download it from the
     * official server.
     */
    ///@{

    /** Append a payload to the list of locations */
    void appendPayloadLocation(const std::string& location) { ensureEditable(); m_payloadLocations.append(location); }
    /** Prepend a payload to the list of locations */
    void prependPayloadLocation(const std::string& location) { ensureEditable(); m_payloadLocations.prepend(location); }
    /** Set the list of payload locations */
    void setPayloadLocations(const std::vector<std::string>& list) { ensureEditable(); m_payloadLocations.ensureCpp() = list; }
    /** Set the list of payload locations in python */
    void setPayloadLocationsPy(const boost::python::list& list) { ensureEditable(); m_payloadLocations.shallowCopy(list); }
    /** Get the list of payload locations */
    std::vector<std::string> getPayloadLocations() { return m_payloadLocations.ensureCpp(); }
    /** Get the list og payload locations in python */
    boost::python::list getPayloadLocationsPy() { return m_payloadLocations.ensurePy(); }

    ///@}

    /** @name Expert Settings
     *
     * These members are for changing some expert settings which should not be
     * necessary for most users.
     *
     * These functions are exported to python using one `exper_settings()` function
     * so no separate signatures for python are necessary.
     */
    ///@{

    /** Set the file where to save newly created payload information */
    void setNewPayloadLocation(const std::string& filename) { ensureEditable(); m_newPayloadFile = filename; }
    /** Get the filename where to save newly created payload information */
    std::string getNewPayloadLocation() const { return m_newPayloadFile; }

    /** Set the directory where to place downloaded payloads. Empty string is
     * shorthand to put them in a folder `basf2-conditions` in the temp dir */
    void setDownloadCacheDirectory(const std::string& directory) { ensureEditable(); m_downloadCacheDirectory = directory; }
    /** Get the directory where to place downloaded payloads. Empty string is
     * shorthand to put them in a folder `basf2-conditions` in the temp dir */
    std::string getDownloadCacheDirectory() const { return m_downloadCacheDirectory; }

    /** Set the timout we try to lock a file in the download cache directory for downloading */
    void setDownloadLockTimeout(size_t timeout) { ensureEditable(); m_downloadLockTimeout = timeout; }
    /** Get the timout we try to lock a file in the download cache directory for downloading */
    size_t getDownloadLockTimeout() const { return m_downloadLockTimeout; }

    /** Set the set of usable globaltag states to be allowed for processing.
     * The state INVALID will always be ignored and not permitted */
    void setUsableTagStates(const std::set<std::string>& states) { ensureEditable(); m_usableTagStates = states; }
    /** Get the set of usable globaltag states allowed to be used for processing */
    const std::set<std::string>& getUsableTagStates() const { return m_usableTagStates; }

    /** Set a callback function from python which will be called when processing starts
     * and should return the final list of globaltags to be used. See the python documentation
     * for more details */
    void setGlobaltagCallbackPy(const boost::python::object& obj) { ensureEditable(); m_callback = obj; }

    ///@}

    /** Set by the Database singleton upon initialization and cleanup */
    void setInitialized(bool value) { m_databaseInitialized = value; }
    /** expose this class to python */
    static void exposePythonAPI();
  private:
    /** Fill a target object from a list of environment variables */
    template<class T> static void fillFromEnv(T& target, const std::string& envName, const std::string& defaultValue)
    {
      const auto values = EnvironmentVariables::getOrCreateList(envName, defaultValue);
      for (const std::string& v : values) target.append(v);
    }
    /** Check whether the configuration object can be edited or if the database has been initialized already */
    void ensureEditable() const
    {
      if (m_databaseInitialized)
        throw std::runtime_error("Database already initialized, please reset before changing the configuration object");
    }
    /** is the globaltag override enabled? */
    bool m_overrideEnabled{false};
    /** the list of globaltags from all the input files to be used in addition
     * to the user globaltags */
    std::optional<std::vector<std::string>> m_inputGlobaltags;
    /** the file metadata of all input files if globaltag replay is requested by input module */
    std::vector<FileMetaData> m_inputMetadata;
    /** the list with all user globaltags */
    CppOrPyList m_globalTags;
    /** the files with testing payloads to use during processing */
    CppOrPyList m_testingPayloadLocations;
    /** the list with all the metadata providers */
    CppOrPyList m_metadataProviders;
    /** the list with all the payload locations */
    CppOrPyList m_payloadLocations;
    /** the file to put the newly created payload information */
    std::string m_newPayloadFile{"localdb/database.txt"};
    /** the directory to put newly downloaded payloads */
    std::string m_downloadCacheDirectory{""};
    /** the timeout when trying to lock files in the download directory */
    size_t m_downloadLockTimeout{120};
    /** the tag states accepted for processing */
    std::set<std::string> m_usableTagStates{"TESTING", "VALIDATED", "PUBLISHED", "RUNNING"};
    /** the callback function to determine the final final list of globaltags */
    std::optional<boost::python::object> m_callback;
    /** bool indicating whether the database has been initialized, in which case any changes to the configuration object
     * would not be reflected by the database setup so we refuse them */
    bool m_databaseInitialized{false};
  };
} // Belle2::Conditions namespace
