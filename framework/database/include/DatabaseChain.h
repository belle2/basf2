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
#include <framework/database/IntervalOfValidity.h>

#include <string>
#include <vector>


namespace Belle2 {
  /**
   * Implentation of a database backend that uses a chain of other
   * database backends to obtain the payloads.
   */
  class DatabaseChain: public Database {
  public:

    /**
     * Method to set the database instance to a local database.
     *
     * @param resetIoVs   A flag to indicate whether IoVs from non-primary databases should be set to the current run
     * @param logLevel    The level of log messages about not-found payloads.
     * @return            A pointer to the created database instance
     */
    static void createInstance(bool resetIoVs = false, LogConfig::ELogLevel logLevel = LogConfig::c_Warning);

    /**
     * Add a database backend.
     *
     * @param database      The backend instance.
     */
    void addDatabase(Database* database);

    /**
     * Request an object from the database.
     *
     * @param event      The metadata of the event for which the object should be valid.
     * @param name       Name that identifies the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    virtual std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& package,
                                                            const std::string& module);

    /**
     * Store an object in the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& package, const std::string& module, TObject* object, IntervalOfValidity& iov);

  private:
    /** Hidden constructor, as it is a singleton. */
    explicit DatabaseChain(bool resetIoVs) : m_resetIoVs(resetIoVs) {};

    /** Hidden copy constructor, as it is a singleton. */
    DatabaseChain(const DatabaseChain&);

    /** Hidden destructor, as it is a singleton. */
    ~DatabaseChain();

    /** The database file name. */
    std::vector<Database*> m_databases;

    /** Flag whether the IoVs obtained from non-primary databases should be reset to a single run. */
    bool m_resetIoVs;
  };
} // namespace Belle2
