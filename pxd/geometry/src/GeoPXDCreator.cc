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
#include <pxd/vxd/VxdID.h>
#include <pxd/vxd/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <pxd/simulation/SensitiveDetector.h>

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

using namespace std;
using namespace boost;

namespace Belle2 {


  using namespace geometry;

  namespace pxd {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoPXDCreator> GeoPXDFactory("PXDCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoPXDCreator::GeoPXDCreator()
    {
    }

    GeoPXDCreator::~GeoPXDCreator()
    {
      //Lets assume that is impossible that SVD lives on while PXD gets destructed,
      //so we can just clean the geometry cache here
      VXD::GeoCache::getInstance().clear();

      //Delete all sensitive detectors
      BOOST_FOREACH(SensitiveDetector* sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    G4Transform3D GeoPXDCreator::getAlignment(const string& component)
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

    vector<GeoPXDPlacement> GeoPXDCreator::getSubComponents(GearDir path)
    {
      vector<GeoPXDPlacement> result;
      BOOST_FOREACH(const GearDir& component, path.getNodes("Component")) {
        string type = component.getString("@type");
        int nPos = max(component.getNumberNodes("u"), component.getNumberNodes("v"));
        for (int iPos = 1; iPos <= nPos; ++iPos) {
          string index = (boost::format("[%1%]") % iPos).str();
          result.push_back(GeoPXDPlacement(
                             type,
                             component.getLength("u" + index, 0) / Unit::mm,
                             component.getLength("v" + index, 0) / Unit::mm,
                             component.getString("w" + index, "bottom")
                           ));
        }
      }
      return result;
    }

    GeoPXDComponent GeoPXDCreator::getComponent(const string &name)
    {
      //Ceck if component already exists
      map<string, GeoPXDComponent >::iterator cached = m_componentCache.find(name);
      if (cached != m_componentCache.end()) {
        return cached->second;
      }
      //Not cached, so lets create a new one
      string path = (boost::format("Component[@name='%1%']/") % name).str();
      GearDir params(m_components, path);
      if (!params) B2FATAL("Could not find definition for component " << name);

      GeoPXDComponent c(
        params.getString("Material", "Air"),
        params.getString("Color", ""),
        params.getLength("width", 0) / Unit::mm,
        params.getLength("length", 0) / Unit::mm,
        params.getLength("height", 0) / Unit::mm
      );
      double angle  = params.getAngle("angle", 0);

      if (c.width == 0 || c.length == 0 || c.height == 0) {
        B2DEBUG(100, "One dimension empty, using auto resize for component");
      } else {
        G4VSolid* solid;
        if (angle == 0) {
          solid = new G4Box("PXD." + name, c.width / 2.0, c.length / 2.0, c.height / 2.0);
        } else {
          //If we have an angle!=0, the shape is a trapezoidal where all for edges are slanted by angle
          //so we calculate the corresponding values
          const double tana = tan(angle);
          const double maxheight = min(tana * c.length / 2.0, min(tana * c.width / 2.0, c.height));
          const double offset = maxheight / tana;
          c.height = maxheight;
          solid = new G4Trd(("PXD." + name),
                            c.width / 2.0, c.width / 2.0 - offset,
                            c.length / 2.0, c.length / 2.0 - offset,
                            c.height / 2.0
                           );
        }
        c.volume = new G4LogicalVolume(solid, Materials::get(c.material), "PXD." + name);
      }
      vector<GeoPXDPlacement> subComponents = getSubComponents(params);
      addSubComponents("PXD." + name, c, subComponents);
      m_componentCache[name] = c;
      return c;
    }

    double GeoPXDCreator::addSubComponents(const string& name, GeoPXDComponent &component, vector<GeoPXDPlacement> placements, bool createContainer, bool originCenter)
    {
      B2DEBUG(100, "Creating component " << name);
      vector<GeoPXDComponent> subComponents;
      subComponents.reserve(placements.size());
      double heightAbove = 0;
      double heightBelow = 0;
      //Go over all subcomponents and check if they will fit inside.
      //If component.volume is zero we will create one so sum up needed space
      BOOST_FOREACH(GeoPXDPlacement &p, placements) {
        //Flip placement if component if flipped dimensions
        if (component.flipU) p.u = -p.u;
        if (component.flipV) p.v = -p.v;
        if (component.flipW) p.w = (GeoPXDPlacement::EPosW)(GeoPXDPlacement::c_above - p.w);

        GeoPXDComponent sub = getComponent(p.name);
        B2DEBUG(100, "SubComponent " << p.name);
        B2DEBUG(100, boost::format("Placement: %1%, %2% cm") % p.u % p.v);
        B2DEBUG(100, boost::format("Dimensions: %1%x%2%x%3% cm") % sub.width % sub.length % sub.height);

        if (p.w == GeoPXDPlacement::c_above) {
          //Above placement only valid if we are allowed to create a container around component
          if (!createContainer) B2FATAL("Cannot place component " << p.name << " outside of component " << name);
          heightAbove = max(heightAbove, sub.height);
        } else if (p.w == GeoPXDPlacement::c_below) {
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
        G4Box *componentShape = new G4Box(name.c_str(), component.width / 2.0, component.length / 2.0, component.height / 2.0);
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
        G4Box *containerShape = new G4Box((name + ".Container"), component.width / 2.0, component.length / 2.0, height / 2.0);
        container = new G4LogicalVolume(containerShape, Materials::get("Air"), name + ".Container");
        componentW = -(heightAbove - heightBelow) / 2.0;
        new G4PVPlacement(0, G4ThreeVector(0, 0, componentW), component.volume, name + ".Container", container, false, 1);
      }
      B2DEBUG(100, boost::format("Component %1% dimensions: %2%x%3%x%4% cm") % name % component.width % component.length % component.height);

      //Ok, all volumes set up, now add them together
      for (size_t i = 0; i < placements.size(); ++i) {
        GeoPXDPlacement &p = placements[i];
        GeoPXDComponent &s = subComponents[i];
        G4LogicalVolume* mother(0);
        double w = 0;
        switch (p.w) {
          case GeoPXDPlacement::c_below: //Place below component
            mother = container;
            w = componentW - component.height / 2.0 - s.height / 2.0;
            break;
          case GeoPXDPlacement::c_bottom: //Place inside, at bottom of component
            mother = component.volume;
            w = - component.height / 2.0 + s.height / 2.0;
            break;
          case GeoPXDPlacement::c_center: //Place inside, centered
            mother = component.volume;
            w = 0;
            break;
          case GeoPXDPlacement::c_top:    //Place inside, at top of component
            mother = component.volume;
            w = component.height / 2.0 - s.height / 2.0;
            break;
          case GeoPXDPlacement::c_above:  //Place above component
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

    void GeoPXDCreator::addLadder(int ladderID, double phi, G4LogicalVolume* volume, const G4Transform3D& placement)
    {
      VxdID ladder(m_ladder.layerID, ladderID, 0);

      G4RotationMatrix ladderRotation(0, 0, phi);
      G4Translate3D ladderPos(m_ladder.shift, m_ladder.radius, 0.0);
      G4Transform3D ladderPlacement = placement * G4Rotate3D(ladderRotation) * ladderPos * getAlignment(ladder);

      BOOST_FOREACH(GeoPXDSensor s, m_ladder.sensors) {
        VxdID sensorID(ladder);
        sensorID.setSensor(s.sensorID);
        string name = "PXD." + (string)sensorID;
        G4Box* sensorShape = new G4Box(name, s.width / 2.0, s.length / 2.0, s.height / 2.0);
        s.volume = new G4LogicalVolume(sensorShape, Materials::get(s.material), name);

        // Create sensitive Area: this Part is created separately since we want full control over the coordinate system:
        // local x (called u) should point in RPhi direction
        // local y (called v) should point in global z
        // local z (called w) should away from the origin
        G4Box* activeShape = new G4Box(
          name + ".Active",
          s.info.getWidth() / Unit::mm / 2.0,
          s.info.getLength() / Unit::mm / 2.0,
          s.info.getThickness() / Unit::mm / 2.0
        );

        //Create appropriate sensitive detector instance
        SensorInfo* sensorInfo = new SensorInfo(s.info);
        sensorInfo->setID(sensorID);
        if (s.flipV) sensorInfo->flipVSegmentation();
        SensitiveDetector *sensitive = new SensitiveDetector(sensorInfo);
        m_sensitive.push_back(sensitive);

        G4LogicalVolume* active = new G4LogicalVolume(activeShape,  Materials::get(s.material),
                                                      name + ".Active", 0, sensitive);
        active->SetUserLimits(new G4UserLimits(s.active.stepSize));
        setColor(*active, "#ddd");
        //The coordinates of the active region are given relative to the corner of the sensor, not to the center
        //so we convert them. If the sensor is flipped, the sign of coordinates need to be inverted
        double activeU = (s.active.u + (s.info.getWidth() / Unit::mm - s.width) / 2.0) * (s.flipU ? -1 : 1);
        double activeV = (s.active.v + (s.info.getLength() / Unit::mm - s.length) / 2.0) * (s.flipV ? -1 : 1);
        //active area is always at the top, so active.w would be active.height, otherwise the same as above
        double activeW = (s.height - s.info.getThickness() / Unit::mm) / 2.0 * (s.flipW ? -1 : 1);
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
        double shiftX = s.height / 2.0 * (s.flipW ? -1 : 1) + addSubComponents(name, s, s.components, true, false);
        G4RotationMatrix rotation(0, -M_PI / 2.0, -M_PI / 2.0);
        G4Transform3D sensorAlign = getAlignment(sensorID);
        G4Transform3D placement = ladderPlacement * G4Translate3D(-shiftX, 0.0, s.z) * G4Rotate3D(rotation) * sensorAlign;
        new G4PVPlacement(placement, s.volume, name, volume, false, 1);
      }
    }

    G4AssemblyVolume* GeoPXDCreator::createSupport(GearDir support)
    {
      if (!support) return 0;
      G4AssemblyVolume *supportAssembly = new G4AssemblyVolume();

      BOOST_FOREACH(const GearDir &endflange, support.getNodes("Endflange")) {
        double minZ(0), maxZ(0);
        string name = endflange.getString("@name");
        G4VSolid *supportCone = createPolyCone(name, endflange, minZ, maxZ);

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
        G4Material* material = Materials::get(materialName);
        if (!material) B2FATAL("Material '" << materialName << "', required by PXD component " << name << ", could not be found");

        G4LogicalVolume *volume = new G4LogicalVolume(supportCone, material, name);
        setColor(*volume, endflange.getString("color", "#ccc4"));
        G4Transform3D identity;
        supportAssembly->AddPlacedVolume(volume, identity);
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

        G4Tubs* tube = new G4Tubs("CarbonTube", minR, maxR, sizeZ, 0, 2*M_PI);
        G4LogicalVolume* tubeVol = new G4LogicalVolume(tube, Materials::get(material), "CarbonTube");
        setColor(*tubeVol, "#000");
        for (int i = 0; i < nTubes; ++i) {
          G4Transform3D placement = G4RotateZ3D(phi0 + i * dphi) * G4Translate3D(shiftX, shiftY, shiftZ);
          supportAssembly->AddPlacedVolume(tubeVol, placement);
        }
      }

      return supportAssembly;
    }

    G4Polycone* GeoPXDCreator::createPolyCone(const string& name, GearDir params, double &minZ, double &maxZ)
    {
      if (!params) return 0;

      double minPhi = params.getAngle("minPhi", 0);
      double dPhi   = params.getAngle("maxPhi", 2 * M_PI) - minPhi;
      const std::vector<GearDir> planes = params.getNodes("Plane");
      int nPlanes = planes.size();
      if (nPlanes < 2) {
        B2ERROR("Polycone needs at least two planes");
        return 0;
      }
      double z[nPlanes];
      double rMin[nPlanes];
      double rMax[nPlanes];
      int index(0);
      minZ = numeric_limits<double>::infinity();
      maxZ = -numeric_limits<double>::infinity();
      BOOST_FOREACH(const GearDir &plane, planes) {
        z[index]    = plane.getLength("posZ") / Unit::mm;
        minZ = min(minZ, z[index]);
        maxZ = max(maxZ, z[index]);
        rMin[index] = plane.getLength("innerRadius") / Unit::mm;
        rMax[index] = plane.getLength("outerRadius") / Unit::mm;
        ++index;
      }
      G4Polycone* polycone = new G4Polycone(name, minPhi, dPhi, nPlanes, z, rMin, rMax);
      return polycone;
    }

    void GeoPXDCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {
      m_alignment = GearDir(content, "Alignment/");
      m_components = GearDir(content, "Components/");

      //Build envelope
      G4LogicalVolume* envelope(0);
      GearDir envelopeParams(content, "Envelope");
      if (!envelopeParams) {
        B2FATAL("Could not find definition for PXD Envelope");
      }
      double minZ(0), maxZ(0);
      G4Polycone *envelopeCone = createPolyCone("PXD", GearDir(content, "Envelope/"), minZ, maxZ);
      string materialName = content.getString("Envelope/Material", "Air");
      G4Material* material = Materials::get(materialName);
      if (!material) B2FATAL("Material '" << materialName << "', required by PXD Envelope could not be found");
      envelope = new G4LogicalVolume(envelopeCone, material, "PXD");
      setVisibility(*envelope, false);
      G4Region* pxdRegion = G4RegionStore::GetInstance()->FindOrCreateRegion("PXD");
      envelope->SetRegion(pxdRegion);
      pxdRegion->AddRootLogicalVolume(envelope);
      G4VPhysicalVolume* physEnvelope = new G4PVPlacement(getAlignment("PXD"), envelope, "PXD", &topVolume, false, 1);

      //Build support
      G4AssemblyVolume* support = createSupport(GearDir(content, "Support/"));
      if (!support) {
        B2WARNING("Could not find Definition of PXD mechanical support, continuing without support");
      }

      //Build all ladders including Sensors
      BOOST_FOREACH(const GearDir &shell, content.getNodes("HalfShell")) {
        string shellName =  shell.getString("@name");
        B2INFO("Building PXD half-shell " << shellName);
        G4Transform3D shellAlignment = getAlignment("PXD." + shellName);

        // Add mechanical support if available and defined
        if (support && shell.exists("Support")) {
          double phi = shell.getAngle("Support/phi", 0);
          G4Transform3D placement = shellAlignment * G4RotateZ3D(phi);
          support->MakeImprint(envelope, placement);
        }

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

      //Check hierachy of PXD
      /*VXD::GeoCache &geo = VXD::GeoCache::getInstance();
      BOOST_FOREACH(VxdID layer, geo.getLayers()){
        cout << "Layer " << layer << endl;
        BOOST_FOREACH(VxdID ladder, geo.getLadders(layer)){
          cout << "  Ladder " << ladder << ": ";
          BOOST_FOREACH(VxdID sensor, geo.getSensors(ladder)){
            cout << sensor << " ";
          }
          cout << endl;
        }
      }*/
    }

    void GeoPXDCreator::setLayer(int layer)
    {
      string path = (boost::format("Ladder[@layer=%1%]/") % layer).str();
      GearDir paramsLadder(m_components, path);
      if (!paramsLadder) {
        B2FATAL("Could not find Ladder definition for layer " << layer);
      }

      path = (boost::format("Sensor[@layer=%1%]/") % layer).str();
      GearDir paramsSensor(m_components, path);
      if (!paramsSensor) {
        B2FATAL("Could not find Sensor definition for layer " << layer);
      }

      m_ladder = GeoPXDLadder(
                   layer,
                   paramsLadder.getLength("radius") / Unit::mm,
                   paramsLadder.getLength("shift") / Unit::mm
                 );
      GeoPXDSensor sensor(
        paramsSensor.getString("Material"),
        paramsSensor.getString("Color", ""),
        paramsSensor.getLength("width") / Unit::mm,
        paramsSensor.getLength("length") / Unit::mm,
        paramsSensor.getLength("height") / Unit::mm
      );
      sensor.active = GeoPXDActiveArea(
                        paramsSensor.getLength("Active/u") / Unit::mm,
                        paramsSensor.getLength("Active/v") / Unit::mm,
                        paramsSensor.getLength("Active/stepSize") / Unit::mm
                      );
      sensor.info = SensorInfo(
                      VxdID(layer, 0, 0),
                      paramsSensor.getLength("Active/width"),
                      paramsSensor.getLength("Active/length"),
                      paramsSensor.getLength("Active/height"),
                      paramsSensor.getInt("Active/pixelsR"),
                      paramsSensor.getInt("Active/pixelsZ[1]"),
                      paramsSensor.getLength("Active/splitLength", 0),
                      paramsSensor.getInt("Active/pixelsZ[2]", 0)
                    );

      sensor.components = getSubComponents(paramsSensor);

      BOOST_FOREACH(const GearDir &sensorInfo, paramsLadder.getNodes("Sensor")) {
        sensor.sensorID = sensorInfo.getInt("@id");
        sensor.z =        sensorInfo.getLength(".") / Unit::mm;
        sensor.flipU =    sensorInfo.getBool("@flipU", false);
        sensor.flipV =    sensorInfo.getBool("@flipV", false);
        sensor.flipW =    sensorInfo.getBool("@flipW", false);
        m_ladder.sensors.push_back(sensor);
      }
    }
  }
}
