/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *               Leonid Burmistrov                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOARICHCREATOR_H
#define GEOARICHCREATOR_H

#include <geometry/CreatorBase.h>
#include <framework/logging/Logger.h>
#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <arich/dbobjects/ARICHModulesInfo.h>

#include <G4AssemblyVolume.hh>

class G4LogicalVolume;
class G4Material;

namespace Belle2 {

  class GearDir;

  namespace arich {

    class SensitiveDetector;
    class SensitiveAero;

    /** The creator for the ARICH geometry of the Belle II detector.   */
    class GeoARICHCreator : public geometry::CreatorBase {

    private:

      /** Reads ARICH geometry parameters from the xml files and createst DB class ARICHGeometryConfig */
      ARICHGeometryConfig createConfiguration(const GearDir& param)
      {
        ARICHGeometryConfig arichGeometryConfig(param);
        return arichGeometryConfig;
      }

      /** Create detector geometry */
      void createGeometry(G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    public:

      /** Constructor of the GeoARICHCreator class. */
      GeoARICHCreator();

      /** The destructor of the GeoARICHreator class. */
      virtual ~GeoARICHCreator();

      /**
       * Creates the ROOT Objects for the ARICH geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the ROOT
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        m_config = createConfiguration(content);

        // override geometry configuration from the DB
        DBStore::Instance().addConstantOverride("ARICHGeometryConfig", new ARICHGeometryConfig(m_config));

        createGeometry(topVolume, type);
      }

      /** creates DB payload for ARICHGeometryConfig class */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<ARICHGeometryConfig> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<ARICHGeometryConfig> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        m_config = *dbObj;
        createGeometry(topVolume, type);
      }

    private:

      //! build the HAPD modules
      G4LogicalVolume* buildHAPD(const ARICHGeoHAPD& hapdPar);

      //! build the merger PCB logical volume
      G4LogicalVolume* buildMerger(const ARICHGeoMerger& mergerGeo);

      //! build the cables envelop with effective material describing cables
      G4LogicalVolume* buildCables(const ARICHGeoCablesEnvelope& cablesGeo);

      //! build mirrors
      G4LogicalVolume* buildMirror(const ARICHGeometryConfig& detectorGeo);

      //! build detector plane
      G4LogicalVolume* buildDetectorPlane(const ARICHGeometryConfig& detectorGeo);

      //! build merger PCB assembly envelope plane
      G4LogicalVolume* buildMergerPCBEnvelopePlane(const ARICHGeometryConfig& detectorGeo);

      //! build cooling system assembly envelope plane
      G4LogicalVolume* buildCoolingEnvelopePlane(const ARICHGeoCooling& coolingGeo);

      //! build cooling tube (G4Tubs)
      G4LogicalVolume* buildCoolingTube(const unsigned i_volumeID, const ARICHGeoCooling& coolingGeo);

      //! build cooling tube (G4Torus)
      G4LogicalVolume* buildCoolingTorus(const unsigned i_volumeID, const ARICHGeoCooling& coolingGeo);

      //! build cooling test plates
      G4LogicalVolume* buildCoolingTestPlate(const ARICHGeoCooling& coolingGeo);

      //! build aerogel plane
      G4LogicalVolume* buildAerogelPlane(const ARICHGeometryConfig& detectorGeo);

      //! build aerogel plane with average properties of aerogel per layer
      G4LogicalVolume* buildAerogelPlaneAveragedOverLayers(const ARICHGeometryConfig& detectorGeo);

      //! with individual properties of aerogel tiles
      G4LogicalVolume* buildAerogelPlaneWithIndividualTilesProp(const ARICHGeometryConfig& detectorGeo);

      //! build simple aerogel plane (for cosmic test)
      G4LogicalVolume* buildSimpleAerogelPlane(const ARICHGeometryConfig& detectorGeo);

      //! build detector support plate
      G4LogicalVolume* buildDetectorSupportPlate(const ARICHGeometryConfig& detectorGeo);

      //! build joints of the ARICH support structure
      G4AssemblyVolume* makeJoint(G4Material* supportMaterial, const std::vector<double>& pars);

      //! get refractive index of the material
      double getAvgRINDEX(G4Material* material);

      //! geometry configuration
      ARICHGeometryConfig m_config;

      //! pointer to sensitive detector
      SensitiveDetector* m_sensitive;

      //! pointer to sensitive aerogel - used instead of tracking
      SensitiveAero* m_sensitiveAero;

      //! flag the beam background study
      int m_isBeamBkgStudy;

      DBObjPtr<ARICHModulesInfo> m_modInfo; /**< information on installed modules from the DB */
    };

  }
}

#endif /* GEOARICHCREATOR_H */
