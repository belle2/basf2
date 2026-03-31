/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ir/geometry/GeoFarBeamLineCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/Unit.h>
#include <ir/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <cmath>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Torus.hh>
#include <G4Polycone.hh>
#include <G4Trd.hh>
#include <G4Trap.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4MultiUnion.hh>
#include <G4UserLimits.hh>

using namespace std;

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
      // basf2 uses cm, Geant4 uses mm
      const double unitFactor = Unit::cm / Unit::mm;

      const int N = 2;

      double stepMax = 5.0 * Unit::mm * unitFactor;
      int flag_limitStep = int(m_config.getParameter("LimitStepLength"));


      // keep created elements
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

      tubeR.logi = NULL;

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

      tubeL.logi = NULL;

      elements["TubeL"] = tubeL;

      std::vector<double> zero_r(N, 0.);

      std::vector<std::string> straightSections;
      boost::split(straightSections, m_config.getParameterStr("Straight"), boost::is_any_of(" "));
      for (const auto& name : straightSections) {
        //--------------
        //-   Create straight element

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
        double Polycone_Y0 = m_config.getParameter(prep + "Y0", 0) * unitFactor;
        double Polycone_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double Polycone_PHI = m_config.getParameter(prep + "PHI");
        double Polycone_PHIYZ = m_config.getParameter(prep + "PHIYZ", 0);

        polycone.transform = G4Translate3D(Polycone_X0, Polycone_Y0, Polycone_Z0);
        polycone.transform = polycone.transform * G4RotateY3D(Polycone_PHI / Unit::rad);
        if (Polycone_PHIYZ != 0)
          polycone.transform = polycone.transform * G4RotateX3D(Polycone_PHIYZ / Unit::rad);

        //define geometry
        string subtract = m_config.getParameterStr(prep + "Subtract", "");
        string intersect = m_config.getParameterStr(prep + "Intersect", "");

        string geo_polyconexx_name = "geo_" + name + "xx_name";
        string geo_polyconex_name = "geo_" + name + "x_name";
        string geo_polycone_name = "geo_" + name + "_name";

        G4VSolid* geo_polyconexx(NULL), *geo_polycone(NULL);

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
          G4VSolid* geo_polyconex = new G4SubtractionSolid(geo_polyconex_name, geo_polyconexx, elements[subtract].geo,
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

        polycone.logi = logi_polycone;

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

          vacuum.logi = logi_vacuum;

          //put volume
          string phys_vacuum_name = "phys_" + nameVac + "_name";
          //new G4PVPlacement(vacuum.transform, logi_vacuum, phys_vacuum_name, &topVolume, false, 0);
          new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_vacuum, phys_vacuum_name, logi_polycone, false, 0);

          elements[nameVac] = vacuum;
        }
      }


      std::vector<std::string> bendingSections;
      boost::split(bendingSections, m_config.getParameterStr("Bending"), boost::is_any_of(" "));
      for (const auto& name : bendingSections) {
        //--------------
        //-   Create torus element

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

        G4VSolid* geo_torus(NULL);

        if (subtract != "" || intersect != "") {
          G4VSolid* geo_torusxx(NULL);
          if (type == "pipe") // for pipes inner space will be created as vacuum
            geo_torusxx = new G4Torus(geo_torusxx_name, 0, torus_R, torus_RT, torus_SPHI, torus_DPHI);
          else
            geo_torusxx = new G4Torus(geo_torusxx_name, torus_r, torus_R, torus_RT, torus_SPHI, torus_DPHI);
          if (subtract != "" && intersect != "") {
            G4VSolid* geo_torusx = new G4SubtractionSolid(geo_torusx_name, geo_torusxx, elements[subtract].geo,
                                                          torus.transform.inverse()*elements[subtract].transform);
            geo_torus = new G4IntersectionSolid(geo_torus_name, geo_torusx, elements[intersect].geo,
                                                torus.transform.inverse()*elements[intersect].transform);
          } else if (subtract != "")
            geo_torus  = new G4SubtractionSolid(geo_torus_name, geo_torusxx, elements[subtract].geo,
                                                torus.transform.inverse()*elements[subtract].transform);
          else
            geo_torus = new G4IntersectionSolid(geo_torus_name, geo_torusxx, elements[intersect].geo,
                                                torus.transform.inverse()*elements[intersect].transform);
        } else if (type == "pipe") // for pipes inner space will be created as vacuum
          geo_torus = new G4Torus(geo_torus_name, 0, torus_R, torus_RT, torus_SPHI, torus_DPHI);
        else
          geo_torus = new G4Torus(geo_torus_name, torus_r, torus_R, torus_RT, torus_SPHI, torus_DPHI);

        torus.geo = geo_torus;

        // define logical volume
        string strMat_torus = m_config.getParameterStr(prep + "Material");
        G4Material* mat_torus = Materials::get(strMat_torus);
        string logi_torus_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_torus = new G4LogicalVolume(torus.geo, mat_torus, logi_torus_name);
        setColor(*logi_torus, "#CC0000");
        setVisibility(*logi_torus, false);

        torus.logi = logi_torus;

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

          vacuum.logi = logi_vacuum;

          //put volume
          string phys_vacuum_name = "phys_" + nameVac + "_name";
          //new G4PVPlacement(vacuum.transform, logi_vacuum, phys_vacuum_name, &topVolume, false, 0);
          new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_vacuum, phys_vacuum_name, logi_torus, false, 0);

          elements[nameVac] = vacuum;
        }
      }


      //--------------------------------------------------------------------------------------------
      //-   Gate shields, end-of-tunnel concrete shields, polyethylene shields, collimator shields

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
        FarBeamLineElement shield;

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


      //--------------
      //-   Tube (virtual tube for radiation level study)

      //define geometry
      G4Tubs* geo_Tube = new G4Tubs("geo_Tube_name", 3995 * CLHEP::mm, 4000 * CLHEP::mm, 29 * CLHEP::m, 0. * CLHEP::deg, 360.*CLHEP::deg);
      G4Material* mat_Tube = Materials::get("G4_Si");
      G4LogicalVolume* logi_Tube = new G4LogicalVolume(geo_Tube, mat_Tube, "logi_Tube_name");

      //put volume
      setColor(*logi_Tube, "#CC0000");
      setVisibility(*logi_Tube, false);
      bool radiation_study = false;
      // cppcheck-suppress knownConditionTrueFalse
      if (radiation_study && elements.count("GateShieldL")) {
        new G4PVPlacement(elements["GateShieldL"].transform, logi_Tube, "phys_Tube_name", &topVolume, false, 0);
      }


      //---------------------------
      // for dose simulation
      //---------------------------

      // cppcheck-suppress knownConditionTrueFalse
      if (radiation_study) {
        //neutron shield (poly)
        if (elements.count("PolyShieldL"))
          elements["PolyShieldL"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1001));
        if (elements.count("PolyShieldR"))
          elements["PolyShieldR"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1002));

        //additional neutron shield (concrete)
        if (elements.count("ConcreteShieldL"))
          elements["ConcreteShieldL"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1003));
        if (elements.count("ConcreteShieldR"))
          elements["ConcreteShieldR"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1004));

        //gate shield (concrete)
        if (elements.count("GateShieldL"))
          elements["GateShieldL"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1005));
        if (elements.count("GateShieldR"))
          elements["GateShieldR"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1006));

        //virtual material outside gate-shield
        logi_Tube->SetSensitiveDetector(new BkgSensitiveDetector("IR", 1007));
      }


      //------------------
      //-   Collimators

      std::vector<std::string> collimators;
      boost::split(collimators, m_config.getParameterStr("Collimator"), boost::is_any_of(" "));
      for (const auto& name : collimators) {
        //-   Collimators consist of two independent jaws (trapezoids), identical in shape, positioned opposite to each other
        //-   Each jaw consists of copper body and high Z head

        prep = name + ".";

        string type = m_config.getParameterStr(prep + "type");
        string motherVolume = m_config.getParameterStr(prep + "MotherVolume");
        string motherVolumeVacuum = motherVolume + "Vac";

        // If zz < 0 (positioned at negative z) vertical collimator is flipped when rotated into Mother Volume system
        G4Scale3D scale;
        G4Rotate3D rotation;
        G4Translate3D translation;
        elements[motherVolumeVacuum].transform.getDecomposition(scale, rotation, translation);
        double zz = rotation.zz();

        // d1, d2 are collimator jaws displacements from beam center, d1<0, d2>0
        // Z is collimator position inside its Mother Volume
        double collimator_d1 = m_config.getParameter(prep + "d1") * unitFactor;
        double collimator_d2 = m_config.getParameter(prep + "d2") * unitFactor;
        double collimator_fullH = m_config.getParameter(prep + "fullH") * unitFactor;
        double collimator_headH = m_config.getParameter(prep + "headH") * unitFactor;
        double collimator_minW = m_config.getParameter(prep + "minW") * unitFactor;
        double collimator_maxW = m_config.getParameter(prep + "maxW") * unitFactor;
        double collimator_th = m_config.getParameter(prep + "th") * unitFactor;
        double collimator_Z = m_config.getParameter(prep + "Z") * unitFactor;

        B2WARNING("Collimator " << name << " displacement d1 is set to " << collimator_d1 << "mm (must be negative)");
        B2WARNING("Collimator " << name << " displacement d2 is set to " << collimator_d2 << "mm (must be positive)");


        // Collimator heads

        // dx1,2 dy1,2 dz are trapezoid dimensions
        double head_dx1;
        double head_dx2;
        double head_dy1;
        double head_dy2;
        double head_dz = collimator_headH / 2.0;
        if (type == "vertical") {
          head_dx1 = collimator_th / 2.0;
          head_dx2 = collimator_th / 2.0;
          head_dy1 = ((collimator_maxW - collimator_minW) * collimator_headH / collimator_fullH + collimator_minW) / 2.0;
          head_dy2 = collimator_minW / 2.0;
        } else {
          head_dx1 = ((collimator_maxW - collimator_minW) * collimator_headH / collimator_fullH + collimator_minW) / 2.0;
          head_dx2 = collimator_minW / 2.0;
          head_dy1 = collimator_th / 2.0;
          head_dy2 = collimator_th / 2.0;
        }

        // storable elements
        FarBeamLineElement collimator_head1;
        FarBeamLineElement collimator_head2;

        // move collimator to position on beam line
        G4Transform3D transform_head1 = G4Translate3D(0.0, 0.0, collimator_Z);
        G4Transform3D transform_head2 = G4Translate3D(0.0, 0.0, collimator_Z);

        // rotate and move collimator jaws to their relative positions
        if (type == "vertical") {
          transform_head1 = transform_head1 * G4Translate3D(0.0, -head_dz + collimator_d1, 0.0);
          transform_head1 = transform_head1 * G4RotateX3D(-M_PI / 2 / Unit::rad);

          transform_head2 = transform_head2 * G4Translate3D(0.0, head_dz + collimator_d2, 0.0);
          transform_head2 = transform_head2 * G4RotateX3D(M_PI / 2 / Unit::rad);
        } else {
          if (zz > 0) {
            transform_head1 = transform_head1 * G4Translate3D(-head_dz + collimator_d1, 0.0, 0.0);
            transform_head1 = transform_head1 * G4RotateY3D(M_PI / 2 / Unit::rad);

            transform_head2 = transform_head2 * G4Translate3D(head_dz + collimator_d2, 0.0, 0.0);
            transform_head2 = transform_head2 * G4RotateY3D(-M_PI / 2 / Unit::rad);
          } else {
            transform_head1 = transform_head1 * G4Translate3D(head_dz - collimator_d1, 0.0, 0.0);
            transform_head1 = transform_head1 * G4RotateY3D(-M_PI / 2 / Unit::rad);

            transform_head2 = transform_head2 * G4Translate3D(-head_dz - collimator_d2, 0.0, 0.0);
            transform_head2 = transform_head2 * G4RotateY3D(M_PI / 2 / Unit::rad);
          }
        }

        collimator_head1.transform = transform_head1;
        collimator_head2.transform = transform_head2;

        // define geometry
        string geo_headx_name = "geo_" + name + "_headx_name";

        string geo_head1_name = "geo_" + name + "_head1_name";
        string geo_head2_name = "geo_" + name + "_head2_name";

        G4VSolid* geo_headx = new G4Trd(geo_headx_name, head_dx1, head_dx2, head_dy1, head_dy2, head_dz);

        G4VSolid* geo_head1 = new G4IntersectionSolid(geo_head1_name, geo_headx, elements[motherVolumeVacuum].geo,
                                                      collimator_head1.transform.inverse());
        G4VSolid* geo_head2 = new G4IntersectionSolid(geo_head2_name, geo_headx, elements[motherVolumeVacuum].geo,
                                                      collimator_head2.transform.inverse());

        collimator_head1.geo = geo_head1;
        collimator_head2.geo = geo_head2;

        // define logical volume
        string strMat_head = m_config.getParameterStr(prep + "HeadMaterial");
        G4Material* mat_head = Materials::get(strMat_head);
        string logi_head1_name = "logi_" + name + "_head1_name";
        string logi_head2_name = "logi_" + name + "_head2_name";
        G4LogicalVolume* logi_head1 = new G4LogicalVolume(geo_head1, mat_head, logi_head1_name);
        G4LogicalVolume* logi_head2 = new G4LogicalVolume(geo_head2, mat_head, logi_head2_name);
        setColor(*logi_head1, "#CC0000");
        setColor(*logi_head2, "#CC0000");
        setVisibility(*logi_head1, false);
        setVisibility(*logi_head2, false);

        // check if collimator is inside beam pipe
        double volume_head1 = logi_head1->GetSolid()->GetCubicVolume();
        double volume_head2 = logi_head2->GetSolid()->GetCubicVolume();

        collimator_head1.logi = logi_head1;
        collimator_head2.logi = logi_head2;

        // put volume
        string phys_head1_name = "phys_" + name + "_head1" + "_name";
        string phys_head2_name = "phys_" + name + "_head2" + "_name";
        if (volume_head1 != 0)
          new G4PVPlacement(collimator_head1.transform, logi_head1, phys_head1_name, elements[motherVolumeVacuum].logi, false, 0);
        if (volume_head2 != 0)
          new G4PVPlacement(collimator_head2.transform, logi_head2, phys_head2_name, elements[motherVolumeVacuum].logi, false, 0);

        // to use it later in "intersect" and "subtract"
        collimator_head1.transform = collimator_head1.transform * elements[motherVolumeVacuum].transform;
        collimator_head2.transform = collimator_head2.transform * elements[motherVolumeVacuum].transform;

        string name_head1 = name + "_head1";
        string name_head2 = name + "_head2";
        elements[name_head1] = collimator_head1;
        elements[name_head2] = collimator_head2;


        // Collimator bodies

        // dx1,2 dy1,2 dz are trapezoid dimensions
        double body_dx1;
        double body_dx2;
        double body_dy1;
        double body_dy2;
        double body_dz = (collimator_fullH - collimator_headH) / 2.0;
        if (type == "vertical") {
          body_dx1 = collimator_th / 2.0;
          body_dx2 = collimator_th / 2.0;
          body_dy1 = collimator_maxW / 2.0;
          body_dy2 = ((collimator_maxW - collimator_minW) * collimator_headH / collimator_fullH + collimator_minW) / 2.0;
        } else {
          body_dx1 = collimator_maxW / 2.0;
          body_dx2 = ((collimator_maxW - collimator_minW) * collimator_headH / collimator_fullH + collimator_minW) / 2.0;
          body_dy1 = collimator_th / 2.0;
          body_dy2 = collimator_th / 2.0;
        }

        // storable elements
        FarBeamLineElement collimator_body1;
        FarBeamLineElement collimator_body2;

        // reuse head transformation with additional shift
        if (type == "vertical") {
          collimator_body1.transform = G4Translate3D(0.0, -head_dz - body_dz, 0.0) * transform_head1;
          collimator_body2.transform = G4Translate3D(0.0, head_dz + body_dz, 0.0) * transform_head2;
        } else {
          if (zz > 0) {
            collimator_body1.transform = G4Translate3D(-head_dz - body_dz, 0.0, 0.0) * transform_head1;
            collimator_body2.transform = G4Translate3D(head_dz + body_dz, 0.0, 0.0) * transform_head2;
          } else {
            collimator_body1.transform = G4Translate3D(head_dz + body_dz, 0.0, 0.0) * transform_head1;
            collimator_body2.transform = G4Translate3D(-head_dz - body_dz, 0.0, 0.0) * transform_head2;
          }
        }

        // define geometry
        string geo_bodyx_name = "geo_" + name + "_bodyx_name";

        string geo_body1_name = "geo_" + name + "_body1_name";
        string geo_body2_name = "geo_" + name + "_body2_name";

        G4VSolid* geo_bodyx = new G4Trd(geo_bodyx_name, body_dx1, body_dx2, body_dy1, body_dy2, body_dz);

        G4VSolid* geo_body1 = new G4IntersectionSolid(geo_body1_name, geo_bodyx, elements[motherVolumeVacuum].geo,
                                                      collimator_body1.transform.inverse());
        G4VSolid* geo_body2 = new G4IntersectionSolid(geo_body2_name, geo_bodyx, elements[motherVolumeVacuum].geo,
                                                      collimator_body2.transform.inverse());

        collimator_body1.geo = geo_body1;
        collimator_body2.geo = geo_body2;

        // define logical volume
        string strMat_body = m_config.getParameterStr(prep + "Material");
        G4Material* mat_body = Materials::get(strMat_body);
        string logi_body1_name = "logi_" + name + "_body1_name";
        string logi_body2_name = "logi_" + name + "_body2_name";
        G4LogicalVolume* logi_body1 = new G4LogicalVolume(geo_body1, mat_body, logi_body1_name);
        G4LogicalVolume* logi_body2 = new G4LogicalVolume(geo_body2, mat_body, logi_body2_name);
        setColor(*logi_body1, "#CC0000");
        setColor(*logi_body2, "#CC0000");
        setVisibility(*logi_body1, false);
        setVisibility(*logi_body2, false);

        // check if collimator is inside beam pipe
        double volume_body1 = logi_body1->GetSolid()->GetCubicVolume();
        double volume_body2 = logi_body2->GetSolid()->GetCubicVolume();

        collimator_body1.logi = logi_body1;
        collimator_body2.logi = logi_body2;

        // put volume
        string phys_body1_name = "phys_" + name + "_body1" + "_name";
        string phys_body2_name = "phys_" + name + "_body2" + "_name";
        if (volume_body1 != 0)
          new G4PVPlacement(collimator_body1.transform, logi_body1, phys_body1_name, elements[motherVolumeVacuum].logi, false, 0);
        if (volume_body2 != 0)
          new G4PVPlacement(collimator_body2.transform, logi_body2, phys_body2_name, elements[motherVolumeVacuum].logi, false, 0);

        // to use it later in "intersect" and "subtract"
        collimator_body1.transform = collimator_body1.transform * elements[motherVolumeVacuum].transform;
        collimator_body2.transform = collimator_body2.transform * elements[motherVolumeVacuum].transform;

        string name_body1 = name + "_body1";
        string name_body2 = name + "_body2";
        elements[name_body1] = collimator_body1;
        elements[name_body2] = collimator_body2;
      }


      //----------
      //- Additional shields added during LS1

      //--------------
      //-   Additional concrete shield left (backward)
      //-   Fills gap between end-of-tunnel shield and QCS

      if (m_config.getParameter("ACSL.box1_W", -1) > 0) { // Run2
        FarBeamLineElement ACSL;

        //get parameters from .xml file
        std::string name = "ACSL";
        prep = name + ".";

        double acsl_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double acsl_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double acsl_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double acsl_PHI = m_config.getParameter(prep + "PHI");

        double acsl_box1_W = m_config.getParameter(prep + "box1_W") * unitFactor;
        double acsl_box1_H = m_config.getParameter(prep + "box1_H") * unitFactor;
        double acsl_box1_L = m_config.getParameter(prep + "box1_L") * unitFactor;

        double acsl_box2_W = m_config.getParameter(prep + "box2_W") * unitFactor;
        double acsl_box2_H = m_config.getParameter(prep + "box2_H") * unitFactor;
        double acsl_box2_L = m_config.getParameter(prep + "box2_L") * unitFactor;
        double acsl_box2_dX = m_config.getParameter(prep + "box2_dX") * unitFactor;
        double acsl_box2_dY = m_config.getParameter(prep + "box2_dY") * unitFactor;
        double acsl_box2_dZ = m_config.getParameter(prep + "box2_dZ") * unitFactor;

        double acsl_trd1_X1 = m_config.getParameter(prep + "trd1_X1") * unitFactor;
        double acsl_trd1_X2 = m_config.getParameter(prep + "trd1_X2") * unitFactor;
        double acsl_trd1_Y1 = m_config.getParameter(prep + "trd1_Y1") * unitFactor;
        double acsl_trd1_Y2 = m_config.getParameter(prep + "trd1_Y2") * unitFactor;
        double acsl_trd1_Z  = m_config.getParameter(prep + "trd1_Z") * unitFactor;
        double acsl_trd1_dX  = m_config.getParameter(prep + "trd1_dX") * unitFactor;
        double acsl_trd1_dY  = m_config.getParameter(prep + "trd1_dY") * unitFactor;
        double acsl_trd1_dZ  = m_config.getParameter(prep + "trd1_dZ") * unitFactor;
        double acsl_trd1_PHI = m_config.getParameter(prep + "trd1_PHI");

        double acsl_trd2_X1 = m_config.getParameter(prep + "trd2_X1") * unitFactor;
        double acsl_trd2_X2 = m_config.getParameter(prep + "trd2_X2") * unitFactor;
        double acsl_trd2_Y1 = m_config.getParameter(prep + "trd2_Y1") * unitFactor;
        double acsl_trd2_Y2 = m_config.getParameter(prep + "trd2_Y2") * unitFactor;
        double acsl_trd2_Z  = m_config.getParameter(prep + "trd2_Z") * unitFactor;
        double acsl_trd2_dX  = m_config.getParameter(prep + "trd2_dX") * unitFactor;
        double acsl_trd2_dY  = m_config.getParameter(prep + "trd2_dY") * unitFactor;
        double acsl_trd2_dZ  = m_config.getParameter(prep + "trd2_dZ") * unitFactor;
        double acsl_trd2_PHI = m_config.getParameter(prep + "trd2_PHI");

        // solids
        string geo_box1_name = "geo_" + name + "_box1_name";
        string geo_box2_name = "geo_" + name + "_box2_name";
        string geo_trd1_name = "geo_" + name + "_trd1_name";
        string geo_trd2_name = "geo_" + name + "_trd2_name";
        string geo_acsl_name = "geo_" + name + "_name";

        G4Box* geo_box1 = new G4Box(geo_box1_name, acsl_box1_W / 2.0, acsl_box1_H / 2.0, acsl_box1_L / 2.0);
        G4Box* geo_box2 = new G4Box(geo_box2_name, acsl_box2_W / 2.0, acsl_box2_H / 2.0, acsl_box2_L / 2.0);
        G4Trd* geo_trd1 = new G4Trd(geo_trd1_name, acsl_trd1_X1, acsl_trd1_X2, acsl_trd1_Y1, acsl_trd1_Y2, acsl_trd1_Z);
        G4Trd* geo_trd2 = new G4Trd(geo_trd2_name, acsl_trd2_X1, acsl_trd2_X2, acsl_trd2_Y1, acsl_trd2_Y2, acsl_trd2_Z);

        // transformations
        // G4Transform3D box1_trans = G4Translate3D(0, 0, 0);

        G4Transform3D box2_trans = G4Translate3D(acsl_box2_dX, acsl_box2_dY, acsl_box2_dZ);

        G4Transform3D trd1_trans = G4Translate3D(acsl_trd1_dX, acsl_trd1_dY, acsl_trd1_dZ);
        trd1_trans = trd1_trans * G4RotateY3D(acsl_trd1_PHI / Unit::rad);

        G4Transform3D trd2_trans = G4Translate3D(acsl_trd2_dX, acsl_trd2_dY, acsl_trd2_dZ);
        trd2_trans = trd2_trans * G4RotateY3D(acsl_trd2_PHI / Unit::rad);

        ACSL.transform = G4Translate3D(acsl_X0, acsl_Y0, acsl_Z0);
        ACSL.transform = ACSL.transform * G4RotateY3D(acsl_PHI / Unit::rad);;

        /******************
        not supported yet by Geant4VM

        // composite solid
        G4MultiUnion* geo_acsl = new G4MultiUnion(geo_acsl_name);
        geo_acsl->AddNode(*geo_box1, box1_trans);
        geo_acsl->AddNode(*geo_box2, box2_trans);
        geo_acsl->AddNode(*geo_trd1, trd1_trans);
        geo_acsl->AddNode(*geo_trd2, trd2_trans);
        geo_acsl->Voxelize();
         ***********************/

        string geo_interm1_name = "geo_" + name + "_interm1_name";
        string geo_interm2_name = "geo_" + name + "_interm2_name";
        G4UnionSolid* geo_interm1 = new G4UnionSolid(geo_interm1_name, geo_box1, geo_box2, box2_trans);
        G4UnionSolid* geo_interm2 = new G4UnionSolid(geo_interm2_name, geo_interm1, geo_trd1, trd1_trans);
        G4UnionSolid* geo_acsl = new G4UnionSolid(geo_acsl_name, geo_interm2, geo_trd2, trd2_trans);

        ACSL.geo = geo_acsl;

        // define logical volume
        string strMat_acsl = m_config.getParameterStr(prep + "Material");
        G4Material* mat_acsl = Materials::get(strMat_acsl);
        string logi_acsl_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_acsl = new G4LogicalVolume(geo_acsl, mat_acsl, logi_acsl_name);
        setColor(*logi_acsl, "#00CC00");
        setVisibility(*logi_acsl, false);

        ACSL.logi = logi_acsl;

        //put volume
        string phys_acsl_name = "phys_" + name + "_name";
        new G4PVPlacement(ACSL.transform, ACSL.logi, phys_acsl_name, &topVolume, false, 0);

        // elements[name] = ACSL;


        //--------------
        //-   Additional concrete shield right (forward)
        //-   Fills gap between end-of-tunnel shield and QCS

        // Consists of two disjoined trapezoids
        // Part 1

        FarBeamLineElement ACSR1;

        //get parameters from .xml file
        name = "ACSR1";
        prep = name + ".";

        double acsr1_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double acsr1_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double acsr1_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double acsr1_PHI0 = m_config.getParameter(prep + "PHI0");

        double acsr1_X1 = m_config.getParameter(prep + "X1") * unitFactor;
        double acsr1_X2 = m_config.getParameter(prep + "X2") * unitFactor;
        double acsr1_Y1 = m_config.getParameter(prep + "Y1") * unitFactor;
        double acsr1_Y2 = m_config.getParameter(prep + "Y2") * unitFactor;
        double acsr1_Z  = m_config.getParameter(prep + "Z") * unitFactor;
        double acsr1_THETA  = m_config.getParameter(prep + "THETA");
        double acsr1_PHI  = m_config.getParameter(prep + "PHI");
        double acsr1_ANG  = m_config.getParameter(prep + "ANG");

        double acsr1_dx = (acsr1_X1 - acsr1_X2) / 2.0;
        double acsr1_dy = (acsr1_Y2 - acsr1_Y1) / 2.0;

        //      G4ThreeVector acsr1_pt[8];
        //      acsr1_pt[0] = G4ThreeVector(              -acsr1_X1 + acsr1_dx,                -acsr1_Y1 + acsr1_dy, -acsr1_Z);
        //      acsr1_pt[1] = G4ThreeVector(               acsr1_X1 + acsr1_dx,                -acsr1_Y1 + acsr1_dy, -acsr1_Z);
        //      acsr1_pt[2] = G4ThreeVector(              -acsr1_X1 + acsr1_dx,                 acsr1_Y1 + acsr1_dy, -acsr1_Z);
        //      acsr1_pt[3] = G4ThreeVector(               acsr1_X1 + acsr1_dx,                 acsr1_Y1 + acsr1_dy, -acsr1_Z);
        //      acsr1_pt[4] = G4ThreeVector(              -acsr1_X1 + acsr1_dx, -2.0*acsr1_Y2 + acsr1_Y1 + acsr1_dy,  acsr1_Z);
        //      acsr1_pt[5] = G4ThreeVector( 2.0*acsr1_X2 -acsr1_X1 + acsr1_dx, -2.0*acsr1_Y2 + acsr1_Y1 + acsr1_dy,  acsr1_Z);
        //      acsr1_pt[6] = G4ThreeVector(              -acsr1_X1 + acsr1_dx,                 acsr1_Y1 + acsr1_dy,  acsr1_Z);
        //      acsr1_pt[7] = G4ThreeVector( 2.0*acsr1_X2 -acsr1_X1 + acsr1_dx,                 acsr1_Y1 + acsr1_dy,  acsr1_Z);

        // solids
        string geo_acsr1_name = "geo_" + name + "_name";
        G4Trap* geo_acsr1 = new G4Trap(geo_acsr1_name, acsr1_Z, acsr1_THETA, acsr1_PHI, acsr1_Y1, acsr1_X1, acsr1_X1, acsr1_ANG, acsr1_Y2,
                                       acsr1_X2, acsr1_X2, acsr1_ANG);
        //      G4Trap* geo_acsr1 = new G4Trap(geo_acsr1_name, acsr1_pt);

        ACSR1.geo = geo_acsr1;

        // transformations
        ACSR1.transform = G4Translate3D(acsr1_X0, acsr1_Y0 - acsr1_dy, acsr1_Z0 - acsr1_dx);
        ACSR1.transform = ACSR1.transform * G4RotateY3D(acsr1_PHI0 / Unit::rad);;

        // define logical volume
        string strMat_acsr1 = m_config.getParameterStr(prep + "Material");
        G4Material* mat_acsr1 = Materials::get(strMat_acsr1);
        string logi_acsr1_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_acsr1 = new G4LogicalVolume(geo_acsr1, mat_acsr1, logi_acsr1_name);
        setColor(*logi_acsr1, "#00CC00");
        setVisibility(*logi_acsr1, false);

        ACSR1.logi = logi_acsr1;

        //put volume
        string phys_acsr1_name = "phys_" + name + "_name";
        new G4PVPlacement(ACSR1.transform, ACSR1.logi, phys_acsr1_name, &topVolume, false, 0);

        // elements[name] = ACSR1;

        //--------------
        // Part 2

        FarBeamLineElement ACSR2;

        //get parameters from .xml file
        name = "ACSR2";
        prep = name + ".";

        double acsr2_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double acsr2_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double acsr2_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double acsr2_PHI0 = m_config.getParameter(prep + "PHI0");

        double acsr2_X1 = m_config.getParameter(prep + "X1") * unitFactor;
        double acsr2_X2 = m_config.getParameter(prep + "X2") * unitFactor;
        double acsr2_Y1 = m_config.getParameter(prep + "Y1") * unitFactor;
        double acsr2_Y2 = m_config.getParameter(prep + "Y2") * unitFactor;
        double acsr2_Z  = m_config.getParameter(prep + "Z") * unitFactor;
        double acsr2_THETA  = m_config.getParameter(prep + "THETA");
        double acsr2_PHI  = m_config.getParameter(prep + "PHI");
        double acsr2_ANG  = m_config.getParameter(prep + "ANG");

        double acsr2_dx = (acsr2_X1 - acsr2_X2) / 2.0;
        double acsr2_dy = (acsr2_Y2 - acsr2_Y1) / 2.0;

        //      G4ThreeVector acsr2_pt[8];
        //      acsr2_pt[0] = G4ThreeVector(              -acsr2_X1 + acsr2_dx,                -acsr2_Y1 + acsr2_dy, -acsr2_Z);
        //      acsr2_pt[1] = G4ThreeVector(               acsr2_X1 + acsr2_dx,                -acsr2_Y1 + acsr2_dy, -acsr2_Z);
        //      acsr2_pt[2] = G4ThreeVector(              -acsr2_X1 + acsr2_dx,                 acsr2_Y1 + acsr2_dy, -acsr2_Z);
        //      acsr2_pt[3] = G4ThreeVector(               acsr2_X1 + acsr2_dx,                 acsr2_Y1 + acsr2_dy, -acsr2_Z);
        //      acsr2_pt[4] = G4ThreeVector(              -acsr2_X1 + acsr2_dx, -2.0*acsr2_Y2 + acsr2_Y1 + acsr2_dy,  acsr2_Z);
        //      acsr2_pt[5] = G4ThreeVector( 2.0*acsr2_X2 -acsr2_X1 + acsr2_dx, -2.0*acsr2_Y2 + acsr2_Y1 + acsr2_dy,  acsr2_Z);
        //      acsr2_pt[6] = G4ThreeVector(              -acsr2_X1 + acsr2_dx,                 acsr2_Y1 + acsr2_dy,  acsr2_Z);
        //      acsr2_pt[7] = G4ThreeVector( 2.0*acsr2_X2 -acsr2_X1 + acsr2_dx,                 acsr2_Y1 + acsr2_dy,  acsr2_Z);

        // solids
        string geo_acsr2_name = "geo_" + name + "_name";
        G4Trap* geo_acsr2 = new G4Trap(geo_acsr2_name, acsr2_Z, acsr2_THETA, acsr2_PHI, acsr2_Y1, acsr2_X1, acsr2_X1, acsr2_ANG, acsr2_Y2,
                                       acsr2_X2, acsr2_X2, acsr2_ANG);
        //      G4Trap* geo_acsr2 = new G4Trap(geo_acsr2_name, acsr2_pt);

        ACSR2.geo = geo_acsr2;

        // transformations
        ACSR2.transform = G4Translate3D(acsr2_X0, acsr2_Y0 - acsr2_dy, acsr2_Z0 - acsr2_dx);
        ACSR2.transform = ACSR2.transform * G4RotateY3D(acsr2_PHI0 / Unit::rad);;

        // define logical volume
        string strMat_acsr2 = m_config.getParameterStr(prep + "Material");
        G4Material* mat_acsr2 = Materials::get(strMat_acsr2);
        string logi_acsr2_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_acsr2 = new G4LogicalVolume(geo_acsr2, mat_acsr2, logi_acsr2_name);
        setColor(*logi_acsr2, "#00CC00");
        setVisibility(*logi_acsr2, false);

        ACSR2.logi = logi_acsr2;

        //put volume
        string phys_acsr2_name = "phys_" + name + "_name";
        new G4PVPlacement(ACSR2.transform, ACSR2.logi, phys_acsr2_name, &topVolume, false, 0);

        // elements[name] = ACSR2;


        //--------------
        //-   Additional polyethilene shield left (backward)
        //-   Fills gap between PolyShield and QCS

        // Consists of two disjoined parts
        // Part 1

        FarBeamLineElement APSL1;

        //get parameters from .xml file
        name = "APSL1";
        prep = name + ".";

        double apsl1_W = m_config.getParameter(prep + "W") * unitFactor;
        double apsl1_H = m_config.getParameter(prep + "H") * unitFactor;
        double apsl1_L = m_config.getParameter(prep + "L") * unitFactor;
        double apsl1_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double apsl1_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double apsl1_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double apsl1_hole_R = m_config.getParameter(prep + "holeR") * unitFactor;
        double apsl1_hole_D = m_config.getParameter(prep + "holeD") * unitFactor;
        double apsl1_hole_dX = m_config.getParameter(prep + "holeDX") * unitFactor;
        double apsl1_hole_dY = m_config.getParameter(prep + "holeDY") * unitFactor;
        double apsl1_hole_dZ = m_config.getParameter(prep + "holeDZ") * unitFactor;
        double apsl1_PHI = m_config.getParameter(prep + "PHI");

        // transformations
        APSL1.transform = G4Translate3D(apsl1_X0, apsl1_Y0, apsl1_Z0);
        APSL1.transform = APSL1.transform * G4RotateY3D(apsl1_PHI / Unit::rad);;

        G4Transform3D transform_apsl1_hole = G4Translate3D(apsl1_hole_dX, apsl1_hole_dY, apsl1_hole_dZ);

        //define geometry
        string geo_apsl1_box_name = "geo_" + name + "_box_name";
        string geo_apsl1_hole_name = "geo_" + name + "_hole_name";
        string geo_apsl1_name = "geo_" + name + "_name";

        G4Box* geo_apsl1_box = new G4Box(geo_apsl1_box_name, apsl1_W / 2.0, apsl1_H / 2.0, apsl1_L / 2.0);
        G4Tubs* geo_apsl1_hole = new G4Tubs(geo_apsl1_hole_name, 0.0, apsl1_hole_R, apsl1_hole_D / 2.0, 0.0, 2.0 * M_PI);
        G4SubtractionSolid* geo_apsl1 = new G4SubtractionSolid(geo_apsl1_name, geo_apsl1_box, geo_apsl1_hole, transform_apsl1_hole);

        APSL1.geo = geo_apsl1;

        // define logical volume
        string strMat_apsl1 = m_config.getParameterStr(prep + "Material");
        G4Material* mat_apsl1 = Materials::get(strMat_apsl1);
        string logi_apsl1_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_apsl1 = new G4LogicalVolume(geo_apsl1, mat_apsl1, logi_apsl1_name);
        setColor(*logi_apsl1, "#00CC00");
        setVisibility(*logi_apsl1, false);

        APSL1.logi = logi_apsl1;

        //put volume
        string phys_apsl1_name = "phys_" + name + "_name";
        new G4PVPlacement(APSL1.transform, APSL1.logi, phys_apsl1_name, &topVolume, false, 0);

        // elements[name] = APSL1;

        //--------------
        // Part 2

        FarBeamLineElement APSL2;

        //get parameters from .xml file
        name = "APSL2";
        prep = name + ".";

        double apsl2_W = m_config.getParameter(prep + "W") * unitFactor;
        double apsl2_H = m_config.getParameter(prep + "H") * unitFactor;
        double apsl2_L = m_config.getParameter(prep + "L") * unitFactor;
        double apsl2_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double apsl2_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double apsl2_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double apsl2_hole_R = m_config.getParameter(prep + "holeR") * unitFactor;
        double apsl2_hole_D = m_config.getParameter(prep + "holeD") * unitFactor;
        double apsl2_hole_dX = m_config.getParameter(prep + "holeDX") * unitFactor;
        double apsl2_hole_dY = m_config.getParameter(prep + "holeDY") * unitFactor;
        double apsl2_hole_dZ = m_config.getParameter(prep + "holeDZ") * unitFactor;
        double apsl2_PHI = m_config.getParameter(prep + "PHI");

        // transformations
        APSL2.transform = G4Translate3D(apsl2_X0, apsl2_Y0, apsl2_Z0);
        APSL2.transform = APSL2.transform * G4RotateY3D(apsl2_PHI / Unit::rad);;

        G4Transform3D transform_apsl2_hole = G4Translate3D(apsl2_hole_dX, apsl2_hole_dY, apsl2_hole_dZ);

        //define geometry
        string geo_apsl2_box_name = "geo_" + name + "_box_name";
        string geo_apsl2_hole_name = "geo_" + name + "_hole_name";
        string geo_apsl2_name = "geo_" + name + "_name";

        G4Box* geo_apsl2_box = new G4Box(geo_apsl2_box_name, apsl2_W / 2.0, apsl2_H / 2.0, apsl2_L / 2.0);
        G4Tubs* geo_apsl2_hole = new G4Tubs(geo_apsl2_hole_name, 0.0, apsl2_hole_R, apsl2_hole_D / 2.0, 0.0, 2.0 * M_PI);
        G4SubtractionSolid* geo_apsl2 = new G4SubtractionSolid(geo_apsl2_name, geo_apsl2_box, geo_apsl2_hole, transform_apsl2_hole);

        APSL2.geo = geo_apsl2;

        // define logical volume
        string strMat_apsl2 = m_config.getParameterStr(prep + "Material");
        G4Material* mat_apsl2 = Materials::get(strMat_apsl2);
        string logi_apsl2_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_apsl2 = new G4LogicalVolume(geo_apsl2, mat_apsl2, logi_apsl2_name);
        setColor(*logi_apsl2, "#00CC00");
        setVisibility(*logi_apsl2, false);

        APSL2.logi = logi_apsl2;

        //put volume
        string phys_apsl2_name = "phys_" + name + "_name";
        new G4PVPlacement(APSL2.transform, APSL2.logi, phys_apsl2_name, &topVolume, false, 0);

        // elements[name] = APSL2;


        //--------------
        //-   Additional polyethilene shield right (forward)
        //-   Fills gap between PolyShield and QCS

        // Consists of two disjoined parts
        // Part 1

        FarBeamLineElement APSR1;

        //get parameters from .xml file
        name = "APSR1";
        prep = name + ".";

        double apsr1_W = m_config.getParameter(prep + "W") * unitFactor;
        double apsr1_H = m_config.getParameter(prep + "H") * unitFactor;
        double apsr1_L = m_config.getParameter(prep + "L") * unitFactor;
        double apsr1_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double apsr1_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double apsr1_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double apsr1_hole_R = m_config.getParameter(prep + "holeR") * unitFactor;
        double apsr1_hole_D = m_config.getParameter(prep + "holeD") * unitFactor;
        double apsr1_hole_dX = m_config.getParameter(prep + "holeDX") * unitFactor;
        double apsr1_hole_dY = m_config.getParameter(prep + "holeDY") * unitFactor;
        double apsr1_hole_dZ = m_config.getParameter(prep + "holeDZ") * unitFactor;
        double apsr1_PHI = m_config.getParameter(prep + "PHI");

        // transformations
        APSR1.transform = G4Translate3D(apsr1_X0, apsr1_Y0, apsr1_Z0);
        APSR1.transform = APSR1.transform * G4RotateY3D(apsr1_PHI / Unit::rad);;

        G4Transform3D transform_apsr1_hole = G4Translate3D(apsr1_hole_dX, apsr1_hole_dY, apsr1_hole_dZ);

        //define geometry
        string geo_apsr1_box_name = "geo_" + name + "_box_name";
        string geo_apsr1_hole_name = "geo_" + name + "_hole_name";
        string geo_apsr1_name = "geo_" + name + "_name";

        G4Box* geo_apsr1_box = new G4Box(geo_apsr1_box_name, apsr1_W / 2.0, apsr1_H / 2.0, apsr1_L / 2.0);
        G4Tubs* geo_apsr1_hole = new G4Tubs(geo_apsr1_hole_name, 0.0, apsr1_hole_R, apsr1_hole_D / 2.0, 0.0, 2.0 * M_PI);
        G4SubtractionSolid* geo_apsr1 = new G4SubtractionSolid(geo_apsr1_name, geo_apsr1_box, geo_apsr1_hole, transform_apsr1_hole);

        APSR1.geo = geo_apsr1;

        // define logical volume
        string strMat_apsr1 = m_config.getParameterStr(prep + "Material");
        G4Material* mat_apsr1 = Materials::get(strMat_apsr1);
        string logi_apsr1_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_apsr1 = new G4LogicalVolume(geo_apsr1, mat_apsr1, logi_apsr1_name);
        setColor(*logi_apsr1, "#00CC00");
        setVisibility(*logi_apsr1, false);

        APSR1.logi = logi_apsr1;

        //put volume
        string phys_apsr1_name = "phys_" + name + "_name";
        new G4PVPlacement(APSR1.transform, APSR1.logi, phys_apsr1_name, &topVolume, false, 0);

        // elements[name] = APSR1;

        //--------------
        // Part 2

        FarBeamLineElement APSR2;

        //get parameters from .xml file
        name = "APSR2";
        prep = name + ".";

        double apsr2_W = m_config.getParameter(prep + "W") * unitFactor;
        double apsr2_H = m_config.getParameter(prep + "H") * unitFactor;
        double apsr2_L = m_config.getParameter(prep + "L") * unitFactor;
        double apsr2_X0 = m_config.getParameter(prep + "X0") * unitFactor;
        double apsr2_Y0 = m_config.getParameter(prep + "Y0") * unitFactor;
        double apsr2_Z0 = m_config.getParameter(prep + "Z0") * unitFactor;
        double apsr2_hole_R = m_config.getParameter(prep + "holeR") * unitFactor;
        double apsr2_hole_D = m_config.getParameter(prep + "holeD") * unitFactor;
        double apsr2_hole_dX = m_config.getParameter(prep + "holeDX") * unitFactor;
        double apsr2_hole_dY = m_config.getParameter(prep + "holeDY") * unitFactor;
        double apsr2_hole_dZ = m_config.getParameter(prep + "holeDZ") * unitFactor;
        double apsr2_PHI = m_config.getParameter(prep + "PHI");

        // transformations
        APSR2.transform = G4Translate3D(apsr2_X0, apsr2_Y0, apsr2_Z0);
        APSR2.transform = APSR2.transform * G4RotateY3D(apsr2_PHI / Unit::rad);;

        G4Transform3D transform_apsr2_hole = G4Translate3D(apsr2_hole_dX, apsr2_hole_dY, apsr2_hole_dZ);

        //define geometry
        string geo_apsr2_box_name = "geo_" + name + "_box_name";
        string geo_apsr2_hole_name = "geo_" + name + "_hole_name";
        string geo_apsr2_name = "geo_" + name + "_name";

        G4Box* geo_apsr2_box = new G4Box(geo_apsr2_box_name, apsr2_W / 2.0, apsr2_H / 2.0, apsr2_L / 2.0);
        G4Tubs* geo_apsr2_hole = new G4Tubs(geo_apsr2_hole_name, 0.0, apsr2_hole_R, apsr2_hole_D / 2.0, 0.0, 2.0 * M_PI);
        G4SubtractionSolid* geo_apsr2 = new G4SubtractionSolid(geo_apsr2_name, geo_apsr2_box, geo_apsr2_hole, transform_apsr2_hole);

        APSR2.geo = geo_apsr2;

        // define logical volume
        string strMat_apsr2 = m_config.getParameterStr(prep + "Material");
        G4Material* mat_apsr2 = Materials::get(strMat_apsr2);
        string logi_apsr2_name = "logi_" + name + "_name";
        G4LogicalVolume* logi_apsr2 = new G4LogicalVolume(geo_apsr2, mat_apsr2, logi_apsr2_name);
        setColor(*logi_apsr2, "#00CC00");
        setVisibility(*logi_apsr2, false);

        APSR2.logi = logi_apsr2;

        //put volume
        string phys_apsr2_name = "phys_" + name + "_name";
        new G4PVPlacement(APSR2.transform, APSR2.logi, phys_apsr2_name, &topVolume, false, 0);

        // elements[name] = APSR2;


        //---------------------------
        // dose calculation
        //---------------------------
        //if (elements.count("ACSL")) elements["ACSL"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2001));
        //if (elements.count("ACSR1")) elements["ACSR1"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2002));
        //if (elements.count("ACSR2")) elements["ACSR2"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2003));
        //if (elements.count("APSL1")) elements["APSL1"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2004));
        //if (elements.count("APSL2")) elements["APSL2"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2005));
        //if (elements.count("APSR1")) elements["APSR1"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2006));
        //if (elements.count("APSR2")) elements["APSR2"].logi->SetSensitiveDetector(new BkgSensitiveDetector("IR", 2007));
      }
    }
  }
}

