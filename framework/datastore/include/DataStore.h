/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr, Martin Ritter,                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DATASTORE_H
#define DATASTORE_H

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TClonesArray.h>
#include <TClass.h>

#include <string>


namespace Belle2 {
  /** In the store you can park objects, that have to be accessed by various modules.
   *
   *  The store saves objects together with names and some flags in maps.
   *  Normal users should try to access the store via StoreAccessor classes like the
   *  StoreObjPtr or the StoreArray. <br>
   *  Currently the store supports either the storage of single objects, that inherit from TObject,
   *  or TClonesArrays, which are faster, if you have to store a large number of objects from the same type.
   *  Besides that, you have to chose the durability of the things you want to store. <br>
   *  Currently you can chose between lifetimes of event, run, and persistent.
   *  basf2 deletes the objects from the store according to the durability map in which the objects are stored.
   *
   *  @sa EDurability StoreObjPtr StoreArray
   *  @author <a href="mailto:belle2_software@bpost.kek.jp?subject=DataStore">The basf2 developers</a>
   */
  class DataStore {
  public:
    //----------------------------------- enums and typedefs ---------------------------------------------------
    /** Durability types.
     *
     *  These types are used to identify the map used in the DataStore.
     */
    enum EDurability {
      c_Event,     /**< Object is deleted after event. */
      c_Run,       /**< Object is deleted after run. */
      c_Persistent /**< Object is persistent. */
    };

    /** Number of Durability Types.
     *
     *  Probably useless, but in principle additional maps are easily created this way.
     */
    enum ENDurabilityTypes {
      c_NDurabilityTypes = 3 /**< Total number of durability types. */
    };

    /** A struct for map entries **/
    struct StoreEntry {
      StoreEntry() : isArray(false), isTransient(false), object(0), ptr(0), name("") {};
      bool        isArray;     /**< Flag that indicates whether the object is a TClonesArray **/
      bool        isTransient; /**< Flag that indicates whether the object should be written to the output by default **/
      TObject*    object;      /**< The pointer to the actual object **/
      TObject*    ptr;         /**< The pointer to the returned object, either equal to 'object' or 0 **/
      std::string name;        /**< Name of the entry. Equal to the key in the map. **/
    };

    // Convenient typedefs.
    typedef std::map<std::string, StoreEntry*> StoreObjMap;  /**< Map for StoreEntries. */
    typedef StoreObjMap::iterator StoreObjIter;              /**< Iterator for a StoreEntry map. */
    typedef StoreObjMap::const_iterator StoreObjConstIter;   /**< const_iterator for a StoreEntry map. */

    //--------------------------------- Instance ---------------------------------------------------------------
    /** Instance of singleton Store.
     *
     *  This method is used to access the DataStore directly. It is used internally in the Store accessor classes.
     */
    static DataStore& Instance();

    //--------------------------------- default name stuff -----------------------------------------------------
    /** Return the default storage name for an object of the given type. */
    template<class T> static const std::string defaultObjectName() {
      std::string classname = (T::Class()->GetName());
      //Strip qualifiers like namespaces
      size_t colon = classname.rfind("::");
      if (colon != std::string::npos) {
        classname = classname.substr(colon + 2);
      }
      return classname;
    }

    /** Return the storage name for an object of the given type and name. */
    template<class T> static const std::string objectName(const std::string& name) {
      return ((name == "") ? defaultObjectName<T>() : name);
    }

    /** Return the default storage name for an array of the given type. */
    template<class T> static const std::string defaultArrayName() {
      return defaultObjectName<T>() + 's';
    }

    /** Return the storage name for an object of the given type and name. */
    template<class T> static const std::string arrayName(const std::string& name) {
      return ((name == "") ? defaultArrayName<T>() : name);
    }

    /** Return the default storage name for a relation between the given types. */
    template<class FROM, class TO> static const std::string defaultRelationName() {
      return defaultArrayName<FROM>() + "To" + defaultArrayName<TO>();
    }

