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

    void GeoCryostatCreator::createGeometry(G4LogicalVolume& topVolume, GeometryTypes)
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
      //             +- C5wal4
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
      int flag_limitStep = int(m_config.getParameter("LimitStepLength"));

      const double unitFactor = Unit::cm / Unit::mm;

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
      setVisibility(*C1wal1.logi, false);
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
      setVisibility(*F1wal1.logi, false);
      new G4PVPlacement(F1wal1.transform, F1wal1.logi, "phys_F1wal1_name", &topVolume, false, 0);

      elements["F1wal1"] = F1wal1;


      //--------------
      //-   Rest of elements with typical geometry
      for (std::pair<std::string, std::string> element : m_config.getParametersStr()) {

        if (element.first != "Straight") continue;

        CryostatElement polycone;

        string name = element.second;
        prep = name + ".";
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

        G4VSolid* geo_polyconexx, *geo_polyconex, *geo_polycone;

        if (subtract != "" && intersect != "") {
          geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, &(Z[0]), &(r[0]), &(R[0]));
          geo_polyconex = new G4SubtractionSolid(geo_polyconex_name, geo_polyconexx, elements[subtract].geo,
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

      // RVC connection structure (simplified shape)
      G4Tubs* geo_rvcR = new G4Tubs("geo_rvcR", 60, 60 + 60, (620 - 560) / 2., 0, 2 * M_PI);
      G4LogicalVolume* logi_rvcR = new G4LogicalVolume(geo_rvcR, Materials::get("SUS316L"), "logi_rvcR_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, (620 + 560) / 2.), logi_rvcR, "phys_rvcR_name", &topVolume, false, 0);

      G4Tubs* geo_rvcL = new G4Tubs("geo_rvcL", 60, 60 + 60, (-560 - (-620)) / 2., 0, 2 * M_PI);
      G4LogicalVolume* logi_rvcL = new G4LogicalVolume(geo_rvcL, Materials::get("SUS316L"), "logi_rvcL_name");
      new G4PVPlacement(0, G4ThreeVector(0, 0, (-620 - 560) / 2.), logi_rvcL, "phys_rvcL_name", &topVolume, false, 0);

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
