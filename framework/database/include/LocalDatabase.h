/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/Database.h>

#include <string>
#include <map>
#include <vector>
#include <utility>


namespace Belle2 {
  /**
   * Implentation of a database backend that uses local root files for the
   * payload storage and a text file for the assignment of IoVs to payloads.
   */
  class LocalDatabase: public Database {
  public:

    /**
     * Method to set the database instance to a local database.
     *
     * @param fileName   The name of the database text file with the IoV assignments.
     * @param payloadDir The name of the directory in which the payloads are stored.
     *                   By default the same directory as the one containing the database text file is used.
     * @param readOnly   If this flag is set the database is opened in read only mode without locking.
     * @param logLevel   The level of log messages about not-found payloads.
     * @param invertLogging  If true log messages will be created when a payload is
     *                   found. This is intended for override setups where a
     *                   few payloads are taken from non standard locations
     */
    static void createInstance(const std::string& fileName = "database.txt", const std::string& payloadDir = "",
                               bool readOnly = false, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
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

  private:
    /**
     * Hidden constructor, as it is a singleton.
     *
     * @param fileName   The name of the database text file with the IoV assignments.
     * @param payloadDir The name of the directory in which the payloads are stored.
     *     By default the same directory as the one containing the database text file is used.
     * @param readOnly   If this flag is set the database is opened in read only mode without locking.
     */
    explicit LocalDatabase(const std::string& fileName, const std::string& payloadDir = "", bool readOnly = false);

    /** Hidden copy constructor, as it is a singleton. */
    LocalDatabase(const LocalDatabase&);

    /** Read IoVs of payloads from database file.
     * @return   True if the database could be successfully read. */
    bool readDatabase();

    /**
     * If a default payload exists return it with an infinite IoV.
     * Otherwise return a null pointer if empty IoV
     *
     * @param name       Name that identifies the object in the database.
     */
    bool tryDefault(DBQuery& query);

    /** Write IoVs of payloads to database file.
     * @return   True if the database could be successfully written. */
    bool writeDatabase();

    /** The database file name as given (will be displayed in case of parsing errors */
    std::string m_fileName;

    /** The absolute database file name. */
    std::string m_absFileName;

    /** The directory of payloads. */
    std::string m_payloadDir;

    /** flag for read-only mode. */
    bool m_readOnly;

    /** Map of payload names to vector of revisions and assigned IoVs. */
    std::map<std::string, std::vector<std::pair<int, IntervalOfValidity>>> m_database;
  };
} // namespace Belle2
