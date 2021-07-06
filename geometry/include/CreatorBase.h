/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <geometry/GeometryManager.h>
#include <framework/core/FrameworkExceptions.h>

class G4LogicalVolume;

namespace Belle2 {

  class GearDir;
  class IntervalOfValidity;

  namespace geometry {

    /** Pure virtual base class for all geometry creators */
    class CreatorBase {
    public:
      /** Exception that will be thrown in createFromDB if member is not yet implemented by creator */
      BELLE2_DEFINE_EXCEPTION(DBNotImplemented, "Cannot create geometry from Database.");
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

      /**
       * Function to create the geometry from the Database
       * @param name name of the component in the database, could be used to disambiguate multiple components created with the same creator
       * @param topVolume Top volume in which the geometry has to be placed
       * @param type Type of geometry to be build
       */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, GeometryTypes type);

      /** Function to create the geometry database.
       * This function should be implemented to convert Gearbox parameters to one ore more database payloads
       * @param content GearDir pointing to the parameters which should be used for construction
       * @param iov interval of validity to use when generating payloads
       */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov);
    };
  }
} //end of namespace Belle2
