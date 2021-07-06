/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef GEOHEAVYMETALSHIELDCREATOR_H
#define GEOHEAVYMETALSHIELDCREATOR_H

#include <geometry/CreatorBase.h>

#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

#include <vxd/dbobjects/HeavyMetalShieldGeometryPar.h>

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {

    /** The creator for the HeavyMetalShield geometry of the Belle II detector.   */
    class GeoHeavyMetalShieldCreator : public geometry::CreatorBase {
    private:
      //! Create a parameter object from the Gearbox XML parameters.
      HeavyMetalShieldGeometryPar createConfiguration(const GearDir& param);

      //! Create the geometry from a parameter object.
      void createGeometry(const HeavyMetalShieldGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    public:
      /** The old create member: create the configuration object(s) on the fly
       * and call the geometry creation routine.*/
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        HeavyMetalShieldGeometryPar config = createConfiguration(content);
        createGeometry(config, topVolume, type);
      }

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<HeavyMetalShieldGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<HeavyMetalShieldGeometryPar> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }
    };
  }
}

#endif /* GEOHEAVYMETALSHIELDCREATOR_H */
