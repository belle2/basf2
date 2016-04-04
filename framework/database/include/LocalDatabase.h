/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
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
     * @param payloadDir The name of the directory in which the payloads are atored. By default the same directory as the one containing the database text file is used.
     * @param readOnly   If this flag is set the database is opened in read only mode without locking.
     * @param logLevel   The level of log messages about not-found payloads.
     * @return           A pointer to the created database instance
     */
    static void createInstance(const std::string& fileName = "database.txt", const std::string& payloadDir = "",
                               bool readOnly = false, LogConfig::ELogLevel logLevel = LogConfig::c_Warning);

    /**
     * Request an object from the database.
     *
     * @param event      The metadata of the event for which the object should be valid.
     * @param package    Name of the package that identifies the object in the database.
     * @param module     Name of the module that identifies the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    virtual std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& package,
                                                            const std::string& module);

    /**
     * Store an object in the database.
     *
     * @param package    Name of the package that identifies the object in the database.
     * @param module     Name of the module that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& package, const std::string& module, TObject* object, IntervalOfValidity& iov);

    /**
     * Add a payload file to the database.
     *
     * @param package    Name of the package that identifies the object in the database.
     * @param module     Name of the module that identifies the object in the database.
     * @param fileName   The name of the payload file.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool addPayload(const std::string& package, const std::string& module, const std::string& fileName,
                            IntervalOfValidity& iov);

  private:
    /**
     * Hidden constructor, as it is a singleton.
     *
     * @param fileName   The name of the database text file with the IoV assignments.
     * @param payloadDir The name of the directory in which the payloads are atored. By default the same directory as the one containing the database text file is used.
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
     * @param package    Name of the package that identifies the object in the database.
     * @param module     Name of the module that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    std::pair<TObject*, IntervalOfValidity> tryDefault(const std::string& package, const std::string& module);

    /** Write IoVs of payloads to database file.
     * @return   True if the database could be successfully written. */
    bool writeDatabase();

    /** The database file name. */
    std::string m_fileName;

    /** The directory of payloads. */
    std::string m_payloadDir;

    /** flag for read-only mode. */
    bool m_readOnly;

    /** Map of packages to map of modules to vector of revisions and assigned IoVs. */
    std::map<std::string, std::map<std::string, std::vector<std::pair<int, IntervalOfValidity>>>> m_database;
  };
} // namespace Belle2
