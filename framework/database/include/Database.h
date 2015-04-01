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
   */
  class Database {
  public:

    /**
     * Instance of a singleton Database.
     */
    static Database& Instance();

    /**
     * Instance of a singleton Database.
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
      DBQuery(std::string aName): name(aName), object(0) {};
      std::string        name;   /**< Identifier of the object */
      TObject*           object; /**< Pointer to the object */
      IntervalOfValidity iov;    /**< Interval of validity of the object */
    };

    /**
     * Request multiple objects from the database.
     *
     * @param event      The metadata of the event for which the objects should be valid.
     * @param names      Names that identify the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    void getData(const EventMetaData& event, std::list<DBQuery>& query);

    /**
     * Exposes setGlobalTag function of the Database class to Python.
     */
    static void exposePythonAPI();

  private:
    /** Hidden constructor, as it is a singleton.*/
    explicit Database();

    /** same for copy constructor */
    Database(const Database&);

    /** and destructor. */
    ~Database();

    /** The global tag. Currently used for the database file name. */
    std::string m_globalTag;

    /** A root file that contains the database content. */
    TFile* m_dbFile;
  };
} // namespace Belle2
