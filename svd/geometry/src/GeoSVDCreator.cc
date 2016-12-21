/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter, Hyacinth Stypula                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geometry/GeoSVDCreator.h>
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
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {

    /** Register the creator */
    geometry::CreatorFactory<GeoSVDCreator> GeoSVDFactory("SVDCreator");

    GeoSVDCreator::~GeoSVDCreator()
    {
      for (SensorInfo* sensorInfo : m_SensorInfo) delete sensorInfo;
      m_SensorInfo.clear();
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
      const double unit_pF = 1000 * Unit::fC / Unit::V; // picofarad
      info->setSensorParams(
        sensor.getWithUnit("DepletionVoltage"),
        sensor.getWithUnit("BiasVoltage"),
        sensor.getDouble("BackplaneCapacitanceU") * unit_pF,
        sensor.getDouble("InterstripCapacitanceU") * unit_pF,
        sensor.getDouble("CouplingCapacitanceU") * unit_pF,
        sensor.getDouble("BackplaneCapacitanceV") * unit_pF,
        sensor.getDouble("InterstripCapacitanceV") * unit_pF,
        sensor.getDouble("CouplingCapacitanceV") * unit_pF,
        sensor.getWithUnit("ElectronicNoiseU"),
        sensor.getWithUnit("ElectronicNoiseV")
      );

      m_SensorInfo.push_back(info);
      return info;
    }

    VXD::SensitiveDetectorBase* GeoSVDCreator::createSensitiveDetector(
      VxdID sensorID, const VXD::GeoVXDSensor& sensor, const VXD::GeoVXDSensorPlacement&)
    {
      SensorInfo* sensorInfo = new SensorInfo(dynamic_cast<SensorInfo&>(*sensor.info));
      sensorInfo->setID(sensorID);
      SensitiveDetector* sensitive = new SensitiveDetector(sensorInfo);
      return sensitive;
    }

    VXD::GeoVXDAssembly GeoSVDCreator::createHalfShellSupport(GearDir support)
    {
      VXD::GeoVXDAssembly supportAssembly;

      //Half shell support is easy as we just add all the defined RotationSolids from the xml file
      double minZ(0), maxZ(0);
      BOOST_FOREACH(const GearDir & component, support.getNodes("HalfShell/RotationSolid")) {
        string name = component.getString("Name");
        string material = component.getString("Material");

        G4Polycone* solid  = geometry::createRotationSolid(name, component, minZ, maxZ);
        G4LogicalVolume* volume = new G4LogicalVolume(
          solid, geometry::Materials::get(material), m_prefix + ". " + name);
        geometry::setColor(*volume, component.getString("Color"));
        supportAssembly.add(volume);
      }

      return supportAssembly;
    }

    VXD::GeoVXDAssembly GeoSVDCreator::createLayerSupport(int layer, GearDir support)
    {
      VXD::GeoVXDAssembly supportAssembly;
      //Check if there are any endrings defined for this layer. If not we don't create any
      GearDir endrings(support, (boost::format("Endrings/Layer[@id='%1%']") % layer).str());
      if (endrings) {
        string material      = support.getString("Endrings/Material");
        double length        = support.getLength("Endrings/length") / Unit::mm / 2.0;
        double gapWidth      = support.getLength("Endrings/gapWidth") / Unit::mm;
        double baseThickness = support.getLength("Endrings/baseThickness") / Unit::mm / 2.0;

        //Create  the endrings
        BOOST_FOREACH(const GearDir & endring, endrings.getNodes("Endring")) {
          double z             = endring.getLength("z") / Unit::mm;
          double baseRadius    = endring.getLength("baseRadius") / Unit::mm;
          double innerRadius   = endring.getLength("innerRadius") / Unit::mm;
          double outerRadius   = endring.getLength("outerRadius") / Unit::mm;
          double horiBarWidth  = endring.getLength("horizontalBar") / Unit::mm / 2.0;
          double vertBarWidth  = endring.getLength("verticalBar") / Unit::mm / 2.0;

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
            (boost::format("%1%.Layer%2%.%3%") % m_prefix % layer % endring.getString("@name")).str());
          supportAssembly.add(endringVolume, G4TranslateZ3D(z));
        }
      }

      // Now let's add the cooling pipes to the Support
      GearDir pipes(support, (boost::format("CoolingPipes/Layer[@id='%1%']") % layer).str());
      if (pipes) {
        string material    = support.getString("CoolingPipes/Material");
        double outerRadius = support.getLength("CoolingPipes/outerDiameter") / Unit::mm / 2.0;
        double innerRadius = outerRadius - support.getLength("CoolingPipes/wallThickness") / Unit::mm;
        int    nPipes      = pipes.getInt("nPipes");
        double startPhi    = pipes.getAngle("startPhi");
        double deltaPhi    = pipes.getAngle("deltaPhi");
        double radius      = pipes.getLength("radius") / Unit::mm;
        double zstart      = pipes.getLength("zstart") / Unit::mm;
        double zend        = pipes.getLength("zend") / Unit::mm;
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
        if (pipes.exists("deltaL")) {
          double deltaL = pipes.getLength("deltaL") / Unit::mm;
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

    VXD::GeoVXDAssembly GeoSVDCreator::createLadderSupport(int layer, GearDir support)
    {
      VXD::GeoVXDAssembly supportAssembly;
      // Check if there are any support ribs defined for this layer. If not return empty assembly
      GearDir params(support, (boost::format("SupportRibs/Layer[@id='%1%']") % layer).str());
      if (!params) return supportAssembly;

      // Get the common values for all layers
      double spacing    = support.getLength("SupportRibs/spacing") / Unit::mm / 2.0;
      double height     = support.getLength("SupportRibs/height") / Unit::mm / 2.0;
      double innerWidth = support.getLength("SupportRibs/inner/width") / Unit::mm / 2.0;
      double outerWidth = support.getLength("SupportRibs/outer/width") / Unit::mm / 2.0;
      double tabLength  = support.getLength("SupportRibs/inner/tabLength") / Unit::mm / 2.0;
      G4VSolid* inner(0);
      G4VSolid* outer(0);
      G4Transform3D placement;

      // Get values for the layer if available
      if (params.exists("spacing")) spacing = params.getLength("spacing") / Unit::mm / 2.0;
      if (params.exists("height")) height = params.getLength("height") / Unit::mm / 2.0;

      // Now lets create the ribs by adding all boxes to form one union solid
      BOOST_FOREACH(const GearDir & box, params.getNodes("box")) {
        double theta = box.getAngle("theta");
        double zpos = box.getLength("z") / Unit::mm;
        double rpos = box.getLength("r") / Unit::mm;
        double length = box.getLength("length") / Unit::mm / 2.0;
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
      BOOST_FOREACH(const GearDir & tab, params.getNodes("tab")) {
        double theta = tab.getAngle("theta");
        double zpos = tab.getLength("z") / Unit::mm;
        double rpos = tab.getLength("r") / Unit::mm;
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
      BOOST_FOREACH(const GearDir & endmount, params.getNodes("Endmount")) {
        double height = endmount.getLength("height") / Unit::mm / 2.0;
        double width = endmount.getLength("width") / Unit::mm / 2.0;
        double length = endmount.getLength("length") / Unit::mm / 2.0;
        double zpos = endmount.getLength("z") / Unit::mm;
        double rpos = endmount.getLength("r") / Unit::mm;
        G4VSolid* endmountBox = new G4Box("endmountBox", height, width, length);
        if (outer) { // holes for the ribs
          endmountBox = new G4SubtractionSolid("endmountBox", endmountBox, outer, G4TranslateY3D(-spacing)*placement * G4Translate3D(-rpos, 0,
                                               -zpos));
          endmountBox = new G4SubtractionSolid("endmountBox", endmountBox, outer, G4TranslateY3D(spacing)*placement * G4Translate3D(-rpos, 0,
                                               -zpos));
        }
        G4LogicalVolume* endmountVolume = new G4LogicalVolume(
          endmountBox, geometry::Materials::get(support.getString("SupportRibs/endmount/Material")),
          (boost::format("%1%.Layer%2%.%3%Endmount") % m_prefix % layer % endmount.getString("@name")).str());
        supportAssembly.add(endmountVolume, G4Translate3D(rpos, 0, zpos));
      }

      // If there has been at least one Box, create the volumes and add them to the assembly
      if (inner) {
        outer = new G4SubtractionSolid("outerBox", outer, inner);
        G4LogicalVolume* outerVolume = new G4LogicalVolume(
          outer, geometry::Materials::get(support.getString("SupportRibs/outer/Material")),
          (boost::format("%1%.Layer%2%.SupportRib") % m_prefix % layer).str());
        G4LogicalVolume* innerVolume = new G4LogicalVolume(
          inner, geometry::Materials::get(support.getString("SupportRibs/inner/Material")),
          (boost::format("%1%.Layer%2%.SupportRib.Airex") % m_prefix % layer).str());
        geometry::setColor(*outerVolume, support.getString("SupportRibs/outer/Color"));
        geometry::setColor(*innerVolume, support.getString("SupportRibs/inner/Color"));
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
