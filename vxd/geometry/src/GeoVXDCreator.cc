/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Jozef Koval                               *
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

    GeoVXDAssembly GeoVXDCreator::createHalfShellSupport(GearDir) { return GeoVXDAssembly(); }

    GeoVXDAssembly GeoVXDCreator::createLayerSupport(int, GearDir) { return GeoVXDAssembly(); }

    GeoVXDAssembly GeoVXDCreator::createLadderSupport(int, GearDir) { return GeoVXDAssembly(); }

    vector<VXDGeoPlacement> GeoVXDCreator::getSubComponents(GearDir path)
    {
      vector<VXDGeoPlacement> result;
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
          result.push_back(VXDGeoPlacement(
                             type,
                             component.getLength("u" + index, 0) / Unit::mm,
                             component.getLength("v" + index, 0) / Unit::mm,
                             component.getString("w" + index, "bottom"),
                             component.getLength("woffset" + index, 0) / Unit::mm
                           ));
        }
      }
      return result;
    }

    VXDGeoComponent GeoVXDCreator::getComponent(const string& name)
    {
      //Check if component already exists
      map<string, VXDGeoComponent>::iterator cached = m_componentCache.find(name);
      if (cached != m_componentCache.end()) {
        return cached->second;
      }
      //Not cached, so lets create a new one
      string path = (boost::format("descendant::Component[@name='%1%']/") % name).str();
      GearDir params(m_components, path);
      if (!params) B2FATAL("Could not find definition for component " << name);

      VXDGeoComponent c(
        params.getString("Material", m_defaultMaterial),
        params.getString("Color", ""),
        params.getLength("width", 0) / Unit::mm,
        params.getLength("width2", 0) / Unit::mm,
        params.getLength("length", 0) / Unit::mm,
        params.getLength("height", 0) / Unit::mm
      );
      double angle  = params.getAngle("angle", 0);

      if (c.getWidth() <= 0 || c.getLength() <= 0 || c.getHeight() <= 0) {
        B2DEBUG(100, "One dimension empty, using auto resize for component");
      } else {
        G4VSolid* solid = createTrapezoidal(m_prefix + "." + name, c.getWidth(), c.getWidth2(), c.getLength(), c.getHeight(), angle);
        c.setVolume(new G4LogicalVolume(solid, Materials::get(c.getMaterial()), m_prefix + "." + name));
      }
      vector<VXDGeoPlacement> subComponents = getSubComponents(params);
      createSubComponents(m_prefix + "." + name, c, subComponents);
      if (m_activeChips && params.exists("activeChipID")) {
        int chipID = params.getInt("activeChipID");
        B2DEBUG(50, "Creating BkgSensitiveDetector for component " << name << " with chipID " <<  chipID);
        BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(m_prefix.c_str(), chipID);
        c.getVolume()->SetSensitiveDetector(sensitive);
        m_sensitive.push_back(sensitive);
      }
      m_componentCache[name] = c;
      return c;
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

        VXDGeoComponent sub = getComponent(p.getName());
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

    G4Transform3D GeoVXDCreator::getAlignment(const string& component)
    {
      string path = (boost::format("Align[@component='%1%']/") % component).str();
      GearDir params(m_alignment, path);
      if (!params) {
        B2WARNING("Could not find alignment parameters for component " << component);
        return G4Transform3D();
      }
      double dU = params.getLength("du") / Unit::mm;
      double dV = params.getLength("dv") / Unit::mm;
      double dW = params.getLength("dw") / Unit::mm;
      double alpha = params.getAngle("alpha");
      double beta  = params.getAngle("beta");
      double gamma = params.getAngle("gamma");
      G4RotationMatrix rotation(alpha, beta, gamma);
      G4ThreeVector translation(dU, dV, dW);
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



    G4Transform3D GeoVXDCreator::placeLadder(int ladderID, double phi, G4LogicalVolume* volume, const G4Transform3D& placement)
    {
      VxdID ladder(m_ladder.getLayerID(), ladderID, 0);

      G4Translate3D ladderPos(m_ladder.getRadius(), m_ladder.getShift(), 0);
      G4Transform3D ladderPlacement = placement * G4RotateZ3D(phi) * ladderPos * getAlignment(ladder);

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

        G4VSolid* sensorShape = createTrapezoidal(name, s.getWidth(), s.getWidth2(), s.getLength(), s.getHeight());
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

        // Draw local axes to check orientation
        // Beware: do not enable for simulation, this is purely for visualization
        // {
        //   G4LogicalVolume* uBox = new G4LogicalVolume(new G4Box("u",5*CLHEP::mm,0.1*CLHEP::mm,0.1*CLHEP::mm), Materials::get(s.material), "u");
        //   G4LogicalVolume* vBox = new G4LogicalVolume(new G4Box("v",0.1*CLHEP::mm,5*CLHEP::mm,0.1*CLHEP::mm), Materials::get(s.material), "u");
        //   G4LogicalVolume* wBox = new G4LogicalVolume(new G4Box("w",0.1*CLHEP::mm,0.1*CLHEP::mm,5*CLHEP::mm), Materials::get(s.material), "u");
        //   setColor(*uBox,"#f00");
        //   setColor(*vBox,"#0f0");
        //   setColor(*wBox,"#00f");
        //   new G4PVPlacement(G4Translate3D(5*CLHEP::mm,0,0),uBox,"u",active,false,1);
        //   new G4PVPlacement(G4Translate3D(0,5*CLHEP::mm,0),vBox,"v",active,false,1);
        //   new G4PVPlacement(G4Translate3D(0,0,5*CLHEP::mm),wBox,"w",active,false,1);
        // }

        setColor(*active, s.getActiveArea().getColor());
        //The coordinates of the active region are given as the distance between the corners, not to the center
        //Place the active area
        G4Transform3D activePosition = G4Translate3D(s.getActiveArea().getWidth() / 2.0, s.getActiveArea().getLength() / 2.0, 0) *
                                       getPosition(s, s.getActiveArea(), s.getActivePlacement(), false);
        G4ReflectionFactory::Instance()->Place(activePosition * reflection, name + ".Active", active, s.getVolume(),
                                               false, (int)sensorID, false);

        //Now create all the other components and place the Sensor
        GeoVXDAssembly assembly;
        if (!m_onlyActiveMaterial) assembly = createSubComponents(name, s, s.getComponents(), false, true);
        G4RotationMatrix rotation(0, -M_PI / 2.0, -M_PI / 2.0);
        G4Transform3D sensorAlign = getAlignment(sensorID);
        G4Transform3D placement = G4Rotate3D(rotation) * sensorAlign * reflection;

        if (s.getSlanted()) {
          placement = G4TranslateX3D(m_ladder.getSlantedRadius() - m_ladder.getRadius()) * G4RotateY3D(
                        -m_ladder.getSlantedAngle()) * placement;
        }
        placement = ladderPlacement * G4Translate3D(0.0, 0.0, p.getZ()) * placement;

        assembly.add(s.getVolume());
        assembly.place(volume, placement);

        //See if we want to glue the modules together
        if (!m_ladder.getGlueMaterial().empty() && !m_onlyActiveMaterial) {
          double u = s.getWidth() / 2.0 + m_ladder.getGlueSize();
          double v = s.getLength() / 2.0;
          double w = s.getHeight() / 2.0 + m_ladder.getGlueSize();
          std::vector<G4Point3D> curSensorEdge(4);
          //Lets get the forward corners of the sensor by applying the unreflected placement matrix
          curSensorEdge[0] = placement * reflection * G4Point3D(u, v, + w);
          curSensorEdge[1] = placement * reflection * G4Point3D(u, v, - w);
          curSensorEdge[2] = placement * reflection * G4Point3D(-u, v, - w);
          curSensorEdge[3] = placement * reflection * G4Point3D(-u, v, + w);
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
          lastSensorEdge[0] = placement * reflection * G4Point3D(u, -v, + w);
          lastSensorEdge[1] = placement * reflection * G4Point3D(u, -v, - w);
          lastSensorEdge[2] = placement * reflection * G4Point3D(-u, -v, - w);
          lastSensorEdge[3] = placement * reflection * G4Point3D(-u, -v, + w);
        }
      }
      return ladderPlacement;
    }

    void GeoVXDCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      m_activeStepSize = content.getLength("ActiveStepSize", m_activeStepSize) / Unit::mm;
      m_activeChips = content.getBool("ActiveChips", m_activeChips);
      m_seeNeutrons = content.getBool("SeeNeutrons", m_seeNeutrons);
      m_onlyPrimaryTrueHits = content.getBool("OnlyPrimaryTrueHits", m_onlyPrimaryTrueHits);
      m_distanceTolerance = (float)content.getLength("DistanceTolerance", m_distanceTolerance);
      m_electronTolerance = (float)content.getDouble("ElectronTolerance", m_electronTolerance);
      m_minimumElectrons = (float)content.getDouble("MinimumElectrons", m_minimumElectrons);
      m_onlyActiveMaterial = content.getBool("OnlyActiveMaterial", m_onlyActiveMaterial);
      m_alignment = GearDir(content, "Alignment/");
      m_components = GearDir(content, "Components/");
      GearDir support(content, "Support/");

      m_defaultMaterial = content.getString("DefaultMaterial", "Air");
      G4Material* material = Materials::get(m_defaultMaterial);
      if (!material) B2FATAL("Default Material of VXD, '" << m_defaultMaterial << "', could not be found");

      //Build envelope
      G4LogicalVolume* envelope(0);
      GearDir envelopeParams(content, "Envelope");
      G4VPhysicalVolume* physEnvelope{nullptr};
      if (!envelopeParams) {
        B2INFO("Could not find definition for " + m_prefix + " Envelope, placing directly in top volume");
        envelope = &topVolume;
      } else {
        double minZ(0), maxZ(0);
        G4Polycone* envelopeCone = geometry::createRotationSolid("Envelope", GearDir(content, "Envelope/"), minZ, maxZ);
        envelope = new G4LogicalVolume(envelopeCone, material, m_prefix + ".Envelope");
        setVisibility(*envelope, false);
        physEnvelope = new G4PVPlacement(getAlignment(m_prefix), envelope, m_prefix + ".Envelope", &topVolume, false, 1);
      }

      //Read the definition of all sensor types
      for (const GearDir& paramsSensor : m_components.getNodes("Sensor")) {
        string sensorTypeID = paramsSensor.getString("@type");
        VXDGeoSensor sensor(
          paramsSensor.getString("Material"),
          paramsSensor.getString("Color", ""),
          paramsSensor.getLength("width") / Unit::mm,
          paramsSensor.getLength("width2", 0) / Unit::mm,
          paramsSensor.getLength("length") / Unit::mm,
          paramsSensor.getLength("height") / Unit::mm,
          paramsSensor.getBool("@slanted", false)
        );
        sensor.setActive(VXDGeoComponent(
                           paramsSensor.getString("Material"),
                           paramsSensor.getString("Active/Color", "#f00"),
                           paramsSensor.getLength("Active/width") / Unit::mm,
                           paramsSensor.getLength("Active/width2", 0) / Unit::mm,
                           paramsSensor.getLength("Active/length") / Unit::mm,
                           paramsSensor.getLength("Active/height") / Unit::mm
                         ), VXDGeoPlacement(
                           "Active",
                           paramsSensor.getLength("Active/u") / Unit::mm,
                           paramsSensor.getLength("Active/v") / Unit::mm,
                           paramsSensor.getString("Active/w", "center"),
                           paramsSensor.getLength("Active/woffset", 0) / Unit::mm
                         ));
        sensor.setSensorInfo(createSensorInfo(GearDir(paramsSensor, "Active")));
        sensor.setComponents(getSubComponents(paramsSensor));
        m_sensorMap[sensorTypeID] = sensor;
      }

      //Build all ladders including Sensors
      GeoVXDAssembly shellSupport = createHalfShellSupport(support);
      for (const GearDir& shell : content.getNodes("HalfShell")) {
        string shellName =  shell.getString("@name");
        B2INFO("Building " << m_prefix << " half-shell " << shellName);
        G4Transform3D shellAlignment = getAlignment(m_prefix + "." + shellName);

        //Place shell support
        double shellAngle = shell.getAngle("shellAngle", 0);
        if (!m_onlyActiveMaterial) shellSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));

        for (const GearDir& layer : shell.getNodes("Layer")) {
          int layerID = layer.getInt("@id");
          setCurrentLayer(layerID);

          //Place Layer support
          GeoVXDAssembly layerSupport = createLayerSupport(layerID, support);
          if (!m_onlyActiveMaterial) layerSupport.place(envelope, shellAlignment * G4RotateZ3D(shellAngle));
          GeoVXDAssembly ladderSupport = createLadderSupport(layerID, support);

          //Loop over defined ladders
          for (const GearDir& ladder : layer.getNodes("Ladder")) {
            int ladderID = ladder.getInt("@id");
            double phi = ladder.getAngle("phi", 0);
            G4Transform3D ladderPlacement = placeLadder(ladderID, phi, envelope, shellAlignment);
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
      GearDir radiationDir(content, "RadiationSensors");
      if (m_activeChips && radiationDir) {
        m_radiationsensors.create(radiationDir, topVolume, *envelope);
      }

      //Free some space
      m_componentCache.clear();
      //FIXME: delete SensorInfo instances
      m_sensorMap.clear();
    }

    void GeoVXDCreator::setCurrentLayer(int layer)
    {
      string path = (boost::format("Ladder[@layer=%1%]/") % layer).str();
      GearDir paramsLadder(m_components, path);
      if (!paramsLadder) {
        B2FATAL("Could not find Ladder definition for layer " << layer);
      }
      m_ladder = VXDGeoLadder(
                   layer,
                   paramsLadder.getLength("shift") / Unit::mm,
                   paramsLadder.getLength("radius") / Unit::mm,
                   paramsLadder.getAngle("slantedAngle", 0),
                   paramsLadder.getLength("slantedRadius", 0) / Unit::mm,
                   paramsLadder.getLength("Glue/oversize", 0) / Unit::mm,
                   paramsLadder.getString("Glue/Material", "")
                 );

      for (const GearDir& sensorInfo : paramsLadder.getNodes("Sensor")) {
        m_ladder.addSensor(VXDGeoSensorPlacement(
                             sensorInfo.getInt("@id"),
                             sensorInfo.getString("@type"),
                             sensorInfo.getLength(".") / Unit::mm,
                             sensorInfo.getBool("@flipU", false),
                             sensorInfo.getBool("@flipV", false),
                             sensorInfo.getBool("@flipW", false)
                           ));
      }
    }
  }  // namespace VXD
}  // namespace Belle2
