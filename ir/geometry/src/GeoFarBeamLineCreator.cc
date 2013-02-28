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

#include <ir/geometry/GeoFarBeamLineCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <ir/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

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
#include <G4Torus.hh>
#include <G4Polycone.hh>
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

    geometry::CreatorFactory<GeoFarBeamLineCreator> GeoFarBeamLineFactory("FarBeamLineCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoFarBeamLineCreator::GeoFarBeamLineCreator()
    {
      m_sensitive = new SensitiveDetector();
    }

    GeoFarBeamLineCreator::~GeoFarBeamLineCreator()
    {
      delete m_sensitive;
    }

    void GeoFarBeamLineCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes)
    {

      const int N = 2;

      double stepMax = 5.0 * Unit::mm;
      //bool flag_limitStep = true;
      bool flag_limitStep = false;

      double unitFactor = 10.0;

      //--------------
      //-   limits

      //--------------
      //-   TubeR

      //get parameters from .xml file
      GearDir cTubeR(content, "TubeR/");

      double TubeR_Z[N];
      double TubeR_R[N];
      double TubeR_r[N];
      TubeR_Z[0] = cTubeR.getLength("Z0") * unitFactor;
      TubeR_Z[1] = cTubeR.getLength("Z1") * unitFactor;
      TubeR_R[0] = cTubeR.getLength("R") * unitFactor;
      TubeR_R[1] = cTubeR.getLength("R") * unitFactor;
      TubeR_r[0] = 0;
      TubeR_r[1] = 0;

      //define geometry
      G4Polycone* geo_TubeR = new G4Polycone("geo_TubeRxx_name", 0.0, 2 * M_PI, N, TubeR_Z, TubeR_r, TubeR_R);

      //--------------
      //-   TubeL

      //get parameters from .xml file
      GearDir cTubeL(content, "TubeL/");

      double TubeL_Z[N];
      double TubeL_R[N];
      double TubeL_r[N];
      TubeL_Z[0] = cTubeL.getLength("Z0") * unitFactor;
      TubeL_Z[1] = cTubeL.getLength("Z1") * unitFactor;
      TubeL_R[0] = cTubeL.getLength("R") * unitFactor;
      TubeL_R[1] = cTubeL.getLength("R") * unitFactor;
      TubeL_r[0] = 0;
      TubeL_r[1] = 0;

      //define geometry
      G4Polycone* geo_TubeL = new G4Polycone("geo_TubeLxx_name", 0.0, 2 * M_PI, N, TubeL_Z, TubeL_r, TubeL_R);


      //--------------
      //-   beam pipe

      //--------------
      //-   LHR1

      //get parameters from .xml file
      GearDir cLHR1(content, "LHR1/");

      double LHR1_Z[N];
      double LHR1_R[N];
      double LHR1_r[N];
      LHR1_Z[0] = 0;
      LHR1_Z[1] = cLHR1.getLength("L") * unitFactor;
      LHR1_R[0] = cLHR1.getLength("R") * unitFactor;
      LHR1_R[1] = cLHR1.getLength("R") * unitFactor;
      LHR1_r[0] = cLHR1.getLength("r") * unitFactor;
      LHR1_r[1] = cLHR1.getLength("r") * unitFactor;

      double LHR1_X0 = cLHR1.getLength("X0") * unitFactor;
      double LHR1_Z0 = cLHR1.getLength("Z0") * unitFactor;
      double LHR1_PHI = cLHR1.getLength("PHI");
      G4Transform3D transform_LHR1 = G4Translate3D(LHR1_X0, 0.0, LHR1_Z0);
      transform_LHR1 = transform_LHR1 * G4RotateY3D(LHR1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LHR1xx = new G4Polycone("geo_LHR1xx_name", 0.0, 2 * M_PI, N, LHR1_Z, LHR1_r, LHR1_R);
      G4IntersectionSolid* geo_LHR1 = new G4IntersectionSolid("geo_LHR1_name", geo_LHR1xx, geo_TubeR, transform_LHR1.inverse());

      string strMat_LHR1 = cLHR1.getString("Material");
      G4Material* mat_LHR1 = Materials::get(strMat_LHR1);
      G4LogicalVolume* logi_LHR1 = new G4LogicalVolume(geo_LHR1, mat_LHR1, "logi_LHR1_name");

      //put volume
      setColor(*logi_LHR1, cLHR1.getString("Color", "#CC0000"));
      //setVisibility(*logi_LHR1, false);
      new G4PVPlacement(transform_LHR1, logi_LHR1, "phys_LHR1_name", &topVolume, false, 0);

      //--------------
      //-   BLC2REtube

      //get parameters from .xml file
      GearDir cBLC2REtube(content, "BLC2REtube/");

      double BLC2REtube_r = cBLC2REtube.getLength("r") * unitFactor;
      double BLC2REtube_R = cBLC2REtube.getLength("R") * unitFactor;
      double BLC2REtube_RT = cBLC2REtube.getLength("RT") * unitFactor;
      double BLC2REtube_X0 = cBLC2REtube.getLength("X0") * unitFactor;
      double BLC2REtube_Z0 = cBLC2REtube.getLength("Z0") * unitFactor;
      double BLC2REtube_SPHI = cBLC2REtube.getLength("SPHI");
      double BLC2REtube_DPHI = cBLC2REtube.getLength("DPHI");

      G4Transform3D transform_BLC2REtube = G4Translate3D(BLC2REtube_X0, 0.0, BLC2REtube_Z0);
      transform_BLC2REtube = transform_BLC2REtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLC2REtubexx = new G4Torus("geo_BLC2REtubexx_name", BLC2REtube_r, BLC2REtube_R, BLC2REtube_RT, BLC2REtube_SPHI, BLC2REtube_DPHI);
      G4SubtractionSolid* geo_BLC2REtube = new G4SubtractionSolid("geo_BLC2REtube_name", geo_BLC2REtubexx, geo_LHR1, transform_BLC2REtube.inverse()*transform_LHR1);

      string strMat_BLC2REtube = cBLC2REtube.getString("Material");
      G4Material* mat_BLC2REtube = Materials::get(strMat_BLC2REtube);
      G4LogicalVolume* logi_BLC2REtube = new G4LogicalVolume(geo_BLC2REtube, mat_BLC2REtube, "logi_BLC2REtube_name");

      //put volume
      setColor(*logi_BLC2REtube, cBLC2REtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLC2REtube, false);
      new G4PVPlacement(transform_BLC2REtube, logi_BLC2REtube, "phys_BLC2REtube_name", &topVolume, false, 0);

      //--------------
      //-   LHR2

      //get parameters from .xml file
      GearDir cLHR2(content, "LHR2/");

      double LHR2_Z[N];
      double LHR2_R[N];
      double LHR2_r[N];
      LHR2_Z[0] = 0;
      LHR2_Z[1] = cLHR2.getLength("L") * unitFactor;
      LHR2_R[0] = cLHR2.getLength("R") * unitFactor;
      LHR2_R[1] = cLHR2.getLength("R") * unitFactor;
      LHR2_r[0] = cLHR2.getLength("r") * unitFactor;
      LHR2_r[1] = cLHR2.getLength("r") * unitFactor;

      double LHR2_X0 = cLHR2.getLength("X0") * unitFactor;
      double LHR2_Z0 = cLHR2.getLength("Z0") * unitFactor;
      double LHR2_PHI = cLHR2.getLength("PHI");
      G4Transform3D transform_LHR2 = G4Translate3D(LHR2_X0, 0.0, LHR2_Z0);
      transform_LHR2 = transform_LHR2 * G4RotateY3D(LHR2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LHR2xx = new G4Polycone("geo_LHR2xx_name", 0.0, 2 * M_PI, N, LHR2_Z, LHR2_r, LHR2_R);
      G4IntersectionSolid* geo_LHR2 = new G4IntersectionSolid("geo_LHR2x_name", geo_LHR2xx, geo_TubeR, transform_LHR2.inverse());
      //G4SubtractionSolid* geo_LHR2 = new G4SubtractionSolid("geo_LHR2_name", geo_LHR2x, geo_BLC2REtube, transform_LHR2.inverse()*transform_BLC2REtube);

      string strMat_LHR2 = cLHR2.getString("Material");
      G4Material* mat_LHR2 = Materials::get(strMat_LHR2);
      G4LogicalVolume* logi_LHR2 = new G4LogicalVolume(geo_LHR2, mat_LHR2, "logi_LHR2_name");

      //put volume
      setColor(*logi_LHR2, cLHR2.getString("Color", "#CC0000"));
      //setVisibility(*logi_LHR2, false);
      new G4PVPlacement(transform_LHR2, logi_LHR2, "phys_LHR2_name", &topVolume, false, 0);

      //--------------
      //-   LLR1

      //get parameters from .xml file
      GearDir cLLR1(content, "LLR1/");

      double LLR1_Z[N];
      double LLR1_R[N];
      double LLR1_r[N];
      LLR1_Z[0] = 0;
      LLR1_Z[1] = cLLR1.getLength("L") * unitFactor;
      LLR1_R[0] = cLLR1.getLength("R") * unitFactor;
      LLR1_R[1] = cLLR1.getLength("R") * unitFactor;
      LLR1_r[0] = cLLR1.getLength("r") * unitFactor;
      LLR1_r[1] = cLLR1.getLength("r") * unitFactor;

      double LLR1_X0 = cLLR1.getLength("X0") * unitFactor;
      double LLR1_Z0 = cLLR1.getLength("Z0") * unitFactor;
      double LLR1_PHI = cLLR1.getLength("PHI");
      G4Transform3D transform_LLR1 = G4Translate3D(LLR1_X0, 0.0, LLR1_Z0);
      transform_LLR1 = transform_LLR1 * G4RotateY3D(LLR1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLR1xx = new G4Polycone("geo_LLR1xx_name", 0.0, 2 * M_PI, N, LLR1_Z, LLR1_r, LLR1_R);
      G4IntersectionSolid* geo_LLR1 = new G4IntersectionSolid("geo_LLR1_name", geo_LLR1xx, geo_TubeR, transform_LLR1.inverse());

      string strMat_LLR1 = cLLR1.getString("Material");
      G4Material* mat_LLR1 = Materials::get(strMat_LLR1);
      G4LogicalVolume* logi_LLR1 = new G4LogicalVolume(geo_LLR1, mat_LLR1, "logi_LLR1_name");

      //put volume
      setColor(*logi_LLR1, cLLR1.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLR1, false);
      new G4PVPlacement(transform_LLR1, logi_LLR1, "phys_LLR1_name", &topVolume, false, 0);

      //--------------
      //-   BC1RPtube

      //get parameters from .xml file
      GearDir cBC1RPtube(content, "BC1RPtube/");

      double BC1RPtube_r = cBC1RPtube.getLength("r") * unitFactor;
      double BC1RPtube_R = cBC1RPtube.getLength("R") * unitFactor;
      double BC1RPtube_RT = cBC1RPtube.getLength("RT") * unitFactor;
      double BC1RPtube_X0 = cBC1RPtube.getLength("X0") * unitFactor;
      double BC1RPtube_Z0 = cBC1RPtube.getLength("Z0") * unitFactor;
      double BC1RPtube_SPHI = cBC1RPtube.getLength("SPHI");
      double BC1RPtube_DPHI = cBC1RPtube.getLength("DPHI");

      G4Transform3D transform_BC1RPtube = G4Translate3D(BC1RPtube_X0, 0.0, BC1RPtube_Z0);
      transform_BC1RPtube = transform_BC1RPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BC1RPtube = new G4Torus("geo_BC1RPtubexx_name", BC1RPtube_r, BC1RPtube_R, BC1RPtube_RT, BC1RPtube_SPHI, BC1RPtube_DPHI);
      //G4SubtractionSolid* geo_BC1RPtube = new G4SubtractionSolid("geo_BC1RPtube_name", geo_BC1RPtubexx, geo_LLR1, transform_BC1RPtube.inverse()*transform_LLR1);

      string strMat_BC1RPtube = cBC1RPtube.getString("Material");
      G4Material* mat_BC1RPtube = Materials::get(strMat_BC1RPtube);
      G4LogicalVolume* logi_BC1RPtube = new G4LogicalVolume(geo_BC1RPtube, mat_BC1RPtube, "logi_BC1RPtube_name");

      //put volume
      setColor(*logi_BC1RPtube, cBC1RPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BC1RPtube, false);
      new G4PVPlacement(transform_BC1RPtube, logi_BC1RPtube, "phys_BC1RPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLR2

      //get parameters from .xml file
      GearDir cLLR2(content, "LLR2/");

      double LLR2_Z[N];
      double LLR2_R[N];
      double LLR2_r[N];
      LLR2_Z[0] = 0;
      LLR2_Z[1] = cLLR2.getLength("L") * unitFactor;
      LLR2_R[0] = cLLR2.getLength("R") * unitFactor;
      LLR2_R[1] = cLLR2.getLength("R") * unitFactor;
      LLR2_r[0] = cLLR2.getLength("r") * unitFactor;
      LLR2_r[1] = cLLR2.getLength("r") * unitFactor;

      double LLR2_X0 = cLLR2.getLength("X0") * unitFactor;
      double LLR2_Z0 = cLLR2.getLength("Z0") * unitFactor;
      double LLR2_PHI = cLLR2.getLength("PHI");
      G4Transform3D transform_LLR2 = G4Translate3D(LLR2_X0, 0.0, LLR2_Z0);
      transform_LLR2 = transform_LLR2 * G4RotateY3D(LLR2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLR2xx = new G4Polycone("geo_LLR2xx_name", 0.0, 2 * M_PI, N, LLR2_Z, LLR2_r, LLR2_R);
      G4SubtractionSolid* geo_LLR2 = new G4SubtractionSolid("geo_LLR2_name", geo_LLR2xx, geo_BC1RPtube, transform_LLR2.inverse()*transform_BC1RPtube);

      string strMat_LLR2 = cLLR2.getString("Material");
      G4Material* mat_LLR2 = Materials::get(strMat_LLR2);
      G4LogicalVolume* logi_LLR2 = new G4LogicalVolume(geo_LLR2, mat_LLR2, "logi_LLR2_name");

      //put volume
      setColor(*logi_LLR2, cLLR2.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLR2, false);
      new G4PVPlacement(transform_LLR2, logi_LLR2, "phys_LLR2_name", &topVolume, false, 0);

      //--------------
      //-   BLCWRPtube

      //get parameters from .xml file
      GearDir cBLCWRPtube(content, "BLCWRPtube/");

      double BLCWRPtube_r = cBLCWRPtube.getLength("r") * unitFactor;
      double BLCWRPtube_R = cBLCWRPtube.getLength("R") * unitFactor;
      double BLCWRPtube_RT = cBLCWRPtube.getLength("RT") * unitFactor;
      double BLCWRPtube_X0 = cBLCWRPtube.getLength("X0") * unitFactor;
      double BLCWRPtube_Z0 = cBLCWRPtube.getLength("Z0") * unitFactor;
      double BLCWRPtube_SPHI = cBLCWRPtube.getLength("SPHI");
      double BLCWRPtube_DPHI = cBLCWRPtube.getLength("DPHI");

      G4Transform3D transform_BLCWRPtube = G4Translate3D(BLCWRPtube_X0, 0.0, BLCWRPtube_Z0);
      transform_BLCWRPtube = transform_BLCWRPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLCWRPtube = new G4Torus("geo_BLCWRPtubexx_name", BLCWRPtube_r, BLCWRPtube_R, BLCWRPtube_RT, BLCWRPtube_SPHI, BLCWRPtube_DPHI);
      //G4SubtractionSolid* geo_BLCWRPtube = new G4SubtractionSolid("geo_BLCWRPtube_name", geo_BLCWRPtubexx, geo_LLR2, transform_BLCWRPtube.inverse()*transform_LLR2);

      string strMat_BLCWRPtube = cBLCWRPtube.getString("Material");
      G4Material* mat_BLCWRPtube = Materials::get(strMat_BLCWRPtube);
      G4LogicalVolume* logi_BLCWRPtube = new G4LogicalVolume(geo_BLCWRPtube, mat_BLCWRPtube, "logi_BLCWRPtube_name");

      //put volume
      setColor(*logi_BLCWRPtube, cBLCWRPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLCWRPtube, false);
      new G4PVPlacement(transform_BLCWRPtube, logi_BLCWRPtube, "phys_BLCWRPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLR3

      //get parameters from .xml file
      GearDir cLLR3(content, "LLR3/");

      double LLR3_Z[N];
      double LLR3_R[N];
      double LLR3_r[N];
      LLR3_Z[0] = 0;
      LLR3_Z[1] = cLLR3.getLength("L") * unitFactor;
      LLR3_R[0] = cLLR3.getLength("R") * unitFactor;
      LLR3_R[1] = cLLR3.getLength("R") * unitFactor;
      LLR3_r[0] = cLLR3.getLength("r") * unitFactor;
      LLR3_r[1] = cLLR3.getLength("r") * unitFactor;

      double LLR3_X0 = cLLR3.getLength("X0") * unitFactor;
      double LLR3_Z0 = cLLR3.getLength("Z0") * unitFactor;
      double LLR3_PHI = cLLR3.getLength("PHI");
      G4Transform3D transform_LLR3 = G4Translate3D(LLR3_X0, 0.0, LLR3_Z0);
      transform_LLR3 = transform_LLR3 * G4RotateY3D(LLR3_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLR3 = new G4Polycone("geo_LLR3xx_name", 0.0, 2 * M_PI, N, LLR3_Z, LLR3_r, LLR3_R);
      //G4SubtractionSolid* geo_LLR3 = new G4SubtractionSolid("geo_LLR3_name", geo_LLR3xx, geo_BLCWRPtube, transform_LLR3.inverse()*transform_BLCWRPtube);

      string strMat_LLR3 = cLLR3.getString("Material");
      G4Material* mat_LLR3 = Materials::get(strMat_LLR3);
      G4LogicalVolume* logi_LLR3 = new G4LogicalVolume(geo_LLR3, mat_LLR3, "logi_LLR3_name");

      //put volume
      setColor(*logi_LLR3, cLLR3.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLR3, false);
      new G4PVPlacement(transform_LLR3, logi_LLR3, "phys_LLR3_name", &topVolume, false, 0);

      //--------------
      //-   BLC1RPtube

      //get parameters from .xml file
      GearDir cBLC1RPtube(content, "BLC1RPtube/");

      //double BLC1RPtube_r = cBLC1RPtube.getLength("r") * unitFactor;
      double BLC1RPtube_r = 0; //movable collimator
      double BLC1RPtube_R = cBLC1RPtube.getLength("R") * unitFactor;
      double BLC1RPtube_RT = cBLC1RPtube.getLength("RT") * unitFactor;
      double BLC1RPtube_X0 = cBLC1RPtube.getLength("X0") * unitFactor;
      double BLC1RPtube_Z0 = cBLC1RPtube.getLength("Z0") * unitFactor;
      double BLC1RPtube_SPHI = cBLC1RPtube.getLength("SPHI");
      double BLC1RPtube_DPHI = cBLC1RPtube.getLength("DPHI");

      G4Transform3D transform_BLC1RPtube = G4Translate3D(BLC1RPtube_X0, 0.0, BLC1RPtube_Z0);
      transform_BLC1RPtube = transform_BLC1RPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLC1RPtubexx = new G4Torus("geo_BLC1RPtubexx_name", BLC1RPtube_r, BLC1RPtube_R, BLC1RPtube_RT, BLC1RPtube_SPHI, BLC1RPtube_DPHI);
      G4SubtractionSolid* geo_BLC1RPtube = new G4SubtractionSolid("geo_BLC1RPtube", geo_BLC1RPtubexx, geo_LLR3, transform_BLC1RPtube.inverse()*transform_LLR3);

      string strMat_BLC1RPtube = cBLC1RPtube.getString("Material");
      G4Material* mat_BLC1RPtube = Materials::get(strMat_BLC1RPtube);
      G4LogicalVolume* logi_BLC1RPtube = new G4LogicalVolume(geo_BLC1RPtube, mat_BLC1RPtube, "logi_BLC1RPtube_name");

      //put volume
      setColor(*logi_BLC1RPtube, cBLC1RPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLC1RPtube, false);
      new G4PVPlacement(transform_BLC1RPtube, logi_BLC1RPtube, "phys_BLC1RPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLR4

      //get parameters from .xml file
      GearDir cLLR4(content, "LLR4/");

      double LLR4_Z[N];
      double LLR4_R[N];
      double LLR4_r[N];
      LLR4_Z[0] = 0;
      LLR4_Z[1] = cLLR4.getLength("L") * unitFactor;
      LLR4_R[0] = cLLR4.getLength("R") * unitFactor;
      LLR4_R[1] = cLLR4.getLength("R") * unitFactor;
      LLR4_r[0] = cLLR4.getLength("r") * unitFactor;
      LLR4_r[1] = cLLR4.getLength("r") * unitFactor;

      double LLR4_X0 = cLLR4.getLength("X0") * unitFactor;
      double LLR4_Z0 = cLLR4.getLength("Z0") * unitFactor;
      double LLR4_PHI = cLLR4.getLength("PHI");
      G4Transform3D transform_LLR4 = G4Translate3D(LLR4_X0, 0.0, LLR4_Z0);
      transform_LLR4 = transform_LLR4 * G4RotateY3D(LLR4_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLR4 = new G4Polycone("geo_LLR4xx_name", 0.0, 2 * M_PI, N, LLR4_Z, LLR4_r, LLR4_R);
      //G4SubtractionSolid* geo_LLR4 = new G4SubtractionSolid("geo_LLR4_name", geo_LLR4xx, geo_BLC1RPtube, transform_LLR4.inverse()*transform_BLC1RPtube);

      string strMat_LLR4 = cLLR4.getString("Material");
      G4Material* mat_LLR4 = Materials::get(strMat_LLR4);
      G4LogicalVolume* logi_LLR4 = new G4LogicalVolume(geo_LLR4, mat_LLR4, "logi_LLR4_name");

      //put volume
      setColor(*logi_LLR4, cLLR4.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLR4, false);
      new G4PVPlacement(transform_LLR4, logi_LLR4, "phys_LLR4_name", &topVolume, false, 0);

      //--------------
      //-   BLC2RPtube

      //get parameters from .xml file
      GearDir cBLC2RPtube(content, "BLC2RPtube/");

      double BLC2RPtube_r = cBLC2RPtube.getLength("r") * unitFactor;
      double BLC2RPtube_R = cBLC2RPtube.getLength("R") * unitFactor;
      double BLC2RPtube_RT = cBLC2RPtube.getLength("RT") * unitFactor;
      double BLC2RPtube_X0 = cBLC2RPtube.getLength("X0") * unitFactor;
      double BLC2RPtube_Z0 = cBLC2RPtube.getLength("Z0") * unitFactor;
      double BLC2RPtube_SPHI = cBLC2RPtube.getLength("SPHI");
      double BLC2RPtube_DPHI = cBLC2RPtube.getLength("DPHI");

      G4Transform3D transform_BLC2RPtube = G4Translate3D(BLC2RPtube_X0, 0.0, BLC2RPtube_Z0);
      transform_BLC2RPtube = transform_BLC2RPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLC2RPtube = new G4Torus("geo_BLC2RPtubexx_name", BLC2RPtube_r, BLC2RPtube_R, BLC2RPtube_RT, BLC2RPtube_SPHI, BLC2RPtube_DPHI);

      string strMat_BLC2RPtube = cBLC2RPtube.getString("Material");
      G4Material* mat_BLC2RPtube = Materials::get(strMat_BLC2RPtube);
      G4LogicalVolume* logi_BLC2RPtube = new G4LogicalVolume(geo_BLC2RPtube, mat_BLC2RPtube, "logi_BLC2RPtube_name");

      //put volume
      setColor(*logi_BLC2RPtube, cBLC2RPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLC2RPtube, false);
      new G4PVPlacement(transform_BLC2RPtube, logi_BLC2RPtube, "phys_BLC2RPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLR5

      //get parameters from .xml file
      GearDir cLLR5(content, "LLR5/");

      double LLR5_Z[N];
      double LLR5_R[N];
      double LLR5_r[N];
      LLR5_Z[0] = 0;
      LLR5_Z[1] = cLLR5.getLength("L") * unitFactor;
      LLR5_R[0] = cLLR5.getLength("R") * unitFactor;
      LLR5_R[1] = cLLR5.getLength("R") * unitFactor;
      LLR5_r[0] = cLLR5.getLength("r") * unitFactor;
      LLR5_r[1] = cLLR5.getLength("r") * unitFactor;

      double LLR5_X0 = cLLR5.getLength("X0") * unitFactor;
      double LLR5_Z0 = cLLR5.getLength("Z0") * unitFactor;
      double LLR5_PHI = cLLR5.getLength("PHI");
      G4Transform3D transform_LLR5 = G4Translate3D(LLR5_X0, 0.0, LLR5_Z0);
      transform_LLR5 = transform_LLR5 * G4RotateY3D(LLR5_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLR5xx = new G4Polycone("geo_LLR5xx_name", 0.0, 2 * M_PI, N, LLR5_Z, LLR5_r, LLR5_R);
      G4IntersectionSolid* geo_LLR5x = new G4IntersectionSolid("geo_LLR5_namex", geo_LLR5xx, geo_TubeR, transform_LLR5.inverse());
      G4SubtractionSolid* geo_LLR5 = new G4SubtractionSolid("geo_LLR5_name", geo_LLR5x, geo_BLC2RPtube, transform_LLR5.inverse()*transform_BLC2RPtube);

      string strMat_LLR5 = cLLR5.getString("Material");
      G4Material* mat_LLR5 = Materials::get(strMat_LLR5);
      G4LogicalVolume* logi_LLR5 = new G4LogicalVolume(geo_LLR5, mat_LLR5, "logi_LLR5_name");

      //put volume
      setColor(*logi_LLR5, cLLR5.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLR5, false);
      new G4PVPlacement(transform_LLR5, logi_LLR5, "phys_LLR5_name", &topVolume, false, 0);

      //--------------
      //-   LHL1

      //get parameters from .xml file
      GearDir cLHL1(content, "LHL1/");

      double LHL1_Z[N];
      double LHL1_R[N];
      double LHL1_r[N];
      LHL1_Z[0] = 0;
      LHL1_Z[1] = cLHL1.getLength("L") * unitFactor;
      LHL1_R[0] = cLHL1.getLength("R") * unitFactor;
      LHL1_R[1] = cLHL1.getLength("R") * unitFactor;
      LHL1_r[0] = cLHL1.getLength("r") * unitFactor;
      LHL1_r[1] = cLHL1.getLength("r") * unitFactor;

      double LHL1_X0 = cLHL1.getLength("X0") * unitFactor;
      double LHL1_Z0 = cLHL1.getLength("Z0") * unitFactor;
      double LHL1_PHI = cLHL1.getLength("PHI");
      G4Transform3D transform_LHL1 = G4Translate3D(LHL1_X0, 0.0, LHL1_Z0);
      transform_LHL1 = transform_LHL1 * G4RotateY3D(LHL1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LHL1xx = new G4Polycone("geo_LHL1xx_name", 0.0, 2 * M_PI, N, LHL1_Z, LHL1_r, LHL1_R);
      G4IntersectionSolid* geo_LHL1 = new G4IntersectionSolid("geo_LHL1_name", geo_LHL1xx, geo_TubeL, transform_LHL1.inverse());

      string strMat_LHL1 = cLHL1.getString("Material");
      G4Material* mat_LHL1 = Materials::get(strMat_LHL1);
      G4LogicalVolume* logi_LHL1 = new G4LogicalVolume(geo_LHL1, mat_LHL1, "logi_LHL1_name");

      //put volume
      setColor(*logi_LHL1, cLHL1.getString("Color", "#CC0000"));
      //setVisibility(*logi_LHL1, false);
      new G4PVPlacement(transform_LHL1, logi_LHL1, "phys_LHL1_name", &topVolume, false, 0);

      //--------------
      //-   BLC1LEtube

      //get parameters from .xml file
      GearDir cBLC1LEtube(content, "BLC1LEtube/");

      double BLC1LEtube_r = cBLC1LEtube.getLength("r") * unitFactor;
      double BLC1LEtube_R = cBLC1LEtube.getLength("R") * unitFactor;
      double BLC1LEtube_RT = cBLC1LEtube.getLength("RT") * unitFactor;
      double BLC1LEtube_X0 = cBLC1LEtube.getLength("X0") * unitFactor;
      double BLC1LEtube_Z0 = cBLC1LEtube.getLength("Z0") * unitFactor;
      double BLC1LEtube_SPHI = cBLC1LEtube.getLength("SPHI");
      double BLC1LEtube_DPHI = cBLC1LEtube.getLength("DPHI");

      G4Transform3D transform_BLC1LEtube = G4Translate3D(BLC1LEtube_X0, 0.0, BLC1LEtube_Z0);
      transform_BLC1LEtube = transform_BLC1LEtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLC1LEtubexx = new G4Torus("geo_BLC1LEtubexx_name", BLC1LEtube_r, BLC1LEtube_R, BLC1LEtube_RT, BLC1LEtube_SPHI, BLC1LEtube_DPHI);
      G4SubtractionSolid* geo_BLC1LEtube = new G4SubtractionSolid("geo_BLC1LEtube_name", geo_BLC1LEtubexx, geo_LHL1, transform_BLC1LEtube.inverse()*transform_LHL1);

      string strMat_BLC1LEtube = cBLC1LEtube.getString("Material");
      G4Material* mat_BLC1LEtube = Materials::get(strMat_BLC1LEtube);
      G4LogicalVolume* logi_BLC1LEtube = new G4LogicalVolume(geo_BLC1LEtube, mat_BLC1LEtube, "logi_BLC1LEtube_name");

      //put volume
      setColor(*logi_BLC1LEtube, cBLC1LEtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLC1LEtube, false);
      new G4PVPlacement(transform_BLC1LEtube, logi_BLC1LEtube, "phys_BLC1LEtube_name", &topVolume, false, 0);

      //--------------
      //-   LHL2

      //get parameters from .xml file
      GearDir cLHL2(content, "LHL2/");

      double LHL2_Z[N];
      double LHL2_R[N];
      double LHL2_r[N];
      LHL2_Z[0] = 0;
      LHL2_Z[1] = cLHL2.getLength("L") * unitFactor;
      LHL2_R[0] = cLHL2.getLength("R") * unitFactor;
      LHL2_R[1] = cLHL2.getLength("R") * unitFactor;
      //LHL2_r[0] = cLHL2.getLength("r") * unitFactor;
      //LHL2_r[1] = cLHL2.getLength("r") * unitFactor;
      LHL2_r[0] = 0; //movable collimator
      LHL2_r[1] = 0; //movable collimator

      double LHL2_X0 = cLHL2.getLength("X0") * unitFactor;
      double LHL2_Z0 = cLHL2.getLength("Z0") * unitFactor;
      double LHL2_PHI = cLHL2.getLength("PHI");
      G4Transform3D transform_LHL2 = G4Translate3D(LHL2_X0, 0.0, LHL2_Z0);
      transform_LHL2 = transform_LHL2 * G4RotateY3D(LHL2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LHL2xx = new G4Polycone("geo_LHL2xx_name", 0.0, 2 * M_PI, N, LHL2_Z, LHL2_r, LHL2_R);
      G4IntersectionSolid* geo_LHL2 = new G4IntersectionSolid("geo_LHL2_name", geo_LHL2xx, geo_TubeL, transform_LHL2.inverse());

      string strMat_LHL2 = cLHL2.getString("Material");
      G4Material* mat_LHL2 = Materials::get(strMat_LHL2);
      G4LogicalVolume* logi_LHL2 = new G4LogicalVolume(geo_LHL2, mat_LHL2, "logi_LHL2_name");

      //put volume
      setColor(*logi_LHL2, cLHL2.getString("Color", "#CC0000"));
      //setVisibility(*logi_LHL2, false);
      new G4PVPlacement(transform_LHL2, logi_LHL2, "phys_LHL2_name", &topVolume, false, 0);

      //--------------
      //-   LLL1

      //get parameters from .xml file
      GearDir cLLL1(content, "LLL1/");

      double LLL1_Z[N];
      double LLL1_R[N];
      double LLL1_r[N];
      LLL1_Z[0] = 0;
      LLL1_Z[1] = cLLL1.getLength("L") * unitFactor;
      LLL1_R[0] = cLLL1.getLength("R") * unitFactor;
      LLL1_R[1] = cLLL1.getLength("R") * unitFactor;
      LLL1_r[0] = cLLL1.getLength("r") * unitFactor;
      LLL1_r[1] = cLLL1.getLength("r") * unitFactor;

      double LLL1_X0 = cLLL1.getLength("X0") * unitFactor;
      double LLL1_Z0 = cLLL1.getLength("Z0") * unitFactor;
      double LLL1_PHI = cLLL1.getLength("PHI");
      G4Transform3D transform_LLL1 = G4Translate3D(LLL1_X0, 0.0, LLL1_Z0);
      transform_LLL1 = transform_LLL1 * G4RotateY3D(LLL1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLL1xx = new G4Polycone("geo_LLL1xx_name", 0.0, 2 * M_PI, N, LLL1_Z, LLL1_r, LLL1_R);
      G4IntersectionSolid* geo_LLL1 = new G4IntersectionSolid("geo_LLL1_name", geo_LLL1xx, geo_TubeL, transform_LLL1.inverse());

      string strMat_LLL1 = cLLL1.getString("Material");
      G4Material* mat_LLL1 = Materials::get(strMat_LLL1);
      G4LogicalVolume* logi_LLL1 = new G4LogicalVolume(geo_LLL1, mat_LLL1, "logi_LLL1_name");

      //put volume
      setColor(*logi_LLL1, cLLL1.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLL1, false);
      new G4PVPlacement(transform_LLL1, logi_LLL1, "phys_LLL1_name", &topVolume, false, 0);

      //--------------
      //-   BC1LPtube

      //get parameters from .xml file
      GearDir cBC1LPtube(content, "BC1LPtube/");

      double BC1LPtube_r = cBC1LPtube.getLength("r") * unitFactor;
      double BC1LPtube_R = cBC1LPtube.getLength("R") * unitFactor;
      double BC1LPtube_RT = cBC1LPtube.getLength("RT") * unitFactor;
      double BC1LPtube_X0 = cBC1LPtube.getLength("X0") * unitFactor;
      double BC1LPtube_Z0 = cBC1LPtube.getLength("Z0") * unitFactor;
      double BC1LPtube_SPHI = cBC1LPtube.getLength("SPHI");
      double BC1LPtube_DPHI = cBC1LPtube.getLength("DPHI");

      G4Transform3D transform_BC1LPtube = G4Translate3D(BC1LPtube_X0, 0.0, BC1LPtube_Z0);
      transform_BC1LPtube = transform_BC1LPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BC1LPtube = new G4Torus("geo_BC1LPtubexx_name", BC1LPtube_r, BC1LPtube_R, BC1LPtube_RT, BC1LPtube_SPHI, BC1LPtube_DPHI);

      string strMat_BC1LPtube = cBC1LPtube.getString("Material");
      G4Material* mat_BC1LPtube = Materials::get(strMat_BC1LPtube);
      G4LogicalVolume* logi_BC1LPtube = new G4LogicalVolume(geo_BC1LPtube, mat_BC1LPtube, "logi_BC1LPtube_name");

      //put volume
      setColor(*logi_BC1LPtube, cBC1LPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BC1LPtube, false);
      new G4PVPlacement(transform_BC1LPtube, logi_BC1LPtube, "phys_BC1LPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLL2

      //get parameters from .xml file
      GearDir cLLL2(content, "LLL2/");

      double LLL2_Z[N];
      double LLL2_R[N];
      double LLL2_r[N];
      LLL2_Z[0] = 0;
      LLL2_Z[1] = cLLL2.getLength("L") * unitFactor;
      LLL2_R[0] = cLLL2.getLength("R") * unitFactor;
      LLL2_R[1] = cLLL2.getLength("R") * unitFactor;
      LLL2_r[0] = cLLL2.getLength("r") * unitFactor;
      LLL2_r[1] = cLLL2.getLength("r") * unitFactor;

      double LLL2_X0 = cLLL2.getLength("X0") * unitFactor;
      double LLL2_Z0 = cLLL2.getLength("Z0") * unitFactor;
      double LLL2_PHI = cLLL2.getLength("PHI");
      G4Transform3D transform_LLL2 = G4Translate3D(LLL2_X0, 0.0, LLL2_Z0);
      transform_LLL2 = transform_LLL2 * G4RotateY3D(LLL2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLL2xx = new G4Polycone("geo_LLL2xx_name", 0.0, 2 * M_PI, N, LLL2_Z, LLL2_r, LLL2_R);
      G4SubtractionSolid* geo_LLL2 = new G4SubtractionSolid("geo_LLL2_name", geo_LLL2xx, geo_BC1LPtube, transform_LLL2.inverse()*transform_BC1LPtube);

      string strMat_LLL2 = cLLL2.getString("Material");
      G4Material* mat_LLL2 = Materials::get(strMat_LLL2);
      G4LogicalVolume* logi_LLL2 = new G4LogicalVolume(geo_LLL2, mat_LLL2, "logi_LLL2_name");

      //put volume
      setColor(*logi_LLL2, cLLL2.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLL2, false);
      new G4PVPlacement(transform_LLL2, logi_LLL2, "phys_LLL2_name", &topVolume, false, 0);

      //--------------
      //-   BLC1LPtube

      //get parameters from .xml file
      GearDir cBLC1LPtube(content, "BLC1LPtube/");

      double BLC1LPtube_r = cBLC1LPtube.getLength("r") * unitFactor;
      double BLC1LPtube_R = cBLC1LPtube.getLength("R") * unitFactor;
      double BLC1LPtube_RT = cBLC1LPtube.getLength("RT") * unitFactor;
      double BLC1LPtube_X0 = cBLC1LPtube.getLength("X0") * unitFactor;
      double BLC1LPtube_Z0 = cBLC1LPtube.getLength("Z0") * unitFactor;
      double BLC1LPtube_SPHI = cBLC1LPtube.getLength("SPHI");
      double BLC1LPtube_DPHI = cBLC1LPtube.getLength("DPHI");

      G4Transform3D transform_BLC1LPtube = G4Translate3D(BLC1LPtube_X0, 0.0, BLC1LPtube_Z0);
      transform_BLC1LPtube = transform_BLC1LPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLC1LPtube = new G4Torus("geo_BLC1LPtubexx_name", BLC1LPtube_r, BLC1LPtube_R, BLC1LPtube_RT, BLC1LPtube_SPHI, BLC1LPtube_DPHI);

      string strMat_BLC1LPtube = cBLC1LPtube.getString("Material");
      G4Material* mat_BLC1LPtube = Materials::get(strMat_BLC1LPtube);
      G4LogicalVolume* logi_BLC1LPtube = new G4LogicalVolume(geo_BLC1LPtube, mat_BLC1LPtube, "logi_BLC1LPtube_name");

      //put volume
      setColor(*logi_BLC1LPtube, cBLC1LPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLC1LPtube, false);
      new G4PVPlacement(transform_BLC1LPtube, logi_BLC1LPtube, "phys_BLC1LPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLL3

      //get parameters from .xml file
      GearDir cLLL3(content, "LLL3/");

      double LLL3_Z[N];
      double LLL3_R[N];
      double LLL3_r[N];
      LLL3_Z[0] = 0;
      LLL3_Z[1] = cLLL3.getLength("L") * unitFactor;
      LLL3_R[0] = cLLL3.getLength("R") * unitFactor;
      LLL3_R[1] = cLLL3.getLength("R") * unitFactor;
      LLL3_r[0] = cLLL3.getLength("r") * unitFactor;
      LLL3_r[1] = cLLL3.getLength("r") * unitFactor;

      double LLL3_X0 = cLLL3.getLength("X0") * unitFactor;
      double LLL3_Z0 = cLLL3.getLength("Z0") * unitFactor;
      double LLL3_PHI = cLLL3.getLength("PHI");
      G4Transform3D transform_LLL3 = G4Translate3D(LLL3_X0, 0.0, LLL3_Z0);
      transform_LLL3 = transform_LLL3 * G4RotateY3D(LLL3_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLL3xx = new G4Polycone("geo_LLL3xx_name", 0.0, 2 * M_PI, N, LLL3_Z, LLL3_r, LLL3_R);
      G4SubtractionSolid* geo_LLL3 = new G4SubtractionSolid("geo_LLL3_name", geo_LLL3xx, geo_BLC1LPtube, transform_LLL3.inverse()*transform_BLC1LPtube);

      string strMat_LLL3 = cLLL3.getString("Material");
      G4Material* mat_LLL3 = Materials::get(strMat_LLL3);
      G4LogicalVolume* logi_LLL3 = new G4LogicalVolume(geo_LLL3, mat_LLL3, "logi_LLL3_name");

      //put volume
      setColor(*logi_LLL3, cLLL3.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLL3, false);
      new G4PVPlacement(transform_LLL3, logi_LLL3, "phys_LLL3_name", &topVolume, false, 0);

      //--------------
      //-   BLC2LPtube

      //get parameters from .xml file
      GearDir cBLC2LPtube(content, "BLC2LPtube/");

      double BLC2LPtube_r = cBLC2LPtube.getLength("r") * unitFactor;
      double BLC2LPtube_R = cBLC2LPtube.getLength("R") * unitFactor;
      double BLC2LPtube_RT = cBLC2LPtube.getLength("RT") * unitFactor;
      double BLC2LPtube_X0 = cBLC2LPtube.getLength("X0") * unitFactor;
      double BLC2LPtube_Z0 = cBLC2LPtube.getLength("Z0") * unitFactor;
      double BLC2LPtube_SPHI = cBLC2LPtube.getLength("SPHI");
      double BLC2LPtube_DPHI = cBLC2LPtube.getLength("DPHI");

      G4Transform3D transform_BLC2LPtube = G4Translate3D(BLC2LPtube_X0, 0.0, BLC2LPtube_Z0);
      transform_BLC2LPtube = transform_BLC2LPtube * G4RotateX3D(M_PI / 2 / Unit::rad);

      //define geometry
      G4Torus* geo_BLC2LPtubexx = new G4Torus("geo_BLC2LPtubexx_name", BLC2LPtube_r, BLC2LPtube_R, BLC2LPtube_RT, BLC2LPtube_SPHI, BLC2LPtube_DPHI);
      G4SubtractionSolid* geo_BLC2LPtube = new G4SubtractionSolid("geo_BLC2LPtube_name", geo_BLC2LPtubexx, geo_LLL3, transform_BLC2LPtube.inverse()*transform_LLL3);

      string strMat_BLC2LPtube = cBLC2LPtube.getString("Material");
      G4Material* mat_BLC2LPtube = Materials::get(strMat_BLC2LPtube);
      G4LogicalVolume* logi_BLC2LPtube = new G4LogicalVolume(geo_BLC2LPtube, mat_BLC2LPtube, "logi_BLC2LPtube_name");

      //put volume
      setColor(*logi_BLC2LPtube, cBLC2LPtube.getString("Color", "#00CC00"));
      //setVisibility(*logi_BLC2LPtube, false);
      new G4PVPlacement(transform_BLC2LPtube, logi_BLC2LPtube, "phys_BLC2LPtube_name", &topVolume, false, 0);

      //--------------
      //-   LLL4

      //get parameters from .xml file
      GearDir cLLL4(content, "LLL4/");

      double LLL4_Z[N];
      double LLL4_R[N];
      double LLL4_r[N];
      LLL4_Z[0] = 0;
      LLL4_Z[1] = cLLL4.getLength("L") * unitFactor;
      LLL4_R[0] = cLLL4.getLength("R") * unitFactor;
      LLL4_R[1] = cLLL4.getLength("R") * unitFactor;
      LLL4_r[0] = cLLL4.getLength("r") * unitFactor;
      LLL4_r[1] = cLLL4.getLength("r") * unitFactor;

      double LLL4_X0 = cLLL4.getLength("X0") * unitFactor;
      double LLL4_Z0 = cLLL4.getLength("Z0") * unitFactor;
      double LLL4_PHI = cLLL4.getLength("PHI");
      G4Transform3D transform_LLL4 = G4Translate3D(LLL4_X0, 0.0, LLL4_Z0);
      transform_LLL4 = transform_LLL4 * G4RotateY3D(LLL4_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_LLL4xx = new G4Polycone("geo_LLL4xx_name", 0.0, 2 * M_PI, N, LLL4_Z, LLL4_r, LLL4_R);
      G4IntersectionSolid* geo_LLL4 = new G4IntersectionSolid("geo_LLL4_name", geo_LLL4xx, geo_TubeL, transform_LLL4.inverse());

      string strMat_LLL4 = cLLL4.getString("Material");
      G4Material* mat_LLL4 = Materials::get(strMat_LLL4);
      G4LogicalVolume* logi_LLL4 = new G4LogicalVolume(geo_LLL4, mat_LLL4, "logi_LLL4_name");

      //put volume
      setColor(*logi_LLL4, cLLL4.getString("Color", "#CC0000"));
      //setVisibility(*logi_LLL4, false);
      new G4PVPlacement(transform_LLL4, logi_LLL4, "phys_LLL4_name", &topVolume, false, 0);

      //--------------
      //-   bending magnets

      //--------------
      //-   BLC2REp1

      //get parameters from .xml file
      GearDir cBLC2REp1(content, "BLC2REp1/");

      double BLC2REp1_r = cBLC2REp1.getLength("r") * unitFactor;
      double BLC2REp1_R = cBLC2REp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC2REp1 = new G4Torus("geo_BLC2REp1_name", BLC2REp1_r, BLC2REp1_R, BLC2REtube_RT, BLC2REtube_SPHI, BLC2REtube_DPHI);

      string strMat_BLC2REp1 = cBLC2REp1.getString("Material");
      G4Material* mat_BLC2REp1 = Materials::get(strMat_BLC2REp1);
      G4LogicalVolume* logi_BLC2REp1 = new G4LogicalVolume(geo_BLC2REp1, mat_BLC2REp1, "logi_BLC2REp1_name");

      //put volume
      setColor(*logi_BLC2REp1, cBLC2REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC2REp1, false);
      new G4PVPlacement(transform_BLC2REtube, logi_BLC2REp1, "phys_BLC2REp1_name", &topVolume, false, 0);

      //--------------
      //-   BLC2REp2

      //get parameters from .xml file
      GearDir cBLC2REp2(content, "BLC2REp2/");

      double BLC2REp2_r = cBLC2REp2.getLength("r") * unitFactor;
      double BLC2REp2_R = cBLC2REp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC2REp2 = new G4Torus("geo_BLC2REp2_name", BLC2REp2_r, BLC2REp2_R, BLC2REtube_RT, BLC2REtube_SPHI, BLC2REtube_DPHI);

      string strMat_BLC2REp2 = cBLC2REp2.getString("Material");
      G4Material* mat_BLC2REp2 = Materials::get(strMat_BLC2REp2);
      G4LogicalVolume* logi_BLC2REp2 = new G4LogicalVolume(geo_BLC2REp2, mat_BLC2REp2, "logi_BLC2REp2_name");

      //put volume
      setColor(*logi_BLC2REp2, cBLC2REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC2REp2, false);
      new G4PVPlacement(transform_BLC2REtube, logi_BLC2REp2, "phys_BLC2REp2_name", &topVolume, false, 0);

      //--------------
      //-   BC1RPp1

      //get parameters from .xml file
      GearDir cBC1RPp1(content, "BC1RPp1/");

      double BC1RPp1_r = cBC1RPp1.getLength("r") * unitFactor;
      double BC1RPp1_R = cBC1RPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BC1RPp1 = new G4Torus("geo_BC1RPp1_name", BC1RPp1_r, BC1RPp1_R, BC1RPtube_RT, BC1RPtube_SPHI, BC1RPtube_DPHI);

      string strMat_BC1RPp1 = cBC1RPp1.getString("Material");
      G4Material* mat_BC1RPp1 = Materials::get(strMat_BC1RPp1);
      G4LogicalVolume* logi_BC1RPp1 = new G4LogicalVolume(geo_BC1RPp1, mat_BC1RPp1, "logi_BC1RPp1_name");

      //put volume
      setColor(*logi_BC1RPp1, cBC1RPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BC1RPp1, false);
      new G4PVPlacement(transform_BC1RPtube, logi_BC1RPp1, "phys_BC1RPp1_name", &topVolume, false, 0);

      //--------------
      //-   BC1RPp2

      //get parameters from .xml file
      GearDir cBC1RPp2(content, "BC1RPp2/");

      double BC1RPp2_r = cBC1RPp2.getLength("r") * unitFactor;
      double BC1RPp2_R = cBC1RPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BC1RPp2 = new G4Torus("geo_BC1RPp2_name", BC1RPp2_r, BC1RPp2_R, BC1RPtube_RT, BC1RPtube_SPHI, BC1RPtube_DPHI);

      string strMat_BC1RPp2 = cBC1RPp2.getString("Material");
      G4Material* mat_BC1RPp2 = Materials::get(strMat_BC1RPp2);
      G4LogicalVolume* logi_BC1RPp2 = new G4LogicalVolume(geo_BC1RPp2, mat_BC1RPp2, "logi_BC1RPp2_name");

      //put volume
      setColor(*logi_BC1RPp2, cBC1RPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BC1RPp2, false);
      new G4PVPlacement(transform_BC1RPtube, logi_BC1RPp2, "phys_BC1RPp2_name", &topVolume, false, 0);

      //--------------
      //-   BLCWRPp1

      //get parameters from .xml file
      GearDir cBLCWRPp1(content, "BLCWRPp1/");

      double BLCWRPp1_r = cBLCWRPp1.getLength("r") * unitFactor;
      double BLCWRPp1_R = cBLCWRPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLCWRPp1 = new G4Torus("geo_BLCWRPp1_name", BLCWRPp1_r, BLCWRPp1_R, BLCWRPtube_RT, BLCWRPtube_SPHI, BLCWRPtube_DPHI);

      string strMat_BLCWRPp1 = cBLCWRPp1.getString("Material");
      G4Material* mat_BLCWRPp1 = Materials::get(strMat_BLCWRPp1);
      G4LogicalVolume* logi_BLCWRPp1 = new G4LogicalVolume(geo_BLCWRPp1, mat_BLCWRPp1, "logi_BLCWRPp1_name");

      //put volume
      setColor(*logi_BLCWRPp1, cBLCWRPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLCWRPp1, false);
      new G4PVPlacement(transform_BLCWRPtube, logi_BLCWRPp1, "phys_BLCWRPp1_name", &topVolume, false, 0);

      //--------------
      //-   BLCWRPp2

      //get parameters from .xml file
      GearDir cBLCWRPp2(content, "BLCWRPp2/");

      double BLCWRPp2_r = cBLCWRPp2.getLength("r") * unitFactor;
      double BLCWRPp2_R = cBLCWRPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLCWRPp2 = new G4Torus("geo_BLCWRPp2_name", BLCWRPp2_r, BLCWRPp2_R, BLCWRPtube_RT, BLCWRPtube_SPHI, BLCWRPtube_DPHI);

      string strMat_BLCWRPp2 = cBLCWRPp2.getString("Material");
      G4Material* mat_BLCWRPp2 = Materials::get(strMat_BLCWRPp2);
      G4LogicalVolume* logi_BLCWRPp2 = new G4LogicalVolume(geo_BLCWRPp2, mat_BLCWRPp2, "logi_BLCWRPp2_name");

      //put volume
      setColor(*logi_BLCWRPp2, cBLCWRPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLCWRPp2, false);
      new G4PVPlacement(transform_BLCWRPtube, logi_BLCWRPp2, "phys_BLCWRPp2_name", &topVolume, false, 0);

      //--------------
      //-   BLC1RPp1

      //get parameters from .xml file
      GearDir cBLC1RPp1(content, "BLC1RPp1/");

      double BLC1RPp1_r = cBLC1RPp1.getLength("r") * unitFactor;
      double BLC1RPp1_R = cBLC1RPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC1RPp1 = new G4Torus("geo_BLC1RPp1_name", BLC1RPp1_r, BLC1RPp1_R, BLC1RPtube_RT, BLC1RPtube_SPHI, BLC1RPtube_DPHI);

      string strMat_BLC1RPp1 = cBLC1RPp1.getString("Material");
      G4Material* mat_BLC1RPp1 = Materials::get(strMat_BLC1RPp1);
      G4LogicalVolume* logi_BLC1RPp1 = new G4LogicalVolume(geo_BLC1RPp1, mat_BLC1RPp1, "logi_BLC1RPp1_name");

      //put volume
      setColor(*logi_BLC1RPp1, cBLC1RPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC1RPp1, false);
      new G4PVPlacement(transform_BLC1RPtube, logi_BLC1RPp1, "phys_BLC1RPp1_name", &topVolume, false, 0);

      //--------------
      //-   BLC1RPp2

      //get parameters from .xml file
      GearDir cBLC1RPp2(content, "BLC1RPp2/");

      double BLC1RPp2_r = cBLC1RPp2.getLength("r") * unitFactor;
      double BLC1RPp2_R = cBLC1RPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC1RPp2 = new G4Torus("geo_BLC1RPp2_name", BLC1RPp2_r, BLC1RPp2_R, BLC1RPtube_RT, BLC1RPtube_SPHI, BLC1RPtube_DPHI);

      string strMat_BLC1RPp2 = cBLC1RPp2.getString("Material");
      G4Material* mat_BLC1RPp2 = Materials::get(strMat_BLC1RPp2);
      G4LogicalVolume* logi_BLC1RPp2 = new G4LogicalVolume(geo_BLC1RPp2, mat_BLC1RPp2, "logi_BLC1RPp2_name");

      //put volume
      setColor(*logi_BLC1RPp2, cBLC1RPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC1RPp2, false);
      new G4PVPlacement(transform_BLC1RPtube, logi_BLC1RPp2, "phys_BLC1RPp2_name", &topVolume, false, 0);

      //--------------
      //-   BLC2RPp1

      //get parameters from .xml file
      GearDir cBLC2RPp1(content, "BLC2RPp1/");

      double BLC2RPp1_r = cBLC2RPp1.getLength("r") * unitFactor;
      double BLC2RPp1_R = cBLC2RPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC2RPp1 = new G4Torus("geo_BLC2RPp1_name", BLC2RPp1_r, BLC2RPp1_R, BLC2RPtube_RT, BLC2RPtube_SPHI, BLC2RPtube_DPHI);

      string strMat_BLC2RPp1 = cBLC2RPp1.getString("Material");
      G4Material* mat_BLC2RPp1 = Materials::get(strMat_BLC2RPp1);
      G4LogicalVolume* logi_BLC2RPp1 = new G4LogicalVolume(geo_BLC2RPp1, mat_BLC2RPp1, "logi_BLC2RPp1_name");

      //put volume
      setColor(*logi_BLC2RPp1, cBLC2RPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC2RPp1, false);
      new G4PVPlacement(transform_BLC2RPtube, logi_BLC2RPp1, "phys_BLC2RPp1_name", &topVolume, false, 0);

      //--------------
      //-   BLC2RPp2

      //get parameters from .xml file
      GearDir cBLC2RPp2(content, "BLC2RPp2/");

      double BLC2RPp2_r = cBLC2RPp2.getLength("r") * unitFactor;
      double BLC2RPp2_R = cBLC2RPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC2RPp2 = new G4Torus("geo_BLC2RPp2_name", BLC2RPp2_r, BLC2RPp2_R, BLC2RPtube_RT, BLC2RPtube_SPHI, BLC2RPtube_DPHI);

      string strMat_BLC2RPp2 = cBLC2RPp2.getString("Material");
      G4Material* mat_BLC2RPp2 = Materials::get(strMat_BLC2RPp2);
      G4LogicalVolume* logi_BLC2RPp2 = new G4LogicalVolume(geo_BLC2RPp2, mat_BLC2RPp2, "logi_BLC2RPp2_name");

      //put volume
      setColor(*logi_BLC2RPp2, cBLC2RPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC2RPp2, false);
      new G4PVPlacement(transform_BLC2RPtube, logi_BLC2RPp2, "phys_BLC2RPp2_name", &topVolume, false, 0);

      //--------------
      //-   BLC1LEp1

      //get parameters from .xml file
      GearDir cBLC1LEp1(content, "BLC1LEp1/");

      double BLC1LEp1_r = cBLC1LEp1.getLength("r") * unitFactor;
      double BLC1LEp1_R = cBLC1LEp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC1LEp1 = new G4Torus("geo_BLC1LEp1_name", BLC1LEp1_r, BLC1LEp1_R, BLC1LEtube_RT, BLC1LEtube_SPHI, BLC1LEtube_DPHI);

      string strMat_BLC1LEp1 = cBLC1LEp1.getString("Material");
      G4Material* mat_BLC1LEp1 = Materials::get(strMat_BLC1LEp1);
      G4LogicalVolume* logi_BLC1LEp1 = new G4LogicalVolume(geo_BLC1LEp1, mat_BLC1LEp1, "logi_BLC1LEp1_name");

      //put volume
      setColor(*logi_BLC1LEp1, cBLC1LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC1LEp1, false);
      new G4PVPlacement(transform_BLC1LEtube, logi_BLC1LEp1, "phys_BLC1LEp1_name", &topVolume, false, 0);

      //--------------
      //-   BLC1LEp2

      //get parameters from .xml file
      GearDir cBLC1LEp2(content, "BLC1LEp2/");

      double BLC1LEp2_r = cBLC1LEp2.getLength("r") * unitFactor;
      double BLC1LEp2_R = cBLC1LEp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC1LEp2 = new G4Torus("geo_BLC1LEp2_name", BLC1LEp2_r, BLC1LEp2_R, BLC1LEtube_RT, BLC1LEtube_SPHI, BLC1LEtube_DPHI);

      string strMat_BLC1LEp2 = cBLC1LEp2.getString("Material");
      G4Material* mat_BLC1LEp2 = Materials::get(strMat_BLC1LEp2);
      G4LogicalVolume* logi_BLC1LEp2 = new G4LogicalVolume(geo_BLC1LEp2, mat_BLC1LEp2, "logi_BLC1LEp2_name");

      //put volume
      setColor(*logi_BLC1LEp2, cBLC1LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC1LEp2, false);
      new G4PVPlacement(transform_BLC1LEtube, logi_BLC1LEp2, "phys_BLC1LEp2_name", &topVolume, false, 0);

      //--------------
      //-   BC1LPp1

      //get parameters from .xml file
      GearDir cBC1LPp1(content, "BC1LPp1/");

      double BC1LPp1_r = cBC1LPp1.getLength("r") * unitFactor;
      double BC1LPp1_R = cBC1LPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BC1LPp1 = new G4Torus("geo_BC1LPp1_name", BC1LPp1_r, BC1LPp1_R, BC1LPtube_RT, BC1LPtube_SPHI, BC1LPtube_DPHI);

      string strMat_BC1LPp1 = cBC1LPp1.getString("Material");
      G4Material* mat_BC1LPp1 = Materials::get(strMat_BC1LPp1);
      G4LogicalVolume* logi_BC1LPp1 = new G4LogicalVolume(geo_BC1LPp1, mat_BC1LPp1, "logi_BC1LPp1_name");

      //put volume
      setColor(*logi_BC1LPp1, cBC1LPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BC1LPp1, false);
      new G4PVPlacement(transform_BC1LPtube, logi_BC1LPp1, "phys_BC1LPp1_name", &topVolume, false, 0);

      //--------------
      //-   BC1LPp2

      //get parameters from .xml file
      GearDir cBC1LPp2(content, "BC1LPp2/");

      double BC1LPp2_r = cBC1LPp2.getLength("r") * unitFactor;
      double BC1LPp2_R = cBC1LPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BC1LPp2 = new G4Torus("geo_BC1LPp2_name", BC1LPp2_r, BC1LPp2_R, BC1LPtube_RT, BC1LPtube_SPHI, BC1LPtube_DPHI);

      string strMat_BC1LPp2 = cBC1LPp2.getString("Material");
      G4Material* mat_BC1LPp2 = Materials::get(strMat_BC1LPp2);
      G4LogicalVolume* logi_BC1LPp2 = new G4LogicalVolume(geo_BC1LPp2, mat_BC1LPp2, "logi_BC1LPp2_name");

      //put volume
      setColor(*logi_BC1LPp2, cBC1LPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BC1LPp2, false);
      new G4PVPlacement(transform_BC1LPtube, logi_BC1LPp2, "phys_BC1LPp2_name", &topVolume, false, 0);

      //--------------
      //-   BLC1LPp1

      //get parameters from .xml file
      GearDir cBLC1LPp1(content, "BLC1LPp1/");

      double BLC1LPp1_r = cBLC1LPp1.getLength("r") * unitFactor;
      double BLC1LPp1_R = cBLC1LPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC1LPp1 = new G4Torus("geo_BLC1LPp1_name", BLC1LPp1_r, BLC1LPp1_R, BLC1LPtube_RT, BLC1LPtube_SPHI, BLC1LPtube_DPHI);

      string strMat_BLC1LPp1 = cBLC1LPp1.getString("Material");
      G4Material* mat_BLC1LPp1 = Materials::get(strMat_BLC1LPp1);
      G4LogicalVolume* logi_BLC1LPp1 = new G4LogicalVolume(geo_BLC1LPp1, mat_BLC1LPp1, "logi_BLC1LPp1_name");

      //put volume
      setColor(*logi_BLC1LPp1, cBLC1LPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC1LPp1, false);
      new G4PVPlacement(transform_BLC1LPtube, logi_BLC1LPp1, "phys_BLC1LPp1_name", &topVolume, false, 0);

      //--------------
      //-   BLC1LPp2

      //get parameters from .xml file
      GearDir cBLC1LPp2(content, "BLC1LPp2/");

      double BLC1LPp2_r = cBLC1LPp2.getLength("r") * unitFactor;
      double BLC1LPp2_R = cBLC1LPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC1LPp2 = new G4Torus("geo_BLC1LPp2_name", BLC1LPp2_r, BLC1LPp2_R, BLC1LPtube_RT, BLC1LPtube_SPHI, BLC1LPtube_DPHI);

      string strMat_BLC1LPp2 = cBLC1LPp2.getString("Material");
      G4Material* mat_BLC1LPp2 = Materials::get(strMat_BLC1LPp2);
      G4LogicalVolume* logi_BLC1LPp2 = new G4LogicalVolume(geo_BLC1LPp2, mat_BLC1LPp2, "logi_BLC1LPp2_name");

      //put volume
      setColor(*logi_BLC1LPp2, cBLC1LPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC1LPp2, false);
      new G4PVPlacement(transform_BLC1LPtube, logi_BLC1LPp2, "phys_BLC1LPp2_name", &topVolume, false, 0);

      //--------------
      //-   BLC2LPp1

      //get parameters from .xml file
      GearDir cBLC2LPp1(content, "BLC2LPp1/");

      double BLC2LPp1_r = cBLC2LPp1.getLength("r") * unitFactor;
      double BLC2LPp1_R = cBLC2LPp1.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC2LPp1 = new G4Torus("geo_BLC2LPp1_name", BLC2LPp1_r, BLC2LPp1_R, BLC2LPtube_RT, BLC2LPtube_SPHI, BLC2LPtube_DPHI);

      string strMat_BLC2LPp1 = cBLC2LPp1.getString("Material");
      G4Material* mat_BLC2LPp1 = Materials::get(strMat_BLC2LPp1);
      G4LogicalVolume* logi_BLC2LPp1 = new G4LogicalVolume(geo_BLC2LPp1, mat_BLC2LPp1, "logi_BLC2LPp1_name");

      //put volume
      setColor(*logi_BLC2LPp1, cBLC2LPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC2LPp1, false);
      new G4PVPlacement(transform_BLC2LPtube, logi_BLC2LPp1, "phys_BLC2LPp1_name", &topVolume, false, 0);

      //--------------
      //-   BLC2LPp2

      //get parameters from .xml file
      GearDir cBLC2LPp2(content, "BLC2LPp2/");

      double BLC2LPp2_r = cBLC2LPp2.getLength("r") * unitFactor;
      double BLC2LPp2_R = cBLC2LPp2.getLength("R") * unitFactor;

      //define geometry
      G4Torus* geo_BLC2LPp2 = new G4Torus("geo_BLC2LPp2_name", BLC2LPp2_r, BLC2LPp2_R, BLC2LPtube_RT, BLC2LPtube_SPHI, BLC2LPtube_DPHI);

      string strMat_BLC2LPp2 = cBLC2LPp2.getString("Material");
      G4Material* mat_BLC2LPp2 = Materials::get(strMat_BLC2LPp2);
      G4LogicalVolume* logi_BLC2LPp2 = new G4LogicalVolume(geo_BLC2LPp2, mat_BLC2LPp2, "logi_BLC2LPp2_name");

      //put volume
      setColor(*logi_BLC2LPp2, cBLC2LPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BLC2LPp2, false);
      new G4PVPlacement(transform_BLC2LPtube, logi_BLC2LPp2, "phys_BLC2LPp2_name", &topVolume, false, 0);

      //--------------
      //-   magnets

      //--------------
      //-   BKC1REp1

      //get parameters from .xml file
      GearDir cBKC1REp1(content, "BKC1REp1/");

      double BKC1REp1_Z[N];
      double BKC1REp1_R[N];
      double BKC1REp1_r[N];
      BKC1REp1_Z[0] = 0;
      BKC1REp1_Z[1] = cBKC1REp1.getLength("L") * unitFactor;
      BKC1REp1_R[0] = cBKC1REp1.getLength("R") * unitFactor;
      BKC1REp1_R[1] = cBKC1REp1.getLength("R") * unitFactor;
      BKC1REp1_r[0] = cBKC1REp1.getLength("r") * unitFactor;
      BKC1REp1_r[1] = cBKC1REp1.getLength("r") * unitFactor;

      double BKC1REp1_X0 = cBKC1REp1.getLength("X0") * unitFactor;
      double BKC1REp1_Z0 = cBKC1REp1.getLength("Z0") * unitFactor;
      double BKC1REp1_PHI = cBKC1REp1.getLength("PHI");
      G4Transform3D transform_BKC1REp1 = G4Translate3D(BKC1REp1_X0, 0.0, BKC1REp1_Z0);
      transform_BKC1REp1 = transform_BKC1REp1 * G4RotateY3D(BKC1REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC1REp1 = new G4Polycone("geo_BKC1REp1xx_name", 0.0, 2 * M_PI, N, BKC1REp1_Z, BKC1REp1_r, BKC1REp1_R);

      string strMat_BKC1REp1 = cBKC1REp1.getString("Material");
      G4Material* mat_BKC1REp1 = Materials::get(strMat_BKC1REp1);
      G4LogicalVolume* logi_BKC1REp1 = new G4LogicalVolume(geo_BKC1REp1, mat_BKC1REp1, "logi_BKC1REp1_name");

      //put volume
      setColor(*logi_BKC1REp1, cBKC1REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC1REp1, false);
      new G4PVPlacement(transform_BKC1REp1, logi_BKC1REp1, "phys_BKC1REp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC1REp2

      //get parameters from .xml file
      GearDir cBKC1REp2(content, "BKC1REp2/");

      double BKC1REp2_Z[N];
      double BKC1REp2_R[N];
      double BKC1REp2_r[N];
      BKC1REp2_Z[0] = 0;
      BKC1REp2_Z[1] = cBKC1REp2.getLength("L") * unitFactor;
      BKC1REp2_R[0] = cBKC1REp2.getLength("R") * unitFactor;
      BKC1REp2_R[1] = cBKC1REp2.getLength("R") * unitFactor;
      BKC1REp2_r[0] = cBKC1REp2.getLength("r") * unitFactor;
      BKC1REp2_r[1] = cBKC1REp2.getLength("r") * unitFactor;

      double BKC1REp2_X0 = cBKC1REp2.getLength("X0") * unitFactor;
      double BKC1REp2_Z0 = cBKC1REp2.getLength("Z0") * unitFactor;
      double BKC1REp2_PHI = cBKC1REp2.getLength("PHI");
      G4Transform3D transform_BKC1REp2 = G4Translate3D(BKC1REp2_X0, 0.0, BKC1REp2_Z0);
      transform_BKC1REp2 = transform_BKC1REp2 * G4RotateY3D(BKC1REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC1REp2 = new G4Polycone("geo_BKC1REp2xx_name", 0.0, 2 * M_PI, N, BKC1REp2_Z, BKC1REp2_r, BKC1REp2_R);

      string strMat_BKC1REp2 = cBKC1REp2.getString("Material");
      G4Material* mat_BKC1REp2 = Materials::get(strMat_BKC1REp2);
      G4LogicalVolume* logi_BKC1REp2 = new G4LogicalVolume(geo_BKC1REp2, mat_BKC1REp2, "logi_BKC1REp2_name");

      //put volume
      setColor(*logi_BKC1REp2, cBKC1REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC1REp2, false);
      new G4PVPlacement(transform_BKC1REp2, logi_BKC1REp2, "phys_BKC1REp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC2REp1

      //get parameters from .xml file
      GearDir cBKC2REp1(content, "BKC2REp1/");

      double BKC2REp1_Z[N];
      double BKC2REp1_R[N];
      double BKC2REp1_r[N];
      BKC2REp1_Z[0] = 0;
      BKC2REp1_Z[1] = cBKC2REp1.getLength("L") * unitFactor;
      BKC2REp1_R[0] = cBKC2REp1.getLength("R") * unitFactor;
      BKC2REp1_R[1] = cBKC2REp1.getLength("R") * unitFactor;
      BKC2REp1_r[0] = cBKC2REp1.getLength("r") * unitFactor;
      BKC2REp1_r[1] = cBKC2REp1.getLength("r") * unitFactor;

      double BKC2REp1_X0 = cBKC2REp1.getLength("X0") * unitFactor;
      double BKC2REp1_Z0 = cBKC2REp1.getLength("Z0") * unitFactor;
      double BKC2REp1_PHI = cBKC2REp1.getLength("PHI");
      G4Transform3D transform_BKC2REp1 = G4Translate3D(BKC2REp1_X0, 0.0, BKC2REp1_Z0);
      transform_BKC2REp1 = transform_BKC2REp1 * G4RotateY3D(BKC2REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC2REp1 = new G4Polycone("geo_BKC2REp1xx_name", 0.0, 2 * M_PI, N, BKC2REp1_Z, BKC2REp1_r, BKC2REp1_R);

      string strMat_BKC2REp1 = cBKC2REp1.getString("Material");
      G4Material* mat_BKC2REp1 = Materials::get(strMat_BKC2REp1);
      G4LogicalVolume* logi_BKC2REp1 = new G4LogicalVolume(geo_BKC2REp1, mat_BKC2REp1, "logi_BKC2REp1_name");

      //put volume
      setColor(*logi_BKC2REp1, cBKC2REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC2REp1, false);
      new G4PVPlacement(transform_BKC2REp1, logi_BKC2REp1, "phys_BKC2REp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC2REp2

      //get parameters from .xml file
      GearDir cBKC2REp2(content, "BKC2REp2/");

      double BKC2REp2_Z[N];
      double BKC2REp2_R[N];
      double BKC2REp2_r[N];
      BKC2REp2_Z[0] = 0;
      BKC2REp2_Z[1] = cBKC2REp2.getLength("L") * unitFactor;
      BKC2REp2_R[0] = cBKC2REp2.getLength("R") * unitFactor;
      BKC2REp2_R[1] = cBKC2REp2.getLength("R") * unitFactor;
      BKC2REp2_r[0] = cBKC2REp2.getLength("r") * unitFactor;
      BKC2REp2_r[1] = cBKC2REp2.getLength("r") * unitFactor;

      double BKC2REp2_X0 = cBKC2REp2.getLength("X0") * unitFactor;
      double BKC2REp2_Z0 = cBKC2REp2.getLength("Z0") * unitFactor;
      double BKC2REp2_PHI = cBKC2REp2.getLength("PHI");
      G4Transform3D transform_BKC2REp2 = G4Translate3D(BKC2REp2_X0, 0.0, BKC2REp2_Z0);
      transform_BKC2REp2 = transform_BKC2REp2 * G4RotateY3D(BKC2REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC2REp2 = new G4Polycone("geo_BKC2REp2xx_name", 0.0, 2 * M_PI, N, BKC2REp2_Z, BKC2REp2_r, BKC2REp2_R);

      string strMat_BKC2REp2 = cBKC2REp2.getString("Material");
      G4Material* mat_BKC2REp2 = Materials::get(strMat_BKC2REp2);
      G4LogicalVolume* logi_BKC2REp2 = new G4LogicalVolume(geo_BKC2REp2, mat_BKC2REp2, "logi_BKC2REp2_name");

      //put volume
      setColor(*logi_BKC2REp2, cBKC2REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC2REp2, false);
      new G4PVPlacement(transform_BKC2REp2, logi_BKC2REp2, "phys_BKC2REp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC3REp1

      //get parameters from .xml file
      GearDir cBKC3REp1(content, "BKC3REp1/");

      double BKC3REp1_Z[N];
      double BKC3REp1_R[N];
      double BKC3REp1_r[N];
      BKC3REp1_Z[0] = 0;
      BKC3REp1_Z[1] = cBKC3REp1.getLength("L") * unitFactor;
      BKC3REp1_R[0] = cBKC3REp1.getLength("R") * unitFactor;
      BKC3REp1_R[1] = cBKC3REp1.getLength("R") * unitFactor;
      BKC3REp1_r[0] = cBKC3REp1.getLength("r") * unitFactor;
      BKC3REp1_r[1] = cBKC3REp1.getLength("r") * unitFactor;

      double BKC3REp1_X0 = cBKC3REp1.getLength("X0") * unitFactor;
      double BKC3REp1_Z0 = cBKC3REp1.getLength("Z0") * unitFactor;
      double BKC3REp1_PHI = cBKC3REp1.getLength("PHI");
      G4Transform3D transform_BKC3REp1 = G4Translate3D(BKC3REp1_X0, 0.0, BKC3REp1_Z0);
      transform_BKC3REp1 = transform_BKC3REp1 * G4RotateY3D(BKC3REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC3REp1 = new G4Polycone("geo_BKC3REp1xx_name", 0.0, 2 * M_PI, N, BKC3REp1_Z, BKC3REp1_r, BKC3REp1_R);

      string strMat_BKC3REp1 = cBKC3REp1.getString("Material");
      G4Material* mat_BKC3REp1 = Materials::get(strMat_BKC3REp1);
      G4LogicalVolume* logi_BKC3REp1 = new G4LogicalVolume(geo_BKC3REp1, mat_BKC3REp1, "logi_BKC3REp1_name");

      //put volume
      setColor(*logi_BKC3REp1, cBKC3REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC3REp1, false);
      new G4PVPlacement(transform_BKC3REp1, logi_BKC3REp1, "phys_BKC3REp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC3REp2

      //get parameters from .xml file
      GearDir cBKC3REp2(content, "BKC3REp2/");

      double BKC3REp2_Z[N];
      double BKC3REp2_R[N];
      double BKC3REp2_r[N];
      BKC3REp2_Z[0] = 0;
      BKC3REp2_Z[1] = cBKC3REp2.getLength("L") * unitFactor;
      BKC3REp2_R[0] = cBKC3REp2.getLength("R") * unitFactor;
      BKC3REp2_R[1] = cBKC3REp2.getLength("R") * unitFactor;
      BKC3REp2_r[0] = cBKC3REp2.getLength("r") * unitFactor;
      BKC3REp2_r[1] = cBKC3REp2.getLength("r") * unitFactor;

      double BKC3REp2_X0 = cBKC3REp2.getLength("X0") * unitFactor;
      double BKC3REp2_Z0 = cBKC3REp2.getLength("Z0") * unitFactor;
      double BKC3REp2_PHI = cBKC3REp2.getLength("PHI");
      G4Transform3D transform_BKC3REp2 = G4Translate3D(BKC3REp2_X0, 0.0, BKC3REp2_Z0);
      transform_BKC3REp2 = transform_BKC3REp2 * G4RotateY3D(BKC3REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC3REp2 = new G4Polycone("geo_BKC3REp2xx_name", 0.0, 2 * M_PI, N, BKC3REp2_Z, BKC3REp2_r, BKC3REp2_R);

      string strMat_BKC3REp2 = cBKC3REp2.getString("Material");
      G4Material* mat_BKC3REp2 = Materials::get(strMat_BKC3REp2);
      G4LogicalVolume* logi_BKC3REp2 = new G4LogicalVolume(geo_BKC3REp2, mat_BKC3REp2, "logi_BKC3REp2_name");

      //put volume
      setColor(*logi_BKC3REp2, cBKC3REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC3REp2, false);
      new G4PVPlacement(transform_BKC3REp2, logi_BKC3REp2, "phys_BKC3REp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC4REp1

      //get parameters from .xml file
      GearDir cBKC4REp1(content, "BKC4REp1/");

      double BKC4REp1_Z[N];
      double BKC4REp1_R[N];
      double BKC4REp1_r[N];
      BKC4REp1_Z[0] = 0;
      BKC4REp1_Z[1] = cBKC4REp1.getLength("L") * unitFactor;
      BKC4REp1_R[0] = cBKC4REp1.getLength("R") * unitFactor;
      BKC4REp1_R[1] = cBKC4REp1.getLength("R") * unitFactor;
      BKC4REp1_r[0] = cBKC4REp1.getLength("r") * unitFactor;
      BKC4REp1_r[1] = cBKC4REp1.getLength("r") * unitFactor;

      double BKC4REp1_X0 = cBKC4REp1.getLength("X0") * unitFactor;
      double BKC4REp1_Z0 = cBKC4REp1.getLength("Z0") * unitFactor;
      double BKC4REp1_PHI = cBKC4REp1.getLength("PHI");
      G4Transform3D transform_BKC4REp1 = G4Translate3D(BKC4REp1_X0, 0.0, BKC4REp1_Z0);
      transform_BKC4REp1 = transform_BKC4REp1 * G4RotateY3D(BKC4REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC4REp1 = new G4Polycone("geo_BKC4REp1xx_name", 0.0, 2 * M_PI, N, BKC4REp1_Z, BKC4REp1_r, BKC4REp1_R);

      string strMat_BKC4REp1 = cBKC4REp1.getString("Material");
      G4Material* mat_BKC4REp1 = Materials::get(strMat_BKC4REp1);
      G4LogicalVolume* logi_BKC4REp1 = new G4LogicalVolume(geo_BKC4REp1, mat_BKC4REp1, "logi_BKC4REp1_name");

      //put volume
      setColor(*logi_BKC4REp1, cBKC4REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC4REp1, false);
      new G4PVPlacement(transform_BKC4REp1, logi_BKC4REp1, "phys_BKC4REp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC4REp2

      //get parameters from .xml file
      GearDir cBKC4REp2(content, "BKC4REp2/");

      double BKC4REp2_Z[N];
      double BKC4REp2_R[N];
      double BKC4REp2_r[N];
      BKC4REp2_Z[0] = 0;
      BKC4REp2_Z[1] = cBKC4REp2.getLength("L") * unitFactor;
      BKC4REp2_R[0] = cBKC4REp2.getLength("R") * unitFactor;
      BKC4REp2_R[1] = cBKC4REp2.getLength("R") * unitFactor;
      BKC4REp2_r[0] = cBKC4REp2.getLength("r") * unitFactor;
      BKC4REp2_r[1] = cBKC4REp2.getLength("r") * unitFactor;

      double BKC4REp2_X0 = cBKC4REp2.getLength("X0") * unitFactor;
      double BKC4REp2_Z0 = cBKC4REp2.getLength("Z0") * unitFactor;
      double BKC4REp2_PHI = cBKC4REp2.getLength("PHI");
      G4Transform3D transform_BKC4REp2 = G4Translate3D(BKC4REp2_X0, 0.0, BKC4REp2_Z0);
      transform_BKC4REp2 = transform_BKC4REp2 * G4RotateY3D(BKC4REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC4REp2 = new G4Polycone("geo_BKC4REp2xx_name", 0.0, 2 * M_PI, N, BKC4REp2_Z, BKC4REp2_r, BKC4REp2_R);

      string strMat_BKC4REp2 = cBKC4REp2.getString("Material");
      G4Material* mat_BKC4REp2 = Materials::get(strMat_BKC4REp2);
      G4LogicalVolume* logi_BKC4REp2 = new G4LogicalVolume(geo_BKC4REp2, mat_BKC4REp2, "logi_BKC4REp2_name");

      //put volume
      setColor(*logi_BKC4REp2, cBKC4REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC4REp2, false);
      new G4PVPlacement(transform_BKC4REp2, logi_BKC4REp2, "phys_BKC4REp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC1LEp1

      //get parameters from .xml file
      GearDir cBKC1LEp1(content, "BKC1LEp1/");

      double BKC1LEp1_Z[N];
      double BKC1LEp1_R[N];
      double BKC1LEp1_r[N];
      BKC1LEp1_Z[0] = 0;
      BKC1LEp1_Z[1] = cBKC1LEp1.getLength("L") * unitFactor;
      BKC1LEp1_R[0] = cBKC1LEp1.getLength("R") * unitFactor;
      BKC1LEp1_R[1] = cBKC1LEp1.getLength("R") * unitFactor;
      BKC1LEp1_r[0] = cBKC1LEp1.getLength("r") * unitFactor;
      BKC1LEp1_r[1] = cBKC1LEp1.getLength("r") * unitFactor;

      double BKC1LEp1_X0 = cBKC1LEp1.getLength("X0") * unitFactor;
      double BKC1LEp1_Z0 = cBKC1LEp1.getLength("Z0") * unitFactor;
      double BKC1LEp1_PHI = cBKC1LEp1.getLength("PHI");
      G4Transform3D transform_BKC1LEp1 = G4Translate3D(BKC1LEp1_X0, 0.0, BKC1LEp1_Z0);
      transform_BKC1LEp1 = transform_BKC1LEp1 * G4RotateY3D(BKC1LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC1LEp1 = new G4Polycone("geo_BKC1LEp1xx_name", 0.0, 2 * M_PI, N, BKC1LEp1_Z, BKC1LEp1_r, BKC1LEp1_R);

      string strMat_BKC1LEp1 = cBKC1LEp1.getString("Material");
      G4Material* mat_BKC1LEp1 = Materials::get(strMat_BKC1LEp1);
      G4LogicalVolume* logi_BKC1LEp1 = new G4LogicalVolume(geo_BKC1LEp1, mat_BKC1LEp1, "logi_BKC1LEp1_name");

      //put volume
      setColor(*logi_BKC1LEp1, cBKC1LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC1LEp1, false);
      new G4PVPlacement(transform_BKC1LEp1, logi_BKC1LEp1, "phys_BKC1LEp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC1LEp2

      //get parameters from .xml file
      GearDir cBKC1LEp2(content, "BKC1LEp2/");

      double BKC1LEp2_Z[N];
      double BKC1LEp2_R[N];
      double BKC1LEp2_r[N];
      BKC1LEp2_Z[0] = 0;
      BKC1LEp2_Z[1] = cBKC1LEp2.getLength("L") * unitFactor;
      BKC1LEp2_R[0] = cBKC1LEp2.getLength("R") * unitFactor;
      BKC1LEp2_R[1] = cBKC1LEp2.getLength("R") * unitFactor;
      BKC1LEp2_r[0] = cBKC1LEp2.getLength("r") * unitFactor;
      BKC1LEp2_r[1] = cBKC1LEp2.getLength("r") * unitFactor;

      double BKC1LEp2_X0 = cBKC1LEp2.getLength("X0") * unitFactor;
      double BKC1LEp2_Z0 = cBKC1LEp2.getLength("Z0") * unitFactor;
      double BKC1LEp2_PHI = cBKC1LEp2.getLength("PHI");
      G4Transform3D transform_BKC1LEp2 = G4Translate3D(BKC1LEp2_X0, 0.0, BKC1LEp2_Z0);
      transform_BKC1LEp2 = transform_BKC1LEp2 * G4RotateY3D(BKC1LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC1LEp2 = new G4Polycone("geo_BKC1LEp2xx_name", 0.0, 2 * M_PI, N, BKC1LEp2_Z, BKC1LEp2_r, BKC1LEp2_R);

      string strMat_BKC1LEp2 = cBKC1LEp2.getString("Material");
      G4Material* mat_BKC1LEp2 = Materials::get(strMat_BKC1LEp2);
      G4LogicalVolume* logi_BKC1LEp2 = new G4LogicalVolume(geo_BKC1LEp2, mat_BKC1LEp2, "logi_BKC1LEp2_name");

      //put volume
      setColor(*logi_BKC1LEp2, cBKC1LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC1LEp2, false);
      new G4PVPlacement(transform_BKC1LEp2, logi_BKC1LEp2, "phys_BKC1LEp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC2LEp1

      //get parameters from .xml file
      GearDir cBKC2LEp1(content, "BKC2LEp1/");

      double BKC2LEp1_Z[N];
      double BKC2LEp1_R[N];
      double BKC2LEp1_r[N];
      BKC2LEp1_Z[0] = 0;
      BKC2LEp1_Z[1] = cBKC2LEp1.getLength("L") * unitFactor;
      BKC2LEp1_R[0] = cBKC2LEp1.getLength("R") * unitFactor;
      BKC2LEp1_R[1] = cBKC2LEp1.getLength("R") * unitFactor;
      BKC2LEp1_r[0] = cBKC2LEp1.getLength("r") * unitFactor;
      BKC2LEp1_r[1] = cBKC2LEp1.getLength("r") * unitFactor;

      double BKC2LEp1_X0 = cBKC2LEp1.getLength("X0") * unitFactor;
      double BKC2LEp1_Z0 = cBKC2LEp1.getLength("Z0") * unitFactor;
      double BKC2LEp1_PHI = cBKC2LEp1.getLength("PHI");
      G4Transform3D transform_BKC2LEp1 = G4Translate3D(BKC2LEp1_X0, 0.0, BKC2LEp1_Z0);
      transform_BKC2LEp1 = transform_BKC2LEp1 * G4RotateY3D(BKC2LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC2LEp1 = new G4Polycone("geo_BKC2LEp1xx_name", 0.0, 2 * M_PI, N, BKC2LEp1_Z, BKC2LEp1_r, BKC2LEp1_R);

      string strMat_BKC2LEp1 = cBKC2LEp1.getString("Material");
      G4Material* mat_BKC2LEp1 = Materials::get(strMat_BKC2LEp1);
      G4LogicalVolume* logi_BKC2LEp1 = new G4LogicalVolume(geo_BKC2LEp1, mat_BKC2LEp1, "logi_BKC2LEp1_name");

      //put volume
      setColor(*logi_BKC2LEp1, cBKC2LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC2LEp1, false);
      new G4PVPlacement(transform_BKC2LEp1, logi_BKC2LEp1, "phys_BKC2LEp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC2LEp2

      //get parameters from .xml file
      GearDir cBKC2LEp2(content, "BKC2LEp2/");

      double BKC2LEp2_Z[N];
      double BKC2LEp2_R[N];
      double BKC2LEp2_r[N];
      BKC2LEp2_Z[0] = 0;
      BKC2LEp2_Z[1] = cBKC2LEp2.getLength("L") * unitFactor;
      BKC2LEp2_R[0] = cBKC2LEp2.getLength("R") * unitFactor;
      BKC2LEp2_R[1] = cBKC2LEp2.getLength("R") * unitFactor;
      BKC2LEp2_r[0] = cBKC2LEp2.getLength("r") * unitFactor;
      BKC2LEp2_r[1] = cBKC2LEp2.getLength("r") * unitFactor;

      double BKC2LEp2_X0 = cBKC2LEp2.getLength("X0") * unitFactor;
      double BKC2LEp2_Z0 = cBKC2LEp2.getLength("Z0") * unitFactor;
      double BKC2LEp2_PHI = cBKC2LEp2.getLength("PHI");
      G4Transform3D transform_BKC2LEp2 = G4Translate3D(BKC2LEp2_X0, 0.0, BKC2LEp2_Z0);
      transform_BKC2LEp2 = transform_BKC2LEp2 * G4RotateY3D(BKC2LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC2LEp2 = new G4Polycone("geo_BKC2LEp2xx_name", 0.0, 2 * M_PI, N, BKC2LEp2_Z, BKC2LEp2_r, BKC2LEp2_R);

      string strMat_BKC2LEp2 = cBKC2LEp2.getString("Material");
      G4Material* mat_BKC2LEp2 = Materials::get(strMat_BKC2LEp2);
      G4LogicalVolume* logi_BKC2LEp2 = new G4LogicalVolume(geo_BKC2LEp2, mat_BKC2LEp2, "logi_BKC2LEp2_name");

      //put volume
      setColor(*logi_BKC2LEp2, cBKC2LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC2LEp2, false);
      new G4PVPlacement(transform_BKC2LEp2, logi_BKC2LEp2, "phys_BKC2LEp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC3LEp1

      //get parameters from .xml file
      GearDir cBKC3LEp1(content, "BKC3LEp1/");

      double BKC3LEp1_Z[N];
      double BKC3LEp1_R[N];
      double BKC3LEp1_r[N];
      BKC3LEp1_Z[0] = 0;
      BKC3LEp1_Z[1] = cBKC3LEp1.getLength("L") * unitFactor;
      BKC3LEp1_R[0] = cBKC3LEp1.getLength("R") * unitFactor;
      BKC3LEp1_R[1] = cBKC3LEp1.getLength("R") * unitFactor;
      BKC3LEp1_r[0] = cBKC3LEp1.getLength("r") * unitFactor;
      BKC3LEp1_r[1] = cBKC3LEp1.getLength("r") * unitFactor;

      double BKC3LEp1_X0 = cBKC3LEp1.getLength("X0") * unitFactor;
      double BKC3LEp1_Z0 = cBKC3LEp1.getLength("Z0") * unitFactor;
      double BKC3LEp1_PHI = cBKC3LEp1.getLength("PHI");
      G4Transform3D transform_BKC3LEp1 = G4Translate3D(BKC3LEp1_X0, 0.0, BKC3LEp1_Z0);
      transform_BKC3LEp1 = transform_BKC3LEp1 * G4RotateY3D(BKC3LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC3LEp1 = new G4Polycone("geo_BKC3LEp1xx_name", 0.0, 2 * M_PI, N, BKC3LEp1_Z, BKC3LEp1_r, BKC3LEp1_R);

      string strMat_BKC3LEp1 = cBKC3LEp1.getString("Material");
      G4Material* mat_BKC3LEp1 = Materials::get(strMat_BKC3LEp1);
      G4LogicalVolume* logi_BKC3LEp1 = new G4LogicalVolume(geo_BKC3LEp1, mat_BKC3LEp1, "logi_BKC3LEp1_name");

      //put volume
      setColor(*logi_BKC3LEp1, cBKC3LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC3LEp1, false);
      new G4PVPlacement(transform_BKC3LEp1, logi_BKC3LEp1, "phys_BKC3LEp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC3LEp2

      //get parameters from .xml file
      GearDir cBKC3LEp2(content, "BKC3LEp2/");

      double BKC3LEp2_Z[N];
      double BKC3LEp2_R[N];
      double BKC3LEp2_r[N];
      BKC3LEp2_Z[0] = 0;
      BKC3LEp2_Z[1] = cBKC3LEp2.getLength("L") * unitFactor;
      BKC3LEp2_R[0] = cBKC3LEp2.getLength("R") * unitFactor;
      BKC3LEp2_R[1] = cBKC3LEp2.getLength("R") * unitFactor;
      BKC3LEp2_r[0] = cBKC3LEp2.getLength("r") * unitFactor;
      BKC3LEp2_r[1] = cBKC3LEp2.getLength("r") * unitFactor;

      double BKC3LEp2_X0 = cBKC3LEp2.getLength("X0") * unitFactor;
      double BKC3LEp2_Z0 = cBKC3LEp2.getLength("Z0") * unitFactor;
      double BKC3LEp2_PHI = cBKC3LEp2.getLength("PHI");
      G4Transform3D transform_BKC3LEp2 = G4Translate3D(BKC3LEp2_X0, 0.0, BKC3LEp2_Z0);
      transform_BKC3LEp2 = transform_BKC3LEp2 * G4RotateY3D(BKC3LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC3LEp2 = new G4Polycone("geo_BKC3LEp2xx_name", 0.0, 2 * M_PI, N, BKC3LEp2_Z, BKC3LEp2_r, BKC3LEp2_R);

      string strMat_BKC3LEp2 = cBKC3LEp2.getString("Material");
      G4Material* mat_BKC3LEp2 = Materials::get(strMat_BKC3LEp2);
      G4LogicalVolume* logi_BKC3LEp2 = new G4LogicalVolume(geo_BKC3LEp2, mat_BKC3LEp2, "logi_BKC3LEp2_name");

      //put volume
      setColor(*logi_BKC3LEp2, cBKC3LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC3LEp2, false);
      new G4PVPlacement(transform_BKC3LEp2, logi_BKC3LEp2, "phys_BKC3LEp2_name", &topVolume, false, 0);

      //--------------
      //-   BKC4LEp1

      //get parameters from .xml file
      GearDir cBKC4LEp1(content, "BKC4LEp1/");

      double BKC4LEp1_Z[N];
      double BKC4LEp1_R[N];
      double BKC4LEp1_r[N];
      BKC4LEp1_Z[0] = 0;
      BKC4LEp1_Z[1] = cBKC4LEp1.getLength("L") * unitFactor;
      BKC4LEp1_R[0] = cBKC4LEp1.getLength("R") * unitFactor;
      BKC4LEp1_R[1] = cBKC4LEp1.getLength("R") * unitFactor;
      BKC4LEp1_r[0] = cBKC4LEp1.getLength("r") * unitFactor;
      BKC4LEp1_r[1] = cBKC4LEp1.getLength("r") * unitFactor;

      double BKC4LEp1_X0 = cBKC4LEp1.getLength("X0") * unitFactor;
      double BKC4LEp1_Z0 = cBKC4LEp1.getLength("Z0") * unitFactor;
      double BKC4LEp1_PHI = cBKC4LEp1.getLength("PHI");
      G4Transform3D transform_BKC4LEp1 = G4Translate3D(BKC4LEp1_X0, 0.0, BKC4LEp1_Z0);
      transform_BKC4LEp1 = transform_BKC4LEp1 * G4RotateY3D(BKC4LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC4LEp1 = new G4Polycone("geo_BKC4LEp1xx_name", 0.0, 2 * M_PI, N, BKC4LEp1_Z, BKC4LEp1_r, BKC4LEp1_R);

      string strMat_BKC4LEp1 = cBKC4LEp1.getString("Material");
      G4Material* mat_BKC4LEp1 = Materials::get(strMat_BKC4LEp1);
      G4LogicalVolume* logi_BKC4LEp1 = new G4LogicalVolume(geo_BKC4LEp1, mat_BKC4LEp1, "logi_BKC4LEp1_name");

      //put volume
      setColor(*logi_BKC4LEp1, cBKC4LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC4LEp1, false);
      new G4PVPlacement(transform_BKC4LEp1, logi_BKC4LEp1, "phys_BKC4LEp1_name", &topVolume, false, 0);

      //--------------
      //-   BKC4LEp2

      //get parameters from .xml file
      GearDir cBKC4LEp2(content, "BKC4LEp2/");

      double BKC4LEp2_Z[N];
      double BKC4LEp2_R[N];
      double BKC4LEp2_r[N];
      BKC4LEp2_Z[0] = 0;
      BKC4LEp2_Z[1] = cBKC4LEp2.getLength("L") * unitFactor;
      BKC4LEp2_R[0] = cBKC4LEp2.getLength("R") * unitFactor;
      BKC4LEp2_R[1] = cBKC4LEp2.getLength("R") * unitFactor;
      BKC4LEp2_r[0] = cBKC4LEp2.getLength("r") * unitFactor;
      BKC4LEp2_r[1] = cBKC4LEp2.getLength("r") * unitFactor;

      double BKC4LEp2_X0 = cBKC4LEp2.getLength("X0") * unitFactor;
      double BKC4LEp2_Z0 = cBKC4LEp2.getLength("Z0") * unitFactor;
      double BKC4LEp2_PHI = cBKC4LEp2.getLength("PHI");
      G4Transform3D transform_BKC4LEp2 = G4Translate3D(BKC4LEp2_X0, 0.0, BKC4LEp2_Z0);
      transform_BKC4LEp2 = transform_BKC4LEp2 * G4RotateY3D(BKC4LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_BKC4LEp2 = new G4Polycone("geo_BKC4LEp2xx_name", 0.0, 2 * M_PI, N, BKC4LEp2_Z, BKC4LEp2_r, BKC4LEp2_R);

      string strMat_BKC4LEp2 = cBKC4LEp2.getString("Material");
      G4Material* mat_BKC4LEp2 = Materials::get(strMat_BKC4LEp2);
      G4LogicalVolume* logi_BKC4LEp2 = new G4LogicalVolume(geo_BKC4LEp2, mat_BKC4LEp2, "logi_BKC4LEp2_name");

      //put volume
      setColor(*logi_BKC4LEp2, cBKC4LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_BKC4LEp2, false);
      new G4PVPlacement(transform_BKC4LEp2, logi_BKC4LEp2, "phys_BKC4LEp2_name", &topVolume, false, 0);

      //--------------
      //-   QKAREp1

      //get parameters from .xml file
      GearDir cQKAREp1(content, "QKAREp1/");

      double QKAREp1_Z[N];
      double QKAREp1_R[N];
      double QKAREp1_r[N];
      QKAREp1_Z[0] = 0;
      QKAREp1_Z[1] = cQKAREp1.getLength("L") * unitFactor;
      QKAREp1_R[0] = cQKAREp1.getLength("R") * unitFactor;
      QKAREp1_R[1] = cQKAREp1.getLength("R") * unitFactor;
      QKAREp1_r[0] = cQKAREp1.getLength("r") * unitFactor;
      QKAREp1_r[1] = cQKAREp1.getLength("r") * unitFactor;

      double QKAREp1_X0 = cQKAREp1.getLength("X0") * unitFactor;
      double QKAREp1_Z0 = cQKAREp1.getLength("Z0") * unitFactor;
      double QKAREp1_PHI = cQKAREp1.getLength("PHI");
      G4Transform3D transform_QKAREp1 = G4Translate3D(QKAREp1_X0, 0.0, QKAREp1_Z0);
      transform_QKAREp1 = transform_QKAREp1 * G4RotateY3D(QKAREp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKAREp1 = new G4Polycone("geo_QKAREp1xx_name", 0.0, 2 * M_PI, N, QKAREp1_Z, QKAREp1_r, QKAREp1_R);

      string strMat_QKAREp1 = cQKAREp1.getString("Material");
      G4Material* mat_QKAREp1 = Materials::get(strMat_QKAREp1);
      G4LogicalVolume* logi_QKAREp1 = new G4LogicalVolume(geo_QKAREp1, mat_QKAREp1, "logi_QKAREp1_name");

      //put volume
      setColor(*logi_QKAREp1, cQKAREp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKAREp1, false);
      new G4PVPlacement(transform_QKAREp1, logi_QKAREp1, "phys_QKAREp1_name", &topVolume, false, 0);

      //--------------
      //-   QKAREp2

      //get parameters from .xml file
      GearDir cQKAREp2(content, "QKAREp2/");

      double QKAREp2_Z[N];
      double QKAREp2_R[N];
      double QKAREp2_r[N];
      QKAREp2_Z[0] = 0;
      QKAREp2_Z[1] = cQKAREp2.getLength("L") * unitFactor;
      QKAREp2_R[0] = cQKAREp2.getLength("R") * unitFactor;
      QKAREp2_R[1] = cQKAREp2.getLength("R") * unitFactor;
      QKAREp2_r[0] = cQKAREp2.getLength("r") * unitFactor;
      QKAREp2_r[1] = cQKAREp2.getLength("r") * unitFactor;

      double QKAREp2_X0 = cQKAREp2.getLength("X0") * unitFactor;
      double QKAREp2_Z0 = cQKAREp2.getLength("Z0") * unitFactor;
      double QKAREp2_PHI = cQKAREp2.getLength("PHI");
      G4Transform3D transform_QKAREp2 = G4Translate3D(QKAREp2_X0, 0.0, QKAREp2_Z0);
      transform_QKAREp2 = transform_QKAREp2 * G4RotateY3D(QKAREp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKAREp2xx = new G4Polycone("geo_QKAREp2xx_name", 0.0, 2 * M_PI, N, QKAREp2_Z, QKAREp2_r, QKAREp2_R);
      G4SubtractionSolid* geo_QKAREp2 = new G4SubtractionSolid("geo_QKAREp2_name", geo_QKAREp2xx, geo_BC1RPp2, transform_QKAREp2.inverse()*transform_BC1RPtube);

      string strMat_QKAREp2 = cQKAREp2.getString("Material");
      G4Material* mat_QKAREp2 = Materials::get(strMat_QKAREp2);
      G4LogicalVolume* logi_QKAREp2 = new G4LogicalVolume(geo_QKAREp2, mat_QKAREp2, "logi_QKAREp2_name");

      //put volume
      setColor(*logi_QKAREp2, cQKAREp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKAREp2, false);
      new G4PVPlacement(transform_QKAREp2, logi_QKAREp2, "phys_QKAREp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC2REp1

      //get parameters from .xml file
      GearDir cQLC2REp1(content, "QLC2REp1/");

      double QLC2REp1_Z[N];
      double QLC2REp1_R[N];
      double QLC2REp1_r[N];
      QLC2REp1_Z[0] = 0;
      QLC2REp1_Z[1] = cQLC2REp1.getLength("L") * unitFactor;
      QLC2REp1_R[0] = cQLC2REp1.getLength("R") * unitFactor;
      QLC2REp1_R[1] = cQLC2REp1.getLength("R") * unitFactor;
      QLC2REp1_r[0] = cQLC2REp1.getLength("r") * unitFactor;
      QLC2REp1_r[1] = cQLC2REp1.getLength("r") * unitFactor;

      double QLC2REp1_X0 = cQLC2REp1.getLength("X0") * unitFactor;
      double QLC2REp1_Z0 = cQLC2REp1.getLength("Z0") * unitFactor;
      double QLC2REp1_PHI = cQLC2REp1.getLength("PHI");
      G4Transform3D transform_QLC2REp1 = G4Translate3D(QLC2REp1_X0, 0.0, QLC2REp1_Z0);
      transform_QLC2REp1 = transform_QLC2REp1 * G4RotateY3D(QLC2REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2REp1 = new G4Polycone("geo_QLC2REp1xx_name", 0.0, 2 * M_PI, N, QLC2REp1_Z, QLC2REp1_r, QLC2REp1_R);

      string strMat_QLC2REp1 = cQLC2REp1.getString("Material");
      G4Material* mat_QLC2REp1 = Materials::get(strMat_QLC2REp1);
      G4LogicalVolume* logi_QLC2REp1 = new G4LogicalVolume(geo_QLC2REp1, mat_QLC2REp1, "logi_QLC2REp1_name");

      //put volume
      setColor(*logi_QLC2REp1, cQLC2REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2REp1, false);
      new G4PVPlacement(transform_QLC2REp1, logi_QLC2REp1, "phys_QLC2REp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC2REp2

      //get parameters from .xml file
      GearDir cQLC2REp2(content, "QLC2REp2/");

      double QLC2REp2_Z[N];
      double QLC2REp2_R[N];
      double QLC2REp2_r[N];
      QLC2REp2_Z[0] = 0;
      QLC2REp2_Z[1] = cQLC2REp2.getLength("L") * unitFactor;
      QLC2REp2_R[0] = cQLC2REp2.getLength("R") * unitFactor;
      QLC2REp2_R[1] = cQLC2REp2.getLength("R") * unitFactor;
      QLC2REp2_r[0] = cQLC2REp2.getLength("r") * unitFactor;
      QLC2REp2_r[1] = cQLC2REp2.getLength("r") * unitFactor;

      double QLC2REp2_X0 = cQLC2REp2.getLength("X0") * unitFactor;
      double QLC2REp2_Z0 = cQLC2REp2.getLength("Z0") * unitFactor;
      double QLC2REp2_PHI = cQLC2REp2.getLength("PHI");
      G4Transform3D transform_QLC2REp2 = G4Translate3D(QLC2REp2_X0, 0.0, QLC2REp2_Z0);
      transform_QLC2REp2 = transform_QLC2REp2 * G4RotateY3D(QLC2REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2REp2 = new G4Polycone("geo_QLC2REp2xx_name", 0.0, 2 * M_PI, N, QLC2REp2_Z, QLC2REp2_r, QLC2REp2_R);

      string strMat_QLC2REp2 = cQLC2REp2.getString("Material");
      G4Material* mat_QLC2REp2 = Materials::get(strMat_QLC2REp2);
      G4LogicalVolume* logi_QLC2REp2 = new G4LogicalVolume(geo_QLC2REp2, mat_QLC2REp2, "logi_QLC2REp2_name");

      //put volume
      setColor(*logi_QLC2REp2, cQLC2REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2REp2, false);
      new G4PVPlacement(transform_QLC2REp2, logi_QLC2REp2, "phys_QLC2REp2_name", &topVolume, false, 0);

      //--------------
      //-   QKBREp1

      //get parameters from .xml file
      GearDir cQKBREp1(content, "QKBREp1/");

      double QKBREp1_Z[N];
      double QKBREp1_R[N];
      double QKBREp1_r[N];
      QKBREp1_Z[0] = 0;
      QKBREp1_Z[1] = cQKBREp1.getLength("L") * unitFactor;
      QKBREp1_R[0] = cQKBREp1.getLength("R") * unitFactor;
      QKBREp1_R[1] = cQKBREp1.getLength("R") * unitFactor;
      QKBREp1_r[0] = cQKBREp1.getLength("r") * unitFactor;
      QKBREp1_r[1] = cQKBREp1.getLength("r") * unitFactor;

      double QKBREp1_X0 = cQKBREp1.getLength("X0") * unitFactor;
      double QKBREp1_Z0 = cQKBREp1.getLength("Z0") * unitFactor;
      double QKBREp1_PHI = cQKBREp1.getLength("PHI");
      G4Transform3D transform_QKBREp1 = G4Translate3D(QKBREp1_X0, 0.0, QKBREp1_Z0);
      transform_QKBREp1 = transform_QKBREp1 * G4RotateY3D(QKBREp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBREp1 = new G4Polycone("geo_QKBREp1xx_name", 0.0, 2 * M_PI, N, QKBREp1_Z, QKBREp1_r, QKBREp1_R);

      string strMat_QKBREp1 = cQKBREp1.getString("Material");
      G4Material* mat_QKBREp1 = Materials::get(strMat_QKBREp1);
      G4LogicalVolume* logi_QKBREp1 = new G4LogicalVolume(geo_QKBREp1, mat_QKBREp1, "logi_QKBREp1_name");

      //put volume
      setColor(*logi_QKBREp1, cQKBREp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBREp1, false);
      new G4PVPlacement(transform_QKBREp1, logi_QKBREp1, "phys_QKBREp1_name", &topVolume, false, 0);

      //--------------
      //-   QKBREp2

      //get parameters from .xml file
      GearDir cQKBREp2(content, "QKBREp2/");

      double QKBREp2_Z[N];
      double QKBREp2_R[N];
      double QKBREp2_r[N];
      QKBREp2_Z[0] = 0;
      QKBREp2_Z[1] = cQKBREp2.getLength("L") * unitFactor;
      QKBREp2_R[0] = cQKBREp2.getLength("R") * unitFactor;
      QKBREp2_R[1] = cQKBREp2.getLength("R") * unitFactor;
      QKBREp2_r[0] = cQKBREp2.getLength("r") * unitFactor;
      QKBREp2_r[1] = cQKBREp2.getLength("r") * unitFactor;

      double QKBREp2_X0 = cQKBREp2.getLength("X0") * unitFactor;
      double QKBREp2_Z0 = cQKBREp2.getLength("Z0") * unitFactor;
      double QKBREp2_PHI = cQKBREp2.getLength("PHI");
      G4Transform3D transform_QKBREp2 = G4Translate3D(QKBREp2_X0, 0.0, QKBREp2_Z0);
      transform_QKBREp2 = transform_QKBREp2 * G4RotateY3D(QKBREp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBREp2 = new G4Polycone("geo_QKBREp2xx_name", 0.0, 2 * M_PI, N, QKBREp2_Z, QKBREp2_r, QKBREp2_R);

      string strMat_QKBREp2 = cQKBREp2.getString("Material");
      G4Material* mat_QKBREp2 = Materials::get(strMat_QKBREp2);
      G4LogicalVolume* logi_QKBREp2 = new G4LogicalVolume(geo_QKBREp2, mat_QKBREp2, "logi_QKBREp2_name");

      //put volume
      setColor(*logi_QKBREp2, cQKBREp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBREp2, false);
      new G4PVPlacement(transform_QKBREp2, logi_QKBREp2, "phys_QKBREp2_name", &topVolume, false, 0);

      //--------------
      //-   QKCREp1

      //get parameters from .xml file
      GearDir cQKCREp1(content, "QKCREp1/");

      double QKCREp1_Z[N];
      double QKCREp1_R[N];
      double QKCREp1_r[N];
      QKCREp1_Z[0] = 0;
      QKCREp1_Z[1] = cQKCREp1.getLength("L") * unitFactor;
      QKCREp1_R[0] = cQKCREp1.getLength("R") * unitFactor;
      QKCREp1_R[1] = cQKCREp1.getLength("R") * unitFactor;
      QKCREp1_r[0] = cQKCREp1.getLength("r") * unitFactor;
      QKCREp1_r[1] = cQKCREp1.getLength("r") * unitFactor;

      double QKCREp1_X0 = cQKCREp1.getLength("X0") * unitFactor;
      double QKCREp1_Z0 = cQKCREp1.getLength("Z0") * unitFactor;
      double QKCREp1_PHI = cQKCREp1.getLength("PHI");
      G4Transform3D transform_QKCREp1 = G4Translate3D(QKCREp1_X0, 0.0, QKCREp1_Z0);
      transform_QKCREp1 = transform_QKCREp1 * G4RotateY3D(QKCREp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCREp1 = new G4Polycone("geo_QKCREp1xx_name", 0.0, 2 * M_PI, N, QKCREp1_Z, QKCREp1_r, QKCREp1_R);

      string strMat_QKCREp1 = cQKCREp1.getString("Material");
      G4Material* mat_QKCREp1 = Materials::get(strMat_QKCREp1);
      G4LogicalVolume* logi_QKCREp1 = new G4LogicalVolume(geo_QKCREp1, mat_QKCREp1, "logi_QKCREp1_name");

      //put volume
      setColor(*logi_QKCREp1, cQKCREp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCREp1, false);
      new G4PVPlacement(transform_QKCREp1, logi_QKCREp1, "phys_QKCREp1_name", &topVolume, false, 0);

      //--------------
      //-   QKCREp2

      //get parameters from .xml file
      GearDir cQKCREp2(content, "QKCREp2/");

      double QKCREp2_Z[N];
      double QKCREp2_R[N];
      double QKCREp2_r[N];
      QKCREp2_Z[0] = 0;
      QKCREp2_Z[1] = cQKCREp2.getLength("L") * unitFactor;
      QKCREp2_R[0] = cQKCREp2.getLength("R") * unitFactor;
      QKCREp2_R[1] = cQKCREp2.getLength("R") * unitFactor;
      QKCREp2_r[0] = cQKCREp2.getLength("r") * unitFactor;
      QKCREp2_r[1] = cQKCREp2.getLength("r") * unitFactor;

      double QKCREp2_X0 = cQKCREp2.getLength("X0") * unitFactor;
      double QKCREp2_Z0 = cQKCREp2.getLength("Z0") * unitFactor;
      double QKCREp2_PHI = cQKCREp2.getLength("PHI");
      G4Transform3D transform_QKCREp2 = G4Translate3D(QKCREp2_X0, 0.0, QKCREp2_Z0);
      transform_QKCREp2 = transform_QKCREp2 * G4RotateY3D(QKCREp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCREp2 = new G4Polycone("geo_QKCREp2xx_name", 0.0, 2 * M_PI, N, QKCREp2_Z, QKCREp2_r, QKCREp2_R);

      string strMat_QKCREp2 = cQKCREp2.getString("Material");
      G4Material* mat_QKCREp2 = Materials::get(strMat_QKCREp2);
      G4LogicalVolume* logi_QKCREp2 = new G4LogicalVolume(geo_QKCREp2, mat_QKCREp2, "logi_QKCREp2_name");

      //put volume
      setColor(*logi_QKCREp2, cQKCREp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCREp2, false);
      new G4PVPlacement(transform_QKCREp2, logi_QKCREp2, "phys_QKCREp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC3REp1

      //get parameters from .xml file
      GearDir cQLC3REp1(content, "QLC3REp1/");

      double QLC3REp1_Z[N];
      double QLC3REp1_R[N];
      double QLC3REp1_r[N];
      QLC3REp1_Z[0] = 0;
      QLC3REp1_Z[1] = cQLC3REp1.getLength("L") * unitFactor;
      QLC3REp1_R[0] = cQLC3REp1.getLength("R") * unitFactor;
      QLC3REp1_R[1] = cQLC3REp1.getLength("R") * unitFactor;
      QLC3REp1_r[0] = cQLC3REp1.getLength("r") * unitFactor;
      QLC3REp1_r[1] = cQLC3REp1.getLength("r") * unitFactor;

      double QLC3REp1_X0 = cQLC3REp1.getLength("X0") * unitFactor;
      double QLC3REp1_Z0 = cQLC3REp1.getLength("Z0") * unitFactor;
      double QLC3REp1_PHI = cQLC3REp1.getLength("PHI");
      G4Transform3D transform_QLC3REp1 = G4Translate3D(QLC3REp1_X0, 0.0, QLC3REp1_Z0);
      transform_QLC3REp1 = transform_QLC3REp1 * G4RotateY3D(QLC3REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3REp1 = new G4Polycone("geo_QLC3REp1xx_name", 0.0, 2 * M_PI, N, QLC3REp1_Z, QLC3REp1_r, QLC3REp1_R);

      string strMat_QLC3REp1 = cQLC3REp1.getString("Material");
      G4Material* mat_QLC3REp1 = Materials::get(strMat_QLC3REp1);
      G4LogicalVolume* logi_QLC3REp1 = new G4LogicalVolume(geo_QLC3REp1, mat_QLC3REp1, "logi_QLC3REp1_name");

      //put volume
      setColor(*logi_QLC3REp1, cQLC3REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3REp1, false);
      new G4PVPlacement(transform_QLC3REp1, logi_QLC3REp1, "phys_QLC3REp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC3REp2

      //get parameters from .xml file
      GearDir cQLC3REp2(content, "QLC3REp2/");

      double QLC3REp2_Z[N];
      double QLC3REp2_R[N];
      double QLC3REp2_r[N];
      QLC3REp2_Z[0] = 0;
      QLC3REp2_Z[1] = cQLC3REp2.getLength("L") * unitFactor;
      QLC3REp2_R[0] = cQLC3REp2.getLength("R") * unitFactor;
      QLC3REp2_R[1] = cQLC3REp2.getLength("R") * unitFactor;
      QLC3REp2_r[0] = cQLC3REp2.getLength("r") * unitFactor;
      QLC3REp2_r[1] = cQLC3REp2.getLength("r") * unitFactor;

      double QLC3REp2_X0 = cQLC3REp2.getLength("X0") * unitFactor;
      double QLC3REp2_Z0 = cQLC3REp2.getLength("Z0") * unitFactor;
      double QLC3REp2_PHI = cQLC3REp2.getLength("PHI");
      G4Transform3D transform_QLC3REp2 = G4Translate3D(QLC3REp2_X0, 0.0, QLC3REp2_Z0);
      transform_QLC3REp2 = transform_QLC3REp2 * G4RotateY3D(QLC3REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3REp2 = new G4Polycone("geo_QLC3REp2xx_name", 0.0, 2 * M_PI, N, QLC3REp2_Z, QLC3REp2_r, QLC3REp2_R);

      string strMat_QLC3REp2 = cQLC3REp2.getString("Material");
      G4Material* mat_QLC3REp2 = Materials::get(strMat_QLC3REp2);
      G4LogicalVolume* logi_QLC3REp2 = new G4LogicalVolume(geo_QLC3REp2, mat_QLC3REp2, "logi_QLC3REp2_name");

      //put volume
      setColor(*logi_QLC3REp2, cQLC3REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3REp2, false);
      new G4PVPlacement(transform_QLC3REp2, logi_QLC3REp2, "phys_QLC3REp2_name", &topVolume, false, 0);

      //--------------
      //-   QKDREp1

      //get parameters from .xml file
      GearDir cQKDREp1(content, "QKDREp1/");

      double QKDREp1_Z[N];
      double QKDREp1_R[N];
      double QKDREp1_r[N];
      QKDREp1_Z[0] = 0;
      QKDREp1_Z[1] = cQKDREp1.getLength("L") * unitFactor;
      QKDREp1_R[0] = cQKDREp1.getLength("R") * unitFactor;
      QKDREp1_R[1] = cQKDREp1.getLength("R") * unitFactor;
      QKDREp1_r[0] = cQKDREp1.getLength("r") * unitFactor;
      QKDREp1_r[1] = cQKDREp1.getLength("r") * unitFactor;

      double QKDREp1_X0 = cQKDREp1.getLength("X0") * unitFactor;
      double QKDREp1_Z0 = cQKDREp1.getLength("Z0") * unitFactor;
      double QKDREp1_PHI = cQKDREp1.getLength("PHI");
      G4Transform3D transform_QKDREp1 = G4Translate3D(QKDREp1_X0, 0.0, QKDREp1_Z0);
      transform_QKDREp1 = transform_QKDREp1 * G4RotateY3D(QKDREp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKDREp1 = new G4Polycone("geo_QKDREp1xx_name", 0.0, 2 * M_PI, N, QKDREp1_Z, QKDREp1_r, QKDREp1_R);

      string strMat_QKDREp1 = cQKDREp1.getString("Material");
      G4Material* mat_QKDREp1 = Materials::get(strMat_QKDREp1);
      G4LogicalVolume* logi_QKDREp1 = new G4LogicalVolume(geo_QKDREp1, mat_QKDREp1, "logi_QKDREp1_name");

      //put volume
      setColor(*logi_QKDREp1, cQKDREp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKDREp1, false);
      new G4PVPlacement(transform_QKDREp1, logi_QKDREp1, "phys_QKDREp1_name", &topVolume, false, 0);

      //--------------
      //-   QKDREp2

      //get parameters from .xml file
      GearDir cQKDREp2(content, "QKDREp2/");

      double QKDREp2_Z[N];
      double QKDREp2_R[N];
      double QKDREp2_r[N];
      QKDREp2_Z[0] = 0;
      QKDREp2_Z[1] = cQKDREp2.getLength("L") * unitFactor;
      QKDREp2_R[0] = cQKDREp2.getLength("R") * unitFactor;
      QKDREp2_R[1] = cQKDREp2.getLength("R") * unitFactor;
      QKDREp2_r[0] = cQKDREp2.getLength("r") * unitFactor;
      QKDREp2_r[1] = cQKDREp2.getLength("r") * unitFactor;

      double QKDREp2_X0 = cQKDREp2.getLength("X0") * unitFactor;
      double QKDREp2_Z0 = cQKDREp2.getLength("Z0") * unitFactor;
      double QKDREp2_PHI = cQKDREp2.getLength("PHI");
      G4Transform3D transform_QKDREp2 = G4Translate3D(QKDREp2_X0, 0.0, QKDREp2_Z0);
      transform_QKDREp2 = transform_QKDREp2 * G4RotateY3D(QKDREp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKDREp2 = new G4Polycone("geo_QKDREp2xx_name", 0.0, 2 * M_PI, N, QKDREp2_Z, QKDREp2_r, QKDREp2_R);

      string strMat_QKDREp2 = cQKDREp2.getString("Material");
      G4Material* mat_QKDREp2 = Materials::get(strMat_QKDREp2);
      G4LogicalVolume* logi_QKDREp2 = new G4LogicalVolume(geo_QKDREp2, mat_QKDREp2, "logi_QKDREp2_name");

      //put volume
      setColor(*logi_QKDREp2, cQKDREp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKDREp2, false);
      new G4PVPlacement(transform_QKDREp2, logi_QKDREp2, "phys_QKDREp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC5REp1

      //get parameters from .xml file
      GearDir cQLC5REp1(content, "QLC5REp1/");

      double QLC5REp1_Z[N];
      double QLC5REp1_R[N];
      double QLC5REp1_r[N];
      QLC5REp1_Z[0] = 0;
      QLC5REp1_Z[1] = cQLC5REp1.getLength("L") * unitFactor;
      QLC5REp1_R[0] = cQLC5REp1.getLength("R") * unitFactor;
      QLC5REp1_R[1] = cQLC5REp1.getLength("R") * unitFactor;
      QLC5REp1_r[0] = cQLC5REp1.getLength("r") * unitFactor;
      QLC5REp1_r[1] = cQLC5REp1.getLength("r") * unitFactor;

      double QLC5REp1_X0 = cQLC5REp1.getLength("X0") * unitFactor;
      double QLC5REp1_Z0 = cQLC5REp1.getLength("Z0") * unitFactor;
      double QLC5REp1_PHI = cQLC5REp1.getLength("PHI");
      G4Transform3D transform_QLC5REp1 = G4Translate3D(QLC5REp1_X0, 0.0, QLC5REp1_Z0);
      transform_QLC5REp1 = transform_QLC5REp1 * G4RotateY3D(QLC5REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC5REp1 = new G4Polycone("geo_QLC5REp1xx_name", 0.0, 2 * M_PI, N, QLC5REp1_Z, QLC5REp1_r, QLC5REp1_R);

      string strMat_QLC5REp1 = cQLC5REp1.getString("Material");
      G4Material* mat_QLC5REp1 = Materials::get(strMat_QLC5REp1);
      G4LogicalVolume* logi_QLC5REp1 = new G4LogicalVolume(geo_QLC5REp1, mat_QLC5REp1, "logi_QLC5REp1_name");

      //put volume
      setColor(*logi_QLC5REp1, cQLC5REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC5REp1, false);
      new G4PVPlacement(transform_QLC5REp1, logi_QLC5REp1, "phys_QLC5REp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC5REp2

      //get parameters from .xml file
      GearDir cQLC5REp2(content, "QLC5REp2/");

      double QLC5REp2_Z[N];
      double QLC5REp2_R[N];
      double QLC5REp2_r[N];
      QLC5REp2_Z[0] = 0;
      QLC5REp2_Z[1] = cQLC5REp2.getLength("L") * unitFactor;
      QLC5REp2_R[0] = cQLC5REp2.getLength("R") * unitFactor;
      QLC5REp2_R[1] = cQLC5REp2.getLength("R") * unitFactor;
      QLC5REp2_r[0] = cQLC5REp2.getLength("r") * unitFactor;
      QLC5REp2_r[1] = cQLC5REp2.getLength("r") * unitFactor;

      double QLC5REp2_X0 = cQLC5REp2.getLength("X0") * unitFactor;
      double QLC5REp2_Z0 = cQLC5REp2.getLength("Z0") * unitFactor;
      double QLC5REp2_PHI = cQLC5REp2.getLength("PHI");
      G4Transform3D transform_QLC5REp2 = G4Translate3D(QLC5REp2_X0, 0.0, QLC5REp2_Z0);
      transform_QLC5REp2 = transform_QLC5REp2 * G4RotateY3D(QLC5REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC5REp2 = new G4Polycone("geo_QLC5REp2xx_name", 0.0, 2 * M_PI, N, QLC5REp2_Z, QLC5REp2_r, QLC5REp2_R);

      string strMat_QLC5REp2 = cQLC5REp2.getString("Material");
      G4Material* mat_QLC5REp2 = Materials::get(strMat_QLC5REp2);
      G4LogicalVolume* logi_QLC5REp2 = new G4LogicalVolume(geo_QLC5REp2, mat_QLC5REp2, "logi_QLC5REp2_name");

      //put volume
      setColor(*logi_QLC5REp2, cQLC5REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC5REp2, false);
      new G4PVPlacement(transform_QLC5REp2, logi_QLC5REp2, "phys_QLC5REp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC6REp1

      //get parameters from .xml file
      GearDir cQLC6REp1(content, "QLC6REp1/");

      double QLC6REp1_Z[N];
      double QLC6REp1_R[N];
      double QLC6REp1_r[N];
      QLC6REp1_Z[0] = 0;
      QLC6REp1_Z[1] = cQLC6REp1.getLength("L") * unitFactor;
      QLC6REp1_R[0] = cQLC6REp1.getLength("R") * unitFactor;
      QLC6REp1_R[1] = cQLC6REp1.getLength("R") * unitFactor;
      QLC6REp1_r[0] = cQLC6REp1.getLength("r") * unitFactor;
      QLC6REp1_r[1] = cQLC6REp1.getLength("r") * unitFactor;

      double QLC6REp1_X0 = cQLC6REp1.getLength("X0") * unitFactor;
      double QLC6REp1_Z0 = cQLC6REp1.getLength("Z0") * unitFactor;
      double QLC6REp1_PHI = cQLC6REp1.getLength("PHI");
      G4Transform3D transform_QLC6REp1 = G4Translate3D(QLC6REp1_X0, 0.0, QLC6REp1_Z0);
      transform_QLC6REp1 = transform_QLC6REp1 * G4RotateY3D(QLC6REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC6REp1 = new G4Polycone("geo_QLC6REp1xx_name", 0.0, 2 * M_PI, N, QLC6REp1_Z, QLC6REp1_r, QLC6REp1_R);

      string strMat_QLC6REp1 = cQLC6REp1.getString("Material");
      G4Material* mat_QLC6REp1 = Materials::get(strMat_QLC6REp1);
      G4LogicalVolume* logi_QLC6REp1 = new G4LogicalVolume(geo_QLC6REp1, mat_QLC6REp1, "logi_QLC6REp1_name");

      //put volume
      setColor(*logi_QLC6REp1, cQLC6REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC6REp1, false);
      new G4PVPlacement(transform_QLC6REp1, logi_QLC6REp1, "phys_QLC6REp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC6REp2

      //get parameters from .xml file
      GearDir cQLC6REp2(content, "QLC6REp2/");

      double QLC6REp2_Z[N];
      double QLC6REp2_R[N];
      double QLC6REp2_r[N];
      QLC6REp2_Z[0] = 0;
      QLC6REp2_Z[1] = cQLC6REp2.getLength("L") * unitFactor;
      QLC6REp2_R[0] = cQLC6REp2.getLength("R") * unitFactor;
      QLC6REp2_R[1] = cQLC6REp2.getLength("R") * unitFactor;
      QLC6REp2_r[0] = cQLC6REp2.getLength("r") * unitFactor;
      QLC6REp2_r[1] = cQLC6REp2.getLength("r") * unitFactor;

      double QLC6REp2_X0 = cQLC6REp2.getLength("X0") * unitFactor;
      double QLC6REp2_Z0 = cQLC6REp2.getLength("Z0") * unitFactor;
      double QLC6REp2_PHI = cQLC6REp2.getLength("PHI");
      G4Transform3D transform_QLC6REp2 = G4Translate3D(QLC6REp2_X0, 0.0, QLC6REp2_Z0);
      transform_QLC6REp2 = transform_QLC6REp2 * G4RotateY3D(QLC6REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC6REp2 = new G4Polycone("geo_QLC6REp2xx_name", 0.0, 2 * M_PI, N, QLC6REp2_Z, QLC6REp2_r, QLC6REp2_R);

      string strMat_QLC6REp2 = cQLC6REp2.getString("Material");
      G4Material* mat_QLC6REp2 = Materials::get(strMat_QLC6REp2);
      G4LogicalVolume* logi_QLC6REp2 = new G4LogicalVolume(geo_QLC6REp2, mat_QLC6REp2, "logi_QLC6REp2_name");

      //put volume
      setColor(*logi_QLC6REp2, cQLC6REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC6REp2, false);
      new G4PVPlacement(transform_QLC6REp2, logi_QLC6REp2, "phys_QLC6REp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC7REp1

      //get parameters from .xml file
      GearDir cQLC7REp1(content, "QLC7REp1/");

      double QLC7REp1_Z[N];
      double QLC7REp1_R[N];
      double QLC7REp1_r[N];
      QLC7REp1_Z[0] = 0;
      QLC7REp1_Z[1] = cQLC7REp1.getLength("L") * unitFactor;
      QLC7REp1_R[0] = cQLC7REp1.getLength("R") * unitFactor;
      QLC7REp1_R[1] = cQLC7REp1.getLength("R") * unitFactor;
      QLC7REp1_r[0] = cQLC7REp1.getLength("r") * unitFactor;
      QLC7REp1_r[1] = cQLC7REp1.getLength("r") * unitFactor;

      double QLC7REp1_X0 = cQLC7REp1.getLength("X0") * unitFactor;
      double QLC7REp1_Z0 = cQLC7REp1.getLength("Z0") * unitFactor;
      double QLC7REp1_PHI = cQLC7REp1.getLength("PHI");
      G4Transform3D transform_QLC7REp1 = G4Translate3D(QLC7REp1_X0, 0.0, QLC7REp1_Z0);
      transform_QLC7REp1 = transform_QLC7REp1 * G4RotateY3D(QLC7REp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC7REp1 = new G4Polycone("geo_QLC7REp1xx_name", 0.0, 2 * M_PI, N, QLC7REp1_Z, QLC7REp1_r, QLC7REp1_R);

      string strMat_QLC7REp1 = cQLC7REp1.getString("Material");
      G4Material* mat_QLC7REp1 = Materials::get(strMat_QLC7REp1);
      G4LogicalVolume* logi_QLC7REp1 = new G4LogicalVolume(geo_QLC7REp1, mat_QLC7REp1, "logi_QLC7REp1_name");

      //put volume
      setColor(*logi_QLC7REp1, cQLC7REp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC7REp1, false);
      new G4PVPlacement(transform_QLC7REp1, logi_QLC7REp1, "phys_QLC7REp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC7REp2

      //get parameters from .xml file
      GearDir cQLC7REp2(content, "QLC7REp2/");

      double QLC7REp2_Z[N];
      double QLC7REp2_R[N];
      double QLC7REp2_r[N];
      QLC7REp2_Z[0] = 0;
      QLC7REp2_Z[1] = cQLC7REp2.getLength("L") * unitFactor;
      QLC7REp2_R[0] = cQLC7REp2.getLength("R") * unitFactor;
      QLC7REp2_R[1] = cQLC7REp2.getLength("R") * unitFactor;
      QLC7REp2_r[0] = cQLC7REp2.getLength("r") * unitFactor;
      QLC7REp2_r[1] = cQLC7REp2.getLength("r") * unitFactor;

      double QLC7REp2_X0 = cQLC7REp2.getLength("X0") * unitFactor;
      double QLC7REp2_Z0 = cQLC7REp2.getLength("Z0") * unitFactor;
      double QLC7REp2_PHI = cQLC7REp2.getLength("PHI");
      G4Transform3D transform_QLC7REp2 = G4Translate3D(QLC7REp2_X0, 0.0, QLC7REp2_Z0);
      transform_QLC7REp2 = transform_QLC7REp2 * G4RotateY3D(QLC7REp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC7REp2 = new G4Polycone("geo_QLC7REp2xx_name", 0.0, 2 * M_PI, N, QLC7REp2_Z, QLC7REp2_r, QLC7REp2_R);

      string strMat_QLC7REp2 = cQLC7REp2.getString("Material");
      G4Material* mat_QLC7REp2 = Materials::get(strMat_QLC7REp2);
      G4LogicalVolume* logi_QLC7REp2 = new G4LogicalVolume(geo_QLC7REp2, mat_QLC7REp2, "logi_QLC7REp2_name");

      //put volume
      setColor(*logi_QLC7REp2, cQLC7REp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC7REp2, false);
      new G4PVPlacement(transform_QLC7REp2, logi_QLC7REp2, "phys_QLC7REp2_name", &topVolume, false, 0);

      //--------------
      //-   SLYTRE2p1

      //get parameters from .xml file
      GearDir cSLYTRE2p1(content, "SLYTRE2p1/");

      double SLYTRE2p1_Z[N];
      double SLYTRE2p1_R[N];
      double SLYTRE2p1_r[N];
      SLYTRE2p1_Z[0] = 0;
      SLYTRE2p1_Z[1] = cSLYTRE2p1.getLength("L") * unitFactor;
      SLYTRE2p1_R[0] = cSLYTRE2p1.getLength("R") * unitFactor;
      SLYTRE2p1_R[1] = cSLYTRE2p1.getLength("R") * unitFactor;
      SLYTRE2p1_r[0] = cSLYTRE2p1.getLength("r") * unitFactor;
      SLYTRE2p1_r[1] = cSLYTRE2p1.getLength("r") * unitFactor;

      double SLYTRE2p1_X0 = cSLYTRE2p1.getLength("X0") * unitFactor;
      double SLYTRE2p1_Z0 = cSLYTRE2p1.getLength("Z0") * unitFactor;
      double SLYTRE2p1_PHI = cSLYTRE2p1.getLength("PHI");
      G4Transform3D transform_SLYTRE2p1 = G4Translate3D(SLYTRE2p1_X0, 0.0, SLYTRE2p1_Z0);
      transform_SLYTRE2p1 = transform_SLYTRE2p1 * G4RotateY3D(SLYTRE2p1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTRE2p1 = new G4Polycone("geo_SLYTRE2p1xx_name", 0.0, 2 * M_PI, N, SLYTRE2p1_Z, SLYTRE2p1_r, SLYTRE2p1_R);

      string strMat_SLYTRE2p1 = cSLYTRE2p1.getString("Material");
      G4Material* mat_SLYTRE2p1 = Materials::get(strMat_SLYTRE2p1);
      G4LogicalVolume* logi_SLYTRE2p1 = new G4LogicalVolume(geo_SLYTRE2p1, mat_SLYTRE2p1, "logi_SLYTRE2p1_name");

      //put volume
      setColor(*logi_SLYTRE2p1, cSLYTRE2p1.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTRE2p1, false);
      new G4PVPlacement(transform_SLYTRE2p1, logi_SLYTRE2p1, "phys_SLYTRE2p1_name", &topVolume, false, 0);

      //--------------
      //-   SLYTRE2p2

      //get parameters from .xml file
      GearDir cSLYTRE2p2(content, "SLYTRE2p2/");

      double SLYTRE2p2_Z[N];
      double SLYTRE2p2_R[N];
      double SLYTRE2p2_r[N];
      SLYTRE2p2_Z[0] = 0;
      SLYTRE2p2_Z[1] = cSLYTRE2p2.getLength("L") * unitFactor;
      SLYTRE2p2_R[0] = cSLYTRE2p2.getLength("R") * unitFactor;
      SLYTRE2p2_R[1] = cSLYTRE2p2.getLength("R") * unitFactor;
      SLYTRE2p2_r[0] = cSLYTRE2p2.getLength("r") * unitFactor;
      SLYTRE2p2_r[1] = cSLYTRE2p2.getLength("r") * unitFactor;

      double SLYTRE2p2_X0 = cSLYTRE2p2.getLength("X0") * unitFactor;
      double SLYTRE2p2_Z0 = cSLYTRE2p2.getLength("Z0") * unitFactor;
      double SLYTRE2p2_PHI = cSLYTRE2p2.getLength("PHI");
      G4Transform3D transform_SLYTRE2p2 = G4Translate3D(SLYTRE2p2_X0, 0.0, SLYTRE2p2_Z0);
      transform_SLYTRE2p2 = transform_SLYTRE2p2 * G4RotateY3D(SLYTRE2p2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTRE2p2 = new G4Polycone("geo_SLYTRE2p2xx_name", 0.0, 2 * M_PI, N, SLYTRE2p2_Z, SLYTRE2p2_r, SLYTRE2p2_R);

      string strMat_SLYTRE2p2 = cSLYTRE2p2.getString("Material");
      G4Material* mat_SLYTRE2p2 = Materials::get(strMat_SLYTRE2p2);
      G4LogicalVolume* logi_SLYTRE2p2 = new G4LogicalVolume(geo_SLYTRE2p2, mat_SLYTRE2p2, "logi_SLYTRE2p2_name");

      //put volume
      setColor(*logi_SLYTRE2p2, cSLYTRE2p2.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTRE2p2, false);
      new G4PVPlacement(transform_SLYTRE2p2, logi_SLYTRE2p2, "phys_SLYTRE2p2_name", &topVolume, false, 0);

      //--------------
      //-   QLY2RE2p1

      //get parameters from .xml file
      GearDir cQLY2RE2p1(content, "QLY2RE2p1/");

      double QLY2RE2p1_Z[N];
      double QLY2RE2p1_R[N];
      double QLY2RE2p1_r[N];
      QLY2RE2p1_Z[0] = 0;
      QLY2RE2p1_Z[1] = cQLY2RE2p1.getLength("L") * unitFactor;
      QLY2RE2p1_R[0] = cQLY2RE2p1.getLength("R") * unitFactor;
      QLY2RE2p1_R[1] = cQLY2RE2p1.getLength("R") * unitFactor;
      QLY2RE2p1_r[0] = cQLY2RE2p1.getLength("r") * unitFactor;
      QLY2RE2p1_r[1] = cQLY2RE2p1.getLength("r") * unitFactor;

      double QLY2RE2p1_X0 = cQLY2RE2p1.getLength("X0") * unitFactor;
      double QLY2RE2p1_Z0 = cQLY2RE2p1.getLength("Z0") * unitFactor;
      double QLY2RE2p1_PHI = cQLY2RE2p1.getLength("PHI");
      G4Transform3D transform_QLY2RE2p1 = G4Translate3D(QLY2RE2p1_X0, 0.0, QLY2RE2p1_Z0);
      transform_QLY2RE2p1 = transform_QLY2RE2p1 * G4RotateY3D(QLY2RE2p1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLY2RE2p1 = new G4Polycone("geo_QLY2RE2p1xx_name", 0.0, 2 * M_PI, N, QLY2RE2p1_Z, QLY2RE2p1_r, QLY2RE2p1_R);

      string strMat_QLY2RE2p1 = cQLY2RE2p1.getString("Material");
      G4Material* mat_QLY2RE2p1 = Materials::get(strMat_QLY2RE2p1);
      G4LogicalVolume* logi_QLY2RE2p1 = new G4LogicalVolume(geo_QLY2RE2p1, mat_QLY2RE2p1, "logi_QLY2RE2p1_name");

      //put volume
      setColor(*logi_QLY2RE2p1, cQLY2RE2p1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLY2RE2p1, false);
      new G4PVPlacement(transform_QLY2RE2p1, logi_QLY2RE2p1, "phys_QLY2RE2p1_name", &topVolume, false, 0);

      //--------------
      //-   QLY2RE2p2

      //get parameters from .xml file
      GearDir cQLY2RE2p2(content, "QLY2RE2p2/");

      double QLY2RE2p2_Z[N];
      double QLY2RE2p2_R[N];
      double QLY2RE2p2_r[N];
      QLY2RE2p2_Z[0] = 0;
      QLY2RE2p2_Z[1] = cQLY2RE2p2.getLength("L") * unitFactor;
      QLY2RE2p2_R[0] = cQLY2RE2p2.getLength("R") * unitFactor;
      QLY2RE2p2_R[1] = cQLY2RE2p2.getLength("R") * unitFactor;
      QLY2RE2p2_r[0] = cQLY2RE2p2.getLength("r") * unitFactor;
      QLY2RE2p2_r[1] = cQLY2RE2p2.getLength("r") * unitFactor;

      double QLY2RE2p2_X0 = cQLY2RE2p2.getLength("X0") * unitFactor;
      double QLY2RE2p2_Z0 = cQLY2RE2p2.getLength("Z0") * unitFactor;
      double QLY2RE2p2_PHI = cQLY2RE2p2.getLength("PHI");
      G4Transform3D transform_QLY2RE2p2 = G4Translate3D(QLY2RE2p2_X0, 0.0, QLY2RE2p2_Z0);
      transform_QLY2RE2p2 = transform_QLY2RE2p2 * G4RotateY3D(QLY2RE2p2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLY2RE2p2 = new G4Polycone("geo_QLY2RE2p2xx_name", 0.0, 2 * M_PI, N, QLY2RE2p2_Z, QLY2RE2p2_r, QLY2RE2p2_R);

      string strMat_QLY2RE2p2 = cQLY2RE2p2.getString("Material");
      G4Material* mat_QLY2RE2p2 = Materials::get(strMat_QLY2RE2p2);
      G4LogicalVolume* logi_QLY2RE2p2 = new G4LogicalVolume(geo_QLY2RE2p2, mat_QLY2RE2p2, "logi_QLY2RE2p2_name");

      //put volume
      setColor(*logi_QLY2RE2p2, cQLY2RE2p2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLY2RE2p2, false);
      new G4PVPlacement(transform_QLY2RE2p2, logi_QLY2RE2p2, "phys_QLY2RE2p2_name", &topVolume, false, 0);

      //--------------
      //-   QKARPp1

      //get parameters from .xml file
      GearDir cQKARPp1(content, "QKARPp1/");

      double QKARPp1_Z[N];
      double QKARPp1_R[N];
      double QKARPp1_r[N];
      QKARPp1_Z[0] = 0;
      QKARPp1_Z[1] = cQKARPp1.getLength("L") * unitFactor;
      QKARPp1_R[0] = cQKARPp1.getLength("R") * unitFactor;
      QKARPp1_R[1] = cQKARPp1.getLength("R") * unitFactor;
      QKARPp1_r[0] = cQKARPp1.getLength("r") * unitFactor;
      QKARPp1_r[1] = cQKARPp1.getLength("r") * unitFactor;

      double QKARPp1_X0 = cQKARPp1.getLength("X0") * unitFactor;
      double QKARPp1_Z0 = cQKARPp1.getLength("Z0") * unitFactor;
      double QKARPp1_PHI = cQKARPp1.getLength("PHI");
      G4Transform3D transform_QKARPp1 = G4Translate3D(QKARPp1_X0, 0.0, QKARPp1_Z0);
      transform_QKARPp1 = transform_QKARPp1 * G4RotateY3D(QKARPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKARPp1 = new G4Polycone("geo_QKARPp1xx_name", 0.0, 2 * M_PI, N, QKARPp1_Z, QKARPp1_r, QKARPp1_R);

      string strMat_QKARPp1 = cQKARPp1.getString("Material");
      G4Material* mat_QKARPp1 = Materials::get(strMat_QKARPp1);
      G4LogicalVolume* logi_QKARPp1 = new G4LogicalVolume(geo_QKARPp1, mat_QKARPp1, "logi_QKARPp1_name");

      //put volume
      setColor(*logi_QKARPp1, cQKARPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKARPp1, false);
      new G4PVPlacement(transform_QKARPp1, logi_QKARPp1, "phys_QKARPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKARPp2

      //get parameters from .xml file
      GearDir cQKARPp2(content, "QKARPp2/");

      double QKARPp2_Z[N];
      double QKARPp2_R[N];
      double QKARPp2_r[N];
      QKARPp2_Z[0] = 0;
      QKARPp2_Z[1] = cQKARPp2.getLength("L") * unitFactor;
      QKARPp2_R[0] = cQKARPp2.getLength("R") * unitFactor;
      QKARPp2_R[1] = cQKARPp2.getLength("R") * unitFactor;
      QKARPp2_r[0] = cQKARPp2.getLength("r") * unitFactor;
      QKARPp2_r[1] = cQKARPp2.getLength("r") * unitFactor;

      double QKARPp2_X0 = cQKARPp2.getLength("X0") * unitFactor;
      double QKARPp2_Z0 = cQKARPp2.getLength("Z0") * unitFactor;
      double QKARPp2_PHI = cQKARPp2.getLength("PHI");
      G4Transform3D transform_QKARPp2 = G4Translate3D(QKARPp2_X0, 0.0, QKARPp2_Z0);
      transform_QKARPp2 = transform_QKARPp2 * G4RotateY3D(QKARPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKARPp2 = new G4Polycone("geo_QKARPp2xx_name", 0.0, 2 * M_PI, N, QKARPp2_Z, QKARPp2_r, QKARPp2_R);

      string strMat_QKARPp2 = cQKARPp2.getString("Material");
      G4Material* mat_QKARPp2 = Materials::get(strMat_QKARPp2);
      G4LogicalVolume* logi_QKARPp2 = new G4LogicalVolume(geo_QKARPp2, mat_QKARPp2, "logi_QKARPp2_name");

      //put volume
      setColor(*logi_QKARPp2, cQKARPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKARPp2, false);
      new G4PVPlacement(transform_QKARPp2, logi_QKARPp2, "phys_QKARPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKBRPp1

      //get parameters from .xml file
      GearDir cQKBRPp1(content, "QKBRPp1/");

      double QKBRPp1_Z[N];
      double QKBRPp1_R[N];
      double QKBRPp1_r[N];
      QKBRPp1_Z[0] = 0;
      QKBRPp1_Z[1] = cQKBRPp1.getLength("L") * unitFactor;
      QKBRPp1_R[0] = cQKBRPp1.getLength("R") * unitFactor;
      QKBRPp1_R[1] = cQKBRPp1.getLength("R") * unitFactor;
      QKBRPp1_r[0] = cQKBRPp1.getLength("r") * unitFactor;
      QKBRPp1_r[1] = cQKBRPp1.getLength("r") * unitFactor;

      double QKBRPp1_X0 = cQKBRPp1.getLength("X0") * unitFactor;
      double QKBRPp1_Z0 = cQKBRPp1.getLength("Z0") * unitFactor;
      double QKBRPp1_PHI = cQKBRPp1.getLength("PHI");
      G4Transform3D transform_QKBRPp1 = G4Translate3D(QKBRPp1_X0, 0.0, QKBRPp1_Z0);
      transform_QKBRPp1 = transform_QKBRPp1 * G4RotateY3D(QKBRPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBRPp1 = new G4Polycone("geo_QKBRPp1xx_name", 0.0, 2 * M_PI, N, QKBRPp1_Z, QKBRPp1_r, QKBRPp1_R);

      string strMat_QKBRPp1 = cQKBRPp1.getString("Material");
      G4Material* mat_QKBRPp1 = Materials::get(strMat_QKBRPp1);
      G4LogicalVolume* logi_QKBRPp1 = new G4LogicalVolume(geo_QKBRPp1, mat_QKBRPp1, "logi_QKBRPp1_name");

      //put volume
      setColor(*logi_QKBRPp1, cQKBRPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBRPp1, false);
      new G4PVPlacement(transform_QKBRPp1, logi_QKBRPp1, "phys_QKBRPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKBRPp2

      //get parameters from .xml file
      GearDir cQKBRPp2(content, "QKBRPp2/");

      double QKBRPp2_Z[N];
      double QKBRPp2_R[N];
      double QKBRPp2_r[N];
      QKBRPp2_Z[0] = 0;
      QKBRPp2_Z[1] = cQKBRPp2.getLength("L") * unitFactor;
      QKBRPp2_R[0] = cQKBRPp2.getLength("R") * unitFactor;
      QKBRPp2_R[1] = cQKBRPp2.getLength("R") * unitFactor;
      QKBRPp2_r[0] = cQKBRPp2.getLength("r") * unitFactor;
      QKBRPp2_r[1] = cQKBRPp2.getLength("r") * unitFactor;

      double QKBRPp2_X0 = cQKBRPp2.getLength("X0") * unitFactor;
      double QKBRPp2_Z0 = cQKBRPp2.getLength("Z0") * unitFactor;
      double QKBRPp2_PHI = cQKBRPp2.getLength("PHI");
      G4Transform3D transform_QKBRPp2 = G4Translate3D(QKBRPp2_X0, 0.0, QKBRPp2_Z0);
      transform_QKBRPp2 = transform_QKBRPp2 * G4RotateY3D(QKBRPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBRPp2 = new G4Polycone("geo_QKBRPp2xx_name", 0.0, 2 * M_PI, N, QKBRPp2_Z, QKBRPp2_r, QKBRPp2_R);

      string strMat_QKBRPp2 = cQKBRPp2.getString("Material");
      G4Material* mat_QKBRPp2 = Materials::get(strMat_QKBRPp2);
      G4LogicalVolume* logi_QKBRPp2 = new G4LogicalVolume(geo_QKBRPp2, mat_QKBRPp2, "logi_QKBRPp2_name");

      //put volume
      setColor(*logi_QKBRPp2, cQKBRPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBRPp2, false);
      new G4PVPlacement(transform_QKBRPp2, logi_QKBRPp2, "phys_QKBRPp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC1RPp1

      //get parameters from .xml file
      GearDir cQLC1RPp1(content, "QLC1RPp1/");

      double QLC1RPp1_Z[N];
      double QLC1RPp1_R[N];
      double QLC1RPp1_r[N];
      QLC1RPp1_Z[0] = 0;
      QLC1RPp1_Z[1] = cQLC1RPp1.getLength("L") * unitFactor;
      QLC1RPp1_R[0] = cQLC1RPp1.getLength("R") * unitFactor;
      QLC1RPp1_R[1] = cQLC1RPp1.getLength("R") * unitFactor;
      QLC1RPp1_r[0] = cQLC1RPp1.getLength("r") * unitFactor;
      QLC1RPp1_r[1] = cQLC1RPp1.getLength("r") * unitFactor;

      double QLC1RPp1_X0 = cQLC1RPp1.getLength("X0") * unitFactor;
      double QLC1RPp1_Z0 = cQLC1RPp1.getLength("Z0") * unitFactor;
      double QLC1RPp1_PHI = cQLC1RPp1.getLength("PHI");
      G4Transform3D transform_QLC1RPp1 = G4Translate3D(QLC1RPp1_X0, 0.0, QLC1RPp1_Z0);
      transform_QLC1RPp1 = transform_QLC1RPp1 * G4RotateY3D(QLC1RPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC1RPp1 = new G4Polycone("geo_QLC1RPp1xx_name", 0.0, 2 * M_PI, N, QLC1RPp1_Z, QLC1RPp1_r, QLC1RPp1_R);

      string strMat_QLC1RPp1 = cQLC1RPp1.getString("Material");
      G4Material* mat_QLC1RPp1 = Materials::get(strMat_QLC1RPp1);
      G4LogicalVolume* logi_QLC1RPp1 = new G4LogicalVolume(geo_QLC1RPp1, mat_QLC1RPp1, "logi_QLC1RPp1_name");

      //put volume
      setColor(*logi_QLC1RPp1, cQLC1RPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC1RPp1, false);
      new G4PVPlacement(transform_QLC1RPp1, logi_QLC1RPp1, "phys_QLC1RPp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC1RPp2

      //get parameters from .xml file
      GearDir cQLC1RPp2(content, "QLC1RPp2/");

      double QLC1RPp2_Z[N];
      double QLC1RPp2_R[N];
      double QLC1RPp2_r[N];
      QLC1RPp2_Z[0] = 0;
      QLC1RPp2_Z[1] = cQLC1RPp2.getLength("L") * unitFactor;
      QLC1RPp2_R[0] = cQLC1RPp2.getLength("R") * unitFactor;
      QLC1RPp2_R[1] = cQLC1RPp2.getLength("R") * unitFactor;
      QLC1RPp2_r[0] = cQLC1RPp2.getLength("r") * unitFactor;
      QLC1RPp2_r[1] = cQLC1RPp2.getLength("r") * unitFactor;

      double QLC1RPp2_X0 = cQLC1RPp2.getLength("X0") * unitFactor;
      double QLC1RPp2_Z0 = cQLC1RPp2.getLength("Z0") * unitFactor;
      double QLC1RPp2_PHI = cQLC1RPp2.getLength("PHI");
      G4Transform3D transform_QLC1RPp2 = G4Translate3D(QLC1RPp2_X0, 0.0, QLC1RPp2_Z0);
      transform_QLC1RPp2 = transform_QLC1RPp2 * G4RotateY3D(QLC1RPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC1RPp2 = new G4Polycone("geo_QLC1RPp2xx_name", 0.0, 2 * M_PI, N, QLC1RPp2_Z, QLC1RPp2_r, QLC1RPp2_R);

      string strMat_QLC1RPp2 = cQLC1RPp2.getString("Material");
      G4Material* mat_QLC1RPp2 = Materials::get(strMat_QLC1RPp2);
      G4LogicalVolume* logi_QLC1RPp2 = new G4LogicalVolume(geo_QLC1RPp2, mat_QLC1RPp2, "logi_QLC1RPp2_name");

      //put volume
      setColor(*logi_QLC1RPp2, cQLC1RPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC1RPp2, false);
      new G4PVPlacement(transform_QLC1RPp2, logi_QLC1RPp2, "phys_QLC1RPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKCRPp1

      //get parameters from .xml file
      GearDir cQKCRPp1(content, "QKCRPp1/");

      double QKCRPp1_Z[N];
      double QKCRPp1_R[N];
      double QKCRPp1_r[N];
      QKCRPp1_Z[0] = 0;
      QKCRPp1_Z[1] = cQKCRPp1.getLength("L") * unitFactor;
      QKCRPp1_R[0] = cQKCRPp1.getLength("R") * unitFactor;
      QKCRPp1_R[1] = cQKCRPp1.getLength("R") * unitFactor;
      QKCRPp1_r[0] = cQKCRPp1.getLength("r") * unitFactor;
      QKCRPp1_r[1] = cQKCRPp1.getLength("r") * unitFactor;

      double QKCRPp1_X0 = cQKCRPp1.getLength("X0") * unitFactor;
      double QKCRPp1_Z0 = cQKCRPp1.getLength("Z0") * unitFactor;
      double QKCRPp1_PHI = cQKCRPp1.getLength("PHI");
      G4Transform3D transform_QKCRPp1 = G4Translate3D(QKCRPp1_X0, 0.0, QKCRPp1_Z0);
      transform_QKCRPp1 = transform_QKCRPp1 * G4RotateY3D(QKCRPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCRPp1 = new G4Polycone("geo_QKCRPp1xx_name", 0.0, 2 * M_PI, N, QKCRPp1_Z, QKCRPp1_r, QKCRPp1_R);

      string strMat_QKCRPp1 = cQKCRPp1.getString("Material");
      G4Material* mat_QKCRPp1 = Materials::get(strMat_QKCRPp1);
      G4LogicalVolume* logi_QKCRPp1 = new G4LogicalVolume(geo_QKCRPp1, mat_QKCRPp1, "logi_QKCRPp1_name");

      //put volume
      setColor(*logi_QKCRPp1, cQKCRPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCRPp1, false);
      new G4PVPlacement(transform_QKCRPp1, logi_QKCRPp1, "phys_QKCRPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKCRPp2

      //get parameters from .xml file
      GearDir cQKCRPp2(content, "QKCRPp2/");

      double QKCRPp2_Z[N];
      double QKCRPp2_R[N];
      double QKCRPp2_r[N];
      QKCRPp2_Z[0] = 0;
      QKCRPp2_Z[1] = cQKCRPp2.getLength("L") * unitFactor;
      QKCRPp2_R[0] = cQKCRPp2.getLength("R") * unitFactor;
      QKCRPp2_R[1] = cQKCRPp2.getLength("R") * unitFactor;
      QKCRPp2_r[0] = cQKCRPp2.getLength("r") * unitFactor;
      QKCRPp2_r[1] = cQKCRPp2.getLength("r") * unitFactor;

      double QKCRPp2_X0 = cQKCRPp2.getLength("X0") * unitFactor;
      double QKCRPp2_Z0 = cQKCRPp2.getLength("Z0") * unitFactor;
      double QKCRPp2_PHI = cQKCRPp2.getLength("PHI");
      G4Transform3D transform_QKCRPp2 = G4Translate3D(QKCRPp2_X0, 0.0, QKCRPp2_Z0);
      transform_QKCRPp2 = transform_QKCRPp2 * G4RotateY3D(QKCRPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCRPp2 = new G4Polycone("geo_QKCRPp2xx_name", 0.0, 2 * M_PI, N, QKCRPp2_Z, QKCRPp2_r, QKCRPp2_R);

      string strMat_QKCRPp2 = cQKCRPp2.getString("Material");
      G4Material* mat_QKCRPp2 = Materials::get(strMat_QKCRPp2);
      G4LogicalVolume* logi_QKCRPp2 = new G4LogicalVolume(geo_QKCRPp2, mat_QKCRPp2, "logi_QKCRPp2_name");

      //put volume
      setColor(*logi_QKCRPp2, cQKCRPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCRPp2, false);
      new G4PVPlacement(transform_QKCRPp2, logi_QKCRPp2, "phys_QKCRPp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC2RPp1

      //get parameters from .xml file
      GearDir cQLC2RPp1(content, "QLC2RPp1/");

      double QLC2RPp1_Z[N];
      double QLC2RPp1_R[N];
      double QLC2RPp1_r[N];
      QLC2RPp1_Z[0] = 0;
      QLC2RPp1_Z[1] = cQLC2RPp1.getLength("L") * unitFactor;
      QLC2RPp1_R[0] = cQLC2RPp1.getLength("R") * unitFactor;
      QLC2RPp1_R[1] = cQLC2RPp1.getLength("R") * unitFactor;
      QLC2RPp1_r[0] = cQLC2RPp1.getLength("r") * unitFactor;
      QLC2RPp1_r[1] = cQLC2RPp1.getLength("r") * unitFactor;

      double QLC2RPp1_X0 = cQLC2RPp1.getLength("X0") * unitFactor;
      double QLC2RPp1_Z0 = cQLC2RPp1.getLength("Z0") * unitFactor;
      double QLC2RPp1_PHI = cQLC2RPp1.getLength("PHI");
      G4Transform3D transform_QLC2RPp1 = G4Translate3D(QLC2RPp1_X0, 0.0, QLC2RPp1_Z0);
      transform_QLC2RPp1 = transform_QLC2RPp1 * G4RotateY3D(QLC2RPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2RPp1 = new G4Polycone("geo_QLC2RPp1xx_name", 0.0, 2 * M_PI, N, QLC2RPp1_Z, QLC2RPp1_r, QLC2RPp1_R);

      string strMat_QLC2RPp1 = cQLC2RPp1.getString("Material");
      G4Material* mat_QLC2RPp1 = Materials::get(strMat_QLC2RPp1);
      G4LogicalVolume* logi_QLC2RPp1 = new G4LogicalVolume(geo_QLC2RPp1, mat_QLC2RPp1, "logi_QLC2RPp1_name");

      //put volume
      setColor(*logi_QLC2RPp1, cQLC2RPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2RPp1, false);
      new G4PVPlacement(transform_QLC2RPp1, logi_QLC2RPp1, "phys_QLC2RPp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC2RPp2

      //get parameters from .xml file
      GearDir cQLC2RPp2(content, "QLC2RPp2/");

      double QLC2RPp2_Z[N];
      double QLC2RPp2_R[N];
      double QLC2RPp2_r[N];
      QLC2RPp2_Z[0] = 0;
      QLC2RPp2_Z[1] = cQLC2RPp2.getLength("L") * unitFactor;
      QLC2RPp2_R[0] = cQLC2RPp2.getLength("R") * unitFactor;
      QLC2RPp2_R[1] = cQLC2RPp2.getLength("R") * unitFactor;
      QLC2RPp2_r[0] = cQLC2RPp2.getLength("r") * unitFactor;
      QLC2RPp2_r[1] = cQLC2RPp2.getLength("r") * unitFactor;

      double QLC2RPp2_X0 = cQLC2RPp2.getLength("X0") * unitFactor;
      double QLC2RPp2_Z0 = cQLC2RPp2.getLength("Z0") * unitFactor;
      double QLC2RPp2_PHI = cQLC2RPp2.getLength("PHI");
      G4Transform3D transform_QLC2RPp2 = G4Translate3D(QLC2RPp2_X0, 0.0, QLC2RPp2_Z0);
      transform_QLC2RPp2 = transform_QLC2RPp2 * G4RotateY3D(QLC2RPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2RPp2 = new G4Polycone("geo_QLC2RPp2xx_name", 0.0, 2 * M_PI, N, QLC2RPp2_Z, QLC2RPp2_r, QLC2RPp2_R);

      string strMat_QLC2RPp2 = cQLC2RPp2.getString("Material");
      G4Material* mat_QLC2RPp2 = Materials::get(strMat_QLC2RPp2);
      G4LogicalVolume* logi_QLC2RPp2 = new G4LogicalVolume(geo_QLC2RPp2, mat_QLC2RPp2, "logi_QLC2RPp2_name");

      //put volume
      setColor(*logi_QLC2RPp2, cQLC2RPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2RPp2, false);
      new G4PVPlacement(transform_QLC2RPp2, logi_QLC2RPp2, "phys_QLC2RPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKDRPp1

      //get parameters from .xml file
      GearDir cQKDRPp1(content, "QKDRPp1/");

      double QKDRPp1_Z[N];
      double QKDRPp1_R[N];
      double QKDRPp1_r[N];
      QKDRPp1_Z[0] = 0;
      QKDRPp1_Z[1] = cQKDRPp1.getLength("L") * unitFactor;
      QKDRPp1_R[0] = cQKDRPp1.getLength("R") * unitFactor;
      QKDRPp1_R[1] = cQKDRPp1.getLength("R") * unitFactor;
      QKDRPp1_r[0] = cQKDRPp1.getLength("r") * unitFactor;
      QKDRPp1_r[1] = cQKDRPp1.getLength("r") * unitFactor;

      double QKDRPp1_X0 = cQKDRPp1.getLength("X0") * unitFactor;
      double QKDRPp1_Z0 = cQKDRPp1.getLength("Z0") * unitFactor;
      double QKDRPp1_PHI = cQKDRPp1.getLength("PHI");
      G4Transform3D transform_QKDRPp1 = G4Translate3D(QKDRPp1_X0, 0.0, QKDRPp1_Z0);
      transform_QKDRPp1 = transform_QKDRPp1 * G4RotateY3D(QKDRPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKDRPp1 = new G4Polycone("geo_QKDRPp1xx_name", 0.0, 2 * M_PI, N, QKDRPp1_Z, QKDRPp1_r, QKDRPp1_R);

      string strMat_QKDRPp1 = cQKDRPp1.getString("Material");
      G4Material* mat_QKDRPp1 = Materials::get(strMat_QKDRPp1);
      G4LogicalVolume* logi_QKDRPp1 = new G4LogicalVolume(geo_QKDRPp1, mat_QKDRPp1, "logi_QKDRPp1_name");

      //put volume
      setColor(*logi_QKDRPp1, cQKDRPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKDRPp1, false);
      new G4PVPlacement(transform_QKDRPp1, logi_QKDRPp1, "phys_QKDRPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKDRPp2

      //get parameters from .xml file
      GearDir cQKDRPp2(content, "QKDRPp2/");

      double QKDRPp2_Z[N];
      double QKDRPp2_R[N];
      double QKDRPp2_r[N];
      QKDRPp2_Z[0] = 0;
      QKDRPp2_Z[1] = cQKDRPp2.getLength("L") * unitFactor;
      QKDRPp2_R[0] = cQKDRPp2.getLength("R") * unitFactor;
      QKDRPp2_R[1] = cQKDRPp2.getLength("R") * unitFactor;
      QKDRPp2_r[0] = cQKDRPp2.getLength("r") * unitFactor;
      QKDRPp2_r[1] = cQKDRPp2.getLength("r") * unitFactor;

      double QKDRPp2_X0 = cQKDRPp2.getLength("X0") * unitFactor;
      double QKDRPp2_Z0 = cQKDRPp2.getLength("Z0") * unitFactor;
      double QKDRPp2_PHI = cQKDRPp2.getLength("PHI");
      G4Transform3D transform_QKDRPp2 = G4Translate3D(QKDRPp2_X0, 0.0, QKDRPp2_Z0);
      transform_QKDRPp2 = transform_QKDRPp2 * G4RotateY3D(QKDRPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKDRPp2 = new G4Polycone("geo_QKDRPp2xx_name", 0.0, 2 * M_PI, N, QKDRPp2_Z, QKDRPp2_r, QKDRPp2_R);

      string strMat_QKDRPp2 = cQKDRPp2.getString("Material");
      G4Material* mat_QKDRPp2 = Materials::get(strMat_QKDRPp2);
      G4LogicalVolume* logi_QKDRPp2 = new G4LogicalVolume(geo_QKDRPp2, mat_QKDRPp2, "logi_QKDRPp2_name");

      //put volume
      setColor(*logi_QKDRPp2, cQKDRPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKDRPp2, false);
      new G4PVPlacement(transform_QKDRPp2, logi_QKDRPp2, "phys_QKDRPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKFRPp1

      //get parameters from .xml file
      GearDir cQKFRPp1(content, "QKFRPp1/");

      double QKFRPp1_Z[N];
      double QKFRPp1_R[N];
      double QKFRPp1_r[N];
      QKFRPp1_Z[0] = 0;
      QKFRPp1_Z[1] = cQKFRPp1.getLength("L") * unitFactor;
      QKFRPp1_R[0] = cQKFRPp1.getLength("R") * unitFactor;
      QKFRPp1_R[1] = cQKFRPp1.getLength("R") * unitFactor;
      QKFRPp1_r[0] = cQKFRPp1.getLength("r") * unitFactor;
      QKFRPp1_r[1] = cQKFRPp1.getLength("r") * unitFactor;

      double QKFRPp1_X0 = cQKFRPp1.getLength("X0") * unitFactor;
      double QKFRPp1_Z0 = cQKFRPp1.getLength("Z0") * unitFactor;
      double QKFRPp1_PHI = cQKFRPp1.getLength("PHI");
      G4Transform3D transform_QKFRPp1 = G4Translate3D(QKFRPp1_X0, 0.0, QKFRPp1_Z0);
      transform_QKFRPp1 = transform_QKFRPp1 * G4RotateY3D(QKFRPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKFRPp1 = new G4Polycone("geo_QKFRPp1xx_name", 0.0, 2 * M_PI, N, QKFRPp1_Z, QKFRPp1_r, QKFRPp1_R);

      string strMat_QKFRPp1 = cQKFRPp1.getString("Material");
      G4Material* mat_QKFRPp1 = Materials::get(strMat_QKFRPp1);
      G4LogicalVolume* logi_QKFRPp1 = new G4LogicalVolume(geo_QKFRPp1, mat_QKFRPp1, "logi_QKFRPp1_name");

      //put volume
      setColor(*logi_QKFRPp1, cQKFRPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKFRPp1, false);
      new G4PVPlacement(transform_QKFRPp1, logi_QKFRPp1, "phys_QKFRPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKFRPp2

      //get parameters from .xml file
      GearDir cQKFRPp2(content, "QKFRPp2/");

      double QKFRPp2_Z[N];
      double QKFRPp2_R[N];
      double QKFRPp2_r[N];
      QKFRPp2_Z[0] = 0;
      QKFRPp2_Z[1] = cQKFRPp2.getLength("L") * unitFactor;
      QKFRPp2_R[0] = cQKFRPp2.getLength("R") * unitFactor;
      QKFRPp2_R[1] = cQKFRPp2.getLength("R") * unitFactor;
      QKFRPp2_r[0] = cQKFRPp2.getLength("r") * unitFactor;
      QKFRPp2_r[1] = cQKFRPp2.getLength("r") * unitFactor;

      double QKFRPp2_X0 = cQKFRPp2.getLength("X0") * unitFactor;
      double QKFRPp2_Z0 = cQKFRPp2.getLength("Z0") * unitFactor;
      double QKFRPp2_PHI = cQKFRPp2.getLength("PHI");
      G4Transform3D transform_QKFRPp2 = G4Translate3D(QKFRPp2_X0, 0.0, QKFRPp2_Z0);
      transform_QKFRPp2 = transform_QKFRPp2 * G4RotateY3D(QKFRPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKFRPp2 = new G4Polycone("geo_QKFRPp2xx_name", 0.0, 2 * M_PI, N, QKFRPp2_Z, QKFRPp2_r, QKFRPp2_R);

      string strMat_QKFRPp2 = cQKFRPp2.getString("Material");
      G4Material* mat_QKFRPp2 = Materials::get(strMat_QKFRPp2);
      G4LogicalVolume* logi_QKFRPp2 = new G4LogicalVolume(geo_QKFRPp2, mat_QKFRPp2, "logi_QKFRPp2_name");

      //put volume
      setColor(*logi_QKFRPp2, cQKFRPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKFRPp2, false);
      new G4PVPlacement(transform_QKFRPp2, logi_QKFRPp2, "phys_QKFRPp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC3RPp1

      //get parameters from .xml file
      GearDir cQLC3RPp1(content, "QLC3RPp1/");

      double QLC3RPp1_Z[N];
      double QLC3RPp1_R[N];
      double QLC3RPp1_r[N];
      QLC3RPp1_Z[0] = 0;
      QLC3RPp1_Z[1] = cQLC3RPp1.getLength("L") * unitFactor;
      QLC3RPp1_R[0] = cQLC3RPp1.getLength("R") * unitFactor;
      QLC3RPp1_R[1] = cQLC3RPp1.getLength("R") * unitFactor;
      QLC3RPp1_r[0] = cQLC3RPp1.getLength("r") * unitFactor;
      QLC3RPp1_r[1] = cQLC3RPp1.getLength("r") * unitFactor;

      double QLC3RPp1_X0 = cQLC3RPp1.getLength("X0") * unitFactor;
      double QLC3RPp1_Z0 = cQLC3RPp1.getLength("Z0") * unitFactor;
      double QLC3RPp1_PHI = cQLC3RPp1.getLength("PHI");
      G4Transform3D transform_QLC3RPp1 = G4Translate3D(QLC3RPp1_X0, 0.0, QLC3RPp1_Z0);
      transform_QLC3RPp1 = transform_QLC3RPp1 * G4RotateY3D(QLC3RPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3RPp1 = new G4Polycone("geo_QLC3RPp1xx_name", 0.0, 2 * M_PI, N, QLC3RPp1_Z, QLC3RPp1_r, QLC3RPp1_R);

      string strMat_QLC3RPp1 = cQLC3RPp1.getString("Material");
      G4Material* mat_QLC3RPp1 = Materials::get(strMat_QLC3RPp1);
      G4LogicalVolume* logi_QLC3RPp1 = new G4LogicalVolume(geo_QLC3RPp1, mat_QLC3RPp1, "logi_QLC3RPp1_name");

      //put volume
      setColor(*logi_QLC3RPp1, cQLC3RPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3RPp1, false);
      new G4PVPlacement(transform_QLC3RPp1, logi_QLC3RPp1, "phys_QLC3RPp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC3RPp2

      //get parameters from .xml file
      GearDir cQLC3RPp2(content, "QLC3RPp2/");

      double QLC3RPp2_Z[N];
      double QLC3RPp2_R[N];
      double QLC3RPp2_r[N];
      QLC3RPp2_Z[0] = 0;
      QLC3RPp2_Z[1] = cQLC3RPp2.getLength("L") * unitFactor;
      QLC3RPp2_R[0] = cQLC3RPp2.getLength("R") * unitFactor;
      QLC3RPp2_R[1] = cQLC3RPp2.getLength("R") * unitFactor;
      QLC3RPp2_r[0] = cQLC3RPp2.getLength("r") * unitFactor;
      QLC3RPp2_r[1] = cQLC3RPp2.getLength("r") * unitFactor;

      double QLC3RPp2_X0 = cQLC3RPp2.getLength("X0") * unitFactor;
      double QLC3RPp2_Z0 = cQLC3RPp2.getLength("Z0") * unitFactor;
      double QLC3RPp2_PHI = cQLC3RPp2.getLength("PHI");
      G4Transform3D transform_QLC3RPp2 = G4Translate3D(QLC3RPp2_X0, 0.0, QLC3RPp2_Z0);
      transform_QLC3RPp2 = transform_QLC3RPp2 * G4RotateY3D(QLC3RPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3RPp2 = new G4Polycone("geo_QLC3RPp2xx_name", 0.0, 2 * M_PI, N, QLC3RPp2_Z, QLC3RPp2_r, QLC3RPp2_R);

      string strMat_QLC3RPp2 = cQLC3RPp2.getString("Material");
      G4Material* mat_QLC3RPp2 = Materials::get(strMat_QLC3RPp2);
      G4LogicalVolume* logi_QLC3RPp2 = new G4LogicalVolume(geo_QLC3RPp2, mat_QLC3RPp2, "logi_QLC3RPp2_name");

      //put volume
      setColor(*logi_QLC3RPp2, cQLC3RPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3RPp2, false);
      new G4PVPlacement(transform_QLC3RPp2, logi_QLC3RPp2, "phys_QLC3RPp2_name", &topVolume, false, 0);

      //--------------
      //-   SLYTRP2p1

      //get parameters from .xml file
      GearDir cSLYTRP2p1(content, "SLYTRP2p1/");

      double SLYTRP2p1_Z[N];
      double SLYTRP2p1_R[N];
      double SLYTRP2p1_r[N];
      SLYTRP2p1_Z[0] = 0;
      SLYTRP2p1_Z[1] = cSLYTRP2p1.getLength("L") * unitFactor;
      SLYTRP2p1_R[0] = cSLYTRP2p1.getLength("R") * unitFactor;
      SLYTRP2p1_R[1] = cSLYTRP2p1.getLength("R") * unitFactor;
      SLYTRP2p1_r[0] = cSLYTRP2p1.getLength("r") * unitFactor;
      SLYTRP2p1_r[1] = cSLYTRP2p1.getLength("r") * unitFactor;

      double SLYTRP2p1_X0 = cSLYTRP2p1.getLength("X0") * unitFactor;
      double SLYTRP2p1_Z0 = cSLYTRP2p1.getLength("Z0") * unitFactor;
      double SLYTRP2p1_PHI = cSLYTRP2p1.getLength("PHI");
      G4Transform3D transform_SLYTRP2p1 = G4Translate3D(SLYTRP2p1_X0, 0.0, SLYTRP2p1_Z0);
      transform_SLYTRP2p1 = transform_SLYTRP2p1 * G4RotateY3D(SLYTRP2p1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTRP2p1 = new G4Polycone("geo_SLYTRP2p1xx_name", 0.0, 2 * M_PI, N, SLYTRP2p1_Z, SLYTRP2p1_r, SLYTRP2p1_R);

      string strMat_SLYTRP2p1 = cSLYTRP2p1.getString("Material");
      G4Material* mat_SLYTRP2p1 = Materials::get(strMat_SLYTRP2p1);
      G4LogicalVolume* logi_SLYTRP2p1 = new G4LogicalVolume(geo_SLYTRP2p1, mat_SLYTRP2p1, "logi_SLYTRP2p1_name");

      //put volume
      setColor(*logi_SLYTRP2p1, cSLYTRP2p1.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTRP2p1, false);
      new G4PVPlacement(transform_SLYTRP2p1, logi_SLYTRP2p1, "phys_SLYTRP2p1_name", &topVolume, false, 0);

      //--------------
      //-   SLYTRP2p2

      //get parameters from .xml file
      GearDir cSLYTRP2p2(content, "SLYTRP2p2/");

      double SLYTRP2p2_Z[N];
      double SLYTRP2p2_R[N];
      double SLYTRP2p2_r[N];
      SLYTRP2p2_Z[0] = 0;
      SLYTRP2p2_Z[1] = cSLYTRP2p2.getLength("L") * unitFactor;
      SLYTRP2p2_R[0] = cSLYTRP2p2.getLength("R") * unitFactor;
      SLYTRP2p2_R[1] = cSLYTRP2p2.getLength("R") * unitFactor;
      SLYTRP2p2_r[0] = cSLYTRP2p2.getLength("r") * unitFactor;
      SLYTRP2p2_r[1] = cSLYTRP2p2.getLength("r") * unitFactor;

      double SLYTRP2p2_X0 = cSLYTRP2p2.getLength("X0") * unitFactor;
      double SLYTRP2p2_Z0 = cSLYTRP2p2.getLength("Z0") * unitFactor;
      double SLYTRP2p2_PHI = cSLYTRP2p2.getLength("PHI");
      G4Transform3D transform_SLYTRP2p2 = G4Translate3D(SLYTRP2p2_X0, 0.0, SLYTRP2p2_Z0);
      transform_SLYTRP2p2 = transform_SLYTRP2p2 * G4RotateY3D(SLYTRP2p2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTRP2p2 = new G4Polycone("geo_SLYTRP2p2xx_name", 0.0, 2 * M_PI, N, SLYTRP2p2_Z, SLYTRP2p2_r, SLYTRP2p2_R);

      string strMat_SLYTRP2p2 = cSLYTRP2p2.getString("Material");
      G4Material* mat_SLYTRP2p2 = Materials::get(strMat_SLYTRP2p2);
      G4LogicalVolume* logi_SLYTRP2p2 = new G4LogicalVolume(geo_SLYTRP2p2, mat_SLYTRP2p2, "logi_SLYTRP2p2_name");

      //put volume
      setColor(*logi_SLYTRP2p2, cSLYTRP2p2.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTRP2p2, false);
      new G4PVPlacement(transform_SLYTRP2p2, logi_SLYTRP2p2, "phys_SLYTRP2p2_name", &topVolume, false, 0);

      //--------------
      //-   QKALEp1

      //get parameters from .xml file
      GearDir cQKALEp1(content, "QKALEp1/");

      double QKALEp1_Z[N];
      double QKALEp1_R[N];
      double QKALEp1_r[N];
      QKALEp1_Z[0] = 0;
      QKALEp1_Z[1] = cQKALEp1.getLength("L") * unitFactor;
      QKALEp1_R[0] = cQKALEp1.getLength("R") * unitFactor;
      QKALEp1_R[1] = cQKALEp1.getLength("R") * unitFactor;
      QKALEp1_r[0] = cQKALEp1.getLength("r") * unitFactor;
      QKALEp1_r[1] = cQKALEp1.getLength("r") * unitFactor;

      double QKALEp1_X0 = cQKALEp1.getLength("X0") * unitFactor;
      double QKALEp1_Z0 = cQKALEp1.getLength("Z0") * unitFactor;
      double QKALEp1_PHI = cQKALEp1.getLength("PHI");
      G4Transform3D transform_QKALEp1 = G4Translate3D(QKALEp1_X0, 0.0, QKALEp1_Z0);
      transform_QKALEp1 = transform_QKALEp1 * G4RotateY3D(QKALEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKALEp1 = new G4Polycone("geo_QKALEp1xx_name", 0.0, 2 * M_PI, N, QKALEp1_Z, QKALEp1_r, QKALEp1_R);

      string strMat_QKALEp1 = cQKALEp1.getString("Material");
      G4Material* mat_QKALEp1 = Materials::get(strMat_QKALEp1);
      G4LogicalVolume* logi_QKALEp1 = new G4LogicalVolume(geo_QKALEp1, mat_QKALEp1, "logi_QKALEp1_name");

      //put volume
      setColor(*logi_QKALEp1, cQKALEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKALEp1, false);
      new G4PVPlacement(transform_QKALEp1, logi_QKALEp1, "phys_QKALEp1_name", &topVolume, false, 0);

      //--------------
      //-   QKALEp2

      //get parameters from .xml file
      GearDir cQKALEp2(content, "QKALEp2/");

      double QKALEp2_Z[N];
      double QKALEp2_R[N];
      double QKALEp2_r[N];
      QKALEp2_Z[0] = 0;
      QKALEp2_Z[1] = cQKALEp2.getLength("L") * unitFactor;
      QKALEp2_R[0] = cQKALEp2.getLength("R") * unitFactor;
      QKALEp2_R[1] = cQKALEp2.getLength("R") * unitFactor;
      QKALEp2_r[0] = cQKALEp2.getLength("r") * unitFactor;
      QKALEp2_r[1] = cQKALEp2.getLength("r") * unitFactor;

      double QKALEp2_X0 = cQKALEp2.getLength("X0") * unitFactor;
      double QKALEp2_Z0 = cQKALEp2.getLength("Z0") * unitFactor;
      double QKALEp2_PHI = cQKALEp2.getLength("PHI");
      G4Transform3D transform_QKALEp2 = G4Translate3D(QKALEp2_X0, 0.0, QKALEp2_Z0);
      transform_QKALEp2 = transform_QKALEp2 * G4RotateY3D(QKALEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKALEp2xx = new G4Polycone("geo_QKALEp2xx_name", 0.0, 2 * M_PI, N, QKALEp2_Z, QKALEp2_r, QKALEp2_R);
      G4SubtractionSolid* geo_QKALEp2 = new G4SubtractionSolid("geo_QKALEp2_name", geo_QKALEp2xx, geo_BC1LPp2, transform_QKALEp2.inverse()*transform_BC1LPtube);

      string strMat_QKALEp2 = cQKALEp2.getString("Material");
      G4Material* mat_QKALEp2 = Materials::get(strMat_QKALEp2);
      G4LogicalVolume* logi_QKALEp2 = new G4LogicalVolume(geo_QKALEp2, mat_QKALEp2, "logi_QKALEp2_name");

      //put volume
      setColor(*logi_QKALEp2, cQKALEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKALEp2, false);
      new G4PVPlacement(transform_QKALEp2, logi_QKALEp2, "phys_QKALEp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC2LEp1

      //get parameters from .xml file
      GearDir cQLC2LEp1(content, "QLC2LEp1/");

      double QLC2LEp1_Z[N];
      double QLC2LEp1_R[N];
      double QLC2LEp1_r[N];
      QLC2LEp1_Z[0] = 0;
      QLC2LEp1_Z[1] = cQLC2LEp1.getLength("L") * unitFactor;
      QLC2LEp1_R[0] = cQLC2LEp1.getLength("R") * unitFactor;
      QLC2LEp1_R[1] = cQLC2LEp1.getLength("R") * unitFactor;
      QLC2LEp1_r[0] = cQLC2LEp1.getLength("r") * unitFactor;
      QLC2LEp1_r[1] = cQLC2LEp1.getLength("r") * unitFactor;

      double QLC2LEp1_X0 = cQLC2LEp1.getLength("X0") * unitFactor;
      double QLC2LEp1_Z0 = cQLC2LEp1.getLength("Z0") * unitFactor;
      double QLC2LEp1_PHI = cQLC2LEp1.getLength("PHI");
      G4Transform3D transform_QLC2LEp1 = G4Translate3D(QLC2LEp1_X0, 0.0, QLC2LEp1_Z0);
      transform_QLC2LEp1 = transform_QLC2LEp1 * G4RotateY3D(QLC2LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2LEp1 = new G4Polycone("geo_QLC2LEp1xx_name", 0.0, 2 * M_PI, N, QLC2LEp1_Z, QLC2LEp1_r, QLC2LEp1_R);

      string strMat_QLC2LEp1 = cQLC2LEp1.getString("Material");
      G4Material* mat_QLC2LEp1 = Materials::get(strMat_QLC2LEp1);
      G4LogicalVolume* logi_QLC2LEp1 = new G4LogicalVolume(geo_QLC2LEp1, mat_QLC2LEp1, "logi_QLC2LEp1_name");

      //put volume
      setColor(*logi_QLC2LEp1, cQLC2LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2LEp1, false);
      new G4PVPlacement(transform_QLC2LEp1, logi_QLC2LEp1, "phys_QLC2LEp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC2LEp2

      //get parameters from .xml file
      GearDir cQLC2LEp2(content, "QLC2LEp2/");

      double QLC2LEp2_Z[N];
      double QLC2LEp2_R[N];
      double QLC2LEp2_r[N];
      QLC2LEp2_Z[0] = 0;
      QLC2LEp2_Z[1] = cQLC2LEp2.getLength("L") * unitFactor;
      QLC2LEp2_R[0] = cQLC2LEp2.getLength("R") * unitFactor;
      QLC2LEp2_R[1] = cQLC2LEp2.getLength("R") * unitFactor;
      QLC2LEp2_r[0] = cQLC2LEp2.getLength("r") * unitFactor;
      QLC2LEp2_r[1] = cQLC2LEp2.getLength("r") * unitFactor;

      double QLC2LEp2_X0 = cQLC2LEp2.getLength("X0") * unitFactor;
      double QLC2LEp2_Z0 = cQLC2LEp2.getLength("Z0") * unitFactor;
      double QLC2LEp2_PHI = cQLC2LEp2.getLength("PHI");
      G4Transform3D transform_QLC2LEp2 = G4Translate3D(QLC2LEp2_X0, 0.0, QLC2LEp2_Z0);
      transform_QLC2LEp2 = transform_QLC2LEp2 * G4RotateY3D(QLC2LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2LEp2 = new G4Polycone("geo_QLC2LEp2xx_name", 0.0, 2 * M_PI, N, QLC2LEp2_Z, QLC2LEp2_r, QLC2LEp2_R);

      string strMat_QLC2LEp2 = cQLC2LEp2.getString("Material");
      G4Material* mat_QLC2LEp2 = Materials::get(strMat_QLC2LEp2);
      G4LogicalVolume* logi_QLC2LEp2 = new G4LogicalVolume(geo_QLC2LEp2, mat_QLC2LEp2, "logi_QLC2LEp2_name");

      //put volume
      setColor(*logi_QLC2LEp2, cQLC2LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2LEp2, false);
      new G4PVPlacement(transform_QLC2LEp2, logi_QLC2LEp2, "phys_QLC2LEp2_name", &topVolume, false, 0);

      //--------------
      //-   QKBLEp1

      //get parameters from .xml file
      GearDir cQKBLEp1(content, "QKBLEp1/");

      double QKBLEp1_Z[N];
      double QKBLEp1_R[N];
      double QKBLEp1_r[N];
      QKBLEp1_Z[0] = 0;
      QKBLEp1_Z[1] = cQKBLEp1.getLength("L") * unitFactor;
      QKBLEp1_R[0] = cQKBLEp1.getLength("R") * unitFactor;
      QKBLEp1_R[1] = cQKBLEp1.getLength("R") * unitFactor;
      QKBLEp1_r[0] = cQKBLEp1.getLength("r") * unitFactor;
      QKBLEp1_r[1] = cQKBLEp1.getLength("r") * unitFactor;

      double QKBLEp1_X0 = cQKBLEp1.getLength("X0") * unitFactor;
      double QKBLEp1_Z0 = cQKBLEp1.getLength("Z0") * unitFactor;
      double QKBLEp1_PHI = cQKBLEp1.getLength("PHI");
      G4Transform3D transform_QKBLEp1 = G4Translate3D(QKBLEp1_X0, 0.0, QKBLEp1_Z0);
      transform_QKBLEp1 = transform_QKBLEp1 * G4RotateY3D(QKBLEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBLEp1 = new G4Polycone("geo_QKBLEp1xx_name", 0.0, 2 * M_PI, N, QKBLEp1_Z, QKBLEp1_r, QKBLEp1_R);

      string strMat_QKBLEp1 = cQKBLEp1.getString("Material");
      G4Material* mat_QKBLEp1 = Materials::get(strMat_QKBLEp1);
      G4LogicalVolume* logi_QKBLEp1 = new G4LogicalVolume(geo_QKBLEp1, mat_QKBLEp1, "logi_QKBLEp1_name");

      //put volume
      setColor(*logi_QKBLEp1, cQKBLEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBLEp1, false);
      new G4PVPlacement(transform_QKBLEp1, logi_QKBLEp1, "phys_QKBLEp1_name", &topVolume, false, 0);

      //--------------
      //-   QKBLEp2

      //get parameters from .xml file
      GearDir cQKBLEp2(content, "QKBLEp2/");

      double QKBLEp2_Z[N];
      double QKBLEp2_R[N];
      double QKBLEp2_r[N];
      QKBLEp2_Z[0] = 0;
      QKBLEp2_Z[1] = cQKBLEp2.getLength("L") * unitFactor;
      QKBLEp2_R[0] = cQKBLEp2.getLength("R") * unitFactor;
      QKBLEp2_R[1] = cQKBLEp2.getLength("R") * unitFactor;
      QKBLEp2_r[0] = cQKBLEp2.getLength("r") * unitFactor;
      QKBLEp2_r[1] = cQKBLEp2.getLength("r") * unitFactor;

      double QKBLEp2_X0 = cQKBLEp2.getLength("X0") * unitFactor;
      double QKBLEp2_Z0 = cQKBLEp2.getLength("Z0") * unitFactor;
      double QKBLEp2_PHI = cQKBLEp2.getLength("PHI");
      G4Transform3D transform_QKBLEp2 = G4Translate3D(QKBLEp2_X0, 0.0, QKBLEp2_Z0);
      transform_QKBLEp2 = transform_QKBLEp2 * G4RotateY3D(QKBLEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBLEp2 = new G4Polycone("geo_QKBLEp2xx_name", 0.0, 2 * M_PI, N, QKBLEp2_Z, QKBLEp2_r, QKBLEp2_R);

      string strMat_QKBLEp2 = cQKBLEp2.getString("Material");
      G4Material* mat_QKBLEp2 = Materials::get(strMat_QKBLEp2);
      G4LogicalVolume* logi_QKBLEp2 = new G4LogicalVolume(geo_QKBLEp2, mat_QKBLEp2, "logi_QKBLEp2_name");

      //put volume
      setColor(*logi_QKBLEp2, cQKBLEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBLEp2, false);
      new G4PVPlacement(transform_QKBLEp2, logi_QKBLEp2, "phys_QKBLEp2_name", &topVolume, false, 0);

      //--------------
      //-   QKCLEp1

      //get parameters from .xml file
      GearDir cQKCLEp1(content, "QKCLEp1/");

      double QKCLEp1_Z[N];
      double QKCLEp1_R[N];
      double QKCLEp1_r[N];
      QKCLEp1_Z[0] = 0;
      QKCLEp1_Z[1] = cQKCLEp1.getLength("L") * unitFactor;
      QKCLEp1_R[0] = cQKCLEp1.getLength("R") * unitFactor;
      QKCLEp1_R[1] = cQKCLEp1.getLength("R") * unitFactor;
      QKCLEp1_r[0] = cQKCLEp1.getLength("r") * unitFactor;
      QKCLEp1_r[1] = cQKCLEp1.getLength("r") * unitFactor;

      double QKCLEp1_X0 = cQKCLEp1.getLength("X0") * unitFactor;
      double QKCLEp1_Z0 = cQKCLEp1.getLength("Z0") * unitFactor;
      double QKCLEp1_PHI = cQKCLEp1.getLength("PHI");
      G4Transform3D transform_QKCLEp1 = G4Translate3D(QKCLEp1_X0, 0.0, QKCLEp1_Z0);
      transform_QKCLEp1 = transform_QKCLEp1 * G4RotateY3D(QKCLEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCLEp1 = new G4Polycone("geo_QKCLEp1xx_name", 0.0, 2 * M_PI, N, QKCLEp1_Z, QKCLEp1_r, QKCLEp1_R);

      string strMat_QKCLEp1 = cQKCLEp1.getString("Material");
      G4Material* mat_QKCLEp1 = Materials::get(strMat_QKCLEp1);
      G4LogicalVolume* logi_QKCLEp1 = new G4LogicalVolume(geo_QKCLEp1, mat_QKCLEp1, "logi_QKCLEp1_name");

      //put volume
      setColor(*logi_QKCLEp1, cQKCLEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCLEp1, false);
      new G4PVPlacement(transform_QKCLEp1, logi_QKCLEp1, "phys_QKCLEp1_name", &topVolume, false, 0);

      //--------------
      //-   QKCLEp2

      //get parameters from .xml file
      GearDir cQKCLEp2(content, "QKCLEp2/");

      double QKCLEp2_Z[N];
      double QKCLEp2_R[N];
      double QKCLEp2_r[N];
      QKCLEp2_Z[0] = 0;
      QKCLEp2_Z[1] = cQKCLEp2.getLength("L") * unitFactor;
      QKCLEp2_R[0] = cQKCLEp2.getLength("R") * unitFactor;
      QKCLEp2_R[1] = cQKCLEp2.getLength("R") * unitFactor;
      QKCLEp2_r[0] = cQKCLEp2.getLength("r") * unitFactor;
      QKCLEp2_r[1] = cQKCLEp2.getLength("r") * unitFactor;

      double QKCLEp2_X0 = cQKCLEp2.getLength("X0") * unitFactor;
      double QKCLEp2_Z0 = cQKCLEp2.getLength("Z0") * unitFactor;
      double QKCLEp2_PHI = cQKCLEp2.getLength("PHI");
      G4Transform3D transform_QKCLEp2 = G4Translate3D(QKCLEp2_X0, 0.0, QKCLEp2_Z0);
      transform_QKCLEp2 = transform_QKCLEp2 * G4RotateY3D(QKCLEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCLEp2 = new G4Polycone("geo_QKCLEp2xx_name", 0.0, 2 * M_PI, N, QKCLEp2_Z, QKCLEp2_r, QKCLEp2_R);

      string strMat_QKCLEp2 = cQKCLEp2.getString("Material");
      G4Material* mat_QKCLEp2 = Materials::get(strMat_QKCLEp2);
      G4LogicalVolume* logi_QKCLEp2 = new G4LogicalVolume(geo_QKCLEp2, mat_QKCLEp2, "logi_QKCLEp2_name");

      //put volume
      setColor(*logi_QKCLEp2, cQKCLEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCLEp2, false);
      new G4PVPlacement(transform_QKCLEp2, logi_QKCLEp2, "phys_QKCLEp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC3LEp1

      //get parameters from .xml file
      GearDir cQLC3LEp1(content, "QLC3LEp1/");

      double QLC3LEp1_Z[N];
      double QLC3LEp1_R[N];
      double QLC3LEp1_r[N];
      QLC3LEp1_Z[0] = 0;
      QLC3LEp1_Z[1] = cQLC3LEp1.getLength("L") * unitFactor;
      QLC3LEp1_R[0] = cQLC3LEp1.getLength("R") * unitFactor;
      QLC3LEp1_R[1] = cQLC3LEp1.getLength("R") * unitFactor;
      QLC3LEp1_r[0] = cQLC3LEp1.getLength("r") * unitFactor;
      QLC3LEp1_r[1] = cQLC3LEp1.getLength("r") * unitFactor;

      double QLC3LEp1_X0 = cQLC3LEp1.getLength("X0") * unitFactor;
      double QLC3LEp1_Z0 = cQLC3LEp1.getLength("Z0") * unitFactor;
      double QLC3LEp1_PHI = cQLC3LEp1.getLength("PHI");
      G4Transform3D transform_QLC3LEp1 = G4Translate3D(QLC3LEp1_X0, 0.0, QLC3LEp1_Z0);
      transform_QLC3LEp1 = transform_QLC3LEp1 * G4RotateY3D(QLC3LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3LEp1 = new G4Polycone("geo_QLC3LEp1xx_name", 0.0, 2 * M_PI, N, QLC3LEp1_Z, QLC3LEp1_r, QLC3LEp1_R);

      string strMat_QLC3LEp1 = cQLC3LEp1.getString("Material");
      G4Material* mat_QLC3LEp1 = Materials::get(strMat_QLC3LEp1);
      G4LogicalVolume* logi_QLC3LEp1 = new G4LogicalVolume(geo_QLC3LEp1, mat_QLC3LEp1, "logi_QLC3LEp1_name");

      //put volume
      setColor(*logi_QLC3LEp1, cQLC3LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3LEp1, false);
      new G4PVPlacement(transform_QLC3LEp1, logi_QLC3LEp1, "phys_QLC3LEp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC3LEp2

      //get parameters from .xml file
      GearDir cQLC3LEp2(content, "QLC3LEp2/");

      double QLC3LEp2_Z[N];
      double QLC3LEp2_R[N];
      double QLC3LEp2_r[N];
      QLC3LEp2_Z[0] = 0;
      QLC3LEp2_Z[1] = cQLC3LEp2.getLength("L") * unitFactor;
      QLC3LEp2_R[0] = cQLC3LEp2.getLength("R") * unitFactor;
      QLC3LEp2_R[1] = cQLC3LEp2.getLength("R") * unitFactor;
      QLC3LEp2_r[0] = cQLC3LEp2.getLength("r") * unitFactor;
      QLC3LEp2_r[1] = cQLC3LEp2.getLength("r") * unitFactor;

      double QLC3LEp2_X0 = cQLC3LEp2.getLength("X0") * unitFactor;
      double QLC3LEp2_Z0 = cQLC3LEp2.getLength("Z0") * unitFactor;
      double QLC3LEp2_PHI = cQLC3LEp2.getLength("PHI");
      G4Transform3D transform_QLC3LEp2 = G4Translate3D(QLC3LEp2_X0, 0.0, QLC3LEp2_Z0);
      transform_QLC3LEp2 = transform_QLC3LEp2 * G4RotateY3D(QLC3LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3LEp2 = new G4Polycone("geo_QLC3LEp2xx_name", 0.0, 2 * M_PI, N, QLC3LEp2_Z, QLC3LEp2_r, QLC3LEp2_R);

      string strMat_QLC3LEp2 = cQLC3LEp2.getString("Material");
      G4Material* mat_QLC3LEp2 = Materials::get(strMat_QLC3LEp2);
      G4LogicalVolume* logi_QLC3LEp2 = new G4LogicalVolume(geo_QLC3LEp2, mat_QLC3LEp2, "logi_QLC3LEp2_name");

      //put volume
      setColor(*logi_QLC3LEp2, cQLC3LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3LEp2, false);
      new G4PVPlacement(transform_QLC3LEp2, logi_QLC3LEp2, "phys_QLC3LEp2_name", &topVolume, false, 0);

      //--------------
      //-   QKDLEp1

      //get parameters from .xml file
      GearDir cQKDLEp1(content, "QKDLEp1/");

      double QKDLEp1_Z[N];
      double QKDLEp1_R[N];
      double QKDLEp1_r[N];
      QKDLEp1_Z[0] = 0;
      QKDLEp1_Z[1] = cQKDLEp1.getLength("L") * unitFactor;
      QKDLEp1_R[0] = cQKDLEp1.getLength("R") * unitFactor;
      QKDLEp1_R[1] = cQKDLEp1.getLength("R") * unitFactor;
      QKDLEp1_r[0] = cQKDLEp1.getLength("r") * unitFactor;
      QKDLEp1_r[1] = cQKDLEp1.getLength("r") * unitFactor;

      double QKDLEp1_X0 = cQKDLEp1.getLength("X0") * unitFactor;
      double QKDLEp1_Z0 = cQKDLEp1.getLength("Z0") * unitFactor;
      double QKDLEp1_PHI = cQKDLEp1.getLength("PHI");
      G4Transform3D transform_QKDLEp1 = G4Translate3D(QKDLEp1_X0, 0.0, QKDLEp1_Z0);
      transform_QKDLEp1 = transform_QKDLEp1 * G4RotateY3D(QKDLEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKDLEp1 = new G4Polycone("geo_QKDLEp1xx_name", 0.0, 2 * M_PI, N, QKDLEp1_Z, QKDLEp1_r, QKDLEp1_R);

      string strMat_QKDLEp1 = cQKDLEp1.getString("Material");
      G4Material* mat_QKDLEp1 = Materials::get(strMat_QKDLEp1);
      G4LogicalVolume* logi_QKDLEp1 = new G4LogicalVolume(geo_QKDLEp1, mat_QKDLEp1, "logi_QKDLEp1_name");

      //put volume
      setColor(*logi_QKDLEp1, cQKDLEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKDLEp1, false);
      new G4PVPlacement(transform_QKDLEp1, logi_QKDLEp1, "phys_QKDLEp1_name", &topVolume, false, 0);

      //--------------
      //-   QKDLEp2

      //get parameters from .xml file
      GearDir cQKDLEp2(content, "QKDLEp2/");

      double QKDLEp2_Z[N];
      double QKDLEp2_R[N];
      double QKDLEp2_r[N];
      QKDLEp2_Z[0] = 0;
      QKDLEp2_Z[1] = cQKDLEp2.getLength("L") * unitFactor;
      QKDLEp2_R[0] = cQKDLEp2.getLength("R") * unitFactor;
      QKDLEp2_R[1] = cQKDLEp2.getLength("R") * unitFactor;
      QKDLEp2_r[0] = cQKDLEp2.getLength("r") * unitFactor;
      QKDLEp2_r[1] = cQKDLEp2.getLength("r") * unitFactor;

      double QKDLEp2_X0 = cQKDLEp2.getLength("X0") * unitFactor;
      double QKDLEp2_Z0 = cQKDLEp2.getLength("Z0") * unitFactor;
      double QKDLEp2_PHI = cQKDLEp2.getLength("PHI");
      G4Transform3D transform_QKDLEp2 = G4Translate3D(QKDLEp2_X0, 0.0, QKDLEp2_Z0);
      transform_QKDLEp2 = transform_QKDLEp2 * G4RotateY3D(QKDLEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKDLEp2 = new G4Polycone("geo_QKDLEp2xx_name", 0.0, 2 * M_PI, N, QKDLEp2_Z, QKDLEp2_r, QKDLEp2_R);

      string strMat_QKDLEp2 = cQKDLEp2.getString("Material");
      G4Material* mat_QKDLEp2 = Materials::get(strMat_QKDLEp2);
      G4LogicalVolume* logi_QKDLEp2 = new G4LogicalVolume(geo_QKDLEp2, mat_QKDLEp2, "logi_QKDLEp2_name");

      //put volume
      setColor(*logi_QKDLEp2, cQKDLEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKDLEp2, false);
      new G4PVPlacement(transform_QKDLEp2, logi_QKDLEp2, "phys_QKDLEp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC4LEp1

      //get parameters from .xml file
      GearDir cQLC4LEp1(content, "QLC4LEp1/");

      double QLC4LEp1_Z[N];
      double QLC4LEp1_R[N];
      double QLC4LEp1_r[N];
      QLC4LEp1_Z[0] = 0;
      QLC4LEp1_Z[1] = cQLC4LEp1.getLength("L") * unitFactor;
      QLC4LEp1_R[0] = cQLC4LEp1.getLength("R") * unitFactor;
      QLC4LEp1_R[1] = cQLC4LEp1.getLength("R") * unitFactor;
      QLC4LEp1_r[0] = cQLC4LEp1.getLength("r") * unitFactor;
      QLC4LEp1_r[1] = cQLC4LEp1.getLength("r") * unitFactor;

      double QLC4LEp1_X0 = cQLC4LEp1.getLength("X0") * unitFactor;
      double QLC4LEp1_Z0 = cQLC4LEp1.getLength("Z0") * unitFactor;
      double QLC4LEp1_PHI = cQLC4LEp1.getLength("PHI");
      G4Transform3D transform_QLC4LEp1 = G4Translate3D(QLC4LEp1_X0, 0.0, QLC4LEp1_Z0);
      transform_QLC4LEp1 = transform_QLC4LEp1 * G4RotateY3D(QLC4LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC4LEp1 = new G4Polycone("geo_QLC4LEp1xx_name", 0.0, 2 * M_PI, N, QLC4LEp1_Z, QLC4LEp1_r, QLC4LEp1_R);

      string strMat_QLC4LEp1 = cQLC4LEp1.getString("Material");
      G4Material* mat_QLC4LEp1 = Materials::get(strMat_QLC4LEp1);
      G4LogicalVolume* logi_QLC4LEp1 = new G4LogicalVolume(geo_QLC4LEp1, mat_QLC4LEp1, "logi_QLC4LEp1_name");

      //put volume
      setColor(*logi_QLC4LEp1, cQLC4LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC4LEp1, false);
      new G4PVPlacement(transform_QLC4LEp1, logi_QLC4LEp1, "phys_QLC4LEp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC4LEp2

      //get parameters from .xml file
      GearDir cQLC4LEp2(content, "QLC4LEp2/");

      double QLC4LEp2_Z[N];
      double QLC4LEp2_R[N];
      double QLC4LEp2_r[N];
      QLC4LEp2_Z[0] = 0;
      QLC4LEp2_Z[1] = cQLC4LEp2.getLength("L") * unitFactor;
      QLC4LEp2_R[0] = cQLC4LEp2.getLength("R") * unitFactor;
      QLC4LEp2_R[1] = cQLC4LEp2.getLength("R") * unitFactor;
      QLC4LEp2_r[0] = cQLC4LEp2.getLength("r") * unitFactor;
      QLC4LEp2_r[1] = cQLC4LEp2.getLength("r") * unitFactor;

      double QLC4LEp2_X0 = cQLC4LEp2.getLength("X0") * unitFactor;
      double QLC4LEp2_Z0 = cQLC4LEp2.getLength("Z0") * unitFactor;
      double QLC4LEp2_PHI = cQLC4LEp2.getLength("PHI");
      G4Transform3D transform_QLC4LEp2 = G4Translate3D(QLC4LEp2_X0, 0.0, QLC4LEp2_Z0);
      transform_QLC4LEp2 = transform_QLC4LEp2 * G4RotateY3D(QLC4LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC4LEp2 = new G4Polycone("geo_QLC4LEp2xx_name", 0.0, 2 * M_PI, N, QLC4LEp2_Z, QLC4LEp2_r, QLC4LEp2_R);

      string strMat_QLC4LEp2 = cQLC4LEp2.getString("Material");
      G4Material* mat_QLC4LEp2 = Materials::get(strMat_QLC4LEp2);
      G4LogicalVolume* logi_QLC4LEp2 = new G4LogicalVolume(geo_QLC4LEp2, mat_QLC4LEp2, "logi_QLC4LEp2_name");

      //put volume
      setColor(*logi_QLC4LEp2, cQLC4LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC4LEp2, false);
      new G4PVPlacement(transform_QLC4LEp2, logi_QLC4LEp2, "phys_QLC4LEp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC5LEp1

      //get parameters from .xml file
      GearDir cQLC5LEp1(content, "QLC5LEp1/");

      double QLC5LEp1_Z[N];
      double QLC5LEp1_R[N];
      double QLC5LEp1_r[N];
      QLC5LEp1_Z[0] = 0;
      QLC5LEp1_Z[1] = cQLC5LEp1.getLength("L") * unitFactor;
      QLC5LEp1_R[0] = cQLC5LEp1.getLength("R") * unitFactor;
      QLC5LEp1_R[1] = cQLC5LEp1.getLength("R") * unitFactor;
      QLC5LEp1_r[0] = cQLC5LEp1.getLength("r") * unitFactor;
      QLC5LEp1_r[1] = cQLC5LEp1.getLength("r") * unitFactor;

      double QLC5LEp1_X0 = cQLC5LEp1.getLength("X0") * unitFactor;
      double QLC5LEp1_Z0 = cQLC5LEp1.getLength("Z0") * unitFactor;
      double QLC5LEp1_PHI = cQLC5LEp1.getLength("PHI");
      G4Transform3D transform_QLC5LEp1 = G4Translate3D(QLC5LEp1_X0, 0.0, QLC5LEp1_Z0);
      transform_QLC5LEp1 = transform_QLC5LEp1 * G4RotateY3D(QLC5LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC5LEp1 = new G4Polycone("geo_QLC5LEp1xx_name", 0.0, 2 * M_PI, N, QLC5LEp1_Z, QLC5LEp1_r, QLC5LEp1_R);

      string strMat_QLC5LEp1 = cQLC5LEp1.getString("Material");
      G4Material* mat_QLC5LEp1 = Materials::get(strMat_QLC5LEp1);
      G4LogicalVolume* logi_QLC5LEp1 = new G4LogicalVolume(geo_QLC5LEp1, mat_QLC5LEp1, "logi_QLC5LEp1_name");

      //put volume
      setColor(*logi_QLC5LEp1, cQLC5LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC5LEp1, false);
      new G4PVPlacement(transform_QLC5LEp1, logi_QLC5LEp1, "phys_QLC5LEp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC5LEp2

      //get parameters from .xml file
      GearDir cQLC5LEp2(content, "QLC5LEp2/");

      double QLC5LEp2_Z[N];
      double QLC5LEp2_R[N];
      double QLC5LEp2_r[N];
      QLC5LEp2_Z[0] = 0;
      QLC5LEp2_Z[1] = cQLC5LEp2.getLength("L") * unitFactor;
      QLC5LEp2_R[0] = cQLC5LEp2.getLength("R") * unitFactor;
      QLC5LEp2_R[1] = cQLC5LEp2.getLength("R") * unitFactor;
      QLC5LEp2_r[0] = cQLC5LEp2.getLength("r") * unitFactor;
      QLC5LEp2_r[1] = cQLC5LEp2.getLength("r") * unitFactor;

      double QLC5LEp2_X0 = cQLC5LEp2.getLength("X0") * unitFactor;
      double QLC5LEp2_Z0 = cQLC5LEp2.getLength("Z0") * unitFactor;
      double QLC5LEp2_PHI = cQLC5LEp2.getLength("PHI");
      G4Transform3D transform_QLC5LEp2 = G4Translate3D(QLC5LEp2_X0, 0.0, QLC5LEp2_Z0);
      transform_QLC5LEp2 = transform_QLC5LEp2 * G4RotateY3D(QLC5LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC5LEp2 = new G4Polycone("geo_QLC5LEp2xx_name", 0.0, 2 * M_PI, N, QLC5LEp2_Z, QLC5LEp2_r, QLC5LEp2_R);

      string strMat_QLC5LEp2 = cQLC5LEp2.getString("Material");
      G4Material* mat_QLC5LEp2 = Materials::get(strMat_QLC5LEp2);
      G4LogicalVolume* logi_QLC5LEp2 = new G4LogicalVolume(geo_QLC5LEp2, mat_QLC5LEp2, "logi_QLC5LEp2_name");

      //put volume
      setColor(*logi_QLC5LEp2, cQLC5LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC5LEp2, false);
      new G4PVPlacement(transform_QLC5LEp2, logi_QLC5LEp2, "phys_QLC5LEp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC7LEp1

      //get parameters from .xml file
      GearDir cQLC7LEp1(content, "QLC7LEp1/");

      double QLC7LEp1_Z[N];
      double QLC7LEp1_R[N];
      double QLC7LEp1_r[N];
      QLC7LEp1_Z[0] = 0;
      QLC7LEp1_Z[1] = cQLC7LEp1.getLength("L") * unitFactor;
      QLC7LEp1_R[0] = cQLC7LEp1.getLength("R") * unitFactor;
      QLC7LEp1_R[1] = cQLC7LEp1.getLength("R") * unitFactor;
      QLC7LEp1_r[0] = cQLC7LEp1.getLength("r") * unitFactor;
      QLC7LEp1_r[1] = cQLC7LEp1.getLength("r") * unitFactor;

      double QLC7LEp1_X0 = cQLC7LEp1.getLength("X0") * unitFactor;
      double QLC7LEp1_Z0 = cQLC7LEp1.getLength("Z0") * unitFactor;
      double QLC7LEp1_PHI = cQLC7LEp1.getLength("PHI");
      G4Transform3D transform_QLC7LEp1 = G4Translate3D(QLC7LEp1_X0, 0.0, QLC7LEp1_Z0);
      transform_QLC7LEp1 = transform_QLC7LEp1 * G4RotateY3D(QLC7LEp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC7LEp1 = new G4Polycone("geo_QLC7LEp1xx_name", 0.0, 2 * M_PI, N, QLC7LEp1_Z, QLC7LEp1_r, QLC7LEp1_R);

      string strMat_QLC7LEp1 = cQLC7LEp1.getString("Material");
      G4Material* mat_QLC7LEp1 = Materials::get(strMat_QLC7LEp1);
      G4LogicalVolume* logi_QLC7LEp1 = new G4LogicalVolume(geo_QLC7LEp1, mat_QLC7LEp1, "logi_QLC7LEp1_name");

      //put volume
      setColor(*logi_QLC7LEp1, cQLC7LEp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC7LEp1, false);
      new G4PVPlacement(transform_QLC7LEp1, logi_QLC7LEp1, "phys_QLC7LEp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC7LEp2

      //get parameters from .xml file
      GearDir cQLC7LEp2(content, "QLC7LEp2/");

      double QLC7LEp2_Z[N];
      double QLC7LEp2_R[N];
      double QLC7LEp2_r[N];
      QLC7LEp2_Z[0] = 0;
      QLC7LEp2_Z[1] = cQLC7LEp2.getLength("L") * unitFactor;
      QLC7LEp2_R[0] = cQLC7LEp2.getLength("R") * unitFactor;
      QLC7LEp2_R[1] = cQLC7LEp2.getLength("R") * unitFactor;
      QLC7LEp2_r[0] = cQLC7LEp2.getLength("r") * unitFactor;
      QLC7LEp2_r[1] = cQLC7LEp2.getLength("r") * unitFactor;

      double QLC7LEp2_X0 = cQLC7LEp2.getLength("X0") * unitFactor;
      double QLC7LEp2_Z0 = cQLC7LEp2.getLength("Z0") * unitFactor;
      double QLC7LEp2_PHI = cQLC7LEp2.getLength("PHI");
      G4Transform3D transform_QLC7LEp2 = G4Translate3D(QLC7LEp2_X0, 0.0, QLC7LEp2_Z0);
      transform_QLC7LEp2 = transform_QLC7LEp2 * G4RotateY3D(QLC7LEp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC7LEp2 = new G4Polycone("geo_QLC7LEp2xx_name", 0.0, 2 * M_PI, N, QLC7LEp2_Z, QLC7LEp2_r, QLC7LEp2_R);

      string strMat_QLC7LEp2 = cQLC7LEp2.getString("Material");
      G4Material* mat_QLC7LEp2 = Materials::get(strMat_QLC7LEp2);
      G4LogicalVolume* logi_QLC7LEp2 = new G4LogicalVolume(geo_QLC7LEp2, mat_QLC7LEp2, "logi_QLC7LEp2_name");

      //put volume
      setColor(*logi_QLC7LEp2, cQLC7LEp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC7LEp2, false);
      new G4PVPlacement(transform_QLC7LEp2, logi_QLC7LEp2, "phys_QLC7LEp2_name", &topVolume, false, 0);

      //--------------
      //-   SLYTLE1p1

      //get parameters from .xml file
      GearDir cSLYTLE1p1(content, "SLYTLE1p1/");

      double SLYTLE1p1_Z[N];
      double SLYTLE1p1_R[N];
      double SLYTLE1p1_r[N];
      SLYTLE1p1_Z[0] = 0;
      SLYTLE1p1_Z[1] = cSLYTLE1p1.getLength("L") * unitFactor;
      SLYTLE1p1_R[0] = cSLYTLE1p1.getLength("R") * unitFactor;
      SLYTLE1p1_R[1] = cSLYTLE1p1.getLength("R") * unitFactor;
      SLYTLE1p1_r[0] = cSLYTLE1p1.getLength("r") * unitFactor;
      SLYTLE1p1_r[1] = cSLYTLE1p1.getLength("r") * unitFactor;

      double SLYTLE1p1_X0 = cSLYTLE1p1.getLength("X0") * unitFactor;
      double SLYTLE1p1_Z0 = cSLYTLE1p1.getLength("Z0") * unitFactor;
      double SLYTLE1p1_PHI = cSLYTLE1p1.getLength("PHI");
      G4Transform3D transform_SLYTLE1p1 = G4Translate3D(SLYTLE1p1_X0, 0.0, SLYTLE1p1_Z0);
      transform_SLYTLE1p1 = transform_SLYTLE1p1 * G4RotateY3D(SLYTLE1p1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTLE1p1 = new G4Polycone("geo_SLYTLE1p1xx_name", 0.0, 2 * M_PI, N, SLYTLE1p1_Z, SLYTLE1p1_r, SLYTLE1p1_R);

      string strMat_SLYTLE1p1 = cSLYTLE1p1.getString("Material");
      G4Material* mat_SLYTLE1p1 = Materials::get(strMat_SLYTLE1p1);
      G4LogicalVolume* logi_SLYTLE1p1 = new G4LogicalVolume(geo_SLYTLE1p1, mat_SLYTLE1p1, "logi_SLYTLE1p1_name");

      //put volume
      setColor(*logi_SLYTLE1p1, cSLYTLE1p1.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTLE1p1, false);
      new G4PVPlacement(transform_SLYTLE1p1, logi_SLYTLE1p1, "phys_SLYTLE1p1_name", &topVolume, false, 0);

      //--------------
      //-   SLYTLE1p2

      //get parameters from .xml file
      GearDir cSLYTLE1p2(content, "SLYTLE1p2/");

      double SLYTLE1p2_Z[N];
      double SLYTLE1p2_R[N];
      double SLYTLE1p2_r[N];
      SLYTLE1p2_Z[0] = 0;
      SLYTLE1p2_Z[1] = cSLYTLE1p2.getLength("L") * unitFactor;
      SLYTLE1p2_R[0] = cSLYTLE1p2.getLength("R") * unitFactor;
      SLYTLE1p2_R[1] = cSLYTLE1p2.getLength("R") * unitFactor;
      SLYTLE1p2_r[0] = cSLYTLE1p2.getLength("r") * unitFactor;
      SLYTLE1p2_r[1] = cSLYTLE1p2.getLength("r") * unitFactor;

      double SLYTLE1p2_X0 = cSLYTLE1p2.getLength("X0") * unitFactor;
      double SLYTLE1p2_Z0 = cSLYTLE1p2.getLength("Z0") * unitFactor;
      double SLYTLE1p2_PHI = cSLYTLE1p2.getLength("PHI");
      G4Transform3D transform_SLYTLE1p2 = G4Translate3D(SLYTLE1p2_X0, 0.0, SLYTLE1p2_Z0);
      transform_SLYTLE1p2 = transform_SLYTLE1p2 * G4RotateY3D(SLYTLE1p2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTLE1p2 = new G4Polycone("geo_SLYTLE1p2xx_name", 0.0, 2 * M_PI, N, SLYTLE1p2_Z, SLYTLE1p2_r, SLYTLE1p2_R);

      string strMat_SLYTLE1p2 = cSLYTLE1p2.getString("Material");
      G4Material* mat_SLYTLE1p2 = Materials::get(strMat_SLYTLE1p2);
      G4LogicalVolume* logi_SLYTLE1p2 = new G4LogicalVolume(geo_SLYTLE1p2, mat_SLYTLE1p2, "logi_SLYTLE1p2_name");

      //put volume
      setColor(*logi_SLYTLE1p2, cSLYTLE1p2.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTLE1p2, false);
      new G4PVPlacement(transform_SLYTLE1p2, logi_SLYTLE1p2, "phys_SLYTLE1p2_name", &topVolume, false, 0);

      //--------------
      //-   QLY1LE1p1

      //get parameters from .xml file
      GearDir cQLY1LE1p1(content, "QLY1LE1p1/");

      double QLY1LE1p1_Z[N];
      double QLY1LE1p1_R[N];
      double QLY1LE1p1_r[N];
      QLY1LE1p1_Z[0] = 0;
      QLY1LE1p1_Z[1] = cQLY1LE1p1.getLength("L") * unitFactor;
      QLY1LE1p1_R[0] = cQLY1LE1p1.getLength("R") * unitFactor;
      QLY1LE1p1_R[1] = cQLY1LE1p1.getLength("R") * unitFactor;
      QLY1LE1p1_r[0] = cQLY1LE1p1.getLength("r") * unitFactor;
      QLY1LE1p1_r[1] = cQLY1LE1p1.getLength("r") * unitFactor;

      double QLY1LE1p1_X0 = cQLY1LE1p1.getLength("X0") * unitFactor;
      double QLY1LE1p1_Z0 = cQLY1LE1p1.getLength("Z0") * unitFactor;
      double QLY1LE1p1_PHI = cQLY1LE1p1.getLength("PHI");
      G4Transform3D transform_QLY1LE1p1 = G4Translate3D(QLY1LE1p1_X0, 0.0, QLY1LE1p1_Z0);
      transform_QLY1LE1p1 = transform_QLY1LE1p1 * G4RotateY3D(QLY1LE1p1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLY1LE1p1 = new G4Polycone("geo_QLY1LE1p1xx_name", 0.0, 2 * M_PI, N, QLY1LE1p1_Z, QLY1LE1p1_r, QLY1LE1p1_R);

      string strMat_QLY1LE1p1 = cQLY1LE1p1.getString("Material");
      G4Material* mat_QLY1LE1p1 = Materials::get(strMat_QLY1LE1p1);
      G4LogicalVolume* logi_QLY1LE1p1 = new G4LogicalVolume(geo_QLY1LE1p1, mat_QLY1LE1p1, "logi_QLY1LE1p1_name");

      //put volume
      setColor(*logi_QLY1LE1p1, cQLY1LE1p1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLY1LE1p1, false);
      new G4PVPlacement(transform_QLY1LE1p1, logi_QLY1LE1p1, "phys_QLY1LE1p1_name", &topVolume, false, 0);

      //--------------
      //-   QLY1LE1p2

      //get parameters from .xml file
      GearDir cQLY1LE1p2(content, "QLY1LE1p2/");

      double QLY1LE1p2_Z[N];
      double QLY1LE1p2_R[N];
      double QLY1LE1p2_r[N];
      QLY1LE1p2_Z[0] = 0;
      QLY1LE1p2_Z[1] = cQLY1LE1p2.getLength("L") * unitFactor;
      QLY1LE1p2_R[0] = cQLY1LE1p2.getLength("R") * unitFactor;
      QLY1LE1p2_R[1] = cQLY1LE1p2.getLength("R") * unitFactor;
      QLY1LE1p2_r[0] = cQLY1LE1p2.getLength("r") * unitFactor;
      QLY1LE1p2_r[1] = cQLY1LE1p2.getLength("r") * unitFactor;

      double QLY1LE1p2_X0 = cQLY1LE1p2.getLength("X0") * unitFactor;
      double QLY1LE1p2_Z0 = cQLY1LE1p2.getLength("Z0") * unitFactor;
      double QLY1LE1p2_PHI = cQLY1LE1p2.getLength("PHI");
      G4Transform3D transform_QLY1LE1p2 = G4Translate3D(QLY1LE1p2_X0, 0.0, QLY1LE1p2_Z0);
      transform_QLY1LE1p2 = transform_QLY1LE1p2 * G4RotateY3D(QLY1LE1p2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLY1LE1p2 = new G4Polycone("geo_QLY1LE1p2xx_name", 0.0, 2 * M_PI, N, QLY1LE1p2_Z, QLY1LE1p2_r, QLY1LE1p2_R);

      string strMat_QLY1LE1p2 = cQLY1LE1p2.getString("Material");
      G4Material* mat_QLY1LE1p2 = Materials::get(strMat_QLY1LE1p2);
      G4LogicalVolume* logi_QLY1LE1p2 = new G4LogicalVolume(geo_QLY1LE1p2, mat_QLY1LE1p2, "logi_QLY1LE1p2_name");

      //put volume
      setColor(*logi_QLY1LE1p2, cQLY1LE1p2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLY1LE1p2, false);
      new G4PVPlacement(transform_QLY1LE1p2, logi_QLY1LE1p2, "phys_QLY1LE1p2_name", &topVolume, false, 0);

      //--------------
      //-   QKALPp1

      //get parameters from .xml file
      GearDir cQKALPp1(content, "QKALPp1/");

      double QKALPp1_Z[N];
      double QKALPp1_R[N];
      double QKALPp1_r[N];
      QKALPp1_Z[0] = 0;
      QKALPp1_Z[1] = cQKALPp1.getLength("L") * unitFactor;
      QKALPp1_R[0] = cQKALPp1.getLength("R") * unitFactor;
      QKALPp1_R[1] = cQKALPp1.getLength("R") * unitFactor;
      QKALPp1_r[0] = cQKALPp1.getLength("r") * unitFactor;
      QKALPp1_r[1] = cQKALPp1.getLength("r") * unitFactor;

      double QKALPp1_X0 = cQKALPp1.getLength("X0") * unitFactor;
      double QKALPp1_Z0 = cQKALPp1.getLength("Z0") * unitFactor;
      double QKALPp1_PHI = cQKALPp1.getLength("PHI");
      G4Transform3D transform_QKALPp1 = G4Translate3D(QKALPp1_X0, 0.0, QKALPp1_Z0);
      transform_QKALPp1 = transform_QKALPp1 * G4RotateY3D(QKALPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKALPp1 = new G4Polycone("geo_QKALPp1xx_name", 0.0, 2 * M_PI, N, QKALPp1_Z, QKALPp1_r, QKALPp1_R);

      string strMat_QKALPp1 = cQKALPp1.getString("Material");
      G4Material* mat_QKALPp1 = Materials::get(strMat_QKALPp1);
      G4LogicalVolume* logi_QKALPp1 = new G4LogicalVolume(geo_QKALPp1, mat_QKALPp1, "logi_QKALPp1_name");

      //put volume
      setColor(*logi_QKALPp1, cQKALPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKALPp1, false);
      new G4PVPlacement(transform_QKALPp1, logi_QKALPp1, "phys_QKALPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKALPp2

      //get parameters from .xml file
      GearDir cQKALPp2(content, "QKALPp2/");

      double QKALPp2_Z[N];
      double QKALPp2_R[N];
      double QKALPp2_r[N];
      QKALPp2_Z[0] = 0;
      QKALPp2_Z[1] = cQKALPp2.getLength("L") * unitFactor;
      QKALPp2_R[0] = cQKALPp2.getLength("R") * unitFactor;
      QKALPp2_R[1] = cQKALPp2.getLength("R") * unitFactor;
      QKALPp2_r[0] = cQKALPp2.getLength("r") * unitFactor;
      QKALPp2_r[1] = cQKALPp2.getLength("r") * unitFactor;

      double QKALPp2_X0 = cQKALPp2.getLength("X0") * unitFactor;
      double QKALPp2_Z0 = cQKALPp2.getLength("Z0") * unitFactor;
      double QKALPp2_PHI = cQKALPp2.getLength("PHI");
      G4Transform3D transform_QKALPp2 = G4Translate3D(QKALPp2_X0, 0.0, QKALPp2_Z0);
      transform_QKALPp2 = transform_QKALPp2 * G4RotateY3D(QKALPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKALPp2 = new G4Polycone("geo_QKALPp2xx_name", 0.0, 2 * M_PI, N, QKALPp2_Z, QKALPp2_r, QKALPp2_R);

      string strMat_QKALPp2 = cQKALPp2.getString("Material");
      G4Material* mat_QKALPp2 = Materials::get(strMat_QKALPp2);
      G4LogicalVolume* logi_QKALPp2 = new G4LogicalVolume(geo_QKALPp2, mat_QKALPp2, "logi_QKALPp2_name");

      //put volume
      setColor(*logi_QKALPp2, cQKALPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKALPp2, false);
      new G4PVPlacement(transform_QKALPp2, logi_QKALPp2, "phys_QKALPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKBLPp1

      //get parameters from .xml file
      GearDir cQKBLPp1(content, "QKBLPp1/");

      double QKBLPp1_Z[N];
      double QKBLPp1_R[N];
      double QKBLPp1_r[N];
      QKBLPp1_Z[0] = 0;
      QKBLPp1_Z[1] = cQKBLPp1.getLength("L") * unitFactor;
      QKBLPp1_R[0] = cQKBLPp1.getLength("R") * unitFactor;
      QKBLPp1_R[1] = cQKBLPp1.getLength("R") * unitFactor;
      QKBLPp1_r[0] = cQKBLPp1.getLength("r") * unitFactor;
      QKBLPp1_r[1] = cQKBLPp1.getLength("r") * unitFactor;

      double QKBLPp1_X0 = cQKBLPp1.getLength("X0") * unitFactor;
      double QKBLPp1_Z0 = cQKBLPp1.getLength("Z0") * unitFactor;
      double QKBLPp1_PHI = cQKBLPp1.getLength("PHI");
      G4Transform3D transform_QKBLPp1 = G4Translate3D(QKBLPp1_X0, 0.0, QKBLPp1_Z0);
      transform_QKBLPp1 = transform_QKBLPp1 * G4RotateY3D(QKBLPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBLPp1 = new G4Polycone("geo_QKBLPp1xx_name", 0.0, 2 * M_PI, N, QKBLPp1_Z, QKBLPp1_r, QKBLPp1_R);

      string strMat_QKBLPp1 = cQKBLPp1.getString("Material");
      G4Material* mat_QKBLPp1 = Materials::get(strMat_QKBLPp1);
      G4LogicalVolume* logi_QKBLPp1 = new G4LogicalVolume(geo_QKBLPp1, mat_QKBLPp1, "logi_QKBLPp1_name");

      //put volume
      setColor(*logi_QKBLPp1, cQKBLPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBLPp1, false);
      new G4PVPlacement(transform_QKBLPp1, logi_QKBLPp1, "phys_QKBLPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKBLPp2

      //get parameters from .xml file
      GearDir cQKBLPp2(content, "QKBLPp2/");

      double QKBLPp2_Z[N];
      double QKBLPp2_R[N];
      double QKBLPp2_r[N];
      QKBLPp2_Z[0] = 0;
      QKBLPp2_Z[1] = cQKBLPp2.getLength("L") * unitFactor;
      QKBLPp2_R[0] = cQKBLPp2.getLength("R") * unitFactor;
      QKBLPp2_R[1] = cQKBLPp2.getLength("R") * unitFactor;
      QKBLPp2_r[0] = cQKBLPp2.getLength("r") * unitFactor;
      QKBLPp2_r[1] = cQKBLPp2.getLength("r") * unitFactor;

      double QKBLPp2_X0 = cQKBLPp2.getLength("X0") * unitFactor;
      double QKBLPp2_Z0 = cQKBLPp2.getLength("Z0") * unitFactor;
      double QKBLPp2_PHI = cQKBLPp2.getLength("PHI");
      G4Transform3D transform_QKBLPp2 = G4Translate3D(QKBLPp2_X0, 0.0, QKBLPp2_Z0);
      transform_QKBLPp2 = transform_QKBLPp2 * G4RotateY3D(QKBLPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKBLPp2 = new G4Polycone("geo_QKBLPp2xx_name", 0.0, 2 * M_PI, N, QKBLPp2_Z, QKBLPp2_r, QKBLPp2_R);

      string strMat_QKBLPp2 = cQKBLPp2.getString("Material");
      G4Material* mat_QKBLPp2 = Materials::get(strMat_QKBLPp2);
      G4LogicalVolume* logi_QKBLPp2 = new G4LogicalVolume(geo_QKBLPp2, mat_QKBLPp2, "logi_QKBLPp2_name");

      //put volume
      setColor(*logi_QKBLPp2, cQKBLPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKBLPp2, false);
      new G4PVPlacement(transform_QKBLPp2, logi_QKBLPp2, "phys_QKBLPp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC1LPp1

      //get parameters from .xml file
      GearDir cQLC1LPp1(content, "QLC1LPp1/");

      double QLC1LPp1_Z[N];
      double QLC1LPp1_R[N];
      double QLC1LPp1_r[N];
      QLC1LPp1_Z[0] = 0;
      QLC1LPp1_Z[1] = cQLC1LPp1.getLength("L") * unitFactor;
      QLC1LPp1_R[0] = cQLC1LPp1.getLength("R") * unitFactor;
      QLC1LPp1_R[1] = cQLC1LPp1.getLength("R") * unitFactor;
      QLC1LPp1_r[0] = cQLC1LPp1.getLength("r") * unitFactor;
      QLC1LPp1_r[1] = cQLC1LPp1.getLength("r") * unitFactor;

      double QLC1LPp1_X0 = cQLC1LPp1.getLength("X0") * unitFactor;
      double QLC1LPp1_Z0 = cQLC1LPp1.getLength("Z0") * unitFactor;
      double QLC1LPp1_PHI = cQLC1LPp1.getLength("PHI");
      G4Transform3D transform_QLC1LPp1 = G4Translate3D(QLC1LPp1_X0, 0.0, QLC1LPp1_Z0);
      transform_QLC1LPp1 = transform_QLC1LPp1 * G4RotateY3D(QLC1LPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC1LPp1 = new G4Polycone("geo_QLC1LPp1xx_name", 0.0, 2 * M_PI, N, QLC1LPp1_Z, QLC1LPp1_r, QLC1LPp1_R);

      string strMat_QLC1LPp1 = cQLC1LPp1.getString("Material");
      G4Material* mat_QLC1LPp1 = Materials::get(strMat_QLC1LPp1);
      G4LogicalVolume* logi_QLC1LPp1 = new G4LogicalVolume(geo_QLC1LPp1, mat_QLC1LPp1, "logi_QLC1LPp1_name");

      //put volume
      setColor(*logi_QLC1LPp1, cQLC1LPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC1LPp1, false);
      new G4PVPlacement(transform_QLC1LPp1, logi_QLC1LPp1, "phys_QLC1LPp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC1LPp2

      //get parameters from .xml file
      GearDir cQLC1LPp2(content, "QLC1LPp2/");

      double QLC1LPp2_Z[N];
      double QLC1LPp2_R[N];
      double QLC1LPp2_r[N];
      QLC1LPp2_Z[0] = 0;
      QLC1LPp2_Z[1] = cQLC1LPp2.getLength("L") * unitFactor;
      QLC1LPp2_R[0] = cQLC1LPp2.getLength("R") * unitFactor;
      QLC1LPp2_R[1] = cQLC1LPp2.getLength("R") * unitFactor;
      QLC1LPp2_r[0] = cQLC1LPp2.getLength("r") * unitFactor;
      QLC1LPp2_r[1] = cQLC1LPp2.getLength("r") * unitFactor;

      double QLC1LPp2_X0 = cQLC1LPp2.getLength("X0") * unitFactor;
      double QLC1LPp2_Z0 = cQLC1LPp2.getLength("Z0") * unitFactor;
      double QLC1LPp2_PHI = cQLC1LPp2.getLength("PHI");
      G4Transform3D transform_QLC1LPp2 = G4Translate3D(QLC1LPp2_X0, 0.0, QLC1LPp2_Z0);
      transform_QLC1LPp2 = transform_QLC1LPp2 * G4RotateY3D(QLC1LPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC1LPp2 = new G4Polycone("geo_QLC1LPp2xx_name", 0.0, 2 * M_PI, N, QLC1LPp2_Z, QLC1LPp2_r, QLC1LPp2_R);

      string strMat_QLC1LPp2 = cQLC1LPp2.getString("Material");
      G4Material* mat_QLC1LPp2 = Materials::get(strMat_QLC1LPp2);
      G4LogicalVolume* logi_QLC1LPp2 = new G4LogicalVolume(geo_QLC1LPp2, mat_QLC1LPp2, "logi_QLC1LPp2_name");

      //put volume
      setColor(*logi_QLC1LPp2, cQLC1LPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC1LPp2, false);
      new G4PVPlacement(transform_QLC1LPp2, logi_QLC1LPp2, "phys_QLC1LPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKCLPp1

      //get parameters from .xml file
      GearDir cQKCLPp1(content, "QKCLPp1/");

      double QKCLPp1_Z[N];
      double QKCLPp1_R[N];
      double QKCLPp1_r[N];
      QKCLPp1_Z[0] = 0;
      QKCLPp1_Z[1] = cQKCLPp1.getLength("L") * unitFactor;
      QKCLPp1_R[0] = cQKCLPp1.getLength("R") * unitFactor;
      QKCLPp1_R[1] = cQKCLPp1.getLength("R") * unitFactor;
      QKCLPp1_r[0] = cQKCLPp1.getLength("r") * unitFactor;
      QKCLPp1_r[1] = cQKCLPp1.getLength("r") * unitFactor;

      double QKCLPp1_X0 = cQKCLPp1.getLength("X0") * unitFactor;
      double QKCLPp1_Z0 = cQKCLPp1.getLength("Z0") * unitFactor;
      double QKCLPp1_PHI = cQKCLPp1.getLength("PHI");
      G4Transform3D transform_QKCLPp1 = G4Translate3D(QKCLPp1_X0, 0.0, QKCLPp1_Z0);
      transform_QKCLPp1 = transform_QKCLPp1 * G4RotateY3D(QKCLPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCLPp1 = new G4Polycone("geo_QKCLPp1xx_name", 0.0, 2 * M_PI, N, QKCLPp1_Z, QKCLPp1_r, QKCLPp1_R);

      string strMat_QKCLPp1 = cQKCLPp1.getString("Material");
      G4Material* mat_QKCLPp1 = Materials::get(strMat_QKCLPp1);
      G4LogicalVolume* logi_QKCLPp1 = new G4LogicalVolume(geo_QKCLPp1, mat_QKCLPp1, "logi_QKCLPp1_name");

      //put volume
      setColor(*logi_QKCLPp1, cQKCLPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCLPp1, false);
      new G4PVPlacement(transform_QKCLPp1, logi_QKCLPp1, "phys_QKCLPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKCLPp2

      //get parameters from .xml file
      GearDir cQKCLPp2(content, "QKCLPp2/");

      double QKCLPp2_Z[N];
      double QKCLPp2_R[N];
      double QKCLPp2_r[N];
      QKCLPp2_Z[0] = 0;
      QKCLPp2_Z[1] = cQKCLPp2.getLength("L") * unitFactor;
      QKCLPp2_R[0] = cQKCLPp2.getLength("R") * unitFactor;
      QKCLPp2_R[1] = cQKCLPp2.getLength("R") * unitFactor;
      QKCLPp2_r[0] = cQKCLPp2.getLength("r") * unitFactor;
      QKCLPp2_r[1] = cQKCLPp2.getLength("r") * unitFactor;

      double QKCLPp2_X0 = cQKCLPp2.getLength("X0") * unitFactor;
      double QKCLPp2_Z0 = cQKCLPp2.getLength("Z0") * unitFactor;
      double QKCLPp2_PHI = cQKCLPp2.getLength("PHI");
      G4Transform3D transform_QKCLPp2 = G4Translate3D(QKCLPp2_X0, 0.0, QKCLPp2_Z0);
      transform_QKCLPp2 = transform_QKCLPp2 * G4RotateY3D(QKCLPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKCLPp2 = new G4Polycone("geo_QKCLPp2xx_name", 0.0, 2 * M_PI, N, QKCLPp2_Z, QKCLPp2_r, QKCLPp2_R);

      string strMat_QKCLPp2 = cQKCLPp2.getString("Material");
      G4Material* mat_QKCLPp2 = Materials::get(strMat_QKCLPp2);
      G4LogicalVolume* logi_QKCLPp2 = new G4LogicalVolume(geo_QKCLPp2, mat_QKCLPp2, "logi_QKCLPp2_name");

      //put volume
      setColor(*logi_QKCLPp2, cQKCLPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKCLPp2, false);
      new G4PVPlacement(transform_QKCLPp2, logi_QKCLPp2, "phys_QKCLPp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC2LPp1

      //get parameters from .xml file
      GearDir cQLC2LPp1(content, "QLC2LPp1/");

      double QLC2LPp1_Z[N];
      double QLC2LPp1_R[N];
      double QLC2LPp1_r[N];
      QLC2LPp1_Z[0] = 0;
      QLC2LPp1_Z[1] = cQLC2LPp1.getLength("L") * unitFactor;
      QLC2LPp1_R[0] = cQLC2LPp1.getLength("R") * unitFactor;
      QLC2LPp1_R[1] = cQLC2LPp1.getLength("R") * unitFactor;
      QLC2LPp1_r[0] = cQLC2LPp1.getLength("r") * unitFactor;
      QLC2LPp1_r[1] = cQLC2LPp1.getLength("r") * unitFactor;

      double QLC2LPp1_X0 = cQLC2LPp1.getLength("X0") * unitFactor;
      double QLC2LPp1_Z0 = cQLC2LPp1.getLength("Z0") * unitFactor;
      double QLC2LPp1_PHI = cQLC2LPp1.getLength("PHI");
      G4Transform3D transform_QLC2LPp1 = G4Translate3D(QLC2LPp1_X0, 0.0, QLC2LPp1_Z0);
      transform_QLC2LPp1 = transform_QLC2LPp1 * G4RotateY3D(QLC2LPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2LPp1 = new G4Polycone("geo_QLC2LPp1xx_name", 0.0, 2 * M_PI, N, QLC2LPp1_Z, QLC2LPp1_r, QLC2LPp1_R);

      string strMat_QLC2LPp1 = cQLC2LPp1.getString("Material");
      G4Material* mat_QLC2LPp1 = Materials::get(strMat_QLC2LPp1);
      G4LogicalVolume* logi_QLC2LPp1 = new G4LogicalVolume(geo_QLC2LPp1, mat_QLC2LPp1, "logi_QLC2LPp1_name");

      //put volume
      setColor(*logi_QLC2LPp1, cQLC2LPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2LPp1, false);
      new G4PVPlacement(transform_QLC2LPp1, logi_QLC2LPp1, "phys_QLC2LPp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC2LPp2

      //get parameters from .xml file
      GearDir cQLC2LPp2(content, "QLC2LPp2/");

      double QLC2LPp2_Z[N];
      double QLC2LPp2_R[N];
      double QLC2LPp2_r[N];
      QLC2LPp2_Z[0] = 0;
      QLC2LPp2_Z[1] = cQLC2LPp2.getLength("L") * unitFactor;
      QLC2LPp2_R[0] = cQLC2LPp2.getLength("R") * unitFactor;
      QLC2LPp2_R[1] = cQLC2LPp2.getLength("R") * unitFactor;
      QLC2LPp2_r[0] = cQLC2LPp2.getLength("r") * unitFactor;
      QLC2LPp2_r[1] = cQLC2LPp2.getLength("r") * unitFactor;

      double QLC2LPp2_X0 = cQLC2LPp2.getLength("X0") * unitFactor;
      double QLC2LPp2_Z0 = cQLC2LPp2.getLength("Z0") * unitFactor;
      double QLC2LPp2_PHI = cQLC2LPp2.getLength("PHI");
      G4Transform3D transform_QLC2LPp2 = G4Translate3D(QLC2LPp2_X0, 0.0, QLC2LPp2_Z0);
      transform_QLC2LPp2 = transform_QLC2LPp2 * G4RotateY3D(QLC2LPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC2LPp2 = new G4Polycone("geo_QLC2LPp2xx_name", 0.0, 2 * M_PI, N, QLC2LPp2_Z, QLC2LPp2_r, QLC2LPp2_R);

      string strMat_QLC2LPp2 = cQLC2LPp2.getString("Material");
      G4Material* mat_QLC2LPp2 = Materials::get(strMat_QLC2LPp2);
      G4LogicalVolume* logi_QLC2LPp2 = new G4LogicalVolume(geo_QLC2LPp2, mat_QLC2LPp2, "logi_QLC2LPp2_name");

      //put volume
      setColor(*logi_QLC2LPp2, cQLC2LPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC2LPp2, false);
      new G4PVPlacement(transform_QLC2LPp2, logi_QLC2LPp2, "phys_QLC2LPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKELPp1

      //get parameters from .xml file
      GearDir cQKELPp1(content, "QKELPp1/");

      double QKELPp1_Z[N];
      double QKELPp1_R[N];
      double QKELPp1_r[N];
      QKELPp1_Z[0] = 0;
      QKELPp1_Z[1] = cQKELPp1.getLength("L") * unitFactor;
      QKELPp1_R[0] = cQKELPp1.getLength("R") * unitFactor;
      QKELPp1_R[1] = cQKELPp1.getLength("R") * unitFactor;
      QKELPp1_r[0] = cQKELPp1.getLength("r") * unitFactor;
      QKELPp1_r[1] = cQKELPp1.getLength("r") * unitFactor;

      double QKELPp1_X0 = cQKELPp1.getLength("X0") * unitFactor;
      double QKELPp1_Z0 = cQKELPp1.getLength("Z0") * unitFactor;
      double QKELPp1_PHI = cQKELPp1.getLength("PHI");
      G4Transform3D transform_QKELPp1 = G4Translate3D(QKELPp1_X0, 0.0, QKELPp1_Z0);
      transform_QKELPp1 = transform_QKELPp1 * G4RotateY3D(QKELPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKELPp1 = new G4Polycone("geo_QKELPp1xx_name", 0.0, 2 * M_PI, N, QKELPp1_Z, QKELPp1_r, QKELPp1_R);

      string strMat_QKELPp1 = cQKELPp1.getString("Material");
      G4Material* mat_QKELPp1 = Materials::get(strMat_QKELPp1);
      G4LogicalVolume* logi_QKELPp1 = new G4LogicalVolume(geo_QKELPp1, mat_QKELPp1, "logi_QKELPp1_name");

      //put volume
      setColor(*logi_QKELPp1, cQKELPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKELPp1, false);
      new G4PVPlacement(transform_QKELPp1, logi_QKELPp1, "phys_QKELPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKELPp2

      //get parameters from .xml file
      GearDir cQKELPp2(content, "QKELPp2/");

      double QKELPp2_Z[N];
      double QKELPp2_R[N];
      double QKELPp2_r[N];
      QKELPp2_Z[0] = 0;
      QKELPp2_Z[1] = cQKELPp2.getLength("L") * unitFactor;
      QKELPp2_R[0] = cQKELPp2.getLength("R") * unitFactor;
      QKELPp2_R[1] = cQKELPp2.getLength("R") * unitFactor;
      QKELPp2_r[0] = cQKELPp2.getLength("r") * unitFactor;
      QKELPp2_r[1] = cQKELPp2.getLength("r") * unitFactor;

      double QKELPp2_X0 = cQKELPp2.getLength("X0") * unitFactor;
      double QKELPp2_Z0 = cQKELPp2.getLength("Z0") * unitFactor;
      double QKELPp2_PHI = cQKELPp2.getLength("PHI");
      G4Transform3D transform_QKELPp2 = G4Translate3D(QKELPp2_X0, 0.0, QKELPp2_Z0);
      transform_QKELPp2 = transform_QKELPp2 * G4RotateY3D(QKELPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKELPp2 = new G4Polycone("geo_QKELPp2xx_name", 0.0, 2 * M_PI, N, QKELPp2_Z, QKELPp2_r, QKELPp2_R);

      string strMat_QKELPp2 = cQKELPp2.getString("Material");
      G4Material* mat_QKELPp2 = Materials::get(strMat_QKELPp2);
      G4LogicalVolume* logi_QKELPp2 = new G4LogicalVolume(geo_QKELPp2, mat_QKELPp2, "logi_QKELPp2_name");

      //put volume
      setColor(*logi_QKELPp2, cQKELPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKELPp2, false);
      new G4PVPlacement(transform_QKELPp2, logi_QKELPp2, "phys_QKELPp2_name", &topVolume, false, 0);

      //--------------
      //-   QKFLPp1

      //get parameters from .xml file
      GearDir cQKFLPp1(content, "QKFLPp1/");

      double QKFLPp1_Z[N];
      double QKFLPp1_R[N];
      double QKFLPp1_r[N];
      QKFLPp1_Z[0] = 0;
      QKFLPp1_Z[1] = cQKFLPp1.getLength("L") * unitFactor;
      QKFLPp1_R[0] = cQKFLPp1.getLength("R") * unitFactor;
      QKFLPp1_R[1] = cQKFLPp1.getLength("R") * unitFactor;
      QKFLPp1_r[0] = cQKFLPp1.getLength("r") * unitFactor;
      QKFLPp1_r[1] = cQKFLPp1.getLength("r") * unitFactor;

      double QKFLPp1_X0 = cQKFLPp1.getLength("X0") * unitFactor;
      double QKFLPp1_Z0 = cQKFLPp1.getLength("Z0") * unitFactor;
      double QKFLPp1_PHI = cQKFLPp1.getLength("PHI");
      G4Transform3D transform_QKFLPp1 = G4Translate3D(QKFLPp1_X0, 0.0, QKFLPp1_Z0);
      transform_QKFLPp1 = transform_QKFLPp1 * G4RotateY3D(QKFLPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKFLPp1 = new G4Polycone("geo_QKFLPp1xx_name", 0.0, 2 * M_PI, N, QKFLPp1_Z, QKFLPp1_r, QKFLPp1_R);

      string strMat_QKFLPp1 = cQKFLPp1.getString("Material");
      G4Material* mat_QKFLPp1 = Materials::get(strMat_QKFLPp1);
      G4LogicalVolume* logi_QKFLPp1 = new G4LogicalVolume(geo_QKFLPp1, mat_QKFLPp1, "logi_QKFLPp1_name");

      //put volume
      setColor(*logi_QKFLPp1, cQKFLPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKFLPp1, false);
      new G4PVPlacement(transform_QKFLPp1, logi_QKFLPp1, "phys_QKFLPp1_name", &topVolume, false, 0);

      //--------------
      //-   QKFLPp2

      //get parameters from .xml file
      GearDir cQKFLPp2(content, "QKFLPp2/");

      double QKFLPp2_Z[N];
      double QKFLPp2_R[N];
      double QKFLPp2_r[N];
      QKFLPp2_Z[0] = 0;
      QKFLPp2_Z[1] = cQKFLPp2.getLength("L") * unitFactor;
      QKFLPp2_R[0] = cQKFLPp2.getLength("R") * unitFactor;
      QKFLPp2_R[1] = cQKFLPp2.getLength("R") * unitFactor;
      QKFLPp2_r[0] = cQKFLPp2.getLength("r") * unitFactor;
      QKFLPp2_r[1] = cQKFLPp2.getLength("r") * unitFactor;

      double QKFLPp2_X0 = cQKFLPp2.getLength("X0") * unitFactor;
      double QKFLPp2_Z0 = cQKFLPp2.getLength("Z0") * unitFactor;
      double QKFLPp2_PHI = cQKFLPp2.getLength("PHI");
      G4Transform3D transform_QKFLPp2 = G4Translate3D(QKFLPp2_X0, 0.0, QKFLPp2_Z0);
      transform_QKFLPp2 = transform_QKFLPp2 * G4RotateY3D(QKFLPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QKFLPp2 = new G4Polycone("geo_QKFLPp2xx_name", 0.0, 2 * M_PI, N, QKFLPp2_Z, QKFLPp2_r, QKFLPp2_R);

      string strMat_QKFLPp2 = cQKFLPp2.getString("Material");
      G4Material* mat_QKFLPp2 = Materials::get(strMat_QKFLPp2);
      G4LogicalVolume* logi_QKFLPp2 = new G4LogicalVolume(geo_QKFLPp2, mat_QKFLPp2, "logi_QKFLPp2_name");

      //put volume
      setColor(*logi_QKFLPp2, cQKFLPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QKFLPp2, false);
      new G4PVPlacement(transform_QKFLPp2, logi_QKFLPp2, "phys_QKFLPp2_name", &topVolume, false, 0);

      //--------------
      //-   QLC3LPp1

      //get parameters from .xml file
      GearDir cQLC3LPp1(content, "QLC3LPp1/");

      double QLC3LPp1_Z[N];
      double QLC3LPp1_R[N];
      double QLC3LPp1_r[N];
      QLC3LPp1_Z[0] = 0;
      QLC3LPp1_Z[1] = cQLC3LPp1.getLength("L") * unitFactor;
      QLC3LPp1_R[0] = cQLC3LPp1.getLength("R") * unitFactor;
      QLC3LPp1_R[1] = cQLC3LPp1.getLength("R") * unitFactor;
      QLC3LPp1_r[0] = cQLC3LPp1.getLength("r") * unitFactor;
      QLC3LPp1_r[1] = cQLC3LPp1.getLength("r") * unitFactor;

      double QLC3LPp1_X0 = cQLC3LPp1.getLength("X0") * unitFactor;
      double QLC3LPp1_Z0 = cQLC3LPp1.getLength("Z0") * unitFactor;
      double QLC3LPp1_PHI = cQLC3LPp1.getLength("PHI");
      G4Transform3D transform_QLC3LPp1 = G4Translate3D(QLC3LPp1_X0, 0.0, QLC3LPp1_Z0);
      transform_QLC3LPp1 = transform_QLC3LPp1 * G4RotateY3D(QLC3LPp1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3LPp1 = new G4Polycone("geo_QLC3LPp1xx_name", 0.0, 2 * M_PI, N, QLC3LPp1_Z, QLC3LPp1_r, QLC3LPp1_R);

      string strMat_QLC3LPp1 = cQLC3LPp1.getString("Material");
      G4Material* mat_QLC3LPp1 = Materials::get(strMat_QLC3LPp1);
      G4LogicalVolume* logi_QLC3LPp1 = new G4LogicalVolume(geo_QLC3LPp1, mat_QLC3LPp1, "logi_QLC3LPp1_name");

      //put volume
      setColor(*logi_QLC3LPp1, cQLC3LPp1.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3LPp1, false);
      new G4PVPlacement(transform_QLC3LPp1, logi_QLC3LPp1, "phys_QLC3LPp1_name", &topVolume, false, 0);

      //--------------
      //-   QLC3LPp2

      //get parameters from .xml file
      GearDir cQLC3LPp2(content, "QLC3LPp2/");

      double QLC3LPp2_Z[N];
      double QLC3LPp2_R[N];
      double QLC3LPp2_r[N];
      QLC3LPp2_Z[0] = 0;
      QLC3LPp2_Z[1] = cQLC3LPp2.getLength("L") * unitFactor;
      QLC3LPp2_R[0] = cQLC3LPp2.getLength("R") * unitFactor;
      QLC3LPp2_R[1] = cQLC3LPp2.getLength("R") * unitFactor;
      QLC3LPp2_r[0] = cQLC3LPp2.getLength("r") * unitFactor;
      QLC3LPp2_r[1] = cQLC3LPp2.getLength("r") * unitFactor;

      double QLC3LPp2_X0 = cQLC3LPp2.getLength("X0") * unitFactor;
      double QLC3LPp2_Z0 = cQLC3LPp2.getLength("Z0") * unitFactor;
      double QLC3LPp2_PHI = cQLC3LPp2.getLength("PHI");
      G4Transform3D transform_QLC3LPp2 = G4Translate3D(QLC3LPp2_X0, 0.0, QLC3LPp2_Z0);
      transform_QLC3LPp2 = transform_QLC3LPp2 * G4RotateY3D(QLC3LPp2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_QLC3LPp2 = new G4Polycone("geo_QLC3LPp2xx_name", 0.0, 2 * M_PI, N, QLC3LPp2_Z, QLC3LPp2_r, QLC3LPp2_R);

      string strMat_QLC3LPp2 = cQLC3LPp2.getString("Material");
      G4Material* mat_QLC3LPp2 = Materials::get(strMat_QLC3LPp2);
      G4LogicalVolume* logi_QLC3LPp2 = new G4LogicalVolume(geo_QLC3LPp2, mat_QLC3LPp2, "logi_QLC3LPp2_name");

      //put volume
      setColor(*logi_QLC3LPp2, cQLC3LPp2.getString("Color", "#0000CC"));
      //setVisibility(*logi_QLC3LPp2, false);
      new G4PVPlacement(transform_QLC3LPp2, logi_QLC3LPp2, "phys_QLC3LPp2_name", &topVolume, false, 0);

      //--------------
      //-   SLYTLP1p1

      //get parameters from .xml file
      GearDir cSLYTLP1p1(content, "SLYTLP1p1/");

      double SLYTLP1p1_Z[N];
      double SLYTLP1p1_R[N];
      double SLYTLP1p1_r[N];
      SLYTLP1p1_Z[0] = 0;
      SLYTLP1p1_Z[1] = cSLYTLP1p1.getLength("L") * unitFactor;
      SLYTLP1p1_R[0] = cSLYTLP1p1.getLength("R") * unitFactor;
      SLYTLP1p1_R[1] = cSLYTLP1p1.getLength("R") * unitFactor;
      SLYTLP1p1_r[0] = cSLYTLP1p1.getLength("r") * unitFactor;
      SLYTLP1p1_r[1] = cSLYTLP1p1.getLength("r") * unitFactor;

      double SLYTLP1p1_X0 = cSLYTLP1p1.getLength("X0") * unitFactor;
      double SLYTLP1p1_Z0 = cSLYTLP1p1.getLength("Z0") * unitFactor;
      double SLYTLP1p1_PHI = cSLYTLP1p1.getLength("PHI");
      G4Transform3D transform_SLYTLP1p1 = G4Translate3D(SLYTLP1p1_X0, 0.0, SLYTLP1p1_Z0);
      transform_SLYTLP1p1 = transform_SLYTLP1p1 * G4RotateY3D(SLYTLP1p1_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTLP1p1 = new G4Polycone("geo_SLYTLP1p1xx_name", 0.0, 2 * M_PI, N, SLYTLP1p1_Z, SLYTLP1p1_r, SLYTLP1p1_R);

      string strMat_SLYTLP1p1 = cSLYTLP1p1.getString("Material");
      G4Material* mat_SLYTLP1p1 = Materials::get(strMat_SLYTLP1p1);
      G4LogicalVolume* logi_SLYTLP1p1 = new G4LogicalVolume(geo_SLYTLP1p1, mat_SLYTLP1p1, "logi_SLYTLP1p1_name");

      //put volume
      setColor(*logi_SLYTLP1p1, cSLYTLP1p1.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTLP1p1, false);
      new G4PVPlacement(transform_SLYTLP1p1, logi_SLYTLP1p1, "phys_SLYTLP1p1_name", &topVolume, false, 0);

      //--------------
      //-   SLYTLP1p2

      //get parameters from .xml file
      GearDir cSLYTLP1p2(content, "SLYTLP1p2/");

      double SLYTLP1p2_Z[N];
      double SLYTLP1p2_R[N];
      double SLYTLP1p2_r[N];
      SLYTLP1p2_Z[0] = 0;
      SLYTLP1p2_Z[1] = cSLYTLP1p2.getLength("L") * unitFactor;
      SLYTLP1p2_R[0] = cSLYTLP1p2.getLength("R") * unitFactor;
      SLYTLP1p2_R[1] = cSLYTLP1p2.getLength("R") * unitFactor;
      SLYTLP1p2_r[0] = cSLYTLP1p2.getLength("r") * unitFactor;
      SLYTLP1p2_r[1] = cSLYTLP1p2.getLength("r") * unitFactor;

      double SLYTLP1p2_X0 = cSLYTLP1p2.getLength("X0") * unitFactor;
      double SLYTLP1p2_Z0 = cSLYTLP1p2.getLength("Z0") * unitFactor;
      double SLYTLP1p2_PHI = cSLYTLP1p2.getLength("PHI");
      G4Transform3D transform_SLYTLP1p2 = G4Translate3D(SLYTLP1p2_X0, 0.0, SLYTLP1p2_Z0);
      transform_SLYTLP1p2 = transform_SLYTLP1p2 * G4RotateY3D(SLYTLP1p2_PHI / Unit::rad);

      //define geometry
      G4Polycone* geo_SLYTLP1p2 = new G4Polycone("geo_SLYTLP1p2xx_name", 0.0, 2 * M_PI, N, SLYTLP1p2_Z, SLYTLP1p2_r, SLYTLP1p2_R);

      string strMat_SLYTLP1p2 = cSLYTLP1p2.getString("Material");
      G4Material* mat_SLYTLP1p2 = Materials::get(strMat_SLYTLP1p2);
      G4LogicalVolume* logi_SLYTLP1p2 = new G4LogicalVolume(geo_SLYTLP1p2, mat_SLYTLP1p2, "logi_SLYTLP1p2_name");

      //put volume
      setColor(*logi_SLYTLP1p2, cSLYTLP1p2.getString("Color", "#0000CC"));
      //setVisibility(*logi_SLYTLP1p2, false);
      new G4PVPlacement(transform_SLYTLP1p2, logi_SLYTLP1p2, "phys_SLYTLP1p2_name", &topVolume, false, 0);

      //--------------
      //-   concrete wall

      //--------------
      //-   GateShield (gate shield)

      //get parameters from .xml file
      GearDir cGateShield(content, "GateShield/");

      double GateShield_X = cGateShield.getLength("X") * unitFactor;
      double GateShield_Y = cGateShield.getLength("Y") * unitFactor;
      double GateShield_Z = cGateShield.getLength("Z") * unitFactor;
      double TUN_X = cGateShield.getLength("TUNX") * unitFactor;
      double TUN_Y = cGateShield.getLength("TUNY") * unitFactor;
      double DET_Z = cGateShield.getLength("DETZ") * unitFactor;
      double DET_DZ = cGateShield.getLength("DETDZ") * unitFactor;
      double ROT = cGateShield.getAngle("ROT");

      G4Transform3D transform_DET = G4Translate3D(0.0, 0.0, DET_DZ);
      G4Transform3D transform_ROT = G4Translate3D(0.0, 0.0, 0.0);
      transform_ROT = transform_ROT * G4RotateY3D(ROT / Unit::rad);

      //define geometry
      // wall is made from the box by excluding spaces for detector and tunnel
      G4Box* geo_GateShieldxx = new G4Box("geo_GateShieldxx_name", GateShield_X, GateShield_Y, GateShield_Z);
      G4Box* geo_TUN = new G4Box("geo_TUN_name", TUN_X, TUN_Y, GateShield_Z);
      G4SubtractionSolid* geo_GateShieldx = new G4SubtractionSolid("geo_GateShieldx_name", geo_GateShieldxx, geo_TUN);
      G4Box* geo_DET = new G4Box("geo_DET_name", GateShield_X, GateShield_Y, DET_Z);
      G4SubtractionSolid* geo_GateShield = new G4SubtractionSolid("geo_GateShield_name", geo_GateShieldx, geo_DET, transform_DET);

      string strMat_GateShield = cGateShield.getString("Material");
      G4Material* mat_GateShield = Materials::get(strMat_GateShield);
      G4LogicalVolume* logi_GateShield = new G4LogicalVolume(geo_GateShield, mat_GateShield, "logi_GateShield_name");

      //put volume
      setColor(*logi_GateShield, cGateShield.getString("Color", "#CC0000"));
      //setVisibility(*logi_GateShield, false);
      new G4PVPlacement(transform_ROT, logi_GateShield, "phys_GateShield_name", &topVolume, false, 0);


      bool radiation_study = false;

      //--------------
      //-   Tube (virtual tube for radiation level study)

      //define geometry
      G4Tubs* geo_Tube = new G4Tubs("geo_Tube_name", 3995 * mm, 4000 * mm, 29 * m, 0. * deg, 360.*deg);
      G4Material* mat_Tube = Materials::get("G4_Si");
      G4LogicalVolume* logi_Tube = new G4LogicalVolume(geo_Tube, mat_Tube, "logi_Tube_name");

      //put volume
      setColor(*logi_Tube, cGateShield.getString("Color", "#CC0000"));
      //setVisibility(*logi_Tube, false);
      if (radiation_study) {
        new G4PVPlacement(transform_ROT, logi_Tube, "phys_Tube_name", &topVolume, false, 0);
      }


      //--------------
      //-   polyethyren shields

      //--------------
      //-   PolyShieldR

      //get parameters from .xml file
      GearDir cPolyShieldR(content, "PolyShieldR/");

      double PolyShieldR_Xp = cPolyShieldR.getLength("Xp") * unitFactor;
      double PolyShieldR_Xm = cPolyShieldR.getLength("Xm") * unitFactor;
      double PolyShieldR_Y = cPolyShieldR.getLength("Y") * unitFactor;
      double PolyShieldR_Z = cPolyShieldR.getLength("Z") * unitFactor;
      double PolyShieldR_DZ = cPolyShieldR.getLength("DZ") * unitFactor;
      double PolyShieldR_r = cPolyShieldR.getLength("r") * unitFactor;
      double PolyShieldR_dx = cPolyShieldR.getLength("dx") * unitFactor;

      double PolyShieldR_X = (PolyShieldR_Xp + PolyShieldR_Xm) / 2;
      G4Transform3D transform_PolyShieldR = G4Translate3D((PolyShieldR_Xp - PolyShieldR_Xm) / 2, 0.0, PolyShieldR_DZ);
      G4Transform3D transform_PolyShieldR_Hole = G4Translate3D(PolyShieldR_dx, 0.0, 0.0);

      //define geometry
      G4Box* geo_PolyShieldRx = new G4Box("geo_PolyShieldRx_name", PolyShieldR_X, PolyShieldR_Y, PolyShieldR_Z);
      G4Tubs* geo_PolyShieldR_Hole = new G4Tubs("geo_PolyShieldRxx_name", 0 * mm, PolyShieldR_r, PolyShieldR_Z, 0. * deg, 360.*deg);
      G4SubtractionSolid* geo_PolyShieldR
        = new G4SubtractionSolid("geo_PolyShieldR_name", geo_PolyShieldRx, geo_PolyShieldR_Hole, transform_PolyShieldR_Hole);

      string strMat_PolyShieldR = cPolyShieldR.getString("Material");
      G4Material* mat_PolyShieldR = Materials::get(strMat_PolyShieldR);
      G4LogicalVolume* logi_PolyShieldR = new G4LogicalVolume(geo_PolyShieldR, mat_PolyShieldR, "logi_PolyShieldR_name");

      //put volume
      setColor(*logi_PolyShieldR, cPolyShieldR.getString("Color", "#0000CC"));
      //setVisibility(*logi_PolyShieldL, false);
      new G4PVPlacement(transform_PolyShieldR, logi_PolyShieldR, "phys_PolyShieldR_name", &topVolume, false, 0);

      //--------------
      //-   PolyShieldL

      //get parameters from .xml file
      GearDir cPolyShieldL(content, "PolyShieldL/");

      double PolyShieldL_Xp = cPolyShieldL.getLength("Xp") * unitFactor;
      double PolyShieldL_Xm = cPolyShieldL.getLength("Xm") * unitFactor;
      double PolyShieldL_Y = cPolyShieldL.getLength("Y") * unitFactor;
      double PolyShieldL_Z = cPolyShieldL.getLength("Z") * unitFactor;
      double PolyShieldL_DZ = cPolyShieldL.getLength("DZ") * unitFactor;
      double PolyShieldL_r = cPolyShieldL.getLength("r") * unitFactor;
      double PolyShieldL_dx = cPolyShieldL.getLength("dx") * unitFactor;

      double PolyShieldL_X = (PolyShieldL_Xp + PolyShieldL_Xm) / 2;
      G4Transform3D transform_PolyShieldL = G4Translate3D((PolyShieldL_Xp - PolyShieldL_Xm) / 2, 0.0, PolyShieldL_DZ);
      G4Transform3D transform_PolyShieldL_Hole = G4Translate3D(PolyShieldL_dx, 0.0, 0.0);

      //define geometry
      G4Box* geo_PolyShieldLx = new G4Box("geo_PolyShieldLx_name", PolyShieldL_X, PolyShieldL_Y, PolyShieldL_Z);
      G4Tubs* geo_PolyShieldL_Hole = new G4Tubs("geo_PolyShieldLxx_name", 0 * mm, PolyShieldL_r, PolyShieldL_Z, 0. * deg, 360.*deg);
      G4SubtractionSolid* geo_PolyShieldL
        = new G4SubtractionSolid("geo_PolyShieldL_name", geo_PolyShieldLx, geo_PolyShieldL_Hole, transform_PolyShieldL_Hole);

      string strMat_PolyShieldL = cPolyShieldL.getString("Material");
      G4Material* mat_PolyShieldL = Materials::get(strMat_PolyShieldL);
      G4LogicalVolume* logi_PolyShieldL = new G4LogicalVolume(geo_PolyShieldL, mat_PolyShieldL, "logi_PolyShieldL_name");

      //put volume
      setColor(*logi_PolyShieldL, cPolyShieldL.getString("Color", "#0000CC"));
      //setVisibility(*logi_PolyShieldL, false);
      new G4PVPlacement(transform_PolyShieldL, logi_PolyShieldL, "phys_PolyShieldL_name", &topVolume, false, 0);

      //--------------
      //-   concrete tunnel-end shields

      //--------------
      //-   ConcreteShieldR

      //get parameters from .xml file
      GearDir cConcreteShieldR(content, "ConcreteShieldR/");

      double ConcreteShieldR_X = cConcreteShieldR.getLength("X") * unitFactor;
      double ConcreteShieldR_Y = cConcreteShieldR.getLength("Y") * unitFactor;
      double ConcreteShieldR_Z = cConcreteShieldR.getLength("Z") * unitFactor;
      double ConcreteShieldR_DZ = cConcreteShieldR.getLength("DZ") * unitFactor;
      double ConcreteShieldR_x = cConcreteShieldR.getLength("x") * unitFactor;
      double ConcreteShieldR_y = cConcreteShieldR.getLength("y") * unitFactor;
      double ConcreteShieldR_dx = cConcreteShieldR.getLength("dx") * unitFactor;
      double ConcreteShieldR_dy = cConcreteShieldR.getLength("dy") * unitFactor;

      G4Transform3D transform_ConcreteShieldR = G4Translate3D(0.0, 0.0, ConcreteShieldR_DZ);
      transform_ConcreteShieldR = transform_ROT * transform_ConcreteShieldR;
      G4Transform3D transform_ConcreteShieldR_Hole = G4Translate3D(ConcreteShieldR_dx, ConcreteShieldR_dy, 0.0);


      //define geometry
      G4Box* geo_ConcreteShieldRx = new G4Box("geo_ConcreteShieldRx_name", ConcreteShieldR_X, ConcreteShieldR_Y, ConcreteShieldR_Z);
      G4Box* geo_ConcreteShieldR_Hole = new G4Box("geo_ConcreteShieldRxx_name", ConcreteShieldR_x, ConcreteShieldR_y, ConcreteShieldR_Z);
      G4SubtractionSolid* geo_ConcreteShieldR = new G4SubtractionSolid("geo_ConcreteShieldR_name", geo_ConcreteShieldRx, geo_ConcreteShieldR_Hole, transform_ConcreteShieldR_Hole);

      string strMat_ConcreteShieldR = cConcreteShieldR.getString("Material");
      G4Material* mat_ConcreteShieldR = Materials::get(strMat_ConcreteShieldR);
      G4LogicalVolume* logi_ConcreteShieldR = new G4LogicalVolume(geo_ConcreteShieldR, mat_ConcreteShieldR, "logi_ConcreteShieldR_name");

      //put volume
      setColor(*logi_ConcreteShieldR, cConcreteShieldR.getString("Color", "#0000CC"));
      //setVisibility(*logi_ConcreteShieldR, false);
      new G4PVPlacement(transform_ConcreteShieldR, logi_ConcreteShieldR, "phys_ConcreteShieldR_name", &topVolume, false, 0);

      //--------------
      //-   ConcreteShieldL

      //get parameters from .xml file
      GearDir cConcreteShieldL(content, "ConcreteShieldL/");

      double ConcreteShieldL_X = cConcreteShieldL.getLength("X") * unitFactor;
      double ConcreteShieldL_Y = cConcreteShieldL.getLength("Y") * unitFactor;
      double ConcreteShieldL_Z = cConcreteShieldL.getLength("Z") * unitFactor;
      double ConcreteShieldL_DZ = cConcreteShieldL.getLength("DZ") * unitFactor;
      double ConcreteShieldL_x = cConcreteShieldL.getLength("x") * unitFactor;
      double ConcreteShieldL_y = cConcreteShieldL.getLength("y") * unitFactor;
      double ConcreteShieldL_dx = cConcreteShieldL.getLength("dx") * unitFactor;
      double ConcreteShieldL_dy = cConcreteShieldL.getLength("dy") * unitFactor;

      G4Transform3D transform_ConcreteShieldL = G4Translate3D(0.0, 0.0, ConcreteShieldL_DZ);
      transform_ConcreteShieldL = transform_ROT * transform_ConcreteShieldL;
      G4Transform3D transform_ConcreteShieldL_Hole = G4Translate3D(ConcreteShieldL_dx, ConcreteShieldL_dy, 0.0);

      //define geometry
      G4Box* geo_ConcreteShieldLx = new G4Box("geo_ConcreteShieldLx_name", ConcreteShieldL_X, ConcreteShieldL_Y, ConcreteShieldL_Z);
      G4Box* geo_ConcreteShieldL_Hole = new G4Box("geo_ConcreteShieldLxx_name", ConcreteShieldL_x, ConcreteShieldL_y, ConcreteShieldL_Z);
      G4SubtractionSolid* geo_ConcreteShieldL = new G4SubtractionSolid("geo_ConcreteShieldL_name", geo_ConcreteShieldLx, geo_ConcreteShieldL_Hole, transform_ConcreteShieldL_Hole);

      string strMat_ConcreteShieldL = cConcreteShieldL.getString("Material");
      G4Material* mat_ConcreteShieldL = Materials::get(strMat_ConcreteShieldL);
      G4LogicalVolume* logi_ConcreteShieldL = new G4LogicalVolume(geo_ConcreteShieldL, mat_ConcreteShieldL, "logi_ConcreteShieldL_name");

      //put volume
      setColor(*logi_ConcreteShieldL, cConcreteShieldL.getString("Color", "#0000CC"));
      //setVisibility(*logi_ConcreteShieldL, false);
      new G4PVPlacement(transform_ConcreteShieldL, logi_ConcreteShieldL, "phys_ConcreteShieldL_name", &topVolume, false, 0);


      //---------------------------
      // for dose simulation
      //---------------------------

      //neutron shield (poly)
      logi_PolyShieldL->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1));
      logi_PolyShieldR->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2));

      //additional neutron shield (concrete)
      logi_ConcreteShieldL->SetSensitiveDetector(new BkgSensitiveDetector("IR", 3));
      logi_ConcreteShieldR->SetSensitiveDetector(new BkgSensitiveDetector("IR", 4));

      //gate shield (concrete)
      //logi_GateShield->SetSensitiveDetector(new BkgSensitiveDetector("IR", 5));

      //virtual material outsire gate-shield
      if (radiation_study) {
        logi_Tube->SetSensitiveDetector(new BkgSensitiveDetector("IR", 6));
      }

    }
  }
}

