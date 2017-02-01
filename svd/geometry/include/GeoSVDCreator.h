/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter, Jozef Koval, Benjamin Schwenker           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSVDCREATOR_H
#define GEOSVDCREATOR_H

#include <vector>
#include <vxd/geometry/GeoVXDCreator.h>
#include <svd/dbobjects/SVDGeometryPar.h>


namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {

    class SensorInfo;

    /** The creator for the SVD geometry of the Belle II detector.   */
    class GeoSVDCreator : public VXD::GeoVXDCreator {
    private:
      //! Create a parameter object from the Gearbox XML parameters.
      SVDGeometryPar createConfiguration(const GearDir& param);

      //! Create the geometry from a parameter object.
      void createGeometry(const SVDGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    public:
      /** Constructor of the GeoSVDCreator class. */
      GeoSVDCreator(): VXD::GeoVXDCreator("SVD") {};

      /** The destructor of the GeoSVDCreator class. */
      virtual ~GeoSVDCreator();

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<SVDGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<SVDGeometryPar> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }

      /**
       * Create support structure for SVD Half Shell, that means everything
       * thagt does not depend on layer or sensor alignment
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createHalfShellSupport(GearDir support);

      /**
       * Create support structure for a SVD Layer
       * @param layer Layer ID to create the support for
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createLayerSupport(int layer, GearDir support);

      /**
       * Create support structure for a SVD Ladder
       * @param layer Layer ID to create the support for
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createLadderSupport(int layer, GearDir support);

      /**
       * Create support structure for a SVD Layer
       * @param layer Layer ID to create the support
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createLayerSupportFromDB(int, const SVDGeometryPar& parameters);


      /**
       * Create support structure for a SVD Ladder
       * @param layer Layer ID to create the support
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createLadderSupportFromDB(int, const SVDGeometryPar& parameters);


      /**
       * Create support structure for SVD Half Shell, that means everything
       * thagt does not depend on layer or sensor alignment
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createHalfShellSupportFromDB(const SVDGeometryPar& parameters);

      /**
       * Read the sensor definitions from the database
       * @param sensors Reference to the database containing the parameters
       */
      virtual VXD::SensorInfoBase* createSensorInfo(const GearDir& sensor);

      /**
       * Read the sensor definitions from the database
       * @param sensors Reference to the database containing the parameters
       */
      virtual VXD::SensorInfoBase* createSensorInfoFromDB(const VXDGeoSensorPar& sensor);

      /**
       * Return a SensitiveDetector implementation for a given sensor
       * @param sensorID  SensorID for the sensor
       * @param sensor    Information about the sensor to create the Sensitive Detector for
       * @param placement Information on how to place the sensor
       */
      virtual VXD::SensitiveDetectorBase* createSensitiveDetector(
        VxdID sensorID, const VXDGeoSensor& sensor, const VXDGeoSensorPlacement& placement);



    private:

      /** Vector of pointers to SensorInfo objects */
      std::vector<SensorInfo*> m_SensorInfo;

    }; // class GeoSVDCreator
  } // namespace SVD
} //namespace Belle2

#endif /* GEOSVDCREATOR_H */
