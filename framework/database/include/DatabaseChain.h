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
     * @param resetIoVs A flag to indicate whether IoVs from non-primary databases should be set to the current run
     * @param logLevel The level of log messages about not-found payloads.
     * @param invertLogging If true log messages will be created when a
     *   payload is found. This is intended for the local database to notify
     *   the user that a non-standard payload from a local directory is used.
     */
    static void createInstance(bool resetIoVs = false, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                               bool invertLogging = false);

    /**
     * Add a database backend. It is added before any existing backends.
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
    virtual bool getData(const EventMetaData& event, DBQuery& query) override;

    /**
     * Store an object in the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& name, TObject* object, const IntervalOfValidity& iov) override;

    /**
     * Add a payload file to the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param fileName   The name of the payload file.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool addPayload(const std::string& name, const std::string& fileName,
                            const IntervalOfValidity& iov) override;

    /** Return the list of registered databases. */
    const std::vector<Database*>& getDatabases() const { return m_databases; }

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
