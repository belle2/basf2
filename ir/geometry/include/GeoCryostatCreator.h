/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <ir/dbobjects/CryostatGeo.h>
#include <string>

#include <G4Transform3D.hh>
#include <G4VSolid.hh>
class G4LogicalVolume;

namespace Belle2 {
  namespace ir {
    /** The struct for CryostatElement */
    struct CryostatElement {
      /** Transformation */
      G4Transform3D transform;
      /** Solid volume */
      G4VSolid* geo;
      /** Logical volume */
      G4LogicalVolume* logi;
    };

    class SensitiveDetector;

    /** The creator for the Cryostat geometry of the Belle II detector.   */
    class GeoCryostatCreator : public geometry::CreatorBase {

    private:
      /** Reads IR geometry parameters from the xml files and createst DB class CryostatGeo */
      CryostatGeo createConfiguration(const GearDir& param)
      {
        CryostatGeo CryostatGeoConfig;
        CryostatGeoConfig.initialize(param);
        return CryostatGeoConfig;
      }

      /**
       * Create detector geometry
       * @param topVolume Geant4 logical top volume.
       * @param type Geometry type.
       */
      void createGeometry(G4LogicalVolume& topVolume, geometry::GeometryTypes type);


    public:

      /** Constructor of the GeoCryostatCreator class. */
      GeoCryostatCreator();

      /** The destructor of the GeoCryostatCreator class. */
      virtual ~GeoCryostatCreator();

      /** Do not want a copy constructor */
      GeoCryostatCreator(const GeoCryostatCreator&) = delete;

      /** Do not want an assignment operator */
      GeoCryostatCreator& operator =(const GeoCryostatCreator&) = delete;

      /**
      * Creates the ROOT Objects for the Cryostat geometry.
      * @param content A reference to the content part of the parameter
      *                description, which should to be used to create the ROOT
      *                objects.
      * @param topVolume Geant4 logical top volume.
      * @param type Geometry type.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        m_config = createConfiguration(content);

        // override geometry configuration from the DB
        DBStore::Instance().addConstantOverride("CryostatGeo", new CryostatGeo(m_config));

        createGeometry(topVolume, type);
      }

      /** creates DB payload for CryostatGeo class */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<CryostatGeo> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<CryostatGeo> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        m_config = *dbObj;
        createGeometry(topVolume, type);
      }


    protected:
      /** Sensitive detector. **/
      SensitiveDetector* m_sensitive;
      CryostatGeo m_config; /**< geometry parameters object */
    };

  }
}
