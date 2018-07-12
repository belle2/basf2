/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroshi Nakano, Hiroyuki Nakayama,Yuri Soloviev          *
 *               Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ir/geometry/GeoBeamPipeCreator.h>

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

    geometry::CreatorFactory<GeoBeamPipeCreator> GeoBeamPipeFactory("BeamPipeCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoBeamPipeCreator::GeoBeamPipeCreator()
    {
    }

    GeoBeamPipeCreator::~GeoBeamPipeCreator()
    {
      //Delete all sensitive detectors
      BOOST_FOREACH(SensitiveDetector * sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    void GeoBeamPipeCreator::createGeometry(G4LogicalVolume& topVolume, GeometryTypes)
    {

      //########## Index ##########
      //
      // "IP pipe" -9.0 cm to 13.85 cm
      // Lv1SUS
      //    Lv2OutBe
      //    Lv2InBe
      //    Lv2Paraf
      //    Lv2Vacuum
      //       Lv3AuCoat
      // "crotch pipe Forward " 13.85 cm to 40.0 cm
      // Lv1TaFwd
      //    Lv2VacFwd
      // "crotch pipe Backward " -9.0 cm to -40.0 cm
      // Lv1TaBwd
      //    Lv2VacBwd
      // "beam pipe more Forward " 40.0 cm to 63.35 cm
      // Lv1TaLERUp
      //    Lv2VacLERUp
      // Lv1TaHERDwn
      //    Lv2VacHERDwn
      // "beam pipe more Backward " -40.0 cm to -62.725 cm
      // Lv1TaHERUp
      //    Lv2VacHERUp
      // Lv1TaLERDwn
      //    Lv2VacLERDwn
      //
      //###########################

      double SafetyLength = m_config.getParameter("Safety.L1") * Unit::cm / Unit::mm;

      double stepMax = 5.0 * Unit::mm;
      int flag_limitStep = int(m_config.getParameter("LimitStepLength"));

      double A11 = 0.03918;

      ////==========
      ////= IP pipe

      //----------
      //- Lv1SUS

      //get parameters from .xml file
      std::string prep = "Lv1SUS.";
      //
      const int Lv1SUS_num = 21;
      //
      double Lv1SUS_Z[Lv1SUS_num];
      Lv1SUS_Z[0] = 0.0;
      for (int tmpn = 0; tmpn < 8; tmpn++) {
        Lv1SUS_Z[0] -= m_config.getParameter(prep + (format("L%1%") % (tmpn + 1)).str().c_str()) * Unit::cm / Unit::mm;
      }
      Lv1SUS_Z[1] = Lv1SUS_Z[0] + m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv1SUS_Z[2] = Lv1SUS_Z[1];
      Lv1SUS_Z[3] = Lv1SUS_Z[2] + m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      Lv1SUS_Z[4] = Lv1SUS_Z[3] + m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      Lv1SUS_Z[5] = Lv1SUS_Z[4];
      Lv1SUS_Z[6] = Lv1SUS_Z[5] + m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      Lv1SUS_Z[7] = Lv1SUS_Z[6] + m_config.getParameter(prep + "L5") * Unit::cm / Unit::mm;
      Lv1SUS_Z[8] = Lv1SUS_Z[7] + m_config.getParameter(prep + "L6") * Unit::cm / Unit::mm;
      Lv1SUS_Z[9] = Lv1SUS_Z[8] + m_config.getParameter(prep + "L7") * Unit::cm / Unit::mm;
      Lv1SUS_Z[10] = Lv1SUS_Z[9] + m_config.getParameter(prep + "L8") * Unit::cm / Unit::mm;
      Lv1SUS_Z[11] = Lv1SUS_Z[10] + m_config.getParameter(prep + "L9") * Unit::cm / Unit::mm;
      Lv1SUS_Z[12] = Lv1SUS_Z[11] + m_config.getParameter(prep + "L10") * Unit::cm / Unit::mm;
      Lv1SUS_Z[13] = Lv1SUS_Z[12] + m_config.getParameter(prep + "L11") * Unit::cm / Unit::mm;
      Lv1SUS_Z[14] = Lv1SUS_Z[13] + m_config.getParameter(prep + "L12") * Unit::cm / Unit::mm;
      Lv1SUS_Z[15] = Lv1SUS_Z[14] + m_config.getParameter(prep + "L13") * Unit::cm / Unit::mm;
      Lv1SUS_Z[16] = Lv1SUS_Z[15];
      Lv1SUS_Z[17] = Lv1SUS_Z[16] + m_config.getParameter(prep + "L14") * Unit::cm / Unit::mm;
      Lv1SUS_Z[18] = Lv1SUS_Z[17] + m_config.getParameter(prep + "L15") * Unit::cm / Unit::mm;
      Lv1SUS_Z[19] = Lv1SUS_Z[18];
      Lv1SUS_Z[20] = Lv1SUS_Z[19] + m_config.getParameter(prep + "L16") * Unit::cm / Unit::mm;
      //
      double Lv1SUS_rI[Lv1SUS_num];
      for (int tmpn = 0; tmpn < Lv1SUS_num; tmpn++)
      { Lv1SUS_rI[tmpn] = 0.0; }
      //m_config.getParameter(prep+"L1")
      double Lv1SUS_rO[Lv1SUS_num];
      Lv1SUS_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv1SUS_rO[1] = Lv1SUS_rO[0];
      Lv1SUS_rO[2] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv1SUS_rO[3] = Lv1SUS_rO[2];
      Lv1SUS_rO[4] = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      Lv1SUS_rO[5] = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      Lv1SUS_rO[6] = Lv1SUS_rO[5];
      Lv1SUS_rO[7] = m_config.getParameter(prep + "R5") * Unit::cm / Unit::mm;
      Lv1SUS_rO[8] = Lv1SUS_rO[7];
      Lv1SUS_rO[9] = m_config.getParameter(prep + "R6") * Unit::cm / Unit::mm;
      Lv1SUS_rO[10] = Lv1SUS_rO[9];
      Lv1SUS_rO[11] = Lv1SUS_rO[10];
      Lv1SUS_rO[12] = m_config.getParameter(prep + "R7") * Unit::cm / Unit::mm;
      Lv1SUS_rO[13] = Lv1SUS_rO[12];
      Lv1SUS_rO[14] = m_config.getParameter(prep + "R8") * Unit::cm / Unit::mm;
      Lv1SUS_rO[15] = Lv1SUS_rO[14];
      Lv1SUS_rO[16] = m_config.getParameter(prep + "R9") * Unit::cm / Unit::mm;
      Lv1SUS_rO[17] = m_config.getParameter(prep + "R10") * Unit::cm / Unit::mm;
      Lv1SUS_rO[18] = Lv1SUS_rO[17];
      Lv1SUS_rO[19] = m_config.getParameter(prep + "R11") * Unit::cm / Unit::mm;
      Lv1SUS_rO[20] = Lv1SUS_rO[19];
      //
      string strMat_Lv1SUS = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1SUS = Materials::get(strMat_Lv1SUS);

      //define geometry
      G4Polycone* geo_Lv1SUS = new G4Polycone("geo_Lv1SUS_name", 0, 2 * M_PI, Lv1SUS_num, Lv1SUS_Z, Lv1SUS_rI, Lv1SUS_rO);
      G4LogicalVolume* logi_Lv1SUS = new G4LogicalVolume(geo_Lv1SUS, mat_Lv1SUS, "logi_Lv1SUS_name");

      //for (int i=0;i<Lv1SUS_num;i++)printf("%f %f\n",Lv1SUS_Z[i],Lv1SUS_rO[i]);

      //-   put volume
      setColor(*logi_Lv1SUS, "#666666");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv1SUS, "phys_Lv1SUS_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2OutBe

      //get parameters from .xml file
      prep = "Lv2OutBe.";
      //
      const int Lv2OutBe_num = 2;
      //
      double Lv2OutBe_Z[Lv2OutBe_num];
      Lv2OutBe_Z[0] = -m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv2OutBe_Z[1] = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //
      double Lv2OutBe_rI[Lv2OutBe_num];
      Lv2OutBe_rI[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2OutBe_rI[1] = Lv2OutBe_rI[0];
      //
      double Lv2OutBe_rO[Lv2OutBe_num];
      Lv2OutBe_rO[0] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv2OutBe_rO[1] = Lv2OutBe_rO[0];
      //
      string strMat_Lv2OutBe =  m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2OutBe = Materials::get(strMat_Lv2OutBe);

      //define geometry
      G4Polycone* geo_Lv2OutBe = new G4Polycone("geo_Lv2OutBe_name", 0, 2 * M_PI, Lv2OutBe_num, Lv2OutBe_Z, Lv2OutBe_rI, Lv2OutBe_rO);
      G4LogicalVolume* logi_Lv2OutBe = new G4LogicalVolume(geo_Lv2OutBe, mat_Lv2OutBe, "logi_Lv2OutBe_name");

      //-   put volume
      setColor(*logi_Lv2OutBe, "#333300");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2OutBe, "phys_Lv2OutBe_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv2InBe
      //-
      //----------

      //get parameters from .xml file
      prep = "Lv2InBe.";
      //
      const int Lv2InBe_num = 2;
      //
      double Lv2InBe_Z[Lv2InBe_num];
      Lv2InBe_Z[0] = -m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv2InBe_Z[1] = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //
      double Lv2InBe_rI[Lv2InBe_num];
      Lv2InBe_rI[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2InBe_rI[1] = Lv2InBe_rI[0];
      //
      double Lv2InBe_rO[Lv2InBe_num];
      Lv2InBe_rO[0] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv2InBe_rO[1] = Lv2InBe_rO[0];
      //
      string strMat_Lv2InBe = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2InBe = Materials::get(strMat_Lv2InBe);

      //define geometry
      G4Polycone* geo_Lv2InBe = new G4Polycone("geo_Lv2InBe_name", 0, 2 * M_PI, Lv2InBe_num, Lv2InBe_Z, Lv2InBe_rI, Lv2InBe_rO);
      G4LogicalVolume* logi_Lv2InBe = new G4LogicalVolume(geo_Lv2InBe, mat_Lv2InBe, "logi_Lv2InBe_name");

      //-   put volume
      setColor(*logi_Lv2InBe, "#333300");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2InBe, "phys_Lv2InBe_name", logi_Lv1SUS, false, 0);

      //----------
      //- Lv2Vacuum

      //get parameters from .xml file
      prep = "Lv2Vacuum.";
      //
      double Lv2Vacuum_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double Lv2Vacuum_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      double Lv2Vacuum_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv2Vacuum_L4 = m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      double Lv2Vacuum_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double Lv2Vacuum_R2 = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      double Lv2Vacuum_R3 = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      //double Lv2Vacuum_A1 = cLv2Vacuum.getAngle("A1");
      double Lv2Vacuum_A2 = m_config.getParameter(prep + "A2");
      //
      string strMat_Lv2Vacuum = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2Vacuum = Materials::get(strMat_Lv2Vacuum);
      //
      // Part 1
      const int Lv2Vacuum1_num = 2;
      //
      double Lv2Vacuum1_Z[Lv2Vacuum1_num];
      Lv2Vacuum1_Z[0] = -Lv2Vacuum_L1;
      Lv2Vacuum1_Z[1] = Lv2Vacuum_L2;
      double Lv2Vacuum1_rI[Lv2Vacuum1_num];
      for (int tmpn = 0; tmpn < Lv2Vacuum1_num; tmpn++)
      { Lv2Vacuum1_rI[tmpn] = 0.0; }
      double Lv2Vacuum1_rO[Lv2Vacuum1_num];
      Lv2Vacuum1_rO[0] = Lv2Vacuum_R1;
      Lv2Vacuum1_rO[1] = Lv2Vacuum_R1;
      // Part 2
      double Lv2Vacuum2_Z1  = 2. * Lv2Vacuum_L3;
      double Lv2Vacuum2_rI1 = 0.0;
      double Lv2Vacuum2_rO1 = Lv2Vacuum_R2;
      //
      double Lv2Vacuum2_Z2  = (Lv2Vacuum_L3 * cos(Lv2Vacuum_A2)) + SafetyLength;
      double Lv2Vacuum2_rI2 = 0.0;
      double Lv2Vacuum2_rO2 = 2 * Lv2Vacuum_R2;
      // Part 3
      const int Lv2Vacuum3_num = 2;
      //
      double Lv2Vacuum3_Z[Lv2Vacuum3_num];
      Lv2Vacuum3_Z[0] = 0.0;
      Lv2Vacuum3_Z[1] = Lv2Vacuum_L4;
      double Lv2Vacuum3_rI[Lv2Vacuum3_num];
      for (int tmpn = 0; tmpn < Lv2Vacuum3_num; tmpn++)
      { Lv2Vacuum3_rI[tmpn] = 0.0; }
      double Lv2Vacuum3_rO[Lv2Vacuum3_num];
      Lv2Vacuum3_rO[0] = Lv2Vacuum_R3;
      Lv2Vacuum3_rO[1] = Lv2Vacuum_R3;

      //define geometry
      // Part 1
      G4Polycone* geo_Lv2VacuumPart1 = new G4Polycone("geo_Lv2VacuumPart1_name", 0, 2 * M_PI, Lv2Vacuum1_num, Lv2Vacuum1_Z, Lv2Vacuum1_rI,
                                                      Lv2Vacuum1_rO);
      // Part 2
      G4Tubs* geo_Lv2VacuumPart2_1 = new G4Tubs("geo_Lv2VacuumPart2_1_name", Lv2Vacuum2_rI1, Lv2Vacuum2_rO1, Lv2Vacuum2_Z1, 0, 2 * M_PI);
      G4Tubs* geo_Lv2VacuumPart2_2 = new G4Tubs("geo_Lv2VacuumPart2_2_name", Lv2Vacuum2_rI2, Lv2Vacuum2_rO2, Lv2Vacuum2_Z2, 0, 2 * M_PI);
      G4Transform3D transform_Lv2VacuumPart2_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacuumPart2_2 = transform_Lv2VacuumPart2_2 * G4RotateY3D(-Lv2Vacuum_A2);
      G4IntersectionSolid* geo_Lv2VacuumPart2 = new G4IntersectionSolid("geo_Lv2VacuumPart2_name", geo_Lv2VacuumPart2_1,
          geo_Lv2VacuumPart2_2, transform_Lv2VacuumPart2_2);
      // Part 3
      G4Polycone* geo_Lv2VacuumPart3 = new G4Polycone("geo_Lv2VacuumPart3_name", 0, 2 * M_PI, Lv2Vacuum3_num, Lv2Vacuum3_Z, Lv2Vacuum3_rI,
                                                      Lv2Vacuum3_rO);
      // Part1+2+3
//      G4Transform3D transform_Lv2VacuumPart3 = G4Translate3D(0., 0., 0.);
      G4Transform3D transform_Lv2VacuumPart3 = G4Translate3D(-0.5, 0., 0.);
      // A11 instead of A1
      transform_Lv2VacuumPart3 = transform_Lv2VacuumPart3 * G4RotateY3D(-A11);
      G4UnionSolid* geo_Lv2Vacuumxx = new G4UnionSolid("geo_Lv2Vacuumxx_name", geo_Lv2VacuumPart1, geo_Lv2VacuumPart3,
                                                       transform_Lv2VacuumPart3);
      G4Transform3D transform_Lv2VacuumPart2 = G4Translate3D(Lv2Vacuum_L3 * sin(Lv2Vacuum_A2), 0.,
                                                             Lv2Vacuum_L2 + Lv2Vacuum_L3 * cos(Lv2Vacuum_A2));
      transform_Lv2VacuumPart2 = transform_Lv2VacuumPart2 * G4RotateY3D(Lv2Vacuum_A2);
      G4UnionSolid* geo_Lv2Vacuumx = new G4UnionSolid("geo_Lv2Vacuumx_name", geo_Lv2Vacuumxx, geo_Lv2VacuumPart2,
                                                      transform_Lv2VacuumPart2);
      // Intersection with mother
      G4IntersectionSolid* geo_Lv2Vacuum = new G4IntersectionSolid("geo_Lv2Vacuum_name", geo_Lv2Vacuumx, geo_Lv1SUS);
      G4LogicalVolume* logi_Lv2Vacuum = new G4LogicalVolume(geo_Lv2Vacuum, mat_Lv2Vacuum, "logi_Lv2Vacuum_name");
      if (flag_limitStep) logi_Lv2Vacuum->SetUserLimits(new G4UserLimits(stepMax));

      //-   put volume
      setColor(*logi_Lv2Vacuum, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2Vacuum, "phys_Lv2Vacuum_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv2Paraf

      //get parameters from .xml file
      prep = "Lv2Paraf.";
      //
      const int Lv2Paraf1_num = 20;
      const int Lv2Paraf2_num = 3;
      //
      double Lv2Paraf1_Z[Lv2Paraf1_num];
      Lv2Paraf1_Z[0] = 0.0;
      for (int tmpn = 0; tmpn < 9; tmpn++) {
        Lv2Paraf1_Z[0] -= m_config.getParameter(prep + (format("L%1%") % (tmpn + 1)).str().c_str()) * Unit::cm / Unit::mm;
      }
      Lv2Paraf1_Z[1] = Lv2Paraf1_Z[0] + m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[2] = Lv2Paraf1_Z[1];
      Lv2Paraf1_Z[3] = Lv2Paraf1_Z[2] + m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[4] = Lv2Paraf1_Z[3] + m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[5] = Lv2Paraf1_Z[4] + m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[6] = Lv2Paraf1_Z[5] + m_config.getParameter(prep + "L5") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[7] = Lv2Paraf1_Z[6] + m_config.getParameter(prep + "L6") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[8] = Lv2Paraf1_Z[7] + m_config.getParameter(prep + "L7") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[9] = Lv2Paraf1_Z[8] + m_config.getParameter(prep + "L8") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[10] = Lv2Paraf1_Z[9] + m_config.getParameter(prep + "L9") * Unit::cm / Unit::mm + m_config.getParameter(
                          prep + "L10") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[11] = Lv2Paraf1_Z[10] + m_config.getParameter(prep + "L11") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[12] = Lv2Paraf1_Z[11] + m_config.getParameter(prep + "L12") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[13] = Lv2Paraf1_Z[12] + m_config.getParameter(prep + "L13") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[14] = Lv2Paraf1_Z[13] + m_config.getParameter(prep + "L14") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[15] = Lv2Paraf1_Z[14] + m_config.getParameter(prep + "L15") * Unit::cm / Unit::mm + m_config.getParameter(
                          prep + "L16") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[16] = Lv2Paraf1_Z[15] + m_config.getParameter(prep + "L17") * Unit::cm / Unit::mm + m_config.getParameter(
                          prep + "L18") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[17] = Lv2Paraf1_Z[16] + m_config.getParameter(prep + "L19") * Unit::cm / Unit::mm;
      Lv2Paraf1_Z[18] = Lv2Paraf1_Z[17];
      Lv2Paraf1_Z[19] = Lv2Paraf1_Z[18] + m_config.getParameter(prep + "L20") * Unit::cm / Unit::mm;
      //
      double Lv2Paraf1_rI[Lv2Paraf1_num];
      Lv2Paraf1_rI[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2Paraf1_rI[1] = Lv2Paraf1_rI[0];
      Lv2Paraf1_rI[2] = Lv2Paraf1_rI[1];
      Lv2Paraf1_rI[3] = Lv2Paraf1_rI[2];
      Lv2Paraf1_rI[4] = Lv2Paraf1_rI[3];
      Lv2Paraf1_rI[5] = Lv2Paraf1_rI[4];
      Lv2Paraf1_rI[6] = Lv2Paraf1_rI[5];
      Lv2Paraf1_rI[7] = m_config.getParameter(prep + "R6") * Unit::cm / Unit::mm;
      Lv2Paraf1_rI[8] = Lv2Paraf1_rI[7];
      Lv2Paraf1_rI[9] = Lv2Paraf1_rI[8];
      Lv2Paraf1_rI[10] = Lv2Paraf1_rI[9];
      Lv2Paraf1_rI[11] = Lv2Paraf1_rI[10];
      Lv2Paraf1_rI[12] = Lv2Paraf1_rI[11];
      Lv2Paraf1_rI[13] = m_config.getParameter(prep + "R9") * Unit::cm / Unit::mm;
      Lv2Paraf1_rI[14] = Lv2Paraf1_rI[13];
      Lv2Paraf1_rI[15] = Lv2Paraf1_rI[14];
      Lv2Paraf1_rI[16] = Lv2Paraf1_rI[15];
      Lv2Paraf1_rI[17] = Lv2Paraf1_rI[16];
      Lv2Paraf1_rI[18] = Lv2Paraf1_rI[17];
      Lv2Paraf1_rI[19] = Lv2Paraf1_rI[18];
      //
      double Lv2Paraf1_rO[Lv2Paraf1_num];
      Lv2Paraf1_rO[0] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[1] = Lv2Paraf1_rO[0];
      Lv2Paraf1_rO[2] = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[3] = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[4] = Lv2Paraf1_rO[3];
      Lv2Paraf1_rO[5] = m_config.getParameter(prep + "R5") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[6] = Lv2Paraf1_rO[5];
      Lv2Paraf1_rO[7] = Lv2Paraf1_rO[6];
      Lv2Paraf1_rO[8] = Lv2Paraf1_rO[7];
      Lv2Paraf1_rO[9] = m_config.getParameter(prep + "R7") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[10] = Lv2Paraf1_rO[9];
      Lv2Paraf1_rO[11] = m_config.getParameter(prep + "R8") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[12] = Lv2Paraf1_rO[11];
      Lv2Paraf1_rO[13] = Lv2Paraf1_rO[12];
      Lv2Paraf1_rO[14] = Lv2Paraf1_rO[13];
      Lv2Paraf1_rO[15] = m_config.getParameter(prep + "R10") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[16] = Lv2Paraf1_rO[15];
      Lv2Paraf1_rO[17] = m_config.getParameter(prep + "R12") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[18] = m_config.getParameter(prep + "R13") * Unit::cm / Unit::mm;
      Lv2Paraf1_rO[19] = Lv2Paraf1_rO[18];
      //
      //
      double Lv2Paraf2_Z[Lv2Paraf2_num];
      Lv2Paraf2_Z[0] = 0.0;
      for (int tmpn = 10; tmpn <= 15; tmpn++) {
        Lv2Paraf2_Z[0] += m_config.getParameter(prep + (format("L%1%") % tmpn).str().c_str()) * Unit::cm / Unit::mm;
      }
      Lv2Paraf2_Z[1] = Lv2Paraf2_Z[0] + m_config.getParameter(prep + "L16") * Unit::cm / Unit::mm + m_config.getParameter(
                         prep + "L17") * Unit::cm / Unit::mm;
      Lv2Paraf2_Z[2] = Lv2Paraf2_Z[1] + m_config.getParameter(prep + "L18") * Unit::cm / Unit::mm + m_config.getParameter(
                         prep + "L19") * Unit::cm / Unit::mm +
                       m_config.getParameter(prep + "L20") * Unit::cm / Unit::mm + 1.0;
      //
      double Lv2Paraf2_rI[Lv2Paraf2_num];
      for (int tmpn = 0; tmpn < Lv2Paraf2_num; tmpn++)
      { Lv2Paraf2_rI[tmpn] = 0.0; }
      //
      double Lv2Paraf2_rO[Lv2Paraf2_num];
      Lv2Paraf2_rO[0] = m_config.getParameter(prep + "R9") * Unit::cm / Unit::mm;
      Lv2Paraf2_rO[1] = m_config.getParameter(prep + "R11") * Unit::cm / Unit::mm;
      Lv2Paraf2_rO[2] = Lv2Paraf2_rO[1];
      //
      string strMat_Lv2Paraf = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2Paraf = Materials::get(strMat_Lv2Paraf);

      //define geometry
      G4Polycone* geo_Lv2Parafpcon1 = new G4Polycone("geo_Lv2Parafpcon1_name", 0, 2 * M_PI, Lv2Paraf1_num, Lv2Paraf1_Z, Lv2Paraf1_rI,
                                                     Lv2Paraf1_rO);
      G4Polycone* geo_Lv2Parafpcon2 = new G4Polycone("geo_Lv2Parafpcon2_name", 0, 2 * M_PI, Lv2Paraf2_num, Lv2Paraf2_Z, Lv2Paraf2_rI,
                                                     Lv2Paraf2_rO);
      G4SubtractionSolid* geo_Lv2Paraf = new G4SubtractionSolid("geo_Lv2Paraf_name", geo_Lv2Parafpcon1, geo_Lv2Parafpcon2);
      G4LogicalVolume* logi_Lv2Paraf = new G4LogicalVolume(geo_Lv2Paraf, mat_Lv2Paraf, "logi_Lv2Paraf_name");

      //-   put volume
      setColor(*logi_Lv2Paraf, "#00CCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2Paraf, "phys_Lv2Paraf_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv3AuCoat

      //get parameters from .xml file
      prep = "Lv3AuCoat.";
      //
      const int Lv3AuCoat_num = 2;
      //
      double Lv3AuCoat_Z[Lv3AuCoat_num];
      Lv3AuCoat_Z[0] = -m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv3AuCoat_Z[1] = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //
      double Lv3AuCoat_rI[Lv3AuCoat_num];
      Lv3AuCoat_rI[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv3AuCoat_rI[1] = Lv3AuCoat_rI[0];
      //
      double Lv3AuCoat_rO[Lv3AuCoat_num];
      Lv3AuCoat_rO[0] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv3AuCoat_rO[1] = Lv3AuCoat_rO[0];
      //
      string strMat_Lv3AuCoat = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv3AuCoat = Materials::get(strMat_Lv3AuCoat);

      //define geometry
      G4Polycone* geo_Lv3AuCoat = new G4Polycone("geo_Lv3AuCoat_name", 0, 2 * M_PI, Lv3AuCoat_num, Lv3AuCoat_Z, Lv3AuCoat_rI,
                                                 Lv3AuCoat_rO);
      G4LogicalVolume* logi_Lv3AuCoat = new G4LogicalVolume(geo_Lv3AuCoat, mat_Lv3AuCoat, "logi_Lv3AuCoat_name");

      //-   put volume
      setColor(*logi_Lv3AuCoat, "#CCCC00");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv3AuCoat, "phys_Lv3AuCoat_name", logi_Lv2Vacuum, false, 0);

      //-
      //----------

      ////==============
      ////= flanges

      //get parameters from .xml file
      prep =  "Flange.";
      //
      double Flange_R  = m_config.getParameter(prep + "R") * Unit::cm / Unit::mm;
      double Flange_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      //double Flange_L2 = m_config.getParameter(prep+"L2") * Unit::cm / Unit::mm;// Not used (2015/April/16. masked by T.Hara)
      double Flange_D  = m_config.getParameter(prep + "D") * Unit::cm / Unit::mm;
      double Flange_T  = m_config.getParameter(prep + "T") * Unit::cm / Unit::mm;

      //define geometry
      //G4Box* geo_Flange0 = new G4Box("geo_Flange0_name", Flange_L2, Flange_R, Flange_T);
      G4Tubs* geo_Flange0 = new G4Tubs("geo_Flange0_name", 0, 424 * Unit::mm, Flange_T, 0, 2 * M_PI);
      G4Tubs* geo_Flange1 = new G4Tubs("geo_Flange1_name", 0, Flange_R, Flange_T, 0, 2 * M_PI);
      G4Tubs* geo_Flange2 = new G4Tubs("geo_Flange2_name", 0, Flange_R, Flange_T, 0, 2 * M_PI);

      G4UnionSolid* geo_Flange_x = new G4UnionSolid("geo_Flange_x_name", geo_Flange1, geo_Flange2, G4Translate3D(-Flange_L1 * 2, 0, 0));
      G4IntersectionSolid* geo_Flange = new G4IntersectionSolid("geo_Flange_name", geo_Flange0, geo_Flange_x, G4Translate3D(Flange_L1, 0,
                                                                0));


      ////=
      ////==========

      ////==========
      ////= Ta pipe Forward


      //----------
      //- Lv1TaFwd

      //get parameters from .xml file
      prep = "Lv1TaFwd.";
      //
      double Lv1TaFwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double Lv1TaFwd_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double Lv1TaFwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      double Lv1TaFwd_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv1TaFwd_T1 = m_config.getParameter(prep + "T1") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaFwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaFwd = Materials::get(strMat_Lv1TaFwd);

      //define geometry
      G4Trd* geo_Lv1TaFwd_xx = new G4Trd("geo_Lv1TaFwd_xx_name", Lv1TaFwd_L2, Lv1TaFwd_L3, Lv1TaFwd_T1, Lv1TaFwd_T1, Lv1TaFwd_L1 / 2.0);
      G4UnionSolid* geo_Lv1TaFwd_x
        = new G4UnionSolid("geo_Lv1TaFwd_x_name", geo_Lv1TaFwd_xx, geo_Flange,
                           G4Translate3D(0, 0, Flange_D - (Lv1TaFwd_D1 + Lv1TaFwd_L1 / 2.0)));

      //===MODIFY ME!!===
      //avoid overlap with HeavyMetalShield
      double HMS_Z[4]  = {350, 410, 450, 482};
      double HMS_rI[4] = {35.5, 35.5, 42.5, 42.5};
      double HMS_rO[4] = {100, 100, 100, 100};
      G4Polycone* geo_HMS = new G4Polycone("geo_HMS_name", 0, 2 * M_PI, 4, HMS_Z, HMS_rI, HMS_rO);
      G4SubtractionSolid* geo_Lv1TaFwd = new G4SubtractionSolid("geo_Lv1TaFwd_name", geo_Lv1TaFwd_x, geo_HMS,
                                                                G4Translate3D(0, 0, -(Lv1TaFwd_D1 + Lv1TaFwd_L1 / 2.0)));

      G4LogicalVolume* logi_Lv1TaFwd = new G4LogicalVolume(geo_Lv1TaFwd, mat_Lv1TaFwd, "logi_Lv1TaFwd_name");

      //-   put volume at (0.,0.,D1 + L1/2)
      setColor(*logi_Lv1TaFwd, "#333333");
      new G4PVPlacement(0, G4ThreeVector(0, 0, Lv1TaFwd_D1 + Lv1TaFwd_L1 / 2.0), logi_Lv1TaFwd, "phys_Lv1TaFwd_name", &topVolume, false,
                        0);


      //----------
      //- Lv2VacFwd

      //get parameters from .xml file
      prep = "Lv2VacFwd.";
      //
      double Lv2VacFwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double Lv2VacFwd_D2 = m_config.getParameter(prep + "D2") * Unit::cm / Unit::mm;
      double Lv2VacFwd_D3 = m_config.getParameter(prep + "D3") * Unit::cm / Unit::mm;
      double Lv2VacFwd_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double Lv2VacFwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      double Lv2VacFwd_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv2VacFwd_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double Lv2VacFwd_R2 = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      double Lv2VacFwd_R3 = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      double Lv2VacFwd_R4 = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      double Lv2VacFwd_A1 = m_config.getParameter(prep + "A1");
      double Lv2VacFwd_A2 = m_config.getParameter(prep + "A2");
      //
      string strMat_Lv2VacFwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2VacFwd = Materials::get(strMat_Lv2VacFwd);
      //
      // Part 1
      double Lv2VacFwd1_Z1  = sqrt(Lv2VacFwd_D1 * Lv2VacFwd_D1 + Lv2VacFwd_D2 * Lv2VacFwd_D2 - 2.*Lv2VacFwd_D1 * Lv2VacFwd_D2 * cos(
                                     Lv2VacFwd_A1));
      double Lv2VacFwd1_rI1 = 0.0;
      double Lv2VacFwd1_rO1 = Lv2VacFwd_R1;
      //
      double Lv2VacFwd1_Z2  = (Lv2VacFwd1_Z1 / 2. * cos(Lv2VacFwd_A2)) + SafetyLength;
      double Lv2VacFwd1_rI2 = 0.0;
      double Lv2VacFwd1_rO2 = 2 * Lv2VacFwd_R1;
      // Part 2
      double Lv2VacFwd2_Z1  = Lv2VacFwd_L1;
      double Lv2VacFwd2_rI1 = 0.0;
      double Lv2VacFwd2_rO1 = Lv2VacFwd_R2;
      //
      double Lv2VacFwd2_Z2  = (Lv2VacFwd_L1 / 2. * cos(Lv2VacFwd_A2)) + SafetyLength;
      double Lv2VacFwd2_rI2 = 0.0;
      double Lv2VacFwd2_rO2 = 2 * Lv2VacFwd_R2;
      // Part 3
      const int Lv2VacFwd3_num1 = 2;  // Cylindrical part
      const int Lv2VacFwd3_num2 = 3;  // Policone part
      //-----------> Cylindrical part
      double Lv2VacFwd_Z1[Lv2VacFwd3_num1];
      Lv2VacFwd_Z1[0] = 0.0;
//      Lv2VacFwd_Z1[1] = Lv2VacFwd_D3 - Lv2VacFwd_L2;
      Lv2VacFwd_Z1[1] = Lv2VacFwd_D3 - Lv2VacFwd_L2 + 0.03 * SafetyLength;
      double Lv2VacFwd_rI1[Lv2VacFwd3_num1];
      for (int tmpn = 0; tmpn < Lv2VacFwd3_num1; tmpn++)
      { Lv2VacFwd_rI1[tmpn] = 0.0; }
      double Lv2VacFwd_rO1[Lv2VacFwd3_num1];
      Lv2VacFwd_rO1[0] = Lv2VacFwd_R3;
      Lv2VacFwd_rO1[1] = Lv2VacFwd_R3;
      //<---------------
      //----------->Policone part
      double Lv2VacFwd_Z2[Lv2VacFwd3_num2];
      Lv2VacFwd_Z2[0] = Lv2VacFwd_D3 - Lv2VacFwd_L2;
      Lv2VacFwd_Z2[1] = Lv2VacFwd_D3;
      Lv2VacFwd_Z2[2] = Lv2VacFwd_D3 + Lv2VacFwd_L3;
      double Lv2VacFwd_rI2[Lv2VacFwd3_num2];
      for (int tmpn = 0; tmpn < Lv2VacFwd3_num2; tmpn++)
      { Lv2VacFwd_rI2[tmpn] = 0.0; }
      double Lv2VacFwd_rO2[Lv2VacFwd3_num2];
      Lv2VacFwd_rO2[0] = Lv2VacFwd_R3;
      Lv2VacFwd_rO2[1] = Lv2VacFwd_R4;
      Lv2VacFwd_rO2[2] = Lv2VacFwd_R4;
      //<-------------------
      //

      //define geometry
      // Part 1
      G4Tubs* geo_Lv2VacFwdPart1_1 = new G4Tubs("geo_Lv2VacFwdPart1_1_name", Lv2VacFwd1_rI1, Lv2VacFwd1_rO1, Lv2VacFwd1_Z1, 0, 2 * M_PI);
      G4Tubs* geo_Lv2VacFwdPart1_2 = new G4Tubs("geo_Lv2VacFwdPart1_2_name", Lv2VacFwd1_rI2, Lv2VacFwd1_rO2, Lv2VacFwd1_Z2, 0, 2 * M_PI);
      //Slanted tube of Part 1
      G4Transform3D transform_Lv2VacFwdPart1_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacFwdPart1_2 = transform_Lv2VacFwdPart1_2 * G4RotateY3D(-Lv2VacFwd_A2 / 2.);
      G4IntersectionSolid* geo_Lv2VacFwdPart1 = new G4IntersectionSolid("geo_Lv2VacFwdPart1_name", geo_Lv2VacFwdPart1_1,
          geo_Lv2VacFwdPart1_2, transform_Lv2VacFwdPart1_2);
      // Part 2
      G4Tubs* geo_Lv2VacFwdPart2_1 = new G4Tubs("geo_Lv2VacFwdPart2_1_name", Lv2VacFwd2_rI1, Lv2VacFwd2_rO1, Lv2VacFwd2_Z1, 0, 2 * M_PI);
      G4Tubs* geo_Lv2VacFwdPart2_2 = new G4Tubs("geo_Lv2VacFwdPart2_2_name", Lv2VacFwd2_rI2, Lv2VacFwd2_rO2, Lv2VacFwd2_Z2, 0, 2 * M_PI);
      //Slanted tube of Part 2
      G4Transform3D transform_Lv2VacFwdPart2_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacFwdPart2_2 = transform_Lv2VacFwdPart2_2 * G4RotateY3D(Lv2VacFwd_A2 / 2.);
      G4IntersectionSolid* geo_Lv2VacFwdPart2 = new G4IntersectionSolid("geo_Lv2VacFwdPart2_name", geo_Lv2VacFwdPart2_1,
          geo_Lv2VacFwdPart2_2, transform_Lv2VacFwdPart2_2);
      // Part 3
//      G4Polycone* geo_Lv2VacFwdPart3 = new G4Polycone("geo_Lv2VacFwdPart3", 0, 2 * M_PI, Lv2VacFwd3_num, Lv2VacFwd_Z, Lv2VacFwd_rI, Lv2VacFwd_rO);
      G4Polycone* geo_Lv2VacFwdPart3_1 = new G4Polycone("geo_Lv2VacFwdPart3_1", 0, 2 * M_PI, Lv2VacFwd3_num1, Lv2VacFwd_Z1, Lv2VacFwd_rI1,
                                                        Lv2VacFwd_rO1);
      G4Polycone* geo_Lv2VacFwdPart3_2 = new G4Polycone("geo_Lv2VacFwdPart3_2", 0, 2 * M_PI, Lv2VacFwd3_num2, Lv2VacFwd_Z2, Lv2VacFwd_rI2,
                                                        Lv2VacFwd_rO2);
      G4Transform3D transform_Lv2VacFwdPart3_1 = G4Translate3D(-0.5, 0., 0.);
      transform_Lv2VacFwdPart3_1 = transform_Lv2VacFwdPart3_1  * G4RotateY3D(Lv2VacFwd_A1 - A11);
      G4UnionSolid* geo_Lv2VacFwdPart3 = new G4UnionSolid("geo_Lv2VacFwdPart3_name", geo_Lv2VacFwdPart3_2, geo_Lv2VacFwdPart3_1,
                                                          transform_Lv2VacFwdPart3_1);
      // Part1+2+3
      //tmp begin
      G4Transform3D transform_Lv2VacFwdPart1 = G4Translate3D((Lv2VacFwd_D1 * sin(Lv2VacFwd_A1) + Lv2VacFwd_D2 * sin(
                                                                2.*Lv2VacFwd_A1)) / 2. ,
                                                             0.,
                                                             (Lv2VacFwd_D1 * cos(Lv2VacFwd_A1) + Lv2VacFwd_D2 * cos(2.*Lv2VacFwd_A1)) / 2.);
      //G4Transform3D transform_Lv2VacFwdPart1 = G4Translate3D((Lv2VacFwd_D1 * sin(Lv2VacFwd_A1) + Lv2VacFwd_D2 * sin(2.*Lv2VacFwd_A1)) / 1.9 ,
      //                                                       0.,
      //                                                       (Lv2VacFwd_D1 * cos(Lv2VacFwd_A1) + Lv2VacFwd_D2 * cos(2.*Lv2VacFwd_A1)) / 1.9);
      transform_Lv2VacFwdPart1 = transform_Lv2VacFwdPart1 * G4RotateY3D(Lv2VacFwd_A1 + Lv2VacFwd_A2);
      //
      G4Transform3D transform_Lv2VacFwdPart2 = G4Translate3D(Lv2VacFwd_D2 * sin(2.*Lv2VacFwd_A1) + Lv2VacFwd_L1 * sin(
                                                               2.*Lv2VacFwd_A1) / 2.0,
                                                             0.,
                                                             Lv2VacFwd_D2 * cos(2.*Lv2VacFwd_A1) + Lv2VacFwd_L1 * cos(2.*Lv2VacFwd_A1) / 2.0);
      //G4Transform3D transform_Lv2VacFwdPart2 = G4Translate3D(Lv2VacFwd_D2 * sin(2.*Lv2VacFwd_A1) + Lv2VacFwd_L1 * sin(2.*Lv2VacFwd_A1) / 2.05,
      //                                                       0.,
      //                                                       Lv2VacFwd_D2 * cos(2.*Lv2VacFwd_A1) + Lv2VacFwd_L1 * cos(2.*Lv2VacFwd_A1) / 2.05);
      //tmp end
      transform_Lv2VacFwdPart2 = transform_Lv2VacFwdPart2 * G4RotateY3D(2.*Lv2VacFwd_A1);
      //
      G4UnionSolid* geo_Lv2VacFwdxx = new G4UnionSolid("geo_Lv2VacFwdxx_name", geo_Lv2VacFwdPart3, geo_Lv2VacFwdPart1,
                                                       transform_Lv2VacFwdPart1);
      G4UnionSolid* geo_Lv2VacFwdx = new G4UnionSolid("geo_Lv2VacFwdx_name", geo_Lv2VacFwdxx, geo_Lv2VacFwdPart2,
                                                      transform_Lv2VacFwdPart2);
      // Intersection
      G4Transform3D transform_Lv2VacFwd = G4Translate3D(0., 0., -Lv1TaFwd_D1 - Lv1TaFwd_L1 / 2.);
      transform_Lv2VacFwd = transform_Lv2VacFwd * G4RotateY3D(-Lv2VacFwd_A1);
      //
      G4IntersectionSolid* geo_Lv2VacFwd = new G4IntersectionSolid("geo_Lv2VacFwd_name", geo_Lv1TaFwd, geo_Lv2VacFwdx,
          transform_Lv2VacFwd);
      G4LogicalVolume* logi_Lv2VacFwd = new G4LogicalVolume(geo_Lv2VacFwd, mat_Lv2VacFwd, "logi_Lv2VacFwd_name");
      if (flag_limitStep) logi_Lv2VacFwd->SetUserLimits(new G4UserLimits(stepMax));

      //-   put volume
      setColor(*logi_Lv2VacFwd, "#CCCCCC");
      //you must set this invisible, otherwise encounter segV.
      setVisibility(*logi_Lv2VacFwd, false);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logi_Lv2VacFwd, "phys_Lv2VacFwd_name", logi_Lv1TaFwd, false, 0);

      //-
      //----------

      ////=
      ////==========


      ////==========
      ////= Ta pipe Backward

      //----------
      //- Lv1TaBwd

      //get parameters from .xml file
      prep = "Lv1TaBwd.";
      //
      double Lv1TaBwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double Lv1TaBwd_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double Lv1TaBwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      double Lv1TaBwd_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv1TaBwd_T1 = m_config.getParameter(prep + "T1") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaBwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaBwd = Materials::get(strMat_Lv1TaBwd);

      //define geometry
      G4Trd* geo_Lv1TaBwd_x = new G4Trd("geo_Lv1TaBwd_x_name", Lv1TaBwd_L2, Lv1TaBwd_L3, Lv1TaBwd_T1, Lv1TaBwd_T1, Lv1TaBwd_L1 / 2.0);
      G4UnionSolid* geo_Lv1TaBwd = new G4UnionSolid("geo_Lv1TaBwd_name", geo_Lv1TaBwd_x, geo_Flange,
                                                    G4Translate3D(0, 0, -Flange_D - (-Lv1TaBwd_D1 - Lv1TaBwd_L1 / 2.0)));
      G4LogicalVolume* logi_Lv1TaBwd = new G4LogicalVolume(geo_Lv1TaBwd, mat_Lv1TaBwd, "logi_Lv1TaBwd_name");

      //-   put volume
      setColor(*logi_Lv1TaBwd, "#333333");
      new G4PVPlacement(0, G4ThreeVector(0, 0, -Lv1TaBwd_D1 - Lv1TaBwd_L1 / 2.0), logi_Lv1TaBwd, "phys_Lv1TaBwd_name", &topVolume, false,
                        0);

      //-
      //----------

      //----------
      //- Lv2VacBwd

      //get parameters from .xml file
      prep = "Lv2VacBwd.";
      //
      double Lv2VacBwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double Lv2VacBwd_D2 = m_config.getParameter(prep + "D2") * Unit::cm / Unit::mm;
      double Lv2VacBwd_D3 = m_config.getParameter(prep + "D3") * Unit::cm / Unit::mm;
      double Lv2VacBwd_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double Lv2VacBwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      double Lv2VacBwd_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv2VacBwd_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double Lv2VacBwd_R2 = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      double Lv2VacBwd_R3 = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      double Lv2VacBwd_R4 = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      double Lv2VacBwd_A1 = m_config.getParameter(prep + "A1");
      double Lv2VacBwd_A2 = m_config.getParameter(prep + "A2");
      //
      string strMat_Lv2VacBwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2VacBwd = Materials::get(strMat_Lv2VacBwd);
      //
      // Part 1
      double Lv2VacBwd1_Z1  = sqrt(Lv2VacBwd_D1 * Lv2VacBwd_D1 + Lv2VacBwd_D2 * Lv2VacBwd_D2 - 2.*Lv2VacBwd_D1 * Lv2VacBwd_D2 * cos(
                                     Lv2VacBwd_A1));
      double Lv2VacBwd1_rI1 = 0.0;
      double Lv2VacBwd1_rO1 = Lv2VacBwd_R1;
      //
      double Lv2VacBwd1_Z2  = (Lv2VacBwd1_Z1 / 2. * cos(Lv2VacBwd_A2)) + SafetyLength;
      double Lv2VacBwd1_rI2 = 0.0;
      double Lv2VacBwd1_rO2 = 2 * Lv2VacBwd_R1;
      // Part 2
      double Lv2VacBwd2_Z1  = Lv2VacBwd_L1;
      double Lv2VacBwd2_rI1 = 0.0;
      double Lv2VacBwd2_rO1 = Lv2VacBwd_R2;
      //
      double Lv2VacBwd2_Z2  = (Lv2VacBwd_L1 / 2. * cos(Lv2VacBwd_A2)) + SafetyLength;
      double Lv2VacBwd2_rI2 = 0.0;
      double Lv2VacBwd2_rO2 = 2 * Lv2VacBwd_R2;
      // Part 3
      const int Lv2VacBwd3_num1 = 2;  // cylindrical part
      const int Lv2VacBwd3_num2 = 3;  // policone part
      // ----------->Cylindrical part
      double Lv2VacBwd_Z1[Lv2VacBwd3_num1];
      Lv2VacBwd_Z1[0] = 0.0;
      Lv2VacBwd_Z1[1] = -Lv2VacBwd_D3 + Lv2VacBwd_L2 - 0.03 * SafetyLength;
      double Lv2VacBwd_rI1[Lv2VacBwd3_num1];
      for (int tmpn = 0; tmpn < Lv2VacBwd3_num1; tmpn++)
      { Lv2VacBwd_rI1[tmpn] = 0.0; }
      double Lv2VacBwd_rO1[Lv2VacBwd3_num1];
      Lv2VacBwd_rO1[0] = Lv2VacBwd_R3;
      Lv2VacBwd_rO1[1] = Lv2VacBwd_R3;
      //<----------------
      //------------> Policone part
      double Lv2VacBwd_Z2[Lv2VacBwd3_num2];
      Lv2VacBwd_Z2[0] = -Lv2VacBwd_D3 + Lv2VacBwd_L2;
      Lv2VacBwd_Z2[1] = -Lv2VacBwd_D3;
      Lv2VacBwd_Z2[2] = -Lv2VacBwd_D3 - Lv2VacBwd_L3;
      double Lv2VacBwd_rI2[Lv2VacBwd3_num2];
      for (int tmpn = 0; tmpn < Lv2VacBwd3_num2; tmpn++)
      { Lv2VacBwd_rI2[tmpn] = 0.0; }
      double Lv2VacBwd_rO2[Lv2VacBwd3_num2];
      Lv2VacBwd_rO2[0] = Lv2VacBwd_R3;
      Lv2VacBwd_rO2[1] = Lv2VacBwd_R4;
      Lv2VacBwd_rO2[2] = Lv2VacBwd_R4;
      //<--------------
      //

      //define geometry
      // Part 1
      G4Tubs* geo_Lv2VacBwdPart1_1 = new G4Tubs("geo_Lv2VacBwdPart1_1_name", Lv2VacBwd1_rI1, Lv2VacBwd1_rO1, Lv2VacBwd1_Z1, 0, 2 * M_PI);
      G4Tubs* geo_Lv2VacBwdPart1_2 = new G4Tubs("geo_Lv2VacBwdPart1_2_name", Lv2VacBwd1_rI2, Lv2VacBwd1_rO2, Lv2VacBwd1_Z2, 0, 2 * M_PI);
      G4Transform3D transform_Lv2VacBwdPart1_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacBwdPart1_2 = transform_Lv2VacBwdPart1_2 * G4RotateY3D(Lv2VacBwd_A2 / 2.);
      G4IntersectionSolid* geo_Lv2VacBwdPart1 = new G4IntersectionSolid("geo_Lv2VacBwdPart1_name", geo_Lv2VacBwdPart1_1,
          geo_Lv2VacBwdPart1_2, transform_Lv2VacBwdPart1_2);
      // Part 2
      G4Tubs* geo_Lv2VacBwdPart2_1 = new G4Tubs("geo_Lv2VacBwdPart2_1_name", Lv2VacBwd2_rI1, Lv2VacBwd2_rO1, Lv2VacBwd2_Z1, 0, 2 * M_PI);
      G4Tubs* geo_Lv2VacBwdPart2_2 = new G4Tubs("geo_Lv2VacBwdPart2_2_name", Lv2VacBwd2_rI2, Lv2VacBwd2_rO2, Lv2VacBwd2_Z2, 0, 2 * M_PI);
      G4Transform3D transform_Lv2VacBwdPart2_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacBwdPart2_2 = transform_Lv2VacBwdPart2_2 * G4RotateY3D(-Lv2VacBwd_A2 / 2.);
      G4IntersectionSolid* geo_Lv2VacBwdPart2 = new G4IntersectionSolid("geo_Lv2VacBwdPart2_name", geo_Lv2VacBwdPart2_1,
          geo_Lv2VacBwdPart2_2, transform_Lv2VacBwdPart2_2);
      // Part 3
//      G4Polycone* geo_Lv2VacBwdPart3 = new G4Polycone("geo_Lv2VacBwdPart3", 0, 2 * M_PI, Lv2VacBwd3_num, Lv2VacBwd_Z, Lv2VacBwd_rI, Lv2VacBwd_rO);
      G4Polycone* geo_Lv2VacBwdPart3_1 = new G4Polycone("geo_Lv2VacBwdPart3_1", 0, 2 * M_PI, Lv2VacBwd3_num1, Lv2VacBwd_Z1, Lv2VacBwd_rI1,
                                                        Lv2VacBwd_rO1);
      G4Polycone* geo_Lv2VacBwdPart3_2 = new G4Polycone("geo_Lv2VacBwdPart3_2", 0, 2 * M_PI, Lv2VacBwd3_num2, Lv2VacBwd_Z2, Lv2VacBwd_rI2,
                                                        Lv2VacBwd_rO2);
      G4Transform3D transform_Lv2VacBwdPart3_1 = G4Translate3D(-0.5, 0., 0.);
      transform_Lv2VacBwdPart3_1 = transform_Lv2VacBwdPart3_1  * G4RotateY3D(-Lv2VacBwd_A1 + A11);
      G4UnionSolid* geo_Lv2VacBwdPart3 = new G4UnionSolid("geo_Lv2VacBwdPart3_name", geo_Lv2VacBwdPart3_2, geo_Lv2VacBwdPart3_1,
                                                          transform_Lv2VacBwdPart3_1);
      // Part1+2+3
      //tmp begin
      G4Transform3D transform_Lv2VacBwdPart1 = G4Translate3D((Lv2VacBwd_D1 * sin(Lv2VacBwd_A1) + Lv2VacBwd_D2 * sin(
                                                                2.*Lv2VacBwd_A1)) / 2. ,
                                                             0.,
                                                             -(Lv2VacBwd_D1 * cos(Lv2VacBwd_A1) + Lv2VacBwd_D2 * cos(2.*Lv2VacBwd_A1)) / 2.);
      //G4Transform3D transform_Lv2VacBwdPart1 = G4Translate3D((Lv2VacBwd_D1 * sin(Lv2VacBwd_A1) + Lv2VacBwd_D2 * sin(2.*Lv2VacBwd_A1)) / 2.05,
      //                                                       0.,
      //                                                       -(Lv2VacBwd_D1 * cos(Lv2VacBwd_A1) + Lv2VacBwd_D2 * cos(2.*Lv2VacBwd_A1)) / 2.05);
      transform_Lv2VacBwdPart1 = transform_Lv2VacBwdPart1 * G4RotateY3D(-Lv2VacBwd_A1 - Lv2VacBwd_A2);
      //tmp end
      //
      G4Transform3D transform_Lv2VacBwdPart2 = G4Translate3D((Lv2VacBwd_D2 + Lv2VacBwd_L1 / 2.0) * sin(2.*Lv2VacBwd_A1) ,
                                                             0.,
                                                             -(Lv2VacBwd_D2 + Lv2VacBwd_L1 / 2.0) * cos(2.*Lv2VacBwd_A1));
      transform_Lv2VacBwdPart2 = transform_Lv2VacBwdPart2 * G4RotateY3D(-2.*Lv2VacBwd_A1);
      //
      G4UnionSolid* geo_Lv2VacBwdxx = new G4UnionSolid("geo_Lv2VacBwdxx_name", geo_Lv2VacBwdPart3, geo_Lv2VacBwdPart1,
                                                       transform_Lv2VacBwdPart1);
      G4UnionSolid* geo_Lv2VacBwdx = new G4UnionSolid("geo_Lv2VacBwdx_name", geo_Lv2VacBwdxx, geo_Lv2VacBwdPart2,
                                                      transform_Lv2VacBwdPart2);
      // Intersection
      G4Transform3D transform_Lv2VacBwd = G4Translate3D(0., 0., +Lv1TaBwd_D1 + Lv1TaBwd_L1 / 2.);
      transform_Lv2VacBwd = transform_Lv2VacBwd * G4RotateY3D(+Lv2VacBwd_A1);
      //
      G4IntersectionSolid* geo_Lv2VacBwd = new G4IntersectionSolid("geo_Lv2VacBwd_name", geo_Lv1TaBwd, geo_Lv2VacBwdx,
          transform_Lv2VacBwd);
      G4LogicalVolume* logi_Lv2VacBwd = new G4LogicalVolume(geo_Lv2VacBwd, mat_Lv2VacBwd, "logi_Lv2VacBwd_name");
      if (flag_limitStep) logi_Lv2VacBwd->SetUserLimits(new G4UserLimits(stepMax));

      //-   put volume
      setColor(*logi_Lv2VacBwd, "#CCCCCC");
      //you must set this invisible, otherwise encounter segV.
      setVisibility(*logi_Lv2VacBwd, false);
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logi_Lv2VacBwd, "phys_Lv2VacBwd_name", logi_Lv1TaBwd, false, 0);

      //-

      ////==========
      ////= beam pipe Forward Forward

      //get parameters from .xml file
      prep = "AreaTubeFwd.";
      //
      const int AreaTubeFwd_num = 2;
      //
      double AreaTubeFwd_Z[AreaTubeFwd_num];
      AreaTubeFwd_Z[0] = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      AreaTubeFwd_Z[1] = m_config.getParameter(prep + "D2") * Unit::cm / Unit::mm;
      //
      double AreaTubeFwd_rI[AreaTubeFwd_num];
      for (int i = 0; i < AreaTubeFwd_num; i++)
      { AreaTubeFwd_rI[i] = 0.0; }
      //
      double AreaTubeFwd_rO[AreaTubeFwd_num];
      AreaTubeFwd_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      AreaTubeFwd_rO[1] = AreaTubeFwd_rO[0];

      //define geometry
      G4Polycone* geo_AreaTubeFwdpcon = new G4Polycone("geo_AreaTubeFwdpcon_name", 0, 2 * M_PI, AreaTubeFwd_num, AreaTubeFwd_Z,
                                                       AreaTubeFwd_rI, AreaTubeFwd_rO);

      //----------
      //- Lv1TaLERUp

      //get parameters from .xml file
      prep = "Lv1TaLERUp.";
      //
      double Lv1TaLERUp_A1 = m_config.getParameter(prep + "A1");
      //
      const int Lv1TaLERUp_num = 4;
      //
      double Lv1TaLERUp_Z[Lv1TaLERUp_num];
      Lv1TaLERUp_Z[0] = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv1TaLERUp_Z[1] = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      Lv1TaLERUp_Z[2] = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      Lv1TaLERUp_Z[3] = m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      //
      double Lv1TaLERUp_rI[Lv1TaLERUp_num];
      for (int i = 0; i < Lv1TaLERUp_num; i++)
      { Lv1TaLERUp_rI[i] = 0.0; }
      //
      double Lv1TaLERUp_rO[Lv1TaLERUp_num];
      Lv1TaLERUp_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv1TaLERUp_rO[1] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv1TaLERUp_rO[2] = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      Lv1TaLERUp_rO[3] = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaLERUp = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaLERUp = Materials::get(strMat_Lv1TaLERUp);

      //define geometry
      G4Polycone* geo_Lv1TaLERUppcon = new G4Polycone("geo_Lv1TaLERUppcon_name", 0, 2 * M_PI, Lv1TaLERUp_num, Lv1TaLERUp_Z, Lv1TaLERUp_rI,
                                                      Lv1TaLERUp_rO);
      G4Transform3D transform_AreaTubeFwdForLER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeFwdForLER = transform_AreaTubeFwdForLER * G4RotateY3D(-Lv1TaLERUp_A1);
      G4IntersectionSolid* geo_Lv1TaLERUp = new G4IntersectionSolid("geo_Lv1TaLERUp_name", geo_Lv1TaLERUppcon, geo_AreaTubeFwdpcon,
          transform_AreaTubeFwdForLER);
      G4LogicalVolume* logi_Lv1TaLERUp = new G4LogicalVolume(geo_Lv1TaLERUp, mat_Lv1TaLERUp, "logi_Lv1TaLERUp_name");

      //-   put volume
      setColor(*logi_Lv1TaLERUp, "#0000CC");
      G4Transform3D transform_Lv1TaLERUp = G4Translate3D(0., 0., 0.);
      transform_Lv1TaLERUp = transform_Lv1TaLERUp * G4RotateY3D(Lv1TaLERUp_A1);
      new G4PVPlacement(transform_Lv1TaLERUp, logi_Lv1TaLERUp, "phys_Lv1TaLERUp_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacLERUp

      //get parameters from .xml file
      prep = "Lv2VacLERUp.";
      //
      double Lv2VacLERUp_rO[Lv1TaLERUp_num];
      Lv2VacLERUp_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2VacLERUp_rO[1] = Lv2VacLERUp_rO[0];
      Lv2VacLERUp_rO[2] = Lv2VacLERUp_rO[0];
      Lv2VacLERUp_rO[3] = Lv2VacLERUp_rO[0];
      //
      string strMat_Lv2VacLERUp = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2VacLERUp = Materials::get(strMat_Lv2VacLERUp);

      //define geometry
      G4Polycone* geo_Lv2VacLERUppcon = new G4Polycone("geo_Lv2VacLERUppcon_name", 0, 2 * M_PI, Lv1TaLERUp_num, Lv1TaLERUp_Z,
                                                       Lv1TaLERUp_rI, Lv2VacLERUp_rO);
      G4IntersectionSolid* geo_Lv2VacLERUp = new G4IntersectionSolid("geo_Lv2VacLERUp_name", geo_Lv2VacLERUppcon, geo_AreaTubeFwdpcon,
          transform_AreaTubeFwdForLER);
      G4LogicalVolume* logi_Lv2VacLERUp = new G4LogicalVolume(geo_Lv2VacLERUp, mat_Lv2VacLERUp, "logi_Lv2VacLERUp_name");
      if (flag_limitStep) logi_Lv2VacLERUp->SetUserLimits(new G4UserLimits(stepMax));


      //-   put volume
      setColor(*logi_Lv2VacLERUp, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacLERUp, "phys_Lv2VacLERUp_name", logi_Lv1TaLERUp, false, 0);
      //-
      //----------

      //----------
      //- Lv1TaHERDwn

      //get parameters from .xml file
      prep = "Lv1TaHERDwn.";
      //
      double Lv1TaHERDwn_A1 = m_config.getParameter(prep + "A1");
      //
      const int Lv1TaHERDwn_num = 4;
      //
      double Lv1TaHERDwn_Z[Lv1TaHERDwn_num];
      Lv1TaHERDwn_Z[0] = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv1TaHERDwn_Z[1] = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      Lv1TaHERDwn_Z[2] = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      Lv1TaHERDwn_Z[3] = m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      //
      double Lv1TaHERDwn_rI[Lv1TaHERDwn_num];
      for (int i = 0; i < Lv1TaHERDwn_num; i++)
      { Lv1TaHERDwn_rI[i] = 0.0; }
      //
      double Lv1TaHERDwn_rO[Lv1TaHERDwn_num];
      Lv1TaHERDwn_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv1TaHERDwn_rO[1] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv1TaHERDwn_rO[2] = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      Lv1TaHERDwn_rO[3] = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaHERDwn = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaHERDwn = Materials::get(strMat_Lv1TaHERDwn);

      //define geometry
      G4Polycone* geo_Lv1TaHERDwnpcon = new G4Polycone("geo_Lv1TaHERDwnpcon_name", 0, 2 * M_PI, Lv1TaHERDwn_num, Lv1TaHERDwn_Z,
                                                       Lv1TaHERDwn_rI, Lv1TaHERDwn_rO);
      G4Transform3D transform_AreaTubeFwdForHER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeFwdForHER = transform_AreaTubeFwdForHER * G4RotateY3D(-Lv1TaHERDwn_A1);
      G4IntersectionSolid* geo_Lv1TaHERDwn = new G4IntersectionSolid("", geo_Lv1TaHERDwnpcon, geo_AreaTubeFwdpcon,
          transform_AreaTubeFwdForHER);
      G4LogicalVolume* logi_Lv1TaHERDwn = new G4LogicalVolume(geo_Lv1TaHERDwn, mat_Lv1TaHERDwn, "logi_Lv1TaHERDwn_name");

      //-   put volume
      setColor(*logi_Lv1TaHERDwn, "#00CC00");
      G4Transform3D transform_Lv1TaHERDwn = G4Translate3D(0., 0., 0.);
      transform_Lv1TaHERDwn = transform_Lv1TaHERDwn * G4RotateY3D(Lv1TaHERDwn_A1);
      new G4PVPlacement(transform_Lv1TaHERDwn, logi_Lv1TaHERDwn, "phys_Lv1TaHERDwn_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacHERDwn

      //get parameters from .xml file
      prep = "Lv2VacHERDwn.";
      //
      double Lv2VacHERDwn_rO[Lv1TaHERDwn_num];
      Lv2VacHERDwn_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2VacHERDwn_rO[1] = Lv2VacHERDwn_rO[0];
      Lv2VacHERDwn_rO[2] = Lv2VacHERDwn_rO[0];
      Lv2VacHERDwn_rO[3] = Lv2VacHERDwn_rO[0];
      //
      string strMat_Lv2VacHERDwn = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2VacHERDwn = Materials::get(strMat_Lv2VacHERDwn);

      //define geometry
      G4Polycone* geo_Lv2VacHERDwnpcon = new G4Polycone("geo_Lv2VacHERDwnpcon_name", 0, 2 * M_PI, Lv1TaHERDwn_num, Lv1TaHERDwn_Z,
                                                        Lv1TaHERDwn_rI, Lv2VacHERDwn_rO);
      G4IntersectionSolid* geo_Lv2VacHERDwn = new G4IntersectionSolid("", geo_Lv2VacHERDwnpcon, geo_AreaTubeFwdpcon,
          transform_AreaTubeFwdForHER);
      G4LogicalVolume* logi_Lv2VacHERDwn = new G4LogicalVolume(geo_Lv2VacHERDwn, mat_Lv2VacHERDwn, "logi_Lv2VacHERDwn_name");
      if (flag_limitStep) logi_Lv2VacHERDwn->SetUserLimits(new G4UserLimits(stepMax));

      //-   put volume
      setColor(*logi_Lv2VacHERDwn, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacHERDwn, "phys_Lv2VacHERDwn_name", logi_Lv1TaHERDwn, false, 0);

      //-
      //----------

      ////=
      ////==========

      ////==========
      ////= beam pipe Backward Backward

      //get parameters from .xml file
      prep = "AreaTubeBwd.";
      //
      const int AreaTubeBwd_num = 2;
      //
      double AreaTubeBwd_Z[AreaTubeBwd_num];
      AreaTubeBwd_Z[0] = -m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      AreaTubeBwd_Z[1] = -m_config.getParameter(prep + "D2") * Unit::cm / Unit::mm;
      //
      double AreaTubeBwd_rI[AreaTubeBwd_num];
      for (int i = 0; i < AreaTubeBwd_num; i++)
      { AreaTubeBwd_rI[i] = 0.0; }
      //
      double AreaTubeBwd_rO[AreaTubeBwd_num];
      AreaTubeBwd_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      AreaTubeBwd_rO[1] = AreaTubeBwd_rO[0];

      //define geometry
      G4Polycone* geo_AreaTubeBwdpcon = new G4Polycone("geo_AreaTubeBwdpcon_name", 0, 2 * M_PI, AreaTubeBwd_num, AreaTubeBwd_Z,
                                                       AreaTubeBwd_rI, AreaTubeBwd_rO);

      //----------
      //- Lv1TaHERUp

      //get parameters from .xml file
      prep =  "Lv1TaHERUp.";
      //
      double Lv1TaHERUp_A1 = m_config.getParameter(prep + "A1");
      //
      const int Lv1TaHERUp_num = 4;
      //
      double Lv1TaHERUp_Z[Lv1TaHERUp_num];
      Lv1TaHERUp_Z[0] = -m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv1TaHERUp_Z[1] = -m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      Lv1TaHERUp_Z[2] = -m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      Lv1TaHERUp_Z[3] = -m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      //
      double Lv1TaHERUp_rI[Lv1TaHERUp_num];
      for (int i = 0; i < Lv1TaHERUp_num; i++)
      { Lv1TaHERUp_rI[i] = 0.0; }
      //
      double Lv1TaHERUp_rO[Lv1TaHERUp_num];
      Lv1TaHERUp_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv1TaHERUp_rO[1] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv1TaHERUp_rO[2] = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      Lv1TaHERUp_rO[3] = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaHERUp = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaHERUp = Materials::get(strMat_Lv1TaHERUp);

      //define geometry
      G4Polycone* geo_Lv1TaHERUppcon = new G4Polycone("geo_Lv1TaHERUppcon_name", 0, 2 * M_PI, Lv1TaHERUp_num, Lv1TaHERUp_Z, Lv1TaHERUp_rI,
                                                      Lv1TaHERUp_rO);
      G4Transform3D transform_AreaTubeBwdForHER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeBwdForHER = transform_AreaTubeBwdForHER * G4RotateY3D(-Lv1TaHERUp_A1);
      G4IntersectionSolid* geo_Lv1TaHERUp = new G4IntersectionSolid("", geo_Lv1TaHERUppcon, geo_AreaTubeBwdpcon,
          transform_AreaTubeBwdForHER);
      G4LogicalVolume* logi_Lv1TaHERUp = new G4LogicalVolume(geo_Lv1TaHERUp, mat_Lv1TaHERUp, "logi_Lv1TaHERUp_name");

      //-   put volume
      setColor(*logi_Lv1TaHERUp, "#00CC00");
      G4Transform3D transform_Lv1TaHERUp = G4Translate3D(0., 0., 0.);
      transform_Lv1TaHERUp = transform_Lv1TaHERUp * G4RotateY3D(Lv1TaHERUp_A1);
      new G4PVPlacement(transform_Lv1TaHERUp, logi_Lv1TaHERUp, "phys_Lv1TaHERUp_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacHERUp

      //get parameters from .xml file
      prep =  "Lv2VacHERUp.";
      //
      double Lv2VacHERUp_rO[Lv1TaHERUp_num];
      Lv2VacHERUp_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2VacHERUp_rO[1] = Lv2VacHERUp_rO[0];
      Lv2VacHERUp_rO[2] = Lv2VacHERUp_rO[0];
      Lv2VacHERUp_rO[3] = Lv2VacHERUp_rO[0];
      //
      string strMat_Lv2VacHERUp = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2VacHERUp = Materials::get(strMat_Lv2VacHERUp);

      //define geometry
      G4Polycone* geo_Lv2VacHERUppcon = new G4Polycone("geo_Lv2VacHERUppcon_name", 0, 2 * M_PI, Lv1TaHERUp_num, Lv1TaHERUp_Z,
                                                       Lv1TaHERUp_rI, Lv2VacHERUp_rO);
      G4IntersectionSolid* geo_Lv2VacHERUp = new G4IntersectionSolid("", geo_Lv2VacHERUppcon, geo_AreaTubeBwdpcon,
          transform_AreaTubeFwdForHER);
      G4LogicalVolume* logi_Lv2VacHERUp = new G4LogicalVolume(geo_Lv2VacHERUp, mat_Lv2VacHERUp, "logi_Lv2VacHERUp_name");
      if (flag_limitStep) logi_Lv2VacHERUp->SetUserLimits(new G4UserLimits(stepMax));

      //-   put volume
      setColor(*logi_Lv2VacHERUp, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacHERUp, "phys_Lv2VacHERUp_name", logi_Lv1TaHERUp, false, 0);

      //-
      //----------

      //----------
      //- Lv1TaLERDwn

      //get parameters from .xml file
      prep = "Lv1TaLERDwn.";
      //
      double Lv1TaLERDwn_A1 = m_config.getParameter(prep + "A1");
      //
      const int Lv1TaLERDwn_num = 4;
      //
      double Lv1TaLERDwn_Z[Lv1TaLERDwn_num];
      Lv1TaLERDwn_Z[0] = -m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      Lv1TaLERDwn_Z[1] = -m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      Lv1TaLERDwn_Z[2] = -m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      Lv1TaLERDwn_Z[3] = -m_config.getParameter(prep + "L4") * Unit::cm / Unit::mm;
      //
      double Lv1TaLERDwn_rI[Lv1TaLERDwn_num];
      for (int i = 0; i < Lv1TaLERDwn_num; i++)
      { Lv1TaLERDwn_rI[i] = 0.0; }
      //
      double Lv1TaLERDwn_rO[Lv1TaLERDwn_num];
      Lv1TaLERDwn_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv1TaLERDwn_rO[1] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      Lv1TaLERDwn_rO[2] = m_config.getParameter(prep + "R3") * Unit::cm / Unit::mm;
      Lv1TaLERDwn_rO[3] = m_config.getParameter(prep + "R4") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaLERDwn = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaLERDwn = Materials::get(strMat_Lv1TaLERDwn);

      //define geometry
      G4Polycone* geo_Lv1TaLERDwnpcon = new G4Polycone("geo_Lv1TaLERDwnpcon_name", 0, 2 * M_PI, Lv1TaLERDwn_num, Lv1TaLERDwn_Z,
                                                       Lv1TaLERDwn_rI, Lv1TaLERDwn_rO);
      G4Transform3D transform_AreaTubeBwdForLER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeBwdForLER = transform_AreaTubeBwdForLER * G4RotateY3D(-Lv1TaLERDwn_A1);
      G4IntersectionSolid* geo_Lv1TaLERDwn = new G4IntersectionSolid("", geo_Lv1TaLERDwnpcon, geo_AreaTubeBwdpcon,
          transform_AreaTubeBwdForLER);
      G4LogicalVolume* logi_Lv1TaLERDwn = new G4LogicalVolume(geo_Lv1TaLERDwn, mat_Lv1TaLERDwn, "logi_Lv1TaLERDwn_name");

      //-   put volume
      setColor(*logi_Lv1TaLERDwn, "#0000CC");
      G4Transform3D transform_Lv1TaLERDwn = G4Translate3D(0., 0., 0.);
      transform_Lv1TaLERDwn = transform_Lv1TaLERDwn * G4RotateY3D(Lv1TaLERDwn_A1);
      new G4PVPlacement(transform_Lv1TaLERDwn, logi_Lv1TaLERDwn, "phys_Lv1TaLERDwn_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacLERDwn

      //get parameters from .xml file
      prep = "Lv2VacLERDwn.";
      //
      double Lv2VacLERDwn_rO[Lv1TaLERDwn_num];
      Lv2VacLERDwn_rO[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      Lv2VacLERDwn_rO[1] = Lv2VacLERDwn_rO[0];
      Lv2VacLERDwn_rO[2] = Lv2VacLERDwn_rO[0];
      Lv2VacLERDwn_rO[3] = Lv2VacLERDwn_rO[0];
      //
      string strMat_Lv2VacLERDwn = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv2VacLERDwn = Materials::get(strMat_Lv2VacLERDwn);

      //define geometry
      G4Polycone* geo_Lv2VacLERDwnpcon = new G4Polycone("geo_Lv2VacLERDwnpcon_name", 0, 2 * M_PI, Lv1TaLERDwn_num, Lv1TaLERDwn_Z,
                                                        Lv1TaLERDwn_rI, Lv2VacLERDwn_rO);
      G4IntersectionSolid* geo_Lv2VacLERDwn = new G4IntersectionSolid("", geo_Lv2VacLERDwnpcon, geo_AreaTubeBwdpcon,
          transform_AreaTubeBwdForLER);
      G4LogicalVolume* logi_Lv2VacLERDwn = new G4LogicalVolume(geo_Lv2VacLERDwn, mat_Lv2VacLERDwn, "logi_Lv2VacLERDwn_name");
      if (flag_limitStep) logi_Lv2VacLERDwn->SetUserLimits(new G4UserLimits(stepMax));

      //-   put volume
      setColor(*logi_Lv2VacLERDwn, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacLERDwn, "phys_Lv2VacLERDwn_name", logi_Lv1TaLERDwn, false, 0);


      //----------
      // Cu flange

      G4IntersectionSolid* geo_CuFlangeFwd_x2 = new G4IntersectionSolid("geo_CuFlangeFwd_x2_name", geo_AreaTubeFwdpcon, geo_Flange,
          G4Translate3D(0, 0, Flange_D + Flange_T * 2));
      G4SubtractionSolid* geo_CuFlangeFwd_x = new G4SubtractionSolid("geo_CuFlangeFwd_x_name", geo_CuFlangeFwd_x2, geo_Lv1TaLERUp,
          transform_Lv1TaLERUp);
      G4SubtractionSolid* geo_CuFlangeFwd   = new G4SubtractionSolid("geo_CuFlangeFwd_name",  geo_CuFlangeFwd_x,  geo_Lv1TaHERDwn,
          transform_Lv1TaHERDwn);

      G4LogicalVolume* logi_CuFlangeFwd = new G4LogicalVolume(geo_CuFlangeFwd, mat_Lv1TaLERUp, "logi_CuFlangeFwd_name");

      //-   put volume
      setColor(*logi_CuFlangeFwd, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_CuFlangeFwd, "phys_CuFlangeFwd_name", &topVolume, false, 0);




      G4IntersectionSolid* geo_CuFlangeBwd_x2 = new G4IntersectionSolid("geo_CuFlangeBwd_x2_name", geo_AreaTubeBwdpcon, geo_Flange,
          G4Translate3D(0, 0, -Flange_D - Flange_T * 2));
      G4SubtractionSolid* geo_CuFlangeBwd_x = new G4SubtractionSolid("geo_CuFlangeBwd_x_name", geo_CuFlangeBwd_x2, geo_Lv1TaHERUp,
          transform_Lv1TaHERUp);
      G4SubtractionSolid* geo_CuFlangeBwd   = new G4SubtractionSolid("geo_CuFlangeBwd_name",  geo_CuFlangeBwd_x,  geo_Lv1TaLERDwn,
          transform_Lv1TaLERDwn);

      G4LogicalVolume* logi_CuFlangeBwd = new G4LogicalVolume(geo_CuFlangeBwd, mat_Lv1TaLERUp, "logi_CuFlangeBwd_name");

      //-   put volume
      setColor(*logi_CuFlangeBwd, "#CCCCCC");
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_CuFlangeBwd, "phys_CuFlangeBwd_name", &topVolume, false, 0);




      //logi_Lv3AuCoat->SetSensitiveDetector(new BkgSensitiveDetector("IR", 11));
      //logi_Lv1TaFwd->SetSensitiveDetector(new BkgSensitiveDetector("IR", 12));
      //logi_Lv1TaBwd->SetSensitiveDetector(new BkgSensitiveDetector("IR", 13));
      //logi_Lv1TaLERUp->SetSensitiveDetector(new BkgSensitiveDetector("IR", 14));
      //logi_Lv1TaHERDwn->SetSensitiveDetector(new BkgSensitiveDetector("IR", 15));
      //logi_Lv1TaHERUp->SetSensitiveDetector(new BkgSensitiveDetector("IR", 16));
      //logi_Lv1TaLERDwn->SetSensitiveDetector(new BkgSensitiveDetector("IR", 17));
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 11)));
      //logi_Lv3AuCoat->SetSensitiveDetector(m_sensitive.back());
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 12)));
      //logi_Lv1TaFwd->SetSensitiveDetector(m_sensitive.back());
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 13)));
      //logi_Lv1TaBwd->SetSensitiveDetector(m_sensitive.back());
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 14)));
      //logi_Lv1TaLERUp->SetSensitiveDetector(m_sensitive.back());
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 15)));
      //logi_Lv1TaHERDwn->SetSensitiveDetector(m_sensitive.back());
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 16)));
      //logi_Lv1TaHERUp->SetSensitiveDetector(m_sensitive.back());
      //m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", 17)));
      //logi_Lv1TaLERDwn->SetSensitiveDetector(m_sensitive.back());



      //-
      //----------

      ////=
      ////==========
    }
  }
}
