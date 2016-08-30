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

#include <pxd/geometry/GeoPXDCreator.h>
#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>

using namespace std;
using namespace boost;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {

    /** Register the creator */
    geometry::CreatorFactory<GeoPXDCreator> GeoPXDFactory("PXDCreator");

    GeoPXDCreator::~GeoPXDCreator()
    {
      for (SensorInfo* sensorInfo : m_SensorInfo) delete sensorInfo;
      m_SensorInfo.clear();
    }

    VXD::SensorInfoBase* GeoPXDCreator::createSensorInfo(const GearDir& sensor)
    {
      SensorInfo* info = new SensorInfo(
        VxdID(0, 0, 0),
        sensor.getLength("width"),
        sensor.getLength("length"),
        sensor.getLength("height"),
        sensor.getInt("pixelsU"),
        sensor.getInt("pixelsV[1]"),
        sensor.getLength("splitLength", 0),
        sensor.getInt("pixelsV[2]", 0)
      );
      info->setDEPFETParams(
        sensor.getDouble("BulkDoping") / (Unit::um * Unit::um * Unit::um),
        sensor.getWithUnit("BackVoltage"),
        sensor.getWithUnit("TopVoltage"),
        sensor.getLength("SourceBorderSmallPixel"),
        sensor.getLength("ClearBorderSmallPixel"),
        sensor.getLength("DrainBorderSmallPixel"),
        sensor.getLength("SourceBorderLargePixel"),
        sensor.getLength("ClearBorderLargePixel"),
        sensor.getLength("DrainBorderLargePixel"),
        sensor.getLength("GateDepth"),
        sensor.getBool("DoublePixel"),
        sensor.getDouble("ChargeThreshold"),
        sensor.getDouble("NoiseFraction")
      );
      info->setIntegrationWindow(
        sensor.getTime("IntegrationStart"),
        sensor.getTime("IntegrationEnd")
      );
      m_SensorInfo.push_back(info);
      return info;
    }

    VXD::SensitiveDetectorBase* GeoPXDCreator::createSensitiveDetector(VxdID sensorID, const VXDGeoSensor& sensor,
        const VXDGeoSensorPlacement& placement)
    {
      SensorInfo* sensorInfo = new SensorInfo(dynamic_cast<const SensorInfo&>(*sensor.getSensorInfo()));
      sensorInfo->setID(sensorID);
      if (placement.getFlipV()) sensorInfo->flipVSegmentation();
      SensitiveDetector* sensitive = new SensitiveDetector(sensorInfo);
      return sensitive;
    }

    VXD::GeoVXDAssembly GeoPXDCreator::createHalfShellSupport(GearDir support)
    {
      VXD::GeoVXDAssembly supportAssembly;
      if (!support) return supportAssembly;

      for (const GearDir& endflange : support.getNodes("Endflange")) {
        double minZ(0), maxZ(0);
        string name = endflange.getString("@name");
        G4VSolid* supportCone = geometry::createPolyCone(name, endflange, minZ, maxZ);

        //Cutout boxes to make place for modules

        //We get the z dimensions of the polycone from createPolycone. Let's
        //add 1mm on each side to make sure we don't run into problems when the
        //surfaces match
        minZ -= 1. / Unit::mm;
        maxZ += 1. / Unit::mm;

        //Now get the number of cutouts and their size/position/angle
        int nCutouts  = support.getInt("Cutout/count");
        double sizeX  = support.getLength("Cutout/width") / Unit::mm / 2.;
        double sizeY  = support.getLength("Cutout/height") / Unit::mm / 2.;
        double sizeZ  = (maxZ - minZ) / 2.;
        G4ThreeVector origin(
          support.getLength("Cutout/shift") / Unit::mm,
          support.getLength("Cutout/rphi") / Unit::mm,
          minZ + sizeZ
        );
        double phi0   = support.getAngle("Cutout/startPhi");
        double dphi   = support.getAngle("Cutout/deltaPhi");
        for (int i = 0; i < nCutouts; ++i) {
          G4Box* box = new G4Box("Cutout", sizeX, sizeY, sizeZ);
          G4Transform3D placement = G4RotateZ3D(phi0 + i * dphi) * G4Translate3D(origin);
          supportCone = new G4SubtractionSolid("PXD Support endflange", supportCone, box, placement);
        }

        string materialName = endflange.getString("Material", "Air");
        G4Material* material = geometry::Materials::get(materialName);
        if (!material) B2FATAL("Material '" << materialName << "', required by PXD component " << name << ", could not be found");

        G4LogicalVolume* volume = new G4LogicalVolume(supportCone, material, name);
        geometry::setColor(*volume, endflange.getString("color", "#ccc4"));
        supportAssembly.add(volume);
      }

      //Create Carbon cooling tubes
      {
        int nTubes    = support.getInt("CarbonTubes/count");
        double minZ   = support.getLength("CarbonTubes/minZ") / Unit::mm;
        double maxZ   = support.getLength("CarbonTubes/maxZ") / Unit::mm;
        double minR   = support.getLength("CarbonTubes/innerRadius") / Unit::mm;
        double maxR   = support.getLength("CarbonTubes/outerRadius") / Unit::mm;
        double sizeZ  = (maxZ - minZ) / 2.;
        double shiftX = support.getLength("CarbonTubes/rphi") / Unit::mm;
        double shiftY = 0;
        double shiftZ = minZ + sizeZ;
        double phi0   = support.getAngle("CarbonTubes/startPhi");
        double dphi   = support.getAngle("CarbonTubes/deltaPhi");
        string material = support.getString("CarbonTubes/Material", "Carbon");

        G4Tubs* tube = new G4Tubs("CarbonTube", minR, maxR, sizeZ, 0, 2 * M_PI);
        G4LogicalVolume* tubeVol = new G4LogicalVolume(tube, geometry::Materials::get(material), "CarbonTube");
        geometry::setColor(*tubeVol, "#000");
        for (int i = 0; i < nTubes; ++i) {
          G4Transform3D placement = G4RotateZ3D(phi0 + i * dphi) * G4Translate3D(shiftX, shiftY, shiftZ);
          supportAssembly.add(tubeVol, placement);
        }
      }

      return supportAssembly;
    }
  }
}
