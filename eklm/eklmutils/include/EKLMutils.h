/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMUTILS_H
#define EKLMUTILS_H

// Tools collections common for EKLM

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>



namespace Belle2 {
  template < class T >
  void storeEKLMObject(std::string , T*);


  //------------------------  Implemantation of the templates -------------------------

  template < class T >
  void storeEKLMObject(std::string arrayName, T* obj)
  {
    StoreArray<T> array(arrayName);
    // since the array is indexed from 0 GetEntries() points exactly to the next to the last entry
    new(array->AddrAt(array.GetEntries())) T(*obj);
  }

}




#endif //EKLMUTILS_H
