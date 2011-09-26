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

      double radius_Tube = 1000 * mm; //mm
      G4Tubs* geo_TubeR = new G4Tubs("geo_TubeR_name", 0, radius_Tube, TMath::Abs(DistanceR), 0, 2*M_PI);
      G4Tubs* geo_TubeL = new G4Tubs("geo_TubeL_name", 0, radius_Tube, TMath::Abs(DistanceL), 0, 2*M_PI);

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
      double A1wal1_rI[A1wal1_num];
      for (int i = 0; i < A1wal1_num; i++)
        { A1wal1_rI[i] = 0.0; }
      //
      double A1wal1_rO[A1wal1_num];
      A1wal1_rO[0] = cA1wal1.getLength("R1") / Unit::mm;
      A1wal1_rO[1] = A1wal1_rO[0];
      A1wal1_rO[2] = cA1wal1.getLength("R2") / Unit::mm;
      A1wal1_rO[3] = A1wal1_rO[2];
      A1wal1_rO[4] = cA1wal1.getLength("R3") / Unit::mm;
      A1wal1_rO[5] = A1wal1_rO[4];
      A1wal1_rO[6] = cA1wal1.getLength("R4") / Unit::mm;
      A1wal1_rO[7] = A1wal1_rO[6];
      //
      string strMat_A1wal1 = cA1wal1.getString("Material");
      G4Material* mat_A1wal1 = Materials::get(strMat_A1wal1);

      G4Transform3D transform_A1wal1 = G4Translate3D(0., 0., 0.);
      transform_A1wal1 = transform_A1wal1 * G4RotateY3D(AngleHER);
      G4Transform3D transform_A1wal1_inv = G4Translate3D(0., 0., 0.);
      transform_A1wal1_inv = transform_A1wal1_inv * G4RotateY3D(-AngleHER);

      //define geometry
      G4Polycone* geo_A1wal1x = new G4Polycone("geo_A1wal1x_name", 0, 2*M_PI, A1wal1_num, A1wal1_Z, A1wal1_rI, A1wal1_rO);
      G4SubtractionSolid* geo_A1wal1 = new G4SubtractionSolid("geo_A1wal1_name", geo_A1wal1x, geo_TubeR, transform_A1wal1_inv);

      G4LogicalVolume *logi_A1wal1 = new G4LogicalVolume(geo_A1wal1, mat_A1wal1, "logi_A1wal1_name");

      //put volume
      setColor(*logi_A1wal1, cA1wal1.getString("Color", "#00CC00"));
      //setVisibility(*logi_A1wal1, false);
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
      double A2spc1_rO[A1wal1_num];
      A2spc1_rO[0] = A1wal1_rO[0] - A2spc1_bpthick;
      A2spc1_rO[1] = A1wal1_rO[1] - A2spc1_bpthick;
      A2spc1_rO[2] = A1wal1_rO[2] - A2spc1_bpthick;
      A2spc1_rO[3] = A1wal1_rO[3] - A2spc1_bpthick;
      A2spc1_rO[4] = A1wal1_rO[4] - A2spc1_bpthick;
      A2spc1_rO[5] = A1wal1_rO[5] - A2spc1_bpthick;
      A2spc1_rO[6] = A1wal1_rO[6] - A2spc1_bpthick;
      A2spc1_rO[7] = A1wal1_rO[7] - A2spc1_bpthick;
      //
      string strMat_A2spc1 = cA2spc1.getString("Material");
      G4Material* mat_A2spc1 = Materials::get(strMat_A2spc1);

      //define geometry
      G4Polycone* geo_A2spc1x = new G4Polycone("geo_A2spc1x_name", 0, 2*M_PI, A1wal1_num, A1wal1_Z, A1wal1_rI, A2spc1_rO);
      G4IntersectionSolid* geo_A2spc1 = new G4IntersectionSolid("geo_A2spc1_name", geo_A2spc1x, geo_A1wal1);

      G4LogicalVolume *logi_A2spc1 = new G4LogicalVolume(geo_A2spc1, mat_A2spc1, "logi_A2spc1_name");

      //put volume
      setColor(*logi_A2spc1, cA2spc1.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_A2spc1, false);
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
      double B1wal1_rI[B1wal1_num];
      for (int i = 0; i < B1wal1_num; i++)
        { B1wal1_rI[i] = 0.0; }
      //
      double B1wal1_rO[B1wal1_num];
      B1wal1_rO[0] = cB1wal1.getLength("R1") / Unit::mm;
      B1wal1_rO[1] = B1wal1_rO[0];
      B1wal1_rO[2] = cB1wal1.getLength("R2") / Unit::mm;
      B1wal1_rO[3] = cB1wal1.getLength("R3") / Unit::mm;
      B1wal1_rO[4] = B1wal1_rO[3];
      B1wal1_rO[5] = cB1wal1.getLength("R4") / Unit::mm;
      B1wal1_rO[6] = cB1wal1.getLength("R5") / Unit::mm;
      B1wal1_rO[7] = B1wal1_rO[6];
      B1wal1_rO[8] = cB1wal1.getLength("R6") / Unit::mm;
      B1wal1_rO[9] = B1wal1_rO[8];
      //
      string strMat_B1wal1 = cB1wal1.getString("Material");
      G4Material* mat_B1wal1 = Materials::get(strMat_B1wal1);

      G4Transform3D transform_B1wal1 = G4Translate3D(0., 0., 0.);
      transform_B1wal1 = transform_B1wal1 * G4RotateY3D(AngleLER);
      G4Transform3D transform_B1wal1_inv = G4Translate3D(0., 0., 0.);
      transform_B1wal1_inv = transform_B1wal1_inv * G4RotateY3D(-AngleLER);

      //define geometry
      G4Polycone* geo_B1wal1x = new G4Polycone("geo_B1wal1x_name", 0, 2*M_PI, B1wal1_num, B1wal1_Z, B1wal1_rI, B1wal1_rO);
      G4SubtractionSolid* geo_B1wal1 = new G4SubtractionSolid("geo_B1wal1_name", geo_B1wal1x, geo_TubeR, transform_B1wal1_inv);

      G4LogicalVolume *logi_B1wal1 = new G4LogicalVolume(geo_B1wal1, mat_B1wal1, "logi_B1wal1_name");

      //put volume
      setColor(*logi_B1wal1, cB1wal1.getString("Color", "#0000CC"));
      //setVisibility(*logi_B1wal1, false);
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
      double B2spc1_rO[B1wal1_num];
      B2spc1_rO[0] = B1wal1_rO[0] - B2spc1_bpthick;
      B2spc1_rO[1] = B1wal1_rO[1] - B2spc1_bpthick;
      B2spc1_rO[2] = B1wal1_rO[2] - B2spc1_bpthick;
      B2spc1_rO[3] = B1wal1_rO[3] - B2spc1_bpthick;
      B2spc1_rO[4] = B1wal1_rO[4] - B2spc1_bpthick;
      B2spc1_rO[5] = B1wal1_rO[5] - B2spc1_bpthick;
      B2spc1_rO[6] = B1wal1_rO[6] - B2spc1_bpthick;
      B2spc1_rO[7] = B1wal1_rO[7] - B2spc1_bpthick;
      B2spc1_rO[8] = B1wal1_rO[8] - B2spc1_bpthick;
      B2spc1_rO[9] = B1wal1_rO[9] - B2spc1_bpthick;
      //
      string strMat_B2spc1 = cB2spc1.getString("Material");
      G4Material* mat_B2spc1 = Materials::get(strMat_B2spc1);

      //define geometry
      G4Polycone* geo_B2spc1x = new G4Polycone("geo_B2spc1x_name", 0, 2*M_PI, B1wal1_num, B1wal1_Z, B1wal1_rI, B2spc1_rO);
      G4IntersectionSolid* geo_B2spc1 = new G4IntersectionSolid("geo_B2spc1", geo_B2spc1x, geo_B1wal1);
      G4LogicalVolume *logi_B2spc1 = new G4LogicalVolume(geo_B2spc1, mat_B2spc1, "logi_B2spc1_name");

      //put volume
      setColor(*logi_B2spc1, cB2spc1.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_B2spc1, false);
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
      double C1wal1_rI[C1wal1_num];
      for (int i = 0; i < C1wal1_num; i++)
        { C1wal1_rI[i] = 0.0; }
      //
      double C1wal1_rO[C1wal1_num];
      C1wal1_rO[0] = cC1wal1.getLength("R1") / Unit::mm;
      C1wal1_rO[1] = cC1wal1.getLength("R2") / Unit::mm;
      C1wal1_rO[2] = C1wal1_rO[1];
      C1wal1_rO[3] = cC1wal1.getLength("R3") / Unit::mm;
      C1wal1_rO[4] = C1wal1_rO[3];
      C1wal1_rO[5] = cC1wal1.getLength("R4") / Unit::mm;
      C1wal1_rO[6] = C1wal1_rO[5];
      C1wal1_rO[7] = cC1wal1.getLength("R5") / Unit::mm;
      C1wal1_rO[8] = C1wal1_rO[7];
      C1wal1_rO[9] = cC1wal1.getLength("R6") / Unit::mm;
      C1wal1_rO[10] = C1wal1_rO[9];
      //
      string strMat_C1wal1 = cC1wal1.getString("Material");
      G4Material* mat_C1wal1 = Materials::get(strMat_C1wal1);

      //define geometry
      G4Polycone* geo_C1wal1xx = new G4Polycone("geo_C1wal1xx_name", 0, 2*M_PI, C1wal1_num, C1wal1_Z, C1wal1_rI, C1wal1_rO);
      G4SubtractionSolid* geo_C1wal1x = new G4SubtractionSolid("geo_C1wal1x_name", geo_C1wal1xx, geo_A1wal1, transform_A1wal1);
      G4SubtractionSolid* geo_C1wal1 = new G4SubtractionSolid("geo_C1wal1_name", geo_C1wal1x, geo_B1wal1, transform_B1wal1);
      G4LogicalVolume *logi_C1wal1 = new G4LogicalVolume(geo_C1wal1, mat_C1wal1, "logi_C1wal1_name");

      //put volume
      setColor(*logi_C1wal1, cC1wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_C1wal1, false);
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
      double C2spc1_rI[C2spc1_num];
      for (int i = 0; i < C2spc1_num; i++)
        { C2spc1_rI[i] = 0.0; }
      //
      double C2spc1_rO[C2spc1_num];
      C2spc1_rO[0] = cC2spc1.getLength("R1") / Unit::mm;
      C2spc1_rO[1] = cC2spc1.getLength("R2") / Unit::mm;
      C2spc1_rO[2] = C2spc1_rO[1];
      C2spc1_rO[3] = cC2spc1.getLength("R3") / Unit::mm;
      C2spc1_rO[4] = C2spc1_rO[3];
      C2spc1_rO[5] = cC2spc1.getLength("R5") / Unit::mm;
      C2spc1_rO[6] = C2spc1_rO[5];
      C2spc1_rO[7] = cC2spc1.getLength("R6") / Unit::mm;
      C2spc1_rO[8] = C2spc1_rO[7];
      //
      string strMat_C2spc1 = cC2spc1.getString("Material");
      G4Material* mat_C2spc1 = Materials::get(strMat_C2spc1);

      //define geometry
      G4Polycone* geo_C2spc1xx = new G4Polycone("geo_C2spc1xx_name", 0, 2*M_PI, C2spc1_num, C2spc1_Z, C2spc1_rI, C2spc1_rO);
      G4SubtractionSolid* geo_C2spc1x = new G4SubtractionSolid("geo_C2spc1x_name", geo_C2spc1xx, geo_A1wal1, transform_A1wal1);
      G4SubtractionSolid* geo_C2spc1 = new G4SubtractionSolid("geo_C2spc1_name", geo_C2spc1x, geo_B1wal1, transform_B1wal1);
      G4LogicalVolume *logi_C2spc1 = new G4LogicalVolume(geo_C2spc1, mat_C2spc1, "logi_C2spc1_name");

      //put volume
      setColor(*logi_C2spc1, cC2spc1.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C2spc1, false);
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
      double C3wal2_rI[C3wal2_num];
      for (int i = 0; i < C3wal2_num; i++)
        { C3wal2_rI[i] = 0.0; }
      //
      double C3wal2_rO[C3wal2_num];
      C3wal2_rO[0] = cC3wal2.getLength("R1") / Unit::mm;
      C3wal2_rO[1] = cC3wal2.getLength("R2") / Unit::mm;
      C3wal2_rO[2] = C3wal2_rO[1];
      C3wal2_rO[3] = cC3wal2.getLength("R3") / Unit::mm;
      C3wal2_rO[4] = C3wal2_rO[3];
      C3wal2_rO[5] = cC3wal2.getLength("R4") / Unit::mm;
      C3wal2_rO[6] = C3wal2_rO[5];
      //
      string strMat_C3wal2 = cC3wal2.getString("Material");
      G4Material* mat_C3wal2 = Materials::get(strMat_C3wal2);

      //define geometry
      G4Polycone* geo_C3wal2xx = new G4Polycone("geo_C3wal2xx_name", 0, 2*M_PI, C3wal2_num, C3wal2_Z, C3wal2_rI, C3wal2_rO);
      G4SubtractionSolid* geo_C3wal2x = new G4SubtractionSolid("geo_C3wal2x_name", geo_C3wal2xx, geo_A1wal1, transform_A1wal1);
      G4SubtractionSolid* geo_C3wal2 = new G4SubtractionSolid("geo_C3wal2_name", geo_C3wal2x, geo_B1wal1, transform_B1wal1);
      G4LogicalVolume *logi_C3wal2 = new G4LogicalVolume(geo_C3wal2, mat_C3wal2, "logi_C3wal2_name");

      //put volume
      setColor(*logi_C3wal2, cC3wal2.getString("Color", "#CC0000"));
      //setVisibility(*logi_C3wal2, false);
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
      double C4spc2_rI[C4spc2_num];
      for (int i = 0; i < C4spc2_num; i++)
        { C4spc2_rI[i] = 0.0; }
      //
      double C4spc2_rO[C4spc2_num];
      C4spc2_rO[0] = cC4spc2.getLength("R1") / Unit::mm;
      C4spc2_rO[1] = cC4spc2.getLength("R2") / Unit::mm;
      C4spc2_rO[2] = C4spc2_rO[1];
      C4spc2_rO[3] = cC4spc2.getLength("R3") / Unit::mm;
      C4spc2_rO[4] = C4spc2_rO[3];
      C4spc2_rO[5] = cC4spc2.getLength("R4") / Unit::mm;
      C4spc2_rO[6] = C4spc2_rO[5];
      //
      string strMat_C4spc2 = cC4spc2.getString("Material");
      G4Material* mat_C4spc2 = Materials::get(strMat_C4spc2);

      //define geometry
      G4Polycone* geo_C4spc2xx = new G4Polycone("geo_C4spc2xx_name", 0, 2*M_PI, C4spc2_num, C4spc2_Z, C4spc2_rI, C4spc2_rO);
      G4SubtractionSolid* geo_C4spc2x = new G4SubtractionSolid("geo_C4spc2x_name", geo_C4spc2xx, geo_A1wal1, transform_A1wal1);
      G4SubtractionSolid* geo_C4spc2 = new G4SubtractionSolid("geo_C4spc2_name", geo_C4spc2x, geo_B1wal1, transform_B1wal1);
      G4LogicalVolume *logi_C4spc2 = new G4LogicalVolume(geo_C4spc2, mat_C4spc2, "logi_C4spc2_name");

      //put volume
      setColor(*logi_C4spc2, cC4spc2.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C4spc2, false);
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
      double C5wal3_rI[C5wal3_num];
      for (int i = 0; i < C5wal3_num; i++)
        { C5wal3_rI[i] = 0.0; }
      //
      double C5wal3_rO[C5wal3_num];
      C5wal3_rO[0] = cC5wal3.getLength("R1") / Unit::mm;
      C5wal3_rO[1] = cC5wal3.getLength("R2") / Unit::mm;
      C5wal3_rO[2] = C5wal3_rO[1];
      C5wal3_rO[3] = cC5wal3.getLength("R3") / Unit::mm;
      C5wal3_rO[4] = C5wal3_rO[3];
      C5wal3_rO[5] = cC5wal3.getLength("R4") / Unit::mm;
      C5wal3_rO[6] = C5wal3_rO[5];
      C5wal3_rO[7] = cC5wal3.getLength("R5") / Unit::mm;
      //
      string strMat_C5wal3 = cC5wal3.getString("Material");
      G4Material* mat_C5wal3 = Materials::get(strMat_C5wal3);

      //define geometry
      G4Polycone* geo_C5wal3xx = new G4Polycone("geo_C5wal3xx_name", 0, 2*M_PI, C5wal3_num, C5wal3_Z, C5wal3_rI, C5wal3_rO);
      G4SubtractionSolid* geo_C5wal3x = new G4SubtractionSolid("geo_C5wal3x_name", geo_C5wal3xx, geo_A1wal1, transform_A1wal1);
      G4SubtractionSolid* geo_C5wal3 = new G4SubtractionSolid("geo_C5wal3_name", geo_C5wal3x, geo_B1wal1, transform_B1wal1);
      G4LogicalVolume *logi_C5wal3 = new G4LogicalVolume(geo_C5wal3, mat_C5wal3, "logi_C5wal3_name");

      //put volume
      setColor(*logi_C5wal3, cC5wal3.getString("Color", "#CC0000"));
      //setVisibility(*logi_C5wal3, false);
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
      double C5wal5_rI[C5wal5_num];
      for (int i = 0; i < C5wal5_num; i++)
        { C5wal5_rI[i] = 0.0; }
      //
      double C5wal5_rO[C5wal5_num];
      C5wal5_rO[0] = cC5wal5.getLength("R1") / Unit::mm;
      C5wal5_rO[1] = C5wal5_rO[0];
      C5wal5_rO[2] = cC5wal5.getLength("R2") / Unit::mm;
      //
      string strMat_C5wal5 = cC5wal5.getString("Material");
      G4Material* mat_C5wal5 = Materials::get(strMat_C5wal5);

      //define geometry
      G4Polycone* geo_C5wal5xx = new G4Polycone("geo_C5wal5xx_name", 0, 2*M_PI, C5wal5_num, C5wal5_Z, C5wal5_rI, C5wal5_rO);
      G4SubtractionSolid* geo_C5wal5x = new G4SubtractionSolid("geo_C5wal5x_name", geo_C5wal5xx, geo_A1wal1, transform_A1wal1);
      G4SubtractionSolid* geo_C5wal5 = new G4SubtractionSolid("geo_C5wal5_name", geo_C5wal5x, geo_B1wal1, transform_B1wal1);
      G4LogicalVolume *logi_C5wal5 = new G4LogicalVolume(geo_C5wal5, mat_C5wal5, "logi_C5wal5_name");

      //put volume
      setColor(*logi_C5wal5, cC5wal5.getString("Color", "#CC0000"));
      //setVisibility(*logi_C5wal5, false);
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
      double C6spc4_rI[C6spc4_num];
      for (int i = 0; i < C6spc4_num; i++)
        { C6spc4_rI[i] = 0.0; }
      //
      double C6spc4_rO[C6spc4_num];
      C6spc4_rO[0] = cC6spc4.getLength("R1") / Unit::mm;
      C6spc4_rO[1] = C6spc4_rO[0];
      C6spc4_rO[2] = cC6spc4.getLength("R2") / Unit::mm;
      C6spc4_rO[3] = C6spc4_rO[2];
      C6spc4_rO[4] = cC6spc4.getLength("R3") / Unit::mm;
      C6spc4_rO[5] = C6spc4_rO[4];
      //
      string strMat_C6spc4 = cC6spc4.getString("Material");
      G4Material* mat_C6spc4 = Materials::get(strMat_C6spc4);

      //define geometry
      G4Polycone* geo_C6spc4xx = new G4Polycone("geo_C6spc4xx_name", 0, 2*M_PI, C6spc4_num, C6spc4_Z, C6spc4_rI, C6spc4_rO);
      G4SubtractionSolid* geo_C6spc4x = new G4SubtractionSolid("geo_C6spc4x_name", geo_C6spc4xx, geo_A1wal1, transform_A1wal1);
      G4IntersectionSolid* geo_C6spc4 = new G4IntersectionSolid("geo_C6spc4_name", geo_C6spc4x, geo_C5wal3);

      G4LogicalVolume *logi_C6spc4 = new G4LogicalVolume(geo_C6spc4, mat_C6spc4, "logi_C6spc4_name");

      //put volume
      setColor(*logi_C6spc4, cC6spc4.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C6spc4, false);
      new G4PVPlacement(transform_A1wal1, logi_C6spc4, "phys_C6spc4_name", logi_C5wal3, false, 0);

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
      double C6spc5_rI[C6spc5_num];
      for (int i = 0; i < C6spc5_num; i++)
        { C6spc5_rI[i] = 0.0; }
      //
      double C6spc5_rO[C6spc5_num];
      C6spc5_rO[0] = cC6spc5.getLength("R1") / Unit::mm;
      C6spc5_rO[1] = C6spc5_rO[0];
      C6spc5_rO[2] = cC6spc5.getLength("R2") / Unit::mm;
      C6spc5_rO[3] = cC6spc5.getLength("R3") / Unit::mm;
      C6spc5_rO[4] = C6spc5_rO[2];
      C6spc5_rO[5] = cC6spc5.getLength("R4") / Unit::mm;
      C6spc5_rO[6] = cC6spc5.getLength("R5") / Unit::mm;
      C6spc5_rO[7] = C6spc5_rO[4];
      //
      string strMat_C6spc5 = cC6spc5.getString("Material");
      G4Material* mat_C6spc5 = Materials::get(strMat_C6spc5);

      //define geometry
      G4Polycone* geo_C6spc5xx = new G4Polycone("geo_C6spc5xx_name", 0, 2*M_PI, C6spc5_num, C6spc5_Z, C6spc5_rI, C6spc5_rO);
      G4SubtractionSolid* geo_C6spc5x = new G4SubtractionSolid("geo_C6spc5x_name", geo_C6spc5xx, geo_B1wal1, transform_B1wal1);
      G4IntersectionSolid* geo_C6spc5 = new G4IntersectionSolid("geo_C6spc5_name", geo_C6spc5x, geo_C5wal3);

      G4LogicalVolume *logi_C6spc5 = new G4LogicalVolume(geo_C6spc5, mat_C6spc5, "logi_C6spc5_name");

      //put volume
      setColor(*logi_C6spc5, cC6spc5.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C6spc5, false);
      new G4PVPlacement(transform_B1wal1, logi_C6spc5, "phys_C6spc5_name", logi_C5wal3, false, 0);

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
      double C6spc7_rI[C6spc7_num];
      for (int i = 0; i < C6spc7_num; i++)
        { C6spc7_rI[i] = 0.0; }
      //
      double C6spc7_rO[C6spc7_num];
      C6spc7_rO[0] = cC6spc7.getLength("R1") / Unit::mm;
      C6spc7_rO[1] = C6spc7_rO[0];
      //
      string strMat_C6spc7 = cC6spc7.getString("Material");
      G4Material* mat_C6spc7 = Materials::get(strMat_C6spc7);

      //define geometry
      G4Polycone* geo_C6spc7xx = new G4Polycone("geo_C6spc7xx_name", 0, 2*M_PI, C6spc7_num, C6spc7_Z, C6spc7_rI, C6spc7_rO);
      G4SubtractionSolid* geo_C6spc7x = new G4SubtractionSolid("geo_C6spc7x_name", geo_C6spc7xx, geo_A1wal1, transform_A1wal1);
      G4IntersectionSolid* geo_C6spc7 = new G4IntersectionSolid("geo_C6spc7_name", geo_C6spc7x, geo_C5wal5);
      G4LogicalVolume *logi_C6spc7 = new G4LogicalVolume(geo_C6spc7, mat_C6spc7, "logi_C6spc7_name");

      //put volume
      setColor(*logi_C6spc7, cC6spc7.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C6spc7, false);
      new G4PVPlacement(transform_A1wal1, logi_C6spc7, "phys_C6spc7_name", logi_C5wal5, false, 0);

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
      double C6spc8_rI[C6spc8_num];
      for (int i = 0; i < C6spc8_num; i++)
        { C6spc8_rI[i] = 0.0; }
      //
      double C6spc8_rO[C6spc8_num];
      C6spc8_rO[0] = cC6spc8.getLength("R1") / Unit::mm;
      C6spc8_rO[1] = C6spc8_rO[0];
      //
      string strMat_C6spc8 = cC6spc8.getString("Material");
      G4Material* mat_C6spc8 = Materials::get(strMat_C6spc8);

      //define geometry
      G4Polycone* geo_C6spc8xx = new G4Polycone("geo_C6spc8xx_name", 0, 2*M_PI, C6spc8_num, C6spc8_Z, C6spc8_rI, C6spc8_rO);
      G4SubtractionSolid* geo_C6spc8x = new G4SubtractionSolid("geo_C6spc8x_name", geo_C6spc8xx, geo_B1wal1, transform_B1wal1);
      G4IntersectionSolid* geo_C6spc8 = new G4IntersectionSolid("geo_C6spc8_name", geo_C6spc8x, geo_C5wal5);

      G4LogicalVolume *logi_C6spc8 = new G4LogicalVolume(geo_C6spc8, mat_C6spc8, "logi_C6spc8_name");

      //put volume
      setColor(*logi_C6spc8, cC6spc8.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C6spc8, false);
      new G4PVPlacement(transform_B1wal1, logi_C6spc8, "phys_C6spc8_name", logi_C5wal5, false, 0);

      //-
      //--------------


      //--------------
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
      double C6tnl1_rI[C6tnl1_num];
      for (int i = 0; i < C6tnl1_num; i++)
        { C6tnl1_rI[i] = 0.0; }
      //
      double C6tnl1_rO[C6tnl1_num];
      C6tnl1_rO[0] = cC6tnl1.getLength("R1") / Unit::mm;
      C6tnl1_rO[1] = C6tnl1_rO[0];
      C6tnl1_rO[2] = cC6tnl1.getLength("R2") / Unit::mm;
      C6tnl1_rO[3] = C6tnl1_rO[2];
      C6tnl1_rO[4] = cC6tnl1.getLength("R3") / Unit::mm;
      C6tnl1_rO[5] = C6tnl1_rO[4];
      //define geometry
      G4Polycone* geo_C6tnl1 = new G4Polycone("geo_C6tnl1_name", 0, 2*M_PI, C6tnl1_num, C6tnl1_Z, C6tnl1_rI, C6tnl1_rO);

      //-
      //--------------

      //--------------
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
      double C6tnl2_rI[C6tnl2_num];
      for (int i = 0; i < C6tnl2_num; i++)
        { C6tnl2_rI[i] = 0.0; }
      //
      double C6tnl2_rO[C6tnl2_num];
      C6tnl2_rO[0] = cC6tnl2.getLength("R1") / Unit::mm;
      C6tnl2_rO[1] = C6tnl2_rO[0];
      C6tnl2_rO[2] = cC6tnl2.getLength("R2") / Unit::mm;
      C6tnl2_rO[3] = cC6tnl2.getLength("R3") / Unit::mm;
      C6tnl2_rO[4] = C6tnl2_rO[3];
      C6tnl2_rO[5] = cC6tnl2.getLength("R4") / Unit::mm;
      C6tnl2_rO[6] = cC6tnl2.getLength("R5") / Unit::mm;
      C6tnl2_rO[7] = C6tnl2_rO[6];
      //define geometry
      G4Polycone* geo_C6tnl2 = new G4Polycone("geo_C6tnl2_name", 0, 2*M_PI, C6tnl2_num, C6tnl2_Z, C6tnl2_rI, C6tnl2_rO);

      //-
      //--------------

      //--------------
      //- C6tnl3

      //get parameters from .xml file
      GearDir cC6tnl3(content, "C6tnl3/");
      //
      const int C6tnl3_num = 2;
      //
      double C6tnl3_Z[C6tnl3_num];
      C6tnl3_Z[0] = cC6tnl3.getLength("D1") / Unit::mm;
      C6tnl3_Z[1] = C6tnl3_Z[0] + cC6tnl3.getLength("L1") / Unit::mm;
      //
      double C6tnl3_rI[C6tnl3_num];
      for (int i = 0; i < C6tnl3_num; i++)
        { C6tnl3_rI[i] = 0.0; }
      //
      double C6tnl3_rO[C6tnl3_num];
      C6tnl3_rO[0] = cC6tnl3.getLength("R1") / Unit::mm;
      C6tnl3_rO[1] = C6tnl3_rO[0];
      //define geometry
      G4Polycone* geo_C6tnl3 = new G4Polycone("geo_C6tnl3_name", 0, 2*M_PI, C6tnl3_num, C6tnl3_Z, C6tnl3_rI, C6tnl3_rO);

      //-
      //--------------

      //--------------
      //- C6tnl4

      //get parameters from .xml file
      GearDir cC6tnl4(content, "C6tnl4/");
      //
      const int C6tnl4_num = 2;
      //
      double C6tnl4_Z[C6tnl4_num];
      C6tnl4_Z[0] = cC6tnl4.getLength("D1") / Unit::mm;
      C6tnl4_Z[1] = C6tnl4_Z[0] + cC6tnl4.getLength("L1") / Unit::mm;
      //
      double C6tnl4_rI[C6tnl4_num];
      for (int i = 0; i < C6tnl4_num; i++)
        { C6tnl4_rI[i] = 0.0; }
      //
      double C6tnl4_rO[C6tnl4_num];
      C6tnl4_rO[0] = cC6tnl4.getLength("R1") / Unit::mm;
      C6tnl4_rO[1] = C6tnl4_rO[0];
      //define geometry
      G4Polycone* geo_C6tnl4 = new G4Polycone("geo_C6tnl4_name", 0, 2*M_PI, C6tnl4_num, C6tnl4_Z, C6tnl4_rI, C6tnl4_rO);

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
      double C6spc3_rI[C6spc3_num];
      for (int i = 0; i < C6spc3_num; i++)
        { C6spc3_rI[i] = 0.0; }
      //
      double C6spc3_rO[C6spc3_num];
      C6spc3_rO[0] = cC6spc3.getLength("R1") / Unit::mm;
      C6spc3_rO[1] = cC6spc3.getLength("R2") / Unit::mm;
      C6spc3_rO[2] = C6spc3_rO[1];
      C6spc3_rO[3] = cC6spc3.getLength("R3") / Unit::mm;
      C6spc3_rO[4] = C6spc3_rO[3];
      C6spc3_rO[5] = cC6spc3.getLength("R4") / Unit::mm;
      C6spc3_rO[6] = C6spc3_rO[5];
      //
      string strMat_C6spc3 = cC6spc3.getString("Material");
      G4Material* mat_C6spc3 = Materials::get(strMat_C6spc3);

      for (int i = 0; i < C6spc3_num; i++) printf("%f %f %f\n", C6spc3_Z[i], C6spc3_rI[i], C6spc3_rO[i]);

      //define geometry
      G4Polycone* geo_C6spc3xx = new G4Polycone("geo_C6spc3xx_name", 0, 2*M_PI, C6spc3_num, C6spc3_Z, C6spc3_rI, C6spc3_rO);
      G4SubtractionSolid* geo_C6spc3x = new G4SubtractionSolid("geo_C6spc3x_name", geo_C6spc3xx, geo_C6tnl1, transform_A1wal1);
      G4SubtractionSolid* geo_C6spc3 = new G4SubtractionSolid("geo_C6spc3_name", geo_C6spc3x, geo_C6tnl2, transform_B1wal1);

      G4LogicalVolume *logi_C6spc3 = new G4LogicalVolume(geo_C6spc3, mat_C6spc3, "logi_C6spc3_name");

      //put volume
      setColor(*logi_C6spc3, cC6spc3.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C6spc3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc3, "phys_C6spc3_name", logi_C5wal3, false, 0);

      //-
      //--------------

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
      double C6spc6_rI[C6spc6_num];
      for (int i = 0; i < C6spc6_num; i++)
        { C6spc6_rI[i] = 0.0; }
      //
      double C6spc6_rO[C6spc6_num];
      C6spc6_rO[0] = cC6spc6.getLength("R1") / Unit::mm;
      C6spc6_rO[1] = C6spc6_rO[0];
      C6spc6_rO[2] = cC6spc6.getLength("R2") / Unit::mm;
      //
      string strMat_C6spc6 = cC6spc6.getString("Material");
      G4Material* mat_C6spc6 = Materials::get(strMat_C6spc6);

      //define geometry
      G4Polycone* geo_C6spc6xx = new G4Polycone("geo_C6spc6xx_name", 0, 2*M_PI, C6spc6_num, C6spc6_Z, C6spc6_rI, C6spc6_rO);
      G4SubtractionSolid* geo_C6spc6x = new G4SubtractionSolid("geo_C6spc6x_name", geo_C6spc6xx, geo_C6tnl3, transform_A1wal1);
      G4SubtractionSolid* geo_C6spc6 = new G4SubtractionSolid("geo_C6spc6_nam", geo_C6spc6x, geo_C6tnl4, transform_B1wal1);

      G4LogicalVolume *logi_C6spc6 = new G4LogicalVolume(geo_C6spc6, mat_C6spc6, "logi_C6spc6_name");

      //put volume
      setColor(*logi_C6spc6, cC6spc6.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_C6spc6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc6, "phys_C6spc6_name", logi_C5wal5, false, 0);

      //-
      //--------------


      //--------------
      //-   C7mag1

      //get parameters from .xml file
      GearDir cC7mag1(content, "C7mag1/");
      //
      const int C7mag1_num = 2;
      //
      double C7mag1_Z[C7mag1_num];
      C7mag1_Z[0] = cC7mag1.getLength("D1") / Unit::mm;
      C7mag1_Z[1] = C7mag1_Z[0] + cC7mag1.getLength("L1") / Unit::mm;
      //
      double C7mag1_rI[C7mag1_num];
      for (int i = 0; i < C7mag1_num; i++)
        { C7mag1_rI[i] = 0.0; }
      //
      double C7mag1_rO[C7mag1_num];
      C7mag1_rO[0] = cC7mag1.getLength("O1") / Unit::mm;
      C7mag1_rO[1] = C7mag1_rO[0];
      //
      string strMat_C7mag1 = cC7mag1.getString("Material");
      G4Material* mat_C7mag1 = Materials::get(strMat_C7mag1);

      //define geometry
      G4Polycone* geo_C7mag1x = new G4Polycone("geo_C7mag1x_name", 0, 2*M_PI, C7mag1_num, C7mag1_Z, C7mag1_rI, C7mag1_rO);
      G4IntersectionSolid* geo_C7mag1 = new G4IntersectionSolid("geo_C7mag1_name", geo_C6spc3, geo_C7mag1x, transform_A1wal1);

      G4LogicalVolume *logi_C7mag1 = new G4LogicalVolume(geo_C7mag1, mat_C7mag1, "logi_C7mag1_name");

      //put volume
      setColor(*logi_C7mag1, cC7mag1.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag1, "phys_C7mag1_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7mag2

      //get parameters from .xml file
      GearDir cC7mag2(content, "C7mag2/");
      //
      const int C7mag2_num = 2;
      //
      double C7mag2_Z[C7mag2_num];
      C7mag2_Z[0] = C6spc4_Z[2];
      C7mag2_Z[1] = C7mag2_Z[0] + cC7mag2.getLength("L1") / Unit::mm;
      //
      double C7mag2_rI[C7mag2_num];
      for (int i = 0; i < C7mag2_num; i++)
        { C7mag2_rI[i] = 0.0; }
      //
      double C7mag2_rO[C7mag2_num];
      C7mag2_rO[0] = cC7mag2.getLength("O1") / Unit::mm;
      C7mag2_rO[1] = C7mag2_rO[0];
      //
      string strMat_C7mag2 = cC7mag2.getString("Material");
      G4Material* mat_C7mag2 = Materials::get(strMat_C7mag2);

      //define geometry
      G4Polycone* geo_C7mag2x = new G4Polycone("geo_C7mag2x_name", 0, 2*M_PI, C7mag2_num, C7mag2_Z, C7mag2_rI, C7mag2_rO);
      G4IntersectionSolid* geo_C7mag2 = new G4IntersectionSolid("geo_C7mag2_name", geo_C6spc3, geo_C7mag2x, transform_A1wal1);

      G4LogicalVolume *logi_C7mag2 = new G4LogicalVolume(geo_C7mag2, mat_C7mag2, "logi_C7mag2_name");

      //put volume
      setColor(*logi_C7mag2, cC7mag2.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag2, "phys_C7mag2_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7mag3

      //get parameters from .xml file
      GearDir cC7mag3(content, "C7mag3/");
      //
      const int C7mag3_num = 2;
      //
      double C7mag3_Z[C7mag3_num];
      C7mag3_Z[0] = C6spc4_Z[4];
      C7mag3_Z[1] = C7mag3_Z[0] + cC7mag3.getLength("L1") / Unit::mm;
      //
      double C7mag3_rI[C7mag3_num];
      for (int i = 0; i < C7mag3_num; i++)
        { C7mag3_rI[i] = 0.0; }
      //
      double C7mag3_rO[C7mag3_num];
      C7mag3_rO[0] = cC7mag3.getLength("O1") / Unit::mm;
      C7mag3_rO[1] = C7mag3_rO[0];
      //
      string strMat_C7mag3 = cC7mag3.getString("Material");
      G4Material* mat_C7mag3 = Materials::get(strMat_C7mag3);

      //define geometry
      G4Polycone* geo_C7mag3x = new G4Polycone("geo_C7mag3_name", 0, 2*M_PI, C7mag3_num, C7mag3_Z, C7mag3_rI, C7mag3_rO);
      G4IntersectionSolid* geo_C7mag3 = new G4IntersectionSolid("geo_C7mag3_name", geo_C6spc3, geo_C7mag3x, transform_A1wal1);

      G4LogicalVolume *logi_C7mag3 = new G4LogicalVolume(geo_C7mag3, mat_C7mag3, "logi_C7mag3_name");

      //put volume
      setColor(*logi_C7mag3, cC7mag3.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag3, "phys_C7mag3_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7mag4

      //get parameters from .xml file
      GearDir cC7mag4(content, "C7mag4/");
      //
      const int C7mag4_num = 2;
      //
      double C7mag4_Z[C7mag4_num];
      C7mag4_Z[0] = cC7mag4.getLength("D1") / Unit::mm;
      C7mag4_Z[1] = C7mag4_Z[0] + cC7mag4.getLength("L1") / Unit::mm;
      //
      double C7mag4_rI[C7mag4_num];
      for (int i = 0; i < C7mag4_num; i++)
        { C7mag4_rI[i] = 0.0; }
      //
      double C7mag4_rO[C7mag4_num];
      C7mag4_rO[0] = cC7mag4.getLength("O1") / Unit::mm;
      C7mag4_rO[1] = C7mag4_rO[0];
      //
      string strMat_C7mag4 = cC7mag4.getString("Material");
      G4Material* mat_C7mag4 = Materials::get(strMat_C7mag4);

      //define geometry
      G4Polycone* geo_C7mag4x = new G4Polycone("geo_C7mag4x_name", 0, 2*M_PI, C7mag4_num, C7mag4_Z, C7mag4_rI, C7mag4_rO);
      G4IntersectionSolid* geo_C7mag4 = new G4IntersectionSolid("geo_C7mag4_name", geo_C6spc3, geo_C7mag4x, transform_B1wal1);

      G4LogicalVolume *logi_C7mag4 = new G4LogicalVolume(geo_C7mag4, mat_C7mag4, "logi_C7mag4_name");

      //put volume
      setColor(*logi_C7mag4, cC7mag4.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag4, "phys_C7mag4_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7mag5

      //get parameters from .xml file
      GearDir cC7mag5(content, "C7mag5/");
      //
      const int C7mag5_num = 2;
      //
      double C7mag5_Z[C7mag5_num];
      C7mag5_Z[0] = C6spc5_Z[3];
      C7mag5_Z[1] = C7mag5_Z[0] + cC7mag5.getLength("L1") / Unit::mm;
      //
      double C7mag5_rI[C7mag5_num];
      for (int i = 0; i < C7mag5_num; i++)
        { C7mag5_rI[i] = 0.0; }
      //
      double C7mag5_rO[C7mag5_num];
      C7mag5_rO[0] = cC7mag5.getLength("O1") / Unit::mm;
      C7mag5_rO[1] = C7mag5_rO[0];
      //
      string strMat_C7mag5 = cC7mag5.getString("Material");
      G4Material* mat_C7mag5 = Materials::get(strMat_C7mag5);

      //define geometry
      G4Polycone* geo_C7mag5x = new G4Polycone("geo_C7mag5x_name", 0, 2*M_PI, C7mag5_num, C7mag5_Z, C7mag5_rI, C7mag5_rO);
      G4IntersectionSolid* geo_C7mag5 = new G4IntersectionSolid("geo_C7mag5_name", geo_C6spc3, geo_C7mag5x, transform_B1wal1);

      G4LogicalVolume *logi_C7mag5 = new G4LogicalVolume(geo_C7mag5, mat_C7mag5, "logi_C7mag5_name");

      //put volume
      setColor(*logi_C7mag5, cC7mag5.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag5, "phys_C7mag5_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7mag6

      //get parameters from .xml file
      GearDir cC7mag6(content, "C7mag6/");
      //
      const int C7mag6_num = 2;
      //
      double C7mag6_Z[C7mag6_num];
      C7mag6_Z[0] = C6spc5_Z[6];
      C7mag6_Z[1] = C7mag6_Z[0] + cC7mag6.getLength("L1") / Unit::mm;
      //
      double C7mag6_rI[C7mag6_num];
      for (int i = 0; i < C7mag6_num; i++)
        { C7mag6_rI[i] = 0.0; }
      //
      double C7mag6_rO[C7mag6_num];
      C7mag6_rO[0] = cC7mag6.getLength("O1") / Unit::mm;
      C7mag6_rO[1] = C7mag6_rO[0];
      //
      string strMat_C7mag6 = cC7mag6.getString("Material");
      G4Material* mat_C7mag6 = Materials::get(strMat_C7mag6);

      //define geometry
      G4Polycone* geo_C7mag6x = new G4Polycone("geo_C7mag6x_name", 0, 2*M_PI, C7mag6_num, C7mag6_Z, C7mag6_rI, C7mag6_rO);
      G4IntersectionSolid* geo_C7mag6 = new G4IntersectionSolid("geo_C7mag6_name", geo_C6spc3, geo_C7mag6x, transform_B1wal1);

      G4LogicalVolume *logi_C7mag6 = new G4LogicalVolume(geo_C7mag6, mat_C7mag6, "logi_C7mag6_name");

      //put volume
      setColor(*logi_C7mag6, cC7mag6.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag6, "phys_C7mag6_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7mag7

      //get parameters from .xml file
      GearDir cC7mag7(content, "C7mag7/");
      //
      const int C7mag7_num = 2;
      //
      double C7mag7_Z[C7mag7_num];
      C7mag7_Z[0] = cC7mag7.getLength("D1") / Unit::mm;
      C7mag7_Z[1] = C7mag7_Z[0] + cC7mag7.getLength("L1") / Unit::mm;
      //
      double C7mag7_rI[C7mag7_num];
      for (int i = 0; i < C7mag7_num; i++)
        { C7mag7_rI[i] = 0.0; }
      //
      double C7mag7_rO[C7mag7_num];
      C7mag7_rO[0] = cC7mag7.getLength("O1") / Unit::mm;
      C7mag7_rO[1] = C7mag7_rO[0];
      //
      string strMat_C7mag7 = cC7mag7.getString("Material");
      G4Material* mat_C7mag7 = Materials::get(strMat_C7mag7);

      //define geometry
      G4Polycone* geo_C7mag7x = new G4Polycone("geo_C7mag7x_name", 0, 2*M_PI, C7mag7_num, C7mag7_Z, C7mag7_rI, C7mag7_rO);
      G4IntersectionSolid* geo_C7mag7 = new G4IntersectionSolid("geo_C7mag7_name", geo_C6spc6, geo_C7mag7x, transform_A1wal1);

      G4LogicalVolume *logi_C7mag7 = new G4LogicalVolume(geo_C7mag7, mat_C7mag7, "logi_C7mag7_name");

      //put volume
      setColor(*logi_C7mag7, cC7mag7.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7mag7, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7mag7, "phys_C7mag7_name", logi_C6spc6, false, 0);

      //-
      //--------------

      //--------------
      //-   C7hld1

      //get parameters from .xml file
      GearDir cC7hld1(content, "C7hld1/");
      //
      double C7hld1_T[16];
      for (int tmpn = 0; tmpn < 16; tmpn++) {
        C7hld1_T[tmpn] = cC7hld1.getLength((boost::format("T%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double C7hld1_D[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        C7hld1_D[tmpn] = cC7hld1.getLength((boost::format("D%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double C7hld1_I[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        C7hld1_I[tmpn] = cC7hld1.getLength((boost::format("I%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double C7hld1_R[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        C7hld1_R[tmpn] = cC7hld1.getLength((boost::format("R%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double C7hld1_O[16];
      for (int tmpn = 0; tmpn < 16; tmpn++) {
        C7hld1_O[tmpn] = cC7hld1.getLength((boost::format("O%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }

      const int C7hld1_num = 62;
      //
      double C7hld1_Z[C7hld1_num];
      C7hld1_Z[0] = C6spc3_Z[0];
      C7hld1_Z[1] = C7hld1_Z[0] + C7hld1_T[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        C7hld1_Z[4*tmpn + 2] = C7hld1_Z[4*tmpn + 1];
        C7hld1_Z[4*tmpn + 3] = C7hld1_Z[4*tmpn + 2] + C7hld1_D[tmpn];
        C7hld1_Z[4*tmpn + 4] = C7hld1_Z[4*tmpn + 3];
        C7hld1_Z[4*tmpn + 5] = C7hld1_Z[4*tmpn + 4] + C7hld1_T[tmpn+1];
      }
      //
      double C7hld1_rI[C7hld1_num];
      C7hld1_rI[0] = C7hld1_I[0];
      C7hld1_rI[1] = C7hld1_I[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        C7hld1_rI[4*tmpn + 2] = C7hld1_I[tmpn];
        C7hld1_rI[4*tmpn + 3] = C7hld1_I[tmpn];
        C7hld1_rI[4*tmpn + 4] = C7hld1_I[tmpn];
        C7hld1_rI[4*tmpn + 5] = C7hld1_I[tmpn];
      }
      //
      double C7hld1_rO[C7hld1_num];
      C7hld1_rO[0] = C7hld1_O[0];
      C7hld1_rO[1] = C7hld1_O[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        C7hld1_rO[4*tmpn + 2] = C7hld1_R[tmpn];
        C7hld1_rO[4*tmpn + 3] = C7hld1_R[tmpn];
        C7hld1_rO[4*tmpn + 4] = C7hld1_O[tmpn + 1];
        C7hld1_rO[4*tmpn + 5] = C7hld1_O[tmpn + 1];
      }
      //
      string strMat_C7hld1 = cC7hld1.getString("Material");
      G4Material* mat_C7hld1 = Materials::get(strMat_C7hld1);

      for (int i = 0; i < C7hld1_num; i++) printf("%f %f %f\n", C7hld1_Z[i], C7hld1_rI[i], C7hld1_rO[i]);

      //define geometry
      G4Polycone* geo_C7hld1x7 = new G4Polycone("geo_C7hld1x7_name", 0, 2*M_PI, C7hld1_num, C7hld1_Z, C7hld1_rI, C7hld1_rO);
      //G4SubtractionSolid* geo_C7hld1x6 = new G4SubtractionSolid("geo_C7hld1x6_name", geo_C7hld1x7, geo_C7mag6, transform_B1wal1);
      //G4SubtractionSolid* geo_C7hld1x5 = new G4SubtractionSolid("geo_C7hld1x5_name", geo_C7hld1x6, geo_C7mag5, transform_B1wal1);
      //G4SubtractionSolid* geo_C7hld1x4 = new G4SubtractionSolid("geo_C7hld1x4_name", geo_C7hld1x5, geo_C7mag4, transform_B1wal1);
      //G4SubtractionSolid* geo_C7hld1x3 = new G4SubtractionSolid("geo_C7hld1x3_name", geo_C7hld1x4, geo_C7mag3, transform_A1wal1);
      //G4SubtractionSolid* geo_C7hld1x2 = new G4SubtractionSolid("geo_C7hld1x2_name", geo_C7hld1x3, geo_C7mag2, transform_A1wal1);
      //G4SubtractionSolid* geo_C7hld1x1 = new G4SubtractionSolid("geo_C7hld1x1_name", geo_C7hld1x2, geo_C7mag1, transform_A1wal1);
      G4IntersectionSolid* geo_C7hld1 = new G4IntersectionSolid("geo_C7hld1_name", geo_C7hld1x7, geo_C6spc3);

      G4LogicalVolume *logi_C7hld1 = new G4LogicalVolume(geo_C7hld1, mat_C7hld1, "logi_C7hld1_name");

      //put volume
      setColor(*logi_C7hld1, cC7hld1.getString("Color", "#CC0000"));
      //setVisibility(*logi_C7hld1, false);

      //Now C7hld1 goes under C7cil1. C7hld1 will be placed after C7cil1.
      //new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7hld1, "phys_C7hld1_name", logi_C6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   C7cil1

      //get parameters from .xml file
      GearDir cC7cil1(content, "C7cil1/");
      //
      double C7cil1_L[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        C7cil1_L[tmpn] = cC7cil1.getLength((boost::format("L%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }

      double C7cil1_rO[C7hld1_num];
      C7cil1_rO[0] = C7hld1_O[0];
      C7cil1_rO[1] = C7hld1_O[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        C7cil1_rO[4*tmpn + 2] = C7hld1_R[tmpn] + C7cil1_L[tmpn];
        C7cil1_rO[4*tmpn + 3] = C7hld1_R[tmpn] + C7cil1_L[tmpn];
        C7cil1_rO[4*tmpn + 4] = C7hld1_O[tmpn + 1];
        C7cil1_rO[4*tmpn + 5] = C7hld1_O[tmpn + 1];
      }
      //
      string strMat_C7cil1 = cC7cil1.getString("Material");
      G4Material* mat_C7cil1 = Materials::get(strMat_C7cil1);

      for (int i = 0; i < C7hld1_num; i++) printf("%f %f %f\n", C7hld1_Z[i], C7hld1_rI[i], C7cil1_rO[i]);

      //define geometry
      G4Polycone* geo_C7cil1x7 = new G4Polycone("geo_C7cil1x7_name", 0, 2*M_PI, C7hld1_num, C7hld1_Z, C7hld1_rI, C7cil1_rO);
      //G4SubtractionSolid* geo_C7cil1x6 = new G4SubtractionSolid("geo_C7cil1x6_name", geo_C7cil1x7, geo_C7mag6, transform_B1wal1);
      //G4SubtractionSolid* geo_C7cil1x5 = new G4SubtractionSolid("geo_C7cil1x5_name", geo_C7cil1x6, geo_C7mag5, transform_B1wal1);
      //G4SubtractionSolid* geo_C7cil1x4 = new G4SubtractionSolid("geo_C7cil1x4_name", geo_C7cil1x5, geo_C7mag4, transform_B1wal1);
      //G4SubtractionSolid* geo_C7cil1x3 = new G4SubtractionSolid("geo_C7cil1x3_name", geo_C7cil1x4, geo_C7mag3, transform_A1wal1);
      //G4SubtractionSolid* geo_C7cil1x2 = new G4SubtractionSolid("geo_C7cil1x2_name", geo_C7cil1x3, geo_C7mag2, transform_A1wal1);
      //G4SubtractionSolid* geo_C7cil1x1 = new G4SubtractionSolid("geo_C7cil1x1_name", geo_C7cil1x2, geo_C7mag1, transform_A1wal1);
      //G4SubtractionSolid* geo_C7cil1x = new G4SubtractionSolid("geo_C7cil1x_name", geo_C7cil1x7, geo_C7hld1);
      G4IntersectionSolid* geo_C7cil1 = new G4IntersectionSolid("geo_C7cil1_name", geo_C7cil1x7, geo_C6spc3);

      G4LogicalVolume *logi_C7cil1 = new G4LogicalVolume(geo_C7cil1, mat_C7cil1, "logi_C7cil1_name");

      //put volume
      setColor(*logi_C7cil1, cC7cil1.getString("Color", "#CCCC00"));
      //setVisibility(*logi_C7cil1, false);
      //new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7cil1, "phys_C7cil1_name", &topVolume, false, 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7cil1, "phys_C7cil1_name", logi_C6spc3, false, 0);

      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7hld1, "phys_C7hld1_name", logi_C7cil1, false, 0);

      //-
      //--------------


      //--------------
      //-   D1wal1

      //get parameters from .xml file
      GearDir cD1wal1(content, "D1wal1/");
      //
      const int D1wal1_num = 8;
      //
      double D1wal1_Z[D1wal1_num];
      D1wal1_Z[0] = 0.;
      D1wal1_Z[1] = cD1wal1.getLength("L1") / Unit::mm;
      D1wal1_Z[2] = D1wal1_Z[1] + cD1wal1.getLength("L2") / Unit::mm;
      D1wal1_Z[3] = D1wal1_Z[2] + cD1wal1.getLength("L3") / Unit::mm;
      D1wal1_Z[4] = D1wal1_Z[3] + cD1wal1.getLength("L4") / Unit::mm;
      D1wal1_Z[5] = D1wal1_Z[4] + cD1wal1.getLength("L5") / Unit::mm;
      D1wal1_Z[6] = D1wal1_Z[5] + cD1wal1.getLength("L6") / Unit::mm;
      D1wal1_Z[7] = D1wal1_Z[6] + cD1wal1.getLength("L7") / Unit::mm;
      //
      double D1wal1_rI[D1wal1_num];
      for (int i = 0; i < D1wal1_num; i++)
        { D1wal1_rI[i] = 0.0; }
      //
      double D1wal1_rO[D1wal1_num];
      D1wal1_rO[0] = cD1wal1.getLength("R1") / Unit::mm;
      D1wal1_rO[1] = D1wal1_rO[0];
      D1wal1_rO[2] = cD1wal1.getLength("R2") / Unit::mm;
      D1wal1_rO[3] = D1wal1_rO[2];
      D1wal1_rO[4] = cD1wal1.getLength("R3") / Unit::mm;
      D1wal1_rO[5] = D1wal1_rO[4];
      D1wal1_rO[6] = cD1wal1.getLength("R4") / Unit::mm;
      D1wal1_rO[7] = D1wal1_rO[6];
      //
      string strMat_D1wal1 = cD1wal1.getString("Material");
      G4Material* mat_D1wal1 = Materials::get(strMat_D1wal1);

      G4Transform3D transform_D1wal1 = G4Translate3D(0., 0., 0.);
      transform_D1wal1 = transform_D1wal1 * G4RotateY3D(AngleHER);
      G4Transform3D transform_D1wal1_inv = G4Translate3D(0., 0., 0.);
      transform_D1wal1_inv = transform_D1wal1_inv * G4RotateY3D(-AngleHER);

      //define geometry
      G4Polycone* geo_D1wal1x = new G4Polycone("geo_D1wal1x_name", 0, 2*M_PI, D1wal1_num, D1wal1_Z, D1wal1_rI, D1wal1_rO);
      G4SubtractionSolid* geo_D1wal1 = new G4SubtractionSolid("geo_D1wal1_name", geo_D1wal1x, geo_TubeL, transform_D1wal1_inv);

      G4LogicalVolume *logi_D1wal1 = new G4LogicalVolume(geo_D1wal1, mat_D1wal1, "logi_D1wal1_name");

      //put volume
      setColor(*logi_D1wal1, cD1wal1.getString("Color", "#00CC00"));
      //setVisibility(*logi_D1wal1, false);
      new G4PVPlacement(transform_D1wal1, logi_D1wal1, "phys_D1wal1_name", &topVolume, false, 0);

      //-
      //--------------

      //--------------
      //-   D2spc1

      //get parameters from .xml file
      GearDir cD2spc1(content, "D2spc1/");
      //
      double D2spc1_bpthick = cD2spc1.getLength("bpthick") / Unit::mm;
      //
      double D2spc1_rO[D1wal1_num];
      D2spc1_rO[0] = D1wal1_rO[0] - D2spc1_bpthick;
      D2spc1_rO[1] = D1wal1_rO[1] - D2spc1_bpthick;
      D2spc1_rO[2] = D1wal1_rO[2] - D2spc1_bpthick;
      D2spc1_rO[3] = D1wal1_rO[3] - D2spc1_bpthick;
      D2spc1_rO[4] = D1wal1_rO[4] - D2spc1_bpthick;
      D2spc1_rO[5] = D1wal1_rO[5] - D2spc1_bpthick;
      D2spc1_rO[6] = D1wal1_rO[6] - D2spc1_bpthick;
      D2spc1_rO[7] = D1wal1_rO[7] - D2spc1_bpthick;
      //
      string strMat_D2spc1 = cD2spc1.getString("Material");
      G4Material* mat_D2spc1 = Materials::get(strMat_D2spc1);

      //define geometry
      G4Polycone* geo_D2spc1x = new G4Polycone("geo_D2spc1x_name", 0, 2*M_PI, D1wal1_num, D1wal1_Z, D1wal1_rI, D2spc1_rO);
      G4IntersectionSolid* geo_D2spc1 = new G4IntersectionSolid("geo_D2spc1_name", geo_D2spc1x, geo_D1wal1);

      G4LogicalVolume *logi_D2spc1 = new G4LogicalVolume(geo_D2spc1, mat_D2spc1, "logi_D2spc1_name");

      //put volume
      setColor(*logi_D2spc1, cD2spc1.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_D2spc1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D2spc1, "phys_D2spc1_name", logi_D1wal1, false, 0);

      //-
      //--------------

      //--------------
      //-   E1wal1

      //get parameters from .xml file
      GearDir cE1wal1(content, "E1wal1/");
      //
      const int E1wal1_num = 10;
      //
      double E1wal1_Z[E1wal1_num];
      E1wal1_Z[0] = 0.;
      E1wal1_Z[1] = cE1wal1.getLength("L1") / Unit::mm;
      E1wal1_Z[2] = E1wal1_Z[1] + cE1wal1.getLength("L2") / Unit::mm;
      E1wal1_Z[3] = E1wal1_Z[2] + cE1wal1.getLength("L3") / Unit::mm;
      E1wal1_Z[4] = E1wal1_Z[3] + cE1wal1.getLength("L4") / Unit::mm;
      E1wal1_Z[5] = E1wal1_Z[4] + cE1wal1.getLength("L5") / Unit::mm;
      E1wal1_Z[6] = E1wal1_Z[5] + cE1wal1.getLength("L6") / Unit::mm;
      E1wal1_Z[7] = E1wal1_Z[6] + cE1wal1.getLength("L7") / Unit::mm;
      E1wal1_Z[8] = E1wal1_Z[7] + cE1wal1.getLength("L8") / Unit::mm;
      E1wal1_Z[9] = E1wal1_Z[8] + cE1wal1.getLength("L9") / Unit::mm;
      //
      double E1wal1_rI[E1wal1_num];
      for (int i = 0; i < E1wal1_num; i++)
        { E1wal1_rI[i] = 0.0; }
      //
      double E1wal1_rO[E1wal1_num];
      E1wal1_rO[0] = cE1wal1.getLength("R1") / Unit::mm;
      E1wal1_rO[1] = E1wal1_rO[0];
      E1wal1_rO[2] = cE1wal1.getLength("R2") / Unit::mm;
      E1wal1_rO[3] = cE1wal1.getLength("R3") / Unit::mm;
      E1wal1_rO[4] = E1wal1_rO[3];
      E1wal1_rO[5] = cE1wal1.getLength("R4") / Unit::mm;
      E1wal1_rO[6] = cE1wal1.getLength("R5") / Unit::mm;
      E1wal1_rO[7] = E1wal1_rO[6];
      E1wal1_rO[8] = cE1wal1.getLength("R6") / Unit::mm;
      E1wal1_rO[9] = E1wal1_rO[8];
      //
      string strMat_E1wal1 = cE1wal1.getString("Material");
      G4Material* mat_E1wal1 = Materials::get(strMat_E1wal1);

      G4Transform3D transform_E1wal1 = G4Translate3D(0., 0., 0.);
      transform_E1wal1 = transform_E1wal1 * G4RotateY3D(AngleLER);
      G4Transform3D transform_E1wal1_inv = G4Translate3D(0., 0., 0.);
      transform_E1wal1_inv = transform_E1wal1_inv * G4RotateY3D(-AngleLER);

      //define geometry
      G4Polycone* geo_E1wal1x = new G4Polycone("geo_E1wal1x_name", 0, 2*M_PI, E1wal1_num, E1wal1_Z, E1wal1_rI, E1wal1_rO);
      G4SubtractionSolid* geo_E1wal1 = new G4SubtractionSolid("geo_E1wal1_name", geo_E1wal1x, geo_TubeL, transform_E1wal1_inv);

      G4LogicalVolume *logi_E1wal1 = new G4LogicalVolume(geo_E1wal1, mat_E1wal1, "logi_E1wal1_name");

      //put volume
      setColor(*logi_E1wal1, cE1wal1.getString("Color", "#0000CC"));
      //setVisibility(*logi_E1wal1, false);
      new G4PVPlacement(transform_E1wal1, logi_E1wal1, "phys_E1wal1_name", &topVolume, false, 0);

      //-
      //--------------

      //--------------
      //-   E2spc1

      //get parameters from .xml file
      GearDir cE2spc1(content, "E2spc1/");
      //
      double E2spc1_bpthick = cE2spc1.getLength("bpthick") / Unit::mm;
      //
      double E2spc1_rO[E1wal1_num];
      E2spc1_rO[0] = E1wal1_rO[0] - E2spc1_bpthick;
      E2spc1_rO[1] = E1wal1_rO[1] - E2spc1_bpthick;
      E2spc1_rO[2] = E1wal1_rO[2] - E2spc1_bpthick;
      E2spc1_rO[3] = E1wal1_rO[3] - E2spc1_bpthick;
      E2spc1_rO[4] = E1wal1_rO[4] - E2spc1_bpthick;
      E2spc1_rO[5] = E1wal1_rO[5] - E2spc1_bpthick;
      E2spc1_rO[6] = E1wal1_rO[6] - E2spc1_bpthick;
      E2spc1_rO[7] = E1wal1_rO[7] - E2spc1_bpthick;
      E2spc1_rO[8] = E1wal1_rO[8] - E2spc1_bpthick;
      E2spc1_rO[9] = E1wal1_rO[9] - E2spc1_bpthick;
      //
      string strMat_E2spc1 = cE2spc1.getString("Material");
      G4Material* mat_E2spc1 = Materials::get(strMat_E2spc1);

      //define geometry
      G4Polycone* geo_E2spc1x = new G4Polycone("geo_E2spc1x_name", 0, 2*M_PI, E1wal1_num, E1wal1_Z, E1wal1_rI, E2spc1_rO);
      G4IntersectionSolid* geo_E2spc1 = new G4IntersectionSolid("geo_E2spc1", geo_E2spc1x, geo_E1wal1);
      G4LogicalVolume *logi_E2spc1 = new G4LogicalVolume(geo_E2spc1, mat_E2spc1, "logi_E2spc1_name");

      //put volume
      setColor(*logi_E2spc1, cE2spc1.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_E2spc1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E2spc1, "phys_E2spc1_name", logi_E1wal1, false, 0);

      //-
      //--------------

      //hoge

      //--------------
      //-   F1wal1

      //get parameters from .xml file
      GearDir cF1wal1(content, "F1wal1/");
      //
      const int F1wal1_num = 13;
      //
      double F1wal1_Z[F1wal1_num];
      F1wal1_Z[0] = DistanceL;
      F1wal1_Z[1] = F1wal1_Z[0] + cF1wal1.getLength("L1") / Unit::mm;
      F1wal1_Z[2] = F1wal1_Z[1] + cF1wal1.getLength("L2") / Unit::mm;
      F1wal1_Z[3] = F1wal1_Z[2];
      F1wal1_Z[4] = F1wal1_Z[3] + cF1wal1.getLength("L3") / Unit::mm;
      F1wal1_Z[5] = F1wal1_Z[4] + cF1wal1.getLength("L4") / Unit::mm;
      F1wal1_Z[6] = F1wal1_Z[5] + cF1wal1.getLength("L5") / Unit::mm;
      F1wal1_Z[7] = F1wal1_Z[6];
      F1wal1_Z[8] = F1wal1_Z[7] + cF1wal1.getLength("L6") / Unit::mm;
      F1wal1_Z[9] = F1wal1_Z[8];
      F1wal1_Z[10] = F1wal1_Z[9] + cF1wal1.getLength("L7") / Unit::mm;
      F1wal1_Z[11] = F1wal1_Z[10];
      F1wal1_Z[12] = F1wal1_Z[11] + cF1wal1.getLength("L8") / Unit::mm;
      //
      double F1wal1_rI[F1wal1_num];
      for (int i = 0; i < F1wal1_num; i++)
        { F1wal1_rI[i] = 0.0; }
      //
      double F1wal1_rO[F1wal1_num];
      F1wal1_rO[0] = cF1wal1.getLength("R1") / Unit::mm;
      F1wal1_rO[1] = cF1wal1.getLength("R2") / Unit::mm;
      F1wal1_rO[2] = F1wal1_rO[1];
      F1wal1_rO[3] = cF1wal1.getLength("R3") / Unit::mm;
      F1wal1_rO[4] = F1wal1_rO[3];
      F1wal1_rO[5] = cF1wal1.getLength("R4") / Unit::mm;
      F1wal1_rO[6] = F1wal1_rO[5];
      F1wal1_rO[7] = cF1wal1.getLength("R5") / Unit::mm;
      F1wal1_rO[8] = F1wal1_rO[7];
      F1wal1_rO[9] = cF1wal1.getLength("R6") / Unit::mm;
      F1wal1_rO[10] = F1wal1_rO[9];
      F1wal1_rO[11] = cF1wal1.getLength("R7") / Unit::mm;
      F1wal1_rO[12] = F1wal1_rO[11];
      //
      string strMat_F1wal1 = cF1wal1.getString("Material");
      G4Material* mat_F1wal1 = Materials::get(strMat_F1wal1);

      //define geometry
      G4Polycone* geo_F1wal1xx = new G4Polycone("geo_F1wal1xx_name", 0, 2*M_PI, F1wal1_num, F1wal1_Z, F1wal1_rI, F1wal1_rO);
      G4SubtractionSolid* geo_F1wal1x = new G4SubtractionSolid("geo_F1wal1x_name", geo_F1wal1xx, geo_D1wal1, transform_D1wal1);
      G4SubtractionSolid* geo_F1wal1 = new G4SubtractionSolid("geo_F1wal1_name", geo_F1wal1x, geo_E1wal1, transform_E1wal1);
      G4LogicalVolume *logi_F1wal1 = new G4LogicalVolume(geo_F1wal1, mat_F1wal1, "logi_F1wal1_name");

      //put volume
      setColor(*logi_F1wal1, cF1wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_F1wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F1wal1, "phys_F1wal1_name", &topVolume, false, 0);

      //-
      //--------------


      //--------------
      //-   F2spc1

      //get parameters from .xml file
      GearDir cF2spc1(content, "F2spc1/");
      //
      const int F2spc1_num = 11;
      //
      double F2spc1_Z[F2spc1_num];
      F2spc1_Z[0] = DistanceL + cF2spc1.getLength("D1") / Unit::mm;
      F2spc1_Z[1] = F2spc1_Z[0] + cF2spc1.getLength("L1") / Unit::mm;
      F2spc1_Z[2] = F2spc1_Z[1] + cF2spc1.getLength("L2") / Unit::mm;
      F2spc1_Z[3] = F2spc1_Z[2];
      F2spc1_Z[4] = F2spc1_Z[3] + cF2spc1.getLength("L3") / Unit::mm;
      F2spc1_Z[5] = F2spc1_Z[4] + cF2spc1.getLength("L4") / Unit::mm;
      F2spc1_Z[6] = F2spc1_Z[5] + cF2spc1.getLength("L5") / Unit::mm - cF2spc1.getLength("L6") / Unit::mm;
      F2spc1_Z[7] = F2spc1_Z[6];
      F2spc1_Z[8] = F2spc1_Z[7] + cF2spc1.getLength("L7") / Unit::mm;
      F2spc1_Z[9] = F2spc1_Z[8];
      F2spc1_Z[10] = F2spc1_Z[9] + cF2spc1.getLength("L8") / Unit::mm;
      //
      double F2spc1_rI[F2spc1_num];
      for (int i = 0; i < F2spc1_num; i++)
        { F2spc1_rI[i] = 0.0; }
      //
      double F2spc1_rO[F2spc1_num];
      F2spc1_rO[0] = cF2spc1.getLength("R1") / Unit::mm;
      F2spc1_rO[1] = cF2spc1.getLength("R2") / Unit::mm;
      F2spc1_rO[2] = F2spc1_rO[1];
      F2spc1_rO[3] = cF2spc1.getLength("R3") / Unit::mm;
      F2spc1_rO[4] = F2spc1_rO[3];
      F2spc1_rO[5] = cF2spc1.getLength("R4") / Unit::mm;
      F2spc1_rO[6] = F2spc1_rO[5];
      F2spc1_rO[7] = cF2spc1.getLength("R6") / Unit::mm;
      F2spc1_rO[8] = F2spc1_rO[7];
      F2spc1_rO[9] = cF2spc1.getLength("R7") / Unit::mm;
      F2spc1_rO[10] = F2spc1_rO[9];
      //
      string strMat_F2spc1 = cF2spc1.getString("Material");
      G4Material* mat_F2spc1 = Materials::get(strMat_F2spc1);

      //define geometry
      G4Polycone* geo_F2spc1xx = new G4Polycone("geo_F2spc1xx_name", 0, 2*M_PI, F2spc1_num, F2spc1_Z, F2spc1_rI, F2spc1_rO);
      G4SubtractionSolid* geo_F2spc1x = new G4SubtractionSolid("geo_F2spc1x_name", geo_F2spc1xx, geo_D1wal1, transform_D1wal1);
      G4SubtractionSolid* geo_F2spc1 = new G4SubtractionSolid("geo_F2spc1_name", geo_F2spc1x, geo_E1wal1, transform_E1wal1);
      G4LogicalVolume *logi_F2spc1 = new G4LogicalVolume(geo_F2spc1, mat_F2spc1, "logi_F2spc1_name");

      //put volume
      setColor(*logi_F2spc1, cF2spc1.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F2spc1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F2spc1, "phys_F2spc1_name", logi_F1wal1, false, 0);

      //-
      //--------------

      //--------------
      //-   F3wal2

      //get parameters from .xml file
      GearDir cF3wal2(content, "F3wal2/");
      //
      const int F3wal2_num = 7;
      //
      double F3wal2_Z[F3wal2_num];
      F3wal2_Z[0] = F2spc1_Z[0] + cF3wal2.getLength("D1") / Unit::mm;
      F3wal2_Z[1] = F3wal2_Z[0] + cF3wal2.getLength("L1") / Unit::mm;
      F3wal2_Z[2] = F3wal2_Z[1] + cF3wal2.getLength("L2") / Unit::mm;
      F3wal2_Z[3] = F3wal2_Z[2] + cF3wal2.getLength("L3") / Unit::mm;
      F3wal2_Z[4] = F3wal2_Z[3] + cF3wal2.getLength("L4") / Unit::mm;
      F3wal2_Z[5] = F3wal2_Z[4];
      F3wal2_Z[6] = F3wal2_Z[5] + cF3wal2.getLength("L5") / Unit::mm;
      //
      double F3wal2_rI[F3wal2_num];
      for (int i = 0; i < F3wal2_num; i++)
        { F3wal2_rI[i] = 0.0; }
      //
      double F3wal2_rO[F3wal2_num];
      F3wal2_rO[0] = cF3wal2.getLength("R1") / Unit::mm;
      F3wal2_rO[1] = cF3wal2.getLength("R2") / Unit::mm;
      F3wal2_rO[2] = F3wal2_rO[1];
      F3wal2_rO[3] = cF3wal2.getLength("R3") / Unit::mm;
      F3wal2_rO[4] = F3wal2_rO[3];
      F3wal2_rO[5] = cF3wal2.getLength("R4") / Unit::mm;
      F3wal2_rO[6] = F3wal2_rO[5];
      //
      string strMat_F3wal2 = cF3wal2.getString("Material");
      G4Material* mat_F3wal2 = Materials::get(strMat_F3wal2);

      //define geometry
      G4Polycone* geo_F3wal2xx = new G4Polycone("geo_F3wal2xx_name", 0, 2*M_PI, F3wal2_num, F3wal2_Z, F3wal2_rI, F3wal2_rO);
      G4SubtractionSolid* geo_F3wal2x = new G4SubtractionSolid("geo_F3wal2x_name", geo_F3wal2xx, geo_D1wal1, transform_D1wal1);
      G4SubtractionSolid* geo_F3wal2 = new G4SubtractionSolid("geo_F3wal2_name", geo_F3wal2x, geo_E1wal1, transform_E1wal1);
      G4LogicalVolume *logi_F3wal2 = new G4LogicalVolume(geo_F3wal2, mat_F3wal2, "logi_F3wal2_name");

      //put volume
      setColor(*logi_F3wal2, cF3wal2.getString("Color", "#CC0000"));
      //setVisibility(*logi_F3wal2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F3wal2, "phys_F3wal2_name", logi_F2spc1, false, 0);

      //-
      //--------------

      /*

      //--------------
      //-   F4spc2

      //get parameters from .xml file
      GearDir cF4spc2(content, "F4spc2/");
      //
      const int F4spc2_num = 7;
      //
      double F4spc2_Z[F4spc2_num];
      F4spc2_Z[0] = F3wal2_Z[0] + cF4spc2.getLength("D1") / Unit::mm;
      F4spc2_Z[1] = F4spc2_Z[0] + cF4spc2.getLength("L1") / Unit::mm;
      F4spc2_Z[2] = F4spc2_Z[1] + cF4spc2.getLength("L2") / Unit::mm;
      F4spc2_Z[3] = F4spc2_Z[2] + cF4spc2.getLength("L3") / Unit::mm;
      F4spc2_Z[4] = F4spc2_Z[3] + cF4spc2.getLength("L4") / Unit::mm;
      F4spc2_Z[5] = F4spc2_Z[4];
      F4spc2_Z[6] = F4spc2_Z[5] + cF4spc2.getLength("L5") / Unit::mm;
      //
      double F4spc2_rI[F4spc2_num];
      for (int i = 0; i < F4spc2_num; i++)
        { F4spc2_rI[i] = 0.0; }
      //
      double F4spc2_rO[F4spc2_num];
      F4spc2_rO[0] = cF4spc2.getLength("R1") / Unit::mm;
      F4spc2_rO[1] = cF4spc2.getLength("R2") / Unit::mm;
      F4spc2_rO[2] = F4spc2_rO[1];
      F4spc2_rO[3] = cF4spc2.getLength("R3") / Unit::mm;
      F4spc2_rO[4] = F4spc2_rO[3];
      F4spc2_rO[5] = cF4spc2.getLength("R4") / Unit::mm;
      F4spc2_rO[6] = F4spc2_rO[5];
      //
      string strMat_F4spc2 = cF4spc2.getString("Material");
      G4Material* mat_F4spc2 = Materials::get(strMat_F4spc2);

      //define geometry
      G4Polycone* geo_F4spc2xx = new G4Polycone("geo_F4spc2xx_name", 0, 2*M_PI, F4spc2_num, F4spc2_Z, F4spc2_rI, F4spc2_rO);
      G4SubtractionSolid* geo_F4spc2x = new G4SubtractionSolid("geo_F4spc2x_name", geo_F4spc2xx, geo_D1wal1, transform_D1wal1);
      G4SubtractionSolid* geo_F4spc2 = new G4SubtractionSolid("geo_F4spc2_name", geo_F4spc2x, geo_E1wal1, transform_E1wal1);
      G4LogicalVolume *logi_F4spc2 = new G4LogicalVolume(geo_F4spc2, mat_F4spc2, "logi_F4spc2_name");

      //put volume
      setColor(*logi_F4spc2, cF4spc2.getString("Color", "#CCCCCC"));
      setVisibility(*logi_F4spc2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F4spc2, "phys_F4spc2_name", logi_F3wal2, false, 0);

      //-
      //--------------


      //--------------
      //-   F5wal3

      //get parameters from .xml file
      GearDir cF5wal3(content, "F5wal3/");
      //
      const int F5wal3_num = 8;
      //
      double F5wal3_Z[F5wal3_num];
      F5wal3_Z[0] = F4spc2_Z[0] + cF5wal3.getLength("D1") / Unit::mm;
      F5wal3_Z[1] = F5wal3_Z[0] + cF5wal3.getLength("L1") / Unit::mm;
      F5wal3_Z[2] = F5wal3_Z[1] + cF5wal3.getLength("L2") / Unit::mm;
      F5wal3_Z[3] = F5wal3_Z[2];
      F5wal3_Z[4] = F5wal3_Z[3] + cF5wal3.getLength("L3") / Unit::mm;
      F5wal3_Z[5] = F5wal3_Z[4] + cF5wal3.getLength("L4") / Unit::mm;
      F5wal3_Z[6] = F5wal3_Z[5] + cF5wal3.getLength("L5") / Unit::mm;
      F5wal3_Z[7] = F5wal3_Z[6] + cF5wal3.getLength("L6") / Unit::mm;
      //
      double F5wal3_rI[F5wal3_num];
      for (int i = 0; i < F5wal3_num; i++)
        { F5wal3_rI[i] = 0.0; }
      //
      double F5wal3_rO[F5wal3_num];
      F5wal3_rO[0] = cF5wal3.getLength("R1") / Unit::mm;
      F5wal3_rO[1] = cF5wal3.getLength("R2") / Unit::mm;
      F5wal3_rO[2] = F5wal3_rO[1];
      F5wal3_rO[3] = cF5wal3.getLength("R3") / Unit::mm;
      F5wal3_rO[4] = F5wal3_rO[3];
      F5wal3_rO[5] = cF5wal3.getLength("R4") / Unit::mm;
      F5wal3_rO[6] = F5wal3_rO[5];
      F5wal3_rO[7] = cF5wal3.getLength("R5") / Unit::mm;
      //
      string strMat_F5wal3 = cF5wal3.getString("Material");
      G4Material* mat_F5wal3 = Materials::get(strMat_F5wal3);

      //define geometry
      G4Polycone* geo_F5wal3xx = new G4Polycone("geo_F5wal3xx_name", 0, 2*M_PI, F5wal3_num, F5wal3_Z, F5wal3_rI, F5wal3_rO);
      G4SubtractionSolid* geo_F5wal3x = new G4SubtractionSolid("geo_F5wal3x_name", geo_F5wal3xx, geo_D1wal1, transform_D1wal1);
      G4SubtractionSolid* geo_F5wal3 = new G4SubtractionSolid("geo_F5wal3_name", geo_F5wal3x, geo_E1wal1, transform_E1wal1);
      G4LogicalVolume *logi_F5wal3 = new G4LogicalVolume(geo_F5wal3, mat_F5wal3, "logi_F5wal3_name");

      //put volume
      setColor(*logi_F5wal3, cF5wal3.getString("Color", "#CC0000"));
      //setVisibility(*logi_F5wal3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F5wal3, "phys_F5wal3_name", logi_F4spc2, false, 0);

      //-
      //--------------

      //--------------
      //-   F5wal5

      //get parameters from .xml file
      GearDir cF5wal5(content, "F5wal5/");
      //
      const int F5wal5_num = 3;
      //
      double F5wal5_Z[F5wal5_num];
      F5wal5_Z[0] = F4spc2_Z[0] + cF5wal5.getLength("D1") / Unit::mm;
      F5wal5_Z[1] = F5wal5_Z[0] + cF5wal5.getLength("L1") / Unit::mm;
      F5wal5_Z[2] = F5wal5_Z[1] + cF5wal5.getLength("L2") / Unit::mm;
      //
      double F5wal5_rI[F5wal5_num];
      for (int i = 0; i < F5wal5_num; i++)
        { F5wal5_rI[i] = 0.0; }
      //
      double F5wal5_rO[F5wal5_num];
      F5wal5_rO[0] = cF5wal5.getLength("R1") / Unit::mm;
      F5wal5_rO[1] = F5wal5_rO[0];
      F5wal5_rO[2] = cF5wal5.getLength("R2") / Unit::mm;
      //
      string strMat_F5wal5 = cF5wal5.getString("Material");
      G4Material* mat_F5wal5 = Materials::get(strMat_F5wal5);

      //define geometry
      G4Polycone* geo_F5wal5xx = new G4Polycone("geo_F5wal5xx_name", 0, 2*M_PI, F5wal5_num, F5wal5_Z, F5wal5_rI, F5wal5_rO);
      G4SubtractionSolid* geo_F5wal5x = new G4SubtractionSolid("geo_F5wal5x_name", geo_F5wal5xx, geo_D1wal1, transform_D1wal1);
      G4SubtractionSolid* geo_F5wal5 = new G4SubtractionSolid("geo_F5wal5_name", geo_F5wal5x, geo_E1wal1, transform_E1wal1);
      G4LogicalVolume *logi_F5wal5 = new G4LogicalVolume(geo_F5wal5, mat_F5wal5, "logi_F5wal5_name");

      //put volume
      setColor(*logi_F5wal5, cF5wal5.getString("Color", "#CC0000"));
      //setVisibility(*logi_F5wal5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F5wal5, "phys_F5wal5_name", logi_F4spc2, false, 0);

      //-
      //--------------

      //--------------
      //-   F6spc4

      //get parameters from .xml file
      GearDir cF6spc4(content, "F6spc4/");
      //
      const int F6spc4_num = 6;
      //
      double F6spc4_Z[F6spc4_num];
      F6spc4_Z[0] = 0.0;
      F6spc4_Z[1] = F6spc4_Z[0] + cF6spc4.getLength("L1") / Unit::mm;
      F6spc4_Z[2] = F6spc4_Z[1] + cF6spc4.getLength("L2") / Unit::mm;
      F6spc4_Z[3] = F6spc4_Z[2] + cF6spc4.getLength("L3") / Unit::mm;
      F6spc4_Z[4] = F6spc4_Z[3] + cF6spc4.getLength("L4") / Unit::mm;
      F6spc4_Z[5] = F6spc4_Z[4] + cF6spc4.getLength("L5") / Unit::mm;
      //
      double F6spc4_rI[F6spc4_num];
      for (int i = 0; i < F6spc4_num; i++)
        { F6spc4_rI[i] = 0.0; }
      //
      double F6spc4_rO[F6spc4_num];
      F6spc4_rO[0] = cF6spc4.getLength("R1") / Unit::mm;
      F6spc4_rO[1] = F6spc4_rO[0];
      F6spc4_rO[2] = cF6spc4.getLength("R2") / Unit::mm;
      F6spc4_rO[3] = F6spc4_rO[2];
      F6spc4_rO[4] = cF6spc4.getLength("R3") / Unit::mm;
      F6spc4_rO[5] = F6spc4_rO[4];
      //
      string strMat_F6spc4 = cF6spc4.getString("Material");
      G4Material* mat_F6spc4 = Materials::get(strMat_F6spc4);

      //define geometry
      G4Polycone* geo_F6spc4xx = new G4Polycone("geo_F6spc4xx_name", 0, 2*M_PI, F6spc4_num, F6spc4_Z, F6spc4_rI, F6spc4_rO);
      G4SubtractionSolid* geo_F6spc4x = new G4SubtractionSolid("geo_F6spc4x_name", geo_F6spc4xx, geo_D1wal1, transform_D1wal1);
      G4IntersectionSolid* geo_F6spc4 = new G4IntersectionSolid("geo_F6spc4_name", geo_F6spc4x, geo_F5wal3);

      G4LogicalVolume *logi_F6spc4 = new G4LogicalVolume(geo_F6spc4, mat_F6spc4, "logi_F6spc4_name");

      //put volume
      setColor(*logi_F6spc4, cF6spc4.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F6spc4, false);
      new G4PVPlacement(transform_D1wal1, logi_F6spc4, "phys_F6spc4_name", logi_F5wal3, false, 0);

      //-
      //--------------


      //--------------
      //-   F6spc5

      //get parameters from .xml file
      GearDir cF6spc5(content, "F6spc5/");
      //
      const int F6spc5_num = 8;
      //
      double F6spc5_Z[F6spc5_num];
      F6spc5_Z[0] = 0.0;
      F6spc5_Z[1] = F6spc5_Z[0] + cF6spc5.getLength("L1") / Unit::mm;
      F6spc5_Z[2] = F6spc5_Z[1] + cF6spc5.getLength("L2") / Unit::mm;
      F6spc5_Z[3] = F6spc5_Z[2] + cF6spc5.getLength("L3") / Unit::mm;
      F6spc5_Z[4] = F6spc5_Z[3] + cF6spc5.getLength("L4") / Unit::mm;
      F6spc5_Z[5] = F6spc5_Z[4] + cF6spc5.getLength("L5") / Unit::mm;
      F6spc5_Z[6] = F6spc5_Z[5] + cF6spc5.getLength("L6") / Unit::mm;
      F6spc5_Z[7] = F6spc5_Z[6] + cF6spc5.getLength("L7") / Unit::mm;
      //
      double F6spc5_rI[F6spc5_num];
      for (int i = 0; i < F6spc5_num; i++)
        { F6spc5_rI[i] = 0.0; }
      //
      double F6spc5_rO[F6spc5_num];
      F6spc5_rO[0] = cF6spc5.getLength("R1") / Unit::mm;
      F6spc5_rO[1] = F6spc5_rO[0];
      F6spc5_rO[2] = cF6spc5.getLength("R2") / Unit::mm;
      F6spc5_rO[3] = cF6spc5.getLength("R3") / Unit::mm;
      F6spc5_rO[4] = F6spc5_rO[2];
      F6spc5_rO[5] = cF6spc5.getLength("R4") / Unit::mm;
      F6spc5_rO[6] = cF6spc5.getLength("R5") / Unit::mm;
      F6spc5_rO[7] = F6spc5_rO[4];
      //
      string strMat_F6spc5 = cF6spc5.getString("Material");
      G4Material* mat_F6spc5 = Materials::get(strMat_F6spc5);

      //define geometry
      G4Polycone* geo_F6spc5xx = new G4Polycone("geo_F6spc5xx_name", 0, 2*M_PI, F6spc5_num, F6spc5_Z, F6spc5_rI, F6spc5_rO);
      G4SubtractionSolid* geo_F6spc5x = new G4SubtractionSolid("geo_F6spc5x_name", geo_F6spc5xx, geo_E1wal1, transform_E1wal1);
      G4IntersectionSolid* geo_F6spc5 = new G4IntersectionSolid("geo_F6spc5_name", geo_F6spc5x, geo_F5wal3);

      G4LogicalVolume *logi_F6spc5 = new G4LogicalVolume(geo_F6spc5, mat_F6spc5, "logi_F6spc5_name");

      //put volume
      setColor(*logi_F6spc5, cF6spc5.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F6spc5, false);
      new G4PVPlacement(transform_E1wal1, logi_F6spc5, "phys_F6spc5_name", logi_F5wal3, false, 0);

      //-
      //--------------

      //--------------
      //-   F6spc7

      //get parameters from .xml file
      GearDir cF6spc7(content, "F6spc7/");
      //
      const int F6spc7_num = 2;
      //
      double F6spc7_Z[F6spc7_num];
      F6spc7_Z[0] = cF6spc7.getLength("D1") / Unit::mm;
      F6spc7_Z[1] = F6spc7_Z[0] + cF6spc7.getLength("L1") / Unit::mm;
      //
      double F6spc7_rI[F6spc7_num];
      for (int i = 0; i < F6spc7_num; i++)
        { F6spc7_rI[i] = 0.0; }
      //
      double F6spc7_rO[F6spc7_num];
      F6spc7_rO[0] = cF6spc7.getLength("R1") / Unit::mm;
      F6spc7_rO[1] = F6spc7_rO[0];
      //
      string strMat_F6spc7 = cF6spc7.getString("Material");
      G4Material* mat_F6spc7 = Materials::get(strMat_F6spc7);

      //define geometry
      G4Polycone* geo_F6spc7xx = new G4Polycone("geo_F6spc7xx_name", 0, 2*M_PI, F6spc7_num, F6spc7_Z, F6spc7_rI, F6spc7_rO);
      G4SubtractionSolid* geo_F6spc7x = new G4SubtractionSolid("geo_F6spc7x_name", geo_F6spc7xx, geo_D1wal1, transform_D1wal1);
      G4IntersectionSolid* geo_F6spc7 = new G4IntersectionSolid("geo_F6spc7_name", geo_F6spc7x, geo_F5wal5);
      G4LogicalVolume *logi_F6spc7 = new G4LogicalVolume(geo_F6spc7, mat_F6spc7, "logi_F6spc7_name");

      //put volume
      setColor(*logi_F6spc7, cF6spc7.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F6spc7, false);
      new G4PVPlacement(transform_D1wal1, logi_F6spc7, "phys_F6spc7_name", logi_F5wal5, false, 0);

      //-
      //--------------


      //--------------
      //-   F6spc8

      //get parameters from .xml file
      GearDir cF6spc8(content, "F6spc8/");
      //
      const int F6spc8_num = 2;
      //
      double F6spc8_Z[F6spc8_num];
      F6spc8_Z[0] = cF6spc8.getLength("D1") / Unit::mm;
      F6spc8_Z[1] = F6spc8_Z[0] + cF6spc8.getLength("L1") / Unit::mm;
      //
      double F6spc8_rI[F6spc8_num];
      for (int i = 0; i < F6spc8_num; i++)
        { F6spc8_rI[i] = 0.0; }
      //
      double F6spc8_rO[F6spc8_num];
      F6spc8_rO[0] = cF6spc8.getLength("R1") / Unit::mm;
      F6spc8_rO[1] = F6spc8_rO[0];
      //
      string strMat_F6spc8 = cF6spc8.getString("Material");
      G4Material* mat_F6spc8 = Materials::get(strMat_F6spc8);

      //define geometry
      G4Polycone* geo_F6spc8xx = new G4Polycone("geo_F6spc8xx_name", 0, 2*M_PI, F6spc8_num, F6spc8_Z, F6spc8_rI, F6spc8_rO);
      G4SubtractionSolid* geo_F6spc8x = new G4SubtractionSolid("geo_F6spc8x_name", geo_F6spc8xx, geo_E1wal1, transform_E1wal1);
      G4IntersectionSolid* geo_F6spc8 = new G4IntersectionSolid("geo_F6spc8_name", geo_F6spc8x, geo_F5wal5);

      G4LogicalVolume *logi_F6spc8 = new G4LogicalVolume(geo_F6spc8, mat_F6spc8, "logi_F6spc8_name");

      //put volume
      setColor(*logi_F6spc8, cF6spc8.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F6spc8, false);
      new G4PVPlacement(transform_E1wal1, logi_F6spc8, "phys_F6spc8_name", logi_F5wal5, false, 0);

      //-
      //--------------


      //--------------
      //- F6tnl1

      //get parameters from .xml file
      GearDir cF6tnl1(content, "F6tnl1/");
      //
      const int F6tnl1_num = 6;
      //
      double F6tnl1_Z[F6tnl1_num];
      F6tnl1_Z[0] = 0.0;
      F6tnl1_Z[1] = F6tnl1_Z[0] + cF6tnl1.getLength("L1") / Unit::mm;
      F6tnl1_Z[2] = F6tnl1_Z[1] + cF6tnl1.getLength("L2") / Unit::mm;
      F6tnl1_Z[3] = F6tnl1_Z[2] + cF6tnl1.getLength("L3") / Unit::mm;
      F6tnl1_Z[4] = F6tnl1_Z[3] + cF6tnl1.getLength("L4") / Unit::mm;
      F6tnl1_Z[5] = F6tnl1_Z[4] + cF6tnl1.getLength("L5") / Unit::mm;
      //
      double F6tnl1_rI[F6tnl1_num];
      for (int i = 0; i < F6tnl1_num; i++)
        { F6tnl1_rI[i] = 0.0; }
      //
      double F6tnl1_rO[F6tnl1_num];
      F6tnl1_rO[0] = cF6tnl1.getLength("R1") / Unit::mm;
      F6tnl1_rO[1] = F6tnl1_rO[0];
      F6tnl1_rO[2] = cF6tnl1.getLength("R2") / Unit::mm;
      F6tnl1_rO[3] = F6tnl1_rO[2];
      F6tnl1_rO[4] = cF6tnl1.getLength("R3") / Unit::mm;
      F6tnl1_rO[5] = F6tnl1_rO[4];
      //define geometry
      G4Polycone* geo_F6tnl1 = new G4Polycone("geo_F6tnl1_name", 0, 2*M_PI, F6tnl1_num, F6tnl1_Z, F6tnl1_rI, F6tnl1_rO);

      //-
      //--------------

      //--------------
      //- F6tnl2

      //get parameters from .xml file
      GearDir cF6tnl2(content, "F6tnl2/");
      //
      const int F6tnl2_num = 8;
      //
      double F6tnl2_Z[F6tnl2_num];
      F6tnl2_Z[0] = 0.0;
      F6tnl2_Z[1] = F6tnl2_Z[0] + cF6tnl2.getLength("L1") / Unit::mm;
      F6tnl2_Z[2] = F6tnl2_Z[1] + cF6tnl2.getLength("L2") / Unit::mm;
      F6tnl2_Z[3] = F6tnl2_Z[2] + cF6tnl2.getLength("L3") / Unit::mm;
      F6tnl2_Z[4] = F6tnl2_Z[3] + cF6tnl2.getLength("L4") / Unit::mm;
      F6tnl2_Z[5] = F6tnl2_Z[4] + cF6tnl2.getLength("L5") / Unit::mm;
      F6tnl2_Z[6] = F6tnl2_Z[5] + cF6tnl2.getLength("L6") / Unit::mm;
      F6tnl2_Z[7] = F6tnl2_Z[6] + cF6tnl2.getLength("L7") / Unit::mm;
      //
      double F6tnl2_rI[F6tnl2_num];
      for (int i = 0; i < F6tnl2_num; i++)
        { F6tnl2_rI[i] = 0.0; }
      //
      double F6tnl2_rO[F6tnl2_num];
      F6tnl2_rO[0] = cF6tnl2.getLength("R1") / Unit::mm;
      F6tnl2_rO[1] = F6tnl2_rO[0];
      F6tnl2_rO[2] = cF6tnl2.getLength("R2") / Unit::mm;
      F6tnl2_rO[3] = cF6tnl2.getLength("R3") / Unit::mm;
      F6tnl2_rO[4] = F6tnl2_rO[3];
      F6tnl2_rO[5] = cF6tnl2.getLength("R4") / Unit::mm;
      F6tnl2_rO[6] = cF6tnl2.getLength("R5") / Unit::mm;
      F6tnl2_rO[7] = F6tnl2_rO[6];
      //define geometry
      G4Polycone* geo_F6tnl2 = new G4Polycone("geo_F6tnl2_name", 0, 2*M_PI, F6tnl2_num, F6tnl2_Z, F6tnl2_rI, F6tnl2_rO);

      //-
      //--------------

      //--------------
      //- F6tnl3

      //get parameters from .xml file
      GearDir cF6tnl3(content, "F6tnl3/");
      //
      const int F6tnl3_num = 2;
      //
      double F6tnl3_Z[F6tnl3_num];
      F6tnl3_Z[0] = cF6tnl3.getLength("D1") / Unit::mm;
      F6tnl3_Z[1] = F6tnl3_Z[0] + cF6tnl3.getLength("L1") / Unit::mm;
      //
      double F6tnl3_rI[F6tnl3_num];
      for (int i = 0; i < F6tnl3_num; i++)
        { F6tnl3_rI[i] = 0.0; }
      //
      double F6tnl3_rO[F6tnl3_num];
      F6tnl3_rO[0] = cF6tnl3.getLength("R1") / Unit::mm;
      F6tnl3_rO[1] = F6tnl3_rO[0];
      //define geometry
      G4Polycone* geo_F6tnl3 = new G4Polycone("geo_F6tnl3_name", 0, 2*M_PI, F6tnl3_num, F6tnl3_Z, F6tnl3_rI, F6tnl3_rO);

      //-
      //--------------

      //--------------
      //- F6tnl4

      //get parameters from .xml file
      GearDir cF6tnl4(content, "F6tnl4/");
      //
      const int F6tnl4_num = 2;
      //
      double F6tnl4_Z[F6tnl4_num];
      F6tnl4_Z[0] = cF6tnl4.getLength("D1") / Unit::mm;
      F6tnl4_Z[1] = F6tnl4_Z[0] + cF6tnl4.getLength("L1") / Unit::mm;
      //
      double F6tnl4_rI[F6tnl4_num];
      for (int i = 0; i < F6tnl4_num; i++)
        { F6tnl4_rI[i] = 0.0; }
      //
      double F6tnl4_rO[F6tnl4_num];
      F6tnl4_rO[0] = cF6tnl4.getLength("R1") / Unit::mm;
      F6tnl4_rO[1] = F6tnl4_rO[0];
      //define geometry
      G4Polycone* geo_F6tnl4 = new G4Polycone("geo_F6tnl4_name", 0, 2*M_PI, F6tnl4_num, F6tnl4_Z, F6tnl4_rI, F6tnl4_rO);

      //-
      //--------------


      //--------------
      //-   F6spc3

      //get parameters from .xml file
      GearDir cF6spc3(content, "F6spc3/");
      //
      const int F6spc3_num = 7;
      //
      double F6spc3_Z[F6spc3_num];
      F6spc3_Z[0] = F5wal3_Z[0] + cF6spc3.getLength("D1") / Unit::mm;
      F6spc3_Z[1] = F6spc3_Z[0] + cF6spc3.getLength("L1") / Unit::mm;
      F6spc3_Z[2] = F6spc3_Z[1] + cF6spc3.getLength("L2") / Unit::mm;
      F6spc3_Z[3] = F6spc3_Z[2];
      F6spc3_Z[4] = F6spc3_Z[3] + cF6spc3.getLength("L3") / Unit::mm;
      F6spc3_Z[5] = F6spc3_Z[4] + cF6spc3.getLength("L4") / Unit::mm;
      F6spc3_Z[6] = F6spc3_Z[5] + cF6spc3.getLength("L5") / Unit::mm;
      //
      double F6spc3_rI[F6spc3_num];
      for (int i = 0; i < F6spc3_num; i++)
        { F6spc3_rI[i] = 0.0; }
      //
      double F6spc3_rO[F6spc3_num];
      F6spc3_rO[0] = cF6spc3.getLength("R1") / Unit::mm;
      F6spc3_rO[1] = cF6spc3.getLength("R2") / Unit::mm;
      F6spc3_rO[2] = F6spc3_rO[1];
      F6spc3_rO[3] = cF6spc3.getLength("R3") / Unit::mm;
      F6spc3_rO[4] = F6spc3_rO[3];
      F6spc3_rO[5] = cF6spc3.getLength("R4") / Unit::mm;
      F6spc3_rO[6] = F6spc3_rO[5];
      //
      string strMat_F6spc3 = cF6spc3.getString("Material");
      G4Material* mat_F6spc3 = Materials::get(strMat_F6spc3);

      for (int i=0; i< F6spc3_num; i++) printf("%f %f %f\n", F6spc3_Z[i], F6spc3_rI[i], F6spc3_rO[i]);

      //define geometry
      G4Polycone* geo_F6spc3xx = new G4Polycone("geo_F6spc3xx_name", 0, 2*M_PI, F6spc3_num, F6spc3_Z, F6spc3_rI, F6spc3_rO);
      G4SubtractionSolid* geo_F6spc3x = new G4SubtractionSolid("geo_F6spc3x_name", geo_F6spc3xx, geo_F6tnl1, transform_D1wal1);
      G4SubtractionSolid* geo_F6spc3 = new G4SubtractionSolid("geo_F6spc3_name", geo_F6spc3x, geo_F6tnl2, transform_E1wal1);

      G4LogicalVolume *logi_F6spc3 = new G4LogicalVolume(geo_F6spc3, mat_F6spc3, "logi_F6spc3_name");

      //put volume
      setColor(*logi_F6spc3, cF6spc3.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F6spc3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F6spc3, "phys_F6spc3_name", logi_F5wal3, false, 0);

      //-
      //--------------

      //--------------
      //-   F6spc6

      //get parameters from .xml file
      GearDir cF6spc6(content, "F6spc6/");
      //
      const int F6spc6_num = 3;
      //
      double F6spc6_Z[F6spc6_num];
      F6spc6_Z[0] = F5wal5_Z[0] + cF6spc6.getLength("D1") / Unit::mm;
      F6spc6_Z[1] = F6spc6_Z[0] + cF6spc6.getLength("L1") / Unit::mm;
      F6spc6_Z[2] = F6spc6_Z[1] + cF6spc6.getLength("L2") / Unit::mm;
      //
      double F6spc6_rI[F6spc6_num];
      for (int i = 0; i < F6spc6_num; i++)
        { F6spc6_rI[i] = 0.0; }
      //
      double F6spc6_rO[F6spc6_num];
      F6spc6_rO[0] = cF6spc6.getLength("R1") / Unit::mm;
      F6spc6_rO[1] = F6spc6_rO[0];
      F6spc6_rO[2] = cF6spc6.getLength("R2") / Unit::mm;
      //
      string strMat_F6spc6 = cF6spc6.getString("Material");
      G4Material* mat_F6spc6 = Materials::get(strMat_F6spc6);

      //define geometry
      G4Polycone* geo_F6spc6xx = new G4Polycone("geo_F6spc6xx_name", 0, 2*M_PI, F6spc6_num, F6spc6_Z, F6spc6_rI, F6spc6_rO);
      G4SubtractionSolid* geo_F6spc6x = new G4SubtractionSolid("geo_F6spc6x_name", geo_F6spc6xx, geo_F6tnl3, transform_D1wal1);
      G4SubtractionSolid* geo_F6spc6 = new G4SubtractionSolid("geo_F6spc6_nam", geo_F6spc6x, geo_F6tnl4, transform_E1wal1);

      G4LogicalVolume *logi_F6spc6 = new G4LogicalVolume(geo_F6spc6, mat_F6spc6, "logi_F6spc6_name");

      //put volume
      setColor(*logi_F6spc6, cF6spc6.getString("Color", "#CCCCCC"));
      //setVisibility(*logi_F6spc6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F6spc6, "phys_F6spc6_name", logi_F5wal5, false, 0);

      //-
      //--------------


      //--------------
      //-   F7mag1

      //get parameters from .xml file
      GearDir cF7mag1(content, "F7mag1/");
      //
      const int F7mag1_num = 2;
      //
      double F7mag1_Z[F7mag1_num];
      F7mag1_Z[0] = cF7mag1.getLength("D1") / Unit::mm;
      F7mag1_Z[1] = F7mag1_Z[0] + cF7mag1.getLength("L1") / Unit::mm;
      //
      double F7mag1_rI[F7mag1_num];
      for (int i = 0; i < F7mag1_num; i++)
        { F7mag1_rI[i] = 0.0; }
      //
      double F7mag1_rO[F7mag1_num];
      F7mag1_rO[0] = cF7mag1.getLength("O1") / Unit::mm;
      F7mag1_rO[1] = F7mag1_rO[0];
      //
      string strMat_F7mag1 = cF7mag1.getString("Material");
      G4Material* mat_F7mag1 = Materials::get(strMat_F7mag1);

      //define geometry
      G4Polycone* geo_F7mag1x = new G4Polycone("geo_F7mag1x_name", 0, 2*M_PI, F7mag1_num, F7mag1_Z, F7mag1_rI, F7mag1_rO);
      G4IntersectionSolid* geo_F7mag1 = new G4IntersectionSolid("geo_F7mag1_name", geo_F6spc3, geo_F7mag1x, transform_D1wal1);

      G4LogicalVolume *logi_F7mag1 = new G4LogicalVolume(geo_F7mag1, mat_F7mag1, "logi_F7mag1_name");

      //put volume
      setColor(*logi_F7mag1, cF7mag1.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag1, "phys_F7mag1_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7mag2

      //get parameters from .xml file
      GearDir cF7mag2(content, "F7mag2/");
      //
      const int F7mag2_num = 2;
      //
      double F7mag2_Z[F7mag2_num];
      F7mag2_Z[0] = F6spc4_Z[2];
      F7mag2_Z[1] = F7mag2_Z[0] + cF7mag2.getLength("L1") / Unit::mm;
      //
      double F7mag2_rI[F7mag2_num];
      for (int i = 0; i < F7mag2_num; i++)
        { F7mag2_rI[i] = 0.0; }
      //
      double F7mag2_rO[F7mag2_num];
      F7mag2_rO[0] = cF7mag2.getLength("O1") / Unit::mm;
      F7mag2_rO[1] = F7mag2_rO[0];
      //
      string strMat_F7mag2 = cF7mag2.getString("Material");
      G4Material* mat_F7mag2 = Materials::get(strMat_F7mag2);

      //define geometry
      G4Polycone* geo_F7mag2x = new G4Polycone("geo_F7mag2x_name", 0, 2*M_PI, F7mag2_num, F7mag2_Z, F7mag2_rI, F7mag2_rO);
      G4IntersectionSolid* geo_F7mag2 = new G4IntersectionSolid("geo_F7mag2_name", geo_F6spc3, geo_F7mag2x, transform_D1wal1);

      G4LogicalVolume *logi_F7mag2 = new G4LogicalVolume(geo_F7mag2, mat_F7mag2, "logi_F7mag2_name");

      //put volume
      setColor(*logi_F7mag2, cF7mag2.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag2, "phys_F7mag2_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7mag3

      //get parameters from .xml file
      GearDir cF7mag3(content, "F7mag3/");
      //
      const int F7mag3_num = 2;
      //
      double F7mag3_Z[F7mag3_num];
      F7mag3_Z[0] = F6spc4_Z[4];
      F7mag3_Z[1] = F7mag3_Z[0] + cF7mag3.getLength("L1") / Unit::mm;
      //
      double F7mag3_rI[F7mag3_num];
      for (int i = 0; i < F7mag3_num; i++)
        { F7mag3_rI[i] = 0.0; }
      //
      double F7mag3_rO[F7mag3_num];
      F7mag3_rO[0] = cF7mag3.getLength("O1") / Unit::mm;
      F7mag3_rO[1] = F7mag3_rO[0];
      //
      string strMat_F7mag3 = cF7mag3.getString("Material");
      G4Material* mat_F7mag3 = Materials::get(strMat_F7mag3);

      //define geometry
      G4Polycone* geo_F7mag3x = new G4Polycone("geo_F7mag3_name", 0, 2*M_PI, F7mag3_num, F7mag3_Z, F7mag3_rI, F7mag3_rO);
      G4IntersectionSolid* geo_F7mag3 = new G4IntersectionSolid("geo_F7mag3_name", geo_F6spc3, geo_F7mag3x, transform_D1wal1);

      G4LogicalVolume *logi_F7mag3 = new G4LogicalVolume(geo_F7mag3, mat_F7mag3, "logi_F7mag3_name");

      //put volume
      setColor(*logi_F7mag3, cF7mag3.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag3, "phys_F7mag3_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7mag4

      //get parameters from .xml file
      GearDir cF7mag4(content, "F7mag4/");
      //
      const int F7mag4_num = 2;
      //
      double F7mag4_Z[F7mag4_num];
      F7mag4_Z[0] = cF7mag4.getLength("D1") / Unit::mm;
      F7mag4_Z[1] = F7mag4_Z[0] + cF7mag4.getLength("L1") / Unit::mm;
      //
      double F7mag4_rI[F7mag4_num];
      for (int i = 0; i < F7mag4_num; i++)
        { F7mag4_rI[i] = 0.0; }
      //
      double F7mag4_rO[F7mag4_num];
      F7mag4_rO[0] = cF7mag4.getLength("O1") / Unit::mm;
      F7mag4_rO[1] = F7mag4_rO[0];
      //
      string strMat_F7mag4 = cF7mag4.getString("Material");
      G4Material* mat_F7mag4 = Materials::get(strMat_F7mag4);

      //define geometry
      G4Polycone* geo_F7mag4x = new G4Polycone("geo_F7mag4x_name", 0, 2*M_PI, F7mag4_num, F7mag4_Z, F7mag4_rI, F7mag4_rO);
      G4IntersectionSolid* geo_F7mag4 = new G4IntersectionSolid("geo_F7mag4_name", geo_F6spc3, geo_F7mag4x, transform_E1wal1);

      G4LogicalVolume *logi_F7mag4 = new G4LogicalVolume(geo_F7mag4, mat_F7mag4, "logi_F7mag4_name");

      //put volume
      setColor(*logi_F7mag4, cF7mag4.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag4, "phys_F7mag4_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7mag5

      //get parameters from .xml file
      GearDir cF7mag5(content, "F7mag5/");
      //
      const int F7mag5_num = 2;
      //
      double F7mag5_Z[F7mag5_num];
      F7mag5_Z[0] = F6spc5_Z[3];
      F7mag5_Z[1] = F7mag5_Z[0] + cF7mag5.getLength("L1") / Unit::mm;
      //
      double F7mag5_rI[F7mag5_num];
      for (int i = 0; i < F7mag5_num; i++)
        { F7mag5_rI[i] = 0.0; }
      //
      double F7mag5_rO[F7mag5_num];
      F7mag5_rO[0] = cF7mag5.getLength("O1") / Unit::mm;
      F7mag5_rO[1] = F7mag5_rO[0];
      //
      string strMat_F7mag5 = cF7mag5.getString("Material");
      G4Material* mat_F7mag5 = Materials::get(strMat_F7mag5);

      //define geometry
      G4Polycone* geo_F7mag5x = new G4Polycone("geo_F7mag5x_name", 0, 2*M_PI, F7mag5_num, F7mag5_Z, F7mag5_rI, F7mag5_rO);
      G4IntersectionSolid* geo_F7mag5 = new G4IntersectionSolid("geo_F7mag5_name", geo_F6spc3, geo_F7mag5x, transform_E1wal1);

      G4LogicalVolume *logi_F7mag5 = new G4LogicalVolume(geo_F7mag5, mat_F7mag5, "logi_F7mag5_name");

      //put volume
      setColor(*logi_F7mag5, cF7mag5.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag5, "phys_F7mag5_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7mag6

      //get parameters from .xml file
      GearDir cF7mag6(content, "F7mag6/");
      //
      const int F7mag6_num = 2;
      //
      double F7mag6_Z[F7mag6_num];
      F7mag6_Z[0] = F6spc5_Z[6];
      F7mag6_Z[1] = F7mag6_Z[0] + cF7mag6.getLength("L1") / Unit::mm;
      //
      double F7mag6_rI[F7mag6_num];
      for (int i = 0; i < F7mag6_num; i++)
        { F7mag6_rI[i] = 0.0; }
      //
      double F7mag6_rO[F7mag6_num];
      F7mag6_rO[0] = cF7mag6.getLength("O1") / Unit::mm;
      F7mag6_rO[1] = F7mag6_rO[0];
      //
      string strMat_F7mag6 = cF7mag6.getString("Material");
      G4Material* mat_F7mag6 = Materials::get(strMat_F7mag6);

      //define geometry
      G4Polycone* geo_F7mag6x = new G4Polycone("geo_F7mag6x_name", 0, 2*M_PI, F7mag6_num, F7mag6_Z, F7mag6_rI, F7mag6_rO);
      G4IntersectionSolid* geo_F7mag6 = new G4IntersectionSolid("geo_F7mag6_name", geo_F6spc3, geo_F7mag6x, transform_E1wal1);

      G4LogicalVolume *logi_F7mag6 = new G4LogicalVolume(geo_F7mag6, mat_F7mag6, "logi_F7mag6_name");

      //put volume
      setColor(*logi_F7mag6, cF7mag6.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag6, "phys_F7mag6_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7mag7

      //get parameters from .xml file
      GearDir cF7mag7(content, "F7mag7/");
      //
      const int F7mag7_num = 2;
      //
      double F7mag7_Z[F7mag7_num];
      F7mag7_Z[0] = cF7mag7.getLength("D1") / Unit::mm;
      F7mag7_Z[1] = F7mag7_Z[0] + cF7mag7.getLength("L1") / Unit::mm;
      //
      double F7mag7_rI[F7mag7_num];
      for (int i = 0; i < F7mag7_num; i++)
        { F7mag7_rI[i] = 0.0; }
      //
      double F7mag7_rO[F7mag7_num];
      F7mag7_rO[0] = cF7mag7.getLength("O1") / Unit::mm;
      F7mag7_rO[1] = F7mag7_rO[0];
      //
      string strMat_F7mag7 = cF7mag7.getString("Material");
      G4Material* mat_F7mag7 = Materials::get(strMat_F7mag7);

      //define geometry
      G4Polycone* geo_F7mag7x = new G4Polycone("geo_F7mag7x_name", 0, 2*M_PI, F7mag7_num, F7mag7_Z, F7mag7_rI, F7mag7_rO);
      G4IntersectionSolid* geo_F7mag7 = new G4IntersectionSolid("geo_F7mag7_name", geo_F6spc6, geo_F7mag7x, transform_D1wal1);

      G4LogicalVolume *logi_F7mag7 = new G4LogicalVolume(geo_F7mag7, mat_F7mag7, "logi_F7mag7_name");

      //put volume
      setColor(*logi_F7mag7, cF7mag7.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7mag7, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7mag7, "phys_F7mag7_name", logi_F6spc6, false, 0);

      //-
      //--------------

      //--------------
      //-   F7hld1

      //get parameters from .xml file
      GearDir cF7hld1(content, "F7hld1/");
      //
      double F7hld1_T[16];
      for (int tmpn = 0; tmpn < 16; tmpn++) {
        F7hld1_T[tmpn] = cF7hld1.getLength((boost::format("T%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double F7hld1_D[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        F7hld1_D[tmpn] = cF7hld1.getLength((boost::format("D%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double F7hld1_I[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        F7hld1_I[tmpn] = cF7hld1.getLength((boost::format("I%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double F7hld1_R[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        F7hld1_R[tmpn] = cF7hld1.getLength((boost::format("R%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      double F7hld1_O[16];
      for (int tmpn = 0; tmpn < 16; tmpn++) {
        F7hld1_O[tmpn] = cF7hld1.getLength((boost::format("O%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }

      const int F7hld1_num = 62;
      //
      double F7hld1_Z[F7hld1_num];
      F7hld1_Z[0] = F6spc3_Z[0];
      F7hld1_Z[1] = F7hld1_Z[0] + F7hld1_T[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        F7hld1_Z[4*tmpn + 2] = F7hld1_Z[4*tmpn + 1];
        F7hld1_Z[4*tmpn + 3] = F7hld1_Z[4*tmpn + 2] + F7hld1_D[tmpn];
        F7hld1_Z[4*tmpn + 4] = F7hld1_Z[4*tmpn + 3];
        F7hld1_Z[4*tmpn + 5] = F7hld1_Z[4*tmpn + 4] + F7hld1_T[tmpn+1];
      }
      //
      double F7hld1_rI[F7hld1_num];
      F7hld1_rI[0] = F7hld1_I[0];
      F7hld1_rI[1] = F7hld1_I[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        F7hld1_rI[4*tmpn + 2] = F7hld1_I[tmpn];
        F7hld1_rI[4*tmpn + 3] = F7hld1_I[tmpn];
        F7hld1_rI[4*tmpn + 4] = F7hld1_I[tmpn];
        F7hld1_rI[4*tmpn + 5] = F7hld1_I[tmpn];
      }
      //
      double F7hld1_rO[F7hld1_num];
      F7hld1_rO[0] = F7hld1_O[0];
      F7hld1_rO[1] = F7hld1_O[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        F7hld1_rO[4*tmpn + 2] = F7hld1_R[tmpn];
        F7hld1_rO[4*tmpn + 3] = F7hld1_R[tmpn];
        F7hld1_rO[4*tmpn + 4] = F7hld1_O[tmpn + 1];
        F7hld1_rO[4*tmpn + 5] = F7hld1_O[tmpn + 1];
      }
      //
      string strMat_F7hld1 = cF7hld1.getString("Material");
      G4Material* mat_F7hld1 = Materials::get(strMat_F7hld1);

      for (int i=0; i< F7hld1_num; i++) printf("%f %f %f\n", F7hld1_Z[i], F7hld1_rI[i], F7hld1_rO[i]);

      //define geometry
      G4Polycone* geo_F7hld1x7 = new G4Polycone("geo_F7hld1x7_name", 0, 2*M_PI, F7hld1_num, F7hld1_Z, F7hld1_rI, F7hld1_rO);
      //G4SubtractionSolid* geo_F7hld1x6 = new G4SubtractionSolid("geo_F7hld1x6_name", geo_F7hld1x7, geo_F7mag6, transform_E1wal1);
      //G4SubtractionSolid* geo_F7hld1x5 = new G4SubtractionSolid("geo_F7hld1x5_name", geo_F7hld1x6, geo_F7mag5, transform_E1wal1);
      //G4SubtractionSolid* geo_F7hld1x4 = new G4SubtractionSolid("geo_F7hld1x4_name", geo_F7hld1x5, geo_F7mag4, transform_E1wal1);
      //G4SubtractionSolid* geo_F7hld1x3 = new G4SubtractionSolid("geo_F7hld1x3_name", geo_F7hld1x4, geo_F7mag3, transform_D1wal1);
      //G4SubtractionSolid* geo_F7hld1x2 = new G4SubtractionSolid("geo_F7hld1x2_name", geo_F7hld1x3, geo_F7mag2, transform_D1wal1);
      //G4SubtractionSolid* geo_F7hld1x1 = new G4SubtractionSolid("geo_F7hld1x1_name", geo_F7hld1x2, geo_F7mag1, transform_D1wal1);
      G4IntersectionSolid* geo_F7hld1 = new G4IntersectionSolid("geo_F7hld1_name", geo_F7hld1x7, geo_F6spc3);

      G4LogicalVolume *logi_F7hld1 = new G4LogicalVolume(geo_F7hld1, mat_F7hld1, "logi_F7hld1_name");

      //put volume
      setColor(*logi_F7hld1, cF7hld1.getString("Color", "#CC0000"));
      //setVisibility(*logi_F7hld1, false);

      //Now F7hld1 goes under F7cil1. F7hld1 will be placed after F7cil1.
      //new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7hld1, "phys_F7hld1_name", logi_F6spc3, false, 0);

      //-
      //--------------

      //--------------
      //-   F7cil1

      //get parameters from .xml file
      GearDir cF7cil1(content, "F7cil1/");
      //
      double F7cil1_L[15];
      for (int tmpn = 0; tmpn < 15; tmpn++) {
        F7cil1_L[tmpn] = cF7cil1.getLength((boost::format("L%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }

      double F7cil1_rO[F7hld1_num];
      F7cil1_rO[0] = F7hld1_O[0];
      F7cil1_rO[1] = F7hld1_O[0];
      for (int tmpn = 0; tmpn <= 14; tmpn++) {
        F7cil1_rO[4*tmpn + 2] = F7hld1_R[tmpn] + F7cil1_L[tmpn];
        F7cil1_rO[4*tmpn + 3] = F7hld1_R[tmpn] + F7cil1_L[tmpn];
        F7cil1_rO[4*tmpn + 4] = F7hld1_O[tmpn + 1];
        F7cil1_rO[4*tmpn + 5] = F7hld1_O[tmpn + 1];
      }
      //
      string strMat_F7cil1 = cF7cil1.getString("Material");
      G4Material* mat_F7cil1 = Materials::get(strMat_F7cil1);

      for (int i=0; i< F7hld1_num; i++) printf("%f %f %f\n", F7hld1_Z[i], F7hld1_rI[i], F7cil1_rO[i]);

      //define geometry
      G4Polycone* geo_F7cil1x7 = new G4Polycone("geo_F7cil1x7_name", 0, 2*M_PI, F7hld1_num, F7hld1_Z, F7hld1_rI, F7cil1_rO);
      //G4SubtractionSolid* geo_F7cil1x6 = new G4SubtractionSolid("geo_F7cil1x6_name", geo_F7cil1x7, geo_F7mag6, transform_E1wal1);
      //G4SubtractionSolid* geo_F7cil1x5 = new G4SubtractionSolid("geo_F7cil1x5_name", geo_F7cil1x6, geo_F7mag5, transform_E1wal1);
      //G4SubtractionSolid* geo_F7cil1x4 = new G4SubtractionSolid("geo_F7cil1x4_name", geo_F7cil1x5, geo_F7mag4, transform_E1wal1);
      //G4SubtractionSolid* geo_F7cil1x3 = new G4SubtractionSolid("geo_F7cil1x3_name", geo_F7cil1x4, geo_F7mag3, transform_D1wal1);
      //G4SubtractionSolid* geo_F7cil1x2 = new G4SubtractionSolid("geo_F7cil1x2_name", geo_F7cil1x3, geo_F7mag2, transform_D1wal1);
      //G4SubtractionSolid* geo_F7cil1x1 = new G4SubtractionSolid("geo_F7cil1x1_name", geo_F7cil1x2, geo_F7mag1, transform_D1wal1);
      //G4SubtractionSolid* geo_F7cil1x = new G4SubtractionSolid("geo_F7cil1x_name", geo_F7cil1x7, geo_F7hld1);
      G4IntersectionSolid* geo_F7cil1 = new G4IntersectionSolid("geo_F7cil1_name", geo_F7cil1x7, geo_F6spc3);

      G4LogicalVolume *logi_F7cil1 = new G4LogicalVolume(geo_F7cil1, mat_F7cil1, "logi_F7cil1_name");

      //put volume
      setColor(*logi_F7cil1, cF7cil1.getString("Color", "#CCCC00"));
      //setVisibility(*logi_F7cil1, false);
      //new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7cil1, "phys_F7cil1_name", &topVolume, false, 0);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7cil1, "phys_F7cil1_name", logi_F6spc3, false, 0);

      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7hld1, "phys_F7hld1_name", logi_F7cil1, false, 0);

      //-
      //--------------
      */

    }
  }
}
