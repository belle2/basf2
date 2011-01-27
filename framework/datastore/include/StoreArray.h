/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREARRAY_H
#define STOREARRAY_H

#include <framework/datastore/StoreAccessorBase.h>
#include <TClonesArray.h>
#include <framework/datastore/DataStore.h>
#include <utility>

namespace Belle2 {

  /** Accessor to stored TClonesArrays.
   *
   *  This is an accessor class for the TClonesArrays saved in the DataStore.
   *  To add new objects, please use the TClonesArray function
   *  <a href="http://root.cern.ch/root/htmldoc/TClonesArray.html#TClonesArray:New">New</a>.
   *  The TClonesArrays are never deleted, but their content is deleted according to the EDurability type,
   *  that is given to them.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=StoreArray">Martin Heck</a>
  */
  template <class T>
  class StoreArray : public StoreAccessorBase {
  public:

    /** Constructor.
     *
     *  @param name Name with which the TClonesArray is saved.
     *  @param durability Specifies lifetime of array in question.
     *  @param generate Shall array be created, if none with name exists so far.
     */
    StoreArray(const std::string& name, const DataStore::EDurability& durability = DataStore::c_Event, bool generate = true) {
      if (assignArray(name, durability, generate)) {
        B2DEBUG(100, "A TClonesArray with name " + name + " has been generated");
      }
    }

    StoreArray(std::pair<std::string, DataStore::EDurability> accessorParams) {
      assignArray(accessorParams.first, accessorParams.second, false);
    }

    /** Switch the array, the StoreArray points to.
     *
     *  @param name       Name with which the TClonesArray is saved.
     *  @param durability Specifies lifetime of array in question.
     *  @param generate Shall array be created, if none with name exists so far.
     */
    bool assignArray(const std::string& name, const DataStore::EDurability& durability = DataStore::c_Event, bool generate = false);

    /** Imitate array functionality. */
    TClonesArray& operator *() const {return *m_storeArray;}

    /** Imitate array functioanlity. */
    TClonesArray* operator ->() const {return m_storeArray;}

    /** Returns name under which the object is saved in the DataStore.
     */
    std::pair<std::string, DataStore::EDurability> getAccessorParams() {return std::pair<std::string, DataStore::EDurability>(m_name, m_durability);};

    /** Return stored object. */
    TClonesArray* getPtr() {return m_storeArray;}

    /** Imitate array functionality. */
    operator bool() const {return m_storeArray;}

    /** Imitate array functionality.
     *
     *  By default the TClonesArray would return TObjects, so a cast is necessary.
     *  The static cast is save here, because at a previous stage, it is already checked,
     *  that the TClonesArray contains type T.
     */
    T* operator [](int i) const {return static_cast<T*>(m_storeArray->At(i));}

    /** Get the number of occupied slots in the array. */
    int GetEntries() const {return m_storeArray->GetEntriesFast();}

    /** Convinient Relation creating.
     *
     *  Using this way to create Relations is safer than direct creation,
     *  because in this case you use definitively an object, that is already stored
     *  in the DataStore.
     *
     *  @par to     Object towards which the relation shall be.
     *  @par from   index of the object, from which the relation shall point.
     *              0 means, the Relation has the whole TClonesArray stored in the StoreArray as from.
     *  @par weight Assign a weight to the Relation.
     */
//    Relation* relateTo(StoreAccessorAbs<TObject>& to, const int& from = 0, float& weight = 1);


    /** Convenient Relation Creating when pointing to part of TClonesArray.
     *  @par to     StoreArray holding the relevant TClonesArray.
     *  @par index  Index of object pointed to in the TClonesArray.
     *              If you want to point to the whole TClonesArray, use the creator without the index argument.
     *  @par        Assign a weight to the relation.
     */
//    Relation* relateTo(StoreAccessorAbs<TClonesArray>& to, const int& index, const int& from = 0, const float& weight = 1);

    /** Convenient RelationArray creating.
     *
     *  This way of creation can be used, if all weights are the same.
     */
//    RelationArray* relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<int>& indexList, const int& from = 0, float& weight = 1);

    /** RelationArray creation in case of multiple weights.
     */
//    RelationArray* relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<std::pair<int, float> > indexWeightList, const int& from = 0);


  private:

    /** Pointer that actually holds the TClonesArray. */
    TClonesArray* m_storeArray;

    /** Store name under which TClonesArray is saved. */
    std::string m_name;

    /**Store durability under which the TClonesArray is saved. */
    DataStore::EDurability m_durability;

  };


} // end namespace Belle2

//-------------------Implementation of template part of the class ---------------------------------

template <class T>
bool Belle2::StoreArray<T>::assignArray(const std::string& name, const DataStore::EDurability& durability, bool generate)
{
  if (name == "") {B2FATAL("No name was specified!");}
  m_name       = name;
  m_durability = durability;

  m_storeArray =  DataStore::Instance().getArray<T>(name, durability);

  if (m_storeArray) {
    return (false);
  }

  if (generate) {
    m_storeArray = DataStore::Instance().createArray<T>(name, durability);
    return(true);
  }
  return (false);

}
/*
template <class T>
Relation* StoreArray<T>::relateTo(StoreAccessorAbs<TObject>& to, const int& from, float& weight)
{
  if (from == 0) {
    return new Relation(m_storeArray, to.getPtr(), weight);
  } else {
    return new Relation(m_storeArray->At(from), to.getPtr());
  }
}

template <class T>
Relation* StoreArray<T>::relateTo(StoreAccessorAbs<TClonesArray>& to, const int& index, const int& from, const float& weight)
{
  if (from == 0) {
    return new Relation(m_storeArray, to.getPtr()->At(index), weight);
  } else {
    return new Relation(m_storeArray->At(from), to.getPtr()->At(index), weight);
  }
}

template <class T>
RelationArray* StoreArray<T>::relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<int>& indexList, const int& from, float& weight)
{
  if (from == 0) {
    return new RelationArray(dynamic_cast<TObject*>(m_storeArray), to.getPtr(), indexList, weight);
  } else {
    return new RelationArray(m_storeArray->At(from), to.getPtr(), indexList, weight);
  }
}

template <class T>
RelationArray* StoreArray<T>::relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<std::pair<int, float> > indexWeightList, const int& from)
{
  if (from == 0) {
    return new RelationArray(m_storeArray, to.getPtr(), indexWeightList);
  } else {
    return new RelationArray(m_storeArray->At(from), to.getPtr(), indexWeightList);
  }
}
*/
#endif
