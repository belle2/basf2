/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMUTILS_H
#define EKLMUTILS_H

#include "G4VPhysicalVolume.hh"

//! Tools collections common for EKLM

//#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TGeoVolume.h>
#include "boost/lexical_cast.hpp"



namespace Belle2 {

  //! template function to simple store any storable object in the datastore.
  //! (too slow?)
  template < class T >
  void storeEKLMObject(std::string , T*);

  //! Class for manipulting with hits/strips/ectors etc names. containes only
  //! static functions
  class EKLMNameManipulator {

  public:
    //! returns number of the volume, first agument is volume name,
    //! sectond is volume type(Strip, Layer, Plane etc).
    static int getVolumeNumber(const char*, const char*);

    //! returns number of the volume, first agument is volume name,
    //! sectond is volume type(Strip, Layer, Plane etc). .
    static int getVolumeNumber(std::string, std::string);

    //! returns name of the volume, first agument is volume name,
    //! sectond is volume type(Strip, Layer, Plane etc).
    static std::string getVolumeName(const char *, const char *);

    //! returns name of the volume, first agument is volume name,
    //! sectond is volume type(Strip, Layer, Plane etc).
    static std::string getVolumeName(std::string, std::string);

    //! returns path from the top to the volume with given name
    static std::string getNodePath(const char *);

    //! returns path from the top to the volume with given name
    static std::string getNodePath(std::string);

    //! returns true if the strip is along X otherwise returns false
    template < class T >
    static bool isX(T stripName);

  private:
    //! Constructor is hidden since we do not intend to create the objects
    //! of this calss
    EKLMNameManipulator();
    //! Destructor
    ~EKLMNameManipulator();

  };
  //---------------------  Implemantation of the templates --------------------

  template < class T >
  void storeEKLMObject(std::string arrayName, T* obj)
  {
    StoreArray<T> array(arrayName);
    // since the array is indexed from 0 GetEntries() points
    // exactly to the next to the last entry
    new(array->AddrAt(array.GetEntries())) T(*obj);
  }

  template < class T >
  bool EKLMNameManipulator::isX(T stripName)
  {
    return !(boost::lexical_cast<bool>(getVolumeNumber(stripName, "Plane")));
  };

  G4VPhysicalVolume *GetPhysicalVolumeByPoint(const G4ThreeVector &point);
}




#endif //EKLMUTILS_H

