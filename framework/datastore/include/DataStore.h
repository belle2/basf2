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

#include <framework/datastore/RelationEntry.h>

#include <string>
#include <set>
#include <map>

class TObject;
class TClass;

namespace Belle2 {
  /** In the store you can park objects, that have to be accessed by various modules.
   *
   *  The store saves objects together with names and some flags in maps.
   *  Normal users should try to access the store via StoreAccessor classes like the
   *  StoreObjPtr or the StoreArray. <br>
   *  Currently the store supports either the storage of single objects (that inherit from TObject)
   *  or TClonesArrays which can store a large number of objects of the same type.
   *  Besides that, you have to chose the durability of the things you want to store. <br>
   *  Currently you can chose between lifetimes of event and persistent.
   *  basf2 deletes the objects from the store according to the durability map in which the objects are stored.
   *
   *  @sa EDurability StoreObjPtr StoreArray RelationArray
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
      c_Persistent /**< Object is available during entire execution time (not related to persistent/transient distinction). */
    };

    /** Number of Durability Types.
     *
     *  Probably useless, but in principle additional maps are easily created this way.
     */
    enum ENDurabilityTypes {
      c_NDurabilityTypes = 2 /**< Total number of durability types. */
    };

    /** A struct for map entries **/
    struct StoreEntry {
      StoreEntry() : isArray(false), isTransient(false), object(0), ptr(0), name("") {};
      bool        isArray;     /**< Flag that indicates whether the object is a TClonesArray **/
      bool        isTransient; /**< Flag that indicates whether the object should be written to the output by default **/
      TObject*    object;      /**< The pointer to the actual object. Associated memory may exceed object durability, and is kept until the object is replaced.  **/
      TObject*    ptr;         /**< The pointer to the returned object, either equal to 'object' or 0, depending on wether the object was created in the current event **/
      std::string name;        /**< Name of the entry. Equal to the key in the map. **/
    };

    /** Stores information on inputs/outputs of a module, as obtained by require()/createEntry(); */
    struct ModuleInfo {
      std::set<std::string> inputs; /**< required input objects/arrays*/
      std::set<std::string> inputRelations; /**< required input relations */
      std::set<std::string> outputs;/**< output objects/arrays */
      std::set<std::string> outputRelations;/**< ouput relations. */

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
    /** Return the default storage name for given class name. */
    static const std::string defaultObjectName(std::string classname) {
      //Strip qualifiers like namespaces
      size_t colon = classname.rfind("::");
      if (colon != std::string::npos) {
        classname = classname.substr(colon + 2);
      }
      return classname;
    }

    /** Return the default storage name for an object of the given type. */
    template<class T> static const std::string defaultObjectName() {
      return defaultObjectName(T::Class_Name());
    }

    /** Return the storage name for an object of the given type and name. */
    template<class T> static const std::string objectName(const std::string& name) {
      return ((name == "") ? defaultObjectName<T>() : name);
    }

    /** Return the default storage name for an given class name. */
    static const std::string defaultArrayName(const std::string& classname) {
      return defaultObjectName(classname) + 's';
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

    /** Produce ERROR message if no entry of the given type is registered in the DataStore.
     *
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @return           True if the requested object exists.
     */
    bool require(const std::string& name, EDurability durability,
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
     *  @return           Wether the object was successfully inserted/created
     */
    bool createObject(TObject* object, bool replace, const std::string& name, EDurability durability,
                      const TClass* objClass, bool array);

    /** Get a reference to the object/array map. */
    const StoreObjMap& getStoreObjectMap(EDurability durability) { return m_storeObjMap[durability]; }

    /** Add a relation from an object in a store array to another object in a store array.
     *
     *  @param fromObject     Pointer to the object from which the relation points.
     *  @param fromEntry      Data store entry that contains the fromObject. Used for caching. Will be set if 0.
     *  @param fromIndex      Index in TClonesArray that contains the fromObject. Used for caching. Will be set if < 0.
     *  @param toObject       Pointer to the object to which the relation points.
     *  @param weight         Weight of the relation.
     *  @return               True if the relation was created, false otherwise.
     */
    bool addRelation(const TObject* fromObject, StoreEntry*& fromEntry, int& fromIndex, const TObject* toObject, double weight);

    /** Get the relations from an object to other objects in a store array.
     *
     *  @param fromObject     Pointer to the object from which the relations point.
     *  @param fromEntry      Data store entry that contains the fromObject. Used for caching. Will be set if 0.
     *  @param fromIndex      Index in TClonesArray that contains the fromObject. Used for caching. Will be set if < 0.
     *  @param toClass        Class of the objects to which the relations point.
     *  @param toName         The name of the store array to which the relations point.
     *                        If empty the default store array name for toClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type toClass are considered.
     *  @return               Vector of relation entry objects.
     */
    std::vector<RelationEntry> getRelationsFromTo(const TObject* fromObject, StoreEntry*& fromEntry, int& fromIndex, const TClass* toClass, const std::string& toName);

    /** Get the relations to an object from other objects in a store array.
     *
     *  @param toObject       Pointer to the object to which the relations point.
     *  @param toEntry        Data store entry that contains the toObject. Used for caching. Will be set if 0.
     *  @param toIndex        Index in TClonesArray that contains the toObject. Used for caching. Will be set if < 0.
     *  @param fromClass      Class of the objects from which the relations point.
     *  @param fromName       The name of the store array from which the relations point.
     *                        If empty the default store array name for fromClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type fromClass are considered.
     *  @return               Vector of relation entry objects.
     */
    std::vector<RelationEntry> getRelationsToFrom(const TObject* toObject, StoreEntry*& toEntry, int& toIndex, const TClass* fromClass, const std::string& fromName);

    /** Get the relations between an object and other objects in a store array.
     *
     *  Relations in both directions are returned. The direction is encoded in the sign
     *  of the weight. Relations that point to the object have a negative weight.
     *  @param object         Pointer to the object from or to which the relations point.
     *  @param entry          Data store entry that contains the object. Used for caching. Will be set if 0.
     *  @param index          Index in TClonesArray that contains the object. Used for caching. Will be set if < 0.
     *  @param withClass      Class of the objects to or from which the relations point.
     *  @param name           The name of the store array to or from which the relations point.
     *                        If empty the default store array name for withClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objectt of type withClass are considered.
     *  @return               Vector of relation entry objects.
     */
    std::vector<RelationEntry> getRelationsWith(const TObject* object, StoreEntry*& entry, int& index, const TClass* withClass, const std::string& name);

    /** Get the first relation from an object to another object in a store array.
     *
     *  @param fromObject     Pointer to the object from which the relation points.
     *  @param fromEntry      Data store entry that contains the fromObject. Used for caching. Will be set if 0.
     *  @param fromIndex      Index in TClonesArray that contains the fromObject. Used for caching. Will be set if < 0.
     *  @param toClass        Class of the objects to which the relation points.
     *  @param toName         The name of the store array to which the relation points.
     *                        If empty the default store array name for toClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type toClass are considered.
     *  @return               The entry of the first related object.
     */
    RelationEntry getRelationFromTo(const TObject* fromObject, StoreEntry*& fromEntry, int& fromIndex, const TClass* toClass, const std::string& toName);

    /** Get the first relation to an object from another object in a store array.
     *
     *  @param toObject       Pointer to the object to which the relation points.
     *  @param toEntry        Data store entry that contains the toObject. Used for caching. Will be set if 0.
     *  @param toIndex        Index in TClonesArray that contains the toObject. Used for caching. Will be set if < 0.
     *  @param fromClass      Class of the objects from which the relation points.
     *  @param fromName       The name of the store array from which the relation points.
     *                        If empty the default store array name for fromClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type fromClass are considered.
     *  @return               The entry of the first related object.
     */
    RelationEntry getRelationToFrom(const TObject* toObject, StoreEntry*& toEntry, int& toIndex, const TClass* fromClass, const std::string& fromName);

    /** Get the first relation between an object and another object in a store array.
     *
     *  Relations in both directions are considered. The direction is encoded in the sign
     *  of the weight. Relations that point to the object have a negative weight.
     *  @param object         Pointer to the object from or to which the relation points.
     *  @param entry          Data store entry that contains the object. Used for caching. Will be set if 0.
     *  @param index          Index in TClonesArray that contains the object. Used for caching. Will be set if < 0.
     *  @param withClass      Class of the objects to or from which the relation points.
     *  @param name           The name of the store array to or from which the relation points.
     *                        If empty the default store array name for withClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objectt of type withClass are considered.
     *  @return               The entry of the first related object.
     */
    RelationEntry getRelationWith(const TObject* object, StoreEntry*& entry, int& index, const TClass* withClass, const std::string& name);

    /** Add a relation from an object in a store array to another object in a store array.
     *
     *  @sa RelationsInterface::addRelationTo
     *  @param fromObject     Pointer to the object from which the relation points.
     *  @param toObject       Pointer to the object to which the relation points.
     *  @param weight         Weight of the relation.
     *  @return               True if the relation was created, false otherwise.
     */
    static bool addRelationFromTo(const TObject* fromObject, const TObject* toObject, double weight = 1) {
      DataStore::StoreEntry* storeEntry = 0;
      int index = -1;
      return Instance().addRelation(fromObject, storeEntry, index, toObject, weight);
    }

    /** Get the relations from an object to other objects in a store array.
     *
     *  @sa RelationsInterface::getRelationsTo
     *  @param object         Pointer to the object from which the relations point.
     *  @tparam T             Class of the objects to which the relations point.
     *  @param toName         The name of the store array to which the relations point.
     *                        If empty the default store array name for toClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return               Vector of relation entry objects.
     */
    template <class T> static RelationVector<T> getRelationsFromObj(const TObject* object, const std::string& toName = "") {
      StoreEntry* storeEntry = 0;
      int index = -1;
      return RelationVector<T>(Instance().getRelationsFromTo(object, storeEntry, index, T::Class(), toName));
    }

    /** Get the relations to an object from other objects in a store array.
     *
     *  @sa RelationsInterface::getRelationsFrom
     *  @param object         Pointer to the object to which the relations point.
     *  @tparam T             Class of the objects from which the relations point.
     *  @param fromName       The name of the store array from which the relations point.
     *                        If empty the default store array name for fromClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return               Vector of relation entry objects.
     */
    template <class T> static RelationVector<T> getRelationsToObj(const TObject* object, const std::string& fromName = "") {
      StoreEntry* storeEntry = 0;
      int index = -1;
      return RelationVector<T>(Instance().getRelationsToFrom(object, storeEntry, index, T::Class(), fromName));
    }

    /** Get the relations between an object and other objects in a store array.
     *
     *  Relations in both directions are returned. The direction is encoded in the sign
     *  of the weight. Relations that point to the object have a negative weight.
     *  @sa RelationsInterface::getRelationsWith
     *  @param object         Pointer to the object from or to which the relations point.
     *  @tparam T             Class of the objects to or from which the relations point.
     *  @param name           The name of the store array to or from which the relations point.
     *                        If empty the default store array name for withClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return               Vector of relation entry objects.
     */
    template <class T> static RelationVector<T> getRelationsWithObj(const TObject* object, const std::string& name = "") {
      StoreEntry* storeEntry = 0;
      int index = -1;
      return RelationVector<T>(Instance().getRelationsWith(object, storeEntry, index, T::Class(), name));
    }

    /** Get the object to which another object has a relation.
     *
     *  @param object  Pointer to the object from which the relation points.
     *  @tparam T      The class of objects to which the relation points.
     *  @param name    The name of the store array to which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> static const T* getRelatedFromObj(const TObject* object, const std::string& name = "") {
      if (!object) return 0;
      StoreEntry* storeEntry = 0;
      int index = -1;
      return static_cast<const T*>(DataStore::Instance().getRelationFromTo(object, storeEntry, index, T::Class(), name).object);
    }

    /** Get the object from which another object has a relation.
     *
     *  @param object  Pointer to the object to which the relation points.
     *  @tparam T      The class of objects from which the relation points.
     *  @param name    The name of the store array from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> static const T* getRelatedToObj(const TObject* object, const std::string& name = "") {
      if (!object) return 0;
      StoreEntry* storeEntry = 0;
      int index = -1;
      return static_cast<const T*>(DataStore::Instance().getRelationToFrom(object, storeEntry, index, T::Class(), name).object);
    }

    /** Get the object to or from which another object has a relation.
     *
     *  @param object  Pointer to the object to or from which the relation points.
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> static const T* getRelated(const TObject* object, const std::string& name = "") {
      if (!object) return 0;
      StoreEntry* storeEntry = 0;
      int index = -1;
      return static_cast<const T*>(DataStore::Instance().getRelationWith(object, storeEntry, index, T::Class(), name).object);
    }

    //------------------------------ Start and end procedures --------------------------------------------------
    /** Setter for m_initializeActive.
     *
     *  This should only be called by EventProcessor.
     */
    void setInitializeActive(bool active) { m_initializeActive = active; }

    /** Are we currently initializing modules? */
    bool getInitializeActive() const { return m_initializeActive; }

    /** Clears maps of a specified durability.
     *
     *  Called by the framework. Users should usually not use this function without a good reason.
     *
     *  Memory occupied by objects/arrays is only freed once a new object is created in its place (in createObject()).
     *
     *  @param durability Decides which map is cleared.
     */
    void clearMaps(EDurability durability = c_Event);

    /** Frees memory occopied by data store items and removes all objects from the map.
     *
     *  Afterwards, m_storeObjMap[durability] is empty.
     *  Called by the framework. Users should usually not use this function without a good reason.
     */
    void reset(EDurability durability);

    /** Set the current module
     *
     * Currently called only in EventProcessor::processInitialize()
     */
    void setModule(const std::string& name) { m_currentModule = name; }

    /** return information on inputs/outputs of each module, as obtained by require()/createEntry(); */
    const std::map<std::string, ModuleInfo>& getModuleInfoMap() const { return m_moduleInfo; }


  private:
    /** Hidden constructor, as it is a singleton.*/
    explicit DataStore();
    /** same for copy constructor */
    DataStore(const DataStore&);
    /** Destructor. */
    ~DataStore();

    /** Check whether the given entry and the requested class match.
     *
     *  @param name       Name of the DataStore map entry.
     *  @param entry      The existing DataStore entry.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @return           True if both types match.
     */
    bool checkType(const std::string& name, const StoreEntry* entry,
                   const TClass* objClass, bool array) const;

    /** Find an object in an array in the data store.
     *
     *  @param object     Pointer to the object.
     *  @param entry      The DataStore entry that contains the object.
     *  @param index      The index of the object in the array.
     *  @return           True if the object was found in the data store
     */
    bool findStoreEntry(const TObject* object, StoreEntry*& entry, int& index);

    /** Fill the vector with the names of store arrays.
     *
     *  @param names      The resulting vector of array names.
     *  @param arrayName  A given array name, the special string "ALL" for all arrays of the given class, or an empty string for the default array name.
     *  @param arrayClass The class of the array(s).
     */
    void getArrayNames(std::vector<std::string>& names, const std::string& arrayName, const TClass* arrayClass);

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

    /** Stores the current module, used to fill m_moduleInfo. */
    std::string m_currentModule;

    /** Stores information on inputs/outputs of each module, as obtained by require()/createEntry(); */
    std::map<std::string, ModuleInfo> m_moduleInfo;
  };
} // namespace Belle2

#endif // DATASTORE_H
