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
#include <framework/datastore/StoreMapIter.h>

#include <TObject.h>
#include <TClonesArray.h>
#include <TClass.h>

#include <iostream>
#include <string>
#include <utility>

//#include <boost/tuple/tuple.hpp>

namespace Belle2 {

  /** In the store you can park objects, that have to be accessed by various modules.
   *
   *  The store saves objects together with names and some flags in maps.
   *  Normal users should try to access the store via StoreAccessor classes like the
   *  StoreObjPtr or the StoreArray. <br>
   *  Currently the store supports either the storage of single objects, that inherit from TObject,
   *  and TClonesArrays, which are faster, if you have to store a large number of objects from the same type.
   *  Besides that, you have to chose the durability of the things, you want to store. <br>
   *  Currently you can chose between lifetimes of event, run, and persistent.
   *  basf2 deletes the objects from the store according to the durability map, in which the objects are stored.
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

    // Convenient typedefs.
//    For later:
//    typedef std::map<std::string, boost::tuple<TObject*,      ECollectionPropFlags> > StoreObjMap;   /**< Map for TObjects. */
//    typedef std::map<std::string, boost::tuple<TClonesArray*, ECollectionPropFlags> > StoreArrayMap; /**< Map for TClonesArrays. */
    typedef std::map<std::string, TObject*> StoreObjMap;   /**< Map for TObjects. */
    typedef std::map<std::string, TClonesArray*> StoreArrayMap; /**< Map for TClonesArrays. */
    typedef StoreObjMap::iterator   StoreObjIter;             /**< Iterator for TObjectMap. */
    typedef StoreArrayMap::iterator StoreArrayIter;           /**< Iterator for TClonesArraysMap.*/

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

    /** Return the default storage name for an array of the given type. */
    template<class T> static const std::string defaultArrayName() {
      return defaultObjectName<T>() + 's';
    }

    /** Return the default storage name for an relation between the given types. */
    template<class FROM, class TO> static const std::string defaultRelationName() {
      return defaultArrayName<FROM>() + "To" + defaultArrayName<TO>();
    }

    //------------------------------ Accessing objects and arrays ----------------------------------------------
    /** Function to create, get or store objects in the DataStore.
     *
     *  If in the map of requested durability is already an object under the key name with a DIFFERENT type
     *  than the template type one, the program ends. <br>
     *  If this function is called for the first time with a given name and durability, a new map slot is created.
     *  However, for I/O related reasons, this should be usually not done outside of the initialize functions of modules.
     *  Therefore the latter condition is checked, and an error returned, if you try to create a new map slot at other
     *  times.
     *  @return           In case AObject has been NULL, true is returned, if a new object was created.
     *                    In case AObject has been different from NULL, it is returned, if AObject could be stored
     *                    successfully.
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param generate   In case AObject is NULL, this boolean decides, if a new object shall be created, if there is
     *                    not already one occupying the requested slot.
     *                    In case there is an AObject other than NULL pointer, this value is without consequences.
     *  @param AObject    Object pointer reference for possible storage.<br>
     *                    If an existing object is in the map of requested durability at the
     *                    slot with key "name", this pointer is reassigned to that object.
     *                    If the slot is free, AObject is a NULL pointer and the parameter generate is true it is
     *                    reassigned to a new object created with the default constructor and returned.
     */
    template <class T> bool handleObject(const std::string& name, const EDurability& durability,
                                         bool generate, T*& AObject = static_cast<T*>(0));

    /** Store existing Object.
     *
     *  Stored object has to follow the usual rules, like inheritance from TObject, ClassDef Macro... <br>
     *  This function just calls the handleObject function, and is used by some for direct access to the DataStore.
     *  @return           Was storing successful?
     *  @param AObject    Object to be stored.
     *  @param name       Name under which object shall be stored in the DataStore.
     *  @param durability Decides when object is deleted from store again.
     */
    template <class T> inline bool storeObject(T*& AObject, const std::string& name, const EDurability& durability = c_Event) {
      return handleObject<T>(name, durability, true, AObject);
    }

    /** Function to create, get or store TClonesArrays in the DataStore.
     *
     *  If in the map of requested durability is already an array under the key name with a DIFFERENT type
     *  of clone space than the template type one, the program ends. <br>
     *  If this function is called for the first time with a given name and durability, a new map slot is created.
     *  However, for I/O related reasons, this should be usually not done outside of the initialize functions of modules.
     *  Therefore the latter condition is checked, and an error returned, if you try to create a new map slot at other
     *  times. <br>
     *  The TClonesArray has a mechanism to delete the objects without giving the memory free,
     *  and thereby allows faster recreation of objects with a mechanism described at
     *  <a href=http://root.cern.ch/root/html/TClonesArray.html> TClonesArray's root page</a>.
     *  @return           True, if a new map slot was created, false otherwise. As the map slots are never freed,
     *                    this indicates as well, if storing a TClonesArray was successful, if one was given.
     *  @param name       Name under which a TClonesArray shall be stored in the DataStore.
     *  @param durability Decide when TClonesArray shall be destroyed.
     *  @param array      This reference to a TClonesArray pointer is either redirected to an existing TClonesArray,
     *                    or to a newly created one.
     */
    template <class T> bool handleArray(const std::string& name,
                                        const EDurability& durability,
                                        TClonesArray*& array = static_cast<TClonesArray*>(0));

