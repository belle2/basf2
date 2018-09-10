/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter, Hyacinth Stypula, Benjamin Schwenker      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geometry/GeoSVDCreator.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/simulation/SensitiveDetector.h>

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
#include <G4Torus.hh>
#include <G4Polycone.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>

using namespace std;
using namespace boost;

namespace Belle2 {
  using namespace geometry;
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {

    /** Register the creator */
    geometry::CreatorFactory<GeoSVDCreator> GeoSVDFactory("SVDCreator");

    GeoSVDCreator::~GeoSVDCreator()
    {
      for (SensorInfo* sensorInfo : m_SensorInfo) delete sensorInfo;
      m_SensorInfo.clear();
    }

    VXD::SensorInfoBase* GeoSVDCreator::createSensorInfo(const VXDGeoSensorPar& sensor)
    {
      const SVDSensorInfoPar& infoPar = dynamic_cast<const SVDSensorInfoPar&>(*sensor.getSensorInfo());

      SensorInfo* info = new SensorInfo(
        VxdID(0, 0, 0),
        infoPar.getWidth(),
        infoPar.getLength(),
        infoPar.getThickness(),
        infoPar.getUCells(),
        infoPar.getVCells(),
        infoPar.getWidth2()
      );
      info->setSensorParams(
        infoPar.getStripEdgeU(),
        infoPar.getStripEdgeV(),
        infoPar.getDepletionVoltage(),
        infoPar.getBiasVoltage(),
        infoPar.getBackplaneCapacitanceU(),
        infoPar.getInterstripCapacitanceU(),
        infoPar.getCouplingCapacitanceU(),
        infoPar.getBackplaneCapacitanceV(),
        infoPar.getInterstripCapacitanceV(),
        infoPar.getCouplingCapacitanceV(),
        infoPar.getAduEquivalentU(),
        infoPar.getAduEquivalentV(),
        infoPar.getElectronicNoiseU(),
        infoPar.getElectronicNoiseV(),
        infoPar.getAduEquivalentSbwU(),
        infoPar.getAduEquivalentSbwV(),
        infoPar.getElectronicNoiseSbwU(),
        infoPar.getElectronicNoiseSbwV()
      );
      m_SensorInfo.push_back(info);
      return info;
    }

    SVDSensorInfoPar* GeoSVDCreator::readSensorInfo(const GearDir& sensor)
    {

      const double unit_pFcm = 1;
      // This was 1000 * Unit::fC / Unit::V / Unit::cm; // pF/cm.
      // We only use ratios of capacities, and this gives nicer numbers.
      SVDSensorInfoPar* info = new SVDSensorInfoPar(
        VxdID(0, 0, 0),
        sensor.getLength("width"),
        sensor.getLength("length"),
        sensor.getLength("height"),
        sensor.getInt("stripsU"),
        sensor.getInt("stripsV"),
        sensor.getLength("width2", 0)
      );

      info->setSensorParams(
        sensor.getWithUnit("stripEdgeU"),
        sensor.getWithUnit("stripEdgeV"),
        sensor.getWithUnit("DepletionVoltage"),
        sensor.getWithUnit("BiasVoltage"),
        sensor.getDouble("BackplaneCapacitanceU") * unit_pFcm,
        sensor.getDouble("InterstripCapacitanceU") * unit_pFcm,
        sensor.getDouble("CouplingCapacitanceU") * unit_pFcm,
        sensor.getDouble("BackplaneCapacitanceV") * unit_pFcm,
        sensor.getDouble("InterstripCapacitanceV") * unit_pFcm,
        sensor.getDouble("CouplingCapacitanceV") * unit_pFcm,
        sensor.getWithUnit("ADUEquivalentU"),
        sensor.getWithUnit("ADUEquivalentV"),
        sensor.getWithUnit("ElectronicNoiseU"),
        sensor.getWithUnit("ElectronicNoiseV"),
        sensor.getWithUnit("ADUEquivalentSbwU", 0),
        sensor.getWithUnit("ADUEquivalentSbwV", 0),
        sensor.getWithUnit("ElectronicNoiseSbwU", 0),
        sensor.getWithUnit("ElectronicNoiseSbwV", 0)
      );

      return info;
    }

    VXD::SensitiveDetectorBase* GeoSVDCreator::createSensitiveDetector(
      VxdID sensorID, const VXDGeoSensor& sensor, const VXDGeoSensorPlacement&)
    {
      SensorInfo* sensorInfo = new SensorInfo(dynamic_cast<const SensorInfo&>(*sensor.getSensorInfo()));
      sensorInfo->setID(sensorID);
      SensitiveDetector* sensitive = new SensitiveDetector(sensorInfo);
      return sensitive;
    }

    SVDGeometryPar GeoSVDCreator::createConfiguration(const GearDir& content)
    {
      SVDGeometryPar svdGeometryPar;

      //Read prefix ('SVD' or 'PXD')
      svdGeometryPar.setPrefix(m_prefix);

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
      svdGeometryPar.setGlobalParams(globals);

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
      svdGeometryPar.setEnvelope(envelope);

      // Read alignment for detector m_prefix ('PXD' or 'SVD')
      string pathAlign = (boost::format("Align[@component='%1%']/") % m_prefix).str();
      GearDir paramsAlign(GearDir(content, "Alignment/"), pathAlign);
      if (!paramsAlign) {
        B2WARNING("Could not find alignment parameters for component " << m_prefix);
        return svdGeometryPar;
      }
      svdGeometryPar.getAlignmentMap()[m_prefix] = VXDAlignmentPar(paramsAlign.getLength("du"),
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

        SVDSensorInfoPar* svdInfo = readSensorInfo(GearDir(paramsSensor, "Active"));
        sensor.setSensorInfo(svdInfo);
        sensor.setComponents(getSubComponents(paramsSensor));
        svdGeometryPar.getSensorMap()[sensorTypeID] = sensor;
        svdGeometryPar.getSensorInfos().push_back(svdInfo);
      }

      //Build all ladders including Sensors
      GearDir support(content, "Support/");
      readHalfShellSupport(support, svdGeometryPar);

      for (const GearDir& shell : content.getNodes("HalfShell")) {

        string shellName = m_prefix + "." + shell.getString("@name");
        string pathShell = (boost::format("Align[@component='%1%']/") % shellName).str();
        GearDir paramsShell(GearDir(content, "Alignment/"), pathShell);
        if (!paramsShell) {
          B2WARNING("Could not find alignment parameters for component " << shellName);
          return svdGeometryPar;
        }
        svdGeometryPar.getAlignmentMap()[shellName] = VXDAlignmentPar(paramsShell.getLength("du"),
                                                      paramsShell.getLength("dv"),
                                                      paramsShell.getLength("dw"),
                                                      paramsShell.getAngle("alpha"),
                                                      paramsShell.getAngle("beta"),
                                                      paramsShell.getAngle("gamma")
                                                                     );

        VXDHalfShellPar halfShell(shell.getString("@name") , shell.getAngle("shellAngle", 0));

        for (const GearDir& layer : shell.getNodes("Layer")) {
          int layerID = layer.getInt("@id");

          readLadder(layerID, components, svdGeometryPar);
          readLayerSupport(layerID, support, svdGeometryPar);
          readLadderSupport(layerID, support, svdGeometryPar);

          //Loop over defined ladders
          for (const GearDir& ladder : layer.getNodes("Ladder")) {
            int ladderID = ladder.getInt("@id");
            double phi = ladder.getAngle("phi", 0);
            readLadderComponents(layerID, ladderID, content, svdGeometryPar);
            halfShell.addLadder(layerID, ladderID,  phi);
          }
        }
        svdGeometryPar.getHalfShells().push_back(halfShell);
      }

      //Create diamond radiation sensors if defined and in background mode
      GearDir radiationDir(content, "RadiationSensors");
      if (svdGeometryPar.getGlobalParams().getActiveChips() &&  radiationDir) {
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
        svdGeometryPar.setRadiationSensors(radiationSensors);
      }
      return svdGeometryPar;
    }

    void GeoSVDCreator::createGeometry(const SVDGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes)
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
        double shellAngle = shell.getShellAngle();
        if (!m_onlyActiveMaterial) shellSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));

        //const std::map< int, std::vector<std::pair<int, double>> >& Layers = shell.getLayers();
        for (const std::pair<const int, std::vector<std::pair<int, double>> >& layer : shell.getLayers()) {
          int layerID = layer.first;
          const std::vector<std::pair<int, double>>& Ladders = layer.second;


          setCurrentLayer(layerID, parameters);

          //Place Layer support
          VXD::GeoVXDAssembly layerSupport = createLayerSupport(layerID, parameters);
          if (!m_onlyActiveMaterial) layerSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));
          VXD::GeoVXDAssembly ladderSupport = createLadderSupport(layerID, parameters);

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

    void GeoSVDCreator::readHalfShellSupport(GearDir support, SVDGeometryPar& svdGeometryPar)
    {
      if (!support) return;

      for (const GearDir& params : support.getNodes("HalfShell/RotationSolid")) {

        VXDRotationSolidPar rotationSolidPar(params.getString("Name", ""),
                                             params.getString("Material", "Air"),
                                             params.getString("Color", ""),
                                             params.getAngle("minPhi", 0),
                                             params.getAngle("maxPhi", 2 * M_PI),
                                             (params.getNodes("InnerPoints/point").size() > 0)
                                            );

        for (const GearDir point : params.getNodes("InnerPoints/point")) {
          pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
          rotationSolidPar.getInnerPoints().push_back(ZXPoint);
        }
        for (const GearDir point : params.getNodes("OuterPoints/point")) {
          pair<double, double> ZXPoint(point.getLength("z"), point.getLength("x"));
          rotationSolidPar.getOuterPoints().push_back(ZXPoint);
        }
        svdGeometryPar.getRotationSolids().push_back(rotationSolidPar);
      }
      return;
    }

    void GeoSVDCreator::readLayerSupport(int layer, GearDir support, SVDGeometryPar& svdGeometryPar)
    {
      if (!support) return;

      //Check if there are any endrings defined for this layer. If not we don't create any
      GearDir endrings(support, (boost::format("Endrings/Layer[@id='%1%']") % layer).str());
      if (endrings) {
        svdGeometryPar.getEndrings()[layer] = SVDEndringsPar(support.getString("Endrings/Material"),
                                                             support.getLength("Endrings/length"),
                                                             support.getLength("Endrings/gapWidth"),
                                                             support.getLength("Endrings/baseThickness")
                                                            );

        //Create  the endrings
        for (const GearDir& endring : endrings.getNodes("Endring")) {
          SVDEndringsTypePar endringPar(endring.getString("@name"),
                                        endring.getLength("z"),
                                        endring.getLength("baseRadius"),
                                        endring.getLength("innerRadius"),
                                        endring.getLength("outerRadius"),
                                        endring.getLength("horizontalBar"),
                                        endring.getLength("verticalBar")
                                       );
          svdGeometryPar.getEndrings()[layer].getTypes().push_back(endringPar);
        }
      }

      // Now let's add the cooling pipes to the Support
      GearDir pipes(support, (boost::format("CoolingPipes/Layer[@id='%1%']") % layer).str());
      if (pipes) {
        svdGeometryPar.getCoolingPipes()[layer] = SVDCoolingPipesPar(support.getString("CoolingPipes/Material"),
                                                  support.getLength("CoolingPipes/outerDiameter"),
                                                  support.getLength("CoolingPipes/wallThickness"),
                                                  pipes.getInt("nPipes"),
                                                  pipes.getAngle("startPhi"),
                                                  pipes.getAngle("deltaPhi"),
                                                  pipes.getLength("radius"),
                                                  pipes.getLength("zstart"),
                                                  pipes.getLength("zend")
                                                                    );
        if (pipes.exists("deltaL")) svdGeometryPar.getCoolingPipes()[layer].setDeltaL(pipes.getLength("deltaL"));
      }
      return;
    }

    void GeoSVDCreator::readLadderSupport(int layer, GearDir support, SVDGeometryPar& svdGeometryPar)
    {
      if (!support) return;

      // Check if there are any support ribs defined for this layer. If not return empty assembly
      GearDir params(support, (boost::format("SupportRibs/Layer[@id='%1%']") % layer).str());
      if (params) {
        svdGeometryPar.getSupportRibs()[layer] = SVDSupportRibsPar(support.getLength("SupportRibs/spacing"),
                                                                   support.getLength("SupportRibs/height"),
                                                                   support.getLength("SupportRibs/inner/width"),
                                                                   support.getLength("SupportRibs/outer/width"),
                                                                   support.getLength("SupportRibs/inner/tabLength"),
                                                                   support.getString("SupportRibs/outer/Material"),
                                                                   support.getString("SupportRibs/inner/Material"),
                                                                   support.getString("SupportRibs/outer/Color"),
                                                                   support.getString("SupportRibs/inner/Color"),
                                                                   support.getString("SupportRibs/endmount/Material")
                                                                  );

        // Get values for the layer if available
        if (params.exists("spacing")) svdGeometryPar.getSupportRibs()[layer].setSpacing(params.getLength("spacing"));
        if (params.exists("height")) svdGeometryPar.getSupportRibs()[layer].setHeight(params.getLength("height"));

        for (const GearDir& box : params.getNodes("box")) {
          SVDSupportBoxPar boxPar(box.getAngle("theta"),
                                  box.getLength("z"),
                                  box.getLength("r"),
                                  box.getLength("length")
                                 );
          svdGeometryPar.getSupportRibs()[layer].getBoxes().push_back(boxPar);
        }

        for (const GearDir& tab : params.getNodes("tab")) {
          SVDSupportTabPar tabPar(tab.getAngle("theta"),
                                  tab.getLength("z"),
                                  tab.getLength("r")
                                 );
          svdGeometryPar.getSupportRibs()[layer].getTabs().push_back(tabPar);
        }

        for (const GearDir& endmount : params.getNodes("Endmount")) {
          SVDEndmountPar mountPar(endmount.getString("@name"),
                                  endmount.getLength("height"),
                                  endmount.getLength("width"),
                                  endmount.getLength("length"),
                                  endmount.getLength("z"),
                                  endmount.getLength("r")
                                 );
          svdGeometryPar.getSupportRibs()[layer].getEndmounts().push_back(mountPar);
        }
      }
      return;
    }

    VXD::GeoVXDAssembly GeoSVDCreator::createHalfShellSupport(const SVDGeometryPar& parameters)
    {
      VXD::GeoVXDAssembly supportAssembly;

      //Half shell support is easy as we just add all the defined RotationSolids
      double minZ(0), maxZ(0);

      const std::vector<VXDRotationSolidPar>& RotationSolids = parameters.getRotationSolids();

      for (const VXDRotationSolidPar& component : RotationSolids) {

        string name = component.getName();
        string material = component.getMaterial();

        G4Polycone* solid = geometry::createRotationSolid(name,
                                                          component.getInnerPoints(),
                                                          component.getOuterPoints(),
                                                          component.getMinPhi(),
                                                          component.getMaxPhi(),
                                                          minZ, maxZ
                                                         );

        G4LogicalVolume* volume = new G4LogicalVolume(
          solid, geometry::Materials::get(material), m_prefix + ". " + name);
        geometry::setColor(*volume, component.getColor());
        supportAssembly.add(volume);
      }
      return supportAssembly;
    }



    VXD::GeoVXDAssembly GeoSVDCreator::createLayerSupport(int layer, const SVDGeometryPar& parameters)
    {
      VXD::GeoVXDAssembly supportAssembly;

      //Check if there are any endrings defined for this layer. If not we don't create any
      if (parameters.getEndringsExist(layer)) {
        const SVDEndringsPar& support = parameters.getEndrings(layer);

        string material      = support.getMaterial();
        double length        = support.getLength() / Unit::mm / 2.0;
        double gapWidth      = support.getGapWidth() / Unit::mm;
        double baseThickness = support.getBaseThickness() / Unit::mm / 2.0;

        //Create  the endrings
        const std::vector<SVDEndringsTypePar>& Endrings = support.getTypes();
        for (const SVDEndringsTypePar& endring : Endrings) {
          double z             = endring.getZ() / Unit::mm;
          double baseRadius    = endring.getBaseRadius() / Unit::mm;
          double innerRadius   = endring.getInnerRadius() / Unit::mm;
          double outerRadius   = endring.getOuterRadius() / Unit::mm;
          double horiBarWidth  = endring.getHorizontalBarWidth() / Unit::mm / 2.0;
          double vertBarWidth  = endring.getVerticalBarWidth() / Unit::mm / 2.0;

          double angle = asin(gapWidth / innerRadius);
          G4VSolid* endringSolid = new G4Tubs("OuterEndring", innerRadius, outerRadius, length, -M_PI / 2 + angle, M_PI - 2 * angle);
          angle = asin(gapWidth / baseRadius);
          G4VSolid* endringBase  = new G4Tubs("InnerEndring", baseRadius, baseRadius + baseThickness, length, -M_PI / 2 + angle,
                                              M_PI - 2 * angle);
          endringSolid = new G4UnionSolid("Endring", endringSolid, endringBase);

          //Now we need the bars which connect the two rings
          double height = (innerRadius - baseRadius) / 2.0;
          double x = vertBarWidth + gapWidth;
          G4Box* verticalBar = new G4Box("VerticalBar", vertBarWidth, height, length);
          G4Box* horizontalBar = new G4Box("HorizontalBar", height, horiBarWidth, length);
          endringSolid = new G4UnionSolid("Endring", endringSolid, verticalBar, G4Translate3D(x,  baseRadius + height, 0));
          endringSolid = new G4UnionSolid("Endring", endringSolid, verticalBar, G4Translate3D(x, -(baseRadius + height), 0));
          endringSolid = new G4UnionSolid("Endring", endringSolid, horizontalBar, G4Translate3D((baseRadius + height), 0, 0));

          //Finally create the volume and add it to the assembly at the correct z position
          G4LogicalVolume* endringVolume = new G4LogicalVolume(
            endringSolid, geometry::Materials::get(material),
            (boost::format("%1%.Layer%2%.%3%") % m_prefix % layer % endring.getName()).str());
          supportAssembly.add(endringVolume, G4TranslateZ3D(z));
        }
      }

      //Check if there are any coling pipes defined for this layer. If not we don't create any
      if (parameters.getCoolingPipesExist(layer)) {
        const SVDCoolingPipesPar& pipes = parameters.getCoolingPipes(layer);

        string material    = pipes.getMaterial();
        double outerRadius = pipes.getOuterDiameter() / Unit::mm / 2.0;
        double innerRadius = outerRadius - pipes.getWallThickness() / Unit::mm;
        int    nPipes      = pipes.getNPipes();
        double startPhi    = pipes.getStartPhi();
        double deltaPhi    = pipes.getDeltaPhi();
        double radius      = pipes.getRadius() / Unit::mm;
        double zstart      = pipes.getZStart() / Unit::mm;
        double zend        = pipes.getZEnd() / Unit::mm;
        double zlength     = (zend - zstart) / 2.0;


        // There are two parts: the straight pipes and the bendings. So we only need two different volumes
        // which we place multiple times
        G4Tubs* pipeSolid = new G4Tubs("CoolingPipe", innerRadius, outerRadius, zlength, 0, 2 * M_PI);
        G4LogicalVolume* pipeVolume = new G4LogicalVolume(
          pipeSolid, geometry::Materials::get(material),
          (boost::format("%1%.Layer%2%.CoolingPipe") % m_prefix % layer).str());
        geometry::setColor(*pipeVolume, "#ccc");

        G4Torus* bendSolid = new G4Torus("CoolingBend", innerRadius, outerRadius, sin(deltaPhi / 2.0)*radius, -M_PI / 2, M_PI);
        G4LogicalVolume* bendVolume = new G4LogicalVolume(
          bendSolid, geometry::Materials::get(material),
          (boost::format("%1%.Layer%2%.CoolingBend") % m_prefix % layer).str());

        // Last pipe may be closer, thus we need additional bending
        if (pipes.getDeltaL() > 0) {
          double deltaL = pipes.getDeltaL() / Unit::mm;
          G4Torus* bendSolidLast = new G4Torus("CoolingBendLast", innerRadius, outerRadius, sin(deltaPhi / 2.0) * radius - deltaL / 2.0,
                                               -M_PI / 2, M_PI);
          G4LogicalVolume* bendVolumeLast = new G4LogicalVolume(bendSolidLast, geometry::Materials::get(material),
                                                                (boost::format("%1%.Layer%2%.CoolingBendLast") % m_prefix % layer).str());
          --nPipes;

          // Place the last straight pipe
          G4Transform3D placement_pipe = G4RotateZ3D(startPhi + (nPipes - 0.5) * deltaPhi) * G4Translate3D(cos(deltaPhi / 2.0) * radius,
                                         sin(deltaPhi / 2.0) * radius - deltaL, zstart + zlength);
          supportAssembly.add(pipeVolume, placement_pipe);

          // Place forward or backward bend
          double zpos = nPipes % 2 > 0 ? zend : zstart;
          // Calculate transformation
          G4Transform3D placement = G4RotateZ3D(startPhi + (nPipes - 0.5) * deltaPhi) * G4Translate3D(cos(deltaPhi / 2.0) * radius,
                                    -deltaL / 2.0, zpos) * G4RotateY3D(M_PI / 2);
          // If we are at the forward side we rotate the bend by 180 degree
          if (nPipes % 2 > 0) {
            placement = placement * G4RotateZ3D(M_PI);
          }
          // And place the bend
          supportAssembly.add(bendVolumeLast, placement);
        }

        for (int i = 0; i < nPipes; ++i) {
          // Place the straight pipes
          G4Transform3D placement_pipe = G4RotateZ3D(startPhi + i * deltaPhi) * G4Translate3D(radius, 0, zstart + zlength);
          supportAssembly.add(pipeVolume, placement_pipe);

          // This was the easy part, now lets add the connection between the pipes. We only need n-1 bendings
          if (i > 0) {
            // Place forward or backward bend
            double zpos = i % 2 > 0 ? zend : zstart;
            // Calculate transformation
            G4Transform3D placement = G4RotateZ3D(startPhi + (i - 0.5) * deltaPhi) * G4Translate3D(cos(deltaPhi / 2.0) * radius, 0,
                                      zpos) * G4RotateY3D(M_PI / 2);
            // If we are at the forward side we rotate the bend by 180 degree
            if (i % 2 > 0) {
              placement = placement * G4RotateZ3D(M_PI);
            }
            // And place the bend
            supportAssembly.add(bendVolume, placement);
          }
        }
      }

      return supportAssembly;
    }



    VXD::GeoVXDAssembly GeoSVDCreator::createLadderSupport(int layer, const SVDGeometryPar& parameters)
    {
      VXD::GeoVXDAssembly supportAssembly;

      if (!parameters.getSupportRibsExist(layer)) return supportAssembly;
      const SVDSupportRibsPar& support = parameters.getSupportRibs(layer);

      // Get the common values for all layers
      double spacing    = support.getSpacing() / Unit::mm / 2.0;
      double height     = support.getHeight() / Unit::mm / 2.0;
      double innerWidth = support.getInnerWidth() / Unit::mm / 2.0;
      double outerWidth = support.getOuterWidth() / Unit::mm / 2.0;
      double tabLength  = support.getTabLength() / Unit::mm / 2.0;
      G4VSolid* inner(0);
      G4VSolid* outer(0);
      G4Transform3D placement;


      // Now lets create the ribs by adding all boxes to form one union solid
      const std::vector<SVDSupportBoxPar>& Boxes = support.getBoxes();
      for (const SVDSupportBoxPar& box : Boxes) {
        double theta = box.getTheta();
        double zpos = box.getZ() / Unit::mm;
        double rpos = box.getR() / Unit::mm;
        double length = box.getLength() / Unit::mm / 2.0;
        G4Box* innerBox = new G4Box("innerBox", height, innerWidth, length);
        G4Box* outerBox = new G4Box("outerBox", height, outerWidth, length);
        if (!inner) {
          inner = innerBox;
          outer = outerBox;
          placement = G4Translate3D(rpos, 0, zpos) * G4RotateY3D(theta);
        } else {
          G4Transform3D relative = placement.inverse() * G4Translate3D(rpos, 0, zpos) * G4RotateY3D(theta);
          inner = new G4UnionSolid("innerBox", inner, innerBox, relative);
          outer = new G4UnionSolid("outerBox", outer, outerBox, relative);
        }
      }

      // Now lets add the tabs
      const std::vector<SVDSupportTabPar>& Tabs = support.getTabs();
      for (const SVDSupportTabPar& tab : Tabs) {
        double theta = tab.getTheta();
        double zpos = tab.getZ() / Unit::mm;
        double rpos = tab.getR() / Unit::mm;
        G4Box* innerBox = new G4Box("innerBox", height, innerWidth, tabLength);
        if (!inner) {
          inner = innerBox;
          placement = G4Translate3D(rpos, 0, zpos) * G4RotateY3D(theta);
        } else {
          G4Transform3D relative = placement.inverse() * G4Translate3D(rpos, 0, zpos) * G4RotateY3D(theta);
          inner = new G4UnionSolid("innerBox", inner, innerBox, relative);
        }
      }

      // Now lets create forward and backward endmounts for the ribs
      const std::vector<SVDEndmountPar>& Endmounts = support.getEndmounts();
      for (const SVDEndmountPar& endmount : Endmounts) {
        double endMountHeight = endmount.getHeight() / Unit::mm / 2.0;
        double endMountWidth = endmount.getWidth() / Unit::mm / 2.0;
        double endMountLength = endmount.getLength() / Unit::mm / 2.0;
        double zpos = endmount.getZ() / Unit::mm;
        double rpos = endmount.getR() / Unit::mm;
        G4VSolid* endmountBox = new G4Box("endmountBox", endMountHeight, endMountWidth, endMountLength);
        if (outer) { // holes for the ribs
          endmountBox = new G4SubtractionSolid("endmountBox", endmountBox, outer, G4TranslateY3D(-spacing)*placement * G4Translate3D(-rpos, 0,
                                               -zpos));
          endmountBox = new G4SubtractionSolid("endmountBox", endmountBox, outer, G4TranslateY3D(spacing)*placement * G4Translate3D(-rpos, 0,
                                               -zpos));
        }
        G4LogicalVolume* endmountVolume = new G4LogicalVolume(
          endmountBox, geometry::Materials::get(support.getEndmountMaterial()),
          (boost::format("%1%.Layer%2%.%3%Endmount") % m_prefix % layer % endmount.getName()).str());
        supportAssembly.add(endmountVolume, G4Translate3D(rpos, 0, zpos));
      }

      // If there has been at least one Box, create the volumes and add them to the assembly
      if (inner) {
        outer = new G4SubtractionSolid("outerBox", outer, inner);
        G4LogicalVolume* outerVolume = new G4LogicalVolume(
          outer, geometry::Materials::get(support.getOuterMaterial()),
          (boost::format("%1%.Layer%2%.SupportRib") % m_prefix % layer).str());
        G4LogicalVolume* innerVolume = new G4LogicalVolume(
          inner, geometry::Materials::get(support.getInnerMaterial()),
          (boost::format("%1%.Layer%2%.SupportRib.Airex") % m_prefix % layer).str());
        geometry::setColor(*outerVolume, support.getOuterColor());
        geometry::setColor(*innerVolume, support.getInnerColor());
        supportAssembly.add(innerVolume, G4TranslateY3D(-spacing)*placement);
        supportAssembly.add(innerVolume, G4TranslateY3D(spacing)*placement);
        supportAssembly.add(outerVolume, G4TranslateY3D(-spacing)*placement);
        supportAssembly.add(outerVolume, G4TranslateY3D(spacing)*placement);
      }

      // Done, return the finished assembly
      return supportAssembly;
    }



  }
}
