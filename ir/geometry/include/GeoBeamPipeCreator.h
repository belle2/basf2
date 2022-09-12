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
#include <ir/dbobjects/BeamPipeGeo.h>
#include <string>
#include <vector>

class G4LogicalVolume;

namespace Belle2 {
  namespace ir {
    class SensitiveDetector;

    /** The creator for the BeamPipe geometry of the Belle II detector.   */
    class GeoBeamPipeCreator : public geometry::CreatorBase {

    private:
      /** Reads IR geometry parameters from the xml files and createst DB class BeamPipeGeo */
      BeamPipeGeo createConfiguration(const GearDir& param)
      {
        BeamPipeGeo BeamPipeGeoConfig;
        BeamPipeGeoConfig.initialize(param);
        return BeamPipeGeoConfig;
      }

      /**
       * Create detector geometry
       * @param topVolume Geant4 logical top volume.
       * @param type Geometry type.
       */
      void createGeometry(G4LogicalVolume& topVolume, geometry::GeometryTypes type);


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
      * @param topVolume Geant4 logical top volume.
      * @param type Geometry type.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        m_config = createConfiguration(content);

        // override geometry configuration from the DB
        DBStore::Instance().addConstantOverride("BeamPipeGeo", new BeamPipeGeo(m_config));

        createGeometry(topVolume, type);
      }

      /** creates DB payload for BeamPipeGeo class */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<BeamPipeGeo> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<BeamPipeGeo> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        m_config = *dbObj;
        createGeometry(topVolume, type);
      }

    protected:
      /** Sensitive detector. **/
      std::vector<SensitiveDetector*> m_sensitive;

      BeamPipeGeo m_config; /**< geometry parameters object */
    };

  }
}
