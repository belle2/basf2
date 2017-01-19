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
#include <framework/database/DBStore.h>
#include <framework/logging/LogConfig.h>

#include <TClonesArray.h>

#include <string>
#include <utility>
#include <list>

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
     * Request an object from the database.
     *
     * @param event      The metadata of the event for which the object should be valid.
     * @param package    Name of the package that identifies the object in the database.
     * @param module     Name of the module that identifies the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    virtual std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& package,
                                                            const std::string& module) = 0;

    /**
     * Struct for bulk queries.
     */
    struct DBQuery {
      /**
       * Constructor
       * @param aName  The identifier of the object
       */
      DBQuery(const std::string& aPackage, const std::string& aModule, TObject* aObject = 0,
              IntervalOfValidity aIov = 0): package(aPackage), module(aModule), object(aObject), iov(aIov) {};
      std::string        package; /**< First part of identifier of the object */
      std::string        module; /**< Second part of identifier of the object */
      TObject*           object; /**< Pointer to the object */
      IntervalOfValidity iov;    /**< Interval of validity of the object */
    };

    /**
     * Request multiple objects from the database.
     *
     * @param event      The metadata of the event for which the objects should be valid.
     * @param query      A list of DBQuery entries that contains the names of the objects to be retrieved. On return the object and iov fields are filled.
     */
    virtual void getData(const EventMetaData& event, std::list<DBQuery>& query);

    /**
     * Store an object in the database.
     *
     * @param package    Name of the package that identifies the object in the database.
     * @param module     Name of the module that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& package, const std::string& module, TObject* object, const IntervalOfValidity& iov) = 0;

    /**
     * Store an object in the database with the default package name "dbstore".
     *
     * @param module     Name of the module that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    bool storeData(const std::string& module, TObject* object, const IntervalOfValidity& iov) {return storeData("dbstore", module, object, iov);};

    /**
     * Store an ClonesArray in the database with the default package name "dbstore" and default module name.
     *
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    bool storeData(TClonesArray* array, const IntervalOfValidity& iov) {return storeData("dbstore", DataStore::defaultArrayName(array->ClassName()), array, iov);};

    /**
     * Store an object in the database with the default package name "dbstore" and default module name.
     *
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    template<class T> bool storeData(T* object, const IntervalOfValidity& iov) {return storeData("dbstore", DBStore::objectName<T>(""), object, iov);};

    /**
     * Store multiple objects in the database.
     *
     * @param query      A list of DBQuery entries that contains the objects, their names, and their intervals of validity.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(std::list<DBQuery>& query);

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
                            const IntervalOfValidity& iov) = 0;

    /**
     * Add a payload file to the database.
     *
     * @param module     Name of the module that identifies the object in the database.
     * @param fileName   The name of the payload file.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    bool addPayload(const std::string& module, const std::string& fileName, const IntervalOfValidity& iov) {return addPayload("dbstore", module, fileName, iov);};

    /**
     * Exposes setGlobalTag function of the Database class to Python.
     */
    static void exposePythonAPI();

    /**
     * Return the global tag used by the database. If no conditions database is
     * configured return an empty string. If more then once database is
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

    /** Hidden copy constructor, as it is a singleton. */
    Database(const Database&) : m_logLevel(LogConfig::c_Warning) {};

    /** Helper function to construct a payload file name. */
    std::string payloadFileName(const std::string& path, const std::string& package, const std::string& module, int revision) const;

    /** Helper function to read an object from a payload file. */
    TObject* readPayload(const std::string& fileName, const std::string& module) const;

    /** Helper function to write an object to a payload file. */
    bool writePayload(const std::string& fileName, const std::string& module, const TObject* object,
                      const IntervalOfValidity* iov = 0) const;

    /** Level of log messages about not found objects. */
    LogConfig::ELogLevel m_logLevel;

    bool m_invertLogging{false};
  };
} // namespace Belle2
