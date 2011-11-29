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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TGeoVolume.h>
#include "boost/lexical_cast.hpp"



namespace Belle2 {

  //! template function to simple store any storable object in the datastore.
  //! (too slow?)
  template < class T >
  void storeEKLMObject(std::string arrayName, T* obj)
  {
    StoreArray<T> array(arrayName);
    // since the array is indexed from 0 getEntries() points
    // exactly to the next to the last entry
    new(array->AddrAt(array.getEntries())) T(*obj);
  }

  /**
   * Get physical volume by point
   */
  const G4VPhysicalVolume *GetPhysicalVolumeByPoint(const G4ThreeVector &point);

  /**
   * Check whether strip is oriented along X
   * @strip: strip
   */
  bool CheckStripOrientationX(const G4VPhysicalVolume *strip);
}




#endif //EKLMUTILS_H

