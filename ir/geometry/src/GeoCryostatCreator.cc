/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ir/geometry/GeoCryostatCreator.h>
#include <ir/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

#include <simulation/background/BkgSensitiveDetector.h>

//Shapes
#include <G4Box.hh>
#include <G4Trd.hh>
#include <G4Tubs.hh>
#include <G4Cons.hh>
#include <G4Polycone.hh>
#include <G4EllipticalTube.hh>
#include <G4UnionSolid.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4ExtrudedSolid.hh>
#include <G4UserLimits.hh>

using namespace std;

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

    void GeoCryostatCreator::createGeometry(G4LogicalVolume& topVolume, GeometryTypes)
    {

      //######  R side index  ######
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
      //             +- C5wal4
      //       +- C3wal3
      //    +- C2spc3
      // +- C1wal2
      //
      //######  L side index  ######
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
      int flag_limitStep = int(m_config.getParameter("LimitStepLength"));

      const double unitFactor = Unit::cm / Unit::mm;

      double crossingAngleHER = m_config.getParameter("CrossingAngle.HER", 0.0415);
      double crossingAngleLER = m_config.getParameter("CrossingAngle.LER", -0.0415);

      G4Transform3D transform_HER = G4Translate3D(0., 0., 0.);
      transform_HER = transform_HER * G4RotateY3D(crossingAngleHER);

      G4Transform3D transform_LER = G4Translate3D(0., 0., 0.);
      transform_LER = transform_LER * G4RotateY3D(crossingAngleLER);

      map<string, CryostatElement> elements;

      //--------------
      //-   Bounding shapes

      // right bounding shape 1
      CryostatElement tubeR;
      std::string prep = "TubeR.";

      const int TubeR_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> TubeR_Z(TubeR_N);
      std::vector<double> TubeR_R(TubeR_N);
      std::vector<double> TubeR_r(TubeR_N);

      for (int i = 0; i < TubeR_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        TubeR_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        TubeR_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        TubeR_r[i] = m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      tubeR.transform = G4Translate3D(0., 0., 0.);
      tubeR.geo = new G4Polycone("geo_TubeR_name", 0, 2 * M_PI, TubeR_N, &(TubeR_Z[0]), &(TubeR_r[0]), &(TubeR_R[0]));
      tubeR.logi = NULL;
      elements["TubeR"] = tubeR;

      // right bounding shape 2
      CryostatElement tubeR2;
      prep = "TubeR2.";
      const int TubeR2_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> TubeR2_Z(TubeR2_N);
      std::vector<double> TubeR2_R(TubeR2_N);
      std::vector<double> TubeR2_r(TubeR2_N);

      for (int i = 0; i < TubeR2_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        TubeR2_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        TubeR2_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        TubeR2_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      tubeR2.transform = G4Translate3D(0., 0., 0.);
      tubeR2.geo = new G4Polycone("geo_TubeR2_name", 0, 2 * M_PI, TubeR2_N, &(TubeR2_Z[0]), &(TubeR2_r[0]), &(TubeR2_R[0]));
      tubeR2.logi = NULL;
      elements["TubeR2"] = tubeR2;

      // left bounding shape
      CryostatElement tubeL;
      prep = "TubeL.";
      const int TubeL_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> TubeL_Z(TubeL_N);
      std::vector<double> TubeL_R(TubeL_N);
      std::vector<double> TubeL_r(TubeL_N);

      for (int i = 0; i < TubeL_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        TubeL_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        TubeL_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        TubeL_r[i] = m_config.getParameter(prep + ossrID.str()) * unitFactor;
      }

      tubeL.transform = G4Translate3D(0., 0., 0.);
      tubeL.geo = new G4Polycone("geo_TubeL_name", 0, 2 * M_PI, TubeL_N, &(TubeL_Z[0]), &(TubeL_r[0]), &(TubeL_R[0]));
      tubeL.logi = NULL;
      elements["TubeL"] = tubeL;

      //--------------
      // Special cases with complex geometry

      //--------------
      //-   A1spc1 and B1spc1

      // space containing all structures around right HER beam pipe, part 1
      CryostatElement A1spc1;
      prep = "A1spc1.";
      const int A1spc1_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> A1spc1_Z(A1spc1_N);
      std::vector<double> A1spc1_r(A1spc1_N);
      std::vector<double> A1spc1_R(A1spc1_N);

      for (int i = 0; i < A1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A1spc1_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        A1spc1_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        A1spc1_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      A1spc1.transform = transform_HER;
      G4Polycone* geo_A1spc1xx = new G4Polycone("geo_A1spc1xx_name", 0, 2 * M_PI, A1spc1_N, &(A1spc1_Z[0]), &(A1spc1_r[0]),
                                                &(A1spc1_R[0]));

      // space containing all structures around right HER beam pipe, part 2
      CryostatElement A1spc2;
      prep  = "A1spc2.";
      const int A1spc2_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> A1spc2_Z(A1spc2_N);
      std::vector<double> A1spc2_R(A1spc2_N);
      std::vector<double> A1spc2_r(A1spc2_N);

      for (int i = 0; i < A1spc2_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        A1spc2_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        A1spc2_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        A1spc2_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      A1spc2.transform = transform_HER;
      G4Polycone* geo_A1spc2xx = new G4Polycone("geo_A1spc2xx_name", 0, 2 * M_PI, A1spc2_N, &(A1spc2_Z[0]), &(A1spc2_r[0]),
                                                &(A1spc2_R[0]));

      // space containing all structures around right LER beam pipe, part 1
      CryostatElement B1spc1;
      prep = "B1spc1.";
      const int B1spc1_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> B1spc1_Z(B1spc1_N);
      std::vector<double> B1spc1_R(B1spc1_N);
      std::vector<double> B1spc1_r(B1spc1_N);

      for (int i = 0; i < B1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B1spc1_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        B1spc1_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        B1spc1_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      B1spc1.transform = transform_LER;
      G4Polycone* geo_B1spc1xx = new G4Polycone("geo_B1spc1xx_name", 0, 2 * M_PI, B1spc1_N, &(B1spc1_Z[0]), &(B1spc1_r[0]),
                                                &(B1spc1_R[0]));

      // space containing all structures around right LER beam pipe, part 2
      CryostatElement B1spc2;
      prep = "B1spc2.";
      const int B1spc2_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> B1spc2_Z(B1spc2_N);
      std::vector<double> B1spc2_R(B1spc2_N);
      std::vector<double> B1spc2_r(B1spc2_N);

      for (int i = 0; i < B1spc2_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        B1spc2_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        B1spc2_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        B1spc2_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      B1spc2.transform = transform_LER;
      G4Polycone* geo_B1spc2xx = new G4Polycone("geo_B1spc2xx_name", 0, 2 * M_PI, B1spc2_N, &(B1spc2_Z[0]), &(B1spc2_r[0]),
                                                &(B1spc2_R[0]));

      // final cut
      B1spc2.geo = new G4IntersectionSolid("geo_B1spc2_name", geo_B1spc2xx, elements["TubeR2"].geo, B1spc2.transform.inverse());
      B1spc2.logi = NULL;

      G4IntersectionSolid* geo_B1spc1x = new G4IntersectionSolid("geo_B1spc1x_name", geo_B1spc1xx, elements["TubeR"].geo,
                                                                 B1spc1.transform.inverse());
      B1spc1.geo = new G4UnionSolid("geo_B1spc1_name", geo_B1spc1x, B1spc2.geo);

      A1spc2.geo = new G4IntersectionSolid("geo_A1spc2_name", geo_A1spc2xx, elements["TubeR2"].geo, A1spc2.transform.inverse());
      A1spc2.logi = NULL;

      G4IntersectionSolid* geo_A1spc1xy = new G4IntersectionSolid("geo_A1spc1xy_name", geo_A1spc1xx, elements["TubeR"].geo,
                                                                  A1spc1.transform.inverse());
      G4UnionSolid* geo_A1spc1x = new G4UnionSolid("geo_A1spc1x_name", geo_A1spc1xy, A1spc2.geo);
      A1spc1.geo = new G4SubtractionSolid("geo_A1spc1_name", geo_A1spc1x, B1spc1.geo, A1spc1.transform.inverse()*B1spc1.transform);

      string strMat_A1spc1 = m_config.getParameterStr("A1spc1.Material");
      G4Material* mat_A1spc1 = Materials::get(strMat_A1spc1);
      A1spc1.logi =  new G4LogicalVolume(A1spc1.geo, mat_A1spc1, "logi_A1spc1_name");
      if (flag_limitStep)
        A1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*A1spc1.logi, "#CC0000");
      //setVisibility(*A1spc1.logi, false);
      new G4PVPlacement(A1spc1.transform, A1spc1.logi, "phys_A1spc1_name", &topVolume, false, 0);

      string strMat_B1spc1 = m_config.getParameterStr("B1spc1.Material");
      G4Material* mat_B1spc1 = Materials::get(strMat_B1spc1);
      B1spc1.logi =  new G4LogicalVolume(B1spc1.geo, mat_B1spc1, "logi_B1spc1_name");
      if (flag_limitStep)
        B1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*B1spc1.logi, "#CC0000");
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
      prep =  "C1wal1.";
      const int C1wal1_N = m_config.getParameter(prep + "N");

      std::vector<double> C1wal1_Z(C1wal1_N);
      std::vector<double> C1wal1_R(C1wal1_N);
      std::vector<double> C1wal1_r(C1wal1_N);

      for (int i = 0; i < C1wal1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        C1wal1_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        C1wal1_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        C1wal1_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      C1wal1.transform = G4Translate3D(0., 0., 0.);

      //define geometry
      G4Polycone* geo_C1wal1xxx = new G4Polycone("geo_C1wal1xxx_name", 0, 2 * M_PI, C1wal1_N, &(C1wal1_Z[0]), &(C1wal1_r[0]),
                                                 &(C1wal1_R[0]));
      G4IntersectionSolid* geo_C1wal1xx = new G4IntersectionSolid("geo_C1wal1xx_name", geo_C1wal1xxx, elements["TubeR"].geo,
                                                                  elements["TubeR"].transform);
      G4SubtractionSolid* geo_C1wal1x = new G4SubtractionSolid("geo_C1wal1x_name", geo_C1wal1xx, elements["A1spc1"].geo,
                                                               elements["A1spc1"].transform);
      C1wal1.geo = new G4SubtractionSolid("geo_C1wal1_name", geo_C1wal1x, elements["B1spc1"].geo, elements["B1spc1"].transform);

      string strMat_C1wal1 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_C1wal1 = Materials::get(strMat_C1wal1);
      C1wal1.logi =  new G4LogicalVolume(C1wal1.geo, mat_C1wal1, "logi_C1wal1_name");

      //put volume
      setColor(*C1wal1.logi, "#CC0000");
//--andrii      setVisibility(*C1wal1.logi, false);
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), C1wal1.logi, "phys_C1wal1_name", &topVolume, false, 0);

      elements["C1wal1"] = C1wal1;

      //--------------
      //-   D1spc1 and E1spc1

      // space containing all structures around left HER beam pipe
      CryostatElement D1spc1;
      prep = "D1spc1.";
      const int D1spc1_N = m_config.getParameter(prep + "N");

      std::vector<double> D1spc1_Z(D1spc1_N);
      std::vector<double> D1spc1_r(D1spc1_N);
      std::vector<double> D1spc1_R(D1spc1_N);

      for (int i = 0; i < D1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        D1spc1_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        D1spc1_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        D1spc1_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      D1spc1.transform = transform_HER;
      G4Polycone* geo_D1spc1xx = new G4Polycone("geo_D1spc1xx_name", 0, 2 * M_PI, D1spc1_N, &(D1spc1_Z[0]), &(D1spc1_r[0]),
                                                &(D1spc1_R[0]));

      // space containing all structures around left LER beam pipe
      CryostatElement E1spc1;
      prep = "E1spc1.";
      const int E1spc1_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> E1spc1_Z(E1spc1_N);
      std::vector<double> E1spc1_R(E1spc1_N);
      std::vector<double> E1spc1_r(E1spc1_N);

      for (int i = 0; i < E1spc1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        E1spc1_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        E1spc1_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        E1spc1_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      E1spc1.transform = transform_LER;
      G4Polycone* geo_E1spc1xx = new G4Polycone("geo_E1spc1xx_name", 0, 2 * M_PI, E1spc1_N, &(E1spc1_Z[0]), &(E1spc1_r[0]),
                                                &(E1spc1_R[0]));

      // final cut
      G4IntersectionSolid* geo_D1spc1x = new G4IntersectionSolid("geo_D1spc1x_name", geo_D1spc1xx, elements["TubeL"].geo,
                                                                 D1spc1.transform.inverse());
      E1spc1.geo = new G4IntersectionSolid("geo_E1spc1_name", geo_E1spc1xx, elements["TubeL"].geo, E1spc1.transform.inverse());
      D1spc1.geo = new G4SubtractionSolid("geo_D1spc1_name", geo_D1spc1x, E1spc1.geo, D1spc1.transform.inverse()*E1spc1.transform);

      string strMat_D1spc1 = m_config.getParameterStr("D1spc1.Material");
      G4Material* mat_D1spc1 = Materials::get(strMat_D1spc1);
      D1spc1.logi =  new G4LogicalVolume(D1spc1.geo, mat_D1spc1, "logi_D1spc1_name");
      if (flag_limitStep)
        D1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*D1spc1.logi, "#CC0000");
      //setVisibility(*D1spc1.logi, false);
      new G4PVPlacement(D1spc1.transform, D1spc1.logi, "phys_D1spc1_name", &topVolume, false, 0);

      string strMat_E1spc1 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_E1spc1 = Materials::get(strMat_E1spc1);
      E1spc1.logi =  new G4LogicalVolume(E1spc1.geo, mat_E1spc1, "logi_E1spc1_name");
      if (flag_limitStep)
        E1spc1.logi->SetUserLimits(new G4UserLimits(stepMax));

      //put volume
      setColor(*E1spc1.logi, "#CC0000");
      //setVisibility(*E1spc1.logi, false);
      new G4PVPlacement(E1spc1.transform, E1spc1.logi, "phys_E1spc1_name", &topVolume, false, 0);

      elements["E1spc1"] = E1spc1;
      elements["D1spc1"] = D1spc1;


      //--------------
      //-   F1wal1

      //get parameters from .xml file
      CryostatElement F1wal1;
      prep = "F1wal1.";
      const int F1wal1_N = int(m_config.getParameter(prep + "N"));

      std::vector<double> F1wal1_Z(F1wal1_N);
      std::vector<double> F1wal1_R(F1wal1_N);
      std::vector<double> F1wal1_r(F1wal1_N);

      for (int i = 0; i < F1wal1_N; ++i) {
        ostringstream ossZID;
        ossZID << "Z" << i;

        ostringstream ossRID;
        ossRID << "R" << i;

        ostringstream ossrID;
        ossrID << "r" << i;

        F1wal1_Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
        F1wal1_R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
        F1wal1_r[i] =  m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      F1wal1.transform = G4Translate3D(0., 0., 0.);

      //define geometry
      G4Polycone* geo_F1wal1xxx = new G4Polycone("geo_F1wal1xxx_name", 0, 2 * M_PI, F1wal1_N, &(F1wal1_Z[0]), &(F1wal1_r[0]),
                                                 &(F1wal1_R[0]));
      G4IntersectionSolid* geo_F1wal1xx = new G4IntersectionSolid("geo_F1wal1xx_name", geo_F1wal1xxx, elements["TubeL"].geo,
                                                                  elements["TubeL"].transform);
      G4SubtractionSolid* geo_F1wal1x = new G4SubtractionSolid("geo_F1wal1x_name", geo_F1wal1xx, elements["D1spc1"].geo,
                                                               elements["D1spc1"].transform);
      F1wal1.geo = new G4SubtractionSolid("geo_F1wal1_name", geo_F1wal1x, elements["E1spc1"].geo, elements["E1spc1"].transform);

      string strMat_F1wal1 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_F1wal1 = Materials::get(strMat_F1wal1);
      F1wal1.logi =  new G4LogicalVolume(F1wal1.geo, mat_F1wal1, "logi_F1wal1_name");

      //put volume
      setColor(*F1wal1.logi, "#CC0000");
//--andrii      setVisibility(*F1wal1.logi, false);
      new G4PVPlacement(F1wal1.transform, F1wal1.logi, "phys_F1wal1_name", &topVolume, false, 0);

      elements["F1wal1"] = F1wal1;


      //--------------
      //-   Cryostats' supports

      std::vector<std::string> supports;
      boost::split(supports, m_config.getParameterStr("Support"), boost::is_any_of(" "));
      for (const auto& name : supports) {
        // storable element
        CryostatElement sup;
        prep = name + ".";

        double box_W = m_config.getParameter(prep + "W") * unitFactor;
        double box_H = m_config.getParameter(prep + "H") * unitFactor;
        double box_L = m_config.getParameter(prep + "L") * unitFactor;
        double box_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double box_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double box_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;

        double trd_W1 = m_config.getParameter(prep + "trdW1") * unitFactor;
        double trd_W2 = m_config.getParameter(prep + "trdW2") * unitFactor;
        double trd_L = m_config.getParameter(prep + "trdL") * unitFactor;
        double trd_H = m_config.getParameter(prep + "trdH") * unitFactor;
        double trd_X0 = m_config.getParameter(prep + "trdX0") * unitFactor;
        double trd_Y0 = m_config.getParameter(prep + "trdY0") * unitFactor;
        double trd_Z0 = m_config.getParameter(prep + "trdZ0") * unitFactor;

        double sup_PHI = m_config.getParameter(prep + "PHI");

        int sup_cut_N = int(m_config.getParameter(prep + "N", 0));

        // transformations
        G4Transform3D transform_box = G4Translate3D(box_X0, box_Y0, box_Z0);
        transform_box = transform_box * G4RotateY3D(sup_PHI / Unit::rad);

        G4Transform3D transform_trd = G4Translate3D(trd_X0, trd_Y0, trd_Z0);
        transform_trd = transform_trd * G4RotateY3D(sup_PHI / Unit::rad) * G4RotateX3D(M_PI / 2.0 / Unit::rad);

        //define geometry
        string geo_box_name = "geo_" + name + "_box_name";
        string geo_trd_name = "geo_" + name + "_trd_name";

        string geo_sup_name;
        if (sup_cut_N == 0) {
          geo_sup_name = "geo_" + name + "_name";
        } else {
          geo_sup_name = "geo_" + name + "_x_name";
        }

        G4Box* geo_box = new G4Box(geo_box_name, box_W / 2.0, box_H / 2.0, box_L / 2.0);
        G4Trd* geo_trd = new G4Trd(geo_trd_name, trd_W1 / 2.0, trd_W2 / 2.0, trd_L / 2.0, trd_L / 2.0, trd_H / 2.0);

        G4VSolid* geo_sup = new G4UnionSolid(geo_sup_name, geo_box, geo_trd, transform_box.inverse() * transform_trd);

        // cuts
        for (int i = 0; i < sup_cut_N; ++i) {
          ostringstream oss_block_num;
          oss_block_num << i;

          double cut_type = m_config.getParameter(prep + "cutType" + oss_block_num.str());
          string geo_supx_name;
          if (i == sup_cut_N - 1) {
            geo_supx_name = "geo_" + name + "_name";
          } else {
            geo_sup_name = "geo_" + name + "_x" + oss_block_num.str() + "_name";
          }
          string geo_cut_name = "geo_" + name + "_cut" + oss_block_num.str() + "_name";

          G4VSolid* geo_cut;
          //if(cut_type == "Box")
          if (cut_type == 0.0) {
            double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
            double cut_W = m_config.getParameter(prep + "cutW" + oss_block_num.str()) * unitFactor;
            double cut_H = m_config.getParameter(prep + "cutH" + oss_block_num.str()) * unitFactor;

            geo_cut = new G4Box(geo_cut_name, cut_W / 2.0, cut_H / 2.0, cut_L / 2.0);
            // else if(cut_type == "Tubs")
          } else { // if (cut_type != 0.0) {
            double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
            double cut_R = m_config.getParameter(prep + "cutR" + oss_block_num.str()) * unitFactor;

            geo_cut = new G4Tubs(geo_cut_name, 0.0, cut_R, cut_L / 2.0, 0.0, 2.0 * M_PI);
          }
          //} else
          //  continue;

          double cut_X0 = m_config.getParameter(prep + "cutDX" + oss_block_num.str()) * unitFactor;
          double cut_Y0 = m_config.getParameter(prep + "cutDY" + oss_block_num.str()) * unitFactor;
          double cut_Z0 = m_config.getParameter(prep + "cutDZ" + oss_block_num.str()) * unitFactor;
          double cut_PHI = m_config.getParameter(prep + "cutPHI" + oss_block_num.str(), 0.0);
          double cut_TH = m_config.getParameter(prep + "cutTH" + oss_block_num.str(), 0.0);

          G4Transform3D cut_transform = G4Translate3D(cut_X0, cut_Y0, cut_Z0);
          cut_transform = cut_transform * G4RotateX3D(cut_PHI / Unit::rad);
          cut_transform = cut_transform * G4RotateZ3D(cut_TH / Unit::rad);

          geo_sup = new G4SubtractionSolid(geo_sup_name, geo_sup, geo_cut,  cut_transform);
        }

        sup.geo = geo_sup;
        sup.transform = transform_box;

        string strMat_sup = m_config.getParameterStr(prep + "Material");
        G4Material* mat_sup = Materials::get(strMat_sup);

        string logi_sup_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_sup = new G4LogicalVolume(sup.geo, mat_sup, logi_sup_name);

        sup.logi = logi_sup;

        //put volume
        setColor(*logi_sup, "#0000CC");
        //setVisibility(*logi_sup, false);
        string phys_sup_name = "phys_" + name + "_name";
        new G4PVPlacement(sup.transform, sup.logi, phys_sup_name, &topVolume, false, 0);

        elements[name] = sup;
      }


      //--------------
      //-   Rest of elements with typical geometry

      std::vector<std::string> straightSections;
      boost::split(straightSections, m_config.getParameterStr("Straight"), boost::is_any_of(" "));
      for (const auto& name : straightSections) {
        prep = name + ".";

        CryostatElement polycone;

        int N = int(m_config.getParameter(prep + "N"));

        std::vector<double> Z(N);
        std::vector<double> R(N);
        std::vector<double> r(N);

        for (int i = 0; i < N; ++i) {
          ostringstream ossZID;
          ossZID << "Z" << i;

          ostringstream ossRID;
          ossRID << "R" << i;

          ostringstream ossrID;
          ossrID << "r" << i;

          Z[i] = m_config.getParameter(prep + ossZID.str()) * unitFactor;
          R[i] = m_config.getParameter(prep + ossRID.str()) * unitFactor;
          r[i] = m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
        }

        polycone.transform = G4Translate3D(0.0, 0.0, 0.0);

        //define geometry
        string motherVolume = m_config.getParameterStr(prep + "MotherVolume");
        string subtract = m_config.getParameterStr(prep + "Subtract", "");
        string intersect = m_config.getParameterStr(prep + "Intersect", "");

        string geo_polyconexx_name = "geo_" + name + "xx_name";
        string geo_polyconex_name = "geo_" + name + "x_name";
        string geo_polycone_name = "geo_" + name + "_name";

        G4VSolid* geo_polyconexx, *geo_polycone;

        if (subtract != "" && intersect != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, &(Z[0]), &(r[0]), &(R[0]));
          G4VSolid* geo_polyconex = new G4SubtractionSolid(geo_polyconex_name, geo_polyconexx, elements[subtract].geo,
                                                           elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[subtract].transform);
          geo_polycone = new G4IntersectionSolid(geo_polycone_name, geo_polyconex, elements[intersect].geo,
                                                 elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[intersect].transform);
        } else if (subtract != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, &(Z[0]), &(r[0]), &(R[0]));
          geo_polycone = new G4SubtractionSolid(geo_polycone_name, geo_polyconexx, elements[subtract].geo,
                                                elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[subtract].transform);
        } else if (intersect != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, &(Z[0]), &(r[0]), &(R[0]));
          geo_polycone = new G4IntersectionSolid(geo_polycone_name, geo_polyconexx, elements[intersect].geo,
                                                 elements[motherVolume].transform.inverse()*polycone.transform.inverse()*elements[intersect].transform);
        } else
          geo_polycone = new G4Polycone(geo_polycone_name, 0.0, 2 * M_PI, N, &(Z[0]), &(r[0]), &(R[0]));

        polycone.geo = geo_polycone;

        // define logical volume
        string strMat_polycone = m_config.getParameterStr(prep + "Material");
        G4Material* mat_polycone = Materials::get(strMat_polycone);
        string logi_polycone_name = "logi_" + name + "_name";
        polycone.logi =  new G4LogicalVolume(polycone.geo, mat_polycone, logi_polycone_name);
        setColor(*polycone.logi, "#CC0000");
        setVisibility(*polycone.logi, false);

        //put volume
        string phys_polycone_name = "phys_" + name + "_name";
        new G4PVPlacement(polycone.transform, polycone.logi, phys_polycone_name, elements[motherVolume].logi, false, 0);

        //to use it later in "intersect" and "subtract"
        polycone.transform = polycone.transform * elements[motherVolume].transform;

        elements[name] = polycone;
      }


      //--------------------------------------------------------------------------------------------
      //-   Shields

      std::vector<std::string> shields;
      boost::split(shields, m_config.getParameterStr("Shield"), boost::is_any_of(" "));
      for (const auto& name : shields) {
        prep = name + ".";

        //-   Shield made as box with optional subtracted box-shaped inner space (hole)

        double shield_W = m_config.getParameter(prep + "W") * unitFactor;
        double shield_H = m_config.getParameter(prep + "H") * unitFactor;
        double shield_L = m_config.getParameter(prep + "L") * unitFactor;
        double shield_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double shield_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double shield_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;

        double shield_hole_W = m_config.getParameter(prep + "holeW", 0) * unitFactor;
        double shield_hole_H = m_config.getParameter(prep + "holeH", 0) * unitFactor;
        double shield_hole_L = m_config.getParameter(prep + "holeL", 0) * unitFactor;
        double shield_hole_dX = m_config.getParameter(prep + "holeDX", 0) * unitFactor;
        double shield_hole_dY = m_config.getParameter(prep + "holeDY", 0) * unitFactor;
        double shield_hole_dZ = m_config.getParameter(prep + "holeDZ", 0) * unitFactor;

        double shield_PHI = m_config.getParameter(prep + "PHI");

        // storable element
        CryostatElement shield;

        shield.transform = G4Translate3D(shield_X0, shield_Y0, shield_Z0);
        shield.transform = shield.transform * G4RotateY3D(shield_PHI / Unit::rad);

        G4Transform3D transform_shield_hole = G4Translate3D(shield_hole_dX, shield_hole_dY, shield_hole_dZ);

        //define geometry
        string geo_shieldx_name = "geo_" + name + "x_name";
        string geo_shield_hole_name = "geo_" + name + "_hole_name";
        string geo_shield_name = "geo_" + name + "_name";

        if (shield_hole_W == 0 || shield_hole_H == 0 || shield_hole_L == 0) {
          G4Box* geo_shield = new G4Box(geo_shield_name, shield_W / 2.0, shield_H / 2.0, shield_L / 2.0);

          shield.geo = geo_shield;
        } else {
          G4Box* geo_shieldx = new G4Box(geo_shieldx_name, shield_W / 2.0, shield_H / 2.0, shield_L / 2.0);
          G4Box* geo_shield_hole = new G4Box(geo_shield_hole_name, shield_hole_W / 2.0, shield_hole_H / 2.0, shield_hole_L / 2.0);
          G4SubtractionSolid* geo_shield = new G4SubtractionSolid(geo_shield_name, geo_shieldx, geo_shield_hole,
                                                                  transform_shield_hole);

          shield.geo = geo_shield;
        }

        string strMat_shield = m_config.getParameterStr(prep + "Material");
        G4Material* mat_shield = Materials::get(strMat_shield);

        string logi_shield_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_shield = new G4LogicalVolume(shield.geo, mat_shield, logi_shield_name);

        shield.logi = logi_shield;

        //put volume
        setColor(*logi_shield, "#0000CC");
        //setVisibility(*logi_shield, false);
        string phys_shield_name = "phys_" + name + "_name";
        new G4PVPlacement(shield.transform, shield.logi, phys_shield_name, &topVolume, false, 0);

        elements[name] = shield;
      }


      // RVC connection structure (simplified shape)
      G4Tubs* geo_rvcR = new G4Tubs("geo_rvcR", 60, 60 + 60, (620 - 560) / 2., 0, 2 * M_PI);
      G4LogicalVolume* logi_rvcR = new G4LogicalVolume(geo_rvcR, Materials::get("SUS316L"), "logi_rvcR_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, (620 + 560) / 2.), logi_rvcR, "phys_rvcR_name", &topVolume, false, 0);

      G4Tubs* geo_rvcL = new G4Tubs("geo_rvcL", 60, 60 + 60, (-560 - (-620)) / 2., 0, 2 * M_PI);
      G4LogicalVolume* logi_rvcL = new G4LogicalVolume(geo_rvcL, Materials::get("SUS316L"), "logi_rvcL_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, (-620 - 560) / 2.), logi_rvcL, "phys_rvcL_name", &topVolume, false, 0);

      // Added 10 Nov 2018
      // Elliptical inner surface around QC1LE
      G4EllipticalTube* geo_elp_QC1LEx = new G4EllipticalTube("geo_elp_QC1LEx", 10.5, 13.5, (-675 - (-1225)) / 2.); //in mm
      G4IntersectionSolid* geo_elp_QC1LE = new G4IntersectionSolid("geo_elp_QC1LE", elements["D2wal1"].geo, geo_elp_QC1LEx,
          G4Translate3D(0, 0, (-675 - 1225) / 2.));
      G4LogicalVolume* logi_elp_QC1LE = new G4LogicalVolume(geo_elp_QC1LE, Materials::get("Vacuum"), "logi_elp_QC1LE_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_elp_QC1LE, "phys_elp_QC1LE_name", elements["D2wal1"].logi, false, 0);
      // Elliptical inner surface around QC1LP
      G4EllipticalTube* geo_elp_QC1LPx = new G4EllipticalTube("geo_elp_QC1LPx", 10.5, 13.5, (-675 - (-1225)) / 2.); //in mm
      G4IntersectionSolid* geo_elp_QC1LP = new G4IntersectionSolid("geo_elp_QC1LP", elements["E2wal1"].geo, geo_elp_QC1LPx,
          G4Translate3D(0, 0, (-675 - 1225) / 2.));
      G4LogicalVolume* logi_elp_QC1LP = new G4LogicalVolume(geo_elp_QC1LP, Materials::get("Vacuum"), "logi_elp_QC1LP_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_elp_QC1LP, "phys_elp_QC1LP_name", elements["E2wal1"].logi, false, 0);
      // Elliptical inner surface around QC1RE
      G4EllipticalTube* geo_elp_QC1REx = new G4EllipticalTube("geo_elp_QC1REx", 10.5, 13.5, (1225 - 675) / 2.); //in mm
      G4IntersectionSolid* geo_elp_QC1RE = new G4IntersectionSolid("geo_elp_QC1RE", elements["A2wal1"].geo, geo_elp_QC1REx,
          G4Translate3D(0, 0, (1225 + 675) / 2.));
      G4LogicalVolume* logi_elp_QC1RE = new G4LogicalVolume(geo_elp_QC1RE, Materials::get("Vacuum"), "logi_elp_QC1RE_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_elp_QC1RE, "phys_elp_QC1RE_name", elements["A2wal1"].logi, false, 0);
      // Elliptical inner surface around QC1RP
      G4EllipticalTube* geo_elp_QC1RPx = new G4EllipticalTube("geo_elp_QC1RPx", 10.5, 13.5, (1225 - 675) / 2.); //in mm
      G4IntersectionSolid* geo_elp_QC1RP = new G4IntersectionSolid("geo_elp_QC1RP", elements["B2wal1"].geo, geo_elp_QC1RPx,
          G4Translate3D(0, 0, (1225 + 675) / 2.));
      G4LogicalVolume* logi_elp_QC1RP = new G4LogicalVolume(geo_elp_QC1RP, Materials::get("Vacuum"), "logi_elp_QC1RP_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_elp_QC1RP, "phys_elp_QC1RP_name", elements["B2wal1"].logi, false, 0);




      //--------------------------------------------------------------------------------------------
      //-   2023 QCS shielding

      std::vector<std::string> polyBlocks;
      boost::split(polyBlocks, m_config.getParameterStr("PolyBlock"), boost::is_any_of(" "));
      for (const auto& name : polyBlocks) {
        prep = name + ".";
        //string type = m_config.getParameterStr(prep + "type");

        int block_En = int(m_config.getParameter(prep + "Enable"));
        double block_L = m_config.getParameter(prep + "L") * unitFactor;
        double block_R = m_config.getParameter(prep + "R") * unitFactor;
        double block_r = m_config.getParameter(prep + "r") * unitFactor;
        double block_W = m_config.getParameter(prep + "W") * unitFactor;
        double block_w = m_config.getParameter(prep + "w") * unitFactor;
        double block_t = m_config.getParameter(prep + "t") * unitFactor;
        double block_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double block_dr = m_config.getParameter(prep + "dr", 0.0) * unitFactor;
        // Number of instances
        int block_N = int(m_config.getParameter(prep + "N"));

        std::vector<double> block_PHIs(block_N);
        for (int i = 0; i < block_N; ++i) {
          ostringstream oss_block_num;
          oss_block_num << i;
          block_PHIs[i] = m_config.getParameter(prep + "PHI" + oss_block_num.str());
        }

        // Box shaped cuts, if any
        double block_cut_L0 = m_config.getParameter(prep + "cutL0", 0.0) * unitFactor;
        double block_cut_L1 = m_config.getParameter(prep + "cutL1", 0.0) * unitFactor;
        int block_cut_N = 0;
        if (block_cut_L0 != 0.0 && block_cut_L1 != 0.0) {
          block_cut_N = 2;
        } else if (block_cut_L0 != 0.0 || block_cut_L1 != 0.0) {
          block_cut_N = 1;
        } else {
          block_cut_N = 0;
        }

        //define geometry
        string geo_block_name;
        if (block_cut_N == 0) {
          geo_block_name = "geo_" + name + "_name";
        } else {
          geo_block_name = "geo_" + name + "_x_name";
        }

        double block_T = sqrt((block_R - block_r) * (block_R - block_r) + (block_W - block_w) * (block_W - block_w) / 4.0);

        int nSect = 5;
        std::vector<G4TwoVector> xy(nSect);
        xy[0].set(0.0, 0.0);
        xy[1].set(-block_T, (block_W - block_w) / 2.0);
        xy[2].set(-block_t, block_W / 2.0);
        xy[3].set(-block_T, (block_w + block_W) / 2.0);
        xy[4].set(0.0, block_W);

        G4TwoVector offset1(block_dr, 0.0), offset2(0.0, 0.0);
        G4double scale1 = 1.0, scale2 = 1.0;

        G4VSolid* geo_block = new G4ExtrudedSolid(geo_block_name, xy, block_L / 2.0, offset1, scale1, offset2, scale2);

        for (int i = 0; i < block_cut_N; ++i) {
          ostringstream oss_block_num;
          oss_block_num << i;

          string geo_blockx_name;
          if (i == block_cut_N) {
            geo_blockx_name = "geo_" + name + "_name";
          } else {
            geo_block_name = "geo_" + name + "_x" + oss_block_num.str() + "_name";
          }
          string geo_cut_name = "geo_" + name + "_cut" + oss_block_num.str() + "_name";

          double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
          double cut_W = m_config.getParameter(prep + "cutW" + oss_block_num.str()) * unitFactor;
          double cut_H = m_config.getParameter(prep + "cutH" + oss_block_num.str()) * unitFactor;

          G4VSolid* geo_cut = new G4Box(geo_cut_name, cut_W / 2.0, cut_H / 2.0, cut_L / 2.0);

          double cut_X0 = m_config.getParameter(prep + "cutX0" + oss_block_num.str()) * unitFactor;
          double cut_Y0 = m_config.getParameter(prep + "cutY0" + oss_block_num.str()) * unitFactor;
          double cut_Z0 = m_config.getParameter(prep + "cutZ0" + oss_block_num.str()) * unitFactor;
          double cut_PHI = m_config.getParameter(prep + "cutPHI" + oss_block_num.str());

          G4Transform3D cut_transform = G4Translate3D(cut_X0, cut_Y0, cut_Z0);
          cut_transform = cut_transform * G4RotateY3D(cut_PHI / Unit::rad);

          geo_block = new G4SubtractionSolid(geo_block_name, geo_block, geo_cut,  cut_transform);
          //geo_block = new G4UnionSolid(geo_block_name, geo_block, geo_cut,  cut_transform);
        }

        // logical volume
        string strMat_block = m_config.getParameterStr(prep + "Material");
        G4Material* mat_block = Materials::get(strMat_block);

        string logi_block_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_block = new G4LogicalVolume(geo_block, mat_block, logi_block_name);

        //put volume
        setColor(*logi_block, "#0000CC");
        //setVisibility(*logi_block, false);

        for (int i = 0; i < block_N; ++i) {
          // storable element
          CryostatElement block;
          block.geo = geo_block;
          block.logi = logi_block;

          double block_PHI = block_PHIs[i];
          double block_X0 = block_R * cos(block_PHI);
          double block_Y0 = block_R * sin(block_PHI);
          double block_dPHI = asin(block_W / block_R / 2.0);

          block.transform = G4Translate3D(block_X0, block_Y0, block_Z0);
          block.transform = block.transform * G4RotateZ3D((block_PHI + block_dPHI) / Unit::rad);

          ostringstream oss_block_num;
          oss_block_num << i;
          string phys_block_name = "phys_" + name + "-" + oss_block_num.str() + "_name";
          if (block_En == 1)
            new G4PVPlacement(block.transform, block.logi, phys_block_name, &topVolume, false, 0);

          elements[name] = block;
        }
      }

      std::vector<std::string> SWXLayers;
      boost::split(SWXLayers, m_config.getParameterStr("SWXLayer"), boost::is_any_of(" "));
      for (const auto& name : SWXLayers) {
        prep = name + ".";
        //string type = m_config.getParameterStr(prep + "type");

        // storable element
        CryostatElement layer;

        int layer_En = int(m_config.getParameter(prep + "Enable"));
        double layer_L = m_config.getParameter(prep + "L") * unitFactor;
        double layer_r1 = m_config.getParameter(prep + "r1") * unitFactor;
        double layer_r2 = m_config.getParameter(prep + "r2") * unitFactor;
        double layer_t = m_config.getParameter(prep + "t") * unitFactor;
        double layer_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        int layer_cut_N = int(m_config.getParameter(prep + "N", 0));

        layer.transform = G4Translate3D(0.0, 0.0, layer_Z0);

        //define geometry
        string geo_layer_name;
        if (layer_cut_N == 0) {
          geo_layer_name = "geo_" + name + "_name";
        } else {
          geo_layer_name = "geo_" + name + "_x_name";
        }

        G4VSolid* geo_layer = new G4Cons(geo_layer_name, layer_r1, layer_r1 + layer_t, layer_r2, layer_r2 + layer_t, layer_L / 2.0, 0.0,
                                         2.0 * M_PI);

        for (int i = 0; i < layer_cut_N; ++i) {
          ostringstream oss_block_num;
          oss_block_num << i;

          //string cut_type = m_config.getParameterStr(prep + "cutType" + oss_block_num.str());
          double cut_type = m_config.getParameter(prep + "cutType" + oss_block_num.str());
          string geo_layerx_name;
          if (i == layer_cut_N) {
            geo_layerx_name = "geo_" + name + "_name";
          } else {
            geo_layer_name = "geo_" + name + "_x" + oss_block_num.str() + "_name";
          }
          string geo_cut_name = "geo_" + name + "_cut" + oss_block_num.str() + "_name";

          G4VSolid* geo_cut;
          //if(cut_type == "Box") {
          if (cut_type == 0.0) {
            double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
            double cut_W = m_config.getParameter(prep + "cutW" + oss_block_num.str()) * unitFactor;
            double cut_H = m_config.getParameter(prep + "cutH" + oss_block_num.str()) * unitFactor;

            geo_cut = new G4Box(geo_cut_name, cut_W / 2.0, cut_H / 2.0, cut_L / 2.0);
            //} else if(cut_type == "Tubs") {
          } else if (cut_type != 0.0) {
            double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
            double cut_R = m_config.getParameter(prep + "cutR" + oss_block_num.str()) * unitFactor;

            geo_cut = new G4Tubs(geo_cut_name, 0.0, cut_R, cut_L / 2.0, 0.0, 2.0 * M_PI);
          } else
            continue;

          double cut_X0 = m_config.getParameter(prep + "cutX0" + oss_block_num.str()) * unitFactor;
          double cut_Y0 = m_config.getParameter(prep + "cutY0" + oss_block_num.str()) * unitFactor;
          double cut_Z0 = m_config.getParameter(prep + "cutZ0" + oss_block_num.str()) * unitFactor;
          double cut_PHI = m_config.getParameter(prep + "cutPHI" + oss_block_num.str());
          double cut_TH = m_config.getParameter(prep + "cutTH" + oss_block_num.str());

          G4Transform3D cut_transform = G4Translate3D(cut_X0, cut_Y0, cut_Z0);
          cut_transform = cut_transform * G4RotateY3D(cut_PHI / Unit::rad);
          cut_transform = cut_transform * G4RotateX3D(cut_TH / Unit::rad);

          geo_layer = new G4SubtractionSolid(geo_layer_name, geo_layer, geo_cut,  layer.transform.inverse()*cut_transform);
          //geo_layer = new G4UnionSolid(geo_layer_name, geo_layer, geo_cut,  layer.transform.inverse()*cut_transform);
        }

        layer.geo = geo_layer;

        // logical volume
        string strMat_layer = m_config.getParameterStr(prep + "Material");
        G4Material* mat_layer = Materials::get(strMat_layer);

        string logi_layer_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_layer = new G4LogicalVolume(layer.geo, mat_layer, logi_layer_name);
        layer.logi = logi_layer;

        //put volume
        setColor(*logi_layer, "#0000CC");
        //setVisibility(*logi_layer, false);

        string phys_layer_name = "phys_" + name + "_name";
        if (layer_En == 1)
          new G4PVPlacement(layer.transform, layer.logi, phys_layer_name, &topVolume, false, 0);

        elements[name] = layer;
      }

      //---------------------------
      // for dose simulation
      //---------------------------

      /*
       logi_A1spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 101));
       logi_A1spc2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 102));
       logi_A2wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 103));
       logi_A3wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 104));
       logi_A3wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 105));
       logi_A4mag1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 106));
       logi_A4mag2p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 107));
       logi_A4mag2p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 108));
       logi_A4mag2p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 109));
       logi_A4mag2p4 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 110));
       logi_A4mag3p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 111));
       logi_A4mag3p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 112));
       logi_A4mag4p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 113));
       logi_A4mag4p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 114));
       logi_A4mag4p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 115));
       logi_A4mag4p4 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 116));
       logi_A4mag4p5 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 117));
       logi_A4mag4p6 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 118));
       logi_A4mag4p7 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 119));
       logi_A4mag4p8 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 120));
       logi_A4mag4p9 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 121));
       logi_B1spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 122));
       logi_B1spc2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 123));
       logi_B2wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 124));
       logi_B3wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 125));
       logi_B3wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 126));
       logi_B4mag1p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 127));
       logi_B4mag1p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 128));
       logi_B4mag1p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 129));
       logi_B4mag2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 130));
       logi_B4mag3p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 131));
       logi_B4mag3p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 132));
       logi_B4mag3p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 133));
       logi_B4mag3p4 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 134));
       logi_B4mag3p5 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 135));
       logi_B4mag3p6 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 136));
       logi_B4mag4p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 137));
       logi_B4mag4p7 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 138));
       logi_B4mag4p8 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 139));
       logi_B4mag4p9 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 140));
       logi_C1wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 141));
       logi_C1wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 142));
       logi_C2spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 143));
       logi_C2spc2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 144));
       logi_C2spc3   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 145));
       logi_C3wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 146));
       logi_C3wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 147));
       logi_C4spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 148));
       logi_C4spc2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 149));
       logi_C5wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 150));
       logi_C5wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 151));
       logi_C5wal3   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 152));
       logi_C6spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 153));
       logi_C6spc2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 154));
       logi_C7wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 155));
       logi_C7lyr1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 156));
       logi_C7lyr2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 157));
       logi_C7lyr3   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 158));
       logi_C7lyr4   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 159));
       logi_C7lyr5   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 160));
       logi_D1spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 161));
       logi_D2wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 162));
       logi_D3wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 163));
       logi_D3wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 164));
       logi_D4mag1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 165));
       logi_D4mag2p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 166));
       logi_D4mag2p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 167));
       logi_D4mag2p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 168));
       logi_D4mag2p4 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 169));
       logi_D4mag3p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 170));
       logi_D4mag3p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 171));
       logi_D4mag3p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 172));
       logi_D4mag3p4 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 173));
       logi_D4mag3p5 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 174));
       logi_D4mag3p6 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 175));
       logi_E1spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 176));
       logi_E2wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 177));
       logi_E3wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 178));
       logi_E4mag1p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 179));
       logi_E4mag1p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 180));
       logi_E4mag1p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 181));
       logi_E4mag2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 182));
       logi_E4mag3p1 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 183));
       logi_E4mag3p2 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 184));
       logi_E4mag3p3 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 185));
       logi_E4mag3p4 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 186));
       logi_E4mag3p5 ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 187));
       logi_F1wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 188));
       logi_F1wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 189));
       logi_F2spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 190));
       logi_F2spc2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 191));
       logi_F2spc3   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 192));
       logi_F3wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 193));
       logi_F3wal2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 194));
       logi_F3wal3   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 195));
       logi_F4spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 196));
       logi_F5wal1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 197));
       logi_F6spc1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 198));
       logi_F7lyr1   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 199));
       logi_F7lyr2   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 200));
       logi_F7lyr3   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 201));
       logi_F7lyr4   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 202));
       logi_F7lyr5   ->SetSensitiveDetector(new BkgSensitiveDetector("IR", 203));
      */

      //--------------------------- QCS shielding ---------------------------//
      //--- FWD + BWD QCS volme to be sutracted from the shielding volume
      G4UnionSolid* qcs_solid_cut = new G4UnionSolid("qcs_solid_cut", geo_F1wal1xxx, geo_C1wal1xxx);

      //--- Inner layer of the shielding
      // FWD part 1
      CryostatElement APSR9;
      std::string name = "APSR9";
      prep = name + ".";
      int apsr9_En = int(m_config.getParameter(prep + "Enable"));
      int apsr9_N = 2;
      std::vector<double> apsr9_Z(apsr9_N);
      std::vector<double> apsr9_R(apsr9_N);
      std::vector<double> apsr9_r(apsr9_N);

      // in mm
      apsr9_Z[0] = 1049.99 + 580.0 + 330.0;
      apsr9_Z[1] = 1049.99 + 580.0 + 330.0 + 351.0;
      apsr9_R[0] = 290.0;
      apsr9_R[1] = 290.0;
      apsr9_r[0] = 0;
      apsr9_r[1] = 0;

      // rotation matrix
      G4RotationMatrix* Rotation1 = new G4RotationMatrix();
      Rotation1->rotateY(M_PI_2 / Unit::rad);
      Rotation1->rotateX(M_PI_4 / Unit::rad);
      G4RotationMatrix* Rotation2 = new G4RotationMatrix();
      Rotation2->rotateY(M_PI_2 / Unit::rad);
      Rotation2->rotateX(-M_PI_4 / Unit::rad);

      APSR9.transform = G4Translate3D(0, 0, 0);

      G4Polycone* geo_apsr9x1 = new G4Polycone("geo_apsr9x1", 0.0, 2.0 * M_PI, apsr9_N, &(apsr9_Z[0]), &(apsr9_r[0]), &(apsr9_R[0]));
      G4Tubs* geo_apsr9xx1 = new G4Tubs("geo_apsr9xx1", 0, 56.50, 573.0 / 2, 0, 2.0 * M_PI);
      G4SubtractionSolid* geo_apsr9x2 = new G4SubtractionSolid("geo_apsr9x2", geo_apsr9x1, geo_apsr9xx1, Rotation1, G4ThreeVector(0, 0,
                                                               1049.99 + 580.0 + 330.0 + 351.0 - 54.0));
      G4SubtractionSolid* geo_apsr9x3 = new G4SubtractionSolid("geo_apsr9x3", geo_apsr9x2, geo_apsr9xx1, Rotation2, G4ThreeVector(0, 0,
                                                               1049.99 + 580.0 + 330.0 + 350.0 - 54.0));
      G4Box* geo_apsr9xx2 = new G4Box("geo_apsr9xx2", 300. / 2, 100. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsr9x4 = new G4SubtractionSolid("geo_apsr9x4", geo_apsr9x3, geo_apsr9xx2, 0, G4ThreeVector(-290., -30.0,
                                                               1049.99 + 580.0 + 330.0 + 351.0 / 2));
      G4SubtractionSolid* geo_apsr9x5 = new G4SubtractionSolid("geo_apsr9x5", geo_apsr9x4, geo_apsr9xx2, 0, G4ThreeVector(290., 30.0,
                                                               1049.99 + 580.0 + 330.0 + 351.0 / 2));
      G4Box* geo_apsr9xx3 = new G4Box("geo_apsr9xx3", 100. / 2, 300. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsr9x6 = new G4SubtractionSolid("geo_apsr9x6", geo_apsr9x5, geo_apsr9xx3, 0, G4ThreeVector(0, 290,
                                                               1049.99 + 580.0 + 330.0 + 351.0 / 2));
      G4VSolid* geo_apsr9 = new G4SubtractionSolid("geo_apsr9", geo_apsr9x6, qcs_solid_cut);
      // G4VSolid* geo_apsr9 = new G4SubtractionSolid("geo_apsr9", geo_apsr9x1, qcs_solid_cut);

      APSR9.geo = geo_apsr9;

      string strMat_apsr9 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsr9 = Materials::get(strMat_apsr9);
      G4LogicalVolume* logi_apsr9 = new G4LogicalVolume(geo_apsr9, mat_apsr9, "logi_apsr9");
      setColor(*logi_apsr9, "#00CC00");
      //setVisibility(*logi_apsr9, false);

      APSR9.logi = logi_apsr9;

      if (apsr9_En == 1) {
        new G4PVPlacement(APSR9.transform, APSR9.logi, "phys_apsr9", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsr9->GetName() << " : mass = " << logi_apsr9->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsr9->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSR9;

      // FWD part 2
      CryostatElement APSR10;
      name = "APSR10";
      prep = name + ".";
      int apsr10_En = int(m_config.getParameter(prep + "Enable"));
      int apsr10_N = 3;
      std::vector<double> apsr10_Z(apsr10_N);
      std::vector<double> apsr10_R(apsr10_N);
      std::vector<double> apsr10_r(apsr10_N);

      // in mm
      apsr10_Z[0] = 1049.99;
      apsr10_Z[1] = 1049.99 + 22.41;
      apsr10_Z[2] = 1049.99 + 580.0;
      apsr10_R[0] = 278.15;
      apsr10_R[1] = 285.0;
      apsr10_R[2] = 285.0;
      apsr10_r[0] = 0;
      apsr10_r[1] = 0;
      apsr10_r[2] = 0;

      // transform matrix
      G4Transform3D Transform1 = G4RotateZ3D((M_PI_2 * 0 + M_PI_4) / Unit::rad) * G4Translate3D(0, 285.0, 1049.99);
      G4Transform3D Transform2 = G4RotateZ3D((M_PI_2 * 1 + M_PI_4) / Unit::rad) * G4Translate3D(0, 285.0, 1049.99);
      G4Transform3D Transform3 = G4RotateZ3D((M_PI_2 * 2 + M_PI_4) / Unit::rad) * G4Translate3D(0, 285.0, 1049.99);
      G4Transform3D Transform4 = G4RotateZ3D((M_PI_2 * 3 + M_PI_4) / Unit::rad) * G4Translate3D(0, 285.0, 1049.99);

      APSR10.transform = G4Translate3D(0, 0, 0);

      G4Polycone* geo_apsr10x1 = new G4Polycone("geo_apsr10x1", 0.0, 2.0 * M_PI, apsr10_N, &(apsr10_Z[0]), &(apsr10_r[0]),
                                                &(apsr10_R[0]));
      G4Tubs* geo_apsr10xx1 = new G4Tubs("geo_apsr10xx1", 0, 56.50, 540.0 / 2, 0, 2.0 * M_PI);
      G4SubtractionSolid* geo_apsr10x2 = new G4SubtractionSolid("geo_apsr10x2", geo_apsr10x1, geo_apsr10xx1, Rotation1, G4ThreeVector(0,
                                                                0, 1049.99 + 16.0 + 56.50));
      G4SubtractionSolid* geo_apsr10x3 = new G4SubtractionSolid("geo_apsr10x3", geo_apsr10x2, geo_apsr10xx1, Rotation2, G4ThreeVector(0,
                                                                0, 1049.99 + 16.0 + 56.50));
      G4Box* geo_apsr10xx2 = new G4Box("geo_apsr10xx2", 300. / 2, 90. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsr10x4 = new G4SubtractionSolid("geo_apsr10x4", geo_apsr10x3, geo_apsr10xx2, 0, G4ThreeVector(-300., -20,
                                                                1049.99 + 580.0 / 2));
      G4SubtractionSolid* geo_apsr10x5 = new G4SubtractionSolid("geo_apsr10x5", geo_apsr10x4, geo_apsr10xx2, 0, G4ThreeVector(300., 20,
                                                                1049.99 + 580.0 / 2));
      G4Box* geo_apsr10xx3 = new G4Box("geo_apsr10xx3", 100. / 2, 300. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsr10x6 = new G4SubtractionSolid("geo_apsr10x6", geo_apsr10x5, geo_apsr10xx3, 0, G4ThreeVector(0, 300,
                                                                1049.99 + 580.0 / 2));
      G4Tubs* geo_apsr10xx4 = new G4Tubs("geo_apsr10xx4", 0, 41.0, 1000.0 / 2, 0, 2.0 * M_PI);
      G4SubtractionSolid* geo_apsr10x7 = new G4SubtractionSolid("geo_apsr10x7", geo_apsr10x6, geo_apsr10xx4, Rotation1, G4ThreeVector(0,
                                                                0, 1049.99 + 283.50));
      G4SubtractionSolid* geo_apsr10x8 = new G4SubtractionSolid("geo_apsr10x8", geo_apsr10x7, geo_apsr10xx4, Rotation2, G4ThreeVector(0,
                                                                0, 1049.99 + 283.50));
      G4Box* geo_apsr10xx5 = new G4Box("geo_apsr10xx5", 41., 9.45, 283.50);
      G4SubtractionSolid* geo_apsr10x9 = new G4SubtractionSolid("geo_apsr10x9", geo_apsr10x8, geo_apsr10xx5, Transform1);
      G4SubtractionSolid* geo_apsr10x10 = new G4SubtractionSolid("geo_apsr10x10", geo_apsr10x9, geo_apsr10xx5, Transform2);
      G4SubtractionSolid* geo_apsr10x11 = new G4SubtractionSolid("geo_apsr10x11", geo_apsr10x10, geo_apsr10xx5, Transform3);
      G4SubtractionSolid* geo_apsr10x12 = new G4SubtractionSolid("geo_apsr10x12", geo_apsr10x11, geo_apsr10xx5, Transform4);
      G4VSolid* geo_apsr10 = new G4SubtractionSolid("geo_apsr10", geo_apsr10x12, qcs_solid_cut);
      // G4VSolid* geo_apsr10 = new G4SubtractionSolid("geo_apsr10", geo_apsr10x1, qcs_solid_cut);

      APSR10.geo = geo_apsr10;

      string strMat_apsr10 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsr10 = Materials::get(strMat_apsr10);
      G4LogicalVolume* logi_apsr10 = new G4LogicalVolume(geo_apsr10, mat_apsr10, "logi_apsr10");
      setColor(*logi_apsr10, "#00CC00");
      //setVisibility(*logi_apsr10, false);

      APSR10.logi = logi_apsr10;

      if (apsr10_En == 1) {
        new G4PVPlacement(APSR10.transform, APSR10.logi, "phys_apsr10", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsr10->GetName() << " : mass = " << logi_apsr10->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsr10->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSR10;

      // BWD part 1
      CryostatElement APSL9;
      name = "APSL9";
      prep = name + ".";
      int apsl9_En = int(m_config.getParameter(prep + "Enable"));
      int apsl9_N = 2;
      std::vector<double> apsl9_Z(apsl9_N);
      std::vector<double> apsl9_R(apsl9_N);
      std::vector<double> apsl9_r(apsl9_N);

      // in mm
      apsl9_Z[0] = -(628.01 + 363.8 + 20);
      apsl9_Z[1] = -(628.01 + 363.8 + 20 + 316.2);
      apsl9_R[0] = 255.0;
      apsl9_R[1] = 255.0;
      apsl9_r[0] = 0;
      apsl9_r[1] = 0;

      APSL9.transform = G4Translate3D(0, 0, 0);

      G4Polycone* geo_apsl9x1 = new G4Polycone("geo_apsl9x", 0.0, 2.0 * M_PI, apsl9_N, &(apsl9_Z[0]), &(apsl9_r[0]), &(apsl9_R[0]));
      G4Box* geo_apsl9xx1 = new G4Box("geo_apsl9xx1", 300. / 2, 60. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsl9x2 = new G4SubtractionSolid("geo_apsl9x2", geo_apsl9x1, geo_apsl9xx1, 0, G4ThreeVector(-255., 5.,
                                                               -(628.01 + 363.8 + 20 + 316.2 / 2)));
      G4SubtractionSolid* geo_apsl9x3 = new G4SubtractionSolid("geo_apsl9x3", geo_apsl9x2, geo_apsl9xx1, 0, G4ThreeVector(255., -5.,
                                                               -(628.01 + 363.8 + 20 + 316.2 / 2)));
      G4Box* geo_apsl9xx2 = new G4Box("geo_apsl9xx2", 100. / 2, 300. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsl9x4 = new G4SubtractionSolid("geo_apsl9x4", geo_apsl9x3, geo_apsl9xx2, 0, G4ThreeVector(0, 255.0,
                                                               -(628.01 + 363.8 + 20 + 316.2 / 2)));
      G4VSolid* geo_apsl9 = new G4SubtractionSolid("geo_apsl9", geo_apsl9x4, qcs_solid_cut);
      // G4VSolid* geo_apsl9 = new G4SubtractionSolid("geo_apsl9", geo_apsl9x1, qcs_solid_cut);

      APSL9.geo = geo_apsl9;

      string strMat_apsl9 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsl9 = Materials::get(strMat_apsl9);
      G4LogicalVolume* logi_apsl9 = new G4LogicalVolume(geo_apsl9, mat_apsl9, "logi_apsl9");
      setColor(*logi_apsl9, "#00CC00");
      //setVisibility(*logi_apsl9, false);

      APSL9.logi = logi_apsl9;

      if (apsl9_En == 1) {
        new G4PVPlacement(APSL9.transform, APSL9.logi, "phys_apsl9", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsl9->GetName() << " : mass = " << logi_apsl9->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsl9->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSL9;

      // BWD part 2
      CryostatElement APSL10;
      name = "APSL10";
      prep = name + ".";
      int apsl10_En = int(m_config.getParameter(prep + "Enable"));
      int apsl10_N = 3;
      std::vector<double> apsl10_Z(apsl10_N);
      std::vector<double> apsl10_R(apsl10_N);
      std::vector<double> apsl10_r(apsl10_N);

      // in mm
      apsl10_Z[0] = -(628.01);
      apsl10_Z[1] = -(628.01 + 100);
      apsl10_Z[2] = -(628.01 + 363.8);
      apsl10_R[0] = 170.0;
      apsl10_R[1] = 250.0;
      apsl10_R[2] = 250.0;
      apsl10_r[0] = 0;
      apsl10_r[1] = 0;
      apsl10_r[2] = 0;

      APSL10.transform = G4Translate3D(0, 0, 0);

      G4Polycone* geo_apsl10x1 = new G4Polycone("geo_apsl10x1", 0.0, 2.0 * M_PI, apsl10_N, &(apsl10_Z[0]), &(apsl10_r[0]),
                                                &(apsl10_R[0]));
      G4Tubs* geo_apsl10xx1 = new G4Tubs("geo_apsl10xx1", 0, 40.0, 1000.0 / 2, 0, 2.0 * M_PI);
      G4SubtractionSolid* geo_apsl10x2 = new G4SubtractionSolid("geo_apsl10x2", geo_apsl10x1, geo_apsl10xx1, Rotation1, G4ThreeVector(0,
                                                                0, -(628.01 + 363.8) + 12.0 + 40.0));
      G4SubtractionSolid* geo_apsl10x3 = new G4SubtractionSolid("geo_apsl10x3", geo_apsl10x2, geo_apsl10xx1, Rotation2, G4ThreeVector(0,
                                                                0, -(628.01 + 363.8) + 12.0 + 40.0));
      G4Tubs* geo_apsl10xx2 = new G4Tubs("geo_apsl10xx2", 0, 50.0, 1000.0 / 2, 0, 2.0 * M_PI);
      G4SubtractionSolid* geo_apsl10x4 = new G4SubtractionSolid("geo_apsl10x4", geo_apsl10x3, geo_apsl10xx2, Rotation1, G4ThreeVector(0,
                                                                0, -(628.01 + 363.8) + 12.0 + 40.0 + 104.0));
      G4SubtractionSolid* geo_apsl10x5 = new G4SubtractionSolid("geo_apsl10x5", geo_apsl10x4, geo_apsl10xx2, Rotation2, G4ThreeVector(0,
                                                                0, -(628.01 + 363.8) + 12.0 + 40.0 + 104.0));
      G4Tubs* geo_apsl10xx3 = new G4Tubs("geo_apsl10xx3", 0, 64.5, 250.0 - 17.0, 0, 2.0 * M_PI);
      G4SubtractionSolid* geo_apsl10x6 = new G4SubtractionSolid("geo_apsl10x6", geo_apsl10x5, geo_apsl10xx3, Rotation1, G4ThreeVector(0,
                                                                0, -(628.01 + 363.8) + 12.0 + 40.0 + 104.0));
      G4SubtractionSolid* geo_apsl10x7 = new G4SubtractionSolid("geo_apsl10x7", geo_apsl10x6, geo_apsl10xx3, Rotation2, G4ThreeVector(0,
                                                                0, -(628.01 + 363.8) + 12.0 + 40.0 + 104.0));
      G4Box* geo_apsl10xx4 = new G4Box("geo_apsl10xx4", 300. / 2, 60. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsl10x8 = new G4SubtractionSolid("geo_apsl10x8", geo_apsl10x7, geo_apsl10xx4, 0, G4ThreeVector(-250., 5,
                                                                -(628.01 + 363.8 / 2)));
      G4SubtractionSolid* geo_apsl10x9 = new G4SubtractionSolid("geo_apsl10x9", geo_apsl10x8, geo_apsl10xx4, 0, G4ThreeVector(250., -5,
                                                                -(628.01 + 363.8 / 2)));
      G4Box* geo_apsl10xx5 = new G4Box("geo_apsl10xx5", 100. / 2, 300. / 2, 1000.0 / 2);
      G4SubtractionSolid* geo_apsl10x10 = new G4SubtractionSolid("geo_apsl10x10", geo_apsl10x9, geo_apsl10xx5, 0, G4ThreeVector(0, 250.,
                                                                 -(628.01 + 363.8 / 2)));
      G4VSolid* geo_apsl10 = new G4SubtractionSolid("geo_apsl10", geo_apsl10x10, qcs_solid_cut);
      // G4VSolid* geo_apsl10 = new G4SubtractionSolid("geo_apsl10", geo_apsl10x1, qcs_solid_cut);

      APSL10.geo = geo_apsl10;

      string strMat_apsl10 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsl10 = Materials::get(strMat_apsl10);
      G4LogicalVolume* logi_apsl10 = new G4LogicalVolume(geo_apsl10, mat_apsl10, "logi_apsl10");
      setColor(*logi_apsl10, "#00CC00");
      //setVisibility(*logi_apsl10, false);

      APSL10.logi = logi_apsl10;

      if (apsl10_En == 1) {
        new G4PVPlacement(APSL10.transform, APSL10.logi, "phys_apsl10", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsl10->GetName() << " : mass = " << logi_apsl10->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsl10->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSL10;

      //-- Outer layer of the shielding
      // FWD part 1
      CryostatElement APSR11;
      name = "APSR11";
      prep = name + ".";
      int apsr11_En = int(m_config.getParameter(prep + "Enable"));
      int apsr11_N = 2;
      std::vector<double> apsr11_Z(apsr11_N);
      std::vector<double> apsr11_R(apsr11_N);
      std::vector<double> apsr11_r(apsr11_N);

      // in mm
      apsr11_Z[0] = 1049.99 + 580.0 + 330.0 + 351.0 - 640.0 - 11.0;
      apsr11_Z[1] = 1049.99 + 580.0 + 330.0 + 351.0 - 11.0;
      apsr11_R[0] = 290.0 + 3.2;
      apsr11_R[1] = 290.0 + 3.2;
      apsr11_r[0] = 290.0;
      apsr11_r[1] = 290.0;

      APSR11.transform = G4Translate3D(0, 0, 0);

      G4VSolid* geo_apsr11 = new G4Polycone("geo_apsr11", 0.0, 2.0 * M_PI, apsr11_N, &(apsr11_Z[0]), &(apsr11_r[0]), &(apsr11_R[0]));

      APSR11.geo = geo_apsr11;

      string strMat_apsr11 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsr11 = Materials::get(strMat_apsr11);
      G4LogicalVolume* logi_apsr11 = new G4LogicalVolume(geo_apsr11, mat_apsr11, "logi_apsr11");
      setColor(*logi_apsr11, "#B100CC");
      //setVisibility(*logi_apsr11, false);

      APSR11.logi = logi_apsr11;

      if (apsr11_En == 1) {
        new G4PVPlacement(APSR11.transform, APSR11.logi, "phys_apsr11", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsr11->GetName() << " : mass = " << logi_apsr11->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsr11->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSR11;

      // FWD part 2
      CryostatElement APSR12;
      name = "APSR12";
      prep = name + ".";
      int apsr12_En = int(m_config.getParameter(prep + "Enable"));
      int apsr12_N = 2;
      std::vector<double> apsr12_Z(apsr12_N);
      std::vector<double> apsr12_R(apsr12_N);
      std::vector<double> apsr12_r(apsr12_N);

      // in mm
      apsr12_Z[0] = 1049.99 - 20.0;
      apsr12_Z[1] = 1049.99 + 580.0;
      apsr12_R[0] = 285.0 + 3.2;
      apsr12_R[1] = 285.0 + 3.2;
      apsr12_r[0] = 285.0;
      apsr12_r[1] = 285.0;

      APSR12.transform = G4Translate3D(0, 0, 0);

      /*
            G4Polycone* geo_apsr12x1 = new G4Polycone("geo_apsr12x1", 0.0, 2.0 * M_PI, apsr12_N, &(apsr12_Z[0]), &(apsr12_r[0]),
                                                      &(apsr12_R[0]));
            G4Tubs* geo_apsr12xx1 = new G4Tubs("geo_apsr12xx1", 0, 41.0, 1000.0 / 2, 0, 2.0 * M_PI);
            G4SubtractionSolid* geo_apsr12x2 = new G4SubtractionSolid("geo_apsr12x2", geo_apsr12x1, geo_apsr12xx1, Rotation1, G4ThreeVector(0,
                                                                      0, 1049.99 + 283.50));
            G4SubtractionSolid* geo_apsr12x3 = new G4SubtractionSolid("geo_apsr12x3", geo_apsr12x2, geo_apsr12xx1, Rotation2, G4ThreeVector(0,
                                                                      0, 1049.99 + 283.50));
            G4Box* geo_apsr12xx2 = new G4Box("geo_apsr12xx2", 41., (285.0 - 278.15), 283.50);
            G4SubtractionSolid* geo_apsr12x4 = new G4SubtractionSolid("geo_apsr12x4", geo_apsr12x3, geo_apsr12xx2, Transform1);
            G4SubtractionSolid* geo_apsr12x5 = new G4SubtractionSolid("geo_apsr12x5", geo_apsr12x4, geo_apsr12xx2, Transform2);
            G4SubtractionSolid* geo_apsr12x6 = new G4SubtractionSolid("geo_apsr12x6", geo_apsr12x5, geo_apsr12xx2, Transform3);
            G4VSolid* geo_apsr12 = new G4SubtractionSolid("geo_apsr12", geo_apsr12x6, geo_apsr12xx2, Transform4);
      */
      G4VSolid* geo_apsr12 = new G4Polycone("geo_apsr12", 0.0, 2.0 * M_PI, apsr12_N, &(apsr12_Z[0]), &(apsr12_r[0]),
                                            &(apsr12_R[0]));

      APSR12.geo = geo_apsr12;

      string strMat_apsr12 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsr12 = Materials::get(strMat_apsr12);
      G4LogicalVolume* logi_apsr12 = new G4LogicalVolume(geo_apsr12, mat_apsr12, "logi_apsr12");
      setColor(*logi_apsr12, "#B100CC");
      //setVisibility(*logi_apsr12, false);

      APSR12.logi = logi_apsr12;

      if (apsr12_En == 1) {
        new G4PVPlacement(APSR12.transform, APSR12.logi, "phys_apsr12", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsr12->GetName() << " : mass = " << logi_apsr12->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsr12->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSR12;

      // BWD part 1
      CryostatElement APSL11;
      name = "APSL11";
      prep = name + ".";
      int apsl11_En = int(m_config.getParameter(prep + "Enable"));
      int apsl11_N = 2;
      std::vector<double> apsl11_Z(apsl11_N);
      std::vector<double> apsl11_R(apsl11_N);
      std::vector<double> apsl11_r(apsl11_N);

      // in mm
      apsl11_Z[0] = -(628.01 + 363.8 + 20);
      apsl11_Z[1] = -(628.01 + 363.8 + 20 + 316.2 + 84.0);
      apsl11_R[0] = 255.0 + 3.2;
      apsl11_R[1] = 255.0 + 3.2;
      apsl11_r[0] = 255.0;
      apsl11_r[1] = 255.0;

      APSL11.transform = G4Translate3D(0, 0, 0);

      G4VSolid* geo_apsl11 = new G4Polycone("geo_apsl11", 0.0, 2.0 * M_PI, apsl11_N, &(apsl11_Z[0]), &(apsl11_r[0]), &(apsl11_R[0]));

      APSL11.geo = geo_apsl11;

      string strMat_apsl11 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsl11 = Materials::get(strMat_apsl11);
      G4LogicalVolume* logi_apsl11 = new G4LogicalVolume(geo_apsl11, mat_apsl11, "logi_apsl11");
      setColor(*logi_apsl11, "#B100CC");
      //setVisibility(*logi_apsl11, false);

      APSL11.logi = logi_apsl11;

      if (apsl11_En == 1) {
        new G4PVPlacement(APSL11.transform, APSL11.logi, "phys_apsl11", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsl11->GetName() << " : mass = " << logi_apsl11->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsl11->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSL11;

      // BWD part 2
      CryostatElement APSL12;
      name = "APSL12";
      prep = name + ".";
      int apsl12_En = int(m_config.getParameter(prep + "Enable"));
      int apsl12_N = 3;
      std::vector<double> apsl12_Z(apsl12_N);
      std::vector<double> apsl12_R(apsl12_N);
      std::vector<double> apsl12_r(apsl12_N);

      // in mm
      apsl12_Z[0] = -(628.01);
      apsl12_Z[1] = -(628.01 + 100);
      apsl12_Z[2] = -(628.01 + 363.8);
      apsl12_R[0] = 170.0 + 3.2;
      apsl12_R[1] = 250.0 + 3.2;
      apsl12_R[2] = 250.0 + 3.2;
      apsl12_r[0] = 170.0;
      apsl12_r[1] = 250.0;
      apsl12_r[2] = 250.0;

      // transform matrix
      G4Transform3D Transform5 = G4RotateZ3D((M_PI_2 * 0 + M_PI_4) / Unit::rad) * G4Translate3D(0, 250.0 + 3.2, -(628.01 + 100));
      G4Transform3D Transform6 = G4RotateZ3D((M_PI_2 * 1 + M_PI_4) / Unit::rad) * G4Translate3D(0, 250.0 + 3.2, -(628.01 + 100));
      G4Transform3D Transform7 = G4RotateZ3D((M_PI_2 * 2 + M_PI_4) / Unit::rad) * G4Translate3D(0, 250.0 + 3.2, -(628.01 + 100));
      G4Transform3D Transform8 = G4RotateZ3D((M_PI_2 * 3 + M_PI_4) / Unit::rad) * G4Translate3D(0, 250.0 + 3.2, -(628.01 + 100));

      APSL12.transform = G4Translate3D(0, 0, 0);
      /*
            G4Polycone* geo_apsl12x1 = new G4Polycone("geo_apsl12x1", 0.0, 2.0 * M_PI, apsl12_N, &(apsl12_Z[0]), &(apsl12_r[0]),
                                                      &(apsl12_R[0]));
            G4Tubs* geo_apsl12xx1 = new G4Tubs("geo_apsl12xx1", 0, 40.0, 1000.0 / 2, 0, 2.0 * M_PI);
            G4SubtractionSolid* geo_apsl12x2 = new G4SubtractionSolid("geo_apsl12x2", geo_apsl12x1, geo_apsl12xx1, Rotation1, G4ThreeVector(0,
                                                                      0, -(628.01 + 363.8) + 12.0 + 40.0));
            G4SubtractionSolid* geo_apsl12x3 = new G4SubtractionSolid("geo_apsl12x3", geo_apsl12x2, geo_apsl12xx1, Rotation2, G4ThreeVector(0,
                                                                      0, -(628.01 + 363.8) + 12.0 + 40.0));
            G4Box* geo_apsl12xx2 = new G4Box("geo_apsl12xx2", 40.0, 10.0, 363.8 - 100.0 - 12.0 - 40.0);
            G4SubtractionSolid* geo_apsl12x4 = new G4SubtractionSolid("geo_apsl12x4", geo_apsl12x3, geo_apsl12xx2, Transform5);
            G4SubtractionSolid* geo_apsl12x5 = new G4SubtractionSolid("geo_apsl12x5", geo_apsl12x4, geo_apsl12xx2, Transform6);
            G4SubtractionSolid* geo_apsl12x6 = new G4SubtractionSolid("geo_apsl12x6", geo_apsl12x5, geo_apsl12xx2, Transform7);
            G4VSolid* geo_apsl12 = new G4SubtractionSolid("geo_apsl12", geo_apsl12x6, geo_apsl12xx2, Transform8);
      */
      G4VSolid* geo_apsl12 = new G4Polycone("geo_apsl12", 0.0, 2.0 * M_PI, apsl12_N, &(apsl12_Z[0]), &(apsl12_r[0]),
                                            &(apsl12_R[0]));

      APSL12.geo = geo_apsl12;

      string strMat_apsl12 = m_config.getParameterStr(prep + "Material");
      G4Material* mat_apsl12 = Materials::get(strMat_apsl12);
      G4LogicalVolume* logi_apsl12 = new G4LogicalVolume(geo_apsl12, mat_apsl12, "logi_apsl12");
      setColor(*logi_apsl12, "#B100CC");
      //setVisibility(*logi_apsl12, false);

      APSL12.logi = logi_apsl12;

      if (apsl12_En == 1) {
        new G4PVPlacement(APSL12.transform, APSL12.logi, "phys_apsl12", &topVolume, false, 0);
        G4cout << "_____________" << G4endl << logi_apsl12->GetName() << " : mass = " << logi_apsl12->GetMass() / CLHEP::kg <<
               " | materials : " << logi_apsl12->GetMaterial() << G4endl << "_____________" << G4endl;
      }

      elements[name] = APSL12;

      //---------------------------
      // dose calculation
      //---------------------------
      if (elements.count("APSR9") && apsr9_En == 1) elements["APSR9"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5001));
      if (elements.count("APSR10") && apsr10_En == 1) elements["APSR10"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5002));
      if (elements.count("APSR11") && apsr11_En == 1) elements["APSR11"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5003));
      if (elements.count("APSR12") && apsr12_En == 1) elements["APSR12"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5004));
      if (elements.count("APSL9") && apsl9_En == 1) elements["APSL9"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5005));
      if (elements.count("APSL10") && apsl10_En == 1) elements["APSL10"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5006));
      if (elements.count("APSL11") && apsl11_En == 1) elements["APSL11"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5007));
      if (elements.count("APSL12") && apsl12_En == 1) elements["APSL12"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5008));

    }
  }
}
