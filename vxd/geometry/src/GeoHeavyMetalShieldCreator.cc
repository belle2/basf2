/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hyacinth Stypula                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoHeavyMetalShieldCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions/sign.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>

// Shapes
#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VXD */
  namespace VXD {

    /** Register the creator */
    CreatorFactory<GeoHeavyMetalShieldCreator> GeoHeavyMetalShieldFactory("HeavyMetalShieldCreator");

    void GeoHeavyMetalShieldCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {
      for (const GearDir& shield : content.getNodes("Shield")) {
        double minZ(0), maxZ(0);
        string name = shield.getString("@name");
        G4VSolid* geoShield = createPolyCone(name + " IR Shield", shield, minZ, maxZ);

        // Cutouts (if present)
        if (shield.getNodes("Cutout").size() > 0) {
          double sizeX  = shield.getLength("Cutout/width") / Unit::mm / 2.;
          double sizeY  = shield.getLength("Cutout/height") / Unit::mm / 2.;
          double depth2  = shield.getLength("Cutout/depth") / Unit::mm / 2.;
          double sizeZ  = (maxZ - minZ) / 2.;
          double sign = math::sign<double>(minZ);
          double minAbsZ = min(fabs(minZ), fabs(maxZ));

          G4ThreeVector origin1(0, 0, sign * (minAbsZ + sizeZ));
          G4ThreeVector origin2(0, 0, sign * (minAbsZ + depth2));

          G4Box* box1 = new G4Box("Cutout", sizeX, sizeY, sizeZ);
          G4Box* box2 = new G4Box("Cutout", 100 / Unit::mm, sizeY, depth2);

          geoShield = new G4SubtractionSolid(name + " IR Shield", geoShield, box1, G4Translate3D(origin1));
          geoShield = new G4SubtractionSolid(name + " IR Shield", geoShield, box2, G4Translate3D(origin2));
        }

        string materialName = shield.getString("Material", "Air");
        G4Material* material = Materials::get(materialName);
        if (!material) B2FATAL("Material '" << materialName << "', required by " << name << " IR Shield could not be found");

        G4LogicalVolume* volume = new G4LogicalVolume(geoShield, material, name + " IR Shield");
        setColor(*volume, "#cc0000");
        //setVisibility(*volume, false);
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), volume, name + " IR Shield", &topVolume, false, 0);
      }
    }
  }
}
