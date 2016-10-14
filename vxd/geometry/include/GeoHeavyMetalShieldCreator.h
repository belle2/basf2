/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jacek Stypula, Benjamin Schwenker                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOHEAVYMETALSHIELDCREATOR_H
#define GEOHEAVYMETALSHIELDCREATOR_H

#include <geometry/CreatorBase.h>

#include <framework/logging/Logger.h>
#include <vxd/dbobjects/HeavyMetalShieldGeometryPar.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {

    /** The creator for the HeavyMetalShield geometry of the Belle II detector.   */
    class GeoHeavyMetalShieldCreator : public geometry::CreatorBase {
    private:
      //! Create a parameter object from the Gearbox XML parameters.
      HeavyMetalShieldGeometryPar createConfiguration(const GearDir& param)
      {
        HeavyMetalShieldGeometryPar heavyMetalShieldGeometryPar;
        heavyMetalShieldGeometryPar.read(param);
        return heavyMetalShieldGeometryPar;
      };

      //! Create the geometry from a parameter object.
      void createGeometry(const HeavyMetalShieldGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    public:
      /**
       * Creates the ROOT Objects for the HeavyMetalShield geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

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
