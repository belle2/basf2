/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr, Nobu Katayama,                 *
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

namespace Belle2 {

  /** In the store you can park objects, that have to be accessed by various modules.
   *
   *  The store saves objects together with names in maps.
   *  Normal users should try to access the store via StoreAccess classes like the
   *  StoreObjPtr or the StoreArray. <br>
   *  Currently the store supports either the storage of single objects, that inherit from TObject,
   *  and TClonesArrays, which are faster, if you have to store a large number of objects from the same type.
   *  Besides that, you have to chose the durability of the things, you want to store. <br>
   *  Currently you can chose between lifetimes of event, run, and persistent.
   *  basf2 deletes the objects from the store according to the durability map, in which the objects are stored.
   *
   *  @sa EDurability StoreObjPtr StoreArray
   *  @author <a href="mailto:martin.heck@kit.edu?subject=DataStore">Martin Heck</a>
   */
  class DataStore {
  public:

    /** Convenient typedefs. */
    typedef std::map<std::string, TObject*> StoreObjMap;         /**< Map for TObjects. */
    typedef std::map<std::string, TClonesArray*> StoreArrayMap;  /**< Map for TClonesArrays. */
    typedef StoreObjMap::iterator StoreObjIter;                  /**< Iterator for TObjectMap. */
    typedef StoreArrayMap::iterator StoreArrayIter;              /**< Iterator for TClonesArraysMap.*/

    /** Durability types.
     *
     *  These types are used to identify the map used in the DataStore.
     *  @author <a href="mailto:martin.heck@kit.edu?subject=EDurability">Martin Heck</a>
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

    /** Instance of singleton Store.
     *
     *  This method is used to access the DataStore directly. It is used internally in the Store accessor classes.
     */
    static DataStore& Instance();

    /** Return the default storage name for an object of the given type */
    template<class T> static const std::string defaultObjectName() {
      std::string classname = (T::Class()->GetName());
      //Strip qualifiers like namespaces
      size_t colon = classname.rfind("::");
      if (colon != std::string::npos) {
        classname = classname.substr(colon + 2);
      }
      return classname;
    }

    /** Return the default storage name for an array of the given type */
    template<class T> static const std::string defaultArrayName() {
      return defaultObjectName<T>() + 's';
    }

    /** Return the default storage name for an relation between the given types */
    template<class FROM, class TO> static const std::string defaultRelationName() {
      return defaultArrayName<FROM>() + "To" + defaultArrayName<TO>();
    }

    /** Create new object with check of existence.
     *
     *  Checks if object with given name already exists in corresponding map. If one of the given name, but different type exists,
     *  the function will end the programm.
     *
     *  @return           If object of given name, type and durability exists, it is returned, otherwise the new object.
     *  @param name       Name of the object to be created.
     *  @param durability Decides when object shall be destroyed.
     */
    template <class T>
    T* createObject(const std::string& name, const EDurability& durability = c_Event);


    /** Get existing object.
     *
     *  Name, durability and type of object have to match. <br>
     *  For matching the type of the object, however, derived classes are cast to their base classes.
     *
     *  @return Existing object, if all matches or NULL pointer. NULL is as well returned, when the object exists,
     *          but type doesn't match.
     *  @param name       Name of the object.
     *  @param durability Looks up the object in respective map, where objects of the given
     *                    durability are stored.
     */
    template <class T>
    T* getObject(const std::string& name, const EDurability& durability = c_Event);


    /** Create new object without check of existence.
     *
     *  The function just creates a new objects, even if one already exists with the same name.
     *  It is faster than the other createObject function, but has the risk of a memory leak.
     *
     *  @return New object.
     *  @param name       Name of the object to be created.
     *  @param durability Decide when object shall be destroyed.
     */
    template <class T>
    T* createObjectRaw(const std::string& name, const EDurability& durability = c_Event);


    /** Create TClonesArray with check of existence.
     *
     *  First it is checked, if array of corresponding name, durability, and object type to be stored already exists,
     *  and if so is returned. If not, a new object is returned.<br>
     *  Note, that even arrays of durability "Event" are not destroyed, but just cleared.
     *  The TClonesArray has a mechanism to delete the objects without giving the memory free,
     *  and thereby allows faster recreation of objects with a mechanism described at
     *  <a href=http://root.cern.ch/root/html/TClonesArray.html> TClonesArray's root page</a>.
     *
     *  @return TClonesArray, either the created one, or an existing one, if name, durability and type matches.
     *  @param name       Name of TClonesArray to be created.
     *  @param durability Decide when TClonesArray shall be destroyed.
     */
    template <class T>
    TClonesArray* createArray(const std::string& name, const EDurability& durability = c_Event);


    /** Get existing TClonesArray with check of existence.
     *  Aborts the programm, if object of name and durability, but different type already exists.
     *
     *  @return Existing TClonesArray, if one of corresponding name, durability and object type to be stored exists,
     *          otherwise NULL.
     *  @param name       Name of TClonesArray, that shall be returned.
     *  @param durability Decide when TClonesArray shall be destroyed.
     */
    template <class T>
    TClonesArray* getArray(const std::string& name, const EDurability& durability = c_Event);


