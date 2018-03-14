/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Benjamin Schwenker                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoVXDServiceCreator.h>
#include <vxd/geometry/GeoVXDComponents.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <G4Transform3D.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <map>

namespace Belle2 {
  using namespace geometry;
  namespace VXD {

    /** Create factory instance so that the framework can instantiate the VXDServiceCreator */
    geometry::CreatorFactory<GeoVXDServiceCreator> GeoVXDServiceFactory("VXDServiceCreator");

    //! Create a parameter object from the Gearbox XML parameters.
    VXDServiceGeometryPar GeoVXDServiceCreator::createConfiguration(const GearDir& content)
    {
      VXDServiceGeometryPar vxdServiceGeometryPar(content.getBool("RecordBackground", false));

      GearDir content2(content, "DockBoxes");

      // Read parameters to creates boxes
      for (const GearDir& boxtype : content2.getNodes("BoxType")) {
        VXDBoxTypesPar boxtypePar(
          boxtype.getString("@name"),
          boxtype.getString("material"),
          boxtype.getInt("@identifier", 0),
          boxtype.getLength("width"),
          boxtype.getLength("length"),
          boxtype.getLength("height")
        );
        vxdServiceGeometryPar.getBoxTypes().push_back(boxtypePar);
      }

      // Read paramater to place boxes
      for (const GearDir& position : content2.getNodes("Position")) {
        VXDBoxPositionsPar positionPar(
          position.getString("@name"),
          position.getLength("minZ"),
          position.getLength("maxZ"),
          position.getLength("minR"),
          position.getLength("maxR"),
          position.getInt("divisions")
        );
        for (const GearDir& slot : position.getNodes("slots")) {
          VXDSlotsPar slotsPar(slot.getString("@type"));
          for (const double number : slot.getArray("")) {
            slotsPar.getSlotNumbers().push_back(number);
          }
          positionPar.getSlots().push_back(slotsPar);
        }
        vxdServiceGeometryPar.getPositions().push_back(positionPar);
      }

      return vxdServiceGeometryPar;
    }

    void GeoVXDServiceCreator::createGeometry(const VXDServiceGeometryPar& parameters, G4LogicalVolume& topVolume, GeometryTypes)
    {
      m_defaultMaterial = topVolume.GetMaterial();
      bool active = parameters.getRecordBackground();

      // Create the Boxes
      std::map<std::string, VXDGeoComponent> boxes;

      for (const VXDBoxTypesPar& boxtype : parameters.getBoxTypes()) {
        VXDGeoComponent box(
          boxtype.getMaterial(), "",
          boxtype.getWidth() / 2.0 / Unit::mm, 0,
          boxtype.getLength() / 2.0 / Unit::mm,
          boxtype.getHeight() / 2.0 / Unit::mm);
        const std::string name = boxtype.getName();
        G4VSolid* shape_box = new G4Box(name, box.getHeight(), box.getWidth(), box.getLength());
        box.setVolume(new G4LogicalVolume(shape_box, Materials::get(box.getMaterial()), name));
        B2DEBUG(50, "Created " << name << " DockBox with a mass of " << (box.getVolume()->GetMass(true) / CLHEP::kg) << "kg");
        if (active) {
          int identifier = boxtype.getIdentifier();
          B2DEBUG(50, "Creating BkgSensitiveDetector for DockBox " << name << " with identifier " <<  identifier);
          BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(name.c_str(), identifier);
          box.getVolume()->SetSensitiveDetector(sensitive);
        }
        boxes[name] = box;
      }

      // Now place the Boxes
      for (const VXDBoxPositionsPar& position : parameters.getPositions()) {
        const double minZ = position.getMinZ() / Unit::mm;
        const double maxZ = position.getMaxZ() / Unit::mm;
        const double minR = position.getMinR() / Unit::mm;
        const double maxR = position.getMaxR() / Unit::mm;
        const int divisions = position.getDivisions();
        const std::string name = "VXD.DockBoxes." + position.getName();

        //Boxes are aligned to the side closer to the IP so we need the sign in
        //which to shift
        const int signZ = minZ < 0 ? -1 : +1;

        G4VSolid* shape_envelope = new G4Tubs(name, minR, maxR, (maxZ - minZ) / 2.0, 0, 2 * M_PI);
        G4LogicalVolume* envelope = new G4LogicalVolume(shape_envelope, m_defaultMaterial, name);
        new G4PVPlacement(G4TranslateZ3D((minZ + maxZ) / 2.0), envelope, name, &topVolume, false, 1);
        //We have divisions slots with always to slots joining at phi=0
        //As such, the center phi of one slot is 2pi/divisions*(i+0.5);
        //We start numbering with i=0 as the slot with the smallest phi>0
        for (const VXDSlotsPar& slot : position.getSlots()) {
          const std::string type = slot.getType();
          const VXDGeoComponent& box = boxes[type];
          if (!box.getVolume()) B2FATAL("Unknown DockBox type: " << type);
          for (double number : slot.getSlotNumbers()) {
            const double angle = (2 * M_PI / divisions) * (number + 0.5);
            //Position boxes as close to beamline as envelope allows
            const double r = minR + box.getHeight();
            //Shift the box as close to the IP as the envelope allows
            const double z = -signZ * ((maxZ - minZ) / 2.0 - box.getLength());
            const G4Transform3D placement = G4RotateZ3D(angle) * G4Translate3D(r, 0, z);
            new G4PVPlacement(placement, box.getVolume(), name + ".box", envelope, false, static_cast<int>(number));
          }
        }
      }
    }
  } // VXD namespace
} // Belle2 namespace
