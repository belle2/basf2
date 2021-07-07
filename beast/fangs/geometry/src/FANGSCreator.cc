/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/fangs/geometry/FANGSCreator.h>
#include <beast/fangs/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4UserLimits.hh>

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CLAW detector */
  namespace fangs {

    // Register the creator
    /** Creator creates the CLAW geometry */
    geometry::CreatorFactory<FANGSCreator> FANGSFactory("FANGSCreator");

    FANGSCreator::FANGSCreator(Simulation::SensitiveDetectorBase* sensitive): m_sensitive(sensitive)
    {
      /*if (!m_sensitive) {
        m_sensitive = new SensitiveDetector();
      }*/
    }

    FANGSCreator::~FANGSCreator()
    {
      delete m_sensitive;
      delete m_stepLength;
    }

    void FANGSCreator::createShape(const std::string& prefix, const GearDir& params, G4LogicalVolume* parent, double roffset,
                                   bool check)
    {

      if (!m_sensitive) {
        m_sensitive = new SensitiveDetector();
      }

      std::string name = params.getString("@name");
      if (!prefix.empty()) {
        name = prefix + "." + name;
      }
      const std::string type = params.getString("@type");
      const std::string material = params.getString("material", "");
      const double r = params.getLength("r", 0) / Unit::mm * CLHEP::mm;
      const double top = params.getLength("top", 0) / Unit::mm * CLHEP::mm;
      const double u = params.getLength("u", 0) / Unit::mm * CLHEP::mm;
      const bool active = params.getBool("active", false);

      G4Material* mat = m_topMaterial;
      if (!material.empty()) {
        mat = geometry::Materials::get(material);
      }
      G4VSolid* shape{nullptr};
      double height{0};
      if (type == "box") {
        const double length = params.getLength("length") / Unit::mm * CLHEP::mm;
        const double width = params.getLength("width") / Unit::mm * CLHEP::mm;
        height = params.getLength("height") / Unit::mm * CLHEP::mm;
        shape = new G4Box(name, width / 2, height / 2, length / 2);
      } else if (type == "tube") {
        const double length = params.getLength("length") / Unit::mm * CLHEP::mm;
        height = params.getLength("diameter") / Unit::mm * CLHEP::mm;
        shape = new G4Tubs(name, 0, height / 2, length / 2, 0, 2 * M_PI);
      }
      G4LogicalVolume* volume = new G4LogicalVolume(shape, mat, name);
      if (active) {
        volume->SetSensitiveDetector(m_sensitive);
        if (m_stepLength) {
          volume->SetUserLimits(m_stepLength);
        }
      }
      for (const GearDir& child : params.getNodes("shape")) {
        createShape(name, child, volume, height / 2, true);
      }

      int copyNo = 1;
      const double center = r + roffset - height / 2 - top;
      for (double phi : params.getArray("phi", {M_PI / 2})) {
        for (double z : params.getArray("z", {0})) {
          z *= CLHEP::mm / Unit::mm;
          G4Transform3D transform = G4RotateZ3D(phi - M_PI / 2) * G4Translate3D(u, center, z);
          new G4PVPlacement(transform, volume, name, parent, false, copyNo++, check);
        }
      }
    }

    void FANGSCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
      m_topMaterial = topVolume.GetMaterial();
      double stepLength = content.getLength("stepLength", 0) / Unit::mm * CLHEP::mm;
      if (stepLength > 0) {
        if (m_stepLength) delete m_stepLength;
        m_stepLength = new G4UserLimits(stepLength);
      }
      for (auto shape : content.getNodes("shape")) {
        createShape("", shape, &topVolume, 0, false);
      }
    }
  } // fangs namespace
} // Belle2 namespace