    /** Store existing Object.
     *
     *  Stored object has to follow the usual rules, like inheritance from TObject, ClassDef Macro...
     *
     *  @return Was storing successful?
     *  @param object     Object to be stored.
     *  @param name       Name of object to be stored.
     *  @param durability Decides when object is deleted from store again.
     */
    bool storeObject(TObject* object, const std::string& name, const EDurability& durability = c_Event);


    /** Store existing TClonesArray.
     *
     *  This function will overwrite the content of an potentially existing TClonesArray.
     *
     *  @return Was storing successful?
     *  @param array      TClonesArray to be stored.
     *  @param name       Name of array to be stored.
     *  @param durability Decides when object is deleted from store again.
     */
    bool storeArray(TClonesArray* array, const std::string& name, const EDurability& durability = c_Event);


    /** Clearing Maps of a specified durability.
     *
     *  Called by the framework. Users should usually not use this function without a good reason.
     *  Object Maps are just delted. ArrayMaps don't delete the TClonesArrays, but just their content.
     *  The TClonesArray keeps the memory occupied and one can faster store objects into it.
     *
     *  @param durability Decides which Map is cleared.
     */
    void clearMaps(const EDurability& durability = c_Event);


    /** Get an iterator for one of the object map.
     *
     *  @return iterator for the specified map.
     *  @param  durability EDurability type to specify map.
     */
    StoreMapIter<StoreObjMap>* getObjectIterator(const EDurability& durability);

    /** Get an iterator for one of the object map.
     *
     * @return iterator for the specified map.
     * @param  durability EDurability type to specify map.
     */
    StoreMapIter<StoreArrayMap>* getArrayIterator(const EDurability& durability);


  protected:

    /** Constructor is private, as it is a singleton.*/
    explicit DataStore();


  private:

    /** Destructor */
    ~DataStore() {};

    /** Map for TObjects.
     *
     *  The StoreObjMap maps std::strings to TObjects.
     *  There is a separated Map for each durability type that exists.
     */
    StoreObjMap m_objectMap[c_NDurabilityTypes];


    /** Map for TClonesArrays.
     *
     *  Separe map because of the special properties of the TClonesArray.
     *  Otherwise same as map for the TObjects.
     */
    StoreArrayMap m_arrayMap[c_NDurabilityTypes];

    static DataStore* m_instance; /**< Pointer, that actually holds the store instance.*/

  };
}

//---------------------------------------Implementation -------------------------------------------------


template <class T>
T* Belle2::DataStore::createObject(const std::string& name, const Belle2::DataStore::EDurability& durability)
{
  //get iterator to object, if it already exists
  Belle2::DataStore::StoreObjIter iter = m_objectMap[durability].find(name);

  //if name already exists, iter does not point to the end of the map
  if (iter != m_objectMap[durability].end()) {
    B2INFO("Called object, that already exists.");
    //casting includes type check
    T* corrObject = dynamic_cast<T*>(iter->second);

    //if type doesn't match, user probably messed up mightily
    if (!corrObject) {
      B2FATAL("Existing object is of different type than requested one. Name was: " + name);
    }

    //return of the existing object
    return corrObject;
  }

  //no object, create it
  T* corrObject = new T;

  //saved in the map
  m_objectMap[durability][name] = corrObject;

  //and returened
  return corrObject;
}


template <class T>
T* Belle2::DataStore::getObject(const std::string& name, const Belle2::DataStore::EDurability& durability)
{
  //get iterator pointing to the existing object
  Belle2::DataStore::StoreObjIter iter = m_objectMap[durability].find(name);

  //if iter points to existing object, it doesn't point to end
  if (iter != m_objectMap[durability].end()) {
    return dynamic_cast<T*>(iter->second);
  }

  //if object doesn't exist
  return 0;
}


template <class T>
T* Belle2::DataStore::createObjectRaw(const std::string& name, const Belle2::DataStore::EDurability& durability)
{
  //create the new object
  T* object = new T;

  //add it to the map
  m_objectMap[durability][name] = object;

  //return it
  return object;
}


template <class T>
TClonesArray* Belle2::DataStore::getArray(const std::string& name, const Belle2::DataStore::EDurability& durability)
{
  //get iterator pointing to existing array
  Belle2::DataStore::StoreArrayIter iter =  m_arrayMap[durability].find(name);

  if (iter != m_arrayMap[durability].end()) {
    TClonesArray* ptr = iter->second;
    //only return, if type matches
    if (T::Class() != ptr->GetClass()) {
      B2FATAL("Array '" << name << "' contains elements of " << ptr->GetClass()->GetName()
              << ", requested was " << T::Class()->GetName());
    }
    return ptr;
  }

  //if object doesn't exist
  return 0;
}


template <class T>
TClonesArray* Belle2::DataStore::createArray(const std::string& name, const EDurability& durability)
{
  //check; use getArray, as it checks for type safety.
  TClonesArray* array = getArray<T>(name, durability);

  if (array) {
    B2WARNING("Array " + name + " already exists");
    return array;
  }

  //make new one, if none exists
  array = new TClonesArray(T::Class());
  m_arrayMap[durability][name] = array;
  return array;
}

#endif // DATASTORE_H
