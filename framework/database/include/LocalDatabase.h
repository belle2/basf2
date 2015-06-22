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

class TFile;


namespace Belle2 {
  /**
   * Implentation of a database backend that uses a root file for the data
   * storage and is not optimized. The implementation should only be used
   * for test purposes.
   */
  class LocalDatabase: public Database {
  public:

    /**
     * Method to set the database instance to a local database.
     */
    static void createInstance();

    /**
     * Request an object from the database.
     *
     * @param event      The metadata of the event for which the object should be valid.
     * @param name       Name that identifies the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    virtual std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& name);

    /**
     * Store an object in the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& name, TObject* object, IntervalOfValidity& iov);

  private:
    /** Hidden constructor, as it is a singleton. */
    explicit LocalDatabase();

    /** Hidden copy constructor, as it is a singleton. */
    LocalDatabase(const LocalDatabase&);

    /** Hidden destructor, as it is a singleton. */
    ~LocalDatabase();

    /** Connect to the database. Currently this mean opening the database file.
     * @return   True if the connection to the databse could be successfully established. */
    bool connectDatabase();

    /** A root file that contains the database content. */
    TFile* m_dbFile;
  };
} // namespace Belle2
