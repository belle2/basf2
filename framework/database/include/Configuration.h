/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/utilities/EnvironmentVariables.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/utilities/Utils.h>

#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>

#include <variant>
#include <set>

// forward declaration for friend access
namespace Belle2 { class Database; }

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

  /** Class to enable configuration of the conditions database access in C++ and python */
  class Configuration {
  public:
    /** Initialize default values */
    Configuration();
    /** Get a reference to the instance which will be used when the Database is initialized */
    static Configuration& getInstance();
    /** Reset to default values */
    void reset() { *this = Configuration(); }
    /** Get the std::vector of default global tags */
    std::vector<std::string> getDefaultGlobalTags() const;
    /** Get the base tags: Either the default tags or the list of globaltags from the input files */
    std::vector<std::string> getBaseTags() const;
    /** Get the list of user globaltags */
    std::vector<std::string> getGlobalTags() { return m_globalTags.ensureCpp(); }
    /** Set the list of globaltags */
    void setGlobalTags(const std::vector<std::string>& list) { m_globalTags.ensureCpp() = list; }
    /** Append a globaltag */
    void appendGlobalTag(const std::string& globalTag) { m_globalTags.append(globalTag); }
    /** preprend a globaltag */
    void prependGlobalTag(const std::string& globalTag) { m_globalTags.prepend(globalTag); }
    /** check if override is enabled */
    bool overrideEnabled() const { return m_overrideEnabled; }
    /** enable globaltag override */
    void overrideGlobalTags() { m_overrideEnabled = true; }
    /** To be called by input modules with the tags to be added from input
     * files, could be an empty string if the files are not compatible.
     * In this case the user needs to supply all tags and use the override flag
     */
    void setInputGlobalTags(const std::string& inputTags, const std::vector<FileMetaData>& inputMetadata)
    {
      m_inputGlobalTags = inputTags;
      m_inputMetadata = inputMetadata;
    }
    /** Add a local text file with testing payloads */
    void appendTestingPayloadLocation(const std::string& filename) { m_testingPayloadLocations.append(filename); }
    /** Prepend a local text file with testing payloads to the list */
    void prependTestingPayloadLocation(const std::string& filename) { m_testingPayloadLocations.prepend(filename); }
    /** Set the list of local text files to look for testing payloads */
    void setTestingPayloadLocations(const std::vector<std::string>& list) { m_testingPayloadLocations.ensureCpp() = list;}
    /** Get the list of testing payload locations */
    std::vector<std::string> getTestingPayloadLocations() { return m_testingPayloadLocations.ensureCpp(); }
    /** get the list of metadata providers */
    std::vector<std::string> getMetadataProviders() { return m_metadataProviders.ensureCpp(); }
    /** Set the list of metadata providers */
    void setMetadataProviders(const std::vector<std::string>& list) { m_metadataProviders.ensureCpp() = list; }
    /** Append a metadata provider to the list */
    void appendMetadataProvider(const std::string& provider) { m_metadataProviders.append(provider); }
    /** Prepend a metadata provider to the list */
    void prependMetadataProvider(const std::string& provider) { m_metadataProviders.prepend(provider); }
    /** get the list of payload locations */
    std::vector<std::string> getPayloadLocations() { return m_payloadLocations.ensureCpp(); }
    /** Set the list of payload locations */
    void setPayloadLocations(const std::vector<std::string>& list) { m_payloadLocations.ensureCpp() = list; }
    /** Append a payload to the list of locations */
    void appendPayloadLocation(const std::string& location) { m_payloadLocations.append(location); }
    /** Prepend a payload to the list of locations */
    void prependPayloadLocation(const std::string& location) { m_payloadLocations.prepend(location); }
    /** Set the file where to save newly created payload information */
    void setNewPayloadLocation(const std::string& filename) { m_newPayloadFile = filename; }
    /** Get the filename where to save newly created payload information */
    std::string getNewPayloadLocation() const { return m_newPayloadFile; }
    /** Set the directory where to place downloaded payloads. Empty string is
     * shorthand to put them in a folder `basf2-conditions` in the temp dir */
    void setDownloadCacheDirectory(const std::string& directory) { m_downloadCacheDirectory = directory; }
    /** Get the directory where to place downloaded payloads. Empty string is
     * shorthand to put them in a folder `basf2-conditions` in the temp dir */
    std::string getDownloadCacheDirectory() const { return m_downloadCacheDirectory; }
    /** Set the timout we try to lock a file in the download cache directory for downloading */
    void setDownloadLockTimeout(size_t timeout) { m_downloadLockTimeout = timeout; }
    /** Get the timout we try to lock a file in the download cache directory for downloading */
    size_t getDownloadLockTimeout() const { return m_downloadLockTimeout; }
    /** Set the set of valid global tag states to be allowed for processing.
     * The state INVALID will always be ignored and not permitted */
    void setValidTagStates(const std::set<std::string>& states) { m_validTagStates = states; }
    /** Get the set of valid global tag states allowed to be used for processing */
    const std::set<std::string>& getValidTagStates() const { return m_validTagStates; }
    /** expose this class to python */
    static void exposePythonAPI();
  protected:
    /** Get the tuple of default global tags as python version */
    boost::python::tuple getDefaultGlobalTagsPy() const;
    /** Get the list of user globaltags as python version */
    boost::python::list getGlobalTagsPy() { return m_globalTags.ensurePy(); }
    /** Set the list of globaltags from python */
    void setGlobalTagsPy(const boost::python::list& globalTags) { m_globalTags.shallowCopy(globalTags); }
    /** enable globaltag override and set the user globaltags in one go */
    void overrideGlobalTagsPy(const boost::python::list& globalTags);
    /** Get the list of text files containing test payloads in python */
    boost::python::list getTestingPayloadLocationsPy() { return m_testingPayloadLocations.ensurePy(); }
    /** Set the list of text files containing test payloads in python */
    void setTestingPayloadLocationsPy(const boost::python::list& list) { m_testingPayloadLocations.shallowCopy(list); }
    /** Get the list of metadata providers in python */
    boost::python::list getMetadataProvidersPy() { return m_metadataProviders.ensurePy(); }
    /** Set the list of metadata providers in python */
    void setMetadataProvidersPy(const boost::python::list& list) { m_metadataProviders.shallowCopy(list); }
    /** Get the list og payload locations in python */
    boost::python::list getPayloadLocationsPy() { return m_payloadLocations.ensurePy(); }
    /** Set the list of payload locations in python */
    void setPayloadLocationsPy(const boost::python::list& list) { m_payloadLocations.shallowCopy(list); }
    /** Set a callback function from python which will be called when processing starts
     * and should return the final list of globaltags to be used. See the python documentation
     * for more details */
    void setGlobaltagCallbackPy(const boost::python::object& obj) { m_callback = obj; }
    /** get the final list of global tags to be used for processing */
    std::vector<std::string> getFinalListOfTags();
  private:
    /** Fill a target object from a list of environment variables */
    template<class T> static void fillFromEnv(T& target, const std::string& envName, const std::string& defaultValue)
    {
      const auto values = EnvironmentVariables::getOrCreateList(envName, defaultValue);
      for (const std::string& v : values) target.append(v);
    }
    /** is the globaltag override enabled? */
    bool m_overrideEnabled{false};
    /** the string containing the globaltags from all the input files */
    std::optional<std::string> m_inputGlobalTags;
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
    std::set<std::string> m_validTagStates{"TESTING", "VALIDATED", "PUBLISHED", "RUNNING"};
    /** the callback function to determine the final final list of globaltags */
    std::optional<boost::python::object> m_callback;
    /** and let the database get the final list of tags from a protected function */
    friend class ::Belle2::Database;
  };
} // Belle2::Conditions namespace
