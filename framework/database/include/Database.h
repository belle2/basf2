/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/PayloadMetadata.h>
#include <framework/database/TestingPayloadStorage.h>
#include <framework/database/DBStore.h>
#include <framework/utilities/ScopeGuard.h>
#include <TClonesArray.h>

#include <string>
#include <utility>
#include <list>
#include <memory>

class TObject;


namespace Belle2 {
  class EventMetaData;
  namespace Conditions {
    class MetadataProvider;
    class PayloadProvider;
  }

  /**
   * Singleton base class for the low-level interface to the database.
   * To be used by the DBStore.
   * Currently the Database instance keeps the ownership of the objects, but
   * this may change for the final backend.
   */
  class Database {
  public:
    /** State of the database */
    enum EDatabaseState {
      /** Before any initialization */
      c_PreInit = 0,
      /** Globaltag list has been finalized metadata providers not and globaltags are not checked for usability */
      c_InitGlobaltagList = 1,
      /** Everything is ready */
      c_Ready = 2,
    };

    /**
     * Instance of a singleton Database.
     */
    static Database& Instance();

    /**
     * Reset the database instance.
     *
     * @param keepConfig if true the configuration object itself is not
     *   resetted so that initializing the database again will result in the
     *   same configuration as before. This is useful when running process
     *   multiple times.
     */
    static void reset(bool keepConfig = false);

    /**
     * Struct for bulk read queries.
     */
    using DBQuery = Conditions::PayloadMetadata;

    /**
     * Struct for bulk write queries.
     */
    struct DBImportQuery {
      /**
       * Constructor
       * @param aName The identifier of the object
       * @param aObject Pointer to the object
       * @param aIov Iov of the object
       */
      explicit DBImportQuery(const std::string& aName, TObject* aObject = nullptr,
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
    bool getData(const EventMetaData& event, DBQuery& query)
    {
      std::vector<DBQuery> container{query};
      bool found = getData(event, container);
      query = container[0];
      return found;
    }

    /**
     * Request multiple objects from the database.
     *
     * @param event      The metadata of the event for which the objects should be valid.
     * @param query      A list of DBQuery entries that contains the names of
     *                   the objects to be retrieved. On return the object and
     *                   iov fields are filled.
     */
    bool getData(const EventMetaData& event, std::vector<DBQuery>& query);

    /**
     * Convenience function to get an object for an arbitrary experiment and run.
     *
     * @param[in] name       The name identifying the object.
     * @param[in] experiment The experiment number.
     * @param[in] run        The run number.
     */
    const TObject* getData(const std::string& name, int experiment, int run)
    {
      EventMetaData eventMetaData(1, run, experiment);
      return getData(eventMetaData, name).first;
    }


    /**
     * Store an object in the database.
     *
     * @param name   Name that identifies the object in the database.
     * @param object The object that should be stored in the database.
     * @param iov    The interval of validity of the the object.
     * @return       True if the storage of the object succeeded.
     */
    bool storeData(const std::string& name, TObject* object, const IntervalOfValidity& iov);

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
    bool storeData(std::list<DBImportQuery>& query);

    /**
     * Add a payload file to the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param fileName   The name of the payload file.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    bool addPayload(const std::string& name, const std::string& fileName, const IntervalOfValidity& iov)
    {
      if (!m_payloadCreation) initialize();
      return m_payloadCreation->storePayload(name, fileName, iov);
    }

    /**
     * Exposes setGlobalTag function of the Database class to Python.
     */
    static void exposePythonAPI();

    /**
     * Return the global tags used by the database. If no conditions database is
     * configured return an empty string. If more then one database is
     * configured return all global tags concatenated by ','
     */
    std::string getGlobalTags();

    /**
     * Make sure we have efficient http pipelinging during initialize/beginRun
     * but don't keep session alive for full processing time. This will return
     * an object which keeps the session open as long as it is alive.
     */
    ScopeGuard createScopedUpdateSession();

    /** Initialize the database connection settings on first use */
    void initialize(const EDatabaseState target = c_Ready);

  protected:
    /** Hidden constructor, as it is a singleton. */
    Database() = default;
    /** No copy constructor, as it is a singleton. */
    Database(const Database&) = delete;
    /** Hidden destructor, as it is a singleton. */
    ~Database();
    /** Enable the next metadataprovider in the list */
    void nextMetadataProvider();
    /** List of available metadata providers (which haven't been tried yet) */
    std::vector<std::string> m_metadataConfigurations;
    /** List of globaltags to be used */
    std::vector<std::string> m_globalTags;
    /** Set of usable globaltag states to be handed to the metadata providers */
    std::set<std::string> m_usableTagStates;
    /** Currently active metadata provider */
    std::unique_ptr<Conditions::MetadataProvider> m_metadataProvider;
    /** The active payload provider */
    std::unique_ptr<Conditions::PayloadProvider> m_payloadProvider;
    /** testing payload storage to create new payloads */
    std::unique_ptr<Conditions::TestingPayloadStorage> m_payloadCreation;
    /** optional list of testing payload storages to look for existing payloads */
    std::vector<Conditions::TestingPayloadStorage> m_testingPayloads;
    /** Current configuration state of the database */
    EDatabaseState m_configState{c_PreInit};
  };
} // namespace Belle2
