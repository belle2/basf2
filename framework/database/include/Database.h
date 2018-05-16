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

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBStore.h>
#include <framework/logging/LogConfig.h>

#include <TClonesArray.h>

#include <string>
#include <utility>
#include <list>
#include <memory>

class TObject;


namespace Belle2 {
  class EventMetaData;

  /**
   * Singleton base class for the low-level interface to the database.
   * To be used by the DBStore.
   * Currently the Database instance keeps the ownership of the objects, but
   * this may change for the final backend.
   */
  class Database {
  public:

    /**
     * Get the default global tags for the central database.
     * Multiple global tags are separated by spaces.
     *
     * @return           The default global tags
     */
    static std::string getDefaultGlobalTags();

    /**
     * Instance of a singleton Database.
     */
    static Database& Instance();

    /**
     * Helper function to set the database instance.
     * If the current instance is a DatabaseChain the given database is added to it.
     *
     * @param database   The database object.
     */
    static void setInstance(Database* database);

    /**
     * Reset the database instance.
     */
    static void reset();

    /** Destructor, resets the instance pointer if needed. */
    virtual ~Database();

    /**
     * Struct for bulk read queries.
     */
    struct DBQuery {
      /**
       * Constructor
       */
      DBQuery(const std::string& aName, bool required = true): name(aName), missingOK(required) {};
      std::string name;         /**< identifier of the object */
      unsigned int revision{0}; /**< revision of the payload */
      std::string filename{};   /**< filename containing the payload */
      std::string checksum{};   /**< checksum of the filename containing the payload */
      IntervalOfValidity iov{}; /**< Interval of validity of the object */
      bool missingOK{false};    /**< Input parameter: if True don't emit an error if the payload is missing */
    };


    /**
     * Struct for bulk write queries.
     */
    struct DBImportQuery {
      /**
       * Constructor
       * @param aName  The identifier of the object
       * @param aObject Pointer to the object
       * @param aIov Iov of the object
       */
      DBImportQuery(const std::string& aName, TObject* aObject = 0,
                    const IntervalOfValidity& aIov = IntervalOfValidity()): name(aName), object(aObject), iov(aIov) {};
      std::string        name;   /**< identifier of the object */
      TObject*           object; /**< Pointer to the object */
      IntervalOfValidity iov;    /**< Interval of validity of the object */
    };

    /**
     * Request an object from the database.
     *
     * @param event   The metadata of the event for which the object should be valid.
     * @param name    Name that identifies the object in the database.
     * @return        A pair containing the object and the iov for which it is valid.
     *                Ownership will be given to the caller
     *
     * @warning The returned object has to be deleted by the caller
     */
    std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& name);


    /** Request an object from the database.
     * @param event   The metadata of the event for which the object should be valid.
     * @param query   Object containing the necessary identification which will
     *                be filled with all information about the payload.
     * @return        True if the payload could be found. False otherwise.
     */
    virtual bool getData(const EventMetaData& event, DBQuery& query) = 0;

    /**
     * Request multiple objects from the database.
     *
     * @param event      The metadata of the event for which the objects should be valid.
     * @param query      A list of DBQuery entries that contains the names of
     *                   the objects to be retrieved. On return the object and
     *                   iov fields are filled.
     */
    virtual void getData(const EventMetaData& event, std::list<DBQuery>& query);

    /**
     * Store an object in the database.
     *
     * @param name   Name that identifies the object in the database.
     * @param object The object that should be stored in the database.
     * @param iov    The interval of validity of the the object.
     * @return       True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& name, TObject* object, const IntervalOfValidity& iov) = 0;

    /**
     * Store an ClonesArray in the database with the default name.
     *
     * @param array      The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    bool storeData(TClonesArray* array, const IntervalOfValidity& iov) {return storeData(DataStore::defaultArrayName(array->ClassName()), array, iov);};

    /**
     * Store an object in the database with the default name.
     *
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    template<class T> bool storeData(T* object, const IntervalOfValidity& iov) {return storeData(DBStore::objectName<T>(""), object, iov);};

    /**
     * Store multiple objects in the database.
     *
     * @param query      A list of DBImportQuery entries that contains the objects, their names, and their intervals of validity.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(std::list<DBImportQuery>& query);

    /**
     * Add a payload file to the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param fileName   The name of the payload file.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool addPayload(const std::string& name, const std::string& fileName,
                            const IntervalOfValidity& iov) = 0;

    /**
     * Exposes setGlobalTag function of the Database class to Python.
     */
    static void exposePythonAPI();

    /**
     * Return the global tag used by the database. If no conditions database is
     * configured return an empty string. If more then one database is
     * configured return all global tags concatenated by ','
     */
    static std::string getGlobalTag();

    /**
     * Set level of log messages about not-found payloads.
     *
     * @param logLevel  The level of log messages about not-found payloads.
     * @param invertLogging  If true log messages will be created when a
     *                  payload is found. This is intended for the local
     *                  database to notify the user that a non-standard payload
     *                  from a local directory is used.
     */
    void setLogLevel(LogConfig::ELogLevel logLevel = LogConfig::c_Warning, bool invertLogging = false)
    {
      m_logLevel = logLevel;
      m_invertLogging = invertLogging;
    }

  protected:
    /** Pointer to the database instance. */
    static std::unique_ptr<Database> s_instance;

    /** Hidden constructor, as it is a singleton. */
    Database() : m_logLevel(LogConfig::c_Warning) {};

    /** No copy constructor, as it is a singleton. */
    Database(const Database&) = delete;

    /** Helper function to construct a payload file name. */
    std::string payloadFileName(const std::string& path, const std::string& name, int revision) const;

    /** Helper function to write an object to a payload file. */
    bool writePayload(const std::string& fileName, const std::string& name, const TObject* object,
                      const IntervalOfValidity* iov = 0) const;

    /** Level of log messages about not found objects. */
    LogConfig::ELogLevel m_logLevel;

    /** If true logging should be inverted: i.e. show messages if a payload was found, not if it wasn't */
    bool m_invertLogging{false};
  };
} // namespace Belle2
