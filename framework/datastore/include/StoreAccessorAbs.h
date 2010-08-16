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
    /*!
    */
    virtual T* getPtr() = 0;

  };
}


#endif // STOREACCESSORABS_H
