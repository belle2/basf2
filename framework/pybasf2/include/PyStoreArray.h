/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <TCollection.h> //for TIter

#include <TClonesArray.h>

class TClass;
class TObject;

#include <string>

namespace Belle2 {
  /** A (simplified) python wrapper for StoreArray.
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
  * In case you want to access a named relation (for example, from "Tracks"
  * to "KLMClusters" with a named relation "Secondary"), you can follow
  * the following example:
  *
  * \code{.py}
     from ROOT import Belle2
     tracks = Belle2.PyStoreArray('Tracks')
     for tracks in tracks:
       clusters = track.getRelationsTo['KLMCluster']('KLMClusters', 'Secondary')
       for cluster in clusters:
         # Do something
    \endcode
  *
  * The synthax is: `getRelationsTo['ClassName']('StoreArrayName', 'RelationName')`
  * where `StoreArrayName` can be omitted in case of default store arrays.
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
    bool registerInDataStore(const std::string& name = "",
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
     * @param toArray        Array the relation should point to (from this PyStoreArray)
     * @param durability     Durability of the relation.
     * @param storeFlags     ORed combination of DataStore::EStoreFlags
     * @param namedRelation  Additional name for the relation, or "" for the default naming
     */
    bool registerRelationTo(const PyStoreArray& toArray,
                            DataStore::EDurability durability = DataStore::EDurability::c_Event,
                            DataStore::EStoreFlags storeFlags = DataStore::EStoreFlags::c_WriteOut,
                            std::string const& namedRelation = "") const;

    /** Produce error if no relation from this array to 'toArray' has been registered.
     *
     * Must be called in initialization phase, aborts job if it fails. (allowing you to catch problems early)
     *
     * @param toArray    Array the relation should point to (from this PyStoreArray)
     * @param durability Durability of the relation.
     * @param namedRelation Name of the relation in case it's not the default name
     * @return           True if the relations exists.
     */
    bool requireRelationTo(const PyStoreArray& toArray,
                           DataStore::EDurability durability = DataStore::c_Event,
                           std::string const& namedRelation = "") const;

    /** Tell the data store about a relation that we could make use of. (aka. optional input)
     *
     * Mainly useful for creating diagrams of module inputs and outputs.
     * This must be called in the initialization phase.
     *
     * @param toArray    Array the relation should point to (from this PyStoreArray)
     * @param durability Durability of the relation.
     * @param namedRelation Name of the relation in case it's not the default name
     * @return           True if the relations exists.
     */
    bool optionalRelationTo(const PyStoreArray& toArray,
                            DataStore::EDurability durability = DataStore::c_Event,
                            std::string const& namedRelation = "") const;

    /** Check for the existence of a relation to the provided toArray (from this Pystorearray)
     *
     * @param toArray       Array the relation should point to (from this StoreArray)
     * @param durability    Durability of the relation.
     * @param namedRelation Additional name for the relation, or "" for the default naming
     */
    bool hasRelationTo(const PyStoreArray& toArray,
                       DataStore::EDurability durability = DataStore::c_Event,
                       const std::string& namedRelation = "") const;

    /** Check for the existence of a relation from the provided toArray (to this Pystorearray)
     *
     * @param fromArray     Array the relation should point to (from this StoreArray)
     * @param durability    Durability of the relation.
     * @param namedRelation Additional name for the relation, or "" for the default naming
     */
    bool hasRelationFrom(const PyStoreArray& fromArray,
                         DataStore::EDurability durability = DataStore::c_Event,
                         const std::string& namedRelation = "") const;

    /** Return name under which the object is saved in the DataStore. */
    std::string getName() const { return m_storeAccessor.getName(); }

    /** Return class of the object that is saved in the DataStore. */
    TClass* getClass() const {return m_storeAccessor.getClass(); }

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

    /** Templated function to fill the PyStoreArray with objects of a certain class (PXDDigits, CDCHits, ...)
     * This function will append the values to the array if it's not empty.
     * @param len Length of the PyStoreArray.
     * @param args Arrays of length len, one for each of the memebers of the class under consideration.
     *
     * \warning Do not use this function in C++ directly, but only through the python wrapper
     *          by calling PyStoreArray.fillArray(**kwargs) and passing the numpy arrays
     *          for each of the class members.
    */
    template <class T, typename... Args> void fillArray(size_t len, Args... args)
    {
      TClonesArray* array = getPtr();
      for (size_t i = 0; i < len; i++) {
        new ((*array)[i]) T(args[i]...);
      }
    }

    /** Templated function to read from the PyStoreArray objects of a certain class (PXDDigits, CDCHits, ...)
     * @param args Empty arrays which will be filled with the values of the class members.
     *
     * \warning Do not use this function in C++ directly, but only through the python wrapper
     *          by calling PyStoreArray.readArray().
    */
    template <class T, typename... Args> void readArray(Args... args)
    {
      TClonesArray* array = getPtr();
      for (size_t i = 0; i < array->GetEntriesFast(); i++) {
        ((T*)(*array)[i])->fillValues(&args[i]...);
      }
    }



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
