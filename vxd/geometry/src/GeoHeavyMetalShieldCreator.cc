/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
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
#include <ir/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>

//Shapes
#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>

using namespace std;
using namespace boost;

namespace Belle2 {


  using namespace geometry;

  namespace ir {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoHeavyMetalShieldCreator> GeoHeavyMetalShieldFactory("HeavyMetalShieldCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoHeavyMetalShieldCreator::GeoHeavyMetalShieldCreator()
    {
    }

    GeoHeavyMetalShieldCreator::~GeoHeavyMetalShieldCreator()
    {
      //Delete all sensitive detectors
      BOOST_FOREACH(SensitiveDetector * sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    void GeoHeavyMetalShieldCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      //---- Box15
      GearDir cBox15(content, "Box15");
      double Box15_Z = cBox15.getLength("Z") / Unit::mm;
      double Box15_X = cBox15.getLength("X") / Unit::mm;
      double Box15_Y = cBox15.getLength("Y") / Unit::mm;
      G4Box* geo_Box15 = new G4Box("geo_Box15_name", Box15_X, Box15_Y, Box15_Z);

      //---- Box40
      GearDir cBox40(content, "Box40");
      double Box40_Z = cBox40.getLength("Z") / Unit::mm;
      double Box40_X = cBox40.getLength("X") / Unit::mm;
      double Box40_Y = cBox40.getLength("Y") / Unit::mm;
      G4Box* geo_Box40 = new G4Box("geo_Box40_name", Box40_X, Box40_Y, Box40_Z);


      //---- Shield L-side part 1
      GearDir cShieldL1(content, "ShieldL1");
      if (! cShieldL1) B2FATAL("Could not find definition for IR ShieldL1");
      double ShieldL1_minZ(0), ShieldL1_maxZ(0);
      G4Polycone* geo_ShieldL1 = geometry::createPolyCone("geo_ShieldL1_name", cShieldL1, ShieldL1_minZ, ShieldL1_maxZ);

      string strMat_ShieldL1 = cShieldL1.getString("Material", "Air");
      G4Material* mat_ShieldL1 = Materials::get(strMat_ShieldL1);
      if (!mat_ShieldL1) B2FATAL("Material '" << strMat_ShieldL1 << "', required by IR ShieldL1 could not be found");

      G4LogicalVolume* logi_ShieldL1 = new G4LogicalVolume(geo_ShieldL1, mat_ShieldL1, "logi_ShieldL1_name");
      setColor(*logi_ShieldL1, "#CC0000");
      //setVisibility(*logi_ShieldL1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldL1, "phys_ShieldL1", &topVolume, false, 0);


      //---- Shield L-side part 2
      GearDir cShieldL2(content, "ShieldL2");
      if (! cShieldL2)  B2FATAL("Could not find definition for IR ShieldL2");
      double ShieldL2_minZ(0), ShieldL2_maxZ(0);
      G4Polycone* geo_ShieldL2x = geometry::createPolyCone("geo_ShieldL2x_name", cShieldL2, ShieldL2_minZ, ShieldL2_maxZ);
      G4SubtractionSolid* geo_ShieldL2 = new G4SubtractionSolid("geo_ShieldL2_name", geo_ShieldL2x, geo_Box15);

      string strMat_ShieldL2 = cShieldL2.getString("Material", "Air");
      G4Material* mat_ShieldL2 = Materials::get(strMat_ShieldL2);
      if (!mat_ShieldL2) B2FATAL("Material '" << strMat_ShieldL2 << "', required by IR ShieldL2 could not be found");

      G4LogicalVolume* logi_ShieldL2 = new G4LogicalVolume(geo_ShieldL2, mat_ShieldL2, "logi_ShieldL2_name");
      setColor(*logi_ShieldL2, "#CC0000");
      //setVisibility(*logi_ShieldL2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldL2, "phys_ShieldL2", &topVolume, false, 0);


      //---- Shield L-side part 3

      GearDir cShieldL3(content, "ShieldL3");
      if (! cShieldL3)  B2FATAL("Could not find definition for IR ShieldL3");
      double ShieldL3_minZ(0), ShieldL3_maxZ(0);
      G4Polycone* geo_ShieldL3x = geometry::createPolyCone("geo_ShieldL3x_name", cShieldL3, ShieldL3_minZ, ShieldL3_maxZ);
      G4SubtractionSolid* geo_ShieldL3 = new G4SubtractionSolid("geo_ShieldL3_name", geo_ShieldL3x, geo_Box15);

      string strMat_ShieldL3 = cShieldL3.getString("Material", "Air");
      G4Material* mat_ShieldL3 = Materials::get(strMat_ShieldL3);
      if (!mat_ShieldL3) B2FATAL("Material '" << strMat_ShieldL3 << "', required by IR ShieldL3 could not be found");

      G4LogicalVolume* logi_ShieldL3 = new G4LogicalVolume(geo_ShieldL3, mat_ShieldL3, "logi_ShieldL3_name");
      setColor(*logi_ShieldL3, "#CC0000");
      //setVisibility(*logi_ShieldL3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldL3, "phys_ShieldL3", &topVolume, false, 0);


      //---- Shield R-side part 1
      GearDir cShieldR1(content, "ShieldR1");
      if (! cShieldR1) B2FATAL("Could not find definition for IR ShieldR1");
      double ShieldR1_minZ(0), ShieldR1_maxZ(0);
      G4Polycone* geo_ShieldR1 = geometry::createPolyCone("geo_ShieldR1_name", cShieldR1, ShieldR1_minZ, ShieldR1_maxZ);

      string strMat_ShieldR1 = cShieldR1.getString("Material", "Air");
      G4Material* mat_ShieldR1 = Materials::get(strMat_ShieldR1);
      if (!mat_ShieldR1) B2FATAL("Material '" << strMat_ShieldR1 << "', required by IR ShieldR1 could not be found");

      G4LogicalVolume* logi_ShieldR1 = new G4LogicalVolume(geo_ShieldR1, mat_ShieldR1, "logi_ShieldR1_name");
      setColor(*logi_ShieldR1, "#CC0000");
      //setVisibility(*logi_ShieldR1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldR1, "phys_ShieldR1", &topVolume, false, 0);


      //---- Shield R-side part 2
      GearDir cShieldR2(content, "ShieldR2");
      if (! cShieldR2) B2FATAL("Could not find definition for IR ShieldR2");
      double ShieldR2_minZ(0), ShieldR2_maxZ(0);
      G4Polycone* geo_ShieldR2x = geometry::createPolyCone("geo_ShieldR2x_name", cShieldR2, ShieldR2_minZ, ShieldR2_maxZ);
      G4SubtractionSolid* geo_ShieldR2 = new G4SubtractionSolid("geo_ShieldR2_name", geo_ShieldR2x, geo_Box15);

      string strMat_ShieldR2 = cShieldR2.getString("Material", "Air");
      G4Material* mat_ShieldR2 = Materials::get(strMat_ShieldR2);
      if (!mat_ShieldR2) B2FATAL("Material '" << strMat_ShieldR2 << "', required by IR ShieldR2 could not be found");

      G4LogicalVolume* logi_ShieldR2 = new G4LogicalVolume(geo_ShieldR2, mat_ShieldR2, "logi_ShieldR2_name");
      setColor(*logi_ShieldR2, "#CC0000");
      //setVisibility(*logi_ShieldR2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldR2, "phys_ShieldR2", &topVolume, false, 0);


      //---- Shield R side part 3
      GearDir cShieldR3(content, "ShieldR3");
      if (! cShieldR3) B2FATAL("Could not find definition for IR ShieldR3");
      double ShieldR3_minZ(0), ShieldR3_maxZ(0);
      G4Polycone* geo_ShieldR3x = geometry::createPolyCone("geo_ShieldR3x_name", cShieldR3, ShieldR3_minZ, ShieldR3_maxZ);
      G4SubtractionSolid* geo_ShieldR3 = new G4SubtractionSolid("geo_ShieldR3_name", geo_ShieldR3x, geo_Box40);

      string strMat_ShieldR3 = cShieldR3.getString("Material", "Air");
      G4Material* mat_ShieldR3 = Materials::get(strMat_ShieldR3);
      if (!mat_ShieldR3) B2FATAL("Material '" << strMat_ShieldR3 << "', required by IR ShieldR3 could not be found");

      G4LogicalVolume* logi_ShieldR3 = new G4LogicalVolume(geo_ShieldR3, mat_ShieldR3, "logi_ShieldR3_name");
      setColor(*logi_ShieldR3, "#CC0000");
      setVisibility(*logi_ShieldR3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldR3, "phys_ShieldR3", &topVolume, false, 0);


    }
  }
}
