/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "G4TransportationManager.hh"
#include "G4Navigator.hh"

#include <eklm/geoeklm/G4PVPlacementGT.h>

#include <eklm/eklmutils/EKLMutils.h>

namespace Belle2 {

  const  G4VPhysicalVolume *GetPhysicalVolumeByPoint(const G4ThreeVector &point)
  {
    return G4TransportationManager::GetTransportationManager()->
           GetNavigatorForTracking()->LocateGlobalPointAndSetup(point);
  }

  bool CheckStripOrientationX(const G4VPhysicalVolume *strip)
  {
    G4Transform3D t = ((G4PVPlacementGT*)strip)->getTransform().inverse();
    return (fabs(sin(t.getRotation().phiX())) < 0.01);
  }

}
