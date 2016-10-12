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


    void GeoVXDServiceCreator::createGeometry(const VXDServiceGeometryPar& parameters, G4LogicalVolume& topVolume, GeometryTypes)
    {

      m_defaultMaterial = topVolume.GetMaterial();
      bool active = parameters.getRecordBackground();

      // Create the Boxes
      const std::vector<VXDBoxTypesPar> BoxTypes = parameters.getBoxTypes();
      std::map<std::string, VXDGeoComponent> boxes;

      for (const VXDBoxTypesPar& boxtype : BoxTypes) {
        VXDGeoComponent box(
          boxtype.getMaterial(), "",
          boxtype.getWidth() / 2.0 / Unit::mm, 0,
          boxtype.getLength() / 2.0 / Unit::mm,
          boxtype.getHeight() / 2.0 / Unit::mm);
        const std::string name = boxtype.getName();
        G4VSolid* shape_box = new G4Box(name, box.getHeight(), box.getWidth(), box.getLength());
        box.setVolume(new G4LogicalVolume(shape_box, Materials::get(box.getMaterial()), name));
        B2INFO("Created " << name << " DockBox with a mass of " << (box.getVolume()->GetMass(true) / CLHEP::kg) << "kg");
        if (active) {
          int identifier = boxtype.getIdentifier();
          B2DEBUG(50, "Creating BkgSensitiveDetector for DockBox " << name << " with identifier " <<  identifier);
          BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(name.c_str(), identifier);
          box.getVolume()->SetSensitiveDetector(sensitive);
        }
        boxes[name] = box;
      }

      // Now place the Boxes
      const std::vector<VXDBoxPositionsPar> Positions = parameters.getPositions();

      for (const VXDBoxPositionsPar& position : Positions) {
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
        const  std::vector<VXDSlotTypesPar> SlotTypes = position.getSlotTypes();
        for (const VXDSlotTypesPar& slots : SlotTypes) {
          const std::string type = slots.getType();
          const VXDGeoComponent& box = boxes[type];
          if (!box.getVolume()) B2FATAL("Unknown DockBox type: " << type);
          for (const double slot : slots.getSlots()) {
            const double angle = (2 * M_PI / divisions) * (slot + 0.5);
            //Position boxes as close to beamline as envelope allows
            const double r = minR + box.getHeight();
            //Shift the box as close to the IP as the envelope allows
            const double z = -signZ * ((maxZ - minZ) / 2.0 - box.getLength());
            const G4Transform3D placement = G4RotateZ3D(angle) * G4Translate3D(r, 0, z);
            new G4PVPlacement(placement, box.getVolume(), name + ".box", envelope, false, static_cast<int>(slot));
          }
        }
      }
    }

    void GeoVXDServiceCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      m_defaultMaterial = topVolume.GetMaterial();
      createDockBoxes(GearDir(content, "DockBoxes"), topVolume);
    }

    void GeoVXDServiceCreator::createDockBoxes(const GearDir& content, G4LogicalVolume& topVolume)
    {
      //Create the Boxes
      std::map<std::string, VXDGeoComponent> boxes;
      bool active = content.getBool("RecordBackground", false);
      for (const GearDir& boxtype : content.getNodes("BoxType")) {
        VXDGeoComponent box(
          boxtype.getString("material"), "",
          boxtype.getLength("width") / 2.0 / Unit::mm, 0,
          boxtype.getLength("length") / 2.0 / Unit::mm,
          boxtype.getLength("height") / 2.0 / Unit::mm);
        const std::string name = boxtype.getString("@name");
        G4VSolid* shape_box = new G4Box(name, box.getHeight(), box.getWidth(), box.getLength());
        box.setVolume(new G4LogicalVolume(shape_box, Materials::get(box.getMaterial()), name));
        B2INFO("Created " << name << " DockBox with a mass of " << (box.getVolume()->GetMass(true) / CLHEP::kg) << "kg");
        if (active) {
          int identifier = boxtype.getInt("@identifier", 0);
          B2DEBUG(50, "Creating BkgSensitiveDetector for DockBox " << name << " with identifier " <<  identifier);
          BkgSensitiveDetector* sensitive = new BkgSensitiveDetector(name.c_str(), identifier);
          box.getVolume()->SetSensitiveDetector(sensitive);
        }
        boxes[name] = box;
      }

      //Now place the Boxes
      for (const GearDir& position : content.getNodes("Position")) {
        const double minZ = position.getLength("minZ") / Unit::mm;
        const double maxZ = position.getLength("maxZ") / Unit::mm;
        const double minR = position.getLength("minR") / Unit::mm;
        const double maxR = position.getLength("maxR") / Unit::mm;
        const int divisions = position.getInt("divisions");
        const std::string name = "VXD.DockBoxes." + position.getString("@name");

        //Boxes are aligned to the side closer to the IP so we need the sign in
        //which to shift
        const int signZ = minZ < 0 ? -1 : +1;

        G4VSolid* shape_envelope = new G4Tubs(name, minR, maxR, (maxZ - minZ) / 2.0, 0, 2 * M_PI);
        G4LogicalVolume* envelope = new G4LogicalVolume(shape_envelope, m_defaultMaterial, name);
        new G4PVPlacement(G4TranslateZ3D((minZ + maxZ) / 2.0), envelope, name, &topVolume, false, 1);
        //We have divisions slots with always to slots joining at phi=0
        //As such, the center phi of one slot is 2pi/divisions*(i+0.5);
        //We start numbering with i=0 as the slot with the smallest phi>0
        for (const GearDir& slots : position.getNodes("slots")) {
          const std::string type = slots.getString("@type");
          const VXDGeoComponent& box = boxes[type];
          if (!box.getVolume()) B2FATAL("Unknown DockBox type: " << type);
          for (const double slot : slots.getArray("")) {
            const double angle = (2 * M_PI / divisions) * (slot + 0.5);
            //Position boxes as close to beamline as envelope allows
            const double r = minR + box.getHeight();
            //Shift the box as close to the IP as the envelope allows
            const double z = -signZ * ((maxZ - minZ) / 2.0 - box.getLength());
            const G4Transform3D placement = G4RotateZ3D(angle) * G4Translate3D(r, 0, z);
            new G4PVPlacement(placement, box.getVolume(), name + ".box", envelope, false, static_cast<int>(slot));
          }
        }
      }
    }

  } // VXD namespace
} // Belle2 namespace
