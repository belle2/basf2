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

#include <framework/database/IntervalOfValidity.h>

#include <string>
#include <utility>
#include <list>

class TObject;
class TFile;


namespace Belle2 {
  class EventMetaData;

  /**
   * Singleton class for the low-level interface to the database.
   * To be used by the DBStore.
   * The current implentation uses a root file for the data storage and is not
   * optimized. The implementation will be replaced by the actual conditions
   * database backend.
   * Currently the Database instance keeps the ownership of the objects, but
   * this may change for the final backend.
   */
  class Database {
  public:

    /**
     * Instance of a singleton Database.
     */
    static Database& Instance();

    /**
     * Function to set the global tag.
     *
     * Currently the global tag is the file name of the database file.
     * The tag has to be set before any read or write access to the database.
     *
     * @param tag  The global tag identifier
     */
    static void setGlobalTag(std::string tag) {Instance().m_globalTag = tag;};

    /**
     * Request an object from the database.
     *
     * @param event      The metadata of the event for which the object should be valid.
     * @param name       Name that identifies the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& name);

    /**
     * Struct for bulk queries.
     */
    struct DBQuery {
      /**
       * Constructor
       * @param aName  The identifier of the object
       */
      DBQuery(std::string aName, TObject* aObject = 0, IntervalOfValidity aIov = 0): name(aName), object(aObject), iov(aIov) {};
      std::string        name;   /**< Identifier of the object */
      TObject*           object; /**< Pointer to the object */
      IntervalOfValidity iov;    /**< Interval of validity of the object */
    };

    /**
     * Request multiple objects from the database.
     *
     * @param event      The metadata of the event for which the objects should be valid.
     * @param query      A list of DBQuery entries that contains the names of the objects to be retrieved. On return the object and iov fields are filled.
     */
    void getData(const EventMetaData& event, std::list<DBQuery>& query);

    /**
     * Store an object in the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    bool storeData(const std::string& name, TObject* object, IntervalOfValidity& iov);

    /**
     * Store multiple objects in the database.
     *
     * @param query      A list of DBQuery entries that contains the objects, their names, and their intervals of validity.
     * @return           True if the storage of the object succeeded.
     */
    bool storeData(std::list<DBQuery>& query);

    /**
     * Exposes setGlobalTag function of the Database class to Python.
     */
    static void exposePythonAPI();

  private:
    /** Hidden constructor, as it is a singleton. */
    explicit Database();

    /** Hidden copy constructor, as it is a singleton. */
    Database(const Database&);

    /** Hidden destructor, as it is a singleton. */
    ~Database();

    /** Connect to the database. Currently this mean opening the database file.
     * @return   True if the connection to the databse could be successfully established. */
    bool connectDatabase();

    /** The global tag. Currently used for the database file name. */
    std::string m_globalTag;

    /** A root file that contains the database content. */
    TFile* m_dbFile;
  };
} // namespace Belle2
