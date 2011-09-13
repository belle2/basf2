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

#ifndef GEOBEAMPIPECREATOR_H_
#define GEOBEAMPIPECREATOR_H_

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <G4Transform3D.hh>
class G4LogicalVolume;
class G4AssemblyVolume;
class G4Polycone;

namespace Belle2 {
  namespace ir {
    class SensitiveDetector;

    /** The creator for the BeamPipe geometry of the Belle II detector.   */
    class GeoBeamPipeCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoBeamPipeCreator class. */
      GeoBeamPipeCreator();

      /** The destructor of the GeoBeamPipeCreator class. */
      virtual ~GeoBeamPipeCreator();

      /**
       * Creates the ROOT Objects for the BeamPipe geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */

      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:
      std::vector<SensitiveDetector*> m_sensitive;
    };

  }
}

#endif /* GEOBeamPipeCREATOR_H_ */
