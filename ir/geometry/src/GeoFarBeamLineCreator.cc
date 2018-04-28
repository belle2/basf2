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
#include <framework/gearbox/Gearbox.h>
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

#include "CLHEP/Units/PhysicalConstants.h"

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

    void GeoFarBeamLineCreator::createGeometry(G4LogicalVolume& topVolume, GeometryTypes)
    {

      const int N = 2;

      double stepMax = 5.0 * Unit::mm;
      int flag_limitStep = int(m_config.getParameter("LimitStepLength"));


      //double unitFactor = 10.0;
      const double unitFactor = Unit::cm / Unit::mm;

      map<string, FarBeamLineElement> elements;


      //--------------
      //-   limits

      //--------------
      //-   TubeR

      FarBeamLineElement tubeR;

      //get parameters from .xml file
      std::string prep = "TubeR.";

      int TubeR_N = int(m_config.getParameter(prep + "N"));

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

      tubeR.transform = G4Translate3D(0.0, 0.0, 0.0);

      //define geometry
      tubeR.geo = new G4Polycone("geo_TubeR_name", 0.0, 2 * M_PI, TubeR_N, &(TubeR_Z[0]), &(TubeR_r[0]), &(TubeR_R[0]));

      elements["TubeR"] = tubeR;

      //--------------
      //-   TubeL

      FarBeamLineElement tubeL;

      //get parameters from .xml file
      prep = "TubeL.";

      int TubeL_N = int(m_config.getParameter(prep + "N"));

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
        TubeL_r[i] = m_config.getParameter(prep + ossrID.str(), 0.0) * unitFactor;
      }

      tubeL.transform = G4Translate3D(0.0, 0.0, 0.0);

      //define geometry
      tubeL.geo = new G4Polycone("geo_TubeL_name", 0.0, 2 * M_PI, TubeL_N, &(TubeL_Z[0]), &(TubeL_r[0]), &(TubeL_R[0]));

      elements["TubeL"] = tubeL;

      std::vector<double> zero_r(N, 0.);

      for (std::pair<std::string, std::string> element : m_config.getParametersStr()) {

        if (element.first != "Straight") continue;

        //--------------
        //-   Create straight element

        string name = element.second;
        prep = name + ".";
        string type = m_config.getParameterStr(prep + "type");


        FarBeamLineElement polycone;

        std::vector<double> Polycone_Z(N);
        std::vector<double> Polycone_R(N);
        std::vector<double> Polycone_r(N);
        Polycone_Z[0] = 0;
        Polycone_Z[1] = m_config.getParameter(prep + "L") * unitFactor;
        Polycone_R[0] = m_config.getParameter(prep + "R") * unitFactor;
        Polycone_R[1] = m_config.getParameter(prep + "R") * unitFactor;
        Polycone_r[0] = m_config.getParameter(prep + "r") * unitFactor;
        Polycone_r[1] = m_config.getParameter(prep + "r") * unitFactor;

        double Polycone_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double Polycone_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double Polycone_PHI = m_config.getParameter(prep + "PHI");

        polycone.transform = G4Translate3D(Polycone_X0, 0.0, Polycone_Z0);
        polycone.transform = polycone.transform * G4RotateY3D(Polycone_PHI / Unit::rad);

        //define geometry
        string subtract = m_config.getParameterStr(prep + "Subtract", "");
        string intersect = m_config.getParameterStr(prep + "Intersect", "");

        string geo_polyconexx_name = "geo_" + name + "xx_name";
        string geo_polyconex_name = "geo_" + name + "x_name";
        string geo_polycone_name = "geo_" + name + "_name";

        G4VSolid* geo_polyconexx(NULL), *geo_polyconex(NULL), *geo_polycone(NULL);

        if (subtract != "" || intersect != "")
          if (type == "pipe") // for pipes inner space will be created as vacuum
            geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, &(Polycone_Z[0]), &(zero_r[0]), &(Polycone_R[0]));
          else
            geo_polyconexx = new G4Polycone(geo_polyconexx_name, 0.0, 2 * M_PI, N, &(Polycone_Z[0]), &(Polycone_r[0]), &(Polycone_R[0]));
        else if (type == "pipe") // for pipes inner space will be created as vacuum
          geo_polycone = new G4Polycone(geo_polycone_name, 0.0, 2 * M_PI, N, &(Polycone_Z[0]), &(zero_r[0]), &(Polycone_R[0]));
        else
          geo_polycone = new G4Polycone(geo_polycone_name, 0.0, 2 * M_PI, N, &(Polycone_Z[0]), &(Polycone_r[0]), &(Polycone_R[0]));


        if (subtract != "" && intersect != "") {
          geo_polyconex = new G4SubtractionSolid(geo_polyconex_name, geo_polyconexx, elements[subtract].geo,
                                                 polycone.transform.inverse()*elements[subtract].transform);
          geo_polycone = new G4IntersectionSolid(geo_polycone_name, geo_polyconex, elements[intersect].geo,
                                                 polycone.transform.inverse()*elements[intersect].transform);
        } else if (subtract != "")
          geo_polycone = new G4SubtractionSolid(geo_polycone_name, geo_polyconexx, elements[subtract].geo,
                                                polycone.transform.inverse()*elements[subtract].transform);
        else if (intersect != "")
          geo_polycone = new G4IntersectionSolid(geo_polycone_name, geo_polyconexx, elements[intersect].geo,
                                                 polycone.transform.inverse()*elements[intersect].transform);

        polycone.geo = geo_polycone;

        // define logical volume
        string strMat_polycone = m_config.getParameterStr(prep + "Material");
        G4Material* mat_polycone = Materials::get(strMat_polycone);
        string logi_polycone_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_polycone = new G4LogicalVolume(polycone.geo, mat_polycone, logi_polycone_name);
        setColor(*logi_polycone, "#CC0000");
        setVisibility(*logi_polycone, false);

        //put volume
        string phys_polycone_name = "phys_" + name + "_name";
        new G4PVPlacement(polycone.transform, logi_polycone, phys_polycone_name, &topVolume, false, 0);

        elements[name] = polycone;

        double sum = 0.0;
        for (int i = 0; i < N; ++i)
          sum += Polycone_r[i]; // check that there is a space inside a pipe
        if (type == "pipe" && sum != 0) { // add vacuum inside a pipe

          FarBeamLineElement vacuum;

          string nameVac = name + "Vac";

          //define geometry
          string geo_vacuumxx_name = "geo_" + nameVac + "xx_name";
          string geo_vacuum_name = "geo_" + nameVac + "_name";

          G4VSolid* geo_vacuumxx, *geo_vacuum;

          geo_vacuumxx = new G4Polycone(geo_vacuumxx_name, 0.0, 2 * M_PI, N, &(Polycone_Z[0]), &(zero_r[0]), &(Polycone_r[0]));
          geo_vacuum = new G4IntersectionSolid(geo_vacuumxx_name, geo_vacuumxx, geo_polycone);

          vacuum.geo = geo_vacuum;
          vacuum.transform = polycone.transform;

          // define logical volume
          G4Material* mat_vacuum = Materials::get("Vacuum");
          string logi_vacuum_name = "logi_" + nameVac + "_name";
          G4LogicalVolume* logi_vacuum = new G4LogicalVolume(vacuum.geo, mat_vacuum, logi_vacuum_name);
          if (flag_limitStep) logi_vacuum->SetUserLimits(new G4UserLimits(stepMax));
          setColor(*logi_vacuum, "#000000");
          setVisibility(*logi_vacuum, false);

          //put volume
          string phys_vacuum_name = "phys_" + nameVac + "_name";
          //new G4PVPlacement(vacuum.transform, logi_vacuum, phys_vacuum_name, &topVolume, false, 0);
          new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_vacuum, phys_vacuum_name, logi_polycone, false, 0);

          elements[nameVac] = vacuum;
        }
      }


      for (std::pair<std::string, std::string> element : m_config.getParametersStr()) {

        if (element.first != "Bending") continue;

        //--------------
        //-   Create torus element

        string name = element.second;
        prep = name + ".";
        string type = m_config.getParameterStr(prep + "type");

        FarBeamLineElement torus;

        double torus_r = m_config.getParameter(prep + "r") * unitFactor;
        double torus_R = m_config.getParameter(prep + "R") * unitFactor;
        double torus_RT = m_config.getParameter(prep + "RT") * unitFactor;
        double torus_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double torus_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double torus_SPHI = m_config.getParameter(prep + "SPHI");
        double torus_DPHI = m_config.getParameter(prep + "DPHI");

        torus.transform = G4Translate3D(torus_X0, 0.0, torus_Z0);
        torus.transform = torus.transform * G4RotateX3D(M_PI / 2 / Unit::rad);

        //define geometry
        string subtract = m_config.getParameterStr(prep + "Subtract", "");
        string intersect = m_config.getParameterStr(prep + "Intersect", "");

        string geo_torusxx_name = "geo_" + name + "xx_name";
        string geo_torusx_name = "geo_" + name + "x_name";
        string geo_torus_name = "geo_" + name + "_name";

        G4VSolid* geo_torusxx(NULL), *geo_torusx(NULL), *geo_torus(NULL);

        if (subtract != "" || intersect != "")
          if (type == "pipe") // for pipes inner space will be created as vacuum
            geo_torusxx = new G4Torus(geo_torusxx_name, 0, torus_R, torus_RT, torus_SPHI, torus_DPHI);
          else
            geo_torusxx = new G4Torus(geo_torusxx_name, torus_r, torus_R, torus_RT, torus_SPHI, torus_DPHI);
        else if (type == "pipe") // for pipes inner space will be created as vacuum
          geo_torus = new G4Torus(geo_torus_name, 0, torus_R, torus_RT, torus_SPHI, torus_DPHI);
        else
          geo_torus = new G4Torus(geo_torus_name, torus_r, torus_R, torus_RT, torus_SPHI, torus_DPHI);

        if (subtract != "" && intersect != "") {
          geo_torusx = new G4SubtractionSolid(geo_torusx_name, geo_torusxx, elements[subtract].geo,
                                              torus.transform.inverse()*elements[subtract].transform);
          geo_torus = new G4IntersectionSolid(geo_torus_name, geo_torusx, elements[intersect].geo,
                                              torus.transform.inverse()*elements[intersect].transform);
        } else if (subtract != "")
          geo_torus  = new G4SubtractionSolid(geo_torus_name, geo_torusxx, elements[subtract].geo,
                                              torus.transform.inverse()*elements[subtract].transform);
        else if (intersect != "")
          geo_torus = new G4IntersectionSolid(geo_torus_name, geo_torusxx, elements[intersect].geo,
                                              torus.transform.inverse()*elements[intersect].transform);

        torus.geo = geo_torus;

        // define logical volume
        string strMat_torus = m_config.getParameterStr(prep + "Material");
        G4Material* mat_torus = Materials::get(strMat_torus);
        string logi_torus_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_torus = new G4LogicalVolume(torus.geo, mat_torus, logi_torus_name);
        setColor(*logi_torus, "#CC0000");
        setVisibility(*logi_torus, false);

        //put volume
        string phys_torus_name = "phys_" + name + "_name";
        new G4PVPlacement(torus.transform, logi_torus, phys_torus_name, &topVolume, false, 0);

        elements[name] = torus;

        if (type == "pipe" && torus_r != 0) { // add vacuum inside a pipe

          FarBeamLineElement vacuum;

          string nameVac = name + "Vac";

          //define geometry
          string geo_vacuumxx_name = "geo_" + nameVac + "xx_name";
          string geo_vacuum_name = "geo_" + nameVac + "_name";

          G4VSolid* geo_vacuumxx, *geo_vacuum;

          geo_vacuumxx = new G4Torus(geo_vacuumxx_name, 0.0, torus_r, torus_RT, torus_SPHI, torus_DPHI);
          geo_vacuum = new G4IntersectionSolid(geo_vacuum_name, geo_vacuumxx, geo_torus);

          vacuum.geo = geo_vacuum;
          vacuum.transform = torus.transform;

          // define logical volume
          G4Material* mat_vacuum = Materials::get("Vacuum");
          string logi_vacuum_name = "logi_" + nameVac + "_name";
          G4LogicalVolume* logi_vacuum = new G4LogicalVolume(vacuum.geo, mat_vacuum, logi_vacuum_name);
          if (flag_limitStep) logi_vacuum->SetUserLimits(new G4UserLimits(stepMax));
          setColor(*logi_vacuum, "#000000");
          setVisibility(*logi_vacuum, false);

          //put volume
          string phys_vacuum_name = "phys_" + nameVac + "_name";
          //new G4PVPlacement(vacuum.transform, logi_vacuum, phys_vacuum_name, &topVolume, false, 0);
          new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_vacuum, phys_vacuum_name, logi_torus, false, 0);

          elements[nameVac] = vacuum;
        }
      }


      //--------------
      //-   concrete wall

      //--------------
      //-   GateShield (gate shield)

      //get parameters from .xml file
      prep = "GateShield.";

      double GateShield_X = m_config.getParameter(prep + "X") * unitFactor;
      double GateShield_Y = m_config.getParameter(prep + "Y") * unitFactor;
      double GateShield_Z = m_config.getParameter(prep + "Z") * unitFactor;
      double TUN_X = m_config.getParameter(prep + "TUNX") * unitFactor;
      double TUN_Y = m_config.getParameter(prep + "TUNY") * unitFactor;
      double DET_Z = m_config.getParameter(prep + "DETZ") * unitFactor;
      double DET_DZ = m_config.getParameter(prep + "DETDZ") * unitFactor;
      double ROT = m_config.getParameter(prep + "ROT");

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

      string strMat_GateShield = m_config.getParameterStr(prep + "Material");
      G4Material* mat_GateShield = Materials::get(strMat_GateShield);
      G4LogicalVolume* logi_GateShield = new G4LogicalVolume(geo_GateShield, mat_GateShield, "logi_GateShield_name");

      //put volume
      setColor(*logi_GateShield, "#CC0000");
      //setVisibility(*logi_GateShield, false);
      new G4PVPlacement(transform_ROT, logi_GateShield, "phys_GateShield_name", &topVolume, false, 0);


      bool radiation_study = false;

      //--------------
      //-   Tube (virtual tube for radiation level study)

      //define geometry
      G4Tubs* geo_Tube = new G4Tubs("geo_Tube_name", 3995 * CLHEP::mm, 4000 * CLHEP::mm, 29 * CLHEP::m, 0. * CLHEP::deg, 360.*CLHEP::deg);
      G4Material* mat_Tube = Materials::get("G4_Si");
      G4LogicalVolume* logi_Tube = new G4LogicalVolume(geo_Tube, mat_Tube, "logi_Tube_name");

      //put volume
      setColor(*logi_Tube, "#CC0000");
      //setVisibility(*logi_Tube, false);
      if (radiation_study) {
        new G4PVPlacement(transform_ROT, logi_Tube, "phys_Tube_name", &topVolume, false, 0);
      }


      //--------------
      //-   polyethylene shields

      //--------------
      //-   PolyShieldR

      //get parameters from .xml file
      prep = "PolyShieldR.";

      double PolyShieldR_Xp = m_config.getParameter(prep + "Xp") * unitFactor;
      double PolyShieldR_Xm = m_config.getParameter(prep + "Xm") * unitFactor;
      double PolyShieldR_Y = m_config.getParameter(prep + "Y") * unitFactor;
      double PolyShieldR_Z = m_config.getParameter(prep + "Z") * unitFactor;
      double PolyShieldR_DZ = m_config.getParameter(prep + "DZ") * unitFactor;
      double PolyShieldR_r = m_config.getParameter(prep + "r") * unitFactor;
      double PolyShieldR_dx = m_config.getParameter(prep + "dx") * unitFactor;

      double PolyShieldR_X = (PolyShieldR_Xp + PolyShieldR_Xm) / 2;
      G4Transform3D transform_polyShieldR = G4Translate3D((PolyShieldR_Xp - PolyShieldR_Xm) / 2, 0.0, PolyShieldR_DZ);
      G4Transform3D transform_polyShieldR_Hole = G4Translate3D(PolyShieldR_dx, 0.0, 0.0);

      //define geometry
      G4Box* geo_polyShieldRx = new G4Box("geo_polyShieldRx_name", PolyShieldR_X, PolyShieldR_Y, PolyShieldR_Z);
      G4Tubs* geo_polyShieldR_Hole = new G4Tubs("geo_polyShieldRxx_name", 0 * CLHEP::mm, PolyShieldR_r, PolyShieldR_Z, 0. * CLHEP::deg,
                                                360.*CLHEP::deg);
      G4SubtractionSolid* geo_polyShieldR
        = new G4SubtractionSolid("geo_polyShieldR_name", geo_polyShieldRx, geo_polyShieldR_Hole, transform_polyShieldR_Hole);

      string strMat_polyShieldR = m_config.getParameterStr(prep + "Material");
      G4Material* mat_polyShieldR = Materials::get(strMat_polyShieldR);
      G4LogicalVolume* logi_polyShieldR = new G4LogicalVolume(geo_polyShieldR, mat_polyShieldR, "logi_polyShieldR_name");

      //put volume
      setColor(*logi_polyShieldR, "#0000CC");
      //setVisibility(*logi_polyShieldL, false);
      new G4PVPlacement(transform_polyShieldR, logi_polyShieldR, "phys_polyShieldR_name", &topVolume, false, 0);

      //--------------
      //-   PolyShieldL

      //get parameters from .xml file
      prep = "PolyShieldL.";

      double PolyShieldL_Xp = m_config.getParameter(prep + "Xp") * unitFactor;
      double PolyShieldL_Xm = m_config.getParameter(prep + "Xm") * unitFactor;
      double PolyShieldL_Y = m_config.getParameter(prep + "Y") * unitFactor;
      double PolyShieldL_Z = m_config.getParameter(prep + "Z") * unitFactor;
      double PolyShieldL_DZ = m_config.getParameter(prep + "DZ") * unitFactor;
      double PolyShieldL_r = m_config.getParameter(prep + "r") * unitFactor;
      double PolyShieldL_dx = m_config.getParameter(prep + "dx") * unitFactor;

      double PolyShieldL_X = (PolyShieldL_Xp + PolyShieldL_Xm) / 2;
      G4Transform3D transform_polyShieldL = G4Translate3D((PolyShieldL_Xp - PolyShieldL_Xm) / 2, 0.0, PolyShieldL_DZ);
      G4Transform3D transform_polyShieldL_Hole = G4Translate3D(PolyShieldL_dx, 0.0, 0.0);

      //define geometry
      G4Box* geo_polyShieldLxx = new G4Box("geo_polyShieldLxx_name", PolyShieldL_X, PolyShieldL_Y, PolyShieldL_Z);
      G4Tubs* geo_polyShieldL_Hole = new G4Tubs("geo_polyShieldLxxx_name", 0 * CLHEP::mm, PolyShieldL_r, PolyShieldL_Z, 0. * CLHEP::deg,
                                                360.*CLHEP::deg);
      G4SubtractionSolid* geo_polyShieldLx
        = new G4SubtractionSolid("geo_polyShieldLx_name", geo_polyShieldLxx, geo_polyShieldL_Hole, transform_polyShieldL_Hole);
      G4SubtractionSolid* geo_polyShieldL
        = new G4SubtractionSolid("geo_polyShieldL_name", geo_polyShieldLx, geo_GateShield, transform_polyShieldL.inverse()*transform_ROT);

      string strMat_polyShieldL = m_config.getParameterStr(prep + "Material");
      G4Material* mat_polyShieldL = Materials::get(strMat_polyShieldL);
      G4LogicalVolume* logi_polyShieldL = new G4LogicalVolume(geo_polyShieldL, mat_polyShieldL, "logi_polyShieldL_name");

      //put volume
      setColor(*logi_polyShieldL, "#0000CC");
      //setVisibility(*logi_polyShieldL, false);
      new G4PVPlacement(transform_polyShieldL, logi_polyShieldL, "phys_polyShieldL_name", &topVolume, false, 0);

      //--------------
      //-   concrete tunnel-end shields

      //--------------
      //-   ConcreteShieldR

      //get parameters from .xml file
      prep = "ConcreteShieldR.";

      double ConcreteShieldR_X = m_config.getParameter(prep + "X") * unitFactor;
      double ConcreteShieldR_Y = m_config.getParameter(prep + "Y") * unitFactor;
      double ConcreteShieldR_Z = m_config.getParameter(prep + "Z") * unitFactor;
      double ConcreteShieldR_DZ = m_config.getParameter(prep + "DZ") * unitFactor;
      double ConcreteShieldR_x = m_config.getParameter(prep + "x") * unitFactor;
      double ConcreteShieldR_y = m_config.getParameter(prep + "y") * unitFactor;
      double ConcreteShieldR_dx = m_config.getParameter(prep + "dx") * unitFactor;
      double ConcreteShieldR_dy = m_config.getParameter(prep + "dy") * unitFactor;

      G4Transform3D transform_ConcreteShieldR = G4Translate3D(0.0, 0.0, ConcreteShieldR_DZ);
      transform_ConcreteShieldR = transform_ROT * transform_ConcreteShieldR;
      G4Transform3D transform_ConcreteShieldR_Hole = G4Translate3D(ConcreteShieldR_dx, ConcreteShieldR_dy, 0.0);


      //define geometry
      G4Box* geo_ConcreteShieldRx = new G4Box("geo_ConcreteShieldRx_name", ConcreteShieldR_X, ConcreteShieldR_Y, ConcreteShieldR_Z);
      G4Box* geo_ConcreteShieldR_Hole = new G4Box("geo_ConcreteShieldRxx_name", ConcreteShieldR_x, ConcreteShieldR_y, ConcreteShieldR_Z);
      G4SubtractionSolid* geo_ConcreteShieldR = new G4SubtractionSolid("geo_ConcreteShieldR_name", geo_ConcreteShieldRx,
          geo_ConcreteShieldR_Hole, transform_ConcreteShieldR_Hole);

      string strMat_ConcreteShieldR = m_config.getParameterStr(prep + "Material");
      G4Material* mat_ConcreteShieldR = Materials::get(strMat_ConcreteShieldR);
      G4LogicalVolume* logi_ConcreteShieldR = new G4LogicalVolume(geo_ConcreteShieldR, mat_ConcreteShieldR, "logi_ConcreteShieldR_name");

      //put volume
      setColor(*logi_ConcreteShieldR, "#0000CC");
      //setVisibility(*logi_ConcreteShieldR, false);
      new G4PVPlacement(transform_ConcreteShieldR, logi_ConcreteShieldR, "phys_ConcreteShieldR_name", &topVolume, false, 0);

      //--------------
      //-   ConcreteShieldL

      //get parameters from .xml file
      prep = "ConcreteShieldL.";

      double ConcreteShieldL_X = m_config.getParameter(prep + "X") * unitFactor;
      double ConcreteShieldL_Y = m_config.getParameter(prep + "Y") * unitFactor;
      double ConcreteShieldL_Z = m_config.getParameter(prep + "Z") * unitFactor;
      double ConcreteShieldL_DZ = m_config.getParameter(prep + "DZ") * unitFactor;
      double ConcreteShieldL_x = m_config.getParameter(prep + "x") * unitFactor;
      double ConcreteShieldL_y = m_config.getParameter(prep + "y") * unitFactor;
      double ConcreteShieldL_dx = m_config.getParameter(prep + "dx") * unitFactor;
      double ConcreteShieldL_dy = m_config.getParameter(prep + "dy") * unitFactor;

      G4Transform3D transform_ConcreteShieldL = G4Translate3D(0.0, 0.0, ConcreteShieldL_DZ);
      transform_ConcreteShieldL = transform_ROT * transform_ConcreteShieldL;
      G4Transform3D transform_ConcreteShieldL_Hole = G4Translate3D(ConcreteShieldL_dx, ConcreteShieldL_dy, 0.0);

      //define geometry
      G4Box* geo_ConcreteShieldLx = new G4Box("geo_ConcreteShieldLx_name", ConcreteShieldL_X, ConcreteShieldL_Y, ConcreteShieldL_Z);
      G4Box* geo_ConcreteShieldL_Hole = new G4Box("geo_ConcreteShieldLxx_name", ConcreteShieldL_x, ConcreteShieldL_y, ConcreteShieldL_Z);
      G4SubtractionSolid* geo_ConcreteShieldL = new G4SubtractionSolid("geo_ConcreteShieldL_name", geo_ConcreteShieldLx,
          geo_ConcreteShieldL_Hole, transform_ConcreteShieldL_Hole);

      string strMat_ConcreteShieldL = m_config.getParameterStr(prep + "Material");
      G4Material* mat_ConcreteShieldL = Materials::get(strMat_ConcreteShieldL);
      G4LogicalVolume* logi_ConcreteShieldL = new G4LogicalVolume(geo_ConcreteShieldL, mat_ConcreteShieldL, "logi_ConcreteShieldL_name");

      //put volume
      setColor(*logi_ConcreteShieldL, "#0000CC");
      //setVisibility(*logi_ConcreteShieldL, false);
      new G4PVPlacement(transform_ConcreteShieldL, logi_ConcreteShieldL, "phys_ConcreteShieldL_name", &topVolume, false, 0);


      //---------------------------
      // for dose simulation
      //---------------------------

      //neutron shield (poly)
      //logi_polyShieldL->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1001));
      //logi_polyShieldR->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1002));

      //additional neutron shield (concrete)
      //logi_ConcreteShieldL->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1003));
      //logi_ConcreteShieldR->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1004));

      //gate shield (concrete)
      //logi_GateShield->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1005));

      //virtual material outsire gate-shield
      if (radiation_study) {
        logi_Tube->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1006));
      }

    }
  }
}
