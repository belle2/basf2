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

      // debug
      //cout << endl << "!!!  Creating a Cryostat copy..." << endl << endl;

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

          if (cut_type == 0.0) {
            double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
            double cut_W = m_config.getParameter(prep + "cutW" + oss_block_num.str()) * unitFactor;
            double cut_H = m_config.getParameter(prep + "cutH" + oss_block_num.str()) * unitFactor;

            geo_cut = new G4Box(geo_cut_name, cut_W / 2.0, cut_H / 2.0, cut_L / 2.0);
          } else {
            double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
            double cut_R = m_config.getParameter(prep + "cutR" + oss_block_num.str()) * unitFactor;

            geo_cut = new G4Tubs(geo_cut_name, 0.0, cut_R, cut_L / 2.0, 0.0, 2.0 * M_PI);
          }

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

      //----------------------------------
      //-   Bellows shield geometry
      prep = "BellowsShield.";
      double BS_fwdZ0 = m_config.getParameter(prep + "fwdZ0") * unitFactor;
      double BS_fwdZ1 = m_config.getParameter(prep + "fwdZ1") * unitFactor;
      double BS_fwdZ2 = m_config.getParameter(prep + "fwdZ2") * unitFactor;
      double BS_fwdZ3 = m_config.getParameter(prep + "fwdZ3") * unitFactor;
      double BS_fwdZ4 = m_config.getParameter(prep + "fwdZ4") * unitFactor;
      double BS_bwdZ0 = m_config.getParameter(prep + "bwdZ0") * unitFactor;
      double BS_bwdZ1 = m_config.getParameter(prep + "bwdZ1") * unitFactor;
      double BS_bwdZ2 = m_config.getParameter(prep + "bwdZ2") * unitFactor;
      double BS_bwdZ3 = m_config.getParameter(prep + "bwdZ3") * unitFactor;
      //double BS_bwdZ4 = m_config.getParameter(prep + "bwdZ4")*unitFactor;

      string strMat_BS = m_config.getParameterStr(prep + "Material");
      G4Material* mat_BS = Materials::get(strMat_BS);

      // Parameters for Base10/11 (FWD)
      double BS_fwdP0_R     = m_config.getParameter(prep + "fwdP0_R") * unitFactor;
      double BS_fwdP0_Yout  = m_config.getParameter(prep + "fwdP0_Yout") * unitFactor;
      double BS_fwdP0_Yin   = m_config.getParameter(prep + "fwdP0_Yin") * unitFactor;
      double BS_fwdP0_Xin   = m_config.getParameter(prep + "fwdP0_Xin") * unitFactor;
      double BS_fwdP0_Yedge = m_config.getParameter(prep + "fwdP0_Yedge") * unitFactor;
      double BS_fwdP0_Xedge = m_config.getParameter(prep + "fwdP0_Xedge") * unitFactor;
      double BS_fwdP0_Yside = m_config.getParameter(prep + "fwdP0_Yside") * unitFactor;
      double BS_fwdP0_Xside = m_config.getParameter(prep + "fwdP0_Xside") * unitFactor;

      double BS_fwdP0_DHalf = (BS_fwdZ2 - BS_fwdZ0) / 2;
      G4VSolid* geo_BS_fwdP0_B0 = new G4Tubs("geo_BS_fwdP0_B0", 0, BS_fwdP0_R, BS_fwdP0_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_fwdP0_B0 = G4Translate3D(0, 0, BS_fwdP0_DHalf);

      G4VSolid* geo_BS_fwdP0_V0T = new G4Tubs("geo_BS_fwdP0_V0T", 0, BS_fwdP0_Yin, BS_fwdP0_DHalf, 0.25 * M_PI, 0.5 * M_PI);
      G4VSolid* geo_BS_fwdP0_V0B = new G4Tubs("geo_BS_fwdP0_V0B", 0, BS_fwdP0_Yin, BS_fwdP0_DHalf, 1.25 * M_PI, 0.5 * M_PI);
      G4Transform3D tfm_BS_fwdP0_V0L = G4Translate3D(-BS_fwdP0_Xin, 0, BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V0R = G4Translate3D(+BS_fwdP0_Xin, 0, BS_fwdP0_DHalf);

      G4VSolid* geo_BS_fwdP0_V1 = new G4Box("geo_BS_fwdP0_V1", BS_fwdP0_Xside, BS_fwdP0_Yedge, BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V1 = G4Translate3D(0, 0, BS_fwdP0_DHalf);

      G4VSolid* geo_BS_fwdP0_V2 = new G4Box("geo_BS_fwdP0_V2", 100, 100, BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V2TR = G4Translate3D(+(100 + BS_fwdP0_Xedge), +(100 + BS_fwdP0_Yside), BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V2TL = G4Translate3D(-(100 + BS_fwdP0_Xedge), +(100 + BS_fwdP0_Yside), BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V2BR = G4Translate3D(+(100 + BS_fwdP0_Xedge), -(100 + BS_fwdP0_Yside), BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V2BL = G4Translate3D(-(100 + BS_fwdP0_Xedge), -(100 + BS_fwdP0_Yside), BS_fwdP0_DHalf);

      G4VSolid* geo_BS_fwdP0_V3 = new G4Box("geo_BS_fwdP0_V3", BS_fwdP0_Xin, BS_fwdP0_Yin, BS_fwdP0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V3 = G4Translate3D(0, 0, BS_fwdP0_DHalf);

      double BS_fwdP0_C0_DHalf = (BS_fwdZ1 - BS_fwdZ0) / 2;
      G4VSolid* geo_BS_fwdP0_V4 = new G4Box("geo_BS_fwdP0_V4", 100, 100, BS_fwdP0_C0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V4T = G4Translate3D(0, +(100 + BS_fwdP0_Yout), BS_fwdP0_C0_DHalf);
      G4Transform3D tfm_BS_fwdP0_V4B = G4Translate3D(0, -(100 + BS_fwdP0_Yout), BS_fwdP0_C0_DHalf);

      G4VSolid* geo_BS_fwdP0_Base = new G4DisplacedSolid("geo_BS_fwdP0_Base_00", geo_BS_fwdP0_B0, tfm_BS_fwdP0_B0);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_01", geo_BS_fwdP0_Base, geo_BS_fwdP0_V0T, tfm_BS_fwdP0_V0L);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_02", geo_BS_fwdP0_Base, geo_BS_fwdP0_V0T, tfm_BS_fwdP0_V0R);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_03", geo_BS_fwdP0_Base, geo_BS_fwdP0_V0B, tfm_BS_fwdP0_V0L);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_04", geo_BS_fwdP0_Base, geo_BS_fwdP0_V0B, tfm_BS_fwdP0_V0R);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_05", geo_BS_fwdP0_Base, geo_BS_fwdP0_V1, tfm_BS_fwdP0_V1);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_06", geo_BS_fwdP0_Base, geo_BS_fwdP0_V2, tfm_BS_fwdP0_V2TR);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_07", geo_BS_fwdP0_Base, geo_BS_fwdP0_V2, tfm_BS_fwdP0_V2TL);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_08", geo_BS_fwdP0_Base, geo_BS_fwdP0_V2, tfm_BS_fwdP0_V2BR);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_09", geo_BS_fwdP0_Base, geo_BS_fwdP0_V2, tfm_BS_fwdP0_V2BL);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_10", geo_BS_fwdP0_Base, geo_BS_fwdP0_V3, tfm_BS_fwdP0_V3);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_11", geo_BS_fwdP0_Base, geo_BS_fwdP0_V4, tfm_BS_fwdP0_V4T);
      geo_BS_fwdP0_Base = new G4SubtractionSolid("geo_BS_fwdP0_Base_12", geo_BS_fwdP0_Base, geo_BS_fwdP0_V4, tfm_BS_fwdP0_V4B);

      // Parameters for Base20/21 (BWD)
      double BS_bwdP0_R0  = m_config.getParameter(prep + "bwdP0_R0") * unitFactor;
      double BS_bwdP0_R1  = m_config.getParameter(prep + "bwdP0_R1") * unitFactor;
      double BS_bwdP0_Yin = m_config.getParameter(prep + "bwdP0_Yin") * unitFactor;
      double BS_bwdP0_Xin = m_config.getParameter(prep + "bwdP0_Xin") * unitFactor;

      double BS_bwdP0a_DHalf = (BS_bwdZ0 - BS_bwdZ1) / 2;
      G4VSolid* geo_BS_bwdP0_B0 = new G4Tubs("geo_BS_bwdP0_B0", 0, BS_bwdP0_R0, BS_bwdP0a_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_bwdP0_B0 = G4Translate3D(0, 0, -BS_bwdP0a_DHalf);

      double BS_bwdP0b_DHalf = (BS_bwdZ1 - BS_bwdZ2) / 2;
      G4VSolid* geo_BS_bwdP0_B1 = new G4Tubs("geo_BS_bwdP0_B1", 0, BS_bwdP0_R1, BS_bwdP0b_DHalf, 0.5 * M_PI, M_PI);
      G4Transform3D tfm_BS_bwdP0_B1 = G4Translate3D(-BS_bwdP0_Xin, 0, -BS_bwdP0b_DHalf - 2 * BS_bwdP0a_DHalf);

      G4VSolid* geo_BS_bwdP0_B2 = new G4Tubs("geo_BS_bwdP0_B2", 0, BS_bwdP0_R1, BS_bwdP0b_DHalf, 1.5 * M_PI, M_PI);
      G4Transform3D tfm_BS_bwdP0_B2 = G4Translate3D(+BS_bwdP0_Xin, 0, -BS_bwdP0b_DHalf - 2 * BS_bwdP0a_DHalf);

      G4VSolid* geo_BS_bwdP0_B3 = new G4Box("geo_BS_bwdP0_B3", BS_bwdP0_Xin, BS_bwdP0_R1, BS_bwdP0b_DHalf);
      G4Transform3D tfm_BS_bwdP0_B3 = G4Translate3D(0, 0, -BS_bwdP0b_DHalf - 2 * BS_bwdP0a_DHalf);

      double BS_bwdP0c_DHalf = (BS_bwdZ0 - BS_bwdZ2) / 2;
      G4VSolid* geo_BS_bwdP0_V0 = new G4Tubs("geo_BS_bwdP0_V0", 0, BS_bwdP0_Yin, BS_bwdP0c_DHalf, 0.5 * M_PI, M_PI);
      G4Transform3D tfm_BS_bwdP0_V0 = G4Translate3D(-BS_bwdP0_Xin, 0, -BS_bwdP0c_DHalf);

      G4VSolid* geo_BS_bwdP0_V1 = new G4Tubs("geo_BS_bwdP0_V1", 0, BS_bwdP0_Yin, BS_bwdP0c_DHalf, 1.5 * M_PI, M_PI);
      G4Transform3D tfm_BS_bwdP0_V1 = G4Translate3D(+BS_bwdP0_Xin, 0, -BS_bwdP0c_DHalf);

      G4VSolid* geo_BS_bwdP0_V2 = new G4Box("geo_BS_bwdP0_V2", BS_bwdP0_Xin, BS_bwdP0_Yin, BS_bwdP0c_DHalf);
      G4Transform3D tfm_BS_bwdP0_V2 = G4Translate3D(0, 0, -BS_bwdP0c_DHalf);

      G4VSolid* geo_BS_bwdP0_Base = new G4DisplacedSolid("geo_BS_bwdP0_Base_00", geo_BS_bwdP0_B0, tfm_BS_bwdP0_B0);
      geo_BS_bwdP0_Base = new G4UnionSolid("geo_BS_bwdP0_Base_01", geo_BS_bwdP0_Base, geo_BS_bwdP0_B1, tfm_BS_bwdP0_B1);
      geo_BS_bwdP0_Base = new G4UnionSolid("geo_BS_bwdP0_Base_02", geo_BS_bwdP0_Base, geo_BS_bwdP0_B2, tfm_BS_bwdP0_B2);
      geo_BS_bwdP0_Base = new G4UnionSolid("geo_BS_bwdP0_Base_03", geo_BS_bwdP0_Base, geo_BS_bwdP0_B3, tfm_BS_bwdP0_B3);
      geo_BS_bwdP0_Base = new G4SubtractionSolid("geo_BS_bwdP0_Base_04", geo_BS_bwdP0_Base, geo_BS_bwdP0_V0, tfm_BS_bwdP0_V0);
      geo_BS_bwdP0_Base = new G4SubtractionSolid("geo_BS_bwdP0_Base_05", geo_BS_bwdP0_Base, geo_BS_bwdP0_V1, tfm_BS_bwdP0_V1);
      geo_BS_bwdP0_Base = new G4SubtractionSolid("geo_BS_bwdP0_Base_06", geo_BS_bwdP0_Base, geo_BS_bwdP0_V2, tfm_BS_bwdP0_V2);

      // Parameters for Base5/6
      double BS_P1_Yout  = m_config.getParameter(prep + "P1_Yout") * unitFactor;
      double BS_P1_Yin   = m_config.getParameter(prep + "P1_Yin") * unitFactor;
      double BS_P1_Rout  = m_config.getParameter(prep + "P1_Rout") * unitFactor;
      double BS_P1_Rin   = m_config.getParameter(prep + "P1_Rin") * unitFactor;
      double BS_P1_Aout  = m_config.getParameter(prep + "P1_Aout");
      double BS_P1_Ain   = m_config.getParameter(prep + "P1_Ain");
      double BS_P1_RX    = m_config.getParameter(prep + "P1_RX") * unitFactor;
      double BS_fwdZhole = m_config.getParameter(prep + "fwdZhole") * unitFactor;
      double BS_bwdZhole = m_config.getParameter(prep + "bwdZhole") * unitFactor;
      double BS_P1_Rhole = m_config.getParameter(prep + "P1_Rhole") * unitFactor;
      double BS_P1_Rbump = m_config.getParameter(prep + "P1_Rbump") * unitFactor;
      double BS_P1_Yhole = m_config.getParameter(prep + "P1_Yhole") * unitFactor;
      double BS_P1_MillD1 = m_config.getParameter(prep + "P1_MillD1") * unitFactor;
      double BS_P1_MillD2 = m_config.getParameter(prep + "P1_MillD2") * unitFactor;
      double BS_P1_MillX = m_config.getParameter(prep + "P1_MillX") * unitFactor;
      double BS_P1_MillW = m_config.getParameter(prep + "P1_MillW") * unitFactor;
      double BS_fwdZcut   = m_config.getParameter(prep + "fwdZcut") * unitFactor;
      double BS_bwdZcut   = m_config.getParameter(prep + "bwdZcut") * unitFactor;
      double BS_P1_XcutTL = m_config.getParameter(prep + "P1_XcutTL") * unitFactor;
      double BS_P1_XcutTR = m_config.getParameter(prep + "P1_XcutTR") * unitFactor;
      double BS_P1_XcutBL = m_config.getParameter(prep + "P1_XcutBL") * unitFactor;
      double BS_P1_XcutBR = m_config.getParameter(prep + "P1_XcutBR") * unitFactor;

      double BS_P1_Xout  = BS_P1_Rout * cos(BS_P1_Aout) + BS_P1_RX;
      double BS_P1_Xin   = BS_P1_Rin * cos(BS_P1_Ain) + BS_P1_RX;

      double BS_P1fwd_DHalf = (BS_fwdZ3 - BS_fwdZ2) / 2;
      G4VSolid* geo_BS_P1fwd_B0 = new G4Box("geo_BS_P1fwd_B0", BS_P1_Xout, BS_P1_Yout, BS_P1fwd_DHalf);
      G4Transform3D tfm_BS_P1fwd_B0 =  G4Translate3D(0, 0, BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_B1 = new G4Tubs("geo_BS_P1fwd_B1", 0, BS_P1_Rout, BS_P1fwd_DHalf, M_PI - BS_P1_Aout, 2 * BS_P1_Aout);
      G4Transform3D tfm_BS_P1fwd_B1 =  G4Translate3D(-BS_P1_RX, 0, BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_B2 = new G4Tubs("geo_BS_P1fwd_B2", 0, BS_P1_Rout, BS_P1fwd_DHalf, 2 * M_PI - BS_P1_Aout, 2 * BS_P1_Aout);
      G4Transform3D tfm_BS_P1fwd_B2 =  G4Translate3D(+BS_P1_RX, 0, BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_B3 = new G4Tubs("geo_BS_P1fwd_B3", 0, BS_P1_Rbump, BS_P1fwd_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P1fwd_B3T =  G4Translate3D(0, +BS_P1_Yhole, BS_P1fwd_DHalf);
      G4Transform3D tfm_BS_P1fwd_B3B =  G4Translate3D(0, -BS_P1_Yhole, BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_V0 = new G4Box("geo_BS_P1fwd_V0", BS_P1_Xin, BS_P1_Yin, BS_P1fwd_DHalf);
      G4Transform3D tfm_BS_P1fwd_V0 =  G4Translate3D(0, 0, BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_V1 = new G4Tubs("geo_BS_P1fwd_V1", 0, BS_P1_Rin, BS_P1fwd_DHalf, M_PI - BS_P1_Ain, 2 * BS_P1_Ain);
      G4Transform3D tfm_BS_P1fwd_V1 =  G4Translate3D(-BS_P1_RX, 0, BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_V2 = new G4Tubs("geo_BS_P1fwd_V2", 0, BS_P1_Rin, BS_P1fwd_DHalf, 2 * M_PI - BS_P1_Ain, 2 * BS_P1_Ain);
      G4Transform3D tfm_BS_P1fwd_V2 =  G4Translate3D(+BS_P1_RX, 0, BS_P1fwd_DHalf);

      double BS_P1fwd_V3_DHalf = (BS_fwdZ3 - BS_fwdZhole) / 2;
      G4VSolid* geo_BS_P1fwd_V3 = new G4Tubs("geo_BS_P1fwd_V3", 0, BS_P1_Rhole, BS_P1fwd_V3_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P1fwd_V3T =  G4Translate3D(0, +BS_P1_Yhole, -BS_P1fwd_V3_DHalf + 2 * BS_P1fwd_DHalf);
      G4Transform3D tfm_BS_P1fwd_V3B =  G4Translate3D(0, -BS_P1_Yhole, -BS_P1fwd_V3_DHalf + 2 * BS_P1fwd_DHalf);

      G4VSolid* geo_BS_P1fwd_V4 = new G4Box("geo_BS_P1fwd_V4", BS_P1_MillW / 2, 100, BS_P1_MillD1 / 2);
      G4Transform3D tfm_BS_P1fwd_V4a =  G4Translate3D(-BS_P1_MillX, 0, BS_P1_MillD1 / 2) * G4RotateZ3D(0.25 * M_PI);
      G4Transform3D tfm_BS_P1fwd_V4b =  G4Translate3D(-BS_P1_MillX, 0, BS_P1_MillD1 / 2) * G4RotateZ3D(0.75 * M_PI);

      G4VSolid* geo_BS_P1fwd_V5 = new G4Box("geo_BS_P1fwd_V5", BS_P1_MillW / 2, 100, BS_P1_MillD2 / 2);
      G4Transform3D tfm_BS_P1fwd_V5a =  G4Translate3D(+BS_P1_MillX, 0, BS_P1_MillD2 / 2) * G4RotateZ3D(0.25 * M_PI);
      G4Transform3D tfm_BS_P1fwd_V5b =  G4Translate3D(+BS_P1_MillX, 0, BS_P1_MillD2 / 2) * G4RotateZ3D(0.75 * M_PI);

      double BS_P1fwd_V67_DHalf = (BS_fwdZ3 - BS_fwdZcut) / 2;
      double BS_P1fwd_V67_Pos   = (BS_fwdZcut - BS_fwdZ2);
      G4VSolid* geo_BS_P1fwd_V6 = new G4Box("geo_BS_P1fwd_V6", (BS_P1_XcutTR - BS_P1_XcutTL) / 2, 100, BS_P1fwd_V67_DHalf);
      G4Transform3D tfm_BS_P1fwd_V6 =  G4Translate3D((BS_P1_XcutTR + BS_P1_XcutTL) / 2, +100, BS_P1fwd_V67_DHalf + BS_P1fwd_V67_Pos);

      G4VSolid* geo_BS_P1fwd_V7 = new G4Box("geo_BS_P1fwd_V7", (BS_P1_XcutBR - BS_P1_XcutBL) / 2, 100, BS_P1fwd_V67_DHalf);
      G4Transform3D tfm_BS_P1fwd_V7 =  G4Translate3D((BS_P1_XcutBR + BS_P1_XcutBL) / 2, -100, BS_P1fwd_V67_DHalf + BS_P1fwd_V67_Pos);


      G4VSolid* geo_BS_P1fwd_Base = new G4DisplacedSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_B0, tfm_BS_P1fwd_B0);
      geo_BS_P1fwd_Base = new G4UnionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_B1, tfm_BS_P1fwd_B1);
      geo_BS_P1fwd_Base = new G4UnionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_B2, tfm_BS_P1fwd_B2);
      geo_BS_P1fwd_Base = new G4UnionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_B3, tfm_BS_P1fwd_B3T);
      geo_BS_P1fwd_Base = new G4UnionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_B3, tfm_BS_P1fwd_B3B);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V0, tfm_BS_P1fwd_V0);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V1, tfm_BS_P1fwd_V1);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V2, tfm_BS_P1fwd_V2);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V3, tfm_BS_P1fwd_V3T);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V3, tfm_BS_P1fwd_V3B);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V4, tfm_BS_P1fwd_V4a);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V4, tfm_BS_P1fwd_V4b);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V5, tfm_BS_P1fwd_V5a);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V5, tfm_BS_P1fwd_V5b);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V6, tfm_BS_P1fwd_V6);
      geo_BS_P1fwd_Base = new G4SubtractionSolid("geo_BS_P1fwd_Base", geo_BS_P1fwd_Base, geo_BS_P1fwd_V7, tfm_BS_P1fwd_V7);

      double BS_P1bwd_DHalf = -(BS_bwdZ3 - BS_bwdZ2) / 2;
      G4VSolid* geo_BS_P1bwd_B0 = new G4Box("geo_BS_P1bwd_B0", BS_P1_Xout, BS_P1_Yout, BS_P1bwd_DHalf);
      G4Transform3D tfm_BS_P1bwd_B0 =  G4Translate3D(0, 0, BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_B1 = new G4Tubs("geo_BS_P1bwd_B1", 0, BS_P1_Rout, BS_P1bwd_DHalf, M_PI - BS_P1_Aout, 2 * BS_P1_Aout);
      G4Transform3D tfm_BS_P1bwd_B1 =  G4Translate3D(-BS_P1_RX, 0, BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_B2 = new G4Tubs("geo_BS_P1bwd_B2", 0, BS_P1_Rout, BS_P1bwd_DHalf, 2 * M_PI - BS_P1_Aout, 2 * BS_P1_Aout);
      G4Transform3D tfm_BS_P1bwd_B2 =  G4Translate3D(+BS_P1_RX, 0, BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_B3 = new G4Tubs("geo_BS_P1bwd_B3", 0, BS_P1_Rbump, BS_P1bwd_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P1bwd_B3T =  G4Translate3D(0, +BS_P1_Yhole, BS_P1bwd_DHalf);
      G4Transform3D tfm_BS_P1bwd_B3B =  G4Translate3D(0, -BS_P1_Yhole, BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_V0 = new G4Box("geo_BS_P1bwd_V0", BS_P1_Xin, BS_P1_Yin, BS_P1bwd_DHalf);
      G4Transform3D tfm_BS_P1bwd_V0 =  G4Translate3D(0, 0, BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_V1 = new G4Tubs("geo_BS_P1bwd_V1", 0, BS_P1_Rin, BS_P1bwd_DHalf, M_PI - BS_P1_Ain, 2 * BS_P1_Ain);
      G4Transform3D tfm_BS_P1bwd_V1 =  G4Translate3D(-BS_P1_RX, 0, BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_V2 = new G4Tubs("geo_BS_P1bwd_V2", 0, BS_P1_Rin, BS_P1bwd_DHalf, 2 * M_PI - BS_P1_Ain, 2 * BS_P1_Ain);
      G4Transform3D tfm_BS_P1bwd_V2 =  G4Translate3D(+BS_P1_RX, 0, BS_P1bwd_DHalf);

      double BS_P1bwd_V3_DHalf = -(BS_bwdZ3 - BS_bwdZhole) / 2;
      G4VSolid* geo_BS_P1bwd_V3 = new G4Tubs("geo_BS_P1bwd_V3", 0, BS_P1_Rhole, BS_P1bwd_V3_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P1bwd_V3T =  G4Translate3D(0, +BS_P1_Yhole, -BS_P1bwd_V3_DHalf + 2 * BS_P1bwd_DHalf);
      G4Transform3D tfm_BS_P1bwd_V3B =  G4Translate3D(0, -BS_P1_Yhole, -BS_P1bwd_V3_DHalf + 2 * BS_P1bwd_DHalf);

      G4VSolid* geo_BS_P1bwd_V4 = new G4Box("geo_BS_P1bwd_V4", BS_P1_MillW / 2, 100, BS_P1_MillD1 / 2);
      G4Transform3D tfm_BS_P1bwd_V4a =  G4Translate3D(-BS_P1_MillX, 0, BS_P1_MillD1 / 2) * G4RotateZ3D(0.25 * M_PI);
      G4Transform3D tfm_BS_P1bwd_V4b =  G4Translate3D(-BS_P1_MillX, 0, BS_P1_MillD1 / 2) * G4RotateZ3D(0.75 * M_PI);

      G4VSolid* geo_BS_P1bwd_V5 = new G4Box("geo_BS_P1bwd_V5", BS_P1_MillW / 2, 100, BS_P1_MillD2 / 2);
      G4Transform3D tfm_BS_P1bwd_V5a =  G4Translate3D(+BS_P1_MillX, 0, BS_P1_MillD2 / 2) * G4RotateZ3D(0.25 * M_PI);
      G4Transform3D tfm_BS_P1bwd_V5b =  G4Translate3D(+BS_P1_MillX, 0, BS_P1_MillD2 / 2) * G4RotateZ3D(0.75 * M_PI);

      double BS_P1bwd_V67_DHalf = -(BS_bwdZ3 - BS_bwdZcut) / 2;
      double BS_P1bwd_V67_Pos   = -(BS_bwdZcut - BS_bwdZ2);
      G4VSolid* geo_BS_P1bwd_V6 = new G4Box("geo_BS_P1bwd_V6", (BS_P1_XcutTR - BS_P1_XcutTL) / 2, 100, BS_P1bwd_V67_DHalf);
      G4Transform3D tfm_BS_P1bwd_V6 =  G4Translate3D((BS_P1_XcutTR + BS_P1_XcutTL) / 2, +100, BS_P1bwd_V67_DHalf + BS_P1bwd_V67_Pos);

      G4VSolid* geo_BS_P1bwd_V7 = new G4Box("geo_BS_P1bwd_V7", (BS_P1_XcutBR - BS_P1_XcutBL) / 2, 100, BS_P1bwd_V67_DHalf);
      G4Transform3D tfm_BS_P1bwd_V7 =  G4Translate3D((BS_P1_XcutBR + BS_P1_XcutBL) / 2, -100, BS_P1bwd_V67_DHalf + BS_P1bwd_V67_Pos);


      G4VSolid* geo_BS_P1bwd_Base = new G4DisplacedSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_B0, tfm_BS_P1bwd_B0);
      geo_BS_P1bwd_Base = new G4UnionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_B1, tfm_BS_P1bwd_B1);
      geo_BS_P1bwd_Base = new G4UnionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_B2, tfm_BS_P1bwd_B2);
      geo_BS_P1bwd_Base = new G4UnionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_B3, tfm_BS_P1bwd_B3T);
      geo_BS_P1bwd_Base = new G4UnionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_B3, tfm_BS_P1bwd_B3B);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V0, tfm_BS_P1bwd_V0);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V1, tfm_BS_P1bwd_V1);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V2, tfm_BS_P1bwd_V2);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V3, tfm_BS_P1bwd_V3T);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V3, tfm_BS_P1bwd_V3B);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V4, tfm_BS_P1bwd_V4a);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V4, tfm_BS_P1bwd_V4b);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V5, tfm_BS_P1bwd_V5a);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V5, tfm_BS_P1bwd_V5b);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V6, tfm_BS_P1bwd_V6);
      geo_BS_P1bwd_Base = new G4SubtractionSolid("geo_BS_P1bwd_Base", geo_BS_P1bwd_Base, geo_BS_P1bwd_V7, tfm_BS_P1bwd_V7);

      // Parameters for Base7/8 (FWD) Base23/24 (BWD)
      double BS_P2_Rout  = m_config.getParameter(prep + "P2_Rout") * unitFactor;
      double BS_P2_Rin   = m_config.getParameter(prep + "P2_Rin") * unitFactor;
      double BS_P2_Xin   = m_config.getParameter(prep + "P2_Xin") * unitFactor;
      double BS_P2_Rhole = m_config.getParameter(prep + "P2_Rhole") * unitFactor;
      double BS_P2_Yhole = m_config.getParameter(prep + "P2_Yhole") * unitFactor;
      double BS_P2_XcutTL = m_config.getParameter(prep + "P2_XcutTL") * unitFactor;
      double BS_P2_XcutTR = m_config.getParameter(prep + "P2_XcutTR") * unitFactor;
      double BS_P2_XcutBL = m_config.getParameter(prep + "P2_XcutBL") * unitFactor;
      double BS_P2_XcutBR = m_config.getParameter(prep + "P2_XcutBR") * unitFactor;

      double BS_P2fwd_DHalf = (BS_fwdZ4 - BS_fwdZ3) / 2;
      G4VSolid* geo_BS_P2fwd_B0 = new G4Tubs("geo_BS_P2fwd_B0", 0, BS_P2_Rout, BS_P2fwd_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P2fwd_B0 =  G4Translate3D(0, 0, BS_P2fwd_DHalf);

      G4VSolid* geo_BS_P2fwd_V0 = new G4Box("geo_BS_P2fwd_V0", BS_P2_Xin, BS_P2_Rin, BS_P2fwd_DHalf);
      G4Transform3D tfm_BS_P2fwd_V0 =  G4Translate3D(0, 0, BS_P2fwd_DHalf);

      G4VSolid* geo_BS_P2fwd_V1 = new G4Tubs("geo_BS_P2fwd_V1", 0, BS_P2_Rin, BS_P2fwd_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P2fwd_V1L =  G4Translate3D(-BS_P2_Xin, 0, BS_P2fwd_DHalf);
      G4Transform3D tfm_BS_P2fwd_V1R =  G4Translate3D(+BS_P2_Xin, 0, BS_P2fwd_DHalf);

      G4VSolid* geo_BS_P2fwd_V2 = new G4Tubs("geo_BS_P2fwd_V2", 0, BS_P2_Rhole, BS_P2fwd_DHalf, 0, 2 * M_PI);
      G4Transform3D tfm_BS_P2fwd_V2T =  G4Translate3D(0, +BS_P2_Yhole, BS_P2fwd_DHalf);
      G4Transform3D tfm_BS_P2fwd_V2B =  G4Translate3D(0, -BS_P2_Yhole, BS_P2fwd_DHalf);

      G4VSolid* geo_BS_P2fwd_V3 = new G4Box("geo_BS_P2fwd_V3", (BS_P2_XcutTR - BS_P2_XcutTL) / 2, 100, BS_P2fwd_DHalf);
      G4Transform3D tfm_BS_P2fwd_V3 =  G4Translate3D((BS_P2_XcutTR + BS_P2_XcutTL) / 2, +100, BS_P2fwd_DHalf);

      G4VSolid* geo_BS_P2fwd_V4 = new G4Box("geo_BS_P2fwd_V4", (BS_P2_XcutBR - BS_P2_XcutBL) / 2, 100, BS_P2fwd_DHalf);
      G4Transform3D tfm_BS_P2fwd_V4 =  G4Translate3D((BS_P2_XcutBR + BS_P2_XcutBL) / 2, -100, BS_P2fwd_DHalf);

      G4VSolid* geo_BS_P2fwd_Base = new G4DisplacedSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_B0, tfm_BS_P2fwd_B0);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V0, tfm_BS_P2fwd_V0);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V1, tfm_BS_P2fwd_V1L);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V1, tfm_BS_P2fwd_V1R);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V2, tfm_BS_P2fwd_V2T);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V2, tfm_BS_P2fwd_V2B);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V3, tfm_BS_P2fwd_V3);
      geo_BS_P2fwd_Base = new G4SubtractionSolid("geo_BS_P2fwd_Base", geo_BS_P2fwd_Base, geo_BS_P2fwd_V4, tfm_BS_P2fwd_V4);


      //++++ Assemble and place the FWD bellows shield ++++//
      G4Transform3D tfm_BS_fwdP1 = G4Translate3D(0, 0, BS_fwdZ2 - BS_fwdZ0);
      G4Transform3D tfm_BS_fwdP2 = G4Translate3D(0, 0, BS_fwdZ3 - BS_fwdZ0);

      G4VSolid* geo_BS_fwd = geo_BS_fwdP0_Base;
      geo_BS_fwd = new G4UnionSolid("geo_BS_fwd", geo_BS_fwd, geo_BS_P1fwd_Base, tfm_BS_fwdP1);
      geo_BS_fwd = new G4UnionSolid("geo_BS_fwd", geo_BS_fwd, geo_BS_P2fwd_Base, tfm_BS_fwdP2);

      G4LogicalVolume* logi_BS_fwd = new G4LogicalVolume(geo_BS_fwd, mat_BS, "logi_BS_fwd");

      G4Transform3D tfm_BS_fwd = G4Translate3D(0, 0, BS_fwdZ0);
      new G4PVPlacement(tfm_BS_fwd, logi_BS_fwd, "phys_BS_fwd", &topVolume, false, 0);
      //++++ +++++++++++++++++++++++++++++++++++++++++++ ++++//

      //++++ Assemble and place the BWD bellows shield ++++//
      G4Transform3D tfm_BS_bwdP1 = G4Translate3D(0, 0, BS_bwdZ2 - BS_bwdZ0) * G4RotateY3D(M_PI);

      G4VSolid* geo_BS_bwd = geo_BS_bwdP0_Base;
      geo_BS_bwd = new G4UnionSolid("geo_BS_bwd", geo_BS_bwd, geo_BS_P1bwd_Base, tfm_BS_bwdP1);

      G4LogicalVolume* logi_BS_bwd = new G4LogicalVolume(geo_BS_bwd, mat_BS, "logi_BS_bwd");

      G4Transform3D tfm_BS_bwd = G4Translate3D(0, 0, BS_bwdZ0);
      new G4PVPlacement(tfm_BS_bwd, logi_BS_bwd, "phys_BS_bwd", &topVolume, false, 0);
      //++++ +++++++++++++++++++++++++++++++++++++++++++ ++++//


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

      if (m_config.getParameter("polyBlock-R-L1.L", -1) > 0) {
        // earlyPhase3 = false; post LS1

        std::vector<std::string> polyBlocks;
        boost::split(polyBlocks, m_config.getParameterStr("PolyBlock"), boost::is_any_of(" "));
        for (const auto& name : polyBlocks) {
          prep = name + ".";
          //string type = m_config.getParameterStr(prep + "type");

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

            if (i == block_cut_N) {
              geo_block_name = "geo_" + name + "_name";
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
          setColor(*logi_block, "#00CC00");
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
            if (i == layer_cut_N) {
              geo_layer_name = "geo_" + name + "_name";
            } else {
              geo_layer_name = "geo_" + name + "_x" + oss_block_num.str() + "_name";
            }
            string geo_cut_name = "geo_" + name + "_cut" + oss_block_num.str() + "_name";

            G4VSolid* geo_cut;

            if (cut_type == 0.0) {
              double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
              double cut_W = m_config.getParameter(prep + "cutW" + oss_block_num.str()) * unitFactor;
              double cut_H = m_config.getParameter(prep + "cutH" + oss_block_num.str()) * unitFactor;

              geo_cut = new G4Box(geo_cut_name, cut_W / 2.0, cut_H / 2.0, cut_L / 2.0);
            } else {
              double cut_L = m_config.getParameter(prep + "cutL" + oss_block_num.str()) * unitFactor;
              double cut_R = m_config.getParameter(prep + "cutR" + oss_block_num.str()) * unitFactor;

              geo_cut = new G4Tubs(geo_cut_name, 0.0, cut_R, cut_L / 2.0, 0.0, 2.0 * M_PI);
            }

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
          setColor(*logi_layer, "#B100CC");
          //setVisibility(*logi_layer, false);

          string phys_layer_name = "phys_" + name + "_name";
          new G4PVPlacement(layer.transform, layer.logi, phys_layer_name, &topVolume, false, 0);

          elements[name] = layer;
        }
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
    }
  }
}
