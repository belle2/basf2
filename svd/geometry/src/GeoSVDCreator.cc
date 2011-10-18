/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Jozef Koval                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geometry/GeoSVDCreator.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>

#include <G4LogicalVolume.hh>
#include <G4Trd.hh>
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

using namespace std;

namespace Belle2 {

  using namespace geometry;
  namespace svd {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoSVDCreator> GeoSVDFactory("SVDCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoSVDCreator::GeoSVDCreator()
    {
    }

    GeoSVDCreator::~GeoSVDCreator()
    {
      VXD::GeoCache::getInstance().clear();
      //Delete all sensitive detectors
      BOOST_FOREACH(SensitiveDetector* sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    vector<GeoSVDPlacement> GeoSVDCreator::getSubComponents(GearDir path)
    {
      vector<GeoSVDPlacement> result;
      BOOST_FOREACH(const GearDir& component, path.getNodes("Component")) {
        string type = component.getString("@type");
        int nPos = max(component.getNumberNodes("u"), component.getNumberNodes("v"));
        for (int iPos = 1; iPos <= nPos; ++iPos) {
          string index = (boost::format("[%1%]") % iPos).str();
          result.push_back(GeoSVDPlacement(
                             type,
                             component.getLength("u" + index, 0) / Unit::mm,
                             component.getLength("v" + index, 0) / Unit::mm,
                             component.getString("w" + index, "bottom")
                           ));
        }
      }
      return result;
    }

    GeoSVDComponent GeoSVDCreator::getComponent(const string &name)
    {
      //Ceck if component already exists
      map<string, GeoSVDComponent >::iterator cached = m_componentCache.find(name);
      if (cached != m_componentCache.end()) {
        return cached->second;
      }
      //Not cached, so lets create a new one
      string path = (boost::format("Component[@name='%1%']/") % name).str();
      GearDir params(m_components, path);
      if (!params) B2FATAL("Could not find definition for component " << name);

      GeoSVDComponent c(
        params.getString("Material", "Air"),
        params.getString("Color", ""),
        params.getLength("width", 0) / Unit::mm,
        params.getLength("width2", 0) / Unit::mm,
        params.getLength("length", 0) / Unit::mm,
        params.getLength("height", 0) / Unit::mm
      );
      double angle  = params.getAngle("angle", 0);

      if (c.width == 0 || c.length == 0 || c.height == 0) {
        B2DEBUG(100, "One dimension empty, using auto resize for component");
      } else {
        G4VSolid* solid = createTrapezoidal("SVD." + name, c.width, c.width2, c.length, c.height, angle);
        c.volume = new G4LogicalVolume(solid, Materials::get(c.material), "SVD." + name);
      }
      vector<GeoSVDPlacement> subComponents = getSubComponents(params);
      addSubComponents("SVD." + name, c, subComponents);
      m_componentCache[name] = c;
      return c;
    }

    double GeoSVDCreator::addSubComponents(const string& name, GeoSVDComponent &component, vector<GeoSVDPlacement> placements, bool createContainer, bool originCenter)
    {
      B2DEBUG(100, "Creating component " << name);
      vector<GeoSVDComponent> subComponents;
      subComponents.reserve(placements.size());
      double heightAbove = 0;
      double heightBelow = 0;
      //Go over all subcomponents and check if they will fit inside.
      //If component.volume is zero we will create one so sum up needed space
      BOOST_FOREACH(GeoSVDPlacement &p, placements) {
        //Flip placement if component if flipped dimensions
        if (component.flipU) p.u = -p.u;
        if (component.flipV) p.v = -p.v;
        if (component.flipW) p.w = (GeoSVDPlacement::EPosW)(GeoSVDPlacement::c_above - p.w);

        GeoSVDComponent sub = getComponent(p.name);
        B2DEBUG(100, "SubComponent " << p.name);
        B2DEBUG(100, boost::format("Placement: %1%, %2% cm") % p.u % p.v);
        B2DEBUG(100, boost::format("Dimensions: %1%x%2%x%3% cm") % sub.width % sub.length % sub.height);

        if (p.w == GeoSVDPlacement::c_above) {
          //Above placement only valid if we are allowed to create a container around component
          if (!createContainer) B2FATAL("Cannot place component " << p.name << " outside of component " << name);
          heightAbove = max(heightAbove, sub.height);
        } else if (p.w == GeoSVDPlacement::c_below) {
          //Below placement only valid if we are allowed to create a container around component
          if (!createContainer) B2FATAL("Cannot place component " << p.name << " outside of component " << name);
          heightBelow = max(heightAbove, sub.height);
        } else if (sub.height > component.height) {
          //Component will not fit heightwise. If we resize the volume anyway than we don't have problems
          if (component.volume) B2FATAL("Subcomponent " << p.name << " does not fit into volume: "
                                          << "height " << sub.height << " > " << component.height
                                         );
          component.height = sub.height;
        }

        //Check if compoent will fit inside width,length. If we can resize do it if needed, otherwise bail
        double minWidth =  max(abs(p.u + sub.width / 2.0), abs(p.u - sub.width / 2.0));
        double minLength = max(abs(p.v + sub.length / 2.0), abs(p.v - sub.length / 2.0));
        if (minWidth > component.width) {
          if (component.volume) B2FATAL("Subcomponent " << p.name << " does not fit into volume: "
                                          << "minWidth " << minWidth << " > " << component.width
                                         );
          component.width = minWidth * 2.0;
          component.width2 = minWidth * 2.0;
        }
        if (minLength > component.length) {
          if (component.volume) B2FATAL("Subcomponent " << p.name << " does not fit into volume: "
                                          << "minLength " << minLength << " > " << component.length
                                         );
          component.length = minLength * 2.0;
        }
        subComponents.push_back(sub);
      }

      //zero dimensions are fine mathematically but we don't want them in the simulation
      if (component.width == 0 || component.length == 0 || component.height == 0) {
        B2FATAL("At least one dimension of component " << name << " is zero which does not make sense");
      }

      //No volume yet, create a new one automatically assuming air material
      if (!component.volume) {
        G4VSolid *componentShape = createTrapezoidal(name, component.width, component.width2, component.length, component.height);
        component.volume = new G4LogicalVolume(componentShape, Materials::get(component.material), name);
        if (component.material == "Air") {
          B2DEBUG(200, "Component " << name << " is an Air volume, setting invisible");
          setVisibility(*component.volume, false);
        }
      }

      //See if we are allowed to create an container for the component and if it is neccessary
      G4LogicalVolume *container(0);
      double componentW(0);
      if (createContainer && (heightAbove > 0 || heightBelow > 0)) {
        double height = component.height + heightAbove + heightBelow;
        G4VSolid *containerShape = createTrapezoidal(name, component.width, component.width2, component.length, height);        container = new G4LogicalVolume(containerShape, Materials::get("Air"), name + ".Container");
        componentW = -(heightAbove - heightBelow) / 2.0;
        new G4PVPlacement(0, G4ThreeVector(0, 0, componentW), component.volume, name + ".Container", container, false, 1);
      }
      B2DEBUG(100, boost::format("Component %1% dimensions: %2%x%3%x%4% cm") % name % component.width % component.length % component.height);

      //Ok, all volumes set up, now add them together
      for (size_t i = 0; i < placements.size(); ++i) {
        GeoSVDPlacement &p = placements[i];
        GeoSVDComponent &s = subComponents[i];
        G4LogicalVolume* mother(0);
        double w = 0;
        switch (p.w) {
          case GeoSVDPlacement::c_below: //Place below component
            mother = container;
            w = componentW - component.height / 2.0 - s.height / 2.0;
            break;
          case GeoSVDPlacement::c_bottom: //Place inside, at bottom of component
            mother = component.volume;
            w = - component.height / 2.0 + s.height / 2.0;
            break;
          case GeoSVDPlacement::c_center: //Place inside, centered
            mother = component.volume;
            w = 0;
            break;
          case GeoSVDPlacement::c_top:    //Place inside, at top of component
            mother = component.volume;
            w = component.height / 2.0 - s.height / 2.0;
            break;
          case GeoSVDPlacement::c_above:  //Place above component
            mother = container;
            w = componentW + component.height / 2.0 + s.height / 2.0;
            break;
        }
        if (!originCenter) { //Sensor has coordinate origin in the corner, all subcomponents at their center
          p.u -= component.width / 2.0 * (component.flipU ? -1 : 1);
          p.v -= component.length / 2.0 * (component.flipV ? -1 : 1);
        }
        G4Transform3D transform = G4Translate3D(p.u, p.v, w);
        if (component.flipW) transform = transform * G4RotateY3D(M_PI);
        //Add to selected mother (either component or container around component
        new G4PVPlacement(transform, s.volume, "", mother, false, i);
      }

      //Set some visibility options for volume. Done here because all components including sensor go through here
      if (!component.color.empty()) {
        B2DEBUG(200, "Component " << name << " color: " << component.color);
        setColor(*component.volume, component.color);
      }

      //If we created an container, replace the corresponding values in the Component
      if (container) {
        setVisibility(*container, false);
        component.volume = container;
        component.height += heightAbove + heightBelow;
      }
      B2DEBUG(100, "--> Created component " << name);
      //Return the difference in W between the origin of the original component and the including container
      return componentW;
    }

    G4Transform3D GeoSVDCreator::getAlignment(const string& component)
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

    G4VSolid* GeoSVDCreator::createTrapezoidal(const string& name, double width, double width2, double length, double &height, double angle)
    {
      double offset(0);
      if (angle != 0) {
        const double tana = tan(angle);
        height = min(tana * length, min(tana * width, height));
        offset = height / tana;
      }
      const double hwidth  = width / 2.0;
      const double hwidth2 = width2 / 2.0;
      const double hlength = length / 2.0;
      const double hheight = height / 2.0;

      if (width2 <= 0 || width == width2) {
        if (angle == 0) {
          return new G4Box(name, hwidth, hlength, hheight);
        } else {
          return new G4Trd(name, hwidth, hwidth - offset, hlength, hlength - offset, hheight);
        }
      }
      //FIXME: offset not working, g4 complains about nonplanarity of face -X. But we do not need that shape at the moment
      //so lets ignore it for now
      return  new G4Trap(name, hheight, 0, 0, hlength, hwidth, hwidth2, 0, hlength - offset, hwidth - offset, hwidth2 - offset, 0);
    }



    void GeoSVDCreator::addLadder(int ladderID, double phi, G4LogicalVolume* volume, const G4Transform3D& placement)
    {
      VxdID ladder(m_ladder.layerID, ladderID, 0);

      G4Translate3D ladderPos(m_ladder.radius, m_ladder.shift, 0);
      G4Transform3D ladderPlacement = placement * G4RotateZ3D(phi) * ladderPos * getAlignment(ladder);

      BOOST_FOREACH(GeoSVDSensor s, m_ladder.sensors) {
        VxdID sensorID(ladder);
        sensorID.setSensor(s.sensorID);
        string name = "SVD." + (string)sensorID;
        G4VSolid* sensorShape = createTrapezoidal(name, s.width, s.width2, s.length, s.height);

        s.volume = new G4LogicalVolume(sensorShape, Materials::get(s.material), name);
        // Create sensitive Area: this Part is created separately since we want full control over the coordinate system:
        // local x (called u) should point in RPhi direction
        // local y (called v) should point in global z
        // local z (called w) should away from the origin
        G4VSolid* activeShape = createTrapezoidal(name + ".Active", s.active.width, s.active.width2,
                                                  s.active.length, s.active.height);
        //Create appropriate sensitive detector instance
        SensorInfo* sensorInfo = new SensorInfo(s.info);
        sensorInfo->setID(sensorID);
        SensitiveDetector *sensitive = new SensitiveDetector(sensorInfo);
        m_sensitive.push_back(sensitive);

        G4LogicalVolume* active = new G4LogicalVolume(activeShape,  Materials::get(s.material), name + ".Active",
                                                      0, sensitive);
        active->SetUserLimits(new G4UserLimits(s.active.stepSize));
        setColor(*active, "#ddd");
        //The coordinates of the active region are given relative to the corner of the sensor, not to the center
        //so we convert them. If the sensor is flipped, the sign of coordinates need to be inverted
        double activeU = (s.active.u + (s.active.width - s.width) / 2.0) * (s.flipU ? -1 : 1);
        double activeV = (s.active.v + (s.active.length - s.length) / 2.0) * (s.flipV ? -1 : 1);
        //active area is always in the middle, so active.w would be active.height, otherwise the same as above
        double activeW = (s.active.height - s.height) / 2.0 * (s.flipW ? -1 : 1);
        //Place the active area
        new G4PVPlacement(G4Translate3D(activeU, activeV, activeW), active, name + ".Active",
                          s.volume, false, (int)sensorID);
        //Draw local axes to check orientation
        //Beware: do not enable for simulation, this is purely for visualization
        /*{
          G4LogicalVolume* uBox = new G4LogicalVolume(new G4Box("u",5*mm,0.1*mm,0.1*mm), Materials::get(s.material), "u");
          G4LogicalVolume* vBox = new G4LogicalVolume(new G4Box("v",0.1*mm,5*mm,0.1*mm), Materials::get(s.material), "u");
          G4LogicalVolume* wBox = new G4LogicalVolume(new G4Box("w",0.1*mm,0.1*mm,5*mm), Materials::get(s.material), "u");
          setColor(*uBox,"#f00");
          setColor(*vBox,"#0f0");
          setColor(*wBox,"#00f");
          new G4PVPlacement(G4Translate3D(5*mm,0,0),uBox,"u",active,false,1);
          new G4PVPlacement(G4Translate3D(0,5*mm,0),vBox,"v",active,false,1);
          new G4PVPlacement(G4Translate3D(0,0,5*mm),wBox,"w",active,false,1);
        }*/

        //Now create all the other components and place the Sensor
        double shiftX = addSubComponents(name, s, s.components, true, false);
        G4RotationMatrix rotation(0, -M_PI / 2.0, -M_PI / 2.0);
        G4Transform3D sensorAlign = getAlignment(sensorID);
        G4Transform3D placement = G4Rotate3D(rotation) * sensorAlign;
        //SensorID=2 is slanted FIXME: this distinction is a bit arbitary, we should make that more clear
        if (s.sensorTypeID == 2) {
          placement = G4TranslateX3D(m_ladder.slantedRadius - m_ladder.radius) * G4RotateY3D(-m_ladder.slantedAngle) * placement;
        }
        placement = ladderPlacement * G4Translate3D(-shiftX, 0.0, s.z) * placement;

        new G4PVPlacement(placement, s.volume, name, volume, false, 1);
      }
    }

    void GeoSVDCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {
      m_alignment = GearDir(content, "Alignment/");
      m_components = GearDir(content, "Components/");

      //Build envelope
      G4LogicalVolume* envelope(0);
      GearDir envelopeParams(content, "Envelope");
      if (!envelopeParams) {
        B2FATAL("Could not find definition for SVD Envelope.");
      }
      double minZ(0), maxZ(0);
      G4Polycone *envelopeCone = geometry::createPolyCone("Envelope", GearDir(content, "Envelope/"), minZ, maxZ);
      string materialName = content.getString("Envelope/Material", "Air");
      G4Material* material = Materials::get(materialName);
      if (!material) B2FATAL("Material '" << materialName << "', required by SVD Envelope could not be found");
      envelope = new G4LogicalVolume(envelopeCone, material, "SVD");
      setColor(*envelope, "#f00");
      setVisibility(*envelope, false);
      G4Region* svdRegion = G4RegionStore::GetInstance()->FindOrCreateRegion("SVD");
      envelope->SetRegion(svdRegion);
      svdRegion->AddRootLogicalVolume(envelope);
      G4VPhysicalVolume* physEnvelope = new G4PVPlacement(getAlignment("SVD"), envelope, "SVD", &topVolume, false, 1);

      //Read the definition of all sensor types
      BOOST_FOREACH(const GearDir &paramsSensor, m_components.getNodes("Sensor")) {
        int sensorTypeID = paramsSensor.getInt("@sensorType");
        GeoSVDSensor sensor(
          paramsSensor.getString("Material"),
          paramsSensor.getString("Color", ""),
          paramsSensor.getLength("width") / Unit::mm,
          paramsSensor.getLength("width2", 0) / Unit::mm,
          paramsSensor.getLength("length") / Unit::mm,
          paramsSensor.getLength("height") / Unit::mm
        );
        sensor.active = GeoSVDActiveArea(
                          paramsSensor.getLength("Active/u") / Unit::mm,
                          paramsSensor.getLength("Active/v") / Unit::mm,
                          paramsSensor.getLength("Active/width") / Unit::mm,
                          paramsSensor.getLength("Active/width2", 0) / Unit::mm,
                          paramsSensor.getLength("Active/length") / Unit::mm,
                          paramsSensor.getLength("Active/height") / Unit::mm,
                          paramsSensor.getLength("Active/stepSize") / Unit::mm
                        );
        sensor.info = SensorInfo(
                        VxdID(0, 0, 0),
                        paramsSensor.getLength("Active/width"),
                        paramsSensor.getLength("Active/length"),
                        paramsSensor.getLength("Active/height"),
                        paramsSensor.getInt("Active/stripsU", 0),
                        paramsSensor.getInt("Active/stripsV", 0),
                        paramsSensor.getLength("Active/width2", 0)
                      );

        sensor.components = getSubComponents(paramsSensor);
        m_sensorMap.insert(make_pair(sensorTypeID, sensor));
      }

      //Build all ladders including Sensors
      BOOST_FOREACH(const GearDir &shell, content.getNodes("HalfShell")) {
        string shellName =  shell.getString("@name");
        B2INFO("Building SVD half-shell " << shellName);
        G4Transform3D shellAlignment = getAlignment("SVD." + shellName);

        BOOST_FOREACH(const GearDir &layer, shell.getNodes("Layer")) {
          int layerID = layer.getInt("@id");
          setLayer(layerID);

          //Loop over defined ladders
          BOOST_FOREACH(const GearDir &ladder, layer.getNodes("Ladder")) {
            int ladderID = ladder.getInt("@id");
            double phi = ladder.getAngle("phi", 0);
            addLadder(ladderID, phi, envelope, shellAlignment);
          }
        }
      }

      //Now build cache with all transformations
      VXD::GeoCache::getInstance().findVolumes(physEnvelope);

      //Free some space
      m_componentCache.clear();
      m_sensorMap.clear();
    }

    void GeoSVDCreator::setLayer(int layer)
    {
      string path = (boost::format("Ladder[@layer=%1%]/") % layer).str();
      GearDir paramsLadder(m_components, path);
      if (!paramsLadder) {
        B2FATAL("Could not find Ladder definition for layer " << layer);
      }
      m_ladder = GeoSVDLadder(
                   layer,
                   paramsLadder.getLength("shift") / Unit::mm,
                   paramsLadder.getLength("radius") / Unit::mm,
                   paramsLadder.getAngle("slantedAngle", 0),
                   paramsLadder.getLength("slantedRadius", 0) / Unit::mm
                 );

      BOOST_FOREACH(const GearDir &sensorInfo, paramsLadder.getNodes("Sensor")) {
        int sensorTypeID = sensorInfo.getInt("@sensorType");
        m_sensorMapIterator = m_sensorMap.find(sensorTypeID);
        m_sensorMapIterator->second.sensorID = sensorInfo.getInt("@id");
        m_sensorMapIterator->second.z = sensorInfo.getLength(".") / Unit::mm;
        m_sensorMapIterator->second.flipU = sensorInfo.getBool("@flipU", false);
        m_sensorMapIterator->second.flipV = sensorInfo.getBool("@flipV", false);
        m_sensorMapIterator->second.flipW = sensorInfo.getBool("@flipW", false);
        m_sensorMapIterator->second.sensorTypeID = sensorInfo.getInt("@sensorType");
        m_ladder.sensors.push_back(m_sensorMapIterator->second);
      }
    }
  }  // namespace svd
}  // namespace Belle2
