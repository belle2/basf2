/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    HeavyMetalShieldGeometryPar GeoHeavyMetalShieldCreator::createConfiguration(const GearDir& param)
    {
      HeavyMetalShieldGeometryPar heavyMetalShieldGeometryPar;
      //Read the definition of all shields
      for (const GearDir& shield : param.getNodes("Shield")) {
        VXDPolyConePar shieldPar(
          shield.getString("@name"),
          shield.getString("Material", "Air"),
          shield.getAngle("minPhi", 0),
          shield.getAngle("maxPhi", 2 * M_PI),
          (shield.getNodes("Cutout").size() > 0),
          shield.getLength("Cutout/width", 0.),
          shield.getLength("Cutout/height", 0.),
          shield.getLength("Cutout/depth", 0.)
        );

        for (const GearDir& plane : shield.getNodes("Plane")) {
          VXDPolyConePlanePar planePar(
            plane.getLength("posZ"),
            plane.getLength("innerRadius"),
            plane.getLength("outerRadius")
          );
          shieldPar.getPlanes().push_back(planePar);
        }

        heavyMetalShieldGeometryPar.getShields().push_back(shieldPar);
      }
      return heavyMetalShieldGeometryPar;
    }

    void GeoHeavyMetalShieldCreator::createGeometry(const HeavyMetalShieldGeometryPar& parameters, G4LogicalVolume& topVolume,
                                                    GeometryTypes)
    {

      // Create the shields
      for (const VXDPolyConePar& shield : parameters.getShields()) {

        string name = shield.getName();
        double minZ(0), maxZ(0);

        // Create a polycone
        double minPhi = shield.getMinPhi();
        double dPhi   = shield.getMaxPhi() - minPhi;
        int nPlanes = shield.getPlanes().size();
        if (nPlanes < 2) {
          B2ERROR("Polycone needs at least two planes");
          return ;
        }
        std::vector<double> z(nPlanes, 0);
        std::vector<double> rMin(nPlanes, 0);
        std::vector<double> rMax(nPlanes, 0);
        int index(0);
        minZ = numeric_limits<double>::infinity();
        maxZ = -numeric_limits<double>::infinity();


        for (const VXDPolyConePlanePar& plane : shield.getPlanes()) {
          z[index]    = plane.getPosZ() / Unit::mm;
          minZ = min(minZ, z[index]);
          maxZ = max(maxZ, z[index]);
          rMin[index] = plane.getInnerRadius() / Unit::mm;
          rMax[index] = plane.getOuterRadius() / Unit::mm;
          ++index;
        }

        G4VSolid* geoShield = new G4Polycone(name + " IR Shield", minPhi, dPhi, nPlanes, z.data(), rMin.data(), rMax.data());

        // Cutouts (if present)
        if (shield.getDoCutOut()) {
          double sizeX  = shield.getCutOutWidth() / Unit::mm / 2.;
          double sizeY  = shield.getCutOutHeight() / Unit::mm / 2.;
          double depth2 = shield.getCutOutDepth() / Unit::mm / 2.;
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

        string materialName = shield.getMaterial();
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
