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

#include <simulation/background/BkgSensitiveDetector.h>

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

    VXD::SensorInfoBase* GeoPXDCreator::createSensorInfo(const VXDGeoSensorPar& sensor)
    {

      const PXDSensorInfoPar& infoPar = dynamic_cast<const PXDSensorInfoPar&>(*sensor.getSensorInfo());

      SensorInfo* info = new SensorInfo(
        VxdID(0, 0, 0),
        infoPar.getWidth(),
        infoPar.getLength(),
        infoPar.getThickness(),
        infoPar.getUCells(),
        infoPar.getVCells1(),
        infoPar.getSplitLength(),
        infoPar.getVCells2()
      );
      info->setDEPFETParams(
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
      info->setIntegrationWindow(
        infoPar.getIntegrationStart(),
        infoPar.getIntegrationEnd()
      );

      m_SensorInfo.push_back(info);
      return info;
    }

    PXDSensorInfoPar* GeoPXDCreator::readSensorInfo(const GearDir& sensor)
    {
      PXDSensorInfoPar* info = new PXDSensorInfoPar(
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
        sensor.getDouble("BulkDoping"),
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
      return info;
    }


    PXDGeometryPar GeoPXDCreator::createConfiguration(const GearDir& content)
    {
      // Create an empty payload
      PXDGeometryPar pxdGeometryPar;

      //Read prefix ('SVD' or 'PXD')
      pxdGeometryPar.setPrefix(m_prefix);

      //Read some global parameters
      VXDGlobalPar globals((float)content.getDouble("ElectronTolerance", 100),
                           (float)content.getDouble("MinimumElectrons", 10),
                           content.getLength("ActiveStepSize", 0.005),
                           content.getBool("ActiveChips", false),
                           content.getBool("SeeNeutrons", false),
                           content.getBool("OnlyPrimaryTrueHits", false),
                           content.getBool("OnlyActiveMaterial", false),
                           (float)content.getLength("DistanceTolerance", 0.005),
                           content.getString("DefaultMaterial", "Air")
                          );
      pxdGeometryPar.setGlobalParams(globals);

      //Read envelope parameters
      GearDir envelopeParams(content, "Envelope/");
      VXDEnvelopePar envelope(envelopeParams.getString("Name", ""),
                              envelopeParams.getString("Material", "Air"),
                              envelopeParams.getString("Color", ""),
                              envelopeParams.getAngle("minPhi", 0),
                              envelopeParams.getAngle("maxPhi", 2 * M_PI),
                              (envelopeParams.getNodes("InnerPoints/point").size() > 0)
                             );

      for (const GearDir point : envelopeParams.getNodes("InnerPoints/point")) {
        pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
        envelope.getInnerPoints().push_back(ZXPoint);
      }
      for (const GearDir point : envelopeParams.getNodes("OuterPoints/point")) {
        pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
        envelope.getOuterPoints().push_back(ZXPoint);
      }
      pxdGeometryPar.setEnvelope(envelope);

      // Read alignment for detector m_prefix ('PXD' or 'SVD')
      string pathAlign = (boost::format("Align[@component='%1%']/") % m_prefix).str();
      GearDir paramsAlign(GearDir(content, "Alignment/"), pathAlign);
      if (!paramsAlign) {
        B2WARNING("Could not find alignment parameters for component " << m_prefix);
        return pxdGeometryPar;
      }
      pxdGeometryPar.getAlignmentMap()[m_prefix] = VXDAlignmentPar(paramsAlign.getLength("du"),
                                                   paramsAlign.getLength("dv"),
                                                   paramsAlign.getLength("dw"),
                                                   paramsAlign.getAngle("alpha"),
                                                   paramsAlign.getAngle("beta"),
                                                   paramsAlign.getAngle("gamma")
                                                                  );

      //Read the definition of all sensor types
      GearDir components(content, "Components/");
      for (const GearDir& paramsSensor : components.getNodes("Sensor")) {
        string sensorTypeID = paramsSensor.getString("@type");

        VXDGeoSensorPar sensor(paramsSensor.getString("Material"),
                               paramsSensor.getString("Color", ""),
                               paramsSensor.getLength("width"),
                               paramsSensor.getLength("width2", 0),
                               paramsSensor.getLength("length"),
                               paramsSensor.getLength("height"),
                               paramsSensor.getAngle("angle", 0),
                               paramsSensor.getBool("@slanted", false)
                              );
        sensor.setActive(VXDGeoComponentPar(
                           paramsSensor.getString("Material"),
                           paramsSensor.getString("Active/Color", "#f00"),
                           paramsSensor.getLength("Active/width"),
                           paramsSensor.getLength("Active/width2", 0),
                           paramsSensor.getLength("Active/length"),
                           paramsSensor.getLength("Active/height")
                         ), VXDGeoPlacementPar(
                           "Active",
                           paramsSensor.getLength("Active/u"),
                           paramsSensor.getLength("Active/v"),
                           paramsSensor.getString("Active/w", "center"),
                           paramsSensor.getLength("Active/woffset", 0)
                         ));

        PXDSensorInfoPar* pxdInfo = readSensorInfo(GearDir(paramsSensor, "Active"));
        sensor.setSensorInfo(pxdInfo);
        sensor.setComponents(getSubComponents(paramsSensor));
        pxdGeometryPar.getSensorMap()[sensorTypeID] = sensor;
        pxdGeometryPar.getSensorInfos().push_back(pxdInfo);
      }

      //Build all ladders including Sensors
      GearDir support(content, "Support/");
      readHalfShellSupport(support, pxdGeometryPar);

      for (const GearDir& shell : content.getNodes("HalfShell")) {

        string shellName = m_prefix + "." + shell.getString("@name");
        string pathShell = (boost::format("Align[@component='%1%']/") % shellName).str();
        GearDir paramsShell(GearDir(content, "Alignment/"), pathShell);
        if (!paramsShell) {
          B2WARNING("Could not find alignment parameters for component " << shellName);
          return pxdGeometryPar;
        }
        pxdGeometryPar.getAlignmentMap()[shellName] = VXDAlignmentPar(paramsShell.getLength("du"),
                                                      paramsShell.getLength("dv"),
                                                      paramsShell.getLength("dw"),
                                                      paramsShell.getAngle("alpha"),
                                                      paramsShell.getAngle("beta"),
                                                      paramsShell.getAngle("gamma")
                                                                     );

        VXDHalfShellPar halfShell(shell.getString("@name") , shell.getAngle("shellAngle", 0));

        for (const GearDir& layer : shell.getNodes("Layer")) {
          int layerID = layer.getInt("@id");

          readLadder(layerID, components, pxdGeometryPar);

          //Loop over defined ladders
          for (const GearDir& ladder : layer.getNodes("Ladder")) {
            int ladderID = ladder.getInt("@id");
            double phi = ladder.getAngle("phi", 0);
            readLadderComponents(layerID, ladderID, content, pxdGeometryPar);
            halfShell.addLadder(layerID, ladderID,  phi);
          }
        }
        pxdGeometryPar.getHalfShells().push_back(halfShell);
      }

      //Create diamond radiation sensors if defined and in background mode
      GearDir radiationDir(content, "RadiationSensors");
      if (pxdGeometryPar.getGlobalParams().getActiveChips() &&  radiationDir) {
        VXDGeoRadiationSensorsPar radiationSensors(
          m_prefix,
          radiationDir.getBool("insideEnvelope"),
          radiationDir.getLength("width"),
          radiationDir.getLength("length"),
          radiationDir.getLength("height"),
          radiationDir.getString("material")
        );

        //Add radiation sensor positions
        for (GearDir& position : radiationDir.getNodes("position")) {
          VXDGeoRadiationSensorsPositionPar diamonds(position.getLength("z"),
                                                     position.getLength("radius"),
                                                     position.getAngle("theta")
                                                    );

          //Loop over all phi positions
          for (GearDir& sensor : position.getNodes("phi")) {
            //Add sensor with angle and id
            diamonds.addSensor(sensor.getInt("@id"), sensor.getAngle());
          }
          radiationSensors.addPosition(diamonds);
        }
        pxdGeometryPar.setRadiationSensors(radiationSensors);
      }

      return pxdGeometryPar;
    }

    void GeoPXDCreator::readHalfShellSupport(GearDir support, PXDGeometryPar& pxdGeometryPar)
    {
      if (!support) return;

      for (const GearDir& endflange : support.getNodes("Endflange")) {
        VXDPolyConePar endflangePar(
          endflange.getString("@name"),
          endflange.getString("Material", "Air"),
          endflange.getAngle("minPhi", 0),
          endflange.getAngle("maxPhi", 2 * M_PI),
          (endflange.getNodes("Cutout").size() > 0),
          endflange.getLength("Cutout/width", 0.),
          endflange.getLength("Cutout/height", 0.),
          endflange.getLength("Cutout/depth", 0.)
        );

        for (const GearDir& plane : endflange.getNodes("Plane")) {
          VXDPolyConePlanePar planePar(
            plane.getLength("posZ"),
            plane.getLength("innerRadius"),
            plane.getLength("outerRadius")
          );
          endflangePar.getPlanes().push_back(planePar);
        }
        pxdGeometryPar.getEndflanges().push_back(endflangePar);
      }

      // Cout outs for endflanges
      pxdGeometryPar.setNCutOuts(support.getInt("Cutout/count"));
      pxdGeometryPar.setCutOutWidth(support.getLength("Cutout/width"));
      pxdGeometryPar.setCutOutHeight(support.getLength("Cutout/height"));
      pxdGeometryPar.setCutOutShift(support.getLength("Cutout/shift"));
      pxdGeometryPar.setCutOutRPhi(support.getLength("Cutout/rphi"));
      pxdGeometryPar.setCutOutStartPhi(support.getAngle("Cutout/startPhi"));
      pxdGeometryPar.setCutOutDeltaPhi(support.getAngle("Cutout/deltaPhi"));

      //Create Carbon cooling tubes
      pxdGeometryPar.setNTubes(support.getInt("CarbonTubes/count"));
      pxdGeometryPar.setTubesMinZ(support.getLength("CarbonTubes/minZ"));
      pxdGeometryPar.setTubesMaxZ(support.getLength("CarbonTubes/maxZ"));
      pxdGeometryPar.setTubesMinR(support.getLength("CarbonTubes/innerRadius"));
      pxdGeometryPar.setTubesMaxR(support.getLength("CarbonTubes/outerRadius"));
      pxdGeometryPar.setTubesRPhi(support.getLength("CarbonTubes/rphi"));
      pxdGeometryPar.setTubesStartPhi(support.getAngle("CarbonTubes/startPhi"));
      pxdGeometryPar.setTubesDeltaPhi(support.getAngle("CarbonTubes/deltaPhi"));
      pxdGeometryPar.setTubesMaterial(support.getString("CarbonTubes/Material", "Carbon"));

      return;
    }

    void GeoPXDCreator::createGeometry(const PXDGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes)
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
        physEnvelope = new G4PVPlacement(getAlignment(parameters.getAlignment(m_prefix)), envelope, m_prefix + ".Envelope",
                                         &topVolume, false, 1);
      }

      //Read the definition of all sensor types
      for (const pair<const string, VXDGeoSensorPar>& typeAndSensor : parameters.getSensorMap()) {
        const string& sensorTypeID = typeAndSensor.first;
        const VXDGeoSensorPar& paramsSensor = typeAndSensor.second;
        VXDGeoSensor sensor(
          paramsSensor.getMaterial(),
          paramsSensor.getColor(),
          paramsSensor.getWidth() / Unit::mm,
          paramsSensor.getWidth2() / Unit::mm,
          paramsSensor.getLength() / Unit::mm,
          paramsSensor.getHeight() / Unit::mm,
          paramsSensor.getSlanted()
        );
        sensor.setActive(VXDGeoComponent(
                           paramsSensor.getMaterial(),
                           paramsSensor.getActiveArea().getColor(),
                           paramsSensor.getActiveArea().getWidth() / Unit::mm,
                           paramsSensor.getActiveArea().getWidth2() / Unit::mm,
                           paramsSensor.getActiveArea().getLength() / Unit::mm,
                           paramsSensor.getActiveArea().getHeight() / Unit::mm
                         ), VXDGeoPlacement(
                           "Active",
                           paramsSensor.getActivePlacement().getU() / Unit::mm,
                           paramsSensor.getActivePlacement().getV() / Unit::mm,
                           paramsSensor.getActivePlacement().getW(),
                           paramsSensor.getActivePlacement().getWOffset() / Unit::mm
                         ));
        sensor.setSensorInfo(createSensorInfo(paramsSensor));

        vector<VXDGeoPlacement> subcomponents;
        for (const VXDGeoPlacementPar& component : paramsSensor.getComponents()) {
          subcomponents.push_back(VXDGeoPlacement(
                                    component.getName(),
                                    component.getU() / Unit::mm,
                                    component.getV() / Unit::mm,
                                    component.getW(),
                                    component.getWOffset() / Unit::mm
                                  ));
        }
        sensor.setComponents(subcomponents);
        m_sensorMap[sensorTypeID] = sensor;
      }

      //Read the component cache from DB
      for (const string& name : parameters.getComponentInsertOder()) {
        if (m_componentCache.find(name) != m_componentCache.end()) {
          // already created due to being a sub component of a previous
          // component. Seems fishy since the information of this component
          // is in the db at least twice so we could run into
          // inconsistencies.
          B2WARNING("Component " << name << " already created from previous subcomponents, should not be here");
          continue;
        }
        const VXDGeoComponentPar& paramsComponent = parameters.getComponent(name);
        VXDGeoComponent  c(
          paramsComponent.getMaterial(),
          paramsComponent.getColor(),
          paramsComponent.getWidth() / Unit::mm,
          paramsComponent.getWidth2() / Unit::mm,
          paramsComponent.getLength() / Unit::mm,
          paramsComponent.getHeight() / Unit::mm
        );
        double angle  = paramsComponent.getAngle();


        if (c.getWidth() <= 0 || c.getLength() <= 0 || c.getHeight() <= 0) {
          B2DEBUG(100, "One dimension empty, using auto resize for component");
        } else {
          G4VSolid* solid = createTrapezoidal(m_prefix + "." + name, c.getWidth(), c.getWidth2(), c.getLength(), c.getHeight(), angle);
          c.setVolume(new G4LogicalVolume(solid, Materials::get(c.getMaterial()), m_prefix + "." + name));
        }

        vector<VXDGeoPlacement> subComponents;
        for (const VXDGeoPlacementPar& paramsSubComponent : paramsComponent.getSubComponents()) {
          subComponents.push_back(VXDGeoPlacement(
                                    paramsSubComponent.getName(),
                                    paramsSubComponent.getU()  / Unit::mm,
                                    paramsSubComponent.getV()  / Unit::mm,
                                    paramsSubComponent.getW(),
                                    paramsSubComponent.getWOffset()  / Unit::mm
                                  ));

        }

        createSubComponents(m_prefix + "." + name, c, subComponents);
        if (m_activeChips &&  parameters.getSensitiveChipID(name) >= 0) {
          int chipID = parameters.getSensitiveChipID(name);
          B2DEBUG(50, "Creating BkgSensitiveDetector for component " << name << " with chipID " <<  chipID);
          BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(m_prefix.c_str(), chipID);
          c.getVolume()->SetSensitiveDetector(sensitive);
          m_sensitive.push_back(sensitive);
        }

        m_componentCache[name] = c;
      }

      //Build all ladders including Sensors
      VXD::GeoVXDAssembly shellSupport = createHalfShellSupport(parameters);

      //const std::vector<VXDHalfShellPar>& HalfShells = parameters.getHalfShells();
      for (const VXDHalfShellPar& shell : parameters.getHalfShells()) {
        string shellName =  shell.getName();
        m_currentHalfShell = m_prefix + "." + shellName;
        G4Transform3D shellAlignment = getAlignment(parameters.getAlignment(m_currentHalfShell));

        // Remember shell coordinate system (into which ladders are inserted)
        VXD::GeoCache::getInstance().addHalfShellPlacement(m_halfShellVxdIDs[m_currentHalfShell], shellAlignment);

        //Place shell support
        double shellAngle = shell.getShellAngle(); // Only used to move support, not active volumes!
        if (!m_onlyActiveMaterial) shellSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));

        //const std::map< int, std::vector<std::pair<int, double>> >& Layers = shell.getLayers();
        for (const std::pair<const int, std::vector<std::pair<int, double>> >& layer : shell.getLayers()) {
          int layerID = layer.first;
          const std::vector<std::pair<int, double>>& Ladders = layer.second;


          setCurrentLayer(layerID, parameters);

          //Place Layer support
          VXD::GeoVXDAssembly layerSupport = createLayerSupport();
          if (!m_onlyActiveMaterial) layerSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));
          VXD::GeoVXDAssembly ladderSupport = createLadderSupport();

          //Loop over defined ladders
          for (const std::pair<int, double>& ladder : Ladders) {
            int ladderID = ladder.first;
            double phi = ladder.second;

            G4Transform3D ladderPlacement = placeLadder(ladderID, phi, envelope, shellAlignment, parameters);
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
        if (parameters.getRadiationSensors().getSubDetector() == "") {
          B2DEBUG(10, "Apparently no radiation sensors defined, skipping");
        } else {
          createDiamonds(parameters.getRadiationSensors(), topVolume, *envelope);
        }
      }
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

    VXD::GeoVXDAssembly GeoPXDCreator::createLayerSupport() { return VXD::GeoVXDAssembly(); }

    VXD::GeoVXDAssembly GeoPXDCreator::createLadderSupport() { return VXD::GeoVXDAssembly(); }

    VXD::GeoVXDAssembly GeoPXDCreator::createHalfShellSupport(const PXDGeometryPar& parameters)
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


  }
}
