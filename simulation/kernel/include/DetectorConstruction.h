/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DETECTORCONSTRUCTION_H
#define DETECTORCONSTRUCTION_H

#include <geometry/GeometryManager.h>
#include "G4VUserDetectorConstruction.hh"

namespace Belle2 {

  /**
   * Class responsible to connect to geometry to simulation.
   * Normally this class is responsible to build the geometry. In our case the
   * geometry should already be built by the Geometry Module since it is also
   * used for tracking and display, so we just hand over the pointer to Geant4
   */
  class DetectorConstruction: public G4VUserDetectorConstruction {
  public:
    /** Return pointer to top volume */
    G4VPhysicalVolume* Construct() {
      G4VPhysicalVolume* topVolume = geometry::GeometryManager::getInstance().getTopVolume();
      if (!topVolume) B2FATAL("No Geometry defined, please create the geometry"
                                " before doing simulation, normally by using the Geometry module.");
      return topVolume;
    }
  };

} //Belle2 namespace
#endif
