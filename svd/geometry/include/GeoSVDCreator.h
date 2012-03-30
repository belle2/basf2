/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter, Jozef Koval                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSVDCREATOR_H
#define GEOSVDCREATOR_H

#include <vxd/geometry/GeoVXDCreator.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {

    /** The creator for the SVD geometry of the Belle II detector.   */
    class GeoSVDCreator : public VXD::GeoVXDCreator {
    public:
      /** Constructor of the GeoSVDCreator class. */
      GeoSVDCreator(): VXD::GeoVXDCreator("SVD") {};

      /** The destructor of the GeoSVDCreator class. */
      virtual ~GeoSVDCreator();

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
       * Read the sensor definitions from the database
       * @param sensors Reference to the database containing the parameters
       */
      virtual VXD::SensorInfoBase* createSensorInfo(const GearDir& sensor);

      /**
       * Return a SensitiveDetector implementation for a given sensor
       * @param sensorID  SensorID for the sensor
       * @param sensor    Information about the sensor to create the Sensitive Detector for
       * @param placement Information on how to place the sensor
       */
      virtual VXD::SensitiveDetectorBase* createSensitiveDetector(
        VxdID sensorID, const VXD::GeoVXDSensor& sensor, const VXD::GeoVXDSensorPlacement& placement);

    }; // class GeoSVDCreator
  } // namespace SVD
} //namespace Belle2

#endif /* GEOSVDCREATOR_H */
