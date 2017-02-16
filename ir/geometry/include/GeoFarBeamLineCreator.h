/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOFARBEAMLINECREATOR_H_
#define GEOFARBEAMLINECREATOR_H_

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <G4Transform3D.hh>
#include <G4VSolid.hh>
//class G4LogicalVolume;
//class G4AssemblyVolume;
//class G4Polycone;

namespace Belle2 {
  namespace ir {
    /** The struct for FarBeamLineElement */
    struct FarBeamLineElement {
      /** Transformation */
      G4Transform3D transform;
      /** Solid volume */
      G4VSolid* geo;
    };

    class SensitiveDetector;

    /** The creator for the FarBeamLine geometry of the Belle II detector.   */
    class GeoFarBeamLineCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoFarBeamLineCreator class. */
      GeoFarBeamLineCreator();

      /** The destructor of the GeoFarBeamLineCreator class. */
      virtual ~GeoFarBeamLineCreator();

      /**
       * Creates the ROOT Objects for the FarBeamLine geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:
      /** Sensitive detector. **/
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* GEOFarBeamLineCREATOR_H_ */
