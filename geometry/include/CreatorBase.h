/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CREATORBASE_H
#define CREATORBASE_H

#include <geometry/GeometryManager.h>

class G4LogicalVolume;

namespace Belle2 {

  class GearDir;

  namespace geometry {

    /** Pure virtual base class for all geometry creators */
    class CreatorBase {
    public:
      /** Default Constructor */
      CreatorBase() {}
      /** Default Destructor */
      virtual ~CreatorBase() {}

      /**
       * Function to actually create the geometry, has to be overridden by derived classes
       * @param content GearDir pointing to the parameters which should be used for construction
       * @param topVolume Top volume in which the geometry has to be placed
       * @param type Type of geometry to be build
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type) = 0;
    };

  }
} //end of namespace Belle2

#endif /* CREATORBASE_H */
