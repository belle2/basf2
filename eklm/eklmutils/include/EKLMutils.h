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

#include <TGeoVolume.h>
#include <TVector3.h>
#include "boost/lexical_cast.hpp"

#include "CLHEP/Vector/ThreeVector.h"

#include <eklm/dataobjects/EKLMStripHit.h>


namespace Belle2 {

  /**
   * Get physical volume by point
   */
  const G4VPhysicalVolume* GetPhysicalVolumeByPoint(const G4ThreeVector& point);

  /**
   * Check whether strip is oriented along X
   * @strip: strip
   */
  bool CheckStripOrientationX(const G4VPhysicalVolume* strip);


  /**
   * check if two Hitted sterips do intersect and fill Hep3Vector with coordinates of intersection
   * if interseation is not found crossPoint is set to (0,0,0)
   */
  bool doesIntersect(const EKLMStripHit* hit1, const EKLMStripHit* hit2,
                     TVector3& crossPoint);



  /**
   *  Return distance from the point and SiPM of the Strip
   */
  double getLightPropagationLength(const G4VPhysicalVolume* vol, const TVector3&  pos);

  /**
   *  Return distance from the point and SiPM of the Strip
   */
  double getLightPropagationLength(const G4VPhysicalVolume* vol, const CLHEP::Hep3Vector&  pos);

}

#endif //EKLMUTILS_H

