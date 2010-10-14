/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREACCESSORABS_H
#define STOREACCESSORABS_H


namespace Belle2 {
  /*! Abstract base class for the StoreObjPtr and the StoreArray for easier common treatment by the Relation class.
      This class is purely virtual and should be used only, if you need common treatment of single objects and the TClonesArrays
      in the DataStore.
      \author <a href="mailto:martin.heck@kit.edu?subject=StoreAccessorAbs">Martin Heck</a>
  */
  template <class T>
  class StoreAccessorAbs {
  public:

    /*! Destructor.
        Virtual because this is a base class.
    */
    virtual ~StoreAccessorAbs() = 0;


    /*! Returns the object, that holds the information.
        For StoreObjPtr this will be a TObject. <br>
        For StoreArrays, this will give you back a TClonesArray.
    */
    virtual T* getPtr() = 0;

  };
}

//------------------------Implementation of template class -----------------------------------------
using namespace std;
using namespace Belle2;

template <class T>
StoreAccessorAbs<T>::~StoreAccessorAbs() {}


#endif // STOREACCESSORABS_H