    /** Store existing TClonesArray.
     *
     *  @return           True, if storing was successful.
     *  @param array      TClonesArray to be stored.
     *  @param name       Name under which array shall be stored in the DataStore.
     *  @param durability Decides when array is deleted from store again.
     */
    inline bool storeArray(TClonesArray* array, const std::string& name, const EDurability& durability = c_Event) {
      // use of TObject for template class is usually fine, because it is only checked, if there is already a corresponding slot.
      B2DEBUG(250, "Storing will abort, if there is already an object with name " << name << ", durability " << durability);
      return handleArray<TObject>(name, durability, array);
    }

    /** Get an iterator for one of the object maps.
     *
     *  @return            Iterator for the specified map.
     *  @param  durability EDurability type to specify map.
     */
    StoreMapIter<StoreObjMap>* getObjectIterator(const EDurability& durability);

    /** Get an iterator for one of the TClonesArray maps.
     *
     *  @return            Iterator for the specified map.
     *  @param  durability EDurability type to specify map.
     */
    StoreMapIter<StoreArrayMap>* getArrayIterator(const EDurability& durability);

    //------------------------------ Start and end procedures --------------------------------------------------
    /** Setter for initializeActive. */
    void setInitializeActive(const bool active) {
      initializeActive = active;
    }

    /** Creating new map slots is only allowed, if this boolean is true. */
    bool initializeActive;

    /** Clearing Maps of a specified durability.
     *
     *  Called by the framework. Users should usually not use this function without a good reason.
     *  Object Maps are just deleted. ArrayMaps don't delete the TClonesArrays, but just their content.
     *  The TClonesArray keeps the memory occupied and one can faster store objects into it.
     *  @param durability Decides which Map is cleared.
     */
    void clearMaps(const EDurability& durability = c_Event);

  protected:
    /** Constructor is protected, as it is a singleton.*/
    explicit DataStore();

  private:
    /** Destructor. */
    ~DataStore() {};

    static DataStore* m_instance; /**< Pointer, that actually holds the store instance.*/

    /** Map for TObjects.
     *
     *  The StoreObjMap maps std::strings to TObject pointers.
     *  There is a separated Map for each durability type that exists.
     */
    StoreObjMap m_objectMap[c_NDurabilityTypes];


    /** Map for TClonesArrays.
     *
     *  Separate map because of the special properties of the TClonesArray.
     *  Otherwise same as map for the TObjects.
     */
    StoreArrayMap m_arrayMap[c_NDurabilityTypes];
  };
} // namespace Belle2

//---------------------------------------Implementation -------------------------------------------------
template <class T> bool Belle2::DataStore::handleObject(const std::string& name,
                                                        const Belle2::DataStore::EDurability& durability,
                                                        bool generate, T*& AObject)
{
  const bool object_found = (m_objectMap[durability].find(name) != m_objectMap[durability].end());

  if (!object_found || m_objectMap[durability][name] == 0) {
    // new slot in map needs to be created
    if (!object_found && generate && !initializeActive) {
      // should only happen in the initialize phase
      //shall soon be replaced with a B2ERROR message
      B2WARNING("initializeActive is false while you try to create an object " << name << " under EDurability " << durability);
    }
    if (AObject == 0 && generate) {
      AObject = new T;
      B2DEBUG(100, "Object with name " << name << " and durability " << durability << " was created.");
    }
    if (AObject != 0)
      m_objectMap[durability][name] = AObject;
  } else {
    //object found
    if (AObject != 0) { //and new one given...
      B2INFO("Found existing object '" << name << "', overwriting.");
      delete m_objectMap[durability][name];
      m_objectMap[durability][name] = AObject;
    }
    AObject = dynamic_cast<T*>(m_objectMap[durability][name]);
    if (AObject == 0) {
      B2ERROR("Existing object '" << name << "' of type " << m_objectMap[durability][name]->ClassName() << " doesn't match requested type " << T::Class()->GetName());
    }
  }

  return generate;
}


template <class T> bool Belle2::DataStore::handleArray(const std::string& name,
                                                       const Belle2::DataStore::EDurability& durability,
                                                       TClonesArray*& array)
{
  const bool register_new_array = (m_arrayMap[durability].find(name) == m_arrayMap[durability].end());

  if (register_new_array) { // new slot in map needs to be created
    if (!initializeActive) { // should only happen in the initialize phase
      //shall soon be replaced with an B2ERROR message
      B2WARNING("initializeActive is false while you try to create an array " << name << " under EDurability " << durability);
    }
    if (array == 0) {
      array = new TClonesArray(T::Class()); // use default constructor
      B2DEBUG(100, "Array with name " << name << " and durability " << durability << " was created.");
    }

    //actually insert new array
    m_arrayMap[durability][name] = array;
  } else { //map already contains an array
    if (array != 0) { //we have both a new array and an existing one, merge them.
      B2INFO("Found existing array '" << name << "', merging.");
      array->AbsorbObjects(m_arrayMap[durability][name]);
      delete m_arrayMap[durability][name];
      m_arrayMap[durability][name] = array;
    }
    array = m_arrayMap[durability][name];
    B2DEBUG(250, "Attaching to existing TClonesArray with name " << name << " and durability " << durability << ".");

    //assuming array != 0 (valid unless someone stored a null pointer in the DataStore)
    if (!array->GetClass()->InheritsFrom(T::Class())) { // TClonesArray in map slot is for different type than requested one
      B2FATAL("Requested array type (" << T::Class()->GetName() << ") is not a base class of the existing array (" << array->GetClass()->GetName() << "). Name was: " <<  name << " EDurability was " << durability);
    }
  }

  return register_new_array;
}

#endif // DATASTORE_H
