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

#include "TClonesArray.h"
#include "framework/datastore/DataStore.h"
#include "framework/datastore/StoreDefs.h"

namespace Belle2 {

  //! Accessor to stored TClonesArrays.
  /*! This is an accessor class for the TClonesArrays saved in the DataStore.
      To add new objects, please use the TClonesArray function
      <a href="http://root.cern.ch/root/htmldoc/TClonesArray.html#TClonesArray:New">New</a>.
      The TClonesArrays are never deleted, but their content is deleted according to the EDurability type,
      that is given to them.
      \author <a href="mailto:martin.heck@kit.edu?subject=StoreArray">Martin Heck</a>
  */
  template <class T>
  class StoreArray {
  public:

    //!Constructor.
    /*! \param name Name with which the TClonesArray is saved.
        \param durability Specifies lifetime of array in question.
        \param generate Shall array be created, if none with name exists so far.
    */
    StoreArray(const std::string& name, const EDurability& durability = c_Event, bool generate = true) {
      if (assignArray(name, durability, generate)) {
        DEBUG(100, "A TClonesArray with name " + name + " has been generated");
      }
    }

    //!Switch the array, the StoreArray points to.
    /*! \param name Name with which the TClonesArray is saved.
        \param durability Specifies lifetime of array in question.
        \param generate Shall array be created, if none with name exists so far.
    */
    bool assignArray(const std::string& name, const EDurability& durability = c_Event, bool generate = false);

    //! Imitate array functionality.
    TClonesArray& operator *() const {return *m_storeArray;};

    //! Imitate array functioanlity.
    TClonesArray* operator ->() const {return m_storeArray;};

    //! Imitate array functionality.
    operator bool() const {return m_storeArray;};

    //! Imitate array functionality.
    /*! By default the TClonesArray would return TObjects, so a cast is necessary.
        The static cast is save here, because at a previous stage, it is already checked,
        that the TClonesArray contains type T.
    */
    T* operator [](int i) const {return static_cast<T*>(m_storeArray->At(i));}

    //! Get the number of occupied slots in the array.
    int GetEntries() const {return m_storeArray->GetEntriesFast();}

  private:
    //! Pointer that actually holds the TClonesArray.
    TClonesArray* m_storeArray;
  };


} // end namespace Belle2

//-------------------Implementation of template part of the class ---------------------------------
using namespace std;


template <class T>
bool StoreArray<T>::assignArray(const std::string& name, const EDurability& durability, bool generate)
{
  if (name == "") {FATAL("No name was specified!");}

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

#endif
