/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBStoreEntry.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>
#include <unordered_map>
#include <set>

namespace Belle2 {

  /**
   * Singleton class to cache database objects.
   *
   * The DBStore is the owner of the objects retrieved from the database.
   */
  class DBStore {
  public:

    /**
     * Global flag to decide if we can do normal cleanup.
     *
     * If the normal basf2 execution is bypassed, e.g. in tests or separate binaries,
     * we cannot guarantee that exit handlers are called in the correct order. Since
     * this may lead to double deletion of objects through ROOT and misleading error
     * stack traces, we will skip cleanup in these exceptional cases.
     */
    static bool s_DoCleanup;

    /**
     * Instance of a singleton DBStore.
     *
     * This method is used to access the DBStore directly.
     * It is used internally in the DBStore accessor classes.
     */
    static DBStore& Instance();

    /**
     * Return the database name for an object of the given type and name.
     */
    template<class T> static std::string objectName(const std::string& name) { return DataStore::objectName<T>(name); }


    /**
     * Return the database name for an object of the given type and name.
     */
    template<class T> static std::string arrayName(const std::string& name) { return DataStore::arrayName<T>(name); }

    /** Destructor. */
    ~DBStore();

    /**
     * Returns the entry with the requested name in the DBStore.
     * If the DBStore entry does not exist yet it is added to the map.
     *
     * If the DBStore map already contains an object under the key
     * with a DIFFERENT type than the given type one, an error will be reported. <br>
     *
     * @param payloadType Type of the payload. Usually c_Object.
     * @param name       Name under which the object is stored in the database (and in the DBStore).
     * @param objClass   The class of the object.
     * @param array      Whether it is a TClonesArray or not.
     * @param required   If true emit errors if the object cannot be found
     * @return           DBEntry, or nullptr if the requested type does not match the one in the DBStore
     */

    DBStoreEntry* getEntry(DBStoreEntry::EPayloadType payloadType, const std::string& name,
                           const TClass* objClass, bool array, bool required = true);

    /**
     * Returns the entry with the requested name in the DBStore.
     * If the DBStore entry does not exist yet it is added to the map.
     *
     * If the DBStore map already contains an object under the key
     * with a DIFFERENT type than the given type one, an error will be reported. <br>
     *
     * @param name       Name under which the object is stored in the database (and in the DBStore).
     * @param objClass   The class of the object.
     * @param array      Whether it is a TClonesArray or not.
     * @param required   If true emit errors if the object cannot be found
     * @return           DBEntry, or NULL if the requested type does not match the one in the DBStore
     */
    DBStoreEntry* getEntry(const std::string& name, const TClass* objClass,
                           bool array, bool required = true)
    {
      return getEntry(DBStoreEntry::c_Object, name, objClass, array, required);
    }

    /**
     * Updates all objects that are outside their interval of validity.
     * This method is called by the framework for each new run.
     */
    void update();

    /**
     * Updates all intra-run dependent objects.
     * This method is called by the framework for each event.
     */
    void updateEvent();

    /**
     * Invalidate all payloads.
     *
     * @param keepEntries  Keep the existing entries so that future calls to
     *    update the database will try to obtain the payloads which were
     *    registered so far.
     */
    void reset(bool keepEntries = false);

    /**
     * Add constant override payload.
     * This payload will be valid for all possible iov and will be used instead of values from the database.
     *
     * @param name Name under which the object will be accessible
     * @param obj Pointer to the object to be used. Ownership will be transfered
     *            to the DBStore.
     * @param oneRun if true the override will only be in effect for this one run,
     *               not for any other runs
     * @warning don't use this if you do not know exactly what you are doing.
     *          This is meant mainly for beamparameters
     */
    void addConstantOverride(const std::string& name, TObject* obj, bool oneRun = false);

  private:
    /** Hidden constructor, as it is a singleton.*/
    explicit DBStore() {};

    /** same for copy constructor */
    DBStore(const DBStore&);

    /** Map names to DBEntry objects. */
    std::unordered_map<std::string, DBStoreEntry> m_dbEntries;

    /** List of intra-run dependent conditions. */
    std::set<DBStoreEntry*> m_intraRunDependencies;

    /**
     * StoreObjPtr for the EventMetaData to get the current experiment and run
     */
    StoreObjPtr<EventMetaData> m_event;
  };
} // namespace Belle2
