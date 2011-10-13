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
      BOOST_FOREACH(SensitiveDetector* sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    void GeoHeavyMetalShieldCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

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

    }
  }
}
