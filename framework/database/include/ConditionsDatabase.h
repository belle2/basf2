/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/Database.h>
#include <framework/database/EConditionsDirectoryStructure.h>

namespace Belle2 {
  class ConditionsPayloadDownloader;

  /**
   * Database backend that uses the conditions service.
   */
  class ConditionsDatabase: public Database {
  public:

    /**
     * Method to set the database instance to the central database with default parameters.
     *
     * @param globalTag   The name of the global tag
     * @param logLevel    The level of log messages about not-found payloads.
     * @param payloadDir  Directory for local copies of the payloads
     */
    static void createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                                      const std::string& payloadDir = "centraldb");

    /**
     * Method to set the database instance to the central database.
     *
     * @param globalTag      The name of the global tag
     * @param restBaseName   Base name for REST services
     * @param fileBaseName   Base name for conditions files
     * @param fileBaseLocal  Directory name for local conditions files copies
     * @param logLevel       The level of log messages about not-found payloads.
     * @param invertLogging  If true log messages will be created when a
     *                       payload is found. This is intended for override
     *                       setups where a few payloads are taken from non
     *                       standard locations
     */
    static void createInstance(const std::string& globalTag, const std::string& restBaseName, const std::string& fileBaseName,
                               const std::string& fileBaseLocal, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                               bool invertLogging = false);

    /** Request an object from the database.
     * @param event   The metadata of the event for which the object should be valid.
     * @param query   Object containing the necessary identification which will
     *                be filled with all information about the payload.
     * @return        True if the payload could be found. False otherwise.
     */
    virtual bool getData(const EventMetaData& event, DBQuery& query) override;

    /**
     * Store an object in the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& name, TObject* object,
                           const IntervalOfValidity& iov) override;

    /**
     * Add a payload file to the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param fileName   The name of the payload file.
     * @param iov        The interval of validity of the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool addPayload(const std::string& name, const std::string& fileName,
                            const IntervalOfValidity& iov) override;

    /** Return the global tag */
    std::string getGlobalTag() const { return m_globalTag; }

    /** Set the base of the url used for REST requests to the central server */
    void setRESTBase(const std::string& restBase);

    /** Set the server list to try to connect to a central server
     *  @param servList list of urls to try in turn to find a suitable server
     *    to connect to. The servers will be tried in order until one succeeds
     */
    void setServerList(const std::vector<std::string>& serverList);

    /** Add a directory to the list of directories to look for payloads before
     * downloading them.
     * @param localDir path to the directory to add to the list
     * @param structure indicate how the payloads are stored in this directory
     */
    void addLocalDirectory(const std::string& localDir, EConditionsDirectoryStructure structure);

  private:
    /**
     * Hidden constructor, as it is a singleton.
     *
     * @param globalTag      The name of the global tag
     * @param payloadDir     The name of the directory in which the payloads are stored.
     */
    explicit ConditionsDatabase(const std::string& globalTag, const std::string& payloadDir = "");

    /** Hidden copy constructor, as it is a singleton. */
    ConditionsDatabase(const ConditionsDatabase&);

    /** Hidden destructor, as it is a singleton. */
    virtual ~ConditionsDatabase();

    /** Global tag. */
    std::string m_globalTag;

    /** The directory of payloads. */
    std::string m_payloadDir;

    /** Experiment number for which the payloads were obtained. */
    int m_currentExperiment;

    /** Run number for which the payloads were obtained. */
    int m_currentRun;

    /** Class to manage lookup and downloads of payloads */
    std::unique_ptr<ConditionsPayloadDownloader> m_downloader;

  };
} // namespace Belle2
