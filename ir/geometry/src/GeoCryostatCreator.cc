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
#include <ir/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

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
      m_sensitive = new SensitiveDetector();
    }

    GeoCryostatCreator::~GeoCryostatCreator()
    {
      delete m_sensitive;
    }

    void GeoCryostatCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

      //######  L side index  ######

      // +- A2wal1
      //      +- A2spc1
      // +- B2wal1
      //    +- B2spc1
      // +- C1wal1
      //    +- C2spc1
      //       +- C3wal2
      //          +- C4spc2
      //             +- C5wal3
      //                +- C6spc4
      //                +- C6spc5
      //                +- C6spc3
      //                   +- C7mag1
      //                   +- C7mag2
      //                   +- C7mag3
      //                   +- C7mag4
      //                   +- C7mag5
      //                   +- C7mag6
      //                   +- C7cil1
      //                     +- C8hld1
      //             +- C5wal5
      //                +- C6spc7
      //                +- C6spc8
      //                +- C6spc6
      //                   +- C7mag7

      //######  R side index  ######

      // +- D2wal1
      //    +- D2spc1
      // +- E2wal1
      //    +- E2spc1
      // +- F1wal1
      //    +- F2spc1
      //       +- F3wal2
      //          +- F4spc2
      //             +- F5wal4
      //                +- F6spc5
      //                +- F6spc6
      //                +- F6spc4
      //                   +- F7mag1
      //                   +- F7mag2
      //                   +- F7mag3
      //                   +- F7mag4
      //                   +- F7mag5
      //                   +- F7mag6
      //                   +- F7cil1
      //                      +- F8hld1
      //       +- F3wal3
      //          +- F4spc3
      //             +- F5wal5
      //                +- F6spc8
      //                +- F6spc7
      //                   +- F7mag7

      double stepMax = 5.0 * Unit::mm;
      //bool flag_limitStep = true;
      bool flag_limitStep = false;

      double unitFactor = 10.0;

      G4Transform3D transform_HER = G4Translate3D(0., 0., 0.);
      transform_HER = transform_HER * G4RotateY3D(Unit::crossingAngleHER / Unit::rad);
      G4Transform3D transform_HER_inv = G4Translate3D(0., 0., 0.);
      transform_HER_inv = transform_HER_inv * G4RotateY3D(-Unit::crossingAngleHER / Unit::rad);

      G4Transform3D transform_LER = G4Translate3D(0., 0., 0.);
      transform_LER = transform_LER * G4RotateY3D(Unit::crossingAngleLER / Unit::rad);
      G4Transform3D transform_LER_inv = G4Translate3D(0., 0., 0.);
      transform_LER_inv = transform_LER_inv * G4RotateY3D(-Unit::crossingAngleLER / Unit::rad);

      G4Transform3D transform_LER2HER = G4Translate3D(0., 0., 0.);
      transform_LER2HER = transform_LER2HER * G4RotateY3D(-Unit::crossingAngleLER / Unit::rad + Unit::crossingAngleHER / Unit::rad);
      G4Transform3D transform_HER2LER = G4Translate3D(0., 0., 0.);
      transform_HER2LER = transform_HER2LER * G4RotateY3D(-Unit::crossingAngleHER / Unit::rad + Unit::crossingAngleLER / Unit::rad);

      //--------------
      //-   A1spc1 and B1spc1

      // right bounding form 1
      GearDir cTubeR(content, "TubeR/");

      const int TubeR_num = atoi(cTubeR.getString("N").c_str());

      double TubeR_Z[TubeR_num];
      double TubeR_R[TubeR_num];
      double TubeR_r[TubeR_num];

      for (int i = 0; i < TubeR_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        TubeR_Z[i] = cTubeR.getLength(ossZID.str()) * unitFactor;
        TubeR_R[i] = cTubeR.getLength(ossRID.str()) * unitFactor;
        TubeR_r[i] = 0.0;
      }

      G4Polycone* geo_TubeR = new G4Polycone("geo_TubeR_name", 0, 2 * M_PI, TubeR_num, TubeR_Z, TubeR_r, TubeR_R);

      // right bounding form 2
      GearDir cTube2(content, "Tube2/");

      const int Tube2_num = atoi(cTube2.getString("N").c_str());

      double Tube2_Z[Tube2_num];
      double Tube2_R[Tube2_num];
      double Tube2_r[Tube2_num];

      for (int i = 0; i < Tube2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        Tube2_Z[i] = cTube2.getLength(ossZID.str()) * unitFactor;
        Tube2_R[i] = cTube2.getLength(ossRID.str()) * unitFactor;
        Tube2_r[i] = 0.0;
      }

      G4Polycone* geo_Tube2 = new G4Polycone("geo_Tube2_name", 0, 2 * M_PI, Tube2_num, Tube2_Z, Tube2_r, Tube2_R);

      // space containing all structures around right HER beam pipe, part 1
      GearDir cA1spc1(content, "A1spc1/");

      const int A1spc1_num = atoi(cA1spc1.getString("N").c_str());

      double A1spc1_Z[A1spc1_num];
      double A1spc1_r[A1spc1_num];
      double A1spc1_R[A1spc1_num];

      for (int i = 0; i < A1spc1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        A1spc1_Z[i] = cA1spc1.getLength(ossZID.str()) * unitFactor;
        A1spc1_R[i] = cA1spc1.getLength(ossRID.str()) * unitFactor;
        A1spc1_r[i] = 0.0;
      }

      G4Polycone* geo_A1spc1xx = new G4Polycone("geo_A1spc1xx_name", 0, 2 * M_PI, A1spc1_num, A1spc1_Z, A1spc1_r, A1spc1_R);

      // space containing all structures around right HER beam pipe, part 2
      GearDir cA1spc2(content, "A1spc2/");

      const int A1spc2_num = atoi(cA1spc2.getString("N").c_str());

      double A1spc2_Z[A1spc2_num];
      double A1spc2_R[A1spc2_num];
      double A1spc2_r[A1spc2_num];

      for (int i = 0; i < A1spc2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        A1spc2_Z[i] = cA1spc2.getLength(ossZID.str()) * unitFactor;
        A1spc2_R[i] = cA1spc2.getLength(ossRID.str()) * unitFactor;
        A1spc2_r[i] = 0.0;
      }

      G4Polycone* geo_A1spc2xx = new G4Polycone("geo_A1spc2xx_name", 0, 2 * M_PI, A1spc2_num, A1spc2_Z, A1spc2_r, A1spc2_R);

      // space containing all structures around right LER beam pipe, part 1
      GearDir cB1spc1(content, "B1spc1/");

      const int B1spc1_num = atoi(cB1spc1.getString("N").c_str());

      double B1spc1_Z[B1spc1_num];
      double B1spc1_R[B1spc1_num];
      double B1spc1_r[B1spc1_num];

      for (int i = 0; i < B1spc1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        B1spc1_Z[i] = cB1spc1.getLength(ossZID.str()) * unitFactor;
        B1spc1_R[i] = cB1spc1.getLength(ossRID.str()) * unitFactor;
        B1spc1_r[i] = 0.0;
      }

      G4Polycone* geo_B1spc1xx = new G4Polycone("geo_B1spc1xx_name", 0, 2 * M_PI, B1spc1_num, B1spc1_Z, B1spc1_r, B1spc1_R);

      // space containing all structures around right LER beam pipe, part 2
      GearDir cB1spc2(content, "B1spc2/");

      const int B1spc2_num = atoi(cB1spc2.getString("N").c_str());

      double B1spc2_Z[B1spc2_num];
      double B1spc2_R[B1spc2_num];
      double B1spc2_r[B1spc2_num];

      for (int i = 0; i < B1spc2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        B1spc2_Z[i] = cB1spc2.getLength(ossZID.str()) * unitFactor;
        B1spc2_R[i] = cB1spc2.getLength(ossRID.str()) * unitFactor;
        B1spc2_r[i] = 0.0;
      }

      G4Polycone* geo_B1spc2xx = new G4Polycone("geo_B1spc2xx_name", 0, 2 * M_PI, B1spc2_num, B1spc2_Z, B1spc2_r, B1spc2_R);

      // final cut
      G4IntersectionSolid* geo_B1spc2 = new G4IntersectionSolid("geo_B1spc2_name", geo_B1spc2xx, geo_Tube2, transform_LER_inv);

      G4IntersectionSolid* geo_B1spc1x = new G4IntersectionSolid("geo_B1spc1x_name", geo_B1spc1xx, geo_TubeR, transform_LER_inv);
      G4UnionSolid* geo_B1spc1 = new G4UnionSolid("geo_B1spc1_name", geo_B1spc1x, geo_B1spc2);

      G4IntersectionSolid* geo_A1spc2 = new G4IntersectionSolid("geo_A1spc2_name", geo_A1spc2xx, geo_Tube2, transform_HER_inv);

      G4IntersectionSolid* geo_A1spc1xy = new G4IntersectionSolid("geo_A1spc1xy_name", geo_A1spc1xx, geo_TubeR, transform_HER_inv);
      G4UnionSolid* geo_A1spc1x = new G4UnionSolid("geo_A1spc1x_name", geo_A1spc1xy, geo_A1spc2);
      G4SubtractionSolid* geo_A1spc1 = new G4SubtractionSolid("geo_A1spc1_name", geo_A1spc1x, geo_B1spc1, transform_HER2LER);

      string strMat_A1spc1 = cA1spc1.getString("Material");
      G4Material* mat_A1spc1 = Materials::get(strMat_A1spc1);
      G4LogicalVolume* logi_A1spc1 = new G4LogicalVolume(geo_A1spc1, mat_A1spc1, "logi_A1spc1_name");
      if (flag_limitStep) logi_A1spc1->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*logi_A1spc1, cA1spc1.getString("Color", "#CC0000"));
      //setVisibility(*logi_A1spc1, false);
      new G4PVPlacement(transform_HER, logi_A1spc1, "phys_A1spc1_name", &topVolume, false, 0);

      string strMat_B1spc1 = cB1spc1.getString("Material");
      G4Material* mat_B1spc1 = Materials::get(strMat_B1spc1);
      G4LogicalVolume* logi_B1spc1 = new G4LogicalVolume(geo_B1spc1, mat_B1spc1, "logi_B1spc1_name");
      if (flag_limitStep) logi_B1spc1->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*logi_B1spc1, cB1spc1.getString("Color", "#CC0000"));
      //setVisibility(*logi_B1spc1, false);
      new G4PVPlacement(transform_LER, logi_B1spc1, "phys_B1spc1_name", &topVolume, false, 0);

      //--------------
      //-   A2wal1

      //get parameters from .xml file
      GearDir cA2wal1(content, "A2wal1/");

      const int A2wal1_num = atoi(cA2wal1.getString("N").c_str());

      double A2wal1_Z[A2wal1_num];
      double A2wal1_R[A2wal1_num];
      double A2wal1_r[A2wal1_num];

      for (int i = 0; i < A2wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A2wal1_Z[i] = cA2wal1.getLength(ossZID.str()) * unitFactor;
        A2wal1_R[i] = cA2wal1.getLength(ossRID.str()) * unitFactor;
        A2wal1_r[i] = cA2wal1.getLength(ossrID.str()) * unitFactor;
      }

      G4Polycone* geo_A2wal1xx = new G4Polycone("geo_A2wal1xx_name", 0, 2 * M_PI, A2wal1_num, A2wal1_Z, A2wal1_r, A2wal1_R);
      //G4IntersectionSolid* geo_A2wal1 = new G4IntersectionSolid("geo_A2wal1_name", geo_A2wal1xx, geo_A1spc1);
      G4IntersectionSolid* geo_A2wal1 = new G4IntersectionSolid("geo_A2wal1_name", geo_A2wal1xx, geo_TubeR, transform_HER_inv);

      string strMat_A2wal1 = cA2wal1.getString("Material");
      G4Material* mat_A2wal1 = Materials::get(strMat_A2wal1);
      G4LogicalVolume* logi_A2wal1 = new G4LogicalVolume(geo_A2wal1, mat_A2wal1, "logi_A2wal1_name");

      //put volume
      setColor(*logi_A2wal1, cA2wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_A2wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A2wal1, "phys_A2wal1_name", logi_A1spc1, false, 0);

      //--------------
      //-   A3wal1

      //get parameters from .xml file
      GearDir cA3wal1(content, "A3wal1/");

      const int A3wal1_num = atoi(cA3wal1.getString("N").c_str());

      double A3wal1_Z[A3wal1_num];
      double A3wal1_R[A3wal1_num];
      double A3wal1_r[A3wal1_num];

      for (int i = 0; i < A3wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A3wal1_Z[i] = cA3wal1.getLength(ossZID.str()) * unitFactor;
        A3wal1_R[i] = cA3wal1.getLength(ossRID.str()) * unitFactor;
        A3wal1_r[i] = cA3wal1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A3wal1xx = new G4Polycone("geo_A3wal1xx_name", 0, 2 * M_PI, A3wal1_num, A3wal1_Z, A3wal1_r, A3wal1_R);
      G4IntersectionSolid* geo_A3wal1 = new G4IntersectionSolid("geo_A3wal1_name", geo_A3wal1xx, geo_A1spc1);

      string strMat_A3wal1 = cA3wal1.getString("Material");
      G4Material* mat_A3wal1 = Materials::get(strMat_A3wal1);
      G4LogicalVolume* logi_A3wal1 = new G4LogicalVolume(geo_A3wal1, mat_A3wal1, "logi_A3wal1_name");

      //put volume
      setColor(*logi_A3wal1, cA3wal1.getString("Color", "#CC0000"));
      setVisibility(*logi_A3wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A3wal1, "phys_A3wal1_name", logi_A1spc1, false, 0);

      //--------------
      //-   A3wal2

      //get parameters from .xml file
      GearDir cA3wal2(content, "A3wal2/");

      const int A3wal2_num = atoi(cA3wal2.getString("N").c_str());

      double A3wal2_Z[A3wal2_num];
      double A3wal2_R[A3wal2_num];
      double A3wal2_r[A3wal2_num];

      for (int i = 0; i < A3wal2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A3wal2_Z[i] = cA3wal2.getLength(ossZID.str()) * unitFactor;
        A3wal2_R[i] = cA3wal2.getLength(ossRID.str()) * unitFactor;
        A3wal2_r[i] = cA3wal2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A3wal2 = new G4Polycone("geo_A3wal2xx_name", 0, 2 * M_PI, A3wal2_num, A3wal2_Z, A3wal2_r, A3wal2_R);
      //G4IntersectionSolid* geo_A3wal2 = new G4IntersectionSolid("geo_A3wal2_name", geo_A3wal2xx, geo_A1spc1);

      string strMat_A3wal2 = cA3wal2.getString("Material");
      G4Material* mat_A3wal2 = Materials::get(strMat_A3wal2);
      G4LogicalVolume* logi_A3wal2 = new G4LogicalVolume(geo_A3wal2, mat_A3wal2, "logi_A3wal2_name");

      //put volume
      setColor(*logi_A3wal2, cA3wal2.getString("Color", "#CC0000"));
      //setVisibility(*logi_A3wal2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A3wal2, "phys_A3wal2_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag1

      //get parameters from .xml file
      GearDir cA4mag1(content, "A4mag1/");

      const int A4mag1_num = atoi(cA4mag1.getString("N").c_str());

      double A4mag1_Z[A4mag1_num];
      double A4mag1_R[A4mag1_num];
      double A4mag1_r[A4mag1_num];

      for (int i = 0; i < A4mag1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag1_Z[i] = cA4mag1.getLength(ossZID.str()) * unitFactor;
        A4mag1_R[i] = cA4mag1.getLength(ossRID.str()) * unitFactor;
        A4mag1_r[i] = cA4mag1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag1xx = new G4Polycone("geo_A4mag1xx_name", 0, 2 * M_PI, A4mag1_num, A4mag1_Z, A4mag1_r, A4mag1_R);
      G4IntersectionSolid* geo_A4mag1 = new G4IntersectionSolid("geo_A4mag1_name", geo_A4mag1xx, geo_A1spc1);

      string strMat_A4mag1 = cA4mag1.getString("Material");
      G4Material* mat_A4mag1 = Materials::get(strMat_A4mag1);
      G4LogicalVolume* logi_A4mag1 = new G4LogicalVolume(geo_A4mag1, mat_A4mag1, "logi_A4mag1_name");

      //put volume
      setColor(*logi_A4mag1, cA4mag1.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag1, "phys_A4mag1_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag2p1

      //get parameters from .xml file
      GearDir cA4mag2p1(content, "A4mag2p1/");

      const int A4mag2p1_num = atoi(cA4mag2p1.getString("N").c_str());

      double A4mag2p1_Z[A4mag2p1_num];
      double A4mag2p1_R[A4mag2p1_num];
      double A4mag2p1_r[A4mag2p1_num];

      for (int i = 0; i < A4mag2p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag2p1_Z[i] = cA4mag2p1.getLength(ossZID.str()) * unitFactor;
        A4mag2p1_R[i] = cA4mag2p1.getLength(ossRID.str()) * unitFactor;
        A4mag2p1_r[i] = cA4mag2p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag2p1xx = new G4Polycone("geo_A4mag2p1xx_name", 0, 2 * M_PI, A4mag2p1_num, A4mag2p1_Z, A4mag2p1_r, A4mag2p1_R);
      G4IntersectionSolid* geo_A4mag2p1 = new G4IntersectionSolid("geo_A4mag2p1_name", geo_A4mag2p1xx, geo_A1spc1);

      string strMat_A4mag2p1 = cA4mag2p1.getString("Material");
      G4Material* mat_A4mag2p1 = Materials::get(strMat_A4mag2p1);
      G4LogicalVolume* logi_A4mag2p1 = new G4LogicalVolume(geo_A4mag2p1, mat_A4mag2p1, "logi_A4mag2p1_name");

      //put volume
      setColor(*logi_A4mag2p1, cA4mag2p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag2p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag2p1, "phys_A4mag2p1_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag2p2

      //get parameters from .xml file
      GearDir cA4mag2p2(content, "A4mag2p2/");

      const int A4mag2p2_num = atoi(cA4mag2p2.getString("N").c_str());

      double A4mag2p2_Z[A4mag2p2_num];
      double A4mag2p2_R[A4mag2p2_num];
      double A4mag2p2_r[A4mag2p2_num];

      for (int i = 0; i < A4mag2p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag2p2_Z[i] = cA4mag2p2.getLength(ossZID.str()) * unitFactor;
        A4mag2p2_R[i] = cA4mag2p2.getLength(ossRID.str()) * unitFactor;
        A4mag2p2_r[i] = cA4mag2p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag2p2xx = new G4Polycone("geo_A4mag2p2xx_name", 0, 2 * M_PI, A4mag2p2_num, A4mag2p2_Z, A4mag2p2_r, A4mag2p2_R);
      G4IntersectionSolid* geo_A4mag2p2 = new G4IntersectionSolid("geo_A4mag2p2_name", geo_A4mag2p2xx, geo_A1spc1);

      string strMat_A4mag2p2 = cA4mag2p2.getString("Material");
      G4Material* mat_A4mag2p2 = Materials::get(strMat_A4mag2p2);
      G4LogicalVolume* logi_A4mag2p2 = new G4LogicalVolume(geo_A4mag2p2, mat_A4mag2p2, "logi_A4mag2p2_name");

      //put volume
      setColor(*logi_A4mag2p2, cA4mag2p2.getString("Color", "#CC0000"));
      setVisibility(*logi_A4mag2p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag2p2, "phys_A4mag2p2_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag2p3

      //get parameters from .xml file
      GearDir cA4mag2p3(content, "A4mag2p3/");

      const int A4mag2p3_num = atoi(cA4mag2p3.getString("N").c_str());

      double A4mag2p3_Z[A4mag2p3_num];
      double A4mag2p3_R[A4mag2p3_num];
      double A4mag2p3_r[A4mag2p3_num];

      for (int i = 0; i < A4mag2p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag2p3_Z[i] = cA4mag2p3.getLength(ossZID.str()) * unitFactor;
        A4mag2p3_R[i] = cA4mag2p3.getLength(ossRID.str()) * unitFactor;
        A4mag2p3_r[i] = cA4mag2p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag2p3xx = new G4Polycone("geo_A4mag2p3xx_name", 0, 2 * M_PI, A4mag2p3_num, A4mag2p3_Z, A4mag2p3_r, A4mag2p3_R);
      G4IntersectionSolid* geo_A4mag2p3 = new G4IntersectionSolid("geo_A4mag2p3_name", geo_A4mag2p3xx, geo_A1spc1);

      string strMat_A4mag2p3 = cA4mag2p3.getString("Material");
      G4Material* mat_A4mag2p3 = Materials::get(strMat_A4mag2p3);
      G4LogicalVolume* logi_A4mag2p3 = new G4LogicalVolume(geo_A4mag2p3, mat_A4mag2p3, "logi_A4mag2p3_name");

      //put volume
      setColor(*logi_A4mag2p3, cA4mag2p3.getString("Color", "#CC0000"));
      setVisibility(*logi_A4mag2p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag2p3, "phys_A4mag2p3_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag2p4

      //get parameters from .xml file
      GearDir cA4mag2p4(content, "A4mag2p4/");

      const int A4mag2p4_num = atoi(cA4mag2p4.getString("N").c_str());

      double A4mag2p4_Z[A4mag2p4_num];
      double A4mag2p4_R[A4mag2p4_num];
      double A4mag2p4_r[A4mag2p4_num];

      for (int i = 0; i < A4mag2p4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag2p4_Z[i] = cA4mag2p4.getLength(ossZID.str()) * unitFactor;
        A4mag2p4_R[i] = cA4mag2p4.getLength(ossRID.str()) * unitFactor;
        A4mag2p4_r[i] = cA4mag2p4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag2p4xx = new G4Polycone("geo_A4mag2p4xx_name", 0, 2 * M_PI, A4mag2p4_num, A4mag2p4_Z, A4mag2p4_r, A4mag2p4_R);
      G4IntersectionSolid* geo_A4mag2p4 = new G4IntersectionSolid("geo_A4mag2p4_name", geo_A4mag2p4xx, geo_A1spc1);

      string strMat_A4mag2p4 = cA4mag2p4.getString("Material");
      G4Material* mat_A4mag2p4 = Materials::get(strMat_A4mag2p4);
      G4LogicalVolume* logi_A4mag2p4 = new G4LogicalVolume(geo_A4mag2p4, mat_A4mag2p4, "logi_A4mag2p4_name");

      //put volume
      setColor(*logi_A4mag2p4, cA4mag2p4.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag2p4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag2p4, "phys_A4mag2p4_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag3p1

      //get parameters from .xml file
      GearDir cA4mag3p1(content, "A4mag3p1/");

      const int A4mag3p1_num = atoi(cA4mag3p1.getString("N").c_str());

      double A4mag3p1_Z[A4mag3p1_num];
      double A4mag3p1_R[A4mag3p1_num];
      double A4mag3p1_r[A4mag3p1_num];

      for (int i = 0; i < A4mag3p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag3p1_Z[i] = cA4mag3p1.getLength(ossZID.str()) * unitFactor;
        A4mag3p1_R[i] = cA4mag3p1.getLength(ossRID.str()) * unitFactor;
        A4mag3p1_r[i] = cA4mag3p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag3p1xx = new G4Polycone("geo_A4mag3p1xx_name", 0, 2 * M_PI, A4mag3p1_num, A4mag3p1_Z, A4mag3p1_r, A4mag3p1_R);
      G4IntersectionSolid* geo_A4mag3p1 = new G4IntersectionSolid("geo_A4mag3p1_name", geo_A4mag3p1xx, geo_A1spc1);

      string strMat_A4mag3p1 = cA4mag3p1.getString("Material");
      G4Material* mat_A4mag3p1 = Materials::get(strMat_A4mag3p1);
      G4LogicalVolume* logi_A4mag3p1 = new G4LogicalVolume(geo_A4mag3p1, mat_A4mag3p1, "logi_A4mag3p1_name");

      //put volume
      setColor(*logi_A4mag3p1, cA4mag3p1.getString("Color", "#CC0000"));
      setVisibility(*logi_A4mag3p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag3p1, "phys_A4mag3p1_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag3p2

      //get parameters from .xml file
      GearDir cA4mag3p2(content, "A4mag3p2/");

      const int A4mag3p2_num = atoi(cA4mag3p2.getString("N").c_str());

      double A4mag3p2_Z[A4mag3p2_num];
      double A4mag3p2_R[A4mag3p2_num];
      double A4mag3p2_r[A4mag3p2_num];

      for (int i = 0; i < A4mag3p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag3p2_Z[i] = cA4mag3p2.getLength(ossZID.str()) * unitFactor;
        A4mag3p2_R[i] = cA4mag3p2.getLength(ossRID.str()) * unitFactor;
        A4mag3p2_r[i] = cA4mag3p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag3p2xx = new G4Polycone("geo_A4mag3p2xx_name", 0, 2 * M_PI, A4mag3p2_num, A4mag3p2_Z, A4mag3p2_r, A4mag3p2_R);
      G4IntersectionSolid* geo_A4mag3p2 = new G4IntersectionSolid("geo_A4mag3p2_name", geo_A4mag3p2xx, geo_A1spc1);

      string strMat_A4mag3p2 = cA4mag3p2.getString("Material");
      G4Material* mat_A4mag3p2 = Materials::get(strMat_A4mag3p2);
      G4LogicalVolume* logi_A4mag3p2 = new G4LogicalVolume(geo_A4mag3p2, mat_A4mag3p2, "logi_A4mag3p2_name");

      //put volume
      setColor(*logi_A4mag3p2, cA4mag3p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag3p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag3p2, "phys_A4mag3p2_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p1

      //get parameters from .xml file
      GearDir cA4mag4p1(content, "A4mag4p1/");

      const int A4mag4p1_num = atoi(cA4mag4p1.getString("N").c_str());

      double A4mag4p1_Z[A4mag4p1_num];
      double A4mag4p1_R[A4mag4p1_num];
      double A4mag4p1_r[A4mag4p1_num];

      for (int i = 0; i < A4mag4p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p1_Z[i] = cA4mag4p1.getLength(ossZID.str()) * unitFactor;
        A4mag4p1_R[i] = cA4mag4p1.getLength(ossRID.str()) * unitFactor;
        A4mag4p1_r[i] = cA4mag4p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p1 = new G4Polycone("geo_A4mag4p1xx_name", 0, 2 * M_PI, A4mag4p1_num, A4mag4p1_Z, A4mag4p1_r, A4mag4p1_R);
      //G4IntersectionSolid* geo_A4mag4p1 = new G4IntersectionSolid("geo_A4mag4p1_name", geo_A4mag4p1xx, geo_A1spc1);

      string strMat_A4mag4p1 = cA4mag4p1.getString("Material");
      G4Material* mat_A4mag4p1 = Materials::get(strMat_A4mag4p1);
      G4LogicalVolume* logi_A4mag4p1 = new G4LogicalVolume(geo_A4mag4p1, mat_A4mag4p1, "logi_A4mag4p1_name");

      //put volume
      setColor(*logi_A4mag4p1, cA4mag4p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p1, "phys_A4mag4p1_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p2

      //get parameters from .xml file
      GearDir cA4mag4p2(content, "A4mag4p2/");

      const int A4mag4p2_num = atoi(cA4mag4p2.getString("N").c_str());

      double A4mag4p2_Z[A4mag4p2_num];
      double A4mag4p2_R[A4mag4p2_num];
      double A4mag4p2_r[A4mag4p2_num];

      for (int i = 0; i < A4mag4p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p2_Z[i] = cA4mag4p2.getLength(ossZID.str()) * unitFactor;
        A4mag4p2_R[i] = cA4mag4p2.getLength(ossRID.str()) * unitFactor;
        A4mag4p2_r[i] = cA4mag4p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p2 = new G4Polycone("geo_A4mag4p2xx_name", 0, 2 * M_PI, A4mag4p2_num, A4mag4p2_Z, A4mag4p2_r, A4mag4p2_R);
      //G4IntersectionSolid* geo_A4mag4p2 = new G4IntersectionSolid("geo_A4mag4p2_name", geo_A4mag4p2xx, geo_A1spc1);

      string strMat_A4mag4p2 = cA4mag4p2.getString("Material");
      G4Material* mat_A4mag4p2 = Materials::get(strMat_A4mag4p2);
      G4LogicalVolume* logi_A4mag4p2 = new G4LogicalVolume(geo_A4mag4p2, mat_A4mag4p2, "logi_A4mag4p2_name");

      //put volume
      setColor(*logi_A4mag4p2, cA4mag4p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p2, "phys_A4mag4p2_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p3

      //get parameters from .xml file
      GearDir cA4mag4p3(content, "A4mag4p3/");

      const int A4mag4p3_num = atoi(cA4mag4p3.getString("N").c_str());

      double A4mag4p3_Z[A4mag4p3_num];
      double A4mag4p3_R[A4mag4p3_num];
      double A4mag4p3_r[A4mag4p3_num];

      for (int i = 0; i < A4mag4p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p3_Z[i] = cA4mag4p3.getLength(ossZID.str()) * unitFactor;
        A4mag4p3_R[i] = cA4mag4p3.getLength(ossRID.str()) * unitFactor;
        A4mag4p3_r[i] = cA4mag4p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p3 = new G4Polycone("geo_A4mag4p3xx_name", 0, 2 * M_PI, A4mag4p3_num, A4mag4p3_Z, A4mag4p3_r, A4mag4p3_R);
      //G4IntersectionSolid* geo_A4mag4p3 = new G4IntersectionSolid("geo_A4mag4p3_name", geo_A4mag4p3xx, geo_A1spc1);

      string strMat_A4mag4p3 = cA4mag4p3.getString("Material");
      G4Material* mat_A4mag4p3 = Materials::get(strMat_A4mag4p3);
      G4LogicalVolume* logi_A4mag4p3 = new G4LogicalVolume(geo_A4mag4p3, mat_A4mag4p3, "logi_A4mag4p3_name");

      //put volume
      setColor(*logi_A4mag4p3, cA4mag4p3.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p3, "phys_A4mag4p3_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p4

      //get parameters from .xml file
      GearDir cA4mag4p4(content, "A4mag4p4/");

      const int A4mag4p4_num = atoi(cA4mag4p4.getString("N").c_str());

      double A4mag4p4_Z[A4mag4p4_num];
      double A4mag4p4_R[A4mag4p4_num];
      double A4mag4p4_r[A4mag4p4_num];

      for (int i = 0; i < A4mag4p4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p4_Z[i] = cA4mag4p4.getLength(ossZID.str()) * unitFactor;
        A4mag4p4_R[i] = cA4mag4p4.getLength(ossRID.str()) * unitFactor;
        A4mag4p4_r[i] = cA4mag4p4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p4 = new G4Polycone("geo_A4mag4p4xx_name", 0, 2 * M_PI, A4mag4p4_num, A4mag4p4_Z, A4mag4p4_r, A4mag4p4_R);
      //G4IntersectionSolid* geo_A4mag4p4 = new G4IntersectionSolid("geo_A4mag4p4_name", geo_A4mag4p4xx, geo_A1spc1);

      string strMat_A4mag4p4 = cA4mag4p4.getString("Material");
      G4Material* mat_A4mag4p4 = Materials::get(strMat_A4mag4p4);
      G4LogicalVolume* logi_A4mag4p4 = new G4LogicalVolume(geo_A4mag4p4, mat_A4mag4p4, "logi_A4mag4p4_name");

      //put volume
      setColor(*logi_A4mag4p4, cA4mag4p4.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p4, "phys_A4mag4p4_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p5

      //get parameters from .xml file
      GearDir cA4mag4p5(content, "A4mag4p5/");

      const int A4mag4p5_num = atoi(cA4mag4p5.getString("N").c_str());

      double A4mag4p5_Z[A4mag4p5_num];
      double A4mag4p5_R[A4mag4p5_num];
      double A4mag4p5_r[A4mag4p5_num];

      for (int i = 0; i < A4mag4p5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p5_Z[i] = cA4mag4p5.getLength(ossZID.str()) * unitFactor;
        A4mag4p5_R[i] = cA4mag4p5.getLength(ossRID.str()) * unitFactor;
        A4mag4p5_r[i] = cA4mag4p5.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p5 = new G4Polycone("geo_A4mag4p5xx_name", 0, 2 * M_PI, A4mag4p5_num, A4mag4p5_Z, A4mag4p5_r, A4mag4p5_R);
      //G4IntersectionSolid* geo_A4mag4p5 = new G4IntersectionSolid("geo_A4mag4p5_name", geo_A4mag4p5xx, geo_A1spc1);

      string strMat_A4mag4p5 = cA4mag4p5.getString("Material");
      G4Material* mat_A4mag4p5 = Materials::get(strMat_A4mag4p5);
      G4LogicalVolume* logi_A4mag4p5 = new G4LogicalVolume(geo_A4mag4p5, mat_A4mag4p5, "logi_A4mag4p5_name");

      //put volume
      setColor(*logi_A4mag4p5, cA4mag4p5.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p5, "phys_A4mag4p5_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p6

      //get parameters from .xml file
      GearDir cA4mag4p6(content, "A4mag4p6/");

      const int A4mag4p6_num = atoi(cA4mag4p6.getString("N").c_str());

      double A4mag4p6_Z[A4mag4p6_num];
      double A4mag4p6_R[A4mag4p6_num];
      double A4mag4p6_r[A4mag4p6_num];

      for (int i = 0; i < A4mag4p6_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p6_Z[i] = cA4mag4p6.getLength(ossZID.str()) * unitFactor;
        A4mag4p6_R[i] = cA4mag4p6.getLength(ossRID.str()) * unitFactor;
        A4mag4p6_r[i] = cA4mag4p6.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p6 = new G4Polycone("geo_A4mag4p6xx_name", 0, 2 * M_PI, A4mag4p6_num, A4mag4p6_Z, A4mag4p6_r, A4mag4p6_R);
      //G4IntersectionSolid* geo_A4mag4p6 = new G4IntersectionSolid("geo_A4mag4p6_name", geo_A4mag4p6xx, geo_A1spc1);

      string strMat_A4mag4p6 = cA4mag4p6.getString("Material");
      G4Material* mat_A4mag4p6 = Materials::get(strMat_A4mag4p6);
      G4LogicalVolume* logi_A4mag4p6 = new G4LogicalVolume(geo_A4mag4p6, mat_A4mag4p6, "logi_A4mag4p6_name");

      //put volume
      setColor(*logi_A4mag4p6, cA4mag4p6.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p6, "phys_A4mag4p6_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p7

      //get parameters from .xml file
      GearDir cA4mag4p7(content, "A4mag4p7/");

      const int A4mag4p7_num = atoi(cA4mag4p7.getString("N").c_str());

      double A4mag4p7_Z[A4mag4p7_num];
      double A4mag4p7_R[A4mag4p7_num];
      double A4mag4p7_r[A4mag4p7_num];

      for (int i = 0; i < A4mag4p7_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p7_Z[i] = cA4mag4p7.getLength(ossZID.str()) * unitFactor;
        A4mag4p7_R[i] = cA4mag4p7.getLength(ossRID.str()) * unitFactor;
        A4mag4p7_r[i] = cA4mag4p7.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p7 = new G4Polycone("geo_A4mag4p7xx_name", 0, 2 * M_PI, A4mag4p7_num, A4mag4p7_Z, A4mag4p7_r, A4mag4p7_R);
      //G4IntersectionSolid* geo_A4mag4p7 = new G4IntersectionSolid("geo_A4mag4p7_name", geo_A4mag4p7xx, geo_A1spc1);

      string strMat_A4mag4p7 = cA4mag4p7.getString("Material");
      G4Material* mat_A4mag4p7 = Materials::get(strMat_A4mag4p7);
      G4LogicalVolume* logi_A4mag4p7 = new G4LogicalVolume(geo_A4mag4p7, mat_A4mag4p7, "logi_A4mag4p7_name");

      //put volume
      setColor(*logi_A4mag4p7, cA4mag4p7.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p7, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p7, "phys_A4mag4p7_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p8

      //get parameters from .xml file
      GearDir cA4mag4p8(content, "A4mag4p8/");

      const int A4mag4p8_num = atoi(cA4mag4p8.getString("N").c_str());

      double A4mag4p8_Z[A4mag4p8_num];
      double A4mag4p8_R[A4mag4p8_num];
      double A4mag4p8_r[A4mag4p8_num];

      for (int i = 0; i < A4mag4p8_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p8_Z[i] = cA4mag4p8.getLength(ossZID.str()) * unitFactor;
        A4mag4p8_R[i] = cA4mag4p8.getLength(ossRID.str()) * unitFactor;
        A4mag4p8_r[i] = cA4mag4p8.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p8 = new G4Polycone("geo_A4mag4p8xx_name", 0, 2 * M_PI, A4mag4p8_num, A4mag4p8_Z, A4mag4p8_r, A4mag4p8_R);
      //G4IntersectionSolid* geo_A4mag4p8 = new G4IntersectionSolid("geo_A4mag4p8_name", geo_A4mag4p8xx, geo_A1spc1);

      string strMat_A4mag4p8 = cA4mag4p8.getString("Material");
      G4Material* mat_A4mag4p8 = Materials::get(strMat_A4mag4p8);
      G4LogicalVolume* logi_A4mag4p8 = new G4LogicalVolume(geo_A4mag4p8, mat_A4mag4p8, "logi_A4mag4p8_name");

      //put volume
      setColor(*logi_A4mag4p8, cA4mag4p8.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p8, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p8, "phys_A4mag4p8_name", logi_A1spc1, false, 0);

      //--------------
      //-   A4mag4p9

      //get parameters from .xml file
      GearDir cA4mag4p9(content, "A4mag4p9/");

      const int A4mag4p9_num = atoi(cA4mag4p9.getString("N").c_str());

      double A4mag4p9_Z[A4mag4p9_num];
      double A4mag4p9_R[A4mag4p9_num];
      double A4mag4p9_r[A4mag4p9_num];

      for (int i = 0; i < A4mag4p9_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A4mag4p9_Z[i] = cA4mag4p9.getLength(ossZID.str()) * unitFactor;
        A4mag4p9_R[i] = cA4mag4p9.getLength(ossRID.str()) * unitFactor;
        A4mag4p9_r[i] = cA4mag4p9.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_A4mag4p9 = new G4Polycone("geo_A4mag4p9xx_name", 0, 2 * M_PI, A4mag4p9_num, A4mag4p9_Z, A4mag4p9_r, A4mag4p9_R);
      //G4IntersectionSolid* geo_A4mag4p9 = new G4IntersectionSolid("geo_A4mag4p9_name", geo_A4mag4p9xx, geo_A1spc1);

      string strMat_A4mag4p9 = cA4mag4p9.getString("Material");
      G4Material* mat_A4mag4p9 = Materials::get(strMat_A4mag4p9);
      G4LogicalVolume* logi_A4mag4p9 = new G4LogicalVolume(geo_A4mag4p9, mat_A4mag4p9, "logi_A4mag4p9_name");

      //put volume
      setColor(*logi_A4mag4p9, cA4mag4p9.getString("Color", "#CC0000"));
      //setVisibility(*logi_A4mag4p9, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_A4mag4p9, "phys_A4mag4p9_name", logi_A1spc1, false, 0);

      //--------------
      //-   B2wal1

      //get parameters from .xml file
      GearDir cB2wal1(content, "B2wal1/");

      const int B2wal1_num = atoi(cB2wal1.getString("N").c_str());

      double B2wal1_Z[B2wal1_num];
      double B2wal1_R[B2wal1_num];
      double B2wal1_r[B2wal1_num];

      for (int i = 0; i < B2wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B2wal1_Z[i] = cB2wal1.getLength(ossZID.str()) * unitFactor;
        B2wal1_R[i] = cB2wal1.getLength(ossRID.str()) * unitFactor;
        B2wal1_r[i] = cB2wal1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B2wal1xx = new G4Polycone("geo_B2wal1xx_name", 0, 2 * M_PI, B2wal1_num, B2wal1_Z, B2wal1_r, B2wal1_R);
      //G4IntersectionSolid* geo_B2wal1 = new G4IntersectionSolid("geo_B2wal1_name", geo_B2wal1xx, geo_B1spc1);
      G4IntersectionSolid* geo_B2wal1 = new G4IntersectionSolid("geo_B2wal1_name", geo_B2wal1xx, geo_TubeR, transform_LER_inv);

      string strMat_B2wal1 = cB2wal1.getString("Material");
      G4Material* mat_B2wal1 = Materials::get(strMat_B2wal1);
      G4LogicalVolume* logi_B2wal1 = new G4LogicalVolume(geo_B2wal1, mat_B2wal1, "logi_B2wal1_name");

      //put volume
      setColor(*logi_B2wal1, cB2wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_B2wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B2wal1, "phys_B2wal1_name", logi_B1spc1, false, 0);

      //--------------
      //-   B3wal1

      //get parameters from .xml file
      GearDir cB3wal1(content, "B3wal1/");

      const int B3wal1_num = atoi(cB3wal1.getString("N").c_str());

      double B3wal1_Z[B3wal1_num];
      double B3wal1_R[B3wal1_num];
      double B3wal1_r[B3wal1_num];

      for (int i = 0; i < B3wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B3wal1_Z[i] = cB3wal1.getLength(ossZID.str()) * unitFactor;
        B3wal1_R[i] = cB3wal1.getLength(ossRID.str()) * unitFactor;
        B3wal1_r[i] = cB3wal1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B3wal1xx = new G4Polycone("geo_B3wal1xx_name", 0, 2 * M_PI, B3wal1_num, B3wal1_Z, B3wal1_r, B3wal1_R);
      G4IntersectionSolid* geo_B3wal1 = new G4IntersectionSolid("geo_B3wal1_name", geo_B3wal1xx, geo_B1spc1);

      string strMat_B3wal1 = cB3wal1.getString("Material");
      G4Material* mat_B3wal1 = Materials::get(strMat_B3wal1);
      G4LogicalVolume* logi_B3wal1 = new G4LogicalVolume(geo_B3wal1, mat_B3wal1, "logi_B3wal1_name");

      //put volume
      setColor(*logi_B3wal1, cB3wal1.getString("Color", "#CC0000"));
      setVisibility(*logi_B3wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B3wal1, "phys_B3wal1_name", logi_B1spc1, false, 0);

      //--------------
      //-   B3wal2

      //get parameters from .xml file
      GearDir cB3wal2(content, "B3wal2/");

      const int B3wal2_num = atoi(cB3wal2.getString("N").c_str());

      double B3wal2_Z[B3wal2_num];
      double B3wal2_R[B3wal2_num];
      double B3wal2_r[B3wal2_num];

      for (int i = 0; i < B3wal2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B3wal2_Z[i] = cB3wal2.getLength(ossZID.str()) * unitFactor;
        B3wal2_R[i] = cB3wal2.getLength(ossRID.str()) * unitFactor;
        B3wal2_r[i] = cB3wal2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B3wal2 = new G4Polycone("geo_B3wal2xx_name", 0, 2 * M_PI, B3wal2_num, B3wal2_Z, B3wal2_r, B3wal2_R);
      //G4IntersectionSolid* geo_B3wal2 = new G4IntersectionSolid("geo_B3wal2_name", geo_B3wal2xx, geo_B1spc1);

      string strMat_B3wal2 = cB3wal2.getString("Material");
      G4Material* mat_B3wal2 = Materials::get(strMat_B3wal2);
      G4LogicalVolume* logi_B3wal2 = new G4LogicalVolume(geo_B3wal2, mat_B3wal2, "logi_B3wal2_name");

      //put volume
      setColor(*logi_B3wal2, cB3wal2.getString("Color", "#CC0000"));
      //setVisibility(*logi_B3wal2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B3wal2, "phys_B3wal2_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag1p1

      //get parameters from .xml file
      GearDir cB4mag1p1(content, "B4mag1p1/");

      const int B4mag1p1_num = atoi(cB4mag1p1.getString("N").c_str());

      double B4mag1p1_Z[B4mag1p1_num];
      double B4mag1p1_R[B4mag1p1_num];
      double B4mag1p1_r[B4mag1p1_num];

      for (int i = 0; i < B4mag1p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag1p1_Z[i] = cB4mag1p1.getLength(ossZID.str()) * unitFactor;
        B4mag1p1_R[i] = cB4mag1p1.getLength(ossRID.str()) * unitFactor;
        B4mag1p1_r[i] = cB4mag1p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag1p1 = new G4Polycone("geo_B4mag1p1xx_name", 0, 2 * M_PI, B4mag1p1_num, B4mag1p1_Z, B4mag1p1_r, B4mag1p1_R);
      //G4IntersectionSolid* geo_B4mag1p1 = new G4IntersectionSolid("geo_B4mag1p1_name", geo_B4mag1p1xx, geo_B1spc1);

      string strMat_B4mag1p1 = cB4mag1p1.getString("Material");
      G4Material* mat_B4mag1p1 = Materials::get(strMat_B4mag1p1);
      G4LogicalVolume* logi_B4mag1p1 = new G4LogicalVolume(geo_B4mag1p1, mat_B4mag1p1, "logi_B4mag1p1_name");

      //put volume
      setColor(*logi_B4mag1p1, cB4mag1p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag1p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag1p1, "phys_B4mag1p1_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag1p2

      //get parameters from .xml file
      GearDir cB4mag1p2(content, "B4mag1p2/");

      const int B4mag1p2_num = atoi(cB4mag1p2.getString("N").c_str());

      double B4mag1p2_Z[B4mag1p2_num];
      double B4mag1p2_R[B4mag1p2_num];
      double B4mag1p2_r[B4mag1p2_num];

      for (int i = 0; i < B4mag1p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag1p2_Z[i] = cB4mag1p2.getLength(ossZID.str()) * unitFactor;
        B4mag1p2_R[i] = cB4mag1p2.getLength(ossRID.str()) * unitFactor;
        B4mag1p2_r[i] = cB4mag1p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag1p2 = new G4Polycone("geo_B4mag1p2xx_name", 0, 2 * M_PI, B4mag1p2_num, B4mag1p2_Z, B4mag1p2_r, B4mag1p2_R);
      //G4IntersectionSolid* geo_B4mag1p2 = new G4IntersectionSolid("geo_B4mag1p2_name", geo_B4mag1p2xx, geo_B1spc1);

      string strMat_B4mag1p2 = cB4mag1p2.getString("Material");
      G4Material* mat_B4mag1p2 = Materials::get(strMat_B4mag1p2);
      G4LogicalVolume* logi_B4mag1p2 = new G4LogicalVolume(geo_B4mag1p2, mat_B4mag1p2, "logi_B4mag1p2_name");

      //put volume
      setColor(*logi_B4mag1p2, cB4mag1p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag1p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag1p2, "phys_B4mag1p2_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag1p3

      //get parameters from .xml file
      GearDir cB4mag1p3(content, "B4mag1p3/");

      const int B4mag1p3_num = atoi(cB4mag1p3.getString("N").c_str());

      double B4mag1p3_Z[B4mag1p3_num];
      double B4mag1p3_R[B4mag1p3_num];
      double B4mag1p3_r[B4mag1p3_num];

      for (int i = 0; i < B4mag1p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag1p3_Z[i] = cB4mag1p3.getLength(ossZID.str()) * unitFactor;
        B4mag1p3_R[i] = cB4mag1p3.getLength(ossRID.str()) * unitFactor;
        B4mag1p3_r[i] = cB4mag1p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag1p3 = new G4Polycone("geo_B4mag1p3xx_name", 0, 2 * M_PI, B4mag1p3_num, B4mag1p3_Z, B4mag1p3_r, B4mag1p3_R);
      //G4IntersectionSolid* geo_B4mag1p3 = new G4IntersectionSolid("geo_B4mag1p3_name", geo_B4mag1p3xx, geo_B1spc1);

      string strMat_B4mag1p3 = cB4mag1p3.getString("Material");
      G4Material* mat_B4mag1p3 = Materials::get(strMat_B4mag1p3);
      G4LogicalVolume* logi_B4mag1p3 = new G4LogicalVolume(geo_B4mag1p3, mat_B4mag1p3, "logi_B4mag1p3_name");

      //put volume
      setColor(*logi_B4mag1p3, cB4mag1p3.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag1p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag1p3, "phys_B4mag1p3_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag2

      //get parameters from .xml file
      GearDir cB4mag2(content, "B4mag2/");

      const int B4mag2_num = atoi(cB4mag2.getString("N").c_str());

      double B4mag2_Z[B4mag2_num];
      double B4mag2_R[B4mag2_num];
      double B4mag2_r[B4mag2_num];

      for (int i = 0; i < B4mag2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag2_Z[i] = cB4mag2.getLength(ossZID.str()) * unitFactor;
        B4mag2_R[i] = cB4mag2.getLength(ossRID.str()) * unitFactor;
        B4mag2_r[i] = cB4mag2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag2 = new G4Polycone("geo_B4mag2xx_name", 0, 2 * M_PI, B4mag2_num, B4mag2_Z, B4mag2_r, B4mag2_R);
      //G4IntersectionSolid* geo_B4mag2 = new G4IntersectionSolid("geo_B4mag2_name", geo_B4mag2xx, geo_B1spc1);

      string strMat_B4mag2 = cB4mag2.getString("Material");
      G4Material* mat_B4mag2 = Materials::get(strMat_B4mag2);
      G4LogicalVolume* logi_B4mag2 = new G4LogicalVolume(geo_B4mag2, mat_B4mag2, "logi_B4mag2_name");

      //put volume
      setColor(*logi_B4mag2, cB4mag2.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag2, "phys_B4mag2_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag3p1

      //get parameters from .xml file
      GearDir cB4mag3p1(content, "B4mag3p1/");

      const int B4mag3p1_num = atoi(cB4mag3p1.getString("N").c_str());

      double B4mag3p1_Z[B4mag3p1_num];
      double B4mag3p1_R[B4mag3p1_num];
      double B4mag3p1_r[B4mag3p1_num];

      for (int i = 0; i < B4mag3p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag3p1_Z[i] = cB4mag3p1.getLength(ossZID.str()) * unitFactor;
        B4mag3p1_R[i] = cB4mag3p1.getLength(ossRID.str()) * unitFactor;
        B4mag3p1_r[i] = cB4mag3p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag3p1 = new G4Polycone("geo_B4mag3p1xx_name", 0, 2 * M_PI, B4mag3p1_num, B4mag3p1_Z, B4mag3p1_r, B4mag3p1_R);
      //G4IntersectionSolid* geo_B4mag3p1 = new G4IntersectionSolid("geo_B4mag3p1_name", geo_B4mag3p1xx, geo_B1spc1);

      string strMat_B4mag3p1 = cB4mag3p1.getString("Material");
      G4Material* mat_B4mag3p1 = Materials::get(strMat_B4mag3p1);
      G4LogicalVolume* logi_B4mag3p1 = new G4LogicalVolume(geo_B4mag3p1, mat_B4mag3p1, "logi_B4mag3p1_name");

      //put volume
      setColor(*logi_B4mag3p1, cB4mag3p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag3p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag3p1, "phys_B4mag3p1_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag3p2

      //get parameters from .xml file
      GearDir cB4mag3p2(content, "B4mag3p2/");

      const int B4mag3p2_num = atoi(cB4mag3p2.getString("N").c_str());

      double B4mag3p2_Z[B4mag3p2_num];
      double B4mag3p2_R[B4mag3p2_num];
      double B4mag3p2_r[B4mag3p2_num];

      for (int i = 0; i < B4mag3p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag3p2_Z[i] = cB4mag3p2.getLength(ossZID.str()) * unitFactor;
        B4mag3p2_R[i] = cB4mag3p2.getLength(ossRID.str()) * unitFactor;
        B4mag3p2_r[i] = cB4mag3p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag3p2 = new G4Polycone("geo_B4mag3p2xx_name", 0, 2 * M_PI, B4mag3p2_num, B4mag3p2_Z, B4mag3p2_r, B4mag3p2_R);
      //G4IntersectionSolid* geo_B4mag3p2 = new G4IntersectionSolid("geo_B4mag3p2_name", geo_B4mag3p2xx, geo_B1spc1);

      string strMat_B4mag3p2 = cB4mag3p2.getString("Material");
      G4Material* mat_B4mag3p2 = Materials::get(strMat_B4mag3p2);
      G4LogicalVolume* logi_B4mag3p2 = new G4LogicalVolume(geo_B4mag3p2, mat_B4mag3p2, "logi_B4mag3p2_name");

      //put volume
      setColor(*logi_B4mag3p2, cB4mag3p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag3p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag3p2, "phys_B4mag3p2_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag3p3

      //get parameters from .xml file
      GearDir cB4mag3p3(content, "B4mag3p3/");

      const int B4mag3p3_num = atoi(cB4mag3p3.getString("N").c_str());

      double B4mag3p3_Z[B4mag3p3_num];
      double B4mag3p3_R[B4mag3p3_num];
      double B4mag3p3_r[B4mag3p3_num];

      for (int i = 0; i < B4mag3p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag3p3_Z[i] = cB4mag3p3.getLength(ossZID.str()) * unitFactor;
        B4mag3p3_R[i] = cB4mag3p3.getLength(ossRID.str()) * unitFactor;
        B4mag3p3_r[i] = cB4mag3p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag3p3 = new G4Polycone("geo_B4mag3p3xx_name", 0, 2 * M_PI, B4mag3p3_num, B4mag3p3_Z, B4mag3p3_r, B4mag3p3_R);
      //G4IntersectionSolid* geo_B4mag3p3 = new G4IntersectionSolid("geo_B4mag3p3_name", geo_B4mag3p3xx, geo_B1spc1);

      string strMat_B4mag3p3 = cB4mag3p3.getString("Material");
      G4Material* mat_B4mag3p3 = Materials::get(strMat_B4mag3p3);
      G4LogicalVolume* logi_B4mag3p3 = new G4LogicalVolume(geo_B4mag3p3, mat_B4mag3p3, "logi_B4mag3p3_name");

      //put volume
      setColor(*logi_B4mag3p3, cB4mag3p3.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag3p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag3p3, "phys_B4mag3p3_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag3p4

      //get parameters from .xml file
      GearDir cB4mag3p4(content, "B4mag3p4/");

      const int B4mag3p4_num = atoi(cB4mag3p4.getString("N").c_str());

      double B4mag3p4_Z[B4mag3p4_num];
      double B4mag3p4_R[B4mag3p4_num];
      double B4mag3p4_r[B4mag3p4_num];

      for (int i = 0; i < B4mag3p4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag3p4_Z[i] = cB4mag3p4.getLength(ossZID.str()) * unitFactor;
        B4mag3p4_R[i] = cB4mag3p4.getLength(ossRID.str()) * unitFactor;
        B4mag3p4_r[i] = cB4mag3p4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag3p4 = new G4Polycone("geo_B4mag3p4xx_name", 0, 2 * M_PI, B4mag3p4_num, B4mag3p4_Z, B4mag3p4_r, B4mag3p4_R);
      //G4IntersectionSolid* geo_B4mag3p4 = new G4IntersectionSolid("geo_B4mag3p4_name", geo_B4mag3p4xx, geo_B1spc1);

      string strMat_B4mag3p4 = cB4mag3p4.getString("Material");
      G4Material* mat_B4mag3p4 = Materials::get(strMat_B4mag3p4);
      G4LogicalVolume* logi_B4mag3p4 = new G4LogicalVolume(geo_B4mag3p4, mat_B4mag3p4, "logi_B4mag3p4_name");

      //put volume
      setColor(*logi_B4mag3p4, cB4mag3p4.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag3p4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag3p4, "phys_B4mag3p4_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag3p5

      //get parameters from .xml file
      GearDir cB4mag3p5(content, "B4mag3p5/");

      const int B4mag3p5_num = atoi(cB4mag3p5.getString("N").c_str());

      double B4mag3p5_Z[B4mag3p5_num];
      double B4mag3p5_R[B4mag3p5_num];
      double B4mag3p5_r[B4mag3p5_num];

      for (int i = 0; i < B4mag3p5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag3p5_Z[i] = cB4mag3p5.getLength(ossZID.str()) * unitFactor;
        B4mag3p5_R[i] = cB4mag3p5.getLength(ossRID.str()) * unitFactor;
        B4mag3p5_r[i] = cB4mag3p5.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag3p5xx = new G4Polycone("geo_B4mag3p5xx_name", 0, 2 * M_PI, B4mag3p5_num, B4mag3p5_Z, B4mag3p5_r, B4mag3p5_R);
      G4IntersectionSolid* geo_B4mag3p5 = new G4IntersectionSolid("geo_B4mag3p5_name", geo_B4mag3p5xx, geo_B1spc1);

      string strMat_B4mag3p5 = cB4mag3p5.getString("Material");
      G4Material* mat_B4mag3p5 = Materials::get(strMat_B4mag3p5);
      G4LogicalVolume* logi_B4mag3p5 = new G4LogicalVolume(geo_B4mag3p5, mat_B4mag3p5, "logi_B4mag3p5_name");

      //put volume
      setColor(*logi_B4mag3p5, cB4mag3p5.getString("Color", "#CC0000"));
      setVisibility(*logi_B4mag3p5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag3p5, "phys_B4mag3p5_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag3p6

      //get parameters from .xml file
      GearDir cB4mag3p6(content, "B4mag3p6/");

      const int B4mag3p6_num = atoi(cB4mag3p6.getString("N").c_str());

      double B4mag3p6_Z[B4mag3p6_num];
      double B4mag3p6_R[B4mag3p6_num];
      double B4mag3p6_r[B4mag3p6_num];

      for (int i = 0; i < B4mag3p6_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag3p6_Z[i] = cB4mag3p6.getLength(ossZID.str()) * unitFactor;
        B4mag3p6_R[i] = cB4mag3p6.getLength(ossRID.str()) * unitFactor;
        B4mag3p6_r[i] = cB4mag3p6.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag3p6xx = new G4Polycone("geo_B4mag3p6xx_name", 0, 2 * M_PI, B4mag3p6_num, B4mag3p6_Z, B4mag3p6_r, B4mag3p6_R);
      G4IntersectionSolid* geo_B4mag3p6 = new G4IntersectionSolid("geo_B4mag3p6_name", geo_B4mag3p6xx, geo_B1spc1);

      string strMat_B4mag3p6 = cB4mag3p6.getString("Material");
      G4Material* mat_B4mag3p6 = Materials::get(strMat_B4mag3p6);
      G4LogicalVolume* logi_B4mag3p6 = new G4LogicalVolume(geo_B4mag3p6, mat_B4mag3p6, "logi_B4mag3p6_name");

      //put volume
      setColor(*logi_B4mag3p6, cB4mag3p6.getString("Color", "#CC0000"));
      setVisibility(*logi_B4mag3p6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag3p6, "phys_B4mag3p6_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag4p1

      //get parameters from .xml file
      GearDir cB4mag4p1(content, "B4mag4p1/");

      const int B4mag4p1_num = atoi(cB4mag4p1.getString("N").c_str());

      double B4mag4p1_Z[B4mag4p1_num];
      double B4mag4p1_R[B4mag4p1_num];
      double B4mag4p1_r[B4mag4p1_num];

      for (int i = 0; i < B4mag4p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag4p1_Z[i] = cB4mag4p1.getLength(ossZID.str()) * unitFactor;
        B4mag4p1_R[i] = cB4mag4p1.getLength(ossRID.str()) * unitFactor;
        B4mag4p1_r[i] = cB4mag4p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag4p1 = new G4Polycone("geo_B4mag4p1xx_name", 0, 2 * M_PI, B4mag4p1_num, B4mag4p1_Z, B4mag4p1_r, B4mag4p1_R);
      //G4IntersectionSolid* geo_B4mag4p1 = new G4IntersectionSolid("geo_B4mag4p1_name", geo_B4mag4p1xx, geo_B1spc1);

      string strMat_B4mag4p1 = cB4mag4p1.getString("Material");
      G4Material* mat_B4mag4p1 = Materials::get(strMat_B4mag4p1);
      G4LogicalVolume* logi_B4mag4p1 = new G4LogicalVolume(geo_B4mag4p1, mat_B4mag4p1, "logi_B4mag4p1_name");

      //put volume
      setColor(*logi_B4mag4p1, cB4mag4p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag4p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag4p1, "phys_B4mag4p1_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag4p7

      //get parameters from .xml file
      GearDir cB4mag4p7(content, "B4mag4p7/");

      const int B4mag4p7_num = atoi(cB4mag4p7.getString("N").c_str());

      double B4mag4p7_Z[B4mag4p7_num];
      double B4mag4p7_R[B4mag4p7_num];
      double B4mag4p7_r[B4mag4p7_num];

      for (int i = 0; i < B4mag4p7_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag4p7_Z[i] = cB4mag4p7.getLength(ossZID.str()) * unitFactor;
        B4mag4p7_R[i] = cB4mag4p7.getLength(ossRID.str()) * unitFactor;
        B4mag4p7_r[i] = cB4mag4p7.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag4p7 = new G4Polycone("geo_B4mag4p7xx_name", 0, 2 * M_PI, B4mag4p7_num, B4mag4p7_Z, B4mag4p7_r, B4mag4p7_R);
      //G4IntersectionSolid* geo_B4mag4p7 = new G4IntersectionSolid("geo_B4mag4p7_name", geo_B4mag4p7xx, geo_B1spc1);

      string strMat_B4mag4p7 = cB4mag4p7.getString("Material");
      G4Material* mat_B4mag4p7 = Materials::get(strMat_B4mag4p7);
      G4LogicalVolume* logi_B4mag4p7 = new G4LogicalVolume(geo_B4mag4p7, mat_B4mag4p7, "logi_B4mag4p7_name");

      //put volume
      setColor(*logi_B4mag4p7, cB4mag4p7.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag4p7, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag4p7, "phys_B4mag4p7_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag4p8

      //get parameters from .xml file
      GearDir cB4mag4p8(content, "B4mag4p8/");

      const int B4mag4p8_num = atoi(cB4mag4p8.getString("N").c_str());

      double B4mag4p8_Z[B4mag4p8_num];
      double B4mag4p8_R[B4mag4p8_num];
      double B4mag4p8_r[B4mag4p8_num];

      for (int i = 0; i < B4mag4p8_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag4p8_Z[i] = cB4mag4p8.getLength(ossZID.str()) * unitFactor;
        B4mag4p8_R[i] = cB4mag4p8.getLength(ossRID.str()) * unitFactor;
        B4mag4p8_r[i] = cB4mag4p8.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag4p8 = new G4Polycone("geo_B4mag4p8xx_name", 0, 2 * M_PI, B4mag4p8_num, B4mag4p8_Z, B4mag4p8_r, B4mag4p8_R);
      //G4IntersectionSolid* geo_B4mag4p8 = new G4IntersectionSolid("geo_B4mag4p8_name", geo_B4mag4p8xx, geo_B1spc1);

      string strMat_B4mag4p8 = cB4mag4p8.getString("Material");
      G4Material* mat_B4mag4p8 = Materials::get(strMat_B4mag4p8);
      G4LogicalVolume* logi_B4mag4p8 = new G4LogicalVolume(geo_B4mag4p8, mat_B4mag4p8, "logi_B4mag4p8_name");

      //put volume
      setColor(*logi_B4mag4p8, cB4mag4p8.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag4p8, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag4p8, "phys_B4mag4p8_name", logi_B1spc1, false, 0);

      //--------------
      //-   B4mag4p9

      //get parameters from .xml file
      GearDir cB4mag4p9(content, "B4mag4p9/");

      const int B4mag4p9_num = atoi(cB4mag4p9.getString("N").c_str());

      double B4mag4p9_Z[B4mag4p9_num];
      double B4mag4p9_R[B4mag4p9_num];
      double B4mag4p9_r[B4mag4p9_num];

      for (int i = 0; i < B4mag4p9_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B4mag4p9_Z[i] = cB4mag4p9.getLength(ossZID.str()) * unitFactor;
        B4mag4p9_R[i] = cB4mag4p9.getLength(ossRID.str()) * unitFactor;
        B4mag4p9_r[i] = cB4mag4p9.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_B4mag4p9 = new G4Polycone("geo_B4mag4p9xx_name", 0, 2 * M_PI, B4mag4p9_num, B4mag4p9_Z, B4mag4p9_r, B4mag4p9_R);
      //G4IntersectionSolid* geo_B4mag4p9 = new G4IntersectionSolid("geo_B4mag4p9_name", geo_B4mag4p9xx, geo_B1spc1);

      string strMat_B4mag4p9 = cB4mag4p9.getString("Material");
      G4Material* mat_B4mag4p9 = Materials::get(strMat_B4mag4p9);
      G4LogicalVolume* logi_B4mag4p9 = new G4LogicalVolume(geo_B4mag4p9, mat_B4mag4p9, "logi_B4mag4p9_name");

      //put volume
      setColor(*logi_B4mag4p9, cB4mag4p9.getString("Color", "#CC0000"));
      //setVisibility(*logi_B4mag4p9, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_B4mag4p9, "phys_B4mag4p9_name", logi_B1spc1, false, 0);

      //--------------
      //-   C1wal1

      //get parameters from .xml file
      GearDir cC1wal1(content, "C1wal1/");

      const int C1wal1_num = atoi(cC1wal1.getString("N").c_str());

      double C1wal1_Z[C1wal1_num];
      double C1wal1_R[C1wal1_num];
      double C1wal1_r[C1wal1_num];

      for (int i = 0; i < C1wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C1wal1_Z[i] = cC1wal1.getLength(ossZID.str()) * unitFactor;
        C1wal1_R[i] = cC1wal1.getLength(ossRID.str()) * unitFactor;
        C1wal1_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C1wal1xx = new G4Polycone("geo_C1wal1xx_name", 0, 2 * M_PI, C1wal1_num, C1wal1_Z, C1wal1_r, C1wal1_R);
      G4SubtractionSolid* geo_C1wal1x = new G4SubtractionSolid("geo_C1wal1x_name", geo_C1wal1xx, geo_A1spc1, transform_HER);
      G4SubtractionSolid* geo_C1wal1 = new G4SubtractionSolid("geo_C1wal1_name", geo_C1wal1x, geo_B1spc1, transform_LER);

      string strMat_C1wal1 = cC1wal1.getString("Material");
      G4Material* mat_C1wal1 = Materials::get(strMat_C1wal1);
      G4LogicalVolume* logi_C1wal1 = new G4LogicalVolume(geo_C1wal1, mat_C1wal1, "logi_C1wal1_name");

      //logi_C1wal1->SetSensitiveDetector(m_sensitive);
      //logi_C1wal1->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1));

      //put volume
      setColor(*logi_C1wal1, cC1wal1.getString("Color", "#CC0000"));
      setVisibility(*logi_C1wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C1wal1, "phys_C1wal1_name", &topVolume, false, 0);

      //--------------
      //-   C2spc1

      //get parameters from .xml file
      GearDir cC2spc1(content, "C2spc1/");

      const int C2spc1_num = atoi(cC2spc1.getString("N").c_str());

      double C2spc1_Z[C2spc1_num];
      double C2spc1_R[C2spc1_num];
      double C2spc1_r[C2spc1_num];

      for (int i = 0; i < C2spc1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C2spc1_Z[i] = cC2spc1.getLength(ossZID.str()) * unitFactor;
        C2spc1_R[i] = cC2spc1.getLength(ossRID.str()) * unitFactor;
        C2spc1_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C2spc1xx = new G4Polycone("geo_C2spc1xx_name", 0, 2 * M_PI, C2spc1_num, C2spc1_Z, C2spc1_r, C2spc1_R);
      G4IntersectionSolid* geo_C2spc1 = new G4IntersectionSolid("geo_C2spc1_name", geo_C2spc1xx, geo_C1wal1);

      string strMat_C2spc1 = cC2spc1.getString("Material");
      G4Material* mat_C2spc1 = Materials::get(strMat_C2spc1);
      G4LogicalVolume* logi_C2spc1 = new G4LogicalVolume(geo_C2spc1, mat_C2spc1, "logi_C2spc1_name");

      //put volume
      setColor(*logi_C2spc1, cC2spc1.getString("Color", "#CCCCCC"));
      setVisibility(*logi_C2spc1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C2spc1, "phys_C2spc1_name", logi_C1wal1, false, 0);

      //--------------
      //-   C2spc2

      //get parameters from .xml file
      GearDir cC2spc2(content, "C2spc2/");

      const int C2spc2_num = atoi(cC2spc2.getString("N").c_str());

      double C2spc2_Z[C2spc2_num];
      double C2spc2_R[C2spc2_num];
      double C2spc2_r[C2spc2_num];

      for (int i = 0; i < C2spc2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C2spc2_Z[i] = cC2spc2.getLength(ossZID.str()) * unitFactor;
        C2spc2_R[i] = cC2spc2.getLength(ossRID.str()) * unitFactor;
        C2spc2_r[i] = cC2spc2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_C2spc2xx = new G4Polycone("geo_C2spc2xx_name", 0, 2 * M_PI, C2spc2_num, C2spc2_Z, C2spc2_r, C2spc2_R);
      G4IntersectionSolid* geo_C2spc2 = new G4IntersectionSolid("geo_C2spc2_name", geo_C2spc2xx, geo_C1wal1);

      string strMat_C2spc2 = cC2spc2.getString("Material");
      G4Material* mat_C2spc2 = Materials::get(strMat_C2spc2);
      G4LogicalVolume* logi_C2spc2 = new G4LogicalVolume(geo_C2spc2, mat_C2spc2, "logi_C2spc2_name");

      //put volume
      setColor(*logi_C2spc2, cC2spc2.getString("Color", "#CCCCCC"));
      setVisibility(*logi_C2spc2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C2spc2, "phys_C2spc2_name", logi_C1wal1, false, 0);

      //--------------
      //-   C3wal2

      //get parameters from .xml file
      GearDir cC3wal2(content, "C3wal2/");

      const int C3wal2_num = atoi(cC3wal2.getString("N").c_str());

      double C3wal2_Z[C3wal2_num];
      double C3wal2_R[C3wal2_num];
      double C3wal2_r[C3wal2_num];

      for (int i = 0; i < C3wal2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C3wal2_Z[i] = cC3wal2.getLength(ossZID.str()) * unitFactor;
        C3wal2_R[i] = cC3wal2.getLength(ossRID.str()) * unitFactor;
        C3wal2_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C3wal2xx = new G4Polycone("geo_C3wal2xx_name", 0, 2 * M_PI, C3wal2_num, C3wal2_Z, C3wal2_r, C3wal2_R);
      G4IntersectionSolid* geo_C3wal2 = new G4IntersectionSolid("geo_C3wal2_name", geo_C3wal2xx, geo_C1wal1);

      string strMat_C3wal2 = cC3wal2.getString("Material");
      G4Material* mat_C3wal2 = Materials::get(strMat_C3wal2);
      G4LogicalVolume* logi_C3wal2 = new G4LogicalVolume(geo_C3wal2, mat_C3wal2, "logi_C3wal2_name");

      //put volume
      setColor(*logi_C3wal2, cC3wal2.getString("Color", "#CC0000"));
      setVisibility(*logi_C3wal2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C3wal2, "phys_C3wal2_name", logi_C2spc1, false, 0);

      //--------------
      //-   C3wal3

      //get parameters from .xml file
      GearDir cC3wal3(content, "C3wal3/");

      const int C3wal3_num = atoi(cC3wal3.getString("N").c_str());

      double C3wal3_Z[C3wal3_num];
      double C3wal3_R[C3wal3_num];
      double C3wal3_r[C3wal3_num];

      for (int i = 0; i < C3wal3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C3wal3_Z[i] = cC3wal3.getLength(ossZID.str()) * unitFactor;
        C3wal3_R[i] = cC3wal3.getLength(ossRID.str()) * unitFactor;
        C3wal3_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C3wal3xx = new G4Polycone("geo_C3wal3xx_name", 0, 2 * M_PI, C3wal3_num, C3wal3_Z, C3wal3_r, C3wal3_R);
      G4IntersectionSolid* geo_C3wal3 = new G4IntersectionSolid("geo_C3wal3_name", geo_C3wal3xx, geo_C1wal1);

      string strMat_C3wal3 = cC3wal3.getString("Material");
      G4Material* mat_C3wal3 = Materials::get(strMat_C3wal3);
      G4LogicalVolume* logi_C3wal3 = new G4LogicalVolume(geo_C3wal3, mat_C3wal3, "logi_C3wal3_name");

      //put volume
      setColor(*logi_C3wal3, cC3wal3.getString("Color", "#CC0000"));
      setVisibility(*logi_C3wal3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C3wal3, "phys_C3wal3_name", logi_C2spc2, false, 0);

      //--------------
      //-   C3wal4

      //get parameters from .xml file
      GearDir cC3wal4(content, "C3wal4/");

      const int C3wal4_num = atoi(cC3wal4.getString("N").c_str());

      double C3wal4_Z[C3wal4_num];
      double C3wal4_R[C3wal4_num];
      double C3wal4_r[C3wal4_num];

      for (int i = 0; i < C3wal4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C3wal4_Z[i] = cC3wal4.getLength(ossZID.str()) * unitFactor;
        C3wal4_R[i] = cC3wal4.getLength(ossRID.str()) * unitFactor;
        C3wal4_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C3wal4xx = new G4Polycone("geo_C3wal4xx_name", 0, 2 * M_PI, C3wal4_num, C3wal4_Z, C3wal4_r, C3wal4_R);
      G4IntersectionSolid* geo_C3wal4 = new G4IntersectionSolid("geo_C3wal4_name", geo_C3wal4xx, geo_C1wal1);

      string strMat_C3wal4 = cC3wal4.getString("Material");
      G4Material* mat_C3wal4 = Materials::get(strMat_C3wal4);
      G4LogicalVolume* logi_C3wal4 = new G4LogicalVolume(geo_C3wal4, mat_C3wal4, "logi_C3wal4_name");

      //put volume
      setColor(*logi_C3wal4, cC3wal4.getString("Color", "#CC0000"));
      setVisibility(*logi_C3wal4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C3wal4, "phys_C3wal4_name", logi_C2spc2, false, 0);

      //--------------
      //-   C4spc2

      //get parameters from .xml file
      GearDir cC4spc2(content, "C4spc2/");

      const int C4spc2_num = atoi(cC4spc2.getString("N").c_str());

      double C4spc2_Z[C4spc2_num];
      double C4spc2_R[C4spc2_num];
      double C4spc2_r[C4spc2_num];

      for (int i = 0; i < C4spc2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C4spc2_Z[i] = cC4spc2.getLength(ossZID.str()) * unitFactor;
        C4spc2_R[i] = cC4spc2.getLength(ossRID.str()) * unitFactor;
        C4spc2_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C4spc2xx = new G4Polycone("geo_C4spc2xx_name", 0, 2 * M_PI, C4spc2_num, C4spc2_Z, C4spc2_r, C4spc2_R);
      G4IntersectionSolid* geo_C4spc2 = new G4IntersectionSolid("geo_C4spc2_name", geo_C4spc2xx, geo_C1wal1);

      string strMat_C4spc2 = cC4spc2.getString("Material");
      G4Material* mat_C4spc2 = Materials::get(strMat_C4spc2);
      G4LogicalVolume* logi_C4spc2 = new G4LogicalVolume(geo_C4spc2, mat_C4spc2, "logi_C4spc2_name");

      //put volume
      setColor(*logi_C4spc2, cC4spc2.getString("Color", "#CCCCCC"));
      setVisibility(*logi_C4spc2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C4spc2, "phys_C4spc2_name", logi_C3wal2, false, 0);

      //--------------
      //-   C4spc4

      //get parameters from .xml file
      GearDir cC4spc4(content, "C4spc4/");

      const int C4spc4_num = atoi(cC4spc4.getString("N").c_str());

      double C4spc4_Z[C4spc4_num];
      double C4spc4_R[C4spc4_num];
      double C4spc4_r[C4spc4_num];

      for (int i = 0; i < C4spc4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C4spc4_Z[i] = cC4spc4.getLength(ossZID.str()) * unitFactor;
        C4spc4_R[i] = cC4spc4.getLength(ossRID.str()) * unitFactor;
        C4spc4_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C4spc4xx = new G4Polycone("geo_C4spc4xx_name", 0, 2 * M_PI, C4spc4_num, C4spc4_Z, C4spc4_r, C4spc4_R);
      G4IntersectionSolid* geo_C4spc4 = new G4IntersectionSolid("geo_C4spc4_name", geo_C4spc4xx, geo_C1wal1);

      string strMat_C4spc4 = cC4spc4.getString("Material");
      G4Material* mat_C4spc4 = Materials::get(strMat_C4spc4);
      G4LogicalVolume* logi_C4spc4 = new G4LogicalVolume(geo_C4spc4, mat_C4spc4, "logi_C4spc4_name");

      //put volume
      setColor(*logi_C4spc4, cC4spc4.getString("Color", "#CCCCCC"));
      setVisibility(*logi_C4spc4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C4spc4, "phys_C4spc4_name", logi_C3wal3, false, 0);

      //--------------
      //-   C5wal3

      //get parameters from .xml file
      GearDir cC5wal3(content, "C5wal3/");

      const int C5wal3_num = atoi(cC5wal3.getString("N").c_str());

      double C5wal3_Z[C5wal3_num];
      double C5wal3_R[C5wal3_num];
      double C5wal3_r[C5wal3_num];

      for (int i = 0; i < C5wal3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C5wal3_Z[i] = cC5wal3.getLength(ossZID.str()) * unitFactor;
        C5wal3_R[i] = cC5wal3.getLength(ossRID.str()) * unitFactor;
        C5wal3_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C5wal3x = new G4Polycone("geo_C5wal3x_name", 0, 2 * M_PI, C5wal3_num, C5wal3_Z, C5wal3_r, C5wal3_R);
      G4IntersectionSolid* geo_C5wal3 = new G4IntersectionSolid("geo_C5wal3_name", geo_C5wal3x, geo_C1wal1);

      string strMat_C5wal3 = cC5wal3.getString("Material");
      G4Material* mat_C5wal3 = Materials::get(strMat_C5wal3);
      G4LogicalVolume* logi_C5wal3 = new G4LogicalVolume(geo_C5wal3, mat_C5wal3, "logi_C5wal3_name");

      //put volume
      setColor(*logi_C5wal3, cC5wal3.getString("Color", "#CC0000"));
      setVisibility(*logi_C5wal3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C5wal3, "phys_C5wal3_name", logi_C4spc2, false, 0);

      //--------------
      //-   C5wal5

      //get parameters from .xml file
      GearDir cC5wal5(content, "C5wal5/");

      const int C5wal5_num = atoi(cC5wal5.getString("N").c_str());

      double C5wal5_Z[C5wal5_num];
      double C5wal5_R[C5wal5_num];
      double C5wal5_r[C5wal5_num];

      for (int i = 0; i < C5wal5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C5wal5_Z[i] = cC5wal5.getLength(ossZID.str()) * unitFactor;
        C5wal5_R[i] = cC5wal5.getLength(ossRID.str()) * unitFactor;
        C5wal5_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C5wal5x = new G4Polycone("geo_C5wal5x_name", 0, 2 * M_PI, C5wal5_num, C5wal5_Z, C5wal5_r, C5wal5_R);
      G4IntersectionSolid* geo_C5wal5 = new G4IntersectionSolid("geo_C5wal5_name", geo_C5wal5x, geo_C1wal1);

      string strMat_C5wal5 = cC5wal5.getString("Material");
      G4Material* mat_C5wal5 = Materials::get(strMat_C5wal5);
      G4LogicalVolume* logi_C5wal5 = new G4LogicalVolume(geo_C5wal5, mat_C5wal5, "logi_C5wal5_name");

      //put volume
      setColor(*logi_C5wal5, cC5wal5.getString("Color", "#CC0000"));
      setVisibility(*logi_C5wal5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C5wal5, "phys_C5wal5_name", logi_C4spc4, false, 0);

      //--------------
      //-   C6spc3

      //get parameters from .xml file
      GearDir cC6spc3(content, "C6spc3/");

      const int C6spc3_num = atoi(cC6spc3.getString("N").c_str());

      double C6spc3_Z[C6spc3_num];
      double C6spc3_R[C6spc3_num];
      double C6spc3_r[C6spc3_num];

      for (int i = 0; i < C6spc3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C6spc3_Z[i] = cC6spc3.getLength(ossZID.str()) * unitFactor;
        C6spc3_R[i] = cC6spc3.getLength(ossRID.str()) * unitFactor;
        C6spc3_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C6spc3x = new G4Polycone("geo_C6spc3x_name", 0, 2 * M_PI, C6spc3_num, C6spc3_Z, C6spc3_r, C6spc3_R);
      G4IntersectionSolid* geo_C6spc3 = new G4IntersectionSolid("geo_C6spc3_name", geo_C6spc3x, geo_C1wal1);

      string strMat_C6spc3 = cC6spc3.getString("Material");
      G4Material* mat_C6spc3 = Materials::get(strMat_C6spc3);
      G4LogicalVolume* logi_C6spc3 = new G4LogicalVolume(geo_C6spc3, mat_C6spc3, "logi_C6spc3_name");

      //put volume
      setColor(*logi_C6spc3, cC6spc3.getString("Color", "#CC0000"));
      setVisibility(*logi_C6spc3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc3, "phys_C6spc3_name", logi_C5wal3, false, 0);

      //--------------
      //-   C6spc6

      //get parameters from .xml file
      GearDir cC6spc6(content, "C6spc6/");

      const int C6spc6_num = atoi(cC6spc6.getString("N").c_str());

      double C6spc6_Z[C6spc6_num];
      double C6spc6_R[C6spc6_num];
      double C6spc6_r[C6spc6_num];

      for (int i = 0; i < C6spc6_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C6spc6_Z[i] = cC6spc6.getLength(ossZID.str()) * unitFactor;
        C6spc6_R[i] = cC6spc6.getLength(ossRID.str()) * unitFactor;
        C6spc6_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C6spc6x = new G4Polycone("geo_C6spc6x_name", 0, 2 * M_PI, C6spc6_num, C6spc6_Z, C6spc6_r, C6spc6_R);
      G4IntersectionSolid* geo_C6spc6 = new G4IntersectionSolid("geo_C6spc6_name", geo_C6spc6x, geo_C1wal1);

      string strMat_C6spc6 = cC6spc6.getString("Material");
      G4Material* mat_C6spc6 = Materials::get(strMat_C6spc6);
      G4LogicalVolume* logi_C6spc6 = new G4LogicalVolume(geo_C6spc6, mat_C6spc6, "logi_C6spc6_name");

      //put volume
      setColor(*logi_C6spc6, cC6spc6.getString("Color", "#CC0000"));
      setVisibility(*logi_C6spc6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C6spc6, "phys_C6spc6_name", logi_C5wal5, false, 0);

      //--------------
      //-   C7wal7

      //get parameters from .xml file
      GearDir cC7wal7(content, "C7wal7/");

      const int C7wal7_num = atoi(cC7wal7.getString("N").c_str());

      double C7wal7_Z[C7wal7_num];
      double C7wal7_R[C7wal7_num];
      double C7wal7_r[C7wal7_num];

      for (int i = 0; i < C7wal7_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        C7wal7_Z[i] = cC7wal7.getLength(ossZID.str()) * unitFactor;
        C7wal7_R[i] = cC7wal7.getLength(ossRID.str()) * unitFactor;
        C7wal7_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_C7wal7x = new G4Polycone("geo_C7wal7x_name", 0, 2 * M_PI, C7wal7_num, C7wal7_Z, C7wal7_r, C7wal7_R);
      G4IntersectionSolid* geo_C7wal7 = new G4IntersectionSolid("geo_C7wal7_name", geo_C7wal7x, geo_C1wal1);

      string strMat_C7wal7 = cC7wal7.getString("Material");
      G4Material* mat_C7wal7 = Materials::get(strMat_C7wal7);
      G4LogicalVolume* logi_C7wal7 = new G4LogicalVolume(geo_C7wal7, mat_C7wal7, "logi_C7wal7_name");

      //put volume
      setColor(*logi_C7wal7, cC7wal7.getString("Color", "#CC0000"));
      setVisibility(*logi_C7wal7, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7wal7, "phys_C7wal7_name", logi_C6spc6, false, 0);

      //--------------
      //-   C7lyr1

      //get parameters from .xml file
      GearDir cC7lyr1(content, "C7lyr1/");

      const int C7lyr1_num = atoi(cC7lyr1.getString("N").c_str());

      double C7lyr1_Z[C7lyr1_num];
      double C7lyr1_R[C7lyr1_num];
      double C7lyr1_r[C7lyr1_num];

      for (int i = 0; i < C7lyr1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C7lyr1_Z[i] = cC7lyr1.getLength(ossZID.str()) * unitFactor;
        C7lyr1_R[i] = cC7lyr1.getLength(ossRID.str()) * unitFactor;
        C7lyr1_r[i] = cC7lyr1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_C7lyr1x = new G4Polycone("geo_C7lyr1x_name", 0, 2 * M_PI, C7lyr1_num, C7lyr1_Z, C7lyr1_r, C7lyr1_R);
      G4IntersectionSolid* geo_C7lyr1 = new G4IntersectionSolid("geo_C7lyr1_name", geo_C7lyr1x, geo_C1wal1);

      string strMat_C7lyr1 = cC7lyr1.getString("Material");
      G4Material* mat_C7lyr1 = Materials::get(strMat_C7lyr1);
      G4LogicalVolume* logi_C7lyr1 = new G4LogicalVolume(geo_C7lyr1, mat_C7lyr1, "logi_C7lyr1_name");

      //put volume
      setColor(*logi_C7lyr1, cC7lyr1.getString("Color", "#CC0000"));
      setVisibility(*logi_C7lyr1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7lyr1, "phys_C7lyr1_name", logi_C6spc3, false, 0);

      //--------------
      //-   C7lyr2

      //get parameters from .xml file
      GearDir cC7lyr2(content, "C7lyr2/");

      const int C7lyr2_num = atoi(cC7lyr2.getString("N").c_str());

      double C7lyr2_Z[C7lyr2_num];
      double C7lyr2_R[C7lyr2_num];
      double C7lyr2_r[C7lyr2_num];

      for (int i = 0; i < C7lyr2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C7lyr2_Z[i] = cC7lyr2.getLength(ossZID.str()) * unitFactor;
        C7lyr2_R[i] = cC7lyr2.getLength(ossRID.str()) * unitFactor;
        C7lyr2_r[i] = cC7lyr2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_C7lyr2x = new G4Polycone("geo_C7lyr2x_name", 0, 2 * M_PI, C7lyr2_num, C7lyr2_Z, C7lyr2_r, C7lyr2_R);
      G4IntersectionSolid* geo_C7lyr2 = new G4IntersectionSolid("geo_C7lyr2_name", geo_C7lyr2x, geo_C1wal1);

      string strMat_C7lyr2 = cC7lyr2.getString("Material");
      G4Material* mat_C7lyr2 = Materials::get(strMat_C7lyr2);
      G4LogicalVolume* logi_C7lyr2 = new G4LogicalVolume(geo_C7lyr2, mat_C7lyr2, "logi_C7lyr2_name");

      //put volume
      setColor(*logi_C7lyr2, cC7lyr2.getString("Color", "#CC0000"));
      setVisibility(*logi_C7lyr2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7lyr2, "phys_C7lyr2_name", logi_C6spc3, false, 0);

      //--------------
      //-   C7lyr3

      //get parameters from .xml file
      GearDir cC7lyr3(content, "C7lyr3/");

      const int C7lyr3_num = atoi(cC7lyr3.getString("N").c_str());

      double C7lyr3_Z[C7lyr3_num];
      double C7lyr3_R[C7lyr3_num];
      double C7lyr3_r[C7lyr3_num];

      for (int i = 0; i < C7lyr3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C7lyr3_Z[i] = cC7lyr3.getLength(ossZID.str()) * unitFactor;
        C7lyr3_R[i] = cC7lyr3.getLength(ossRID.str()) * unitFactor;
        C7lyr3_r[i] = cC7lyr3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_C7lyr3x = new G4Polycone("geo_C7lyr3x_name", 0, 2 * M_PI, C7lyr3_num, C7lyr3_Z, C7lyr3_r, C7lyr3_R);
      G4IntersectionSolid* geo_C7lyr3 = new G4IntersectionSolid("geo_C7lyr3_name", geo_C7lyr3x, geo_C1wal1);

      string strMat_C7lyr3 = cC7lyr3.getString("Material");
      G4Material* mat_C7lyr3 = Materials::get(strMat_C7lyr3);
      G4LogicalVolume* logi_C7lyr3 = new G4LogicalVolume(geo_C7lyr3, mat_C7lyr3, "logi_C7lyr3_name");

      //put volume
      setColor(*logi_C7lyr3, cC7lyr3.getString("Color", "#CC0000"));
      setVisibility(*logi_C7lyr3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7lyr3, "phys_C7lyr3_name", logi_C6spc3, false, 0);

      //--------------
      //-   C7lyr4

      //get parameters from .xml file
      GearDir cC7lyr4(content, "C7lyr4/");

      const int C7lyr4_num = atoi(cC7lyr4.getString("N").c_str());

      double C7lyr4_Z[C7lyr4_num];
      double C7lyr4_R[C7lyr4_num];
      double C7lyr4_r[C7lyr4_num];

      for (int i = 0; i < C7lyr4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C7lyr4_Z[i] = cC7lyr4.getLength(ossZID.str()) * unitFactor;
        C7lyr4_R[i] = cC7lyr4.getLength(ossRID.str()) * unitFactor;
        C7lyr4_r[i] = cC7lyr4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_C7lyr4x = new G4Polycone("geo_C7lyr4x_name", 0, 2 * M_PI, C7lyr4_num, C7lyr4_Z, C7lyr4_r, C7lyr4_R);
      G4IntersectionSolid* geo_C7lyr4 = new G4IntersectionSolid("geo_C7lyr4_name", geo_C7lyr4x, geo_C1wal1);

      string strMat_C7lyr4 = cC7lyr4.getString("Material");
      G4Material* mat_C7lyr4 = Materials::get(strMat_C7lyr4);
      G4LogicalVolume* logi_C7lyr4 = new G4LogicalVolume(geo_C7lyr4, mat_C7lyr4, "logi_C7lyr4_name");

      //put volume
      setColor(*logi_C7lyr4, cC7lyr4.getString("Color", "#CC0000"));
      setVisibility(*logi_C7lyr4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7lyr4, "phys_C7lyr4_name", logi_C6spc3, false, 0);

      //--------------
      //-   C7lyr5

      //get parameters from .xml file
      GearDir cC7lyr5(content, "C7lyr5/");

      const int C7lyr5_num = atoi(cC7lyr5.getString("N").c_str());

      double C7lyr5_Z[C7lyr5_num];
      double C7lyr5_R[C7lyr5_num];
      double C7lyr5_r[C7lyr5_num];

      for (int i = 0; i < C7lyr5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C7lyr5_Z[i] = cC7lyr5.getLength(ossZID.str()) * unitFactor;
        C7lyr5_R[i] = cC7lyr5.getLength(ossRID.str()) * unitFactor;
        C7lyr5_r[i] = cC7lyr5.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_C7lyr5x = new G4Polycone("geo_C7lyr5x_name", 0, 2 * M_PI, C7lyr5_num, C7lyr5_Z, C7lyr5_r, C7lyr5_R);
      G4IntersectionSolid* geo_C7lyr5 = new G4IntersectionSolid("geo_C7lyr5_name", geo_C7lyr5x, geo_C1wal1);

      string strMat_C7lyr5 = cC7lyr5.getString("Material");
      G4Material* mat_C7lyr5 = Materials::get(strMat_C7lyr5);
      G4LogicalVolume* logi_C7lyr5 = new G4LogicalVolume(geo_C7lyr5, mat_C7lyr5, "logi_C7lyr5_name");

      //put volume
      setColor(*logi_C7lyr5, cC7lyr5.getString("Color", "#CC0000"));
      setVisibility(*logi_C7lyr5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_C7lyr5, "phys_C7lyr5_name", logi_C6spc3, false, 0);

      //--------------
      //-   D1spc1 and E1spc1

      // right bounding form 1
      GearDir cTubeL(content, "TubeL/");

      const int TubeL_num = atoi(cTubeL.getString("N").c_str());

      double TubeL_Z[TubeL_num];
      double TubeL_R[TubeL_num];
      double TubeL_r[TubeL_num];

      for (int i = 0; i < TubeL_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        TubeL_Z[i] = cTubeL.getLength(ossZID.str()) * unitFactor;
        TubeL_R[i] = cTubeL.getLength(ossRID.str()) * unitFactor;
        TubeL_r[i] = 0.0;
      }

      G4Polycone* geo_TubeL = new G4Polycone("geo_TubeL_name", 0, 2 * M_PI, TubeL_num, TubeL_Z, TubeL_r, TubeL_R);

      // space containing all structures around left HER beam pipe
      GearDir cD1spc1(content, "D1spc1/");

      const int D1spc1_num = atoi(cD1spc1.getString("N").c_str());

      double D1spc1_Z[D1spc1_num];
      double D1spc1_r[D1spc1_num];
      double D1spc1_R[D1spc1_num];

      for (int i = 0; i < D1spc1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        D1spc1_Z[i] = cD1spc1.getLength(ossZID.str()) * unitFactor;
        D1spc1_R[i] = cD1spc1.getLength(ossRID.str()) * unitFactor;
        D1spc1_r[i] = 0.0;
      }

      G4Polycone* geo_D1spc1xx = new G4Polycone("geo_D1spc1xx_name", 0, 2 * M_PI, D1spc1_num, D1spc1_Z, D1spc1_r, D1spc1_R);

      // space containing all structures around left LER beam pipe
      GearDir cE1spc1(content, "E1spc1/");

      const int E1spc1_num = atoi(cE1spc1.getString("N").c_str());

      double E1spc1_Z[E1spc1_num];
      double E1spc1_R[E1spc1_num];
      double E1spc1_r[E1spc1_num];

      for (int i = 0; i < E1spc1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        E1spc1_Z[i] = cE1spc1.getLength(ossZID.str()) * unitFactor;
        E1spc1_R[i] = cE1spc1.getLength(ossRID.str()) * unitFactor;
        E1spc1_r[i] = 0.0;
      }

      G4Polycone* geo_E1spc1xx = new G4Polycone("geo_E1spc1xx_name", 0, 2 * M_PI, E1spc1_num, E1spc1_Z, E1spc1_r, E1spc1_R);

      // final cut
      G4IntersectionSolid* geo_D1spc1x = new G4IntersectionSolid("geo_D1spc1x_name", geo_D1spc1xx, geo_TubeL, transform_HER_inv);
      G4IntersectionSolid* geo_E1spc1 = new G4IntersectionSolid("geo_E1spc1_name", geo_E1spc1xx, geo_TubeL, transform_LER_inv);
      G4SubtractionSolid* geo_D1spc1 = new G4SubtractionSolid("geo_D1spc1_name", geo_D1spc1x, geo_E1spc1, transform_HER2LER);

      string strMat_D1spc1 = cD1spc1.getString("Material");
      G4Material* mat_D1spc1 = Materials::get(strMat_D1spc1);
      G4LogicalVolume* logi_D1spc1 = new G4LogicalVolume(geo_D1spc1, mat_D1spc1, "logi_D1spc1_name");
      if (flag_limitStep) logi_D1spc1->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*logi_D1spc1, cD1spc1.getString("Color", "#CC0000"));
      //setVisibility(*logi_D1spc1, false);
      new G4PVPlacement(transform_HER, logi_D1spc1, "phys_D1spc1_name", &topVolume, false, 0);

      string strMat_E1spc1 = cE1spc1.getString("Material");
      G4Material* mat_E1spc1 = Materials::get(strMat_E1spc1);
      G4LogicalVolume* logi_E1spc1 = new G4LogicalVolume(geo_E1spc1, mat_E1spc1, "logi_E1spc1_name");
      if (flag_limitStep) logi_E1spc1->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*logi_E1spc1, cE1spc1.getString("Color", "#CC0000"));
      //setVisibility(*logi_E1spc1, false);
      new G4PVPlacement(transform_LER, logi_E1spc1, "phys_E1spc1_name", &topVolume, false, 0);

      //--------------
      //-   D2wal1

      //get parameters from .xml file
      GearDir cD2wal1(content, "D2wal1/");

      const int D2wal1_num = atoi(cD2wal1.getString("N").c_str());

      double D2wal1_Z[D2wal1_num];
      double D2wal1_R[D2wal1_num];
      double D2wal1_r[D2wal1_num];

      for (int i = 0; i < D2wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D2wal1_Z[i] = cD2wal1.getLength(ossZID.str()) * unitFactor;
        D2wal1_R[i] = cD2wal1.getLength(ossRID.str()) * unitFactor;
        D2wal1_r[i] = cD2wal1.getLength(ossrID.str()) * unitFactor;
      }

      G4Polycone* geo_D2wal1xx = new G4Polycone("geo_D2wal1xx_name", 0, 2 * M_PI, D2wal1_num, D2wal1_Z, D2wal1_r, D2wal1_R);
      //G4IntersectionSolid* geo_D2wal1 = new G4IntersectionSolid("geo_D2wal1_name", geo_D2wal1xx, geo_D1spc1);
      G4IntersectionSolid* geo_D2wal1 = new G4IntersectionSolid("geo_D2wal1_name", geo_D2wal1xx, geo_TubeL, transform_HER_inv);

      string strMat_D2wal1 = cD2wal1.getString("Material");
      G4Material* mat_D2wal1 = Materials::get(strMat_D2wal1);
      G4LogicalVolume* logi_D2wal1 = new G4LogicalVolume(geo_D2wal1, mat_D2wal1, "logi_D2wal1_name");

      //put volume
      setColor(*logi_D2wal1, cD2wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_D2wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D2wal1, "phys_D2wal1_name", logi_D1spc1, false, 0);

      //--------------
      //-   D3wal1

      //get parameters from .xml file
      GearDir cD3wal1(content, "D3wal1/");

      const int D3wal1_num = atoi(cD3wal1.getString("N").c_str());

      double D3wal1_Z[D3wal1_num];
      double D3wal1_R[D3wal1_num];
      double D3wal1_r[D3wal1_num];

      for (int i = 0; i < D3wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D3wal1_Z[i] = cD3wal1.getLength(ossZID.str()) * unitFactor;
        D3wal1_R[i] = cD3wal1.getLength(ossRID.str()) * unitFactor;
        D3wal1_r[i] = cD3wal1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D3wal1xx = new G4Polycone("geo_D3wal1xx_name", 0, 2 * M_PI, D3wal1_num, D3wal1_Z, D3wal1_r, D3wal1_R);
      // to avoid strange overlap error
      G4SubtractionSolid* geo_D3wal1 = new G4SubtractionSolid("geo_D3wal1_name", geo_D3wal1xx, geo_D2wal1);

      string strMat_D3wal1 = cD3wal1.getString("Material");
      G4Material* mat_D3wal1 = Materials::get(strMat_D3wal1);
      G4LogicalVolume* logi_D3wal1 = new G4LogicalVolume(geo_D3wal1, mat_D3wal1, "logi_D3wal1_name");

      //put volume
      setColor(*logi_D3wal1, cD3wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_D3wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D3wal1, "phys_D3wal1_name", logi_D1spc1, false, 0);

      //--------------
      //-   D3wal2

      //get parameters from .xml file
      GearDir cD3wal2(content, "D3wal2/");

      const int D3wal2_num = atoi(cD3wal2.getString("N").c_str());

      double D3wal2_Z[D3wal2_num];
      double D3wal2_R[D3wal2_num];
      double D3wal2_r[D3wal2_num];

      for (int i = 0; i < D3wal2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D3wal2_Z[i] = cD3wal2.getLength(ossZID.str()) * unitFactor;
        D3wal2_R[i] = cD3wal2.getLength(ossRID.str()) * unitFactor;
        D3wal2_r[i] = cD3wal2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D3wal2 = new G4Polycone("geo_D3wal2xx_name", 0, 2 * M_PI, D3wal2_num, D3wal2_Z, D3wal2_r, D3wal2_R);
      //G4IntersectionSolid* geo_D3wal2 = new G4IntersectionSolid("geo_D3wal2_name", geo_D3wal2xx, geo_D1spc1);

      string strMat_D3wal2 = cD3wal2.getString("Material");
      G4Material* mat_D3wal2 = Materials::get(strMat_D3wal2);
      G4LogicalVolume* logi_D3wal2 = new G4LogicalVolume(geo_D3wal2, mat_D3wal2, "logi_D3wal2_name");

      //put volume
      setColor(*logi_D3wal2, cD3wal2.getString("Color", "#CC0000"));
      //setVisibility(*logi_D3wal2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D3wal2, "phys_D3wal2_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag1

      //get parameters from .xml file
      GearDir cD4mag1(content, "D4mag1/");

      const int D4mag1_num = atoi(cD4mag1.getString("N").c_str());

      double D4mag1_Z[D4mag1_num];
      double D4mag1_R[D4mag1_num];
      double D4mag1_r[D4mag1_num];

      for (int i = 0; i < D4mag1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag1_Z[i] = cD4mag1.getLength(ossZID.str()) * unitFactor;
        D4mag1_R[i] = cD4mag1.getLength(ossRID.str()) * unitFactor;
        D4mag1_r[i] = cD4mag1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag1xx = new G4Polycone("geo_D4mag1xx_name", 0, 2 * M_PI, D4mag1_num, D4mag1_Z, D4mag1_r, D4mag1_R);
      G4IntersectionSolid* geo_D4mag1 = new G4IntersectionSolid("geo_D4mag1_name", geo_D4mag1xx, geo_D1spc1);

      string strMat_D4mag1 = cD4mag1.getString("Material");
      G4Material* mat_D4mag1 = Materials::get(strMat_D4mag1);
      G4LogicalVolume* logi_D4mag1 = new G4LogicalVolume(geo_D4mag1, mat_D4mag1, "logi_D4mag1_name");

      //put volume
      setColor(*logi_D4mag1, cD4mag1.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag1, "phys_D4mag1_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag2p1

      //get parameters from .xml file
      GearDir cD4mag2p1(content, "D4mag2p1/");

      const int D4mag2p1_num = atoi(cD4mag2p1.getString("N").c_str());

      double D4mag2p1_Z[D4mag2p1_num];
      double D4mag2p1_R[D4mag2p1_num];
      double D4mag2p1_r[D4mag2p1_num];

      for (int i = 0; i < D4mag2p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag2p1_Z[i] = cD4mag2p1.getLength(ossZID.str()) * unitFactor;
        D4mag2p1_R[i] = cD4mag2p1.getLength(ossRID.str()) * unitFactor;
        D4mag2p1_r[i] = cD4mag2p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag2p1xx = new G4Polycone("geo_D4mag2p1xx_name", 0, 2 * M_PI, D4mag2p1_num, D4mag2p1_Z, D4mag2p1_r, D4mag2p1_R);
      G4IntersectionSolid* geo_D4mag2p1 = new G4IntersectionSolid("geo_D4mag2p1_name", geo_D4mag2p1xx, geo_D1spc1);

      string strMat_D4mag2p1 = cD4mag2p1.getString("Material");
      G4Material* mat_D4mag2p1 = Materials::get(strMat_D4mag2p1);
      G4LogicalVolume* logi_D4mag2p1 = new G4LogicalVolume(geo_D4mag2p1, mat_D4mag2p1, "logi_D4mag2p1_name");

      //put volume
      setColor(*logi_D4mag2p1, cD4mag2p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag2p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag2p1, "phys_D4mag2p1_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag2p2

      //get parameters from .xml file
      GearDir cD4mag2p2(content, "D4mag2p2/");

      const int D4mag2p2_num = atoi(cD4mag2p2.getString("N").c_str());

      double D4mag2p2_Z[D4mag2p2_num];
      double D4mag2p2_R[D4mag2p2_num];
      double D4mag2p2_r[D4mag2p2_num];

      for (int i = 0; i < D4mag2p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag2p2_Z[i] = cD4mag2p2.getLength(ossZID.str()) * unitFactor;
        D4mag2p2_R[i] = cD4mag2p2.getLength(ossRID.str()) * unitFactor;
        D4mag2p2_r[i] = cD4mag2p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag2p2xx = new G4Polycone("geo_D4mag2p2xx_name", 0, 2 * M_PI, D4mag2p2_num, D4mag2p2_Z, D4mag2p2_r, D4mag2p2_R);
      G4IntersectionSolid* geo_D4mag2p2 = new G4IntersectionSolid("geo_D4mag2p2_name", geo_D4mag2p2xx, geo_D1spc1);

      string strMat_D4mag2p2 = cD4mag2p2.getString("Material");
      G4Material* mat_D4mag2p2 = Materials::get(strMat_D4mag2p2);
      G4LogicalVolume* logi_D4mag2p2 = new G4LogicalVolume(geo_D4mag2p2, mat_D4mag2p2, "logi_D4mag2p2_name");

      //put volume
      setColor(*logi_D4mag2p2, cD4mag2p2.getString("Color", "#CC0000"));
      setVisibility(*logi_D4mag2p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag2p2, "phys_D4mag2p2_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag2p3

      //get parameters from .xml file
      GearDir cD4mag2p3(content, "D4mag2p3/");

      const int D4mag2p3_num = atoi(cD4mag2p3.getString("N").c_str());

      double D4mag2p3_Z[D4mag2p3_num];
      double D4mag2p3_R[D4mag2p3_num];
      double D4mag2p3_r[D4mag2p3_num];

      for (int i = 0; i < D4mag2p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag2p3_Z[i] = cD4mag2p3.getLength(ossZID.str()) * unitFactor;
        D4mag2p3_R[i] = cD4mag2p3.getLength(ossRID.str()) * unitFactor;
        D4mag2p3_r[i] = cD4mag2p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag2p3xx = new G4Polycone("geo_D4mag2p3xx_name", 0, 2 * M_PI, D4mag2p3_num, D4mag2p3_Z, D4mag2p3_r, D4mag2p3_R);
      G4IntersectionSolid* geo_D4mag2p3 = new G4IntersectionSolid("geo_D4mag2p3_name", geo_D4mag2p3xx, geo_D1spc1);

      string strMat_D4mag2p3 = cD4mag2p3.getString("Material");
      G4Material* mat_D4mag2p3 = Materials::get(strMat_D4mag2p3);
      G4LogicalVolume* logi_D4mag2p3 = new G4LogicalVolume(geo_D4mag2p3, mat_D4mag2p3, "logi_D4mag2p3_name");

      //put volume
      setColor(*logi_D4mag2p3, cD4mag2p3.getString("Color", "#CC0000"));
      setVisibility(*logi_D4mag2p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag2p3, "phys_D4mag2p3_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag2p4

      //get parameters from .xml file
      GearDir cD4mag2p4(content, "D4mag2p4/");

      const int D4mag2p4_num = atoi(cD4mag2p4.getString("N").c_str());

      double D4mag2p4_Z[D4mag2p4_num];
      double D4mag2p4_R[D4mag2p4_num];
      double D4mag2p4_r[D4mag2p4_num];

      for (int i = 0; i < D4mag2p4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag2p4_Z[i] = cD4mag2p4.getLength(ossZID.str()) * unitFactor;
        D4mag2p4_R[i] = cD4mag2p4.getLength(ossRID.str()) * unitFactor;
        D4mag2p4_r[i] = cD4mag2p4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag2p4xx = new G4Polycone("geo_D4mag2p4xx_name", 0, 2 * M_PI, D4mag2p4_num, D4mag2p4_Z, D4mag2p4_r, D4mag2p4_R);
      G4IntersectionSolid* geo_D4mag2p4 = new G4IntersectionSolid("geo_D4mag2p4_name", geo_D4mag2p4xx, geo_D1spc1);

      string strMat_D4mag2p4 = cD4mag2p4.getString("Material");
      G4Material* mat_D4mag2p4 = Materials::get(strMat_D4mag2p4);
      G4LogicalVolume* logi_D4mag2p4 = new G4LogicalVolume(geo_D4mag2p4, mat_D4mag2p4, "logi_D4mag2p4_name");

      //put volume
      setColor(*logi_D4mag2p4, cD4mag2p4.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag2p4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag2p4, "phys_D4mag2p4_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag3p1

      //get parameters from .xml file
      GearDir cD4mag3p1(content, "D4mag3p1/");

      const int D4mag3p1_num = atoi(cD4mag3p1.getString("N").c_str());

      double D4mag3p1_Z[D4mag3p1_num];
      double D4mag3p1_R[D4mag3p1_num];
      double D4mag3p1_r[D4mag3p1_num];

      for (int i = 0; i < D4mag3p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag3p1_Z[i] = cD4mag3p1.getLength(ossZID.str()) * unitFactor;
        D4mag3p1_R[i] = cD4mag3p1.getLength(ossRID.str()) * unitFactor;
        D4mag3p1_r[i] = cD4mag3p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag3p1 = new G4Polycone("geo_D4mag3p1xx_name", 0, 2 * M_PI, D4mag3p1_num, D4mag3p1_Z, D4mag3p1_r, D4mag3p1_R);
      //G4IntersectionSolid* geo_D4mag3p1 = new G4IntersectionSolid("geo_D4mag3p1_name", geo_D4mag3p1xx, geo_D1spc1);

      string strMat_D4mag3p1 = cD4mag3p1.getString("Material");
      G4Material* mat_D4mag3p1 = Materials::get(strMat_D4mag3p1);
      G4LogicalVolume* logi_D4mag3p1 = new G4LogicalVolume(geo_D4mag3p1, mat_D4mag3p1, "logi_D4mag3p1_name");

      //put volume
      setColor(*logi_D4mag3p1, cD4mag3p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag3p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag3p1, "phys_D4mag3p1_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag3p2

      //get parameters from .xml file
      GearDir cD4mag3p2(content, "D4mag3p2/");

      const int D4mag3p2_num = atoi(cD4mag3p2.getString("N").c_str());

      double D4mag3p2_Z[D4mag3p2_num];
      double D4mag3p2_R[D4mag3p2_num];
      double D4mag3p2_r[D4mag3p2_num];

      for (int i = 0; i < D4mag3p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag3p2_Z[i] = cD4mag3p2.getLength(ossZID.str()) * unitFactor;
        D4mag3p2_R[i] = cD4mag3p2.getLength(ossRID.str()) * unitFactor;
        D4mag3p2_r[i] = cD4mag3p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag3p2 = new G4Polycone("geo_D4mag3p2xx_name", 0, 2 * M_PI, D4mag3p2_num, D4mag3p2_Z, D4mag3p2_r, D4mag3p2_R);
      //G4IntersectionSolid* geo_D4mag3p2 = new G4IntersectionSolid("geo_D4mag3p2_name", geo_D4mag3p2xx, geo_D1spc1);

      string strMat_D4mag3p2 = cD4mag3p2.getString("Material");
      G4Material* mat_D4mag3p2 = Materials::get(strMat_D4mag3p2);
      G4LogicalVolume* logi_D4mag3p2 = new G4LogicalVolume(geo_D4mag3p2, mat_D4mag3p2, "logi_D4mag3p2_name");

      //put volume
      setColor(*logi_D4mag3p2, cD4mag3p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag3p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag3p2, "phys_D4mag3p2_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag3p3

      //get parameters from .xml file
      GearDir cD4mag3p3(content, "D4mag3p3/");

      const int D4mag3p3_num = atoi(cD4mag3p3.getString("N").c_str());

      double D4mag3p3_Z[D4mag3p3_num];
      double D4mag3p3_R[D4mag3p3_num];
      double D4mag3p3_r[D4mag3p3_num];

      for (int i = 0; i < D4mag3p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag3p3_Z[i] = cD4mag3p3.getLength(ossZID.str()) * unitFactor;
        D4mag3p3_R[i] = cD4mag3p3.getLength(ossRID.str()) * unitFactor;
        D4mag3p3_r[i] = cD4mag3p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag3p3 = new G4Polycone("geo_D4mag3p3xx_name", 0, 2 * M_PI, D4mag3p3_num, D4mag3p3_Z, D4mag3p3_r, D4mag3p3_R);
      //G4IntersectionSolid* geo_D4mag3p3 = new G4IntersectionSolid("geo_D4mag3p3_name", geo_D4mag3p3xx, geo_D1spc1);

      string strMat_D4mag3p3 = cD4mag3p3.getString("Material");
      G4Material* mat_D4mag3p3 = Materials::get(strMat_D4mag3p3);
      G4LogicalVolume* logi_D4mag3p3 = new G4LogicalVolume(geo_D4mag3p3, mat_D4mag3p3, "logi_D4mag3p3_name");

      //put volume
      setColor(*logi_D4mag3p3, cD4mag3p3.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag3p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag3p3, "phys_D4mag3p3_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag3p4

      //get parameters from .xml file
      GearDir cD4mag3p4(content, "D4mag3p4/");

      const int D4mag3p4_num = atoi(cD4mag3p4.getString("N").c_str());

      double D4mag3p4_Z[D4mag3p4_num];
      double D4mag3p4_R[D4mag3p4_num];
      double D4mag3p4_r[D4mag3p4_num];

      for (int i = 0; i < D4mag3p4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag3p4_Z[i] = cD4mag3p4.getLength(ossZID.str()) * unitFactor;
        D4mag3p4_R[i] = cD4mag3p4.getLength(ossRID.str()) * unitFactor;
        D4mag3p4_r[i] = cD4mag3p4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag3p4 = new G4Polycone("geo_D4mag3p4xx_name", 0, 2 * M_PI, D4mag3p4_num, D4mag3p4_Z, D4mag3p4_r, D4mag3p4_R);
      //G4IntersectionSolid* geo_D4mag3p4 = new G4IntersectionSolid("geo_D4mag3p4_name", geo_D4mag3p4xx, geo_D1spc1);

      string strMat_D4mag3p4 = cD4mag3p4.getString("Material");
      G4Material* mat_D4mag3p4 = Materials::get(strMat_D4mag3p4);
      G4LogicalVolume* logi_D4mag3p4 = new G4LogicalVolume(geo_D4mag3p4, mat_D4mag3p4, "logi_D4mag3p4_name");

      //put volume
      setColor(*logi_D4mag3p4, cD4mag3p4.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag3p4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag3p4, "phys_D4mag3p4_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag3p5

      //get parameters from .xml file
      GearDir cD4mag3p5(content, "D4mag3p5/");

      const int D4mag3p5_num = atoi(cD4mag3p5.getString("N").c_str());

      double D4mag3p5_Z[D4mag3p5_num];
      double D4mag3p5_R[D4mag3p5_num];
      double D4mag3p5_r[D4mag3p5_num];

      for (int i = 0; i < D4mag3p5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag3p5_Z[i] = cD4mag3p5.getLength(ossZID.str()) * unitFactor;
        D4mag3p5_R[i] = cD4mag3p5.getLength(ossRID.str()) * unitFactor;
        D4mag3p5_r[i] = cD4mag3p5.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag3p5 = new G4Polycone("geo_D4mag3p5xx_name", 0, 2 * M_PI, D4mag3p5_num, D4mag3p5_Z, D4mag3p5_r, D4mag3p5_R);
      //G4IntersectionSolid* geo_D4mag3p5 = new G4IntersectionSolid("geo_D4mag3p5_name", geo_D4mag3p5xx, geo_D1spc1);

      string strMat_D4mag3p5 = cD4mag3p5.getString("Material");
      G4Material* mat_D4mag3p5 = Materials::get(strMat_D4mag3p5);
      G4LogicalVolume* logi_D4mag3p5 = new G4LogicalVolume(geo_D4mag3p5, mat_D4mag3p5, "logi_D4mag3p5_name");

      //put volume
      setColor(*logi_D4mag3p5, cD4mag3p5.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag3p5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag3p5, "phys_D4mag3p5_name", logi_D1spc1, false, 0);

      //--------------
      //-   D4mag3p6

      //get parameters from .xml file
      GearDir cD4mag3p6(content, "D4mag3p6/");

      const int D4mag3p6_num = atoi(cD4mag3p6.getString("N").c_str());

      double D4mag3p6_Z[D4mag3p6_num];
      double D4mag3p6_R[D4mag3p6_num];
      double D4mag3p6_r[D4mag3p6_num];

      for (int i = 0; i < D4mag3p6_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D4mag3p6_Z[i] = cD4mag3p6.getLength(ossZID.str()) * unitFactor;
        D4mag3p6_R[i] = cD4mag3p6.getLength(ossRID.str()) * unitFactor;
        D4mag3p6_r[i] = cD4mag3p6.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_D4mag3p6 = new G4Polycone("geo_D4mag3p6xx_name", 0, 2 * M_PI, D4mag3p6_num, D4mag3p6_Z, D4mag3p6_r, D4mag3p6_R);
      //G4IntersectionSolid* geo_D4mag3p6 = new G4IntersectionSolid("geo_D4mag3p6_name", geo_D4mag3p6xx, geo_D1spc1);

      string strMat_D4mag3p6 = cD4mag3p6.getString("Material");
      G4Material* mat_D4mag3p6 = Materials::get(strMat_D4mag3p6);
      G4LogicalVolume* logi_D4mag3p6 = new G4LogicalVolume(geo_D4mag3p6, mat_D4mag3p6, "logi_D4mag3p6_name");

      //put volume
      setColor(*logi_D4mag3p6, cD4mag3p6.getString("Color", "#CC0000"));
      //setVisibility(*logi_D4mag3p6, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_D4mag3p6, "phys_D4mag3p6_name", logi_D1spc1, false, 0);

      //--------------
      //-   E2wal1

      //get parameters from .xml file
      GearDir cE2wal1(content, "E2wal1/");

      const int E2wal1_num = atoi(cE2wal1.getString("N").c_str());

      double E2wal1_Z[E2wal1_num];
      double E2wal1_R[E2wal1_num];
      double E2wal1_r[E2wal1_num];

      for (int i = 0; i < E2wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E2wal1_Z[i] = cE2wal1.getLength(ossZID.str()) * unitFactor;
        E2wal1_R[i] = cE2wal1.getLength(ossRID.str()) * unitFactor;
        E2wal1_r[i] = cE2wal1.getLength(ossrID.str()) * unitFactor;
      }

      G4Polycone* geo_E2wal1xx = new G4Polycone("geo_E2wal1xx_name", 0, 2 * M_PI, E2wal1_num, E2wal1_Z, E2wal1_r, E2wal1_R);
      //G4IntersectionSolid* geo_E2wal1 = new G4IntersectionSolid("geo_E2wal1_name", geo_E2wal1xx, geo_E1spc1);
      G4IntersectionSolid* geo_E2wal1 = new G4IntersectionSolid("geo_E2wal1_name", geo_E2wal1xx, geo_TubeL, transform_LER_inv);

      string strMat_E2wal1 = cE2wal1.getString("Material");
      G4Material* mat_E2wal1 = Materials::get(strMat_E2wal1);
      G4LogicalVolume* logi_E2wal1 = new G4LogicalVolume(geo_E2wal1, mat_E2wal1, "logi_E2wal1_name");

      //put volume
      setColor(*logi_E2wal1, cE2wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_E2wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E2wal1, "phys_E2wal1_name", logi_E1spc1, false, 0);

      //--------------
      //-   E3wal1

      //get parameters from .xml file
      GearDir cE3wal1(content, "E3wal1/");

      const int E3wal1_num = atoi(cE3wal1.getString("N").c_str());

      double E3wal1_Z[E3wal1_num];
      double E3wal1_R[E3wal1_num];
      double E3wal1_r[E3wal1_num];

      for (int i = 0; i < E3wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E3wal1_Z[i] = cE3wal1.getLength(ossZID.str()) * unitFactor;
        E3wal1_R[i] = cE3wal1.getLength(ossRID.str()) * unitFactor;
        E3wal1_r[i] = cE3wal1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E3wal1xx = new G4Polycone("geo_E3wal1xx_name", 0, 2 * M_PI, E3wal1_num, E3wal1_Z, E3wal1_r, E3wal1_R);
      G4IntersectionSolid* geo_E3wal1 = new G4IntersectionSolid("geo_E3wal1_name", geo_E3wal1xx, geo_E1spc1);

      string strMat_E3wal1 = cE3wal1.getString("Material");
      G4Material* mat_E3wal1 = Materials::get(strMat_E3wal1);
      G4LogicalVolume* logi_E3wal1 = new G4LogicalVolume(geo_E3wal1, mat_E3wal1, "logi_E3wal1_name");

      //put volume
      setColor(*logi_E3wal1, cE3wal1.getString("Color", "#CC0000"));
      //setVisibility(*logi_E3wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E3wal1, "phys_E3wal1_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag1p1

      //get parameters from .xml file
      GearDir cE4mag1p1(content, "E4mag1p1/");

      const int E4mag1p1_num = atoi(cE4mag1p1.getString("N").c_str());

      double E4mag1p1_Z[E4mag1p1_num];
      double E4mag1p1_R[E4mag1p1_num];
      double E4mag1p1_r[E4mag1p1_num];

      for (int i = 0; i < E4mag1p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag1p1_Z[i] = cE4mag1p1.getLength(ossZID.str()) * unitFactor;
        E4mag1p1_R[i] = cE4mag1p1.getLength(ossRID.str()) * unitFactor;
        E4mag1p1_r[i] = cE4mag1p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag1p1 = new G4Polycone("geo_E4mag1p1xx_name", 0, 2 * M_PI, E4mag1p1_num, E4mag1p1_Z, E4mag1p1_r, E4mag1p1_R);
      //G4IntersectionSolid* geo_E4mag1p1 = new G4IntersectionSolid("geo_E4mag1p1_name", geo_E4mag1p1xx, geo_E1spc1);

      string strMat_E4mag1p1 = cE4mag1p1.getString("Material");
      G4Material* mat_E4mag1p1 = Materials::get(strMat_E4mag1p1);
      G4LogicalVolume* logi_E4mag1p1 = new G4LogicalVolume(geo_E4mag1p1, mat_E4mag1p1, "logi_E4mag1p1_name");

      //put volume
      setColor(*logi_E4mag1p1, cE4mag1p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag1p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag1p1, "phys_E4mag1p1_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag1p2

      //get parameters from .xml file
      GearDir cE4mag1p2(content, "E4mag1p2/");

      const int E4mag1p2_num = atoi(cE4mag1p2.getString("N").c_str());

      double E4mag1p2_Z[E4mag1p2_num];
      double E4mag1p2_R[E4mag1p2_num];
      double E4mag1p2_r[E4mag1p2_num];

      for (int i = 0; i < E4mag1p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag1p2_Z[i] = cE4mag1p2.getLength(ossZID.str()) * unitFactor;
        E4mag1p2_R[i] = cE4mag1p2.getLength(ossRID.str()) * unitFactor;
        E4mag1p2_r[i] = cE4mag1p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag1p2 = new G4Polycone("geo_E4mag1p2xx_name", 0, 2 * M_PI, E4mag1p2_num, E4mag1p2_Z, E4mag1p2_r, E4mag1p2_R);
      //G4IntersectionSolid* geo_E4mag1p2 = new G4IntersectionSolid("geo_E4mag1p2_name", geo_E4mag1p2xx, geo_E1spc1);

      string strMat_E4mag1p2 = cE4mag1p2.getString("Material");
      G4Material* mat_E4mag1p2 = Materials::get(strMat_E4mag1p2);
      G4LogicalVolume* logi_E4mag1p2 = new G4LogicalVolume(geo_E4mag1p2, mat_E4mag1p2, "logi_E4mag1p2_name");

      //put volume
      setColor(*logi_E4mag1p2, cE4mag1p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag1p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag1p2, "phys_E4mag1p2_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag1p3

      //get parameters from .xml file
      GearDir cE4mag1p3(content, "E4mag1p3/");

      const int E4mag1p3_num = atoi(cE4mag1p3.getString("N").c_str());

      double E4mag1p3_Z[E4mag1p3_num];
      double E4mag1p3_R[E4mag1p3_num];
      double E4mag1p3_r[E4mag1p3_num];

      for (int i = 0; i < E4mag1p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag1p3_Z[i] = cE4mag1p3.getLength(ossZID.str()) * unitFactor;
        E4mag1p3_R[i] = cE4mag1p3.getLength(ossRID.str()) * unitFactor;
        E4mag1p3_r[i] = cE4mag1p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag1p3xx = new G4Polycone("geo_E4mag1p3xx_name", 0, 2 * M_PI, E4mag1p3_num, E4mag1p3_Z, E4mag1p3_r, E4mag1p3_R);
      //G4IntersectionSolid* geo_E4mag1p3 = new G4IntersectionSolid("geo_E4mag1p3_name", geo_E4mag1p3xx, geo_E1spc1);
      // to avoid strange overlap error
      G4SubtractionSolid* geo_E4mag1p3 = new G4SubtractionSolid("geo_E4mag1p3_name", geo_E4mag1p3xx, geo_E3wal1);

      string strMat_E4mag1p3 = cE4mag1p3.getString("Material");
      G4Material* mat_E4mag1p3 = Materials::get(strMat_E4mag1p3);
      G4LogicalVolume* logi_E4mag1p3 = new G4LogicalVolume(geo_E4mag1p3, mat_E4mag1p3, "logi_E4mag1p3_name");

      //put volume
      setColor(*logi_E4mag1p3, cE4mag1p3.getString("Color", "#CC0000"));
      setVisibility(*logi_E4mag1p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag1p3, "phys_E4mag1p3_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag2

      //get parameters from .xml file
      GearDir cE4mag2(content, "E4mag2/");

      const int E4mag2_num = atoi(cE4mag2.getString("N").c_str());

      double E4mag2_Z[E4mag2_num];
      double E4mag2_R[E4mag2_num];
      double E4mag2_r[E4mag2_num];

      for (int i = 0; i < E4mag2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag2_Z[i] = cE4mag2.getLength(ossZID.str()) * unitFactor;
        E4mag2_R[i] = cE4mag2.getLength(ossRID.str()) * unitFactor;
        E4mag2_r[i] = cE4mag2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag2xx = new G4Polycone("geo_E4mag2xx_name", 0, 2 * M_PI, E4mag2_num, E4mag2_Z, E4mag2_r, E4mag2_R);
      //G4IntersectionSolid* geo_E4mag2 = new G4IntersectionSolid("geo_E4mag2_name", geo_E4mag2xx, geo_E1spc1);
      // to avoid strange overlap error
      G4SubtractionSolid* geo_E4mag2 = new G4SubtractionSolid("geo_E4mag2_name", geo_E4mag2xx, geo_E3wal1);

      string strMat_E4mag2 = cE4mag2.getString("Material");
      G4Material* mat_E4mag2 = Materials::get(strMat_E4mag2);
      G4LogicalVolume* logi_E4mag2 = new G4LogicalVolume(geo_E4mag2, mat_E4mag2, "logi_E4mag2_name");

      //put volume
      setColor(*logi_E4mag2, cE4mag2.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag2, "phys_E4mag2_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag3p1

      //get parameters from .xml file
      GearDir cE4mag3p1(content, "E4mag3p1/");

      const int E4mag3p1_num = atoi(cE4mag3p1.getString("N").c_str());

      double E4mag3p1_Z[E4mag3p1_num];
      double E4mag3p1_R[E4mag3p1_num];
      double E4mag3p1_r[E4mag3p1_num];

      for (int i = 0; i < E4mag3p1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag3p1_Z[i] = cE4mag3p1.getLength(ossZID.str()) * unitFactor;
        E4mag3p1_R[i] = cE4mag3p1.getLength(ossRID.str()) * unitFactor;
        E4mag3p1_r[i] = cE4mag3p1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag3p1xx = new G4Polycone("geo_E4mag3p1xx_name", 0, 2 * M_PI, E4mag3p1_num, E4mag3p1_Z, E4mag3p1_r, E4mag3p1_R);
      //G4IntersectionSolid* geo_E4mag3p1 = new G4IntersectionSolid("geo_E4mag3p1_name", geo_E4mag3p1xx, geo_E1spc1);
      // to avoid strange overlap error
      G4SubtractionSolid* geo_E4mag3p1 = new G4SubtractionSolid("geo_E4mag3p1_name", geo_E4mag3p1xx, geo_E3wal1);

      string strMat_E4mag3p1 = cE4mag3p1.getString("Material");
      G4Material* mat_E4mag3p1 = Materials::get(strMat_E4mag3p1);
      G4LogicalVolume* logi_E4mag3p1 = new G4LogicalVolume(geo_E4mag3p1, mat_E4mag3p1, "logi_E4mag3p1_name");

      //put volume
      setColor(*logi_E4mag3p1, cE4mag3p1.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag3p1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag3p1, "phys_E4mag3p1_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag3p2

      //get parameters from .xml file
      GearDir cE4mag3p2(content, "E4mag3p2/");

      const int E4mag3p2_num = atoi(cE4mag3p2.getString("N").c_str());

      double E4mag3p2_Z[E4mag3p2_num];
      double E4mag3p2_R[E4mag3p2_num];
      double E4mag3p2_r[E4mag3p2_num];

      for (int i = 0; i < E4mag3p2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag3p2_Z[i] = cE4mag3p2.getLength(ossZID.str()) * unitFactor;
        E4mag3p2_R[i] = cE4mag3p2.getLength(ossRID.str()) * unitFactor;
        E4mag3p2_r[i] = cE4mag3p2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag3p2xx = new G4Polycone("geo_E4mag3p2xx_name", 0, 2 * M_PI, E4mag3p2_num, E4mag3p2_Z, E4mag3p2_r, E4mag3p2_R);
      //G4IntersectionSolid* geo_E4mag3p2 = new G4IntersectionSolid("geo_E4mag3p2_name", geo_E4mag3p2xx, geo_E1spc1);
      // to avoid strange overlap error
      G4SubtractionSolid* geo_E4mag3p2 = new G4SubtractionSolid("geo_E4mag3p2_name", geo_E4mag3p2xx, geo_E3wal1);

      string strMat_E4mag3p2 = cE4mag3p2.getString("Material");
      G4Material* mat_E4mag3p2 = Materials::get(strMat_E4mag3p2);
      G4LogicalVolume* logi_E4mag3p2 = new G4LogicalVolume(geo_E4mag3p2, mat_E4mag3p2, "logi_E4mag3p2_name");

      //put volume
      setColor(*logi_E4mag3p2, cE4mag3p2.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag3p2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag3p2, "phys_E4mag3p2_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag3p3

      //get parameters from .xml file
      GearDir cE4mag3p3(content, "E4mag3p3/");

      const int E4mag3p3_num = atoi(cE4mag3p3.getString("N").c_str());

      double E4mag3p3_Z[E4mag3p3_num];
      double E4mag3p3_R[E4mag3p3_num];
      double E4mag3p3_r[E4mag3p3_num];

      for (int i = 0; i < E4mag3p3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag3p3_Z[i] = cE4mag3p3.getLength(ossZID.str()) * unitFactor;
        E4mag3p3_R[i] = cE4mag3p3.getLength(ossRID.str()) * unitFactor;
        E4mag3p3_r[i] = cE4mag3p3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag3p3 = new G4Polycone("geo_E4mag3p3xx_name", 0, 2 * M_PI, E4mag3p3_num, E4mag3p3_Z, E4mag3p3_r, E4mag3p3_R);
      //G4IntersectionSolid* geo_E4mag3p3 = new G4IntersectionSolid("geo_E4mag3p3_name", geo_E4mag3p3xx, geo_E1spc1);

      string strMat_E4mag3p3 = cE4mag3p3.getString("Material");
      G4Material* mat_E4mag3p3 = Materials::get(strMat_E4mag3p3);
      G4LogicalVolume* logi_E4mag3p3 = new G4LogicalVolume(geo_E4mag3p3, mat_E4mag3p3, "logi_E4mag3p3_name");

      //put volume
      setColor(*logi_E4mag3p3, cE4mag3p3.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag3p3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag3p3, "phys_E4mag3p3_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag3p4

      //get parameters from .xml file
      GearDir cE4mag3p4(content, "E4mag3p4/");

      const int E4mag3p4_num = atoi(cE4mag3p4.getString("N").c_str());

      double E4mag3p4_Z[E4mag3p4_num];
      double E4mag3p4_R[E4mag3p4_num];
      double E4mag3p4_r[E4mag3p4_num];

      for (int i = 0; i < E4mag3p4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag3p4_Z[i] = cE4mag3p4.getLength(ossZID.str()) * unitFactor;
        E4mag3p4_R[i] = cE4mag3p4.getLength(ossRID.str()) * unitFactor;
        E4mag3p4_r[i] = cE4mag3p4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag3p4 = new G4Polycone("geo_E4mag3p4xx_name", 0, 2 * M_PI, E4mag3p4_num, E4mag3p4_Z, E4mag3p4_r, E4mag3p4_R);
      //G4IntersectionSolid* geo_E4mag3p4 = new G4IntersectionSolid("geo_E4mag3p4_name", geo_E4mag3p4xx, geo_E1spc1);

      string strMat_E4mag3p4 = cE4mag3p4.getString("Material");
      G4Material* mat_E4mag3p4 = Materials::get(strMat_E4mag3p4);
      G4LogicalVolume* logi_E4mag3p4 = new G4LogicalVolume(geo_E4mag3p4, mat_E4mag3p4, "logi_E4mag3p4_name");

      //put volume
      setColor(*logi_E4mag3p4, cE4mag3p4.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag3p4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag3p4, "phys_E4mag3p4_name", logi_E1spc1, false, 0);

      //--------------
      //-   E4mag3p5

      //get parameters from .xml file
      GearDir cE4mag3p5(content, "E4mag3p5/");

      const int E4mag3p5_num = atoi(cE4mag3p5.getString("N").c_str());

      double E4mag3p5_Z[E4mag3p5_num];
      double E4mag3p5_R[E4mag3p5_num];
      double E4mag3p5_r[E4mag3p5_num];

      for (int i = 0; i < E4mag3p5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E4mag3p5_Z[i] = cE4mag3p5.getLength(ossZID.str()) * unitFactor;
        E4mag3p5_R[i] = cE4mag3p5.getLength(ossRID.str()) * unitFactor;
        E4mag3p5_r[i] = cE4mag3p5.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_E4mag3p5 = new G4Polycone("geo_E4mag3p5xx_name", 0, 2 * M_PI, E4mag3p5_num, E4mag3p5_Z, E4mag3p5_r, E4mag3p5_R);
      //G4IntersectionSolid* geo_E4mag3p5 = new G4IntersectionSolid("geo_E4mag3p5_name", geo_E4mag3p5xx, geo_E1spc1);

      string strMat_E4mag3p5 = cE4mag3p5.getString("Material");
      G4Material* mat_E4mag3p5 = Materials::get(strMat_E4mag3p5);
      G4LogicalVolume* logi_E4mag3p5 = new G4LogicalVolume(geo_E4mag3p5, mat_E4mag3p5, "logi_E4mag3p5_name");

      //put volume
      setColor(*logi_E4mag3p5, cE4mag3p5.getString("Color", "#CC0000"));
      //setVisibility(*logi_E4mag3p5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_E4mag3p5, "phys_E4mag3p5_name", logi_E1spc1, false, 0);

      //--------------
      //-   F2wal1

      //get parameters from .xml file
      GearDir cF1wal1(content, "F1wal1/");

      const int F1wal1_num = atoi(cF1wal1.getString("N").c_str());

      double F1wal1_Z[F1wal1_num];
      double F1wal1_R[F1wal1_num];
      double F1wal1_r[F1wal1_num];

      for (int i = 0; i < F1wal1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F1wal1_Z[i] = cF1wal1.getLength(ossZID.str()) * unitFactor;
        F1wal1_R[i] = cF1wal1.getLength(ossRID.str()) * unitFactor;
        F1wal1_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F1wal1xx = new G4Polycone("geo_F1wal1xx_name", 0, 2 * M_PI, F1wal1_num, F1wal1_Z, F1wal1_r, F1wal1_R);
      G4SubtractionSolid* geo_F1wal1x = new G4SubtractionSolid("geo_F1wal1x_name", geo_F1wal1xx, geo_D1spc1, transform_HER);
      G4SubtractionSolid* geo_F1wal1 = new G4SubtractionSolid("geo_F1wal1_name", geo_F1wal1x, geo_E1spc1, transform_LER);

      string strMat_F1wal1 = cF1wal1.getString("Material");
      G4Material* mat_F1wal1 = Materials::get(strMat_F1wal1);
      G4LogicalVolume* logi_F1wal1 = new G4LogicalVolume(geo_F1wal1, mat_F1wal1, "logi_F1wal1_name");

      //logi_F1wal1->SetSensitiveDetector(m_sensitive);
      //logi_F1wal1->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1));

      //put volume
      setColor(*logi_F1wal1, cF1wal1.getString("Color", "#CC0000"));
      setVisibility(*logi_F1wal1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F1wal1, "phys_F1wal1_name", &topVolume, false, 0);

      //--------------
      //-   F2spc1

      //get parameters from .xml file
      GearDir cF2spc1(content, "F2spc1/");

      const int F2spc1_num = atoi(cF2spc1.getString("N").c_str());

      double F2spc1_Z[F2spc1_num];
      double F2spc1_R[F2spc1_num];
      double F2spc1_r[F2spc1_num];

      for (int i = 0; i < F2spc1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F2spc1_Z[i] = cF2spc1.getLength(ossZID.str()) * unitFactor;
        F2spc1_R[i] = cF2spc1.getLength(ossRID.str()) * unitFactor;
        F2spc1_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F2spc1xx = new G4Polycone("geo_F2spc1xx_name", 0, 2 * M_PI, F2spc1_num, F2spc1_Z, F2spc1_r, F2spc1_R);
      G4IntersectionSolid* geo_F2spc1 = new G4IntersectionSolid("geo_F2spc1_name", geo_F2spc1xx, geo_F1wal1);

      string strMat_F2spc1 = cF2spc1.getString("Material");
      G4Material* mat_F2spc1 = Materials::get(strMat_F2spc1);
      G4LogicalVolume* logi_F2spc1 = new G4LogicalVolume(geo_F2spc1, mat_F2spc1, "logi_F2spc1_name");

      //put volume
      setColor(*logi_F2spc1, cF2spc1.getString("Color", "#CC0000"));
      setVisibility(*logi_F2spc1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F2spc1, "phys_F2spc1_name", logi_F1wal1, false, 0);

      //--------------
      //-   F3wal2

      //get parameters from .xml file
      GearDir cF3wal2(content, "F3wal2/");

      const int F3wal2_num = atoi(cF3wal2.getString("N").c_str());

      double F3wal2_Z[F3wal2_num];
      double F3wal2_R[F3wal2_num];
      double F3wal2_r[F3wal2_num];

      for (int i = 0; i < F3wal2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F3wal2_Z[i] = cF3wal2.getLength(ossZID.str()) * unitFactor;
        F3wal2_R[i] = cF3wal2.getLength(ossRID.str()) * unitFactor;
        F3wal2_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F3wal2xx = new G4Polycone("geo_F3wal2xx_name", 0, 2 * M_PI, F3wal2_num, F3wal2_Z, F3wal2_r, F3wal2_R);
      G4IntersectionSolid* geo_F3wal2 = new G4IntersectionSolid("geo_F3wal2_name", geo_F3wal2xx, geo_F1wal1);

      string strMat_F3wal2 = cF3wal2.getString("Material");
      G4Material* mat_F3wal2 = Materials::get(strMat_F3wal2);
      G4LogicalVolume* logi_F3wal2 = new G4LogicalVolume(geo_F3wal2, mat_F3wal2, "logi_F3wal2_name");

      //put volume
      setColor(*logi_F3wal2, cF3wal2.getString("Color", "#CC0000"));
      setVisibility(*logi_F3wal2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F3wal2, "phys_F3wal2_name", logi_F2spc1, false, 0);

      //--------------
      //-   F3wal3

      //get parameters from .xml file
      GearDir cF3wal3(content, "F3wal3/");

      const int F3wal3_num = atoi(cF3wal3.getString("N").c_str());

      double F3wal3_Z[F3wal3_num];
      double F3wal3_R[F3wal3_num];
      double F3wal3_r[F3wal3_num];

      for (int i = 0; i < F3wal3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F3wal3_Z[i] = cF3wal3.getLength(ossZID.str()) * unitFactor;
        F3wal3_R[i] = cF3wal3.getLength(ossRID.str()) * unitFactor;
        F3wal3_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F3wal3xx = new G4Polycone("geo_F3wal3xx_name", 0, 2 * M_PI, F3wal3_num, F3wal3_Z, F3wal3_r, F3wal3_R);
      G4IntersectionSolid* geo_F3wal3 = new G4IntersectionSolid("geo_F3wal3_name", geo_F3wal3xx, geo_F1wal1);

      string strMat_F3wal3 = cF3wal3.getString("Material");
      G4Material* mat_F3wal3 = Materials::get(strMat_F3wal3);
      G4LogicalVolume* logi_F3wal3 = new G4LogicalVolume(geo_F3wal3, mat_F3wal3, "logi_F3wal3_name");

      //put volume
      setColor(*logi_F3wal3, cF3wal3.getString("Color", "#CC0000"));
      setVisibility(*logi_F3wal3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F3wal3, "phys_F3wal3_name", logi_F2spc1, false, 0);

      //--------------
      //-   F3wal4

      //get parameters from .xml file
      GearDir cF3wal4(content, "F3wal4/");

      const int F3wal4_num = atoi(cF3wal4.getString("N").c_str());

      double F3wal4_Z[F3wal4_num];
      double F3wal4_R[F3wal4_num];
      double F3wal4_r[F3wal4_num];

      for (int i = 0; i < F3wal4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F3wal4_Z[i] = cF3wal4.getLength(ossZID.str()) * unitFactor;
        F3wal4_R[i] = cF3wal4.getLength(ossRID.str()) * unitFactor;
        F3wal4_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F3wal4xx = new G4Polycone("geo_F3wal4xx_name", 0, 2 * M_PI, F3wal4_num, F3wal4_Z, F3wal4_r, F3wal4_R);
      G4IntersectionSolid* geo_F3wal4 = new G4IntersectionSolid("geo_F3wal4_name", geo_F3wal4xx, geo_F1wal1);

      string strMat_F3wal4 = cF3wal4.getString("Material");
      G4Material* mat_F3wal4 = Materials::get(strMat_F3wal4);
      G4LogicalVolume* logi_F3wal4 = new G4LogicalVolume(geo_F3wal4, mat_F3wal4, "logi_F3wal4_name");

      //put volume
      setColor(*logi_F3wal4, cF3wal4.getString("Color", "#CC0000"));
      setVisibility(*logi_F3wal4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F3wal4, "phys_F3wal4_name", logi_F2spc1, false, 0);

      //--------------
      //-   F4spc2

      //get parameters from .xml file
      GearDir cF4spc2(content, "F4spc2/");

      const int F4spc2_num = atoi(cF4spc2.getString("N").c_str());

      double F4spc2_Z[F4spc2_num];
      double F4spc2_R[F4spc2_num];
      double F4spc2_r[F4spc2_num];

      for (int i = 0; i < F4spc2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F4spc2_Z[i] = cF4spc2.getLength(ossZID.str()) * unitFactor;
        F4spc2_R[i] = cF4spc2.getLength(ossRID.str()) * unitFactor;
        F4spc2_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F4spc2xx = new G4Polycone("geo_F4spc2xx_name", 0, 2 * M_PI, F4spc2_num, F4spc2_Z, F4spc2_r, F4spc2_R);
      G4IntersectionSolid* geo_F4spc2 = new G4IntersectionSolid("geo_F4spc2_name", geo_F4spc2xx, geo_F1wal1);

      string strMat_F4spc2 = cF4spc2.getString("Material");
      G4Material* mat_F4spc2 = Materials::get(strMat_F4spc2);
      G4LogicalVolume* logi_F4spc2 = new G4LogicalVolume(geo_F4spc2, mat_F4spc2, "logi_F4spc2_name");

      //put volume
      setColor(*logi_F4spc2, cF4spc2.getString("Color", "#CC0000"));
      setVisibility(*logi_F4spc2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F4spc2, "phys_F4spc2_name", logi_F3wal2, false, 0);

      //--------------
      //-   F5wal3

      //get parameters from .xml file
      GearDir cF5wal3(content, "F5wal3/");

      const int F5wal3_num = atoi(cF5wal3.getString("N").c_str());

      double F5wal3_Z[F5wal3_num];
      double F5wal3_R[F5wal3_num];
      double F5wal3_r[F5wal3_num];

      for (int i = 0; i < F5wal3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F5wal3_Z[i] = cF5wal3.getLength(ossZID.str()) * unitFactor;
        F5wal3_R[i] = cF5wal3.getLength(ossRID.str()) * unitFactor;
        F5wal3_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F5wal3xx = new G4Polycone("geo_F5wal3xx_name", 0, 2 * M_PI, F5wal3_num, F5wal3_Z, F5wal3_r, F5wal3_R);
      G4IntersectionSolid* geo_F5wal3 = new G4IntersectionSolid("geo_F5wal3_name", geo_F5wal3xx, geo_F1wal1);

      string strMat_F5wal3 = cF5wal3.getString("Material");
      G4Material* mat_F5wal3 = Materials::get(strMat_F5wal3);
      G4LogicalVolume* logi_F5wal3 = new G4LogicalVolume(geo_F5wal3, mat_F5wal3, "logi_F5wal3_name");

      //put volume
      setColor(*logi_F5wal3, cF5wal3.getString("Color", "#CC0000"));
      setVisibility(*logi_F5wal3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F5wal3, "phys_F5wal3_name", logi_F4spc2, false, 0);

      //--------------
      //-   F6spc3

      //get parameters from .xml file
      GearDir cF6spc3(content, "F6spc3/");

      const int F6spc3_num = atoi(cF6spc3.getString("N").c_str());

      double F6spc3_Z[F6spc3_num];
      double F6spc3_R[F6spc3_num];
      double F6spc3_r[F6spc3_num];

      for (int i = 0; i < F6spc3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F6spc3_Z[i] = cF6spc3.getLength(ossZID.str()) * unitFactor;
        F6spc3_R[i] = cF6spc3.getLength(ossRID.str()) * unitFactor;
        F6spc3_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F6spc3xx = new G4Polycone("geo_F6spc3xx_name", 0, 2 * M_PI, F6spc3_num, F6spc3_Z, F6spc3_r, F6spc3_R);
      G4IntersectionSolid* geo_F6spc3 = new G4IntersectionSolid("geo_F6spc3_name", geo_F6spc3xx, geo_F1wal1);

      string strMat_F6spc3 = cF6spc3.getString("Material");
      G4Material* mat_F6spc3 = Materials::get(strMat_F6spc3);
      G4LogicalVolume* logi_F6spc3 = new G4LogicalVolume(geo_F6spc3, mat_F6spc3, "logi_F6spc3_name");

      //put volume
      setColor(*logi_F6spc3, cF6spc3.getString("Color", "#CC0000"));
      setVisibility(*logi_F6spc3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F6spc3, "phys_F6spc3_name", logi_F5wal3, false, 0);

      //--------------
      //-   F7lyr1

      //get parameters from .xml file
      GearDir cF7lyr1(content, "F7lyr1/");

      const int F7lyr1_num = atoi(cF7lyr1.getString("N").c_str());

      double F7lyr1_Z[F7lyr1_num];
      double F7lyr1_R[F7lyr1_num];
      double F7lyr1_r[F7lyr1_num];

      for (int i = 0; i < F7lyr1_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        F7lyr1_Z[i] = cF7lyr1.getLength(ossZID.str()) * unitFactor;
        F7lyr1_R[i] = cF7lyr1.getLength(ossRID.str()) * unitFactor;
        F7lyr1_r[i] = cF7lyr1.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_F7lyr1xx = new G4Polycone("geo_F7lyr1xx_name", 0, 2 * M_PI, F7lyr1_num, F7lyr1_Z, F7lyr1_r, F7lyr1_R);
      G4IntersectionSolid* geo_F7lyr1 = new G4IntersectionSolid("geo_F7lyr1_name", geo_F7lyr1xx, geo_F1wal1);

      string strMat_F7lyr1 = cF7lyr1.getString("Material");
      G4Material* mat_F7lyr1 = Materials::get(strMat_F7lyr1);
      G4LogicalVolume* logi_F7lyr1 = new G4LogicalVolume(geo_F7lyr1, mat_F7lyr1, "logi_F7lyr1_name");

      //put volume
      setColor(*logi_F7lyr1, cF7lyr1.getString("Color", "#CC0000"));
      setVisibility(*logi_F7lyr1, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7lyr1, "phys_F7lyr1_name", logi_F6spc3, false, 0);

      //--------------
      //-   F7lyr2

      //get parameters from .xml file
      GearDir cF7lyr2(content, "F7lyr2/");

      const int F7lyr2_num = atoi(cF7lyr2.getString("N").c_str());

      double F7lyr2_Z[F7lyr2_num];
      double F7lyr2_R[F7lyr2_num];
      double F7lyr2_r[F7lyr2_num];

      for (int i = 0; i < F7lyr2_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        F7lyr2_Z[i] = cF7lyr2.getLength(ossZID.str()) * unitFactor;
        F7lyr2_R[i] = cF7lyr2.getLength(ossRID.str()) * unitFactor;
        F7lyr2_r[i] = cF7lyr2.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_F7lyr2xx = new G4Polycone("geo_F7lyr2xx_name", 0, 2 * M_PI, F7lyr2_num, F7lyr2_Z, F7lyr2_r, F7lyr2_R);
      G4IntersectionSolid* geo_F7lyr2 = new G4IntersectionSolid("geo_F7lyr2_name", geo_F7lyr2xx, geo_F1wal1);

      string strMat_F7lyr2 = cF7lyr2.getString("Material");
      G4Material* mat_F7lyr2 = Materials::get(strMat_F7lyr2);
      G4LogicalVolume* logi_F7lyr2 = new G4LogicalVolume(geo_F7lyr2, mat_F7lyr2, "logi_F7lyr2_name");

      //put volume
      setColor(*logi_F7lyr2, cF7lyr2.getString("Color", "#CC0000"));
      setVisibility(*logi_F7lyr2, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7lyr2, "phys_F7lyr2_name", logi_F6spc3, false, 0);

      //--------------
      //-   F7lyr3

      //get parameters from .xml file
      GearDir cF7lyr3(content, "F7lyr3/");

      const int F7lyr3_num = atoi(cF7lyr3.getString("N").c_str());

      double F7lyr3_Z[F7lyr3_num];
      double F7lyr3_R[F7lyr3_num];
      double F7lyr3_r[F7lyr3_num];

      for (int i = 0; i < F7lyr3_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        F7lyr3_Z[i] = cF7lyr3.getLength(ossZID.str()) * unitFactor;
        F7lyr3_R[i] = cF7lyr3.getLength(ossRID.str()) * unitFactor;
        F7lyr3_r[i] = cF7lyr3.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_F7lyr3xx = new G4Polycone("geo_F7lyr3xx_name", 0, 2 * M_PI, F7lyr3_num, F7lyr3_Z, F7lyr3_r, F7lyr3_R);
      G4IntersectionSolid* geo_F7lyr3 = new G4IntersectionSolid("geo_F7lyr3_name", geo_F7lyr3xx, geo_F1wal1);

      string strMat_F7lyr3 = cF7lyr3.getString("Material");
      G4Material* mat_F7lyr3 = Materials::get(strMat_F7lyr3);
      G4LogicalVolume* logi_F7lyr3 = new G4LogicalVolume(geo_F7lyr3, mat_F7lyr3, "logi_F7lyr3_name");

      //put volume
      setColor(*logi_F7lyr3, cF7lyr3.getString("Color", "#CC0000"));
      setVisibility(*logi_F7lyr3, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7lyr3, "phys_F7lyr3_name", logi_F6spc3, false, 0);

      //--------------
      //-   F7lyr4

      //get parameters from .xml file
      GearDir cF7lyr4(content, "F7lyr4/");

      const int F7lyr4_num = atoi(cF7lyr4.getString("N").c_str());

      double F7lyr4_Z[F7lyr4_num];
      double F7lyr4_R[F7lyr4_num];
      double F7lyr4_r[F7lyr4_num];

      for (int i = 0; i < F7lyr4_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        F7lyr4_Z[i] = cF7lyr4.getLength(ossZID.str()) * unitFactor;
        F7lyr4_R[i] = cF7lyr4.getLength(ossRID.str()) * unitFactor;
        F7lyr4_r[i] = cF7lyr4.getLength(ossrID.str()) * unitFactor;
      }

      //define geometry
      G4Polycone* geo_F7lyr4xx = new G4Polycone("geo_F7lyr4xx_name", 0, 2 * M_PI, F7lyr4_num, F7lyr4_Z, F7lyr4_r, F7lyr4_R);
      G4IntersectionSolid* geo_F7lyr4 = new G4IntersectionSolid("geo_F7lyr4_name", geo_F7lyr4xx, geo_F1wal1);

      string strMat_F7lyr4 = cF7lyr4.getString("Material");
      G4Material* mat_F7lyr4 = Materials::get(strMat_F7lyr4);
      G4LogicalVolume* logi_F7lyr4 = new G4LogicalVolume(geo_F7lyr4, mat_F7lyr4, "logi_F7lyr4_name");

      //put volume
      setColor(*logi_F7lyr4, cF7lyr4.getString("Color", "#CC0000"));
      setVisibility(*logi_F7lyr4, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7lyr4, "phys_F7lyr4_name", logi_F6spc3, false, 0);

      //--------------
      //-   F7lyr5

      //get parameters from .xml file
      GearDir cF7lyr5(content, "F7lyr5/");

      const int F7lyr5_num = atoi(cF7lyr5.getString("N").c_str());

      double F7lyr5_Z[F7lyr5_num];
      double F7lyr5_R[F7lyr5_num];
      double F7lyr5_r[F7lyr5_num];

      for (int i = 0; i < F7lyr5_num; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        F7lyr5_Z[i] = cF7lyr5.getLength(ossZID.str()) * unitFactor;
        F7lyr5_R[i] = cF7lyr5.getLength(ossRID.str()) * unitFactor;
        F7lyr5_r[i] = 0.0;
      }

      //define geometry
      G4Polycone* geo_F7lyr5xx = new G4Polycone("geo_F7lyr5xx_name", 0, 2 * M_PI, F7lyr5_num, F7lyr5_Z, F7lyr5_r, F7lyr5_R);
      G4IntersectionSolid* geo_F7lyr5 = new G4IntersectionSolid("geo_F7lyr5_name", geo_F7lyr5xx, geo_F1wal1);

      string strMat_F7lyr5 = cF7lyr5.getString("Material");
      G4Material* mat_F7lyr5 = Materials::get(strMat_F7lyr5);
      G4LogicalVolume* logi_F7lyr5 = new G4LogicalVolume(geo_F7lyr5, mat_F7lyr5, "logi_F7lyr5_name");

      //put volume
      setColor(*logi_F7lyr5, cF7lyr5.getString("Color", "#CC0000"));
      setVisibility(*logi_F7lyr5, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_F7lyr5, "phys_F7lyr5_name", logi_F6spc3, false, 0);

    }
  }
}
