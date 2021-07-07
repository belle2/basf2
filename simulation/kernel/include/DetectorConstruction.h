/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DETECTORCONSTRUCTION_H
#define DETECTORCONSTRUCTION_H

#include <framework/logging/Logger.h>
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
    G4VPhysicalVolume* Construct()
    {
      G4VPhysicalVolume* topVolume = geometry::GeometryManager::getInstance().getTopVolume();
      if (!topVolume) B2FATAL("No Geometry defined, please create the geometry"
                                " before doing simulation, normally by using the Geometry module.");
      return topVolume;
    }
  };

} //Belle2 namespace
#endif
