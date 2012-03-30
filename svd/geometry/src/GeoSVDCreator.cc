/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geometry/GeoSVDCreator.h>
#include <vxd/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>

//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include <G4Point3D.hh>

#include <G4TessellatedSolid.hh>
#include <G4QuadrangularFacet.hh>
#include <G4TriangularFacet.hh>

//#define MATERIAL_SCAN

using namespace std;
using namespace boost;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {

    /** Register the creator */
    geometry::CreatorFactory<GeoSVDCreator> GeoSVDFactory("SVDCreator");

    GeoSVDCreator::~GeoSVDCreator()
    {
    }


    VXD::SensorInfoBase* GeoSVDCreator::createSensorInfo(const GearDir& sensor)
    {
      SensorInfo* info = new SensorInfo(
        VxdID(0, 0, 0),
        sensor.getLength("width"),
        sensor.getLength("length"),
        sensor.getLength("height"),
        sensor.getInt("stripsU"),
        sensor.getInt("stripsV"),
        sensor.getLength("width2", 0)
      );

      return info;
    }

    VXD::SensitiveDetectorBase* GeoSVDCreator::createSensitiveDetector(VxdID sensorID, const VXD::GeoVXDSensor& sensor, const VXD::GeoVXDSensorPlacement& placement)
    {
      SensorInfo* sensorInfo = new SensorInfo(dynamic_cast<SensorInfo&>(*sensor.info));
      sensorInfo->setID(sensorID);
      SensitiveDetector* sensitive = new SensitiveDetector(sensorInfo, m_seeNeutrons, m_onlyPrimaryTrueHits);
      return sensitive;
    }

    VXD::GeoVXDAssembly GeoSVDCreator::createHalfShellSupport(GearDir support)
    {
      VXD::GeoVXDAssembly supportAssembly;
      if (!support) return supportAssembly;

      return supportAssembly;
    }
  }
}
