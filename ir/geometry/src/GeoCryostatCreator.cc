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

#include <ir/geometry/GeoCryostatCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <ir/simulation/SensitiveDetector.h>

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
#include <G4UnionSolid.hh>
#include <G4IntersectionSolid.hh>
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

    geometry::CreatorFactory<GeoCryostatCreator> GeoCryostatFactory("CryostatCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoCryostatCreator::GeoCryostatCreator()
    {
      //m_sensitive = new SensitiveDetector();
    }

    GeoCryostatCreator::~GeoCryostatCreator()
    {
      //delete m_sensitive;
    }

    void GeoCryostatCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

      //##############
      //#

      double DistanceR = content.getLength("DistanceR") / Unit::mm;
      double DistanceL = content.getLength("DistanceL") / Unit::mm;
      double AngleHER = content.getAngle("AngleHER");
      double AngleLER = content.getAngle("AngleLER");

      //--------------
      //-   A1wal1

      //get parameters from .xml file
      GearDir cA1wal1(content, "A1wal1/");
      //
      const int A1wal1_num = 8;
      //
      double A1wal1_Z[A1wal1_num];
      A1wal1_Z[0] = 0.;
      A1wal1_Z[1] = cA1wal1.getLength("L1") / Unit::mm;
      A1wal1_Z[2] = A1wal1_Z[1] + cA1wal1.getLength("L2") / Unit::mm;
      A1wal1_Z[3] = A1wal1_Z[2] + cA1wal1.getLength("L3") / Unit::mm;
      A1wal1_Z[4] = A1wal1_Z[3] + cA1wal1.getLength("L4") / Unit::mm;
      A1wal1_Z[5] = A1wal1_Z[4] + cA1wal1.getLength("L5") / Unit::mm;
      A1wal1_Z[6] = A1wal1_Z[5] + cA1wal1.getLength("L6") / Unit::mm;
      A1wal1_Z[7] = A1wal1_Z[6] + cA1wal1.getLength("L7") / Unit::mm;
      //
      double A1wal1_I[A1wal1_num];
      for (int i = 0; i < A1wal1_num; i++)
        { A1wal1_I[i] = 0.0; }
      //
      double A1wal1_O[A1wal1_num];
      A1wal1_O[0] = cA1wal1.getLength("R1") / Unit::mm;
      A1wal1_O[1] = A1wal1_O[0];
      A1wal1_O[2] = cA1wal1.getLength("R2") / Unit::mm;
      A1wal1_O[3] = A1wal1_O[2];
      A1wal1_O[4] = cA1wal1.getLength("R3") / Unit::mm;
      A1wal1_O[5] = A1wal1_O[4];
      A1wal1_O[6] = cA1wal1.getLength("R4") / Unit::mm;
      A1wal1_O[7] = A1wal1_O[6];
      //
      string strMat_A1wal1 = cA1wal1.getString("Material");
      G4Material* mat_A1wal1 = Materials::get(strMat_A1wal1);

      //define geometry
      G4Polycone* geo_A1wal1pcon = new G4Polycone("geo_A1wal1pcon_name", 0, 2*M_PI, A1wal1_num, A1wal1_Z, A1wal1_I, A1wal1_O);
      G4LogicalVolume *logi_A1wal1 = new G4LogicalVolume(geo_A1wal1pcon, mat_A1wal1, "logi_A1wal1_name");

      //-   put volume
      setColor(*logi_A1wal1, cA1wal1.getString("Color", "#00CC00"));
      G4Transform3D transform_A1wal1 = G4Translate3D(0., 0., 0.);
      transform_A1wal1 = transform_A1wal1 * G4RotateY3D(AngleHER);
      new G4PVPlacement(transform_A1wal1, logi_A1wal1, "phys_A1wal1_name", &topVolume, false, 0);

      //-
      //--------------

      //--------------
      //-   A2spc1

      //get parameters from .xml file
      GearDir cA2spc1(content, "A2spc1/");
      //
      double A2spc1_bpthick = cA2spc1.getLength("bpthick") / Unit::mm;
      //
      double A2spc1_O[A1wal1_num];
      A2spc1_O[0] = A1wal1_O[0] - A2spc1_bpthick;
      A2spc1_O[1] = A1wal1_O[1] - A2spc1_bpthick;
      A2spc1_O[2] = A1wal1_O[2] - A2spc1_bpthick;
      A2spc1_O[3] = A1wal1_O[3] - A2spc1_bpthick;
      A2spc1_O[4] = A1wal1_O[4] - A2spc1_bpthick;
      A2spc1_O[5] = A1wal1_O[5] - A2spc1_bpthick;
      A2spc1_O[6] = A1wal1_O[6] - A2spc1_bpthick;
      A2spc1_O[7] = A1wal1_O[7] - A2spc1_bpthick;
      //
      string strMat_A2spc1 = cA2spc1.getString("Material");
      G4Material* mat_A2spc1 = Materials::get(strMat_A2spc1);

      //define geometry
      G4Polycone* geo_A2spc1pcon = new G4Polycone("geo_A2spc1pcon_name", 0, 2*M_PI, A1wal1_num, A1wal1_Z, A1wal1_I, A2spc1_O);
      G4IntersectionSolid* geo_A2spc1 = new G4IntersectionSolid("", geo_A2spc1pcon, geo_A1wal1pcon);
      G4LogicalVolume *logi_A2spc1 = new G4LogicalVolume(geo_A2spc1, mat_A2spc1, "logi_A2spc1_name");

      //-   put volume
      setColor(*logi_A2spc1, cA2spc1.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A2spc1, "phys_A2spc1_name", logi_A1wal1, false, 0);

      //-
      //--------------

      //--------------
      //-   B1wal1

      //get parameters from .xml file
      GearDir cB1wal1(content, "B1wal1/");
      //
      const int B1wal1_num = 10;
      //
      double B1wal1_Z[B1wal1_num];
      B1wal1_Z[0] = 0.;
      B1wal1_Z[1] = cB1wal1.getLength("L1") / Unit::mm;
      B1wal1_Z[2] = B1wal1_Z[1] + cB1wal1.getLength("L2") / Unit::mm;
      B1wal1_Z[3] = B1wal1_Z[2] + cB1wal1.getLength("L3") / Unit::mm;
      B1wal1_Z[4] = B1wal1_Z[3] + cB1wal1.getLength("L4") / Unit::mm;
      B1wal1_Z[5] = B1wal1_Z[4] + cB1wal1.getLength("L5") / Unit::mm;
      B1wal1_Z[6] = B1wal1_Z[5] + cB1wal1.getLength("L6") / Unit::mm;
      B1wal1_Z[7] = B1wal1_Z[6] + cB1wal1.getLength("L7") / Unit::mm;
      B1wal1_Z[8] = B1wal1_Z[7] + cB1wal1.getLength("L8") / Unit::mm;
      B1wal1_Z[9] = B1wal1_Z[8] + cB1wal1.getLength("L9") / Unit::mm;
      //
      double B1wal1_I[B1wal1_num];
      for (int i = 0; i < B1wal1_num; i++)
        { B1wal1_I[i] = 0.0; }
      //
      double B1wal1_O[B1wal1_num];
      B1wal1_O[0] = cB1wal1.getLength("R1") / Unit::mm;
      B1wal1_O[1] = B1wal1_O[0];
      B1wal1_O[2] = cB1wal1.getLength("R2") / Unit::mm;
      B1wal1_O[3] = cB1wal1.getLength("R3") / Unit::mm;
      B1wal1_O[4] = B1wal1_O[3];
      B1wal1_O[5] = cB1wal1.getLength("R4") / Unit::mm;
      B1wal1_O[6] = cB1wal1.getLength("R5") / Unit::mm;
      B1wal1_O[7] = B1wal1_O[6];
      B1wal1_O[8] = cB1wal1.getLength("R6") / Unit::mm;
      B1wal1_O[9] = B1wal1_O[8];
      //
      string strMat_B1wal1 = cB1wal1.getString("Material");
      G4Material* mat_B1wal1 = Materials::get(strMat_B1wal1);

      //define geometry
      G4Polycone* geo_B1wal1pcon = new G4Polycone("geo_B1wal1pcon_name", 0, 2*M_PI, B1wal1_num, B1wal1_Z, B1wal1_I, B1wal1_O);
      G4LogicalVolume *logi_B1wal1 = new G4LogicalVolume(geo_B1wal1pcon, mat_B1wal1, "logi_B1wal1_name");

      //-   put volume
      setColor(*logi_B1wal1, cB1wal1.getString("Color", "#0000CC"));
      G4Transform3D transform_B1wal1 = G4Translate3D(0., 0., 0.);
      transform_B1wal1 = transform_B1wal1 * G4RotateY3D(AngleLER);
      new G4PVPlacement(transform_B1wal1, logi_B1wal1, "phys_B1wal1_name", &topVolume, false, 0);

      //-
      //--------------

      //--------------
      //-   B2spc1

      //get parameters from .xml file
      GearDir cB2spc1(content, "B2spc1/");
      //
      double B2spc1_bpthick = cB2spc1.getLength("bpthick") / Unit::mm;
      //
      double B2spc1_O[B1wal1_num];
      B2spc1_O[0] = B1wal1_O[0] - B2spc1_bpthick;
      B2spc1_O[1] = B1wal1_O[1] - B2spc1_bpthick;
      B2spc1_O[2] = B1wal1_O[2] - B2spc1_bpthick;
      B2spc1_O[3] = B1wal1_O[3] - B2spc1_bpthick;
      B2spc1_O[4] = B1wal1_O[4] - B2spc1_bpthick;
      B2spc1_O[5] = B1wal1_O[5] - B2spc1_bpthick;
      B2spc1_O[6] = B1wal1_O[6] - B2spc1_bpthick;
      B2spc1_O[7] = B1wal1_O[7] - B2spc1_bpthick;
      B2spc1_O[8] = B1wal1_O[8] - B2spc1_bpthick;
      B2spc1_O[9] = B1wal1_O[9] - B2spc1_bpthick;
      //
      string strMat_B2spc1 = cB2spc1.getString("Material");
      G4Material* mat_B2spc1 = Materials::get(strMat_B2spc1);

      //define geometry
      G4Polycone* geo_B2spc1pcon = new G4Polycone("geo_B2spc1pcon_name", 0, 2*M_PI, B1wal1_num, B1wal1_Z, B1wal1_I, B2spc1_O);
      G4IntersectionSolid* geo_B2spc1 = new G4IntersectionSolid("", geo_B2spc1pcon, geo_B1wal1pcon);
      G4LogicalVolume *logi_B2spc1 = new G4LogicalVolume(geo_B2spc1, mat_B2spc1, "logi_B2spc1_name");

      //-   put volume
      setColor(*logi_B2spc1, cB2spc1.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B2spc1, "phys_B2spc1_name", logi_B1wal1, false, 0);

      //-
      //--------------

      //--------------
      //-   C1wal1

      //get parameters from .xml file
      GearDir cC1wal1(content, "C1wal1/");
      //
      const int C1wal1_num = 11;
      //
      double C1wal1_Z[C1wal1_num];
      C1wal1_Z[0] = DistanceR;
      C1wal1_Z[1] = C1wal1_Z[0] + cC1wal1.getLength("L1") / Unit::mm;
      C1wal1_Z[2] = C1wal1_Z[1] + cC1wal1.getLength("L2") / Unit::mm;
      C1wal1_Z[3] = C1wal1_Z[2] + cC1wal1.getLength("L3") / Unit::mm;
      C1wal1_Z[4] = C1wal1_Z[3] + cC1wal1.getLength("L4") / Unit::mm;
      C1wal1_Z[5] = C1wal1_Z[4];
      C1wal1_Z[6] = C1wal1_Z[5] + cC1wal1.getLength("L5") / Unit::mm;
      C1wal1_Z[7] = C1wal1_Z[6];
      C1wal1_Z[8] = C1wal1_Z[7] + cC1wal1.getLength("L6") / Unit::mm;
      C1wal1_Z[9] = C1wal1_Z[8];
      C1wal1_Z[10] = C1wal1_Z[9] + cC1wal1.getLength("L7") / Unit::mm;
      //
      double C1wal1_I[C1wal1_num];
      for (int i = 0; i < C1wal1_num; i++)
        { C1wal1_I[i] = 0.0; }
      //
      double C1wal1_O[C1wal1_num];
      C1wal1_O[0] = cC1wal1.getLength("R1") / Unit::mm;
      C1wal1_O[1] = cC1wal1.getLength("R2") / Unit::mm;
      C1wal1_O[2] = C1wal1_O[1];
      C1wal1_O[3] = cC1wal1.getLength("R3") / Unit::mm;
      C1wal1_O[4] = C1wal1_O[3];
      C1wal1_O[5] = cC1wal1.getLength("R4") / Unit::mm;
      C1wal1_O[6] = C1wal1_O[5];
      C1wal1_O[7] = cC1wal1.getLength("R5") / Unit::mm;
      C1wal1_O[8] = C1wal1_O[7];
      C1wal1_O[9] = cC1wal1.getLength("R6") / Unit::mm;
      C1wal1_O[10] = C1wal1_O[9];
      //
      string strMat_C1wal1 = cC1wal1.getString("Material");
      G4Material* mat_C1wal1 = Materials::get(strMat_C1wal1);

      //define geometry
      G4Polycone* geo_C1wal1pcon = new G4Polycone("geo_C1wal1pcon_name", 0, 2*M_PI, C1wal1_num, C1wal1_Z, C1wal1_I, C1wal1_O);
      G4SubtractionSolid* geo_C1wal1 = new G4SubtractionSolid("", geo_C1wal1pcon, geo_A1wal1pcon, transform_A1wal1);
      geo_C1wal1 = new G4SubtractionSolid("", geo_C1wal1pcon, geo_B1wal1pcon, transform_B1wal1);
      //geo_C1wal1 = G4SubtractionSolid("", geo_C1wal1pcon, geo_B1wal1pcon, transform_B1wal1);
      G4LogicalVolume *logi_C1wal1 = new G4LogicalVolume(geo_C1wal1, mat_C1wal1, "logi_C1wal1_name");

      //-   put volume
      setColor(*logi_C1wal1, cC1wal1.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C1wal1, "phys_C1wal1_name", &topVolume, false, 0);

      //-
      //--------------

      //--------------
      //-   C2spc1

      //get parameters from .xml file
      GearDir cC2spc1(content, "C2spc1/");
      //
      const int C2spc1_num = 9;
      //
      double C2spc1_Z[C2spc1_num];
      C2spc1_Z[0] = DistanceR + cC2spc1.getLength("D1") / Unit::mm;
      C2spc1_Z[1] = C2spc1_Z[0] + cC2spc1.getLength("L1") / Unit::mm;
      C2spc1_Z[2] = C2spc1_Z[1] + cC2spc1.getLength("L2") / Unit::mm;
      C2spc1_Z[3] = C2spc1_Z[2] + cC2spc1.getLength("L3") / Unit::mm;
      C2spc1_Z[4] = C2spc1_Z[3] + cC2spc1.getLength("L4") / Unit::mm - cC2spc1.getLength("L5") / Unit::mm;
      C2spc1_Z[5] = C2spc1_Z[4];
      C2spc1_Z[6] = C2spc1_Z[5] + cC2spc1.getLength("L6") / Unit::mm;
      C2spc1_Z[7] = C2spc1_Z[6] + cC2spc1.getLength("L7") / Unit::mm;
      C2spc1_Z[8] = C2spc1_Z[7] + cC2spc1.getLength("L8") / Unit::mm;
      //
      double C2spc1_I[C2spc1_num];
      for (int i = 0; i < C2spc1_num; i++)
        { C2spc1_I[i] = 0.0; }
      //
      double C2spc1_O[C2spc1_num];
      C2spc1_O[0] = cC2spc1.getLength("R1") / Unit::mm;
      C2spc1_O[1] = cC2spc1.getLength("R2") / Unit::mm;
      C2spc1_O[2] = C2spc1_O[1];
      C2spc1_O[3] = cC2spc1.getLength("R3") / Unit::mm;
      C2spc1_O[4] = C2spc1_O[3];
      C2spc1_O[5] = cC2spc1.getLength("R5") / Unit::mm;
      C2spc1_O[6] = C2spc1_O[5];
      C2spc1_O[7] = cC2spc1.getLength("R6") / Unit::mm;
      C2spc1_O[8] = C2spc1_O[7];
      //
      string strMat_C2spc1 = cC2spc1.getString("Material");
      G4Material* mat_C2spc1 = Materials::get(strMat_C2spc1);

      //-
      //--------------

      //--------------
      //-

      //-
      //--------------
    }
  }
}