    /** Return the storage name for a relation with given name between the given types. */
    template<class FROM, class TO> static const std::string relationName(const std::string& name) {
      return ((name == "") ? defaultArrayName<FROM, TO>() : name);
    }

    /** Return storage name for a relation between two arrays of the given names. */
    static std::string relationName(const std::string& fromName, const std::string& toName) {
      return fromName + "To" + toName;
    }

    //------------------------------ Accessing objects and arrays ----------------------------------------------
    /** Create an entry in the DataStore map.
     *
     *  If the map of requested durability already contains an object under the key name with a DIFFERENT type
     *  than the given type one, an error will be reported. <br>
     *  Otherwise a new map slot is created.
     *  This must be called in the initialization phase. Otherwise an error is returned.
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @param transient  Whether the object should be stored to the output by default.
     *  @param errorIfExisting  Whether to complain if the entry alreay exists.
     *  @return           True if the registration succeeded.
     */
    bool createEntry(const std::string& name, EDurability durability,
                     const TClass* objClass, bool array, bool transient, bool errorIfExisting);

    /** Check whether an entry with the correct type is registered in the DataStore map.
     *
     *  If the map of requested durability already contains an object under the key name with a DIFFERENT type
     *  than the given type one, an error will be reported. <br>
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @return           True if the requested object exists.
     */
    bool hasEntry(const std::string& name, EDurability durability,
                  const TClass* objClass, bool array);

    /** Get a pointer to a pointer of an object in the DataStore.
     *
     *  If the map of requested durability already contains an object under the key name with a DIFFERENT type
     *  than the given type one, an error will be reported. <br>
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @return           Pointer to pointer to object, NULL if none exists
     */
    TObject** getObject(const std::string& name, EDurability durability,
                        const TClass* objClass, bool array);

    /** Create a new object in the DataStore or add an existing one.
     *
     *  A matching map entry must already exist. Otherwise an error will be generated.
     *  @param object     Pointer to the object that should be stored. If 0, a new default object is created.
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @return           Pointer to pointer to created object.
     */
    bool createObject(TObject* object, bool replace, const std::string& name, EDurability durability,
                      const TClass* objClass, bool array);

    /** Get a reference to the object/array map. */
    const StoreObjMap& getStoreObjectMap(EDurability durability) { return m_storeObjMap[durability]; }


    //------------------------------ Start and end procedures --------------------------------------------------
    /** Setter for m_initializeActive. */
    void setInitializeActive(bool active) {
      m_initializeActive = active;
    }

    /** Clears maps of a specified durability.
     *
     *  Called by the framework. Users should usually not use this function without a good reason.
     *
     *  Memory occupied by objects/arrays is only freed once a new object is created in its place (in createObject()).
     *
     *  @param durability Decides which map is cleared.
     */
    void clearMaps(EDurability durability = c_Event);

    /** FIXME: Temporary solution to provide backward compatibility for StoreObjPtr and StoreArray */
    void backwardCompatibleRegistration(const std::string& name, EDurability durability,
                                        const TClass* objClass, bool array);
    void backwardCompatibleCreation(const std::string& name, EDurability durability,
                                    const TClass* objClass, bool array);


  protected:
    /** Constructor is protected, as it is a singleton.*/
    explicit DataStore();

  private:
    /** Destructor. */
    ~DataStore() {};

    /** Check whether the given entry and the requested class match.
     *
     *  @param name       Name of the DataStore map entry.
     *  @param entry      The existing DataSotre entry.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @return           True if both types match.
     */
    bool checkType(const std::string& name, const StoreEntry* entry,
                   const TClass* objClass, bool array) const;

    static DataStore* m_instance; /**< Pointer, that actually holds the store instance.*/

    /** Map for all objects/arrays in the data store.
     *
     * They map the name to a TObject pointer, separated by durability.
     */
    StoreObjMap m_storeObjMap[c_NDurabilityTypes];

    /** True if modules are currently being initialized.
     *
     * Creating new map slots is only allowed in a Module's initialize() function.
     */
    bool m_initializeActive;
  };
} // namespace Belle2

#endif // DATASTORE_H
