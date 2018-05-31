/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <ir/dbobjects/FarBeamLineGeo.h>
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

    private:
      /** Reads IR geometry parameters from the xml files and createst DB class FarBeamLineGeo */
      FarBeamLineGeo createConfiguration(const GearDir& param)
      {
        FarBeamLineGeo FarBeamLineGeoConfig;
        FarBeamLineGeoConfig.initialize(param);
        return FarBeamLineGeoConfig;
      }

      /** Create detector geometry */
      void createGeometry(G4LogicalVolume& topVolume, geometry::GeometryTypes type);


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
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        m_config = createConfiguration(content);

        // override geometry configuration from the DB
        DBStore::Instance().addConstantOverride("FarBeamLineGeo", new FarBeamLineGeo(m_config));

        createGeometry(topVolume, type);
      }

      /** creates DB payload for FarBeamLineGeo class */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<FarBeamLineGeo> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<FarBeamLineGeo> dbObj;
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
      FarBeamLineGeo m_config; /**< geometry parameters object */
    };

  }
}
