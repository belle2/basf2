/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREACCESSORBASE_H
#define STOREACCESSORBASE_H

#include <string>
#include <utility>
#include <framework/datastore/DataStore.h>


namespace Belle2 {
  /** Abstract base class for the StoreObjPtr and the StoreArray for easier common treatment by the Relation class.
   *
   *  This class is purely virtual. It is the recommended access to the DataStore in case you have
   *  to loop over all objects in the DataStore
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=StoreAccessorAbs">Martin Heck</a>
   */
  class StoreAccessorBase {
  public:

    /** Destructor.
     *
     *  Virtual because this is a base class.
     */
    virtual ~StoreAccessorBase();


    /** Returns name under which stored object is saved.
     */
    virtual std::pair<std::string, DataStore::EDurability> getAccessorParams() = 0;

    /** Returns the object, that holds the information.
     *
     *  For StoreObjPtr this will be a TObject. <br>
     *  For StoreArrays, this will give you back a TClonesArray.
     */
//    virtual T* getPtr() = 0;

  };
}

//------------------------Implementation of template class -----------------------------------------
using namespace std;
using namespace Belle2;

StoreAccessorBase::~StoreAccessorBase() {}


#endif // STOREACCESSORBASE_H
