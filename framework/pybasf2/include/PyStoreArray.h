/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <TCollection.h> //for TIter

class TClonesArray;
class TClass;
class TObject;

#include <string>

namespace Belle2 {
  /** a (simplified) python wrapper for StoreArray.
  *
  * Compared to StoreArray, PyStoreArray returns only TObject pointers (since
  * it doesn't use templates). Thanks to Python, you can still access all
  * public functions and data members of the actual type.
  *
  * Relations can also be accessed using functions like RelationsObject::getRelationsTo()
  * as long as the data objects are derived from RelationsObject. A full example
  * for accessing simhits and some related objects can be found in framework/examples/cdcplotmodule.py
  * Additional registration and filling of PyStoreArrays is demonstrated in
  * framework/examples/register_pystorearray.py
  *
  * Example:
  *
  * \code{.py}
     from ROOT import Belle2
     simhits = Belle2.PyStoreArray('PXDSimHits')
     # Alternative: simhits = Belle2.PyStoreArray(Belle2.PXDSimHits.Class())
     for hit in simhits:
         part = hit.getRelatedFrom('MCParticles')
         print("Edep: ", str(hit.getEnergyDep()))
         print("Particle: ", str(part.getPDG()))
    \endcode
  *
  * You can check the runtime type information of the returned objects by
  * using Python's built-in type() function.
  *
  * \sa PyStoreObj and the Conditions Data interface classes PyDBObj and PyDBArray
  */
  class PyStoreArray {
  public:
    /** Return list of available arrays for given durability. */
    static std::vector<std::string> list(DataStore::EDurability durability = DataStore::EDurability::c_Event);

    /** Print list of available arrays for given durability. */
    static void printList(DataStore::EDurability durability = DataStore::EDurability::c_Event);

    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreArray(const std::string& name,
                          DataStore::EDurability durability =  DataStore::EDurability::c_Event);

    /** constructor.
    * @param objClass Class of the object to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreArray(TClass* objClass,
                          DataStore::EDurability durability =  DataStore::EDurability::c_Event);

    /** constructor.
    * @param objClass Class of the object to be accessed
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreArray(TClass* objClass,
                          const std::string& name,
                          DataStore::EDurability durability =  DataStore::EDurability::c_Event);

    /** Register the array in the data store.
     *  This must be called in the initialization phase.
     *
     *  @param storeFlags  ORed combination of DataStore::EStoreFlags.
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(DataStore::EStoreFlags storeFlags);

    /** Register the array in the data store.
     *  This must be called in the initialization phase.
     *
     *  @param name        Name of the entry to be registered. Empty for default name.
     *  @param storeFlags  ORed combination of DataStore::EStoreFlags. (default: c_WriteOut)
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(std::string name = "",
                             DataStore::EStoreFlags storeFlags = DataStore::EStoreFlags::c_WriteOut);

    /** Ensure this array has been registered previously.
     *  Will cause an ERROR if it does not exist.
     *  This must be called in the initialization phase.
     *
     *  @param name  If not empty, use non-default name for this array.
     *  @return      True if the array exists.
     */
    bool isRequired(const std::string& name = "");

    /** Tell the DataStore about an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *  This must be called in the initialization phase.
     *
     *  @param name  If not empty, use non-default name for this array.
     *  @return      True if the array exists.
     */
    bool isOptional(const std::string& name = "");

    /** Register a relation to the given PyStoreArray.
     *
     *  Use this if you want to create relate objects in this array to objects in 'toArray'.
     *  Must be called in the initialization phase.
     *
     * @param toArray    Array the relation should point to (from this PyStoreArray)
     * @param durability Durability of the relation.
     * @param storeFlags ORed combination of DataStore::EStoreFlags
     */
    bool registerRelationTo(const PyStoreArray& toArray,
                            DataStore::EDurability durability = DataStore::EDurability::c_Event,
                            DataStore::EStoreFlags storeFlags = DataStore::EStoreFlags::c_WriteOut) const;

    /** Produce error if no relation from this array to 'toArray' has been registered.
     *
     * Must be called in initialization phase, aborts job if it fails. (allowing you to catch problems early)
     *
     * @param toArray    Array the relation should point to (from this PyStoreArray)
     * @param durability Durability of the relation.
     * @return           True if the relations exists.
     */
    bool requireRelationTo(const PyStoreArray& toArray,
                           DataStore::EDurability durability = DataStore::c_Event) const;

    /** Tell the data store about a relation that we could make use of. (aka. optional input)
     *
     * Mainly useful for creating diagrams of module inputs and outputs.
     * This must be called in the initialization phase.
     *
     * @param toArray    Array the relation should point to (from this PyStoreArray)
     * @param durability Durability of the relation.
     * @return           True if the relations exists.
     */
    bool optionalRelationTo(const PyStoreArray& toArray,
                            DataStore::EDurability durability = DataStore::c_Event) const;

    /** Return name under which the object is saved in the DataStore. */
    std::string getName() const { return m_storeAccessor.getName(); }

    /** Check whether a TClass of the objects in this PyStoreArray could be determined. */
    bool hasValidClass() const;

    /** Check whether the array was registered and created.*/
    bool isValid() const;

    /** Does this PyStoreArray contain a valid datastore array?  */
    operator bool() const { return isValid(); }

    //------------------------------ Accessing content of the array ----------------------------------
    /** returns object at index i, or null pointer if out of range (+error) */
    TObject* operator [](int i) const;

    /** returns number of entries for current event. */
    int getEntries() const;

    /** Support for len(). */
    int __len__() const { return getEntries(); }

    /** Allow iteration using for in Python.
     **/
    TIter __iter__() const;

    /** Construct a new object of the array's type at the end of the array.
     *
     * @returns the created object, to be modified by the user
     */
    TObject* appendNew();

    /** Raw access to the underlying TClonesArray.
     *
     *  \warning TClonesArray is dangerously easy to misuse. Whatever you do will probably
     *           be slow, leak memory, and murder your pets.
     *           In most cases, you'll want to use functions like operator[],
     *           getEntries() or appendNew() instead.
     */
    TClonesArray* getPtr();

  private:
    /** Ensure that contained TClonesArray has been created on the DataStore. */
    void ensureCreated();

    /** Create constructed TClonesArray in the DataStore.
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false);

    /** Ensure that contained TClonesArray has been attached to a memory location on the DataStore.*/
    void ensureAttached() const;

    /** Lookup the store entry and cache a pointer to it */
    void attach() const;

  private:
    /// Store accessor to retrieve the object.
    StoreAccessorBase m_storeAccessor;

    /// Pointer to the DataStore entry - serves as an internal cache omitting repeated look up from the DataStore.
    mutable StoreEntry* m_storeEntry = nullptr;
  };
}
