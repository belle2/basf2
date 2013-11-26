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

    void GeoCryostatCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      //######  L side index  ######
      //
      // +- A1spc1+A1spc2
      //    +- A2wal1
      //    +- A3wal1
      //    +- A3wal2
      //    +- A4mag1
      //    +- A4mag2
      //    +- A4mag3
      //    +- A4mag4
      //
      // +- B1spc1+B1spc2
      //    +- B2wal1
      //    +- B3wal1
      //    +- B3wal2
      //    +- B4mag1
      //    +- B4mag2
      //    +- B4mag3
      //    +- B4mag4
      //
      // +- C1wal1
      //    +- C2spc1
      //       +- C3wal1
      //          +- C4spc1
      //             +- C5wal1
      //                +- C6spc1
      //                   +- C7lyr1
      //                   +- C7lyr2
      //                   +- C7lyr3
      //                   +- C7lyr4
      //                   +- C7lyr5
      //    +- C2spc2
      //       +- C3wal2
      //          +- C4spc2
      //             +- C5wal2
      //                +- C6spc2
      //                   +- C7wal1
      //             +- C5wal3
      //       +- C3wal3
      //    +- C2spc3
      // +- C1wal2
      //
      //######  R side index  ######
      //
      // +- D1spc1
      //    +- D2wal1
      //    +- D3wal1
      //    +- D3wal2
      //    +- D4mag1
      //    +- D4mag2
      //    +- D4mag3
      //
      // +- E1spc1
      //    +- E2wal1
      //    +- E3wal1
      //    +- E4mag1
      //    +- E4mag2
      //    +- E4mag3
      //
      // +- F1wal1
      //    +- F2spc1
      //       +- F3wal1
      //          +- F4spc1
      //             +- F5wal1
      //                +- F6spc1
      //                   +- F7lyr1
      //                   +- F7lyr2
      //                   +- F7lyr3
      //                   +- F7lyr4
      //                   +- F7lyr5
      //       +- F3wal2
      //       +- F3wal3
      //       +- F3wal4
      // +- F1wal2

      double stepMax = 5.0 * Unit::mm;
      bool flag_limitStep = false;
      //bool flag_limitStep = true;

      const double unitFactor = 1 / Unit::mm;

      map<string, CryostatElement> elements;

      GearDir her("/Detector/SuperKEKB/HER/");
      GearDir ler("/Detector/SuperKEKB/LER/");
      double crossingAngleHER = her.getDouble("angle");
      double crossingAngleLER = ler.getDouble("angle");

      G4Transform3D transform_HER = G4Translate3D(0., 0., 0.);
      transform_HER = transform_HER * G4RotateY3D(crossingAngleHER);

      G4Transform3D transform_LER = G4Translate3D(0., 0., 0.);
      transform_LER = transform_LER * G4RotateY3D(crossingAngleLER);

      //--------------
      //-   Bounding shapes

      // right bounding shape 1
      CryostatElement tubeR;
      GearDir cTubeR(content, "TubeR/");
      const int TubeR_N = cTubeR.getInt("N");

      double TubeR_Z[TubeR_N];
      double TubeR_R[TubeR_N];
      double TubeR_r[TubeR_N];

      for (int i = 0; i < TubeR_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        TubeR_Z[i] = cTubeR.getLength(ossZID.str()) * unitFactor;
        TubeR_R[i] = cTubeR.getLength(ossRID.str()) * unitFactor;
        TubeR_r[i] = cTubeR.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      tubeR.transform = G4Translate3D(0., 0., 0.);
      tubeR.geo = new G4Polycone("geo_TubeR_name", 0, 2 * M_PI, TubeR_N, TubeR_Z, TubeR_r, TubeR_R);
      tubeR.logi = NULL;
      elements["TubeR"] = tubeR;

      // right bounding shape 2
      CryostatElement tubeR2;
      GearDir cTubeR2(content, "TubeR2/");
      const int TubeR2_N = cTubeR2.getInt("N");

      double TubeR2_Z[TubeR2_N];
      double TubeR2_R[TubeR2_N];
      double TubeR2_r[TubeR2_N];

      for (int i = 0; i < TubeR2_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        TubeR2_Z[i] = cTubeR2.getLength(ossZID.str()) * unitFactor;
        TubeR2_R[i] = cTubeR2.getLength(ossRID.str()) * unitFactor;
        TubeR2_r[i] = cTubeR2.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      tubeR2.transform = G4Translate3D(0., 0., 0.);
      tubeR2.geo = new G4Polycone("geo_TubeR2_name", 0, 2 * M_PI, TubeR2_N, TubeR2_Z, TubeR2_r, TubeR2_R);
      tubeR2.logi = NULL;
      elements["TubeR2"] = tubeR2;

      // left bounding shape
      CryostatElement tubeL;
      GearDir cTubeL(content, "TubeL/");
      const int TubeL_N = cTubeL.getInt("N");

      double TubeL_Z[TubeL_N];
      double TubeL_R[TubeL_N];
      double TubeL_r[TubeL_N];

      for (int i = 0; i < TubeL_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        TubeL_Z[i] = cTubeL.getLength(ossZID.str()) * unitFactor;
        TubeL_R[i] = cTubeL.getLength(ossRID.str()) * unitFactor;
        TubeL_r[i] = cTubeL.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      tubeL.transform = G4Translate3D(0., 0., 0.);
      tubeL.geo = new G4Polycone("geo_TubeL_name", 0, 2 * M_PI, TubeL_N, TubeL_Z, TubeL_r, TubeL_R);
      tubeL.logi = NULL;
      elements["TubeL"] = tubeL;

      //--------------
      // Special cases with complex geometry

      //--------------
      //-   A1spc1 and B1spc1

      // space containing all structures around right HER beam pipe, part 1
      CryostatElement A1spc1;
      GearDir cA1spc1(content, "A1spc1/");
      const int A1spc1_N = cA1spc1.getInt("N");

      double A1spc1_Z[A1spc1_N];
      double A1spc1_r[A1spc1_N];
      double A1spc1_R[A1spc1_N];

      for (int i = 0; i < A1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A1spc1_Z[i] = cA1spc1.getLength(ossZID.str()) * unitFactor;
        A1spc1_R[i] = cA1spc1.getLength(ossRID.str()) * unitFactor;
        A1spc1_r[i] = cA1spc1.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      A1spc1.transform = transform_HER;
      G4Polycone* geo_A1spc1xx = new G4Polycone("geo_A1spc1xx_name", 0, 2 * M_PI, A1spc1_N, A1spc1_Z, A1spc1_r, A1spc1_R);

      // space containing all structures around right HER beam pipe, part 2
      CryostatElement A1spc2;
      GearDir cA1spc2(content, "A1spc2/");
      const int A1spc2_N = cA1spc2.getInt("N");

      double A1spc2_Z[A1spc2_N];
      double A1spc2_R[A1spc2_N];
      double A1spc2_r[A1spc2_N];

      for (int i = 0; i < A1spc2_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A1spc2_Z[i] = cA1spc2.getLength(ossZID.str()) * unitFactor;
        A1spc2_R[i] = cA1spc2.getLength(ossRID.str()) * unitFactor;
        A1spc2_r[i] = cA1spc2.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      A1spc2.transform = transform_HER;
      G4Polycone* geo_A1spc2xx = new G4Polycone("geo_A1spc2xx_name", 0, 2 * M_PI, A1spc2_N, A1spc2_Z, A1spc2_r, A1spc2_R);

      // space containing all structures around right LER beam pipe, part 1
      CryostatElement B1spc1;
      GearDir cB1spc1(content, "B1spc1/");
      const int B1spc1_N = cB1spc1.getInt("N");

      double B1spc1_Z[B1spc1_N];
      double B1spc1_R[B1spc1_N];
      double B1spc1_r[B1spc1_N];

      for (int i = 0; i < B1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B1spc1_Z[i] = cB1spc1.getLength(ossZID.str()) * unitFactor;
        B1spc1_R[i] = cB1spc1.getLength(ossRID.str()) * unitFactor;
        B1spc1_r[i] = cB1spc1.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      B1spc1.transform = transform_LER;
      G4Polycone* geo_B1spc1xx = new G4Polycone("geo_B1spc1xx_name", 0, 2 * M_PI, B1spc1_N, B1spc1_Z, B1spc1_r, B1spc1_R);

      // space containing all structures around right LER beam pipe, part 2
      CryostatElement B1spc2;
      GearDir cB1spc2(content, "B1spc2/");
      const int B1spc2_N = cB1spc2.getInt("N");

      double B1spc2_Z[B1spc2_N];
      double B1spc2_R[B1spc2_N];
      double B1spc2_r[B1spc2_N];

      for (int i = 0; i < B1spc2_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B1spc2_Z[i] = cB1spc2.getLength(ossZID.str()) * unitFactor;
        B1spc2_R[i] = cB1spc2.getLength(ossRID.str()) * unitFactor;
        B1spc2_r[i] = cB1spc2.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      B1spc2.transform = transform_LER;
      G4Polycone* geo_B1spc2xx = new G4Polycone("geo_B1spc2xx_name", 0, 2 * M_PI, B1spc2_N, B1spc2_Z, B1spc2_r, B1spc2_R);

      // final cut
      B1spc2.geo = new G4IntersectionSolid("geo_B1spc2_name", geo_B1spc2xx, elements["TubeR2"].geo, B1spc2.transform.inverse());
      B1spc2.logi = NULL;

      G4IntersectionSolid* geo_B1spc1x = new G4IntersectionSolid("geo_B1spc1x_name", geo_B1spc1xx, elements["TubeR"].geo, B1spc1.transform.inverse());
      B1spc1.geo = new G4UnionSolid("geo_B1spc1_name", geo_B1spc1x, B1spc2.geo);

      A1spc2.geo = new G4IntersectionSolid("geo_A1spc2_name", geo_A1spc2xx, elements["TubeR2"].geo, A1spc2.transform.inverse());
      A1spc2.logi = NULL;

      G4IntersectionSolid* geo_A1spc1xy = new G4IntersectionSolid("geo_A1spc1xy_name", geo_A1spc1xx, elements["TubeR"].geo, A1spc1.transform.inverse());
      G4UnionSolid* geo_A1spc1x = new G4UnionSolid("geo_A1spc1x_name", geo_A1spc1xy, A1spc2.geo);
      A1spc1.geo = new G4SubtractionSolid("geo_A1spc1_name", geo_A1spc1x, B1spc1.geo, A1spc1.transform.inverse()*B1spc1.transform);

      string strMat_A1spc1 = cA1spc1.getString("Material");
      G4Material* mat_A1spc1 = Materials::get(strMat_A1spc1);
      A1spc1.logi =  new G4LogicalVolume(A1spc1.geo, mat_A1spc1, "logi_A1spc1_name");
      if (flag_limitStep)
        A1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*A1spc1.logi, cA1spc1.getString("Color", "#CC0000"));
      //setVisibility(*A1spc1.logi, false);
      new G4PVPlacement(A1spc1.transform, A1spc1.logi, "phys_A1spc1_name", &topVolume, false, 0);

      string strMat_B1spc1 = cB1spc1.getString("Material");
      G4Material* mat_B1spc1 = Materials::get(strMat_B1spc1);
      B1spc1.logi =  new G4LogicalVolume(B1spc1.geo, mat_B1spc1, "logi_B1spc1_name");
      if (flag_limitStep)
        B1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*B1spc1.logi, cB1spc1.getString("Color", "#CC0000"));
      //setVisibility(*B1spc1.logi, false);
      new G4PVPlacement(B1spc1.transform, B1spc1.logi, "phys_B1spc1_name", &topVolume, false, 0);

      elements["A1spc1"] = A1spc1;
      elements["A1spc2"] = A1spc2;
      elements["B1spc1"] = B1spc1;
      elements["B1spc2"] = B1spc2;

      //--------------
      //-   C1wal1

      //get parameters from .xml file
      CryostatElement C1wal1;
      GearDir cC1wal1(content, "C1wal1/");
      const int C1wal1_N = cC1wal1.getInt("N");

      double C1wal1_Z[C1wal1_N];
      double C1wal1_R[C1wal1_N];
      double C1wal1_r[C1wal1_N];

      for (int i = 0; i < C1wal1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C1wal1_Z[i] = cC1wal1.getLength(ossZID.str()) * unitFactor;
        C1wal1_R[i] = cC1wal1.getLength(ossRID.str()) * unitFactor;
        C1wal1_r[i] = cC1wal1.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      C1wal1.transform = G4Translate3D(0., 0., 0.);

      //define geometry
      G4Polycone* geo_C1wal1xxx = new G4Polycone("geo_C1wal1xxx_name", 0, 2 * M_PI, C1wal1_N, C1wal1_Z, C1wal1_r, C1wal1_R);
      G4IntersectionSolid* geo_C1wal1xx = new G4IntersectionSolid("geo_C1wal1xx_name", geo_C1wal1xxx, elements["TubeR"].geo, elements["TubeR"].transform);
      G4SubtractionSolid* geo_C1wal1x = new G4SubtractionSolid("geo_C1wal1x_name", geo_C1wal1xx, elements["A1spc1"].geo, elements["A1spc1"].transform);
      C1wal1.geo = new G4SubtractionSolid("geo_C1wal1_name", geo_C1wal1x, elements["B1spc1"].geo, elements["B1spc1"].transform);

      string strMat_C1wal1 = cC1wal1.getString("Material");
      G4Material* mat_C1wal1 = Materials::get(strMat_C1wal1);
      C1wal1.logi =  new G4LogicalVolume(C1wal1.geo, mat_C1wal1, "logi_C1wal1_name");

      //put volume
      setColor(*C1wal1.logi, cC1wal1.getString("Color", "#CC0000"));
      setVisibility(*C1wal1.logi, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), C1wal1.logi, "phys_C1wal1_name", &topVolume, false, 0);

      elements["C1wal1"] = C1wal1;

      //--------------
      //-   D1spc1 and E1spc1

      // space containing all structures around left HER beam pipe
      CryostatElement D1spc1;
      GearDir cD1spc1(content, "D1spc1/");
      const int D1spc1_N = cD1spc1.getInt("N");

      double D1spc1_Z[D1spc1_N];
      double D1spc1_r[D1spc1_N];
      double D1spc1_R[D1spc1_N];

      for (int i = 0; i < D1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D1spc1_Z[i] = cD1spc1.getLength(ossZID.str()) * unitFactor;
        D1spc1_R[i] = cD1spc1.getLength(ossRID.str()) * unitFactor;
        D1spc1_r[i] = cD1spc1.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      D1spc1.transform = transform_HER;
      G4Polycone* geo_D1spc1xx = new G4Polycone("geo_D1spc1xx_name", 0, 2 * M_PI, D1spc1_N, D1spc1_Z, D1spc1_r, D1spc1_R);

      // space containing all structures around left LER beam pipe
      CryostatElement E1spc1;
      GearDir cE1spc1(content, "E1spc1/");
      const int E1spc1_N = cE1spc1.getInt("N");

      double E1spc1_Z[E1spc1_N];
      double E1spc1_R[E1spc1_N];
      double E1spc1_r[E1spc1_N];

      for (int i = 0; i < E1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E1spc1_Z[i] = cE1spc1.getLength(ossZID.str()) * unitFactor;
        E1spc1_R[i] = cE1spc1.getLength(ossRID.str()) * unitFactor;
        E1spc1_r[i] = cE1spc1.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      E1spc1.transform = transform_LER;
      G4Polycone* geo_E1spc1xx = new G4Polycone("geo_E1spc1xx_name", 0, 2 * M_PI, E1spc1_N, E1spc1_Z, E1spc1_r, E1spc1_R);

      // final cut
      G4IntersectionSolid* geo_D1spc1x = new G4IntersectionSolid("geo_D1spc1x_name", geo_D1spc1xx, elements["TubeL"].geo, D1spc1.transform.inverse());
      E1spc1.geo = new G4IntersectionSolid("geo_E1spc1_name", geo_E1spc1xx, elements["TubeL"].geo, E1spc1.transform.inverse());
      D1spc1.geo = new G4SubtractionSolid("geo_D1spc1_name", geo_D1spc1x, E1spc1.geo, D1spc1.transform.inverse()*E1spc1.transform);

      string strMat_D1spc1 = cD1spc1.getString("Material");
      G4Material* mat_D1spc1 = Materials::get(strMat_D1spc1);
      D1spc1.logi =  new G4LogicalVolume(D1spc1.geo, mat_D1spc1, "logi_D1spc1_name");
      if (flag_limitStep)
        D1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*D1spc1.logi, cD1spc1.getString("Color", "#CC0000"));
      //setVisibility(*D1spc1.logi, false);
      new G4PVPlacement(D1spc1.transform, D1spc1.logi, "phys_D1spc1_name", &topVolume, false, 0);

      string strMat_E1spc1 = cE1spc1.getString("Material");
      G4Material* mat_E1spc1 = Materials::get(strMat_E1spc1);
      E1spc1.logi =  new G4LogicalVolume(E1spc1.geo, mat_E1spc1, "logi_E1spc1_name");
      if (flag_limitStep)
        E1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*E1spc1.logi, cE1spc1.getString("Color", "#CC0000"));
      //setVisibility(*E1spc1.logi, false);
      new G4PVPlacement(E1spc1.transform, E1spc1.logi, "phys_E1spc1_name", &topVolume, false, 0);

      elements["E1spc1"] = E1spc1;
      elements["D1spc1"] = D1spc1;


      //--------------
      //-   F1wal1

      //get parameters from .xml file
      CryostatElement F1wal1;
      GearDir cF1wal1(content, "F1wal1/");
      const int F1wal1_N = cF1wal1.getInt("N");

      double F1wal1_Z[F1wal1_N];
      double F1wal1_R[F1wal1_N];
      double F1wal1_r[F1wal1_N];

      for (int i = 0; i < F1wal1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        F1wal1_Z[i] = cF1wal1.getLength(ossZID.str()) * unitFactor;
        F1wal1_R[i] = cF1wal1.getLength(ossRID.str()) * unitFactor;
        F1wal1_r[i] = cF1wal1.getLength(ossrID.str(), 0.0) * unitFactor;
      }

      F1wal1.transform = G4Translate3D(0., 0., 0.);

      //define geometry
      G4Polycone* geo_F1wal1xxx = new G4Polycone("geo_F1wal1xxx_name", 0, 2 * M_PI, F1wal1_N, F1wal1_Z, F1wal1_r, F1wal1_R);
      G4IntersectionSolid* geo_F1wal1xx = new G4IntersectionSolid("geo_F1wal1xx_name", geo_F1wal1xxx, elements["TubeL"].geo, elements["TubeL"].transform);
      G4SubtractionSolid* geo_F1wal1x = new G4SubtractionSolid("geo_F1wal1x_name", geo_F1wal1xx, elements["D1spc1"].geo, elements["D1spc1"].transform);
      F1wal1.geo = new G4SubtractionSolid("geo_F1wal1_name", geo_F1wal1x, elements["E1spc1"].geo, elements["E1spc1"].transform);

      string strMat_F1wal1 = cF1wal1.getString("Material");
      G4Material* mat_F1wal1 = Materials::get(strMat_F1wal1);
      F1wal1.logi =  new G4LogicalVolume(F1wal1.geo, mat_F1wal1, "logi_F1wal1_name");

      //put volume
      setColor(*F1wal1.logi, cF1wal1.getString("Color", "#CC0000"));
      setVisibility(*F1wal1.logi, false);
      new G4PVPlacement(F1wal1.transform, F1wal1.logi, "phys_F1wal1_name", &topVolume, false, 0);

      elements["F1wal1"] = F1wal1;


      //--------------
      //-   Rest of elements with typical geometry

      BOOST_FOREACH(const GearDir & cPolycone, content.getNodes("Straight")) {

        CryostatElement polycone;

        string name = cPolycone.getString("@name");

        int N = cPolycone.getInt("N");

        double Z[N];
        double R[N];
        double r[N];

        for (int i = 0; i < N; ++i) {
          ostringstream ossZID;
          ossZID << "Z" << i;

          ostringstream ossRID;
          ossRID << "R" << i;

          ostringstream ossrID;
          ossrID << "r" << i;

          Z[i] = cPolycone.getLength(ossZID.str()) * unitFactor;
          R[i] = cPolycone.getLength(ossRID.str()) * unitFactor;
          r[i] = cPolycone.getLength(ossrID.str(), 0.0) * unitFactor;
        }

        polycone.transform = G4Translate3D(0.0, 0.0, 0.0);

        //define geometry
        string motherVolume = cPolycone.getString("MotherVolume");
        string subtract = cPolycone.getString("Subtract", "");
        string intersect = cPolycone.getString("Intersect", "");

        string geo_polyconexx_name = "geo_" + name + "xx_name";
        string geo_polyconex_name = "geo_" + name + "x_name";
        string geo_polycone_name = "geo_" + name + "_name";

        G4VSolid* geo_polyconexx, *geo_polyconex, *geo_polycone;

        if (subtract != "" && intersect != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, Z, r, R);
          geo_polyconex = new G4SubtractionSolid(geo_polyconex_name, geo_polyconexx, elements[subtract].geo, elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[subtract].transform);
          geo_polycone = new G4IntersectionSolid(geo_polycone_name, geo_polyconex, elements[intersect].geo, elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[intersect].transform);
        } else if (subtract != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, Z, r, R);
          geo_polycone = new G4SubtractionSolid(geo_polycone_name, geo_polyconexx, elements[subtract].geo, elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[subtract].transform);
        } else if (intersect != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, Z, r, R);
          geo_polycone = new G4IntersectionSolid(geo_polycone_name, geo_polyconexx, elements[intersect].geo, elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[intersect].transform);
        } else
          geo_polycone = new G4Polycone(geo_polycone_name, 0.0, 2 * M_PI, N, Z, r, R);

        polycone.geo = geo_polycone;

        // define logical volume
        string strMat_polycone = cPolycone.getString("Material");
        G4Material* mat_polycone = Materials::get(strMat_polycone);
        string logi_polycone_name = "logi_" + name + "_name";
        polycone.logi =  new G4LogicalVolume(polycone.geo, mat_polycone, logi_polycone_name);
        setColor(*polycone.logi, cPolycone.getString("Color", "#CC0000"));
        setVisibility(*polycone.logi, cPolycone.getBool("Visibility", true));

        //put volume
        string phys_polycone_name = "phys_" + name + "_name";
        new G4PVPlacement(polycone.transform, polycone.logi, phys_polycone_name, elements[motherVolume].logi, false, 0);

        //to use it later in "intersect" and "subtract"
        polycone.transform = polycone.transform * elements[motherVolume].transform;

        elements[name] = polycone;
      }


      //---------------------------
      // for dose simulation
      //---------------------------

      //logi_C1wal1->SetSensitiveDetector(m_sensitive);
      /*
      logi_A1spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1));
      logi_B1spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2));
      logi_A2wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 3));
      logi_A3wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 4));
      logi_A3wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5));
      logi_A4mag1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 6));
      logi_A4mag2p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 7));
      logi_A4mag2p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 8));
      logi_A4mag2p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 9));
      logi_A4mag2p4  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 10));
      logi_A4mag3p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 11));
      logi_A4mag3p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 12));
      logi_A4mag4p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 13));
      logi_A4mag4p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 14));
      logi_A4mag4p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 15));
      logi_A4mag4p4  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 16));
      logi_A4mag4p5  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 17));
      logi_A4mag4p6  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 18));
      logi_A4mag4p7  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 19));
      logi_A4mag4p8  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 20));
      logi_A4mag4p9  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 21));
      logi_B2wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 22));
      logi_B3wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 23));
      logi_B3wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 24));
      logi_B4mag1p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 25));
      logi_B4mag1p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 26));
      logi_B4mag1p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 27));
      logi_B4mag2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 28));
      logi_B4mag3p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 29));
      logi_B4mag3p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 30));
      logi_B4mag3p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 31));
      logi_B4mag3p4  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 32));
      logi_B4mag3p5  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 33));
      logi_B4mag3p6  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 34));
      logi_B4mag4p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 35));
      logi_B4mag4p7  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 36));
      logi_B4mag4p8  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 37));
      logi_B4mag4p9  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 38));
      logi_C1wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 39));
      logi_C1wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 40));
      logi_C2spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 41));
      logi_C2spc2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 42));
      logi_C3wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 43));
      logi_C3wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 44));
      logi_C3wal3    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 45));
      logi_C3wal4    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 46));
      logi_C4spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 47));
      logi_C4spc2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 48));
      logi_C5wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 49));
      logi_C5wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 50));
      logi_C6spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 51));
      logi_C6spc2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 52));
      logi_C7wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 53));
      logi_C7lyr1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 54));
      logi_C7lyr2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 55));
      logi_C7lyr3    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 56));
      logi_C7lyr4    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 57));
      logi_C7lyr5    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 58));
      logi_D1spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 59));
      logi_E1spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 60));
      logi_D2wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 61));
      logi_D3wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 62));
      logi_D3wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 63));
      logi_D4mag1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 64));
      logi_D4mag2p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 65));
      logi_D4mag2p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 66));
      logi_D4mag2p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 67));
      logi_D4mag2p4  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 68));
      logi_D4mag3p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 69));
      logi_D4mag3p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 70));
      logi_D4mag3p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 71));
      logi_D4mag3p4  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 72));
      logi_D4mag3p5  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 73));
      logi_D4mag3p6  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 74));
      logi_E2wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 75));
      logi_E3wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 76));
      logi_E4mag1p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 77));
      logi_E4mag1p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 78));
      logi_E4mag1p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 79));
      logi_E4mag2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 80));
      logi_E4mag3p1  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 81));
      logi_E4mag3p2  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 82));
      logi_E4mag3p3  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 83));
      logi_E4mag3p4  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 84));
      logi_E4mag3p5  ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 85));
      logi_F1wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 86));
      logi_F2spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 87));
      logi_F3wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 88));
      logi_F3wal2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 89));
      logi_F3wal3    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 90));
      logi_F4spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 91));
      logi_F5wal1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 92));
      logi_F6spc1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 93));
      logi_F7lyr1    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 94));
      logi_F7lyr2    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 95));
      logi_F7lyr3    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 96));
      logi_F7lyr4    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 97));
      logi_F7lyr5    ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 98));
      */

    }
  }
}
