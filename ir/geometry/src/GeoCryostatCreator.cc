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
      // -Index-
      // +- A1wal1
      //      +- A2spc1
      //         +- Achecker
      // +- B1wal1
      //    +- B2spc1
      // +- C1wal1
      //    +- C2spc1
      //       +- C3wal2
      //          +- C4spc2
      //             +- C5wal3
      //             +- C5wal5
      //                +- C6spc4
      //                +- C6spc5
      //                +- C6spc7
      //                +- C6spc8
      //                +- C6spc3
      //                +- C6spc6
      //                   +- C7mag1
      //                   +- C7mag2
      //                   +- C7mag3
      //                   +- C7mag4
      //                   +- C7mag5
      //                   +- C7mag6
      //                   +- C7mag7
      //                   +- C7hld1
      //                   +- C7cil1
      // +- D1wal1
      //    +- D2spc1
      // +- E1wal1
      //    +- E2spc1
      // +- F1wal1
      //    +- F2spc1
      //       +- F3wal2
      //       +- F3wal3
      //          +- F4spc2
      //          +- F4spc3
      //             +- F5wal4
      //             +- F5wal5
      //                +- F6spc5
      //                +- F6spc6
      //                +- F6spc4
      //                +- F6spc8
      //                +- F6spc7
      //                   +- F7mag1
      //                   +- F7mag2
      //                   +- F7mag3
      //                   +- F7mag4
      //                   +- F7mag5
      //                   +- F7mag6
      //                   +- F7mag7
      //                   +- F7hld1
      //                   +- F7cil1

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

      //define geometry
      G4Polycone* geo_C2spc1pcon = new G4Polycone("geo_C2spc1pcon_name", 0, 2*M_PI, C2spc1_num, C2spc1_Z, C2spc1_I, C2spc1_O);
      G4SubtractionSolid* geo_C2spc1 = new G4SubtractionSolid("", geo_C2spc1pcon, geo_A1wal1pcon, transform_A1wal1);
      geo_C2spc1 = new G4SubtractionSolid("", geo_C2spc1pcon, geo_B1wal1pcon, transform_B1wal1);
      G4LogicalVolume *logi_C2spc1 = new G4LogicalVolume(geo_C2spc1, mat_C2spc1, "logi_C2spc1_name");

      //-   put volume
      setColor(*logi_C2spc1, cC2spc1.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C2spc1, "phys_C2spc1_name", logi_C1wal1, false, 0);

      //-
      //--------------

      //--------------
      //-   C3wal2

      //get parameters from .xml file
      GearDir cC3wal2(content, "C3wal2/");
      //
      const int C3wal2_num = 7;
      //
      double C3wal2_Z[C3wal2_num];
      C3wal2_Z[0] = C2spc1_Z[0] + cC3wal2.getLength("D1") / Unit::mm;
      C3wal2_Z[1] = C3wal2_Z[0] + cC3wal2.getLength("L1") / Unit::mm;
      C3wal2_Z[2] = C3wal2_Z[1] + cC3wal2.getLength("L2") / Unit::mm;
      C3wal2_Z[3] = C3wal2_Z[2] + cC3wal2.getLength("L3") / Unit::mm;
      C3wal2_Z[4] = C3wal2_Z[3] + cC3wal2.getLength("L4") / Unit::mm;
      C3wal2_Z[5] = C3wal2_Z[4];
      C3wal2_Z[6] = C3wal2_Z[5] + cC3wal2.getLength("L5") / Unit::mm;
      //
      double C3wal2_I[C3wal2_num];
      for (int i = 0; i < C3wal2_num; i++)
        { C3wal2_I[i] = 0.0; }
      //
      double C3wal2_O[C3wal2_num];
      C3wal2_O[0] = cC3wal2.getLength("R1") / Unit::mm;
      C3wal2_O[1] = cC3wal2.getLength("R2") / Unit::mm;
      C3wal2_O[2] = C3wal2_O[1];
      C3wal2_O[3] = cC3wal2.getLength("R3") / Unit::mm;
      C3wal2_O[4] = C3wal2_O[3];
      C3wal2_O[5] = cC3wal2.getLength("R4") / Unit::mm;
      C3wal2_O[6] = C3wal2_O[5];
      //
      string strMat_C3wal2 = cC3wal2.getString("Material");
      G4Material* mat_C3wal2 = Materials::get(strMat_C3wal2);

      //define geometry
      G4Polycone* geo_C3wal2pcon = new G4Polycone("geo_C3wal2pcon_name", 0, 2*M_PI, C3wal2_num, C3wal2_Z, C3wal2_I, C3wal2_O);
      G4SubtractionSolid* geo_C3wal2 = new G4SubtractionSolid("", geo_C3wal2pcon, geo_A1wal1pcon, transform_A1wal1);
      geo_C3wal2 = new G4SubtractionSolid("", geo_C3wal2pcon, geo_B1wal1pcon, transform_B1wal1);
      G4LogicalVolume *logi_C3wal2 = new G4LogicalVolume(geo_C3wal2, mat_C3wal2, "logi_C3wal2_name");

      //-   put volume
      setColor(*logi_C3wal2, cC3wal2.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C3wal2, "phys_C3wal2_name", logi_C2spc1, false, 0);

      //-
      //--------------

      //--------------
      //-   C4spc2

      //get parameters from .xml file
      GearDir cC4spc2(content, "C4spc2/");
      //
      const int C4spc2_num = 7;
      //
      double C4spc2_Z[C4spc2_num];
      C4spc2_Z[0] = C3wal2_Z[0] + cC4spc2.getLength("D1") / Unit::mm;
      C4spc2_Z[1] = C4spc2_Z[0] + cC4spc2.getLength("L1") / Unit::mm;
      C4spc2_Z[2] = C4spc2_Z[1] + cC4spc2.getLength("L2") / Unit::mm;
      C4spc2_Z[3] = C4spc2_Z[2] + cC4spc2.getLength("L3") / Unit::mm;
      C4spc2_Z[4] = C4spc2_Z[3] + cC4spc2.getLength("L4") / Unit::mm;
      C4spc2_Z[5] = C4spc2_Z[4];
      C4spc2_Z[6] = C4spc2_Z[5] + cC4spc2.getLength("L5") / Unit::mm;
      //
      double C4spc2_I[C4spc2_num];
      for (int i = 0; i < C4spc2_num; i++)
        { C4spc2_I[i] = 0.0; }
      //
      double C4spc2_O[C4spc2_num];
      C4spc2_O[0] = cC4spc2.getLength("R1") / Unit::mm;
      C4spc2_O[1] = cC4spc2.getLength("R2") / Unit::mm;
      C4spc2_O[2] = C4spc2_O[1];
      C4spc2_O[3] = cC4spc2.getLength("R3") / Unit::mm;
      C4spc2_O[4] = C4spc2_O[3];
      C4spc2_O[5] = cC4spc2.getLength("R4") / Unit::mm;
      C4spc2_O[6] = C4spc2_O[5];
      //
      string strMat_C4spc2 = cC4spc2.getString("Material");
      G4Material* mat_C4spc2 = Materials::get(strMat_C4spc2);

      //define geometry
      G4Polycone* geo_C4spc2pcon = new G4Polycone("geo_C4spc2pcon_name", 0, 2*M_PI, C4spc2_num, C4spc2_Z, C4spc2_I, C4spc2_O);
      G4SubtractionSolid* geo_C4spc2 = new G4SubtractionSolid("", geo_C4spc2pcon, geo_A1wal1pcon, transform_A1wal1);
      geo_C4spc2 = new G4SubtractionSolid("", geo_C4spc2pcon, geo_B1wal1pcon, transform_B1wal1);
      G4LogicalVolume *logi_C4spc2 = new G4LogicalVolume(geo_C4spc2, mat_C4spc2, "logi_C4spc2_name");

      //-   put volume
      setColor(*logi_C4spc2, cC4spc2.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C4spc2, "phys_C4spc2_name", logi_C3wal2, false, 0);

      //-
      //--------------

      //--------------
      //-   C5wal3

      //get parameters from .xml file
      GearDir cC5wal3(content, "C5wal3/");
      //
      const int C5wal3_num = 8;
      //
      double C5wal3_Z[C5wal3_num];
      C5wal3_Z[0] = C4spc2_Z[0] + cC5wal3.getLength("D1") / Unit::mm;
      C5wal3_Z[1] = C5wal3_Z[0] + cC5wal3.getLength("L1") / Unit::mm;
      C5wal3_Z[2] = C5wal3_Z[1] + cC5wal3.getLength("L2") / Unit::mm;
      C5wal3_Z[3] = C5wal3_Z[2];
      C5wal3_Z[4] = C5wal3_Z[3] + cC5wal3.getLength("L3") / Unit::mm;
      C5wal3_Z[5] = C5wal3_Z[4] + cC5wal3.getLength("L4") / Unit::mm;
      C5wal3_Z[6] = C5wal3_Z[5] + cC5wal3.getLength("L5") / Unit::mm;
      C5wal3_Z[7] = C5wal3_Z[6] + cC5wal3.getLength("L6") / Unit::mm;
      //
      double C5wal3_I[C5wal3_num];
      for (int i = 0; i < C5wal3_num; i++)
        { C5wal3_I[i] = 0.0; }
      //
      double C5wal3_O[C5wal3_num];
      C5wal3_O[0] = cC5wal3.getLength("R1") / Unit::mm;
      C5wal3_O[1] = cC5wal3.getLength("R2") / Unit::mm;
      C5wal3_O[2] = C5wal3_O[1];
      C5wal3_O[3] = cC5wal3.getLength("R3") / Unit::mm;
      C5wal3_O[4] = C5wal3_O[3];
      C5wal3_O[5] = cC5wal3.getLength("R4") / Unit::mm;
      C5wal3_O[6] = C5wal3_O[5];
      C5wal3_O[7] = cC5wal3.getLength("R5") / Unit::mm;
      //
      string strMat_C5wal3 = cC5wal3.getString("Material");
      G4Material* mat_C5wal3 = Materials::get(strMat_C5wal3);

      //define geometry
      G4Polycone* geo_C5wal3pcon = new G4Polycone("geo_C5wal3pcon_name", 0, 2*M_PI, C5wal3_num, C5wal3_Z, C5wal3_I, C5wal3_O);
      G4SubtractionSolid* geo_C5wal3 = new G4SubtractionSolid("", geo_C5wal3pcon, geo_A1wal1pcon, transform_A1wal1);
      geo_C5wal3 = new G4SubtractionSolid("", geo_C5wal3pcon, geo_B1wal1pcon, transform_B1wal1);
      G4LogicalVolume *logi_C5wal3 = new G4LogicalVolume(geo_C5wal3, mat_C5wal3, "logi_C5wal3_name");

      //-   put volume
      setColor(*logi_C5wal3, cC5wal3.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C5wal3, "phys_C5wal3_name", logi_C4spc2, false, 0);

      //-
      //--------------

      //--------------
      //-   C5wal5

      //get parameters from .xml file
      GearDir cC5wal5(content, "C5wal5/");
      //
      const int C5wal5_num = 3;
      //
      double C5wal5_Z[C5wal5_num];
      C5wal5_Z[0] = C4spc2_Z[0] + cC5wal5.getLength("D1") / Unit::mm;
      C5wal5_Z[1] = C5wal5_Z[0] + cC5wal5.getLength("L1") / Unit::mm;
      C5wal5_Z[2] = C5wal5_Z[1] + cC5wal5.getLength("L2") / Unit::mm;
      //
      double C5wal5_I[C5wal5_num];
      for (int i = 0; i < C5wal5_num; i++)
        { C5wal5_I[i] = 0.0; }
      //
      double C5wal5_O[C5wal5_num];
      C5wal5_O[0] = cC5wal5.getLength("R1") / Unit::mm;
      C5wal5_O[1] = C5wal5_O[0];
      C5wal5_O[2] = cC5wal5.getLength("R2") / Unit::mm;
      //
      string strMat_C5wal5 = cC5wal5.getString("Material");
      G4Material* mat_C5wal5 = Materials::get(strMat_C5wal5);

      //define geometry
      G4Polycone* geo_C5wal5pcon = new G4Polycone("geo_C5wal5pcon_name", 0, 2*M_PI, C5wal5_num, C5wal5_Z, C5wal5_I, C5wal5_O);
      G4SubtractionSolid* geo_C5wal5 = new G4SubtractionSolid("", geo_C5wal5pcon, geo_A1wal1pcon, transform_A1wal1);
      geo_C5wal5 = new G4SubtractionSolid("", geo_C5wal5pcon, geo_B1wal1pcon, transform_B1wal1);
      G4LogicalVolume *logi_C5wal5 = new G4LogicalVolume(geo_C5wal5, mat_C5wal5, "logi_C5wal5_name");

      //-   put volume
      setColor(*logi_C5wal5, cC5wal5.getString("Color", "#CC0000"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C5wal5, "phys_C5wal5_name", logi_C4spc2, false, 0);

      //-
      //--------------

      //--------------
      //-   C6spc4

      //get parameters from .xml file
      GearDir cC6spc4(content, "C6spc4/");
      //
      const int C6spc4_num = 6;
      //
      double C6spc4_Z[C6spc4_num];
      C6spc4_Z[0] = 0.0;
      C6spc4_Z[1] = C6spc4_Z[0] + cC6spc4.getLength("L1") / Unit::mm;
      C6spc4_Z[2] = C6spc4_Z[1] + cC6spc4.getLength("L2") / Unit::mm;
      C6spc4_Z[3] = C6spc4_Z[2] + cC6spc4.getLength("L3") / Unit::mm;
      C6spc4_Z[4] = C6spc4_Z[3] + cC6spc4.getLength("L4") / Unit::mm;
      C6spc4_Z[5] = C6spc4_Z[4] + cC6spc4.getLength("L5") / Unit::mm;
      //
      double C6spc4_I[C6spc4_num];
      for (int i = 0; i < C6spc4_num; i++)
        { C6spc4_I[i] = 0.0; }
      //
      double C6spc4_O[C6spc4_num];
      C6spc4_O[0] = cC6spc4.getLength("R1") / Unit::mm;
      C6spc4_O[1] = C6spc4_O[0];
      C6spc4_O[2] = cC6spc4.getLength("R2") / Unit::mm;
      C6spc4_O[3] = C6spc4_O[2];
      C6spc4_O[4] = cC6spc4.getLength("R3") / Unit::mm;
      C6spc4_O[5] = C6spc4_O[4];
      //
      string strMat_C6spc4 = cC6spc4.getString("Material");
      G4Material* mat_C6spc4 = Materials::get(strMat_C6spc4);

      //define geometry
      G4Polycone* geo_C6spc4pcon = new G4Polycone("geo_C6spc4pcon_name", 0, 2*M_PI, C6spc4_num, C6spc4_Z, C6spc4_I, C6spc4_O);
      G4SubtractionSolid* geo_C6spc4_sub = new G4SubtractionSolid("", geo_C6spc4pcon, geo_A1wal1pcon);
      G4IntersectionSolid* geo_C6spc4 = new G4IntersectionSolid("", geo_C6spc4_sub, geo_C5wal3pcon);
      G4LogicalVolume *logi_C6spc4 = new G4LogicalVolume(geo_C6spc4, mat_C6spc4, "logi_C6spc4_name");

      //-   put volume
      setColor(*logi_C6spc4, cC6spc4.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc4, "phys_C6spc4_name", logi_C5wal3, false, 0);

      //-
      //--------------

      //--------------
      //-   C6spc5

      //get parameters from .xml file
      GearDir cC6spc5(content, "C6spc5/");
      //
      const int C6spc5_num = 8;
      //
      double C6spc5_Z[C6spc5_num];
      C6spc5_Z[0] = 0.0;
      C6spc5_Z[1] = C6spc5_Z[0] + cC6spc5.getLength("L1") / Unit::mm;
      C6spc5_Z[2] = C6spc5_Z[1] + cC6spc5.getLength("L2") / Unit::mm;
      C6spc5_Z[3] = C6spc5_Z[2] + cC6spc5.getLength("L3") / Unit::mm;
      C6spc5_Z[4] = C6spc5_Z[3] + cC6spc5.getLength("L4") / Unit::mm;
      C6spc5_Z[5] = C6spc5_Z[4] + cC6spc5.getLength("L5") / Unit::mm;
      C6spc5_Z[6] = C6spc5_Z[5] + cC6spc5.getLength("L6") / Unit::mm;
      C6spc5_Z[7] = C6spc5_Z[6] + cC6spc5.getLength("L7") / Unit::mm;
      //
      double C6spc5_I[C6spc5_num];
      for (int i = 0; i < C6spc5_num; i++)
        { C6spc5_I[i] = 0.0; }
      //
      double C6spc5_O[C6spc5_num];
      C6spc5_O[0] = cC6spc5.getLength("R1") / Unit::mm;
      C6spc5_O[1] = C6spc5_O[0];
      C6spc5_O[2] = cC6spc5.getLength("R2") / Unit::mm;
      C6spc5_O[2] = cC6spc5.getLength("R3") / Unit::mm;
      C6spc5_O[3] = C6spc5_O[2];
      C6spc5_O[4] = cC6spc5.getLength("R4") / Unit::mm;
      C6spc5_O[4] = cC6spc5.getLength("R5") / Unit::mm;
      C6spc5_O[5] = C6spc5_O[4];
      //
      string strMat_C6spc5 = cC6spc5.getString("Material");
      G4Material* mat_C6spc5 = Materials::get(strMat_C6spc5);

      //define geometry
      G4Polycone* geo_C6spc5pcon = new G4Polycone("geo_C6spc5pcon_name", 0, 2*M_PI, C6spc5_num, C6spc5_Z, C6spc5_I, C6spc5_O);
      G4SubtractionSolid* geo_C6spc5_sub = new G4SubtractionSolid("", geo_C6spc5pcon, geo_B1wal1pcon);
      G4IntersectionSolid* geo_C6spc5 = new G4IntersectionSolid("", geo_C6spc5_sub, geo_C5wal3pcon);
      G4LogicalVolume *logi_C6spc5 = new G4LogicalVolume(geo_C6spc5, mat_C6spc5, "logi_C6spc5_name");

      //-   put volume
      setColor(*logi_C6spc5, cC6spc5.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc5, "phys_C6spc5_name", logi_C5wal3, false, 0);

      //-
      //--------------

      //--------------
      //-   C6spc7

      //get parameters from .xml file
      GearDir cC6spc7(content, "C6spc7/");
      //
      const int C6spc7_num = 2;
      //
      double C6spc7_Z[C6spc7_num];
      C6spc7_Z[0] = cC6spc7.getLength("D1") / Unit::mm;
      C6spc7_Z[1] = C6spc7_Z[0] + cC6spc7.getLength("L1") / Unit::mm;
      //
      double C6spc7_I[C6spc7_num];
      for (int i = 0; i < C6spc7_num; i++)
        { C6spc7_I[i] = 0.0; }
      //
      double C6spc7_O[C6spc7_num];
      C6spc7_O[0] = cC6spc7.getLength("R1") / Unit::mm;
      C6spc7_O[1] = C6spc7_O[0];
      //
      string strMat_C6spc7 = cC6spc7.getString("Material");
      G4Material* mat_C6spc7 = Materials::get(strMat_C6spc7);

      //define geometry
      G4Polycone* geo_C6spc7pcon = new G4Polycone("geo_C6spc7pcon_name", 0, 2*M_PI, C6spc7_num, C6spc7_Z, C6spc7_I, C6spc7_O);
      G4SubtractionSolid* geo_C6spc7_sub = new G4SubtractionSolid("", geo_C6spc7pcon, geo_A1wal1pcon);
      G4IntersectionSolid* geo_C6spc7 = new G4IntersectionSolid("", geo_C6spc7_sub, geo_C5wal5pcon);
      G4LogicalVolume *logi_C6spc7 = new G4LogicalVolume(geo_C6spc7, mat_C6spc7, "logi_C6spc7_name");

      //-   put volume
      setColor(*logi_C6spc7, cC6spc7.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc7, "phys_C6spc7_name", logi_C5wal5, false, 0);

      //-
      //--------------

      //--------------
      //-   C6spc8

      //get parameters from .xml file
      GearDir cC6spc8(content, "C6spc8/");
      //
      const int C6spc8_num = 2;
      //
      double C6spc8_Z[C6spc8_num];
      C6spc8_Z[0] = cC6spc8.getLength("D1") / Unit::mm;
      C6spc8_Z[1] = C6spc8_Z[0] + cC6spc8.getLength("L1") / Unit::mm;
      //
      double C6spc8_I[C6spc8_num];
      for (int i = 0; i < C6spc8_num; i++)
        { C6spc8_I[i] = 0.0; }
      //
      double C6spc8_O[C6spc8_num];
      C6spc8_O[0] = cC6spc8.getLength("R1") / Unit::mm;
      C6spc8_O[1] = C6spc8_O[0];
      //
      string strMat_C6spc8 = cC6spc8.getString("Material");
      G4Material* mat_C6spc8 = Materials::get(strMat_C6spc8);

      //define geometry
      G4Polycone* geo_C6spc8pcon = new G4Polycone("geo_C6spc8pcon_name", 0, 2*M_PI, C6spc8_num, C6spc8_Z, C6spc8_I, C6spc8_O);
      G4SubtractionSolid* geo_C6spc8_sub = new G4SubtractionSolid("", geo_C6spc8pcon, geo_B1wal1pcon);
      G4IntersectionSolid* geo_C6spc8 = new G4IntersectionSolid("", geo_C6spc8_sub, geo_C5wal5pcon);
      G4LogicalVolume *logi_C6spc8 = new G4LogicalVolume(geo_C6spc8, mat_C6spc8, "logi_C6spc8_name");

      //-   put volume
      setColor(*logi_C6spc8, cC6spc8.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc8, "phys_C6spc8_name", logi_C5wal5, false, 0);

      //-
      //--------------

      //--------------
      //-   C6spc3

      //get parameters from .xml file
      GearDir cC6spc3(content, "C6spc3/");
      //
      const int C6spc3_num = 7;
      //
      double C6spc3_Z[C6spc3_num];
      C6spc3_Z[0] = C5wal3_Z[0] + cC6spc3.getLength("D1") / Unit::mm;
      C6spc3_Z[1] = C6spc3_Z[0] + cC6spc3.getLength("L1") / Unit::mm;
      C6spc3_Z[2] = C6spc3_Z[1] + cC6spc3.getLength("L2") / Unit::mm;
      C6spc3_Z[3] = C6spc3_Z[2];
      C6spc3_Z[4] = C6spc3_Z[3] + cC6spc3.getLength("L3") / Unit::mm;
      C6spc3_Z[5] = C6spc3_Z[4] + cC6spc3.getLength("L4") / Unit::mm;
      C6spc3_Z[6] = C6spc3_Z[5] + cC6spc3.getLength("L5") / Unit::mm;
      //
      double C6spc3_I[C6spc3_num];
      for (int i = 0; i < C6spc3_num; i++)
        { C6spc3_I[i] = 0.0; }
      //
      double C6spc3_O[C6spc3_num];
      C6spc3_O[0] = cC6spc3.getLength("R1") / Unit::mm;
      C6spc3_O[1] = cC6spc3.getLength("R2") / Unit::mm;
      C6spc3_O[2] = C6spc3_O[1];
      C6spc3_O[3] = cC6spc3.getLength("R3") / Unit::mm;
      C6spc3_O[4] = C6spc3_O[3];
      C6spc3_O[5] = cC6spc3.getLength("R4") / Unit::mm;
      C6spc3_O[6] = C6spc3_O[5];
      //
      string strMat_C6spc3 = cC6spc3.getString("Material");
      G4Material* mat_C6spc3 = Materials::get(strMat_C6spc3);

      //- C6tnl1
      //get parameters from .xml file
      GearDir cC6tnl1(content, "C6tnl1/");
      //
      const int C6tnl1_num = 6;
      //
      double C6tnl1_Z[C6tnl1_num];
      C6tnl1_Z[0] = 0.0;
      C6tnl1_Z[1] = C6tnl1_Z[0] + cC6tnl1.getLength("L1") / Unit::mm;
      C6tnl1_Z[2] = C6tnl1_Z[1] + cC6tnl1.getLength("L2") / Unit::mm;
      C6tnl1_Z[3] = C6tnl1_Z[2] + cC6tnl1.getLength("L3") / Unit::mm;
      C6tnl1_Z[4] = C6tnl1_Z[3] + cC6tnl1.getLength("L4") / Unit::mm;
      C6tnl1_Z[5] = C6tnl1_Z[4] + cC6tnl1.getLength("L5") / Unit::mm;
      //
      double C6tnl1_I[C6tnl1_num];
      for (int i = 0; i < C6tnl1_num; i++)
        { C6tnl1_I[i] = 0.0; }
      //
      double C6tnl1_O[C6tnl1_num];
      C6tnl1_O[0] = cC6tnl1.getLength("R1") / Unit::mm;
      C6tnl1_O[1] = C6tnl1_O[0];
      C6tnl1_O[2] = cC6tnl1.getLength("R2") / Unit::mm;
      C6tnl1_O[3] = C6tnl1_O[2];
      C6tnl1_O[4] = cC6tnl1.getLength("R3") / Unit::mm;
      C6tnl1_O[5] = C6tnl1_O[4];
      //define geometry
      G4Polycone* geo_C6tnl1pcon = new G4Polycone("geo_C6tnl1pcon_name", 0, 2*M_PI, C6tnl1_num, C6tnl1_Z, C6tnl1_I, C6tnl1_O);

      //- C6tnl2
      //get parameters from .xml file
      GearDir cC6tnl2(content, "C6tnl2/");
      //
      const int C6tnl2_num = 8;
      //
      double C6tnl2_Z[C6tnl2_num];
      C6tnl2_Z[0] = 0.0;
      C6tnl2_Z[1] = C6tnl2_Z[0] + cC6tnl2.getLength("L1") / Unit::mm;
      C6tnl2_Z[2] = C6tnl2_Z[1] + cC6tnl2.getLength("L2") / Unit::mm;
      C6tnl2_Z[3] = C6tnl2_Z[2] + cC6tnl2.getLength("L3") / Unit::mm;
      C6tnl2_Z[4] = C6tnl2_Z[3] + cC6tnl2.getLength("L4") / Unit::mm;
      C6tnl2_Z[5] = C6tnl2_Z[4] + cC6tnl2.getLength("L5") / Unit::mm;
      C6tnl2_Z[6] = C6tnl2_Z[5] + cC6tnl2.getLength("L6") / Unit::mm;
      C6tnl2_Z[7] = C6tnl2_Z[6] + cC6tnl2.getLength("L7") / Unit::mm;
      //
      double C6tnl2_I[C6tnl2_num];
      for (int i = 0; i < C6tnl2_num; i++)
        { C6tnl2_I[i] = 0.0; }
      //
      double C6tnl2_O[C6tnl2_num];
      C6tnl2_O[0] = cC6tnl2.getLength("R1") / Unit::mm;
      C6tnl2_O[1] = C6tnl2_O[0];
      C6tnl2_O[2] = cC6tnl2.getLength("R2") / Unit::mm;
      C6tnl2_O[3] = cC6tnl2.getLength("R3") / Unit::mm;
      C6tnl2_O[4] = C6tnl2_O[3];
      C6tnl2_O[5] = cC6tnl2.getLength("R4") / Unit::mm;
      C6tnl2_O[6] = cC6tnl2.getLength("R5") / Unit::mm;
      C6tnl2_O[7] = C6tnl2_O[6];
      //define geometry
      G4Polycone* geo_C6tnl2pcon = new G4Polycone("geo_C6tnl2pcon_name", 0, 2*M_PI, C6tnl2_num, C6tnl2_Z, C6tnl2_I, C6tnl2_O);

      //define geometry
      G4Polycone* geo_C6spc3pcon = new G4Polycone("geo_C6spc3pcon_name", 0, 2*M_PI, C6spc3_num, C6spc3_Z, C6spc3_I, C6spc3_O);
      G4SubtractionSolid* geo_C6spc3 = new G4SubtractionSolid("", geo_C6spc3pcon, geo_C6tnl1pcon, transform_A1wal1);
      geo_C6spc3 = new G4SubtractionSolid("", geo_C6spc3pcon, geo_C6tnl2pcon, transform_B1wal1);
      G4LogicalVolume *logi_C6spc3 = new G4LogicalVolume(geo_C6spc3, mat_C6spc3, "logi_C6spc3_name");

      //-   put volume
      setColor(*logi_C6spc3, cC6spc3.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc3, "phys_C6spc3_name", logi_C5wal3, false, 0);

      //-
      //--------------

      /*
            //--------------
            //-   C6spc6

            //get parameters from .xml file
            GearDir cC6spc6(content, "C6spc6/");
            //
            const int C6spc6_num = 3;
            //
            double C6spc6_Z[C6spc6_num];
            C6spc6_Z[0] = C5wal5_Z[0] + cC6spc6.getLength("D1") / Unit::mm;
            C6spc6_Z[1] = C6spc6_Z[0] + cC6spc6.getLength("L1") / Unit::mm;
            C6spc6_Z[2] = C6spc6_Z[1] + cC6spc6.getLength("L2") / Unit::mm;
            //
            double C6spc6_I[C6spc6_num];
            for (int i = 0; i < C6spc6_num; i++)
              { C6spc6_I[i] = 0.0; }
            //
            double C6spc6_O[C6spc6_num];
            C6spc6_O[0] = cC6spc6.getLength("R1") / Unit::mm;
            C6spc6_O[1] = C6spc6_O[0];
            C6spc6_O[2] = cC6spc6.getLength("R2") / Unit::mm;
            //
            string strMat_C6spc6 = cC6spc6.getString("Material");
            G4Material* mat_C6spc6 = Materials::get(strMat_C6spc6);

            //- C6tnl3
            //get parameters from .xml file
            GearDir cC6tnl3(content, "C6tnl3/");
            //
            const int C6tnl3_num = 2;
            //
            double C6tnl3_Z[C6tnl3_num];
            C6tnl3_Z[0] = cC6tnl3.getLentgh("D1") / Unit::mm;
            C6tnl3_Z[1] = C6tnl3_Z[0] + cC6tnl3.getLength("L1") / Unit::mm;
            //
            double C6tnl3_I[C6tnl3_num];
            for (int i = 0; i < C6tnl3_num; i++)
              { C6tnl3_I[i] = 0.0; }
            //
            double C6tnl3_O[C6tnl3_num];
            C6tnl3_O[0] = cC6tnl3.getLength("R1") / Unit::mm;
            C6tnl3_O[1] = C6tnl3_O[0];
            //define geometry
            G4Polycone* geo_C6tnl3pcon = new G4Polycone("geo_C6tnl3pcon_name", 0, 2*M_PI, C6tnl3_num, C6tnl3_Z, C6tnl3_I, C6tnl3_O);

            //- C6tnl4
            //get parameters from .xml file
            GearDir cC6tnl4(content, "C6tnl4/");
            //
            const int C6tnl4_num = 2;
            //
            double C6tnl4_Z[C6tnl4_num];
            C6tnl4_Z[0] = cC6tnl4.getLentgh("D1") / Unit::mm;
            C6tnl4_Z[1] = C6tnl4_Z[0] + cC6tnl4.getLength("L1") / Unit::mm;
            //
            double C6tnl4_I[C6tnl4_num];
            for (int i = 0; i < C6tnl4_num; i++)
              { C6tnl4_I[i] = 0.0; }
            //
            double C6tnl4_O[C6tnl4_num];
            C6tnl4_O[0] = cC6tnl4.getLength("R1") / Unit::mm;
            C6tnl4_O[1] = C6tnl4_O[0];
            //define geometry
            G4Polycone* geo_C6tnl4pcon = new G4Polycone("geo_C6tnl4pcon_name", 0, 2*M_PI, C6tnl4_num, C6tnl4_Z, C6tnl4_I, C6tnl4_O);

            //define geometry
            G4Polycone* geo_C6spc6pcon = new G4Polycone("geo_C6spc6pcon_name", 0, 2*M_PI, C6spc6_num, C6spc6_Z, C6spc6_I, C6spc6_O);
            G4SubtractionSolid* geo_C6spc6 = new G4SubtractionSolid("", geo_C6spc6pcon, geo_C6tnl3pcon, transform_A1wal1);
            geo_C6spc6 = new G4SubtractionSolid("", geo_C6spc6pcon, geo_C6tnl4pcon, transform_B1wal1);
            G4LogicalVolume *logi_C6spc6 = new G4LogicalVolume(geo_C6spc6, mat_C6spc6, "logi_C6spc6_name");

            //-   put volume
            setColor(*logi_C6spc6, cC6spc6.getString("Color", "#CCCCCC"));
            new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc6, "phys_C6spc6_name", logi_C5wal5, false, 0);

            //-
            //--------------

      */
      //--------------
      //-   C7mag1

      //-
      //--------------

      //--------------
      //-   C7mag2

      //-
      //--------------

      //--------------
      //-   C7mag3

      //-
      //--------------

      //--------------
      //-   C7mag4

      //-
      //--------------

      //--------------
      //-   C7mag5

      //-
      //--------------

      //--------------
      //-   C7mag6

      //-
      //--------------

      //--------------
      //-   C7mag7

      //-
      //--------------

      //--------------
      //-   C7hld1

      //-
      //--------------

      //--------------
      //-   C7cil1

      //-
      //--------------
    }
  }
}
