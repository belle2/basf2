/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vxd/geometry/GeoVXDRadiationSensors.h>
#include <geometry/Materials.h>

#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Box.hh>
#include <G4PVPlacement.hh>
#include <simulation/background/BkgSensitiveDetector.h>

namespace Belle2 {

  void GeoVXDRadiationSensors::create(const GearDir& content, G4LogicalVolume& topVolume, G4LogicalVolume& envelopeVolume)
  {
    //Set the correct top volume to either global top or detector envelope
    G4LogicalVolume* top = &topVolume;
    if (content.getBool("insideEnvelope")) {
      top = &envelopeVolume;
    }

    //shape and material are the same for all sensors so create them now
    const double width = content.getLength("width");
    const double length = content.getLength("length");
    const double height = content.getLength("height");
    G4Box* shape = new G4Box("radiationSensorDiamond", width / 2 * CLHEP::cm, length / 2 * CLHEP::cm, height / 2 * CLHEP::cm);
    G4Material* material = geometry::Materials::get(content.getString("material"));

    //Now loop over all positions
    for (GearDir& position : content.getNodes("position")) {
      //get the radial and z position
      const double r = position.getLength("radius");
      const double z = position.getLength("z");
      const double theta = position.getAngle("theta");
      //and loop over all phi positions
      for (GearDir& sensor : position.getNodes("phi")) {
        //we need angle and Id
        const double phi = sensor.getAngle();
        const int id = sensor.getInt("@id");
        //then we create a nice name
        const std::string name = m_subdetector + ".DiamondSensor." + std::to_string(id);
        //and create the sensor volume
        G4LogicalVolume* volume = new G4LogicalVolume(shape, material, name);
        //add a sensitive detector implementation
        BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(m_subdetector.c_str(), id);
        volume->SetSensitiveDetector(sensitive);
        //and place it at the correct position
        G4Transform3D transform = G4RotateZ3D(phi - M_PI / 2) * G4Translate3D(0, r * CLHEP::cm,
                                  z * CLHEP::cm) * G4RotateX3D(-M_PI / 2 - theta);
        new G4PVPlacement(transform, volume, name, top, false, 1);
      }
    }
  }

} //Belle2 namespace
