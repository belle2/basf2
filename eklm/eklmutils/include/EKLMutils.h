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

#include <TGeoVolume.h>
#include "boost/lexical_cast.hpp"



namespace Belle2 {



  template < class T >
  void storeEKLMObject(std::string , T*);

  class EKLMNameManipulator {
    EKLMNameManipulator();
    ~EKLMNameManipulator();

  public:


    static int getVolumeNumber(const char*, const char*);
    static int getVolumeNumber(std::string, std::string);


    static std::string getVolumeName(const char *, const char *);
    static std::string getVolumeName(std::string, std::string);

    static const char * getNodePath(const char *);
    static const char * getNodePath(std::string);



    template < class T >
    static bool isX(T stripName);

  };
  //------------------------  Implemantation of the templates -------------------------

  template < class T >
  void storeEKLMObject(std::string arrayName, T* obj)
  {
    StoreArray<T> array(arrayName);
    // since the array is indexed from 0 GetEntries() points exactly to the next to the last entry
    new(array->AddrAt(array.GetEntries())) T(*obj);
  }

  template < class T >
  bool EKLMNameManipulator::isX(T stripName)
  {
    return !(boost::lexical_cast<bool>(getVolumeNumber(stripName, "Plane")));
  };


}




#endif //EKLMUTILS_H
