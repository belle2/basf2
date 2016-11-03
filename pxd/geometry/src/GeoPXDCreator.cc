/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter, Benjamin Schwenker                        *
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
  using namespace geometry;
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

    void GeoPXDCreator::createGeometry(const PXDGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {

      m_activeStepSize = parameters.getGlobalParams().getActiveStepSize() / Unit::mm;
      m_activeChips = parameters.getGlobalParams().getActiveChips();
      m_seeNeutrons = parameters.getGlobalParams().getSeeNeutrons();
      m_onlyPrimaryTrueHits = parameters.getGlobalParams().getOnlyPrimaryTrueHits();
      m_distanceTolerance = parameters.getGlobalParams().getDistanceTolerance();
      m_electronTolerance = parameters.getGlobalParams().getElectronTolerance();
      m_minimumElectrons = parameters.getGlobalParams().getMinimumElectrons();
      m_onlyActiveMaterial = parameters.getGlobalParams().getOnlyActiveMaterial();
      m_defaultMaterial = parameters.getGlobalParams().getDefaultMaterial();

      G4Material* material = Materials::get(m_defaultMaterial);
      if (!material) B2FATAL("Default Material of VXD, '" << m_defaultMaterial << "', could not be found");


      //Build envelope
      G4LogicalVolume* envelope(0);
      G4VPhysicalVolume* physEnvelope{nullptr};
      if (!parameters.getEnvelope().getExists()) {
        B2INFO("Could not find definition for " + m_prefix + " Envelope, placing directly in top volume");
        envelope = &topVolume;
      } else {
        double minZ(0), maxZ(0);
        G4Polycone* envelopeCone = geometry::createRotationSolid("Envelope",
                                                                 parameters.getEnvelope().getInnerPoints(),
                                                                 parameters.getEnvelope().getOuterPoints(),
                                                                 parameters.getEnvelope().getMinPhi(),
                                                                 parameters.getEnvelope().getMaxPhi(),
                                                                 minZ, maxZ
                                                                );
        envelope = new G4LogicalVolume(envelopeCone, material, m_prefix + ".Envelope");
        setVisibility(*envelope, false);
        physEnvelope = new G4PVPlacement(getAlignmentFromDB(parameters.getAlignment(m_prefix)), envelope, m_prefix + ".Envelope",
                                         &topVolume, false, 1);
      }

      //Build all ladders including Sensors
      VXD::GeoVXDAssembly shellSupport = createHalfShellSupportFromDB(parameters);

      const std::vector<VXDHalfShellPar>& HalfShells = parameters.getHalfShells();
      for (const VXDHalfShellPar& shell : HalfShells) {
        string shellName =  shell.getName();
        G4Transform3D shellAlignment = getAlignmentFromDB(parameters.getAlignment(m_prefix + "." + shellName));

        //Place shell support
        double shellAngle = shell.getShellAngle();
        if (!m_onlyActiveMaterial) shellSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));

        B2INFO("Building " << m_prefix << " half-shell " << shellName << " shell angle " << shellAngle);

        const std::map< int, std::vector<std::pair<int, double>> >& Layers = shell.getLayers();
        for (const std::pair<const int, std::vector<std::pair<int, double>> >& layer : Layers) {
          int layerID = layer.first;
          const std::vector<std::pair<int, double>>& Ladders = layer.second;

          B2INFO(" layer " << layerID);

          //Place Layer support
          VXD::GeoVXDAssembly layerSupport = createLayerSupportFromDB(layerID, parameters);
          if (!m_onlyActiveMaterial) layerSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));
          VXD::GeoVXDAssembly ladderSupport = createLadderSupportFromDB(layerID, parameters);

          //Loop over defined ladders
          for (const std::pair<int, double>& ladder : Ladders) {
            int ladderID = ladder.first;
            double phi = ladder.second;
            B2INFO(" ladder " << ladderID << " has phi angle " << phi);
            G4Transform3D ladderPlacement = placeLadderFromDB(layerID, ladderID, phi, envelope, shellAlignment, parameters);
            if (!m_onlyActiveMaterial) ladderSupport.place(envelope, ladderPlacement);
          }

        }
      }

      //Now build cache with all transformations
      if (physEnvelope) {
        VXD::GeoCache::getInstance().findVolumes(physEnvelope);
      } else {
        //create a temporary placement of the top volume.
        G4PVPlacement topPlacement(nullptr, G4ThreeVector(0, 0, 0), &topVolume,
                                   "temp_Top", nullptr, false, 1, false);
        //and search for all VXD sensitive sensors within
        VXD::GeoCache::getInstance().findVolumes(&topPlacement);
      }

      //Create diamond radiation sensors if defined and in background mode
      if (m_activeChips) {
        createDiamondsFromDB(parameters.getRadiationSensors(), topVolume, *envelope);
      }


    }

    VXD::SensitiveDetectorBase* GeoPXDCreator::createSensitiveDetectorFromDB(VxdID sensorID, const VXDGeoSensorPar& sensor,
        const VXDGeoSensorPlacementPar& placement)
    {


      const PXDSensorInfoPar& infoPar = dynamic_cast<const PXDSensorInfoPar&>(*sensor.getSensorInfo());


      SensorInfo* sensorInfo = new SensorInfo(
        sensorID,
        infoPar.getWidth(),
        infoPar.getLength(),
        infoPar.getThickness(),
        infoPar.getUCells(),
        infoPar.getVCells1(),
        infoPar.getSplitLength(),
        infoPar.getVCells2()
      );
      sensorInfo->setDEPFETParams(
        infoPar.getBulkDoping() / (Unit::um * Unit::um * Unit::um),
        infoPar.getBackVoltage(),
        infoPar.getTopVoltage(),
        infoPar.getSourceBorderSmallPitch(),
        infoPar.getClearBorderSmallPitch(),
        infoPar.getDrainBorderSmallPitch(),
        infoPar.getSourceBorderLargePitch(),
        infoPar.getClearBorderLargePitch(),
        infoPar.getDrainBorderLargePitch(),
        infoPar.getGateDepth(),
        infoPar.getDoublePixel(),
        infoPar.getChargeThreshold(),
        infoPar.getNoiseFraction()
      );
      sensorInfo->setIntegrationWindow(
        infoPar.getIntegrationStart(),
        infoPar.getIntegrationEnd()
      );

      if (placement.getFlipV()) sensorInfo->flipVSegmentation();

      m_SensorInfo.push_back(sensorInfo);

      SensitiveDetector* sensitive = new SensitiveDetector(sensorInfo);
      return sensitive;
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

    VXD::GeoVXDAssembly GeoPXDCreator::createLayerSupportFromDB(int, const PXDGeometryPar& parameters) { return VXD::GeoVXDAssembly(); }

    VXD::GeoVXDAssembly GeoPXDCreator::createLadderSupportFromDB(int, const PXDGeometryPar& parameters) { return VXD::GeoVXDAssembly(); }

    VXD::GeoVXDAssembly GeoPXDCreator::createHalfShellSupportFromDB(const PXDGeometryPar& parameters)
    {
      VXD::GeoVXDAssembly supportAssembly;

      if (!parameters.getBuildSupport()) return supportAssembly;


      // Create the Endlanges
      const std::vector<VXDPolyConePar> Endflanges = parameters.getEndflanges();
      for (const VXDPolyConePar& endflange : Endflanges) {

        double minZ(0), maxZ(0);
        string name = endflange.getName();

        // Create a polycone
        double minPhi = endflange.getMinPhi();
        double dPhi   = endflange.getMaxPhi() - minPhi;
        int nPlanes = endflange.getPlanes().size();
        if (nPlanes < 2) {
          B2ERROR("Polycone needs at least two planes");
          return supportAssembly;
        }
        std::vector<double> z(nPlanes, 0);
        std::vector<double> rMin(nPlanes, 0);
        std::vector<double> rMax(nPlanes, 0);
        int index(0);
        minZ = numeric_limits<double>::infinity();
        maxZ = -numeric_limits<double>::infinity();

        const std::vector<VXDPolyConePlanePar> Planes = endflange.getPlanes();
        for (const VXDPolyConePlanePar& plane : Planes) {
          z[index]    = plane.getPosZ() / Unit::mm;
          minZ = min(minZ, z[index]);
          maxZ = max(maxZ, z[index]);
          rMin[index] = plane.getInnerRadius() / Unit::mm;
          rMax[index] = plane.getOuterRadius() / Unit::mm;
          ++index;
        }

        G4VSolid* supportCone = new G4Polycone(name, minPhi, dPhi, nPlanes, z.data(), rMin.data(), rMax.data());


        //Cutout boxes to make place for modules

        //We have the z dimensions of the polycon. Let's
        //add 1mm on each side to make sure we don't run into problems when the
        //surfaces match
        minZ -= 1. / Unit::mm;
        maxZ += 1. / Unit::mm;


        //Now get the number of cutouts and their size/position/angle
        int nCutouts  = parameters.getNCutOuts();
        double sizeX  = parameters.getCutOutWidth() / Unit::mm / 2.;
        double sizeY  = parameters.getCutOutHeight() / Unit::mm / 2.;
        double sizeZ  = (maxZ - minZ) / 2.;
        G4ThreeVector origin(
          parameters.getCutOutShift() / Unit::mm,
          parameters.getCutOutRPhi() / Unit::mm,
          minZ + sizeZ
        );

        double phi0   = parameters.getCutOutStartPhi();
        double dphi   = parameters.getCutOutDeltaPhi();
        for (int i = 0; i < nCutouts; ++i) {
          G4Box* box = new G4Box("Cutout", sizeX, sizeY, sizeZ);
          G4Transform3D placement = G4RotateZ3D(phi0 + i * dphi) * G4Translate3D(origin);
          supportCone = new G4SubtractionSolid("PXD Support endflange", supportCone, box, placement);
        }


        string materialName = endflange.getMaterial();
        G4Material* material = geometry::Materials::get(materialName);
        if (!material) B2FATAL("Material '" << materialName << "', required by PXD component " << name << ", could not be found");

        G4LogicalVolume* volume = new G4LogicalVolume(supportCone, material, name);
        geometry::setColor(*volume, "#ccc4");
        supportAssembly.add(volume);

      }


      //Create Carbon cooling tubes
      {
        int nTubes    = parameters.getNTubes();
        double minZ   = parameters.getTubesMinZ() / Unit::mm;
        double maxZ   = parameters.getTubesMaxZ() / Unit::mm;
        double minR   = parameters.getTubesMinR() / Unit::mm;
        double maxR   = parameters.getTubesMaxR() / Unit::mm;
        double sizeZ  = (maxZ - minZ) / 2.;
        double shiftX = parameters.getTubesRPhi() / Unit::mm;
        double shiftY = 0;
        double shiftZ = minZ + sizeZ;
        double phi0   = parameters.getTubesStartPhi();
        double dphi   = parameters.getTubesDeltaPhi();
        string material = parameters.getTubesMaterial();

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
