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
#include <beast/beamabort/dbobjects/BeamabortGeo.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the BEAMABORT detector */
  namespace beamabort {

    class SensitiveDetector;

    /** The creator for the BEAMABORT geometry. */
    class BeamabortCreator : public geometry::CreatorBase {
    private:
      /** Reads beamabort geometry parameters from the xml files and createst DB class BeamabortGeo */
      BeamabortGeo createConfiguration(const GearDir& param)
      {
        BeamabortGeo BeamabortGeoConfig;
        BeamabortGeoConfig.initialize(param);
        return BeamabortGeoConfig;
      }

      /**
       * Create detector geometry
       * @param topVolume Geant4 logical top volume.
       * @param type Geometry type.
       */
      void createGeometry(G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    public:
      /**
       * Constructor
       */
      BeamabortCreator();

      /**
       * Destructor
       */
      virtual ~BeamabortCreator();
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
        DBStore::Instance().addConstantOverride("BeamabortGeo", new BeamabortGeo(m_config));

        createGeometry(topVolume, type);
      }

      /** creates DB payload for BeamabortGeo class */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<BeamabortGeo> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<BeamabortGeo> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        m_config = *dbObj;
        createGeometry(topVolume, type);
      }

    protected:
      BeamabortGeo m_config; /**< geometry parameters object */
    };

  }
}
