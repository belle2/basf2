/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoVXDServiceCreator.h>
#include <vxd/geometry/GeoVXDComponents.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <G4Transform3D.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <map>
#include <boost/foreach.hpp>

namespace Belle2 {
  using namespace geometry;
  namespace VXD {

    /** Create factory instance so that the framework can instantiate the VXDServiceCreator */
    geometry::CreatorFactory<GeoVXDServiceCreator> GeoVXDServiceFactory("VXDServiceCreator");


    void GeoVXDServiceCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes)
    {
      m_defaultMaterial = topVolume.GetMaterial();
      createDockBoxes(GearDir(content, "DockBoxes"), topVolume);
    }

    void GeoVXDServiceCreator::createDockBoxes(const GearDir& content, G4LogicalVolume& topVolume)
    {
      //Create the Boxes
      std::map<std::string, GeoVXDComponent> boxes;
      BOOST_FOREACH(const GearDir & boxtype, content.getNodes("BoxType")) {
        GeoVXDComponent box;
        box.width = boxtype.getLength("width") / 2.0 / Unit::mm;
        box.length = boxtype.getLength("length") / 2.0 / Unit::mm;
        box.height = boxtype.getLength("height") / 2.0 / Unit::mm;
        box.material = boxtype.getString("material");
        const std::string name = boxtype.getString("@name");
        G4VSolid* shape_box = new G4Box(name, box.height, box.width, box.length);
        box.volume = new G4LogicalVolume(shape_box, Materials::get(box.material), name);
        B2INFO("Created " << name << " DockBox with a mass of " << (box.volume->GetMass(true) / CLHEP::kg) << "kg");
        boxes[name] = box;
      }

      //Now place the Boxes
      BOOST_FOREACH(const GearDir & position, content.getNodes("Position")) {
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
        BOOST_FOREACH(const GearDir & slots, position.getNodes("slots")) {
          const std::string type = slots.getString("@type");
          GeoVXDComponent box = boxes[type];
          if (!box.volume) B2FATAL("Unknown DockBox type: " << type);
          BOOST_FOREACH(const double slot, slots.getArray("")) {
            const double angle = (2 * M_PI / divisions) * (slot + 0.5);
            //Position boxes as close to beamline as envelope allows
            const double r = minR + box.height;
            //Shift the box as close to the IP as the envelope allows
            const double z = -signZ * ((maxZ - minZ) / 2.0 - box.length);
            const G4Transform3D placement = G4RotateZ3D(angle) * G4Translate3D(r, 0, z);
            new G4PVPlacement(placement, box.volume, name + ".box", envelope, false, static_cast<int>(slot));
          }
        }
      }
    }

  } // VXD namespace
} // Belle2 namespace
