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

#ifndef GEOCRYOSTATCREATOR_H_
#define GEOCRYOSTATCREATOR_H_

#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <ir/dbobjects/CryostatGeo.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>


#include <G4Transform3D.hh>
#include <G4VSolid.hh>
class G4LogicalVolume;
class G4AssemblyVolume;
class G4Polycone;

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

      /** Create detector geometry */
      void createGeometry(G4LogicalVolume& topVolume, geometry::GeometryTypes type);


    public:

      /** Constructor of the GeoCryostatCreator class. */
      GeoCryostatCreator();

      /** The destructor of the GeoCryostatCreator class. */
      virtual ~GeoCryostatCreator();

      /**
      * Creates the ROOT Objects for the BeamPipe geometry.
      * @param content A reference to the content part of the parameter
      *                description, which should to be used to create the ROOT
      *                objects.
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
      CryostatGeo m_config;
    };

  }
}

#endif /* GEOCryostatCREATOR_H_ */
