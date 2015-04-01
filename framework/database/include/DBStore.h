/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/DBEntry.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <string>
#include <map>


namespace Belle2 {

  /**
   * Singleton class to cache database objects
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
     * Return the default database name for given class name.
     */
    static std::string defaultObjectName(std::string classname);

    /**
     * Return the default database name for an object of the given type.
     */
    template<class T> static std::string defaultObjectName()
    {
      const static std::string s = defaultObjectName(T::Class_Name());
      return s;
    }

    /**
     * Return the database name for an object of the given type and name.
     */
    template<class T> static std::string objectName(const std::string& name)
    {
      return ((name.empty()) ? defaultObjectName<T>() : name);
    }

    /**
     * Return the default database name for an array given class name.
     */
    static std::string defaultArrayName(const std::string& classname)
    {
      return defaultObjectName(classname) + 's';
    }

    /**
     * Return the default database name for an array of the given type.
     */
    template<class T> static std::string defaultArrayName()
    {
      const static std::string s = defaultObjectName<T>() + 's';
      return s;
    }

    /**
     * Return the database name for an object of the given type and name.
     */
    template<class T> static std::string arrayName(const std::string& name)
    {
      return ((name.empty()) ? defaultArrayName<T>() : name);
    }

    /** Destructor. */
    ~DBStore();

    /**
     * Returns the entry with the requested name in the DBStore.
     * If the DBStore entry does not exist yet it is added to the map.
     *
     * If the DBStore map already contains an object under the key name
     * with a DIFFERENT type than the given type one, an error will be reported. <br>
     *
     * @param name       Name under which the object is stored in the database (and in the DBStore).
     * @param objClass   The class of the object.
     * @param array      Whether it is a TClonesArray or not.
     * @return           DBEntry, or NULL if the requested type does not match the one in the DBStore
     */
    DBEntry* getEntry(const std::string& name, const TClass* objClass, bool array);

    /**
     * Updates all objects that are outside their interval of validity.
     * This method is called by the framework for each new run.
     */
    void update();


  private:
    /** Hidden constructor, as it is a singleton.*/
    explicit DBStore() {};

    /** same for copy constructor */
    DBStore(const DBStore&);

    /**
     * Check whether the given entry and the requested class match.
     *
     * @param entry      The existing DBStore entry.
     * @param objClass   The class of the object.
     * @param array      Whether it is a TClonesArray or not.
     * @return           True if both types match.
     */
    bool checkType(const DBEntry& dbEntry, const TClass* objClass, bool array) const;

    /**
     * Check whether the given entry and the type of the object match.
     *
     * @param entry      The existing DBStore entry.
     * @param object     The object whose type is to be checked.
     * @return           True if both types match.
     */
    bool checkType(const DBEntry& dbEntry, const TObject* object) const;

    /** Map of names to DBEntry objects. */
    std::map<std::string, DBEntry> m_dbEntries;

    /**
     * StoreObjPtr for the EventMetaData to get the current experiment and run
     */
    StoreObjPtr<EventMetaData> m_event;
  };
} // namespace Belle2
