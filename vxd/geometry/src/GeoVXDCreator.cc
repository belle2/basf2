/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Jozef Koval, Benjamin Schwenker           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoVXDCreator.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <G4ReflectionFactory.hh>
#include <G4LogicalVolume.hh>
#include <G4Trd.hh>
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4UserLimits.hh>
#include <G4Point3D.hh>

#include <G4TessellatedSolid.hh>
#include <G4QuadrangularFacet.hh>
#include <G4TriangularFacet.hh>

#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <limits>

using namespace std;

namespace Belle2 {

  using namespace geometry;
  namespace VXD {

    GeoVXDCreator::GeoVXDCreator(const string& prefix) : m_prefix(prefix), m_radiationsensors(prefix)
    {
      m_UserLimits.clear();
    }

    GeoVXDCreator::~GeoVXDCreator()
    {
      //Lets assume that it cannot be that only one part of the vxd gets destroyed
      // FIXME: This causes problems: VXD::GeoCache::getInstance().clear();
      //Delete all sensitive detectors
      for (Simulation::SensitiveDetectorBase* sensitive : m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();

      for (G4UserLimits* userLimit : m_UserLimits) delete userLimit;
      m_UserLimits.clear();

    }

    GeoVXDAssembly GeoVXDCreator::createSubComponents(const string& name, VXDGeoComponent& component,
                                                      vector<VXDGeoPlacement> placements, bool originCenter, bool allowOutside)
    {
      GeoVXDAssembly assembly;
      B2DEBUG(100, "Creating component " << name);
      vector<VXDGeoComponent> subComponents;
      subComponents.reserve(placements.size());
      //Go over all subcomponents and check if they will fit inside.
      //If component.volume is zero we will create one so sum up needed space
      bool widthResize  = component.getWidth() <= 0;
      bool lengthResize = component.getLength() <= 0;
      bool heightResize = component.getHeight() <= 0;

      for (VXDGeoPlacement& p : placements) {
        //Test component already exists
        if (m_componentCache.find(p.getName()) == m_componentCache.end()) {
          B2FATAL("A component is requested that was not created before!");
        }
        VXDGeoComponent sub = m_componentCache[p.getName()];

        B2DEBUG(100, "SubComponent " << p.getName());
        B2DEBUG(100, boost::format("Placement: u:%1% cm, v:%2% cm, w:%3% + %4% cm") % p.getU() % p.getV() % p.getW() % p.getWOffset());
        B2DEBUG(100, boost::format("Dimensions: %1%x%2%x%3% cm") % sub.getWidth() % sub.getLength() % sub.getHeight());

        if (p.getW() == VXDGeoPlacement::c_above || p.getW() == VXDGeoPlacement::c_below) {
          //Below placement only valid if we are allowed to create a container around component
          if (!allowOutside) B2FATAL("Cannot place component " << p.getName() << " outside of component " << name);
        } else if (sub.getHeight() + p.getWOffset() > component.getHeight()) {
          //Component will not fit heightwise. If we resize the volume anyway than we don't have problems
          if (!heightResize) {
            B2FATAL("Subcomponent " << p.getName() << " does not fit into volume: "
                    << "height " << sub.getHeight() << " > " << component.getHeight());
          }
          component.getHeight() = sub.getHeight() + p.getWOffset();
        }

        //Check if compoent will fit inside width,length. If we can resize do it if needed, otherwise bail
        double minWidth =  max(abs(p.getU() + sub.getWidth() / 2.0), abs(p.getU() - sub.getWidth() / 2.0));
        double minLength = max(abs(p.getV() + sub.getLength() / 2.0), abs(p.getV() - sub.getLength() / 2.0));
        if (minWidth > component.getWidth() + component.getWidth() * numeric_limits<double>::epsilon()) {
          if (!widthResize) {
            B2FATAL("Subcomponent " << p.getName() << " does not fit into volume: "
                    << "minWidth " << minWidth << " > " << component.getWidth());
          }
          component.setWidth(minWidth * 2.0);
        }
        if (minLength > component.getLength() + component.getLength() * numeric_limits<double>::epsilon()) {
          if (!lengthResize) {
            B2FATAL("Subcomponent " << p.getName() << " does not fit into volume: "
                    << "minLength " << minLength << " > " << component.getLength());
          }
          component.setLength(minLength * 2.0);
        }
        subComponents.push_back(sub);
      }

      //zero dimensions are fine mathematically but we don't want them in the simulation
      if (component.getWidth() <= 0 || component.getLength() <= 0 || component.getHeight() <= 0) {
        B2FATAL("At least one dimension of component " << name << " is zero which does not make sense");
      }

      //No volume yet, create a new one automatically assuming air material
      if (!component.getVolume()) {
        G4VSolid* componentShape = createTrapezoidal(name, component.getWidth(), component.getWidth2(), component.getLength(),
                                                     component.getHeight());
        component.setVolume(new G4LogicalVolume(componentShape, Materials::get(component.getMaterial()), name));
      }

      B2DEBUG(100, boost::format("Component %1% dimensions: %2%x%3%x%4% cm") % name % component.getWidth() % component.getLength() %
              component.getHeight());

      //Ok, all volumes set up, now add them together
      for (size_t i = 0; i < placements.size(); ++i) {
        VXDGeoPlacement& p = placements[i];
        VXDGeoComponent& s = subComponents[i];

        G4Transform3D transform = getPosition(component, s, p, originCenter);
        if (p.getW() ==  VXDGeoPlacement::c_below || p.getW() == VXDGeoPlacement::c_above) {
          //Add to selected mother (either component or container around component
          assembly.add(s.getVolume(), transform);
        } else {
          new G4PVPlacement(transform, s.getVolume(), name + "." + p.getName(), component.getVolume(), false, i);
        }
      }

      //Set some visibility options for volume. Done here because all components including sensor go through here
      if (component.getColor().empty()) {
        B2DEBUG(200, "Component " << name << " is an Air volume, setting invisible");
        setVisibility(*component.getVolume(), false);
      } else {
        B2DEBUG(200, "Component " << name << " color: " << component.getColor());
        setColor(*component.getVolume(), component.getColor());
      }
      B2DEBUG(100, "--> Created component " << name);
      //Return the difference in W between the origin of the original component and the including container
      return assembly;
    }

    G4Transform3D GeoVXDCreator::getAlignment(VXDAlignmentPar params)
    {
      G4RotationMatrix rotation(params.getAlpha(), params.getBeta(), params.getBeta());
      G4ThreeVector translation(params.getDU() / Unit::mm, params.getDV() / Unit::mm, params.getDW() / Unit::mm);
      return G4Transform3D(rotation, translation);
    }

    G4Transform3D GeoVXDCreator::getPosition(const VXDGeoComponent& mother, const VXDGeoComponent& daughter,
                                             const VXDGeoPlacement& placement, bool originCenter)
    {
      double u(placement.getU()), v(placement.getV()), w(0);
      switch (placement.getW()) {
        case VXDGeoPlacement::c_below:  //Place below component
          w = - mother.getHeight() / 2.0 - daughter.getHeight() / 2.0;
          break;
        case VXDGeoPlacement::c_bottom: //Place inside, at bottom of component
          w = - mother.getHeight() / 2.0 + daughter.getHeight() / 2.0;
          break;
        case VXDGeoPlacement::c_center: //Place inside, centered
          w = 0;
          break;
        case VXDGeoPlacement::c_top:    //Place inside, at top of mother
          w = mother.getHeight() / 2.0 - daughter.getHeight() / 2.0;
          break;
        case VXDGeoPlacement::c_above:  //Place above mother
          w = mother.getHeight() / 2.0 + daughter.getHeight() / 2.0;
          break;
      }
      if (!originCenter) { //Sensor has coordinate origin in the corner, all submothers at their center
        u -= mother.getWidth() / 2.0;
        v -= mother.getLength() / 2.0;
      }
      return G4Translate3D(u, v, w + placement.getWOffset());
    }



    void GeoVXDCreator::createDiamonds(const VXDGeoRadiationSensorsPar& params, G4LogicalVolume& topVolume,
                                       G4LogicalVolume& envelopeVolume)
    {
      //Set the correct top volume to either global top or detector envelope
      G4LogicalVolume* top = &topVolume;
      if (params.getInsideEnvelope()) {
        top = &envelopeVolume;
      }

      //shape and material are the same for all sensors so create them now
      const double width = params.getWidth();
      const double length = params.getLength();
      const double height = params.getHeight();
      G4Box* shape = new G4Box("radiationSensorDiamond", width / 2 * CLHEP::cm, length / 2 * CLHEP::cm, height / 2 * CLHEP::cm);
      G4Material* material = geometry::Materials::get(params.getMaterial());

      //Now loop over all positions
      const std::vector<VXDGeoRadiationSensorsPositionPar>& Positions = params.getPositions();
      for (const VXDGeoRadiationSensorsPositionPar& position : Positions) {
        //get the radial and z position
        const double r = position.getRadius();
        const double z = position.getZ();
        const double theta = position.getTheta();
        //and loop over all phi positions
        const std::map<int, double>& Sensors = position.getSensors();
        for (const std::pair<const int, double>& sensor : Sensors) {
          //for (GearDir& sensor : position.getNodes("phi")) {
          //we need angle and Id
          const double phi = sensor.second;
          const int id = sensor.first;
          //then we create a nice name
          const std::string name = params.getSubDetector() + ".DiamondSensor." + std::to_string(id);
          //and create the sensor volume
          G4LogicalVolume* volume = new G4LogicalVolume(shape, material, name);
          //add a sensitive detector implementation
          BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(params.getSubDetector().c_str(), id);
          volume->SetSensitiveDetector(sensitive);
          //and place it at the correct position
          G4Transform3D transform = G4RotateZ3D(phi - M_PI / 2) * G4Translate3D(0, r * CLHEP::cm,
                                    z * CLHEP::cm) * G4RotateX3D(-M_PI / 2 - theta);
          new G4PVPlacement(transform, volume, name, top, false, 1);
        }
      }
    }

    G4VSolid* GeoVXDCreator::createTrapezoidal(const string& name, double width, double width2, double length, double& height,
                                               double angle)
    {
      double offset(0);
      if (angle > 0) {
        const double tana = tan(angle);
        height = min(tana * length, min(tana * width, height));
        offset = height / tana;
      }
      const double hwidth  = width / 2.0;
      const double hwidth2 = width2 / 2.0;
      const double hlength = length / 2.0;
      const double hheight = height / 2.0;

      if (width2 <= 0 || width == width2) {
        if (angle <= 0) {
          return new G4Box(name, hwidth, hlength, hheight);
        } else {
          return new G4Trd(name, hwidth, hwidth - offset, hlength, hlength - offset, hheight);
        }
      }
      //FIXME: offset not working, g4 complains about nonplanarity of face -X. But we do not need that shape at the moment
      //so lets ignore it for now
      return  new G4Trap(name, hheight, 0, 0, hlength, hwidth, hwidth2, 0, hlength - offset, hwidth - offset, hwidth2 - offset, 0);
    }

    G4Transform3D GeoVXDCreator::placeLadder(int ladderID, double phi, G4LogicalVolume* volume,
                                             const G4Transform3D& placement,
                                             const VXDGeometryPar& parameters)
    {
      VxdID ladder(m_ladder.getLayerID(), ladderID, 0);

      G4Translate3D ladderPos(m_ladder.getRadius(), m_ladder.getShift(), 0);
      G4Transform3D ladderPlacement = placement * G4RotateZ3D(phi) * ladderPos * getAlignment(parameters.getAlignment(ladder));
      // The actuall coordinate system of ladder (w still points to Z, there is only phi rotation + move to correct radius + shift)
      VXD::GeoCache::getInstance().addLadderPlacement(m_halfShellVxdIDs[m_currentHalfShell], ladder, ladderPlacement);


      vector<G4Point3D> lastSensorEdge;
      for (const VXDGeoSensorPlacement& p : m_ladder.getSensors()) {
        VxdID sensorID(ladder);
        sensorID.setSensorNumber(p.getSensorID());


        std::map<string, VXDGeoSensor>::iterator it = m_sensorMap.find(p.getSensorTypeID());
        if (it == m_sensorMap.end()) {
          B2FATAL("Invalid SensorTypeID " << p.getSensorTypeID() << ", please check the definition of " << sensorID);
        }
        VXDGeoSensor& s = it->second;
        string name = m_prefix + "." + (string)sensorID;

        //Calculate the reflection transformation needed. Since we want the
        //active area to be non reflected we apply this transformation on the
        //sensor and on the active area
        G4Transform3D reflection;
        if (p.getFlipU()) reflection = reflection * G4ReflectX3D();
        if (p.getFlipV()) reflection = reflection * G4ReflectY3D();
        if (p.getFlipW()) reflection = reflection * G4ReflectZ3D();

        G4VSolid* sensorShape = createTrapezoidal(name, s.getWidth() , s.getWidth2() , s.getLength() ,
                                                  s.getHeight());
        G4Material* sensorMaterial = Materials::get(s.getMaterial());
        if (m_onlyActiveMaterial) {
          s.setVolume(new G4LogicalVolume(sensorShape, Materials::get(m_defaultMaterial), name));
        } else {
          s.setVolume(new G4LogicalVolume(sensorShape, sensorMaterial, name));
        }

        // Create sensitive Area: this Part is created separately since we want full control over the coordinate system:
        // local x (called u) should point in RPhi direction
        // local y (called v) should point in global z
        // local z (called w) should away from the origin
        G4VSolid* activeShape = createTrapezoidal(name + ".Active", s.getActiveArea().getWidth(), s.getActiveArea().getWidth2(),
                                                  s.getActiveArea().getLength(), s.getActiveArea().getHeight());

        //Create appropriate sensitive detector instance
        SensitiveDetectorBase* sensitive = createSensitiveDetector(sensorID, s, p);

        sensitive->setOptions(m_seeNeutrons, m_onlyPrimaryTrueHits,
                              m_distanceTolerance, m_electronTolerance, m_minimumElectrons);
        m_sensitive.push_back(sensitive);
        G4LogicalVolume* active = new G4LogicalVolume(activeShape,  sensorMaterial, name + ".Active",
                                                      0, sensitive);
        m_UserLimits.push_back(new G4UserLimits(m_activeStepSize));
        active->SetUserLimits(m_UserLimits.back());

        setColor(*active, s.getActiveArea().getColor());

        //The coordinates of the active region are given as the distance between the corners, not to the center
        //Place the active area
        G4Transform3D activePosition = G4Translate3D(s.getActiveArea().getWidth() / 2.0, s.getActiveArea().getLength() / 2.0, 0) *
                                       getPosition(s, s.getActiveArea(), s.getActivePlacement(), false);

        G4ReflectionFactory::Instance()->Place(activePosition * reflection, name + ".Active", active, s.getVolume(),
                                               false, (int)sensorID, false);

        //Now create all the other components and place the Sensor
        GeoVXDAssembly assembly;
        if (!m_onlyActiveMaterial) assembly = createSubComponents(name, s, s.getComponents() , false, true);

        G4RotationMatrix rotation(0, -M_PI / 2.0, -M_PI / 2.0);
        G4Transform3D sensorAlign = getAlignment(parameters.getAlignment(sensorID));
        G4Transform3D sensorPlacement = G4Rotate3D(rotation) * sensorAlign * reflection;

        if (s.getSlanted()) {
          sensorPlacement = G4TranslateX3D(m_ladder.getSlantedRadius() - m_ladder.getRadius()) * G4RotateY3D(
                              -m_ladder.getSlantedAngle()) * sensorPlacement;
        }
        sensorPlacement = G4Translate3D(0.0, 0.0, p.getZ()) * sensorPlacement;
        // Remember the placement of sensor into ladder
        VXD::GeoCache::getInstance().addSensorPlacement(ladder, sensorID, sensorPlacement * activePosition * reflection);
        sensorPlacement = ladderPlacement * sensorPlacement;

        assembly.add(s.getVolume());
        assembly.place(volume, sensorPlacement);

        //See if we want to glue the modules together
        if (!m_ladder.getGlueMaterial().empty() && !m_onlyActiveMaterial) {
          double u = s.getWidth() / 2.0 + m_ladder.getGlueSize();
          double v = s.getLength() / 2.0;
          double w = s.getHeight() / 2.0 + m_ladder.getGlueSize();
          std::vector<G4Point3D> curSensorEdge(4);
          //Lets get the forward corners of the sensor by applying the unreflected placement matrix
          curSensorEdge[0] = sensorPlacement * reflection * G4Point3D(u, v, + w);
          curSensorEdge[1] = sensorPlacement * reflection * G4Point3D(u, v, - w);
          curSensorEdge[2] = sensorPlacement * reflection * G4Point3D(-u, v, - w);
          curSensorEdge[3] = sensorPlacement * reflection * G4Point3D(-u, v, + w);
          //If we already have backward edges this is not the first module so we can apply the glue
          if (lastSensorEdge.size()) {
            //Check that the modules don't overlap in z
            bool glueOK = true;
            for (int i = 0; i < 4; ++i) glueOK &= curSensorEdge[i].z() <= lastSensorEdge[i].z();
            if (!glueOK) {
              B2WARNING("Cannot place Glue at sensor " + (string)sensorID +
                        " since it overlaps with the last module in z");
            } else {
              //Create Glue which spans from last sensor to this sensor
              G4TessellatedSolid* solidTarget = new G4TessellatedSolid(m_prefix + ".Glue." + (string)sensorID);

              //Face at end of last Sensor
              solidTarget->AddFacet(new G4QuadrangularFacet(
                                      curSensorEdge[3], curSensorEdge[2], curSensorEdge[1], curSensorEdge[0], ABSOLUTE));
              //Face at begin of current Sensor
              solidTarget->AddFacet(new G4QuadrangularFacet(
                                      lastSensorEdge[0], lastSensorEdge[1], lastSensorEdge[2], lastSensorEdge[3], ABSOLUTE));

              //Top faces
              solidTarget->AddFacet(new G4TriangularFacet(
                                      curSensorEdge[3], curSensorEdge[0], lastSensorEdge[0], ABSOLUTE));
              solidTarget->AddFacet(new G4TriangularFacet(
                                      lastSensorEdge[0], lastSensorEdge[3], curSensorEdge[3], ABSOLUTE));
              //Bottom faces
              solidTarget->AddFacet(new G4TriangularFacet(
                                      curSensorEdge[1], curSensorEdge[2], lastSensorEdge[2], ABSOLUTE));
              solidTarget->AddFacet(new G4TriangularFacet(
                                      lastSensorEdge[2], lastSensorEdge[1], curSensorEdge[1], ABSOLUTE));
              //Right faces
              solidTarget->AddFacet(new G4TriangularFacet(
                                      curSensorEdge[0], curSensorEdge[1], lastSensorEdge[1], ABSOLUTE));
              solidTarget->AddFacet(new G4TriangularFacet(
                                      lastSensorEdge[1], lastSensorEdge[0], curSensorEdge[0], ABSOLUTE));
              //Left faces
              solidTarget->AddFacet(new G4TriangularFacet(
                                      curSensorEdge[2], curSensorEdge[3], lastSensorEdge[3], ABSOLUTE));
              solidTarget->AddFacet(new G4TriangularFacet(
                                      lastSensorEdge[3], lastSensorEdge[2], curSensorEdge[2], ABSOLUTE));

              solidTarget->SetSolidClosed(true);

              G4LogicalVolume* glue = new G4LogicalVolume(solidTarget,  Materials::get(m_ladder.getGlueMaterial()),
                                                          m_prefix + ".Glue." + (string)sensorID);
              setColor(*glue, "#097");
              new G4PVPlacement(G4Transform3D(), glue, m_prefix + ".Glue." + (string)sensorID, volume, false, 1);
            }
          }
          //Remember the backward edge of this sensor to be glued to.
          lastSensorEdge.resize(4);
          lastSensorEdge[0] = sensorPlacement * reflection * G4Point3D(u, -v, + w);
          lastSensorEdge[1] = sensorPlacement * reflection * G4Point3D(u, -v, - w);
          lastSensorEdge[2] = sensorPlacement * reflection * G4Point3D(-u, -v, - w);
          lastSensorEdge[3] = sensorPlacement * reflection * G4Point3D(-u, -v, + w);
        }
      }

      return ladderPlacement;
    }

    void GeoVXDCreator::setCurrentLayer(int layer, const VXDGeometryPar& parameters)
    {
      const VXDGeoLadderPar& paramsLadder = parameters.getLadder(layer);

      m_ladder = VXDGeoLadder(
                   layer,
                   paramsLadder.getShift() / Unit::mm,
                   paramsLadder.getRadius() / Unit::mm,
                   paramsLadder.getSlantedAngle(),
                   paramsLadder.getSlantedRadius() / Unit::mm,
                   paramsLadder.getGlueSize() / Unit::mm,
                   paramsLadder.getGlueMaterial()
                 );


      for (const VXDGeoSensorPlacementPar& sensorInfo : paramsLadder.getSensors()) {
        m_ladder.addSensor(VXDGeoSensorPlacement(
                             sensorInfo.getSensorID(),
                             sensorInfo.getSensorTypeID(),
                             sensorInfo.getZ() / Unit::mm,
                             sensorInfo.getFlipU(),
                             sensorInfo.getFlipV(),
                             sensorInfo.getFlipW()
                           ));
      }
    }



    void GeoVXDCreator::readLadderComponents(int layerID, int ladderID,  GearDir content, VXDGeometryPar& vxdGeometryPar)
    {
      VxdID ladder(layerID, ladderID, 0);

      // Read alignment for ladder
      string path = (boost::format("Align[@component='%1%']/") % ladder).str();
      GearDir params(GearDir(content, "Alignment/"), path);
      if (!params) {
        B2WARNING("Could not find alignment parameters for ladder " << ladder);
        return;
      }
      vxdGeometryPar.getAlignmentMap()[ladder] = VXDAlignmentPar(params.getLength("du"),
                                                                 params.getLength("dv"),
                                                                 params.getLength("dw"),
                                                                 params.getAngle("alpha"),
                                                                 params.getAngle("beta"),
                                                                 params.getAngle("gamma")
                                                                );



      for (const VXDGeoSensorPlacementPar& p : vxdGeometryPar.getLadderMap()[layerID].getSensors()) {
        VxdID sensorID(ladder);
        sensorID.setSensorNumber(p.getSensorID());

        std::map<string, VXDGeoSensorPar>::iterator it = vxdGeometryPar.getSensorMap().find(p.getSensorTypeID());
        if (it == vxdGeometryPar.getSensorMap().end()) {
          B2FATAL("Invalid SensorTypeID " << p.getSensorTypeID() << ", please check the definition of " << sensorID);
        }
        VXDGeoSensorPar& s = it->second;
        string name = m_prefix + "." + (string)sensorID;

        //Now create all the other components and place the Sensor
        if (!vxdGeometryPar.getGlobalParams().getOnlyActiveMaterial()) readSubComponents(s.getComponents() , GearDir(content,
              "Components/"), vxdGeometryPar);
        // Read alignment for sensor
        string pathSensor = (boost::format("Align[@component='%1%']/") % sensorID).str();
        GearDir paramsSensor(GearDir(content, "Alignment/"), pathSensor);
        if (!paramsSensor) {
          B2WARNING("Could not find alignment parameters for sensorID " << sensorID);
          return;
        }
        vxdGeometryPar.getAlignmentMap()[sensorID] = VXDAlignmentPar(paramsSensor.getLength("du"),
                                                     paramsSensor.getLength("dv"),
                                                     paramsSensor.getLength("dw"),
                                                     paramsSensor.getAngle("alpha"),
                                                     paramsSensor.getAngle("beta"),
                                                     paramsSensor.getAngle("gamma")
                                                                    );
      }
      return;
    }

    void GeoVXDCreator::readSubComponents(const std::vector<VXDGeoPlacementPar>& placements , GearDir componentsDir,
                                          VXDGeometryPar& vxdGeometryPar)
    {
      for (const VXDGeoPlacementPar& p : placements) {
        readComponent(p.getName(), componentsDir, vxdGeometryPar);
      }
      return;
    }

    void GeoVXDCreator::readComponent(const std::string& name, GearDir componentsDir, VXDGeometryPar& vxdGeometryPar)
    {


      //Check if component already exists
      if (vxdGeometryPar.getComponentMap().find(name) != vxdGeometryPar.getComponentMap().end()) {
        return; // nothing to do
      }

      //Component does not exist, so lets create a new one
      string path = (boost::format("descendant::Component[@name='%1%']/") % name).str();
      GearDir params(componentsDir, path);
      if (!params) {
        B2FATAL("Could not find definition for component " << name);
        return;
      }

      VXDGeoComponentPar c(
        params.getString("Material",  vxdGeometryPar.getGlobalParams().getDefaultMaterial()),
        params.getString("Color", ""),
        params.getLength("width", 0),
        params.getLength("width2", 0),
        params.getLength("length", 0),
        params.getLength("height", 0),
        params.getAngle("angle", 0)
      );

      if (c.getWidth() <= 0 || c.getLength() <= 0 || c.getHeight() <= 0) {
        B2DEBUG(100, "One dimension empty, using auto resize for component");
      }

      c.setSubComponents(getSubComponents(params));
      readSubComponents(c.getSubComponents(), componentsDir, vxdGeometryPar);

      if (vxdGeometryPar.getGlobalParams().getActiveChips() && params.exists("activeChipID")) {
        int chipID = params.getInt("activeChipID");
        vxdGeometryPar.getSensitiveChipIdMap()[name] = chipID;
      }
      vxdGeometryPar.getComponentMap()[name] = c;
      vxdGeometryPar.getComponentInsertOder().push_back(name);
    }

    void GeoVXDCreator::readLadder(int layer, GearDir components, VXDGeometryPar& geoparameters)
    {
      string path = (boost::format("Ladder[@layer=%1%]/") % layer).str();
      GearDir paramsLadder(components, path);
      if (!paramsLadder) {
        B2FATAL("Could not find Ladder definition for layer " << layer);
      }

      geoparameters.getLadderMap()[layer] = VXDGeoLadderPar(
                                              layer,
                                              paramsLadder.getLength("shift"),
                                              paramsLadder.getLength("radius"),
                                              paramsLadder.getAngle("slantedAngle", 0),
                                              paramsLadder.getLength("slantedRadius", 0),
                                              paramsLadder.getLength("Glue/oversize", 0),
                                              paramsLadder.getString("Glue/Material", "")
                                            );

      for (const GearDir& sensorInfo : paramsLadder.getNodes("Sensor")) {

        geoparameters.getLadderMap()[layer].addSensor(VXDGeoSensorPlacementPar(
                                                        sensorInfo.getInt("@id"),
                                                        sensorInfo.getString("@type"),
                                                        sensorInfo.getLength("."),
                                                        sensorInfo.getBool("@flipU", false),
                                                        sensorInfo.getBool("@flipV", false),
                                                        sensorInfo.getBool("@flipW", false)
                                                      ));
      }
    }

    std::vector<VXDGeoPlacementPar> GeoVXDCreator::getSubComponents(GearDir path)
    {
      vector<VXDGeoPlacementPar> result;
      for (const GearDir& component : path.getNodes("Component")) {
        string type;
        if (!component.exists("@type")) {
          type = component.getString("@name");
        } else {
          type = component.getString("@type");
        }
        int nPos = max(component.getNumberNodes("u"), component.getNumberNodes("v"));
        nPos = max(nPos, component.getNumberNodes("w"));
        nPos = max(nPos, component.getNumberNodes("woffset"));
        for (int iPos = 1; iPos <= nPos; ++iPos) {
          string index = (boost::format("[%1%]") % iPos).str();
          result.push_back(VXDGeoPlacementPar(
                             type,
                             component.getLength("u" + index, 0),
                             component.getLength("v" + index, 0),
                             component.getString("w" + index, "bottom"),
                             component.getLength("woffset" + index, 0)
                           ));
        }
      }
      return result;
    }

  }  // namespace VXD
}  // namespace Belle2
