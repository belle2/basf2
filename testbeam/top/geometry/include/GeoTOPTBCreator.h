/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOTOPTBCREATOR_H_
#define GEOTOPTBCREATOR_H_

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
//#include <G4AssemblyVolume.hh>
//#include <G4LogicalVolume.hh>

namespace Belle2 {
  namespace TOPTB {

    /**
     * Geometry creator for TOP test beam telescope counters etc.
     */
    class GeoTOPTBCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor
       */
      GeoTOPTBCreator();

      /**
       * Destructor
       */
      virtual ~GeoTOPTBCreator();

      /**
       * Creates GEANT4 objects
       * @param content reference to geometry parameters
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);


    protected:


    };

  } // namespace TOPTB
} // namespace Belle2

#endif
