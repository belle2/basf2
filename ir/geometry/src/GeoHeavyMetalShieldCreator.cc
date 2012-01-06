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


#include <ir/geometry/GeoHeavyMetalShieldCreator.h>

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
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
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

    void GeoHeavyMetalShieldCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

      //----
      // Trapezoid L-side
      GearDir cTrdL(content, "TrdL");
      double TrdL_Z1 = cTrdL.getLength("Z1") / Unit::mm;
      double TrdL_Z2 = cTrdL.getLength("Z2") / Unit::mm;
      double TrdL_X1 = cTrdL.getLength("X1") / Unit::mm;
      double TrdL_X2 = cTrdL.getLength("X2") / Unit::mm;
      double TrdL_Y = cTrdL.getLength("Y") / Unit::mm;
      G4Trd* geo_TrdL = new G4Trd("geo_TrdL_name", TrdL_X1, TrdL_X2, TrdL_Y, TrdL_Y, TMath::Abs(TrdL_Z2 - TrdL_Z1) / 2.);
      G4Transform3D transform_TrdL = G4Translate3D(0., 0., (TrdL_Z2 + TrdL_Z1) / 2.);

      //----
      // Trapezoid R-side
      GearDir cTrdR(content, "TrdR");
      double TrdR_Z1 = cTrdR.getLength("Z1") / Unit::mm;
      double TrdR_Z2 = cTrdR.getLength("Z2") / Unit::mm;
      double TrdR_X1 = cTrdR.getLength("X1") / Unit::mm;
      double TrdR_X2 = cTrdR.getLength("X2") / Unit::mm;
      double TrdR_Y = cTrdR.getLength("Y") / Unit::mm;
      G4Trd* geo_TrdR = new G4Trd("geo_TrdR_name", TrdR_X1, TrdR_X2, TrdR_Y, TrdR_Y, TMath::Abs(TrdR_Z2 - TrdR_Z1) / 2.);
      G4Transform3D transform_TrdR = G4Translate3D(0., 0., (TrdR_Z2 + TrdR_Z1) / 2.);


      //----
      // Shield L-side part 1

      GearDir cShieldL1(content, "ShieldL1");
      if (! cShieldL1) B2FATAL("Could not find definition for IR ShieldL1");
      double ShieldL1_minZ(0), ShieldL1_maxZ(0);
      G4Polycone* geo_ShieldL1x = geometry::createPolyCone("geo_ShieldL1x_name", cShieldL1, ShieldL1_minZ, ShieldL1_maxZ);
      G4SubtractionSolid* geo_ShieldL1 = new G4SubtractionSolid("geo_ShieldL1_name", geo_ShieldL1x, geo_TrdL, transform_TrdL);

      string strMat_ShieldL1 = cShieldL1.getString("Material", "Air");
      G4Material* mat_ShieldL1 = Materials::get(strMat_ShieldL1);
      if (!mat_ShieldL1) B2FATAL("Material '" << strMat_ShieldL1 << "', required by IR ShieldL1 could not be found");

      G4LogicalVolume* logi_ShieldL1 = new G4LogicalVolume(geo_ShieldL1, mat_ShieldL1, "logi_ShieldL1_name");
      setColor(*logi_ShieldL1, "#CC0000");
      //setVisibility(*logi_ShieldL1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldL1, "phys_ShieldL1", &topVolume, false, 0);

      //----
      // Shield L-side part 2

      GearDir cShieldL2(content, "ShieldL2");
      if (! cShieldL2)  B2FATAL("Could not find definition for IR ShieldL2");
      double ShieldL2_minZ(0), ShieldL2_maxZ(0);
      G4Polycone* geo_ShieldL2x = geometry::createPolyCone("geo_ShieldL2x_name", cShieldL2, ShieldL2_minZ, ShieldL2_maxZ);
      G4SubtractionSolid* geo_ShieldL2 = new G4SubtractionSolid("geo_ShieldL2_name", geo_ShieldL2x, geo_TrdL, transform_TrdL);

      string strMat_ShieldL2 = cShieldL2.getString("Material", "Air");
      G4Material* mat_ShieldL2 = Materials::get(strMat_ShieldL2);
      if (!mat_ShieldL2) B2FATAL("Material '" << strMat_ShieldL2 << "', required by IR ShieldL2 could not be found");

      G4LogicalVolume* logi_ShieldL2 = new G4LogicalVolume(geo_ShieldL2, mat_ShieldL2, "logi_ShieldL2_name");
      setColor(*logi_ShieldL2, "#CC0000");
      //setVisibility(*logi_ShieldL2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldL2, "phys_ShieldL2", &topVolume, false, 0);


      //-- Shield R side part 1
      GearDir cShieldR1(content, "ShieldR1");
      if (! cShieldR1) B2FATAL("Could not find definition for IR ShieldR1");
      double ShieldR1_minZ(0), ShieldR1_maxZ(0);
      G4Polycone* geo_ShieldR1x = geometry::createPolyCone("geo_ShieldR1x_name", cShieldR1, ShieldR1_minZ, ShieldR1_maxZ);
      G4SubtractionSolid* geo_ShieldR1 = new G4SubtractionSolid("geo_ShieldR1_name", geo_ShieldR1x, geo_TrdR, transform_TrdR);

      string strMat_ShieldR1 = cShieldR1.getString("Material", "Air");
      G4Material* mat_ShieldR1 = Materials::get(strMat_ShieldR1);
      if (!mat_ShieldR1) B2FATAL("Material '" << strMat_ShieldR1 << "', required by IR ShieldR1 could not be found");

      G4LogicalVolume* logi_ShieldR1 = new G4LogicalVolume(geo_ShieldR1, mat_ShieldR1, "logi_ShieldR1_name");
      setColor(*logi_ShieldR1, "#CC0000");
      //setVisibility(*logi_ShieldR1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldR1, "phys_ShieldR1", &topVolume, false, 0);


      //-- Shield R side part 2
      GearDir cShieldR2(content, "ShieldR2");
      if (! cShieldR2) B2FATAL("Could not find definition for IR ShieldR2");
      double ShieldR2_minZ(0), ShieldR2_maxZ(0);
      G4Polycone* geo_ShieldR2x = geometry::createPolyCone("geo_ShieldR2x_name", cShieldR2, ShieldR2_minZ, ShieldR2_maxZ);
      G4SubtractionSolid* geo_ShieldR2 = new G4SubtractionSolid("geo_ShieldR2_name", geo_ShieldR2x, geo_TrdR, transform_TrdR);

      string strMat_ShieldR2 = cShieldR2.getString("Material", "Air");
      G4Material* mat_ShieldR2 = Materials::get(strMat_ShieldR2);
      if (!mat_ShieldR2) B2FATAL("Material '" << strMat_ShieldR2 << "', required by IR ShieldR2 could not be found");

      G4LogicalVolume* logi_ShieldR2 = new G4LogicalVolume(geo_ShieldR2, mat_ShieldR2, "logi_ShieldR2_name");
      setColor(*logi_ShieldR2, "#CC0000");
      //setVisibility(*logi_ShieldR2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_ShieldR2, "phys_ShieldR2", &topVolume, false, 0);


      /*
      //##############
      //#   SVD Heavy metal shield

      //--------------
      //=   R side (Lv1RShield1)

      //get parameters from .xml file
      GearDir cLv1RShield1(content, "Lv1RShield1/");
      //
      double Lv1RShield1_D1 = cLv1RShield1.getLength("D1") / Unit::mm; //from IP
      //
      double Lv1RShield1_L1 = cLv1RShield1.getLength("L1") / Unit::mm;
      double Lv1RShield1_L2 = cLv1RShield1.getLength("L2") / Unit::mm;
      //
      const int Lv1RShield1_num = 3;
      //
      double Lv1RShield1_I[Lv1RShield1_num];
      Lv1RShield1_I[0] = cLv1RShield1.getLength("I1") / Unit::mm;
      Lv1RShield1_I[1] = cLv1RShield1.getLength("I2") / Unit::mm;
      Lv1RShield1_I[2] = cLv1RShield1.getLength("I2") / Unit::mm;
      //
      double Lv1RShield1_O[Lv1RShield1_num];
      Lv1RShield1_O[0] = cLv1RShield1.getLength("O1") / Unit::mm;
      Lv1RShield1_O[1] = cLv1RShield1.getLength("O2") / Unit::mm;
      Lv1RShield1_O[2] = cLv1RShield1.getLength("O2") / Unit::mm;
      //
      double Lv1RShield1_Z[Lv1RShield1_num];
      Lv1RShield1_Z[0] = Lv1RShield1_D1;
      Lv1RShield1_Z[1] = Lv1RShield1_Z[0] + Lv1RShield1_L1;
      Lv1RShield1_Z[2] = Lv1RShield1_Z[1] + Lv1RShield1_L2;
      //
      string strMat_Lv1RShield1 = cLv1RShield1.getString("Material");
      G4Material* mat_Lv1RShield1 = Materials::get(strMat_Lv1RShield1);

      //define geometry
      G4Polycone* geo_Lv1RShield1pcon = new G4Polycone("geo_Lv1RShield1pcon_name", 0, 2*M_PI, Lv1RShield1_num, Lv1RShield1_Z, Lv1RShield1_I, Lv1RShield1_O);
      SensitiveDetector *sensitive = new SensitiveDetector();
      m_sensitive.push_back(sensitive);
      G4LogicalVolume *logi_Lv1RShield1 = new G4LogicalVolume(geo_Lv1RShield1pcon, mat_Lv1RShield1, "logi_Lv1RShield1_name", 0, sensitive);

      //for (int i=0;i<Lv1RShield1_num;i++) printf("%f %f\n",Lv1RShield1_Z[i],Lv1RShield1_O[i]);

      //-   put volume
      setColor(*logi_Lv1RShield1, cLv1RShield1.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv1RShield1, "phys_Lv1RShield1_name", &topVolume, false, 0);
      //-
      //--------------

      //--------------
      //=   R side (Lv1RShield2)

      //get parameters from .xml file
      GearDir cLv1RShield2(content, "Lv1RShield2/");
      //
      double Lv1RShield2_D1 = cLv1RShield2.getLength("D1") / Unit::mm; //from IP
      //
      double Lv1RShield2_L1 = cLv1RShield2.getLength("L1") / Unit::mm;
      double Lv1RShield2_L2 = cLv1RShield2.getLength("L2") / Unit::mm;
      double Lv1RShield2_L3 = cLv1RShield2.getLength("L3") / Unit::mm;
      double Lv1RShield2_L4 = cLv1RShield2.getLength("L4") / Unit::mm;
      double Lv1RShield2_L5 = cLv1RShield2.getLength("L5") / Unit::mm;
      //
      const int Lv1RShield2_num = 8;
      //
      double Lv1RShield2_I[Lv1RShield2_num];
      Lv1RShield2_I[0] = cLv1RShield2.getLength("I1") / Unit::mm;
      Lv1RShield2_I[1] = Lv1RShield2_I[0];
      Lv1RShield2_I[2] = Lv1RShield2_I[1];
      Lv1RShield2_I[3] = Lv1RShield2_I[2];
      Lv1RShield2_I[4] = cLv1RShield2.getLength("I2") / Unit::mm;
      Lv1RShield2_I[5] = Lv1RShield2_I[4];
      Lv1RShield2_I[6] = Lv1RShield2_I[5];
      Lv1RShield2_I[7] = Lv1RShield2_I[6];
      //
      double Lv1RShield2_O[Lv1RShield2_num];
      Lv1RShield2_O[0] = cLv1RShield2.getLength("O1") / Unit::mm;
      Lv1RShield2_O[1] = Lv1RShield2_O[0];
      Lv1RShield2_O[2] = cLv1RShield2.getLength("O2") / Unit::mm;
      Lv1RShield2_O[3] = Lv1RShield2_O[2];
      Lv1RShield2_O[4] = cLv1RShield2.getLength("O3") / Unit::mm;
      Lv1RShield2_O[5] = Lv1RShield2_O[4];
      Lv1RShield2_O[6] = cLv1RShield2.getLength("O4") / Unit::mm;
      Lv1RShield2_O[7] = Lv1RShield2_O[6];
      //
      double Lv1RShield2_Z[Lv1RShield2_num];
      Lv1RShield2_Z[0] = Lv1RShield2_D1;
      Lv1RShield2_Z[1] = Lv1RShield2_Z[0] + Lv1RShield2_L1;
      Lv1RShield2_Z[2] = Lv1RShield2_Z[1];
      Lv1RShield2_Z[3] = Lv1RShield2_Z[2] + Lv1RShield2_L2;
      Lv1RShield2_Z[4] = Lv1RShield2_Z[3] + Lv1RShield2_L3;
      Lv1RShield2_Z[5] = Lv1RShield2_Z[4] + Lv1RShield2_L4;
      Lv1RShield2_Z[6] = Lv1RShield2_Z[5];
      Lv1RShield2_Z[7] = Lv1RShield2_Z[6] + Lv1RShield2_L5;
      //
      string strMat_Lv1RShield2 = cLv1RShield2.getString("Material");
      G4Material* mat_Lv1RShield2 = Materials::get(strMat_Lv1RShield2);

      //for (int i=0;i<Lv1RShield2_num;i++) printf("%f %f\n",Lv1RShield2_Z[i],Lv1RShield2_O[i]);

      //define geometry
      G4Polycone* geo_Lv1RShield2pcon = new G4Polycone("geo_Lv1RShield2pcon_name", 0, 2*M_PI, Lv1RShield2_num, Lv1RShield2_Z, Lv1RShield2_I, Lv1RShield2_O);
      G4LogicalVolume *logi_Lv1RShield2 = new G4LogicalVolume(geo_Lv1RShield2pcon, mat_Lv1RShield2, "logi_Lv1RShield2_name");

      //-   put volume
      setColor(*logi_Lv1RShield2, cLv1RShield2.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv1RShield2, "phys_Lv1RShield2_name", &topVolume, false, 0);
      //-
      //--------------

      //--------------
      //=   R side (Lv1RFlange1)

      //get parameters from .xml file
      GearDir cLv1RFlange1(content, "Lv1RFlange1/");
      //
      double Lv1RFlange1_D1 = cLv1RFlange1.getLength("D1") / Unit::mm;
      double Lv1RFlange1_L1 = cLv1RFlange1.getLength("L1") / Unit::mm;
      double Lv1RFlange1_I1 = cLv1RFlange1.getLength("I1") / Unit::mm;
      double Lv1RFlange1_O1 = cLv1RFlange1.getLength("O1") / Unit::mm;
      //
      string strMat_Lv1RFlange1 = cLv1RFlange1.getString("Material");
      G4Material* mat_Lv1RFlange1 = Materials::get(strMat_Lv1RFlange1);

      //define geometry
      G4Tubs* geo_Lv1RFlange1tubs = new G4Tubs("geo_Lv1RFlange1tubs_name", Lv1RFlange1_I1, Lv1RFlange1_O1, Lv1RFlange1_L1 / 2., 0, 2*M_PI);
      G4LogicalVolume *logi_Lv1RFlange1 = new G4LogicalVolume(geo_Lv1RFlange1tubs, mat_Lv1RFlange1, "logi_Lv1RFlange1_name");

      //-   put volume
      setColor(*logi_Lv1RFlange1, cLv1RFlange1.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, Lv1RFlange1_D1 + Lv1RFlange1_L1 / 2.), logi_Lv1RFlange1, "phys_Lv1RFlange1_name", &topVolume, false, 0);
      //-
      //--------------

      //--------------
      //=   L side (Lv1LShield1)

      //get parameters from .xml file
      GearDir cLv1LShield1(content, "Lv1LShield1/");
      //
      double Lv1LShield1_D1 = cLv1LShield1.getLength("D1") / Unit::mm;
      //
      double Lv1LShield1_L1 = cLv1LShield1.getLength("L1") / Unit::mm;
      double Lv1LShield1_L2 = cLv1LShield1.getLength("L2") / Unit::mm;
      double Lv1LShield1_L3 = cLv1LShield1.getLength("L3") / Unit::mm;
      double Lv1LShield1_L4 = cLv1LShield1.getLength("L4") / Unit::mm;
      //
      const int Lv1LShield1_num = 7;
      //
      double Lv1LShield1_I[Lv1LShield1_num];
      Lv1LShield1_I[0] = cLv1LShield1.getLength("I1") / Unit::mm;
      Lv1LShield1_I[1] = cLv1LShield1.getLength("I2") / Unit::mm;
      Lv1LShield1_I[2] = Lv1LShield1_I[1];
      Lv1LShield1_I[3] = Lv1LShield1_I[2];
      Lv1LShield1_I[4] = Lv1LShield1_I[3];
      Lv1LShield1_I[5] = cLv1LShield1.getLength("I3") / Unit::mm;
      Lv1LShield1_I[6] = Lv1LShield1_I[5];
      //
      double Lv1LShield1_O[Lv1LShield1_num];
      Lv1LShield1_O[0] = cLv1LShield1.getLength("O1") / Unit::mm;
      Lv1LShield1_O[1] = cLv1LShield1.getLength("O2") / Unit::mm;
      Lv1LShield1_O[2] = Lv1LShield1_O[1];
      Lv1LShield1_O[3] = cLv1LShield1.getLength("O3") / Unit::mm;
      Lv1LShield1_O[4] = Lv1LShield1_O[3];
      Lv1LShield1_O[5] = Lv1LShield1_O[4];
      Lv1LShield1_O[6] = Lv1LShield1_O[5];
      //
      double Lv1LShield1_Z[Lv1LShield1_num];
      Lv1LShield1_Z[0] = -Lv1LShield1_D1;
      Lv1LShield1_Z[1] = Lv1LShield1_Z[0] - Lv1LShield1_L1;
      Lv1LShield1_Z[2] = Lv1LShield1_Z[1] - Lv1LShield1_L2;
      Lv1LShield1_Z[3] = Lv1LShield1_Z[2];
      Lv1LShield1_Z[4] = Lv1LShield1_Z[3] - Lv1LShield1_L3;
      Lv1LShield1_Z[5] = Lv1LShield1_Z[4];
      Lv1LShield1_Z[6] = Lv1LShield1_Z[5] - Lv1LShield1_L4;
      //
      string strMat_Lv1LShield1 = cLv1LShield1.getString("Material");
      G4Material* mat_Lv1LShield1 = Materials::get(strMat_Lv1LShield1);

      //for (int i = 0; i < Lv1LShield1_num; i++) printf("%f %f\n", Lv1LShield1_Z[i], Lv1LShield1_O[i]);

      //define geometry
      G4Polycone* geo_Lv1LShield1pcon = new G4Polycone("geo_Lv1LShield1pcon_name", 0, 2*M_PI, Lv1LShield1_num, Lv1LShield1_Z, Lv1LShield1_I, Lv1LShield1_O);
      G4LogicalVolume *logi_Lv1LShield1 = new G4LogicalVolume(geo_Lv1LShield1pcon, mat_Lv1LShield1, "logi_Lv1LShield1_name");

      //-   put volume
      setColor(*logi_Lv1LShield1, cLv1LShield1.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv1LShield1, "phys_Lv1LShield1_name", &topVolume, false, 0);
      //-
      //--------------

      //--------------
      //=   L side (Lv1LShield2)

      //get parameters from .xml file
      GearDir cLv1LShield2(content, "Lv1LShield2/");
      //
      double Lv1LShield2_D1 = cLv1LShield2.getLength("D1") / Unit::mm;
      //
      double Lv1LShield2_L1 = cLv1LShield2.getLength("L1") / Unit::mm;
      double Lv1LShield2_L2 = cLv1LShield2.getLength("L2") / Unit::mm;
      //
      const int Lv1LShield2_num = 4;
      //
      double Lv1LShield2_I[Lv1LShield2_num];
      Lv1LShield2_I[0] = cLv1LShield2.getLength("I1") / Unit::mm;
      Lv1LShield2_I[1] = Lv1LShield2_I[0];
      Lv1LShield2_I[2] = Lv1LShield2_I[1];
      Lv1LShield2_I[3] = Lv1LShield2_I[2];
      //
      double Lv1LShield2_O[Lv1LShield2_num];
      Lv1LShield2_O[0] = cLv1LShield2.getLength("O1") / Unit::mm;
      Lv1LShield2_O[1] = Lv1LShield2_O[0];
      Lv1LShield2_O[2] = cLv1LShield2.getLength("O2") / Unit::mm;
      Lv1LShield2_O[3] = Lv1LShield2_O[2];
      //
      double Lv1LShield2_Z[Lv1LShield2_num];
      Lv1LShield2_Z[0] = -Lv1LShield2_D1;
      Lv1LShield2_Z[1] = Lv1LShield2_Z[0] - Lv1LShield2_L1;
      Lv1LShield2_Z[2] = Lv1LShield2_Z[1];
      Lv1LShield2_Z[3] = Lv1LShield2_Z[2] - Lv1LShield2_L2;
      //
      string strMat_Lv1LShield2 = cLv1LShield2.getString("Material");
      G4Material* mat_Lv1LShield2 = Materials::get(strMat_Lv1LShield2);

      //for (int i=0;i<Lv1LShield2_num;i++) printf("%f %f\n",Lv1LShield2_Z[i],Lv1LShield2_O[i]);

      //define geometry
      G4Polycone* geo_Lv1LShield2pcon = new G4Polycone("geo_Lv1LShield2pcon_name", 0, 2*M_PI, Lv1LShield2_num, Lv1LShield2_Z, Lv1LShield2_I, Lv1LShield2_O);
      G4LogicalVolume *logi_Lv1LShield2 = new G4LogicalVolume(geo_Lv1LShield2pcon, mat_Lv1LShield2, "logi_Lv1LShield2_name");

      //-   put volume
      setColor(*logi_Lv1LShield2, cLv1LShield2.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv1LShield2, "phys_Lv1LShield2_name", &topVolume, false, 0);
      //-
      //--------------

      //--------------
      //=   L side (Lv1LFlange1)

      //get parameters from .xml file
      GearDir cLv1LFlange1(content, "Lv1LFlange1/");
      //
      double Lv1LFlange1_D1 = cLv1LFlange1.getLength("D1") / Unit::mm;
      double Lv1LFlange1_L1 = cLv1LFlange1.getLength("L1") / Unit::mm;
      double Lv1LFlange1_I1 = cLv1LFlange1.getLength("I1") / Unit::mm;
      double Lv1LFlange1_O1 = cLv1LFlange1.getLength("O1") / Unit::mm;
      //
      string strMat_Lv1LFlange1 = cLv1LFlange1.getString("Material");
      G4Material* mat_Lv1LFlange1 = Materials::get(strMat_Lv1LFlange1);

      //define geometry
      G4Tubs* geo_Lv1LFlange1tubs = new G4Tubs("geo_Lv1LFlange1tubs_name", Lv1LFlange1_I1, Lv1LFlange1_O1, Lv1LFlange1_L1 / 2., 0, 2*M_PI);
      G4LogicalVolume *logi_Lv1LFlange1 = new G4LogicalVolume(geo_Lv1LFlange1tubs, mat_Lv1LFlange1, "logi_Lv1LFlange1_name");

      //-   put volume
      setColor(*logi_Lv1LFlange1, cLv1LFlange1.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, -Lv1LFlange1_D1 - Lv1LFlange1_L1 / 2.), logi_Lv1LFlange1, "phys_Lv1LFlange1_name", &topVolume, false, 0);
      //-
      //--------------
      */

    }
  }
}
