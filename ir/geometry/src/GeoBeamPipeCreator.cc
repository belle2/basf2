/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <ir/geometry/GeoBeamPipeCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/Unit.h>
#include <ir/simulation/SensitiveDetector.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <cmath>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <G4EllipticalTube.hh>
#include <G4UnionSolid.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4DisplacedSolid.hh>
#include <G4UserLimits.hh>

using namespace std;

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
      for (SensitiveDetector* sensitive : m_sensitive) {
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
      // Tangusten End Mounts
      //    PXDMountFwd
      //    PXDMountFixtureFwd
      //    PXDMountBwd
      //    PXDMountFixtureBwd
      //
      //###########################

      double SafetyLength = m_config.getParameter("Safety.L1") * Unit::cm / Unit::mm;

      double stepMax = 5.0 * Unit::mm;
      int flag_limitStep = int(m_config.getParameter("LimitStepLength"));

      double A11 = 0.03918;

      std::string prep;
      G4LogicalVolume* logi_Lv3AuCoat = nullptr;

      ////==========
      ////= IP pipe

      //----------
#if 0
      //- Lv1SUS

      //get parameters from .xml file
      std::string prep = "Lv1SUS.";
      //
      const int Lv1SUS_num = 21;
      //
      double Lv1SUS_Z[Lv1SUS_num];
      Lv1SUS_Z[0] = 0.0;
      for (int tmpn = 0; tmpn < 8; tmpn++) {
        Lv1SUS_Z[0] -= m_config.getParameter(prep + (boost::format("L%1%") % (tmpn + 1)).str().c_str()) * Unit::cm / Unit::mm;
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


      //----------
      //- Lv2OutTi added for Phase 3.
      //-
      //----------
      //get parameters from .xml file
      if (m_config.getParameter("Lv2OutTi.L1", -1) > 0) {
        prep = "Lv2OutTi.";
        //
        const int Lv2OutTi_num = 2;
        //
        double Lv2OutTi_Z[Lv2OutTi_num];
        Lv2OutTi_Z[0] = -m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
        Lv2OutTi_Z[1] = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
        //
        double Lv2OutTi_rI[Lv2OutTi_num];
        Lv2OutTi_rI[0] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
        Lv2OutTi_rI[1] = Lv2OutTi_rI[0];
        //
        double Lv2OutTi_rO[Lv2OutTi_num];
        Lv2OutTi_rO[0] = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
        Lv2OutTi_rO[1] = Lv2OutTi_rO[0];
        //
        string strMat_Lv2OutTi =  m_config.getParameterStr(prep + "Material");
        G4Material* mat_Lv2OutTi = Materials::get(strMat_Lv2OutTi);

        //define geometry
        G4Polycone* geo_Lv2OutTi = new G4Polycone("geo_Lv2OutTi_name", 0, 2 * M_PI, Lv2OutTi_num, Lv2OutTi_Z, Lv2OutTi_rI, Lv2OutTi_rO);
        G4LogicalVolume* logi_Lv2OutTi = new G4LogicalVolume(geo_Lv2OutTi, mat_Lv2OutTi, "logi_Lv2OutTi_name");

        //-   put volume
        setColor(*logi_Lv2OutTi, "#333300");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2OutTi, "phys_Lv2OutTi_name", logi_Lv1SUS, false, 0);
      }


      //----------
      //- Lv2OutBe
      //-
      //----------

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
        Lv2Paraf1_Z[0] -= m_config.getParameter(prep + (boost::format("L%1%") % (tmpn + 1)).str().c_str()) * Unit::cm / Unit::mm;
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
        Lv2Paraf2_Z[0] += m_config.getParameter(prep + (boost::format("L%1%") % tmpn).str().c_str()) * Unit::cm / Unit::mm;
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
#endif

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
      //- New Geant4 Geometry from STEP & Blueprints


      G4VSolid* geo_IPBeamPipe_FWD = nullptr;
      G4VSolid* geo_IPBeamPipe_BWD = nullptr;
      G4VSolid* geo_BellowsShield_FWD = nullptr;
      G4VSolid* geo_AdditionalShield_FWD = nullptr;
      G4LogicalVolume* logi_IPBeamPipe_FWD = nullptr;
      G4LogicalVolume* logi_IPBeamPipe_BWD = nullptr;
      G4LogicalVolume* logi_IPChamber_FWD = nullptr;
      G4LogicalVolume* logi_IPChamber_BWD = nullptr;
      G4LogicalVolume* logi_BellowsPipe_FWD = nullptr;
      G4LogicalVolume* logi_BellowsPipe_BWD = nullptr;
      G4LogicalVolume* logi_BellowsShield_FWD = nullptr;
      G4LogicalVolume* logi_BellowsShield_BWD = nullptr;
      G4LogicalVolume* logi_AdditionalShield_FWD = nullptr;
      G4LogicalVolume* logi_AdditionalShield_BWD = nullptr;

      std::vector<std::string> newParts = {"IPChamber_FWD", "IPBeamPipe_FWD", "VacFWD", "BellowsPipe_FWD", "AdditionalShield_FWD", "BellowsShield_FWD",
                                           "IPChamber_BWD", "IPBeamPipe_BWD", "VacBWD", "BellowsPipe_BWD", "AdditionalShield_BWD", "BellowsShield_BWD"
                                          };
      // Counts: IPChamber_FWD=8, IPBeamPipe_FWD=12, VacFWD=4, BellowsPipe_FWD=6,
      //         AdditionalShield_FWD=12 (was 5), AdditionalShield_BWD=4 (was 2)
      std::vector<int> newPartsNum = {8, 12, 4, 6, 12, 8, 4, 12, 2, 6, 4, 8};

      std::vector<std::string> colors = {"#333333", "#333333", "#CCCCCC", "#FFD700", "#555555", "#555555",
                                         "#333333", "#333333", "#CCCCCC", "#FFD700", "#555555", "#555555"
                                        };

      for (size_t i = 0; i < newParts.size(); ++i) {
        prep = newParts[i] + ".";
        int num = newPartsNum[i];
        double* Z = new double[num];
        double* rI = new double[num];
        double* rO = new double[num];
        for (int j = 0; j < num; ++j) {
          Z[j]  = m_config.getParameter(prep + "Z" + std::to_string(j)) * Unit::cm / Unit::mm;
          rI[j] = m_config.getParameter(prep + "RI" + std::to_string(j)) * Unit::cm / Unit::mm;
          rO[j] = m_config.getParameter(prep + "RO" + std::to_string(j)) * Unit::cm / Unit::mm;
        }
        std::string strMat = m_config.getParameterStr(prep + "Material");
        G4Material* mat = Materials::get(strMat);

        G4VSolid* geo = nullptr;
        // Common tube hole radii (matching older geometry reference):
        //   HER (upper) tube: diameter 16 mm -> radius 0.8 cm
        //   LER (lower) tube: diameter 10 mm -> radius 0.5 cm
        const double her_r = 0.8 * Unit::cm / Unit::mm;
        const double ler_r = 0.5 * Unit::cm / Unit::mm;

        if (newParts[i] == "IPChamber_FWD" || newParts[i] == "IPChamber_BWD") {
          // Straight central IP chamber tube — plain polycone, no boolean subtraction
          geo = new G4Polycone("geo_" + newParts[i], 0, 2 * M_PI, num, Z, rI, rO);
        } else if (newParts[i] == "IPBeamPipe_FWD" || newParts[i] == "BellowsPipe_FWD" ||
                   newParts[i] == "IPBeamPipe_BWD" || newParts[i] == "BellowsPipe_BWD") {
          // Mother volume is the solid polycone (RI = 0)
          geo = new G4Polycone("geo_" + newParts[i], 0, 2 * M_PI, num, Z, rI, rO);
        } else {
          geo = new G4Polycone("geo_" + newParts[i], 0, 2 * M_PI, num, Z, rI, rO);
        }

        G4LogicalVolume* logi = new G4LogicalVolume(geo, mat, "logi_" + newParts[i]);
        if (newParts[i].find("Vac") != std::string::npos) {
          if (flag_limitStep) logi->SetUserLimits(new G4UserLimits(stepMax));
          setVisibility(*logi, false);
        }
        setColor(*logi, colors[i]);

        // Place vacuum daughter volumes if this is one of the solid crotch/bellows pipes
        if (newParts[i] == "IPBeamPipe_FWD" || newParts[i] == "BellowsPipe_FWD" ||
            newParts[i] == "IPBeamPipe_BWD" || newParts[i] == "BellowsPipe_BWD") {

          double Z_start = Z[0];
          double Z_end = Z[num - 1];
          double length = std::abs(Z_end - Z_start);
          double tilted_length = length / std::cos(0.0415);

          double z_center = (Z_start + Z_end) / 2.0;

          double z_center_placement = z_center / std::cos(0.0415);

          // On the BWD (left) side z_center < 0, so:
          //   HER tube (angle +0.0415) lands at negative X = lower
          //   LER tube (angle -0.0415) lands at positive X = upper
          // Swap radii so that upper always has 16 mm diameter and lower always has 10 mm.
          bool isBWD = newParts[i].find("BWD") != std::string::npos;
          double r_her_hole = isBWD ? ler_r : her_r;
          double r_ler_hole = isBWD ? her_r : ler_r;

          // HER daughter
          double angle_her = 0.0415;
          double x_her = z_center_placement * std::sin(angle_her);
          double z_her = z_center_placement * std::cos(angle_her);
          G4Tubs* solid_vac_her = new G4Tubs("solid_vac_her_" + newParts[i], 0, r_her_hole, tilted_length / 2.0, 0, 2 * M_PI);
          G4Transform3D transform_her = G4Translate3D(x_her, 0, z_her) * G4RotateY3D(angle_her);

          // LER daughter
          double angle_ler = -0.0415;
          double x_ler = z_center_placement * std::sin(angle_ler);
          double z_ler = z_center_placement * std::cos(angle_ler);
          (void)r_ler_hole; // superseded below by the SAD-matched taper

          // (2026-08-06) LER hole radius corrected from a flat 0.5cm to match
          // SAD's own near-IP LER aperture model (SetIRApertLERPostLS2)
          auto sadLerApertureRadiusNative = [](double zNative) {
            const double toCm = Unit::mm / Unit::cm; // undo the *Unit::cm/Unit::mm used to fill Z[]
            double s_cm = std::abs(zNative) * toCm;
            double r_cm = (s_cm <= 28.2) ? (0.569 + (0.8 - 0.569) / 28.2 * s_cm) : 0.8;
            return r_cm * (Unit::cm / Unit::mm);
          };
          double* lerZ_local = new double[num];
          double* lerR_inner = new double[num];
          double* lerR_outer = new double[num];
          for (int j = 0; j < num; ++j) {
            lerZ_local[j] = (Z[j] - z_center) / std::cos(angle_ler);
            lerR_inner[j] = 0.0;
            lerR_outer[j] = sadLerApertureRadiusNative(Z[j]); // uses the true (uncompensated) global Z / SAD s
          }
          G4Polycone* solid_vac_ler = new G4Polycone("solid_vac_ler_" + newParts[i], 0, 2 * M_PI, num,
                                                     lerZ_local, lerR_inner, lerR_outer);
          G4Transform3D transform_ler = G4Translate3D(x_ler, 0, z_ler) * G4RotateY3D(angle_ler);

          G4Transform3D rel_ler_in_her = transform_her.inverse() * transform_ler;
          G4VSolid* solid_vac_union_raw = new G4UnionSolid("solid_vac_union_raw_" + newParts[i], solid_vac_her, solid_vac_ler,
                                                           rel_ler_in_her);
          const double clip_margin = 0.0;
          G4Tubs* solid_z_clip = new G4Tubs("solid_z_clip_" + newParts[i], 0, 20.0 * Unit::cm / Unit::mm,
                                            length / 2.0 + clip_margin, 0, 2 * M_PI);
          G4Transform3D clip_transform_in_mother = G4Translate3D(0, 0, z_center);
          G4Transform3D rel_clip_in_union = transform_her.inverse() * clip_transform_in_mother;
          G4VSolid* solid_vac_union = new G4IntersectionSolid("solid_vac_union_" + newParts[i], solid_vac_union_raw,
                                                              solid_z_clip, rel_clip_in_union);

          G4LogicalVolume* logi_vac_union = new G4LogicalVolume(solid_vac_union, Materials::get("Vacuum"),
                                                                "logi_vac_union_" + newParts[i]);
          if (flag_limitStep) logi_vac_union->SetUserLimits(new G4UserLimits(stepMax));
          setVisibility(*logi_vac_union, false);
          new G4PVPlacement(transform_her, logi_vac_union, "phys_vac_union_" + newParts[i], logi, false, 0);
        }

        if (newParts[i] == "IPChamber_FWD") { logi_IPChamber_FWD = logi; }
        if (newParts[i] == "IPChamber_BWD") { logi_IPChamber_BWD = logi; }
        if (newParts[i] == "IPBeamPipe_FWD") { geo_IPBeamPipe_FWD = geo; logi_IPBeamPipe_FWD = logi; }
        if (newParts[i] == "IPBeamPipe_BWD") { geo_IPBeamPipe_BWD = geo; logi_IPBeamPipe_BWD = logi; }
        if (newParts[i] == "BellowsShield_FWD") { geo_BellowsShield_FWD = geo; logi_BellowsShield_FWD = logi; }
        if (newParts[i] == "BellowsShield_BWD") { logi_BellowsShield_BWD = logi; }
        if (newParts[i] == "BellowsPipe_FWD") { logi_BellowsPipe_FWD = logi; }
        if (newParts[i] == "BellowsPipe_BWD") { logi_BellowsPipe_BWD = logi; }
        if (newParts[i] == "AdditionalShield_FWD") { geo_AdditionalShield_FWD = geo; logi_AdditionalShield_FWD = logi; }
        if (newParts[i] == "AdditionalShield_BWD") { logi_AdditionalShield_BWD = logi; }

        // Place volume at origin, unrotated as requested
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi, "phys_" + newParts[i], &topVolume, false, 0);

        delete[] Z;
        delete[] rI;
        delete[] rO;
      }

      //----------

      // Legacy Run1 crotch absorber (Lv1Ta*/Lv1SUS*/Lv2Vac*/CuFlange*):
      bool enableCrotchAbsorber = (m_config.getParameter("Lv1TaLERUp.Enable", 1.0) != 0);
      G4LogicalVolume* logi_Lv1TaLERUp = nullptr;
      G4LogicalVolume* logi_Lv1SUSLERUp = nullptr;
      G4LogicalVolume* logi_Lv1TaHERDwn = nullptr;
      G4LogicalVolume* logi_Lv1SUSHERDwn = nullptr;
      G4LogicalVolume* logi_Lv1TaHERUp = nullptr;
      G4LogicalVolume* logi_Lv1SUSHERUp = nullptr;
      G4LogicalVolume* logi_Lv1TaLERDwn = nullptr;
      G4LogicalVolume* logi_Lv1SUSLERDwn = nullptr;
      G4LogicalVolume* logi_CuFlangeFwd = nullptr;
      G4LogicalVolume* logi_CuFlangeBwd = nullptr;
      if (enableCrotchAbsorber) {

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
        const int Lv1TaLERUp_num = 12;
        //
        double Lv1TaLERUp_Z[Lv1TaLERUp_num];
        double Lv1TaLERUp_rO[Lv1TaLERUp_num];
        for (int i = 0; i < Lv1TaLERUp_num; i++) {
          ostringstream ossZ_Lv1TaLERUp;
          ossZ_Lv1TaLERUp << "L" << i + 1;

          ostringstream ossR_Lv1TaLERUp;
          ossR_Lv1TaLERUp << "R" << i + 1;

          Lv1TaLERUp_Z[i] = m_config.getParameter(prep + ossZ_Lv1TaLERUp.str()) * Unit::cm / Unit::mm;
          Lv1TaLERUp_rO[i] = m_config.getParameter(prep + ossR_Lv1TaLERUp.str()) * Unit::cm / Unit::mm;
        }
        //
        double Lv1TaLERUp_rI[Lv1TaLERUp_num];
        for (int i = 0; i < Lv1TaLERUp_num; i++)
        { Lv1TaLERUp_rI[i] = 0.0; }
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
        logi_Lv1TaLERUp = new G4LogicalVolume(geo_Lv1TaLERUp, mat_Lv1TaLERUp, "logi_Lv1TaLERUp_name");

        //-   put volume
        setColor(*logi_Lv1TaLERUp, "#0000CC");
        G4Transform3D transform_Lv1TaLERUp = G4Translate3D(0., 0., 0.);
        transform_Lv1TaLERUp = transform_Lv1TaLERUp * G4RotateY3D(Lv1TaLERUp_A1);
        new G4PVPlacement(transform_Lv1TaLERUp, logi_Lv1TaLERUp, "phys_Lv1TaLERUp_name", &topVolume, false, 0);

        //----------
        //-Lv1SUSLERUp
        prep = "Lv1SUSLERUp.";
        const int Lv1SUSLERUp_num = 6;
        double Lv1SUSLERUp_Z[Lv1SUSLERUp_num];
        double Lv1SUSLERUp_rO[Lv1SUSLERUp_num];
        double Lv1SUSLERUp_rI[Lv1SUSLERUp_num];

        for (int i = 0; i < Lv1SUSLERUp_num; i++) {
          ostringstream ossZ_Lv1SUSLERUp;
          ossZ_Lv1SUSLERUp << "Z" << i + 1;
          ostringstream ossRI_Lv1SUSLERUp;
          ossRI_Lv1SUSLERUp << "RI" << i + 1;
          ostringstream ossRO_Lv1SUSLERUp;
          ossRO_Lv1SUSLERUp << "RO" << i + 1;

          Lv1SUSLERUp_Z[i] = m_config.getParameter(prep + ossZ_Lv1SUSLERUp.str()) * Unit::cm / Unit::mm;
          Lv1SUSLERUp_rI[i] = m_config.getParameter(prep + ossRI_Lv1SUSLERUp.str()) * Unit::cm / Unit::mm;
          Lv1SUSLERUp_rO[i] = m_config.getParameter(prep + ossRO_Lv1SUSLERUp.str()) * Unit::cm / Unit::mm;
        }

        string strMat_Lv1SUSLERUp = m_config.getParameterStr(prep + "Material");
        G4Material* mat_Lv1SUSLERUp = Materials::get(strMat_Lv1SUSLERUp);

        G4Polycone* geo_Lv1SUSLERUppcon = new G4Polycone("geo_Lv1SUSLERUppcon_name", 0, 2 * M_PI, Lv1SUSLERUp_num, Lv1SUSLERUp_Z,
                                                         Lv1SUSLERUp_rI, Lv1SUSLERUp_rO);
        G4IntersectionSolid* geo_Lv1SUSLERUp = new G4IntersectionSolid("", geo_Lv1SUSLERUppcon, geo_AreaTubeFwdpcon,
            transform_AreaTubeFwdForLER);
        logi_Lv1SUSLERUp = new G4LogicalVolume(geo_Lv1SUSLERUp, mat_Lv1SUSLERUp, "logi_Lv1SUSLERUp_name");

        //-put volume
        setColor(*logi_Lv1SUSLERUp, "#666666");
        new G4PVPlacement(transform_Lv1TaLERUp, logi_Lv1SUSLERUp, "phys_Lv1SUSLERUp_name", &topVolume, false, 0);

        //----------
        //- Lv2VacLERUp

        //get parameters from .xml file
        prep = "Lv2VacLERUp.";
        //
        string strMat_Lv2VacLERUp = m_config.getParameterStr(prep + "Material");
        G4Material* mat_Lv2VacLERUp = Materials::get(strMat_Lv2VacLERUp);

        // (2026-08-06) Replaced the flat R1=1.0cm circular hole with a shape
        // matching SAD's own LER aperture model (SetIRApertLERPostLS2)
        struct SadEllipseSeg { double zLo, zHi, H, V; };
        std::vector<SadEllipseSeg> lerVacSegs;
        lerVacSegs.push_back({30.0, 33.426, 0.8, 0.8});
        {
          const int nTaper = 10;
          const double z1 = 33.426, z2 = 64.2;
          for (int k = 0; k < nTaper; k++) {
            double zlo = z1 + (z2 - z1) * k / nTaper;
            double zhi = z1 + (z2 - z1) * (k + 1) / nTaper;
            double zmid = 0.5 * (zlo + zhi);
            double H = 0.8 + (1.05 - 0.8) / (z2 - z1) * (zmid - z1);
            double V = 0.8 + (1.55 - 0.8) / (z2 - z1) * (zmid - z1);
            lerVacSegs.push_back({zlo, zhi, H, V});
          }
        }
        lerVacSegs.push_back({64.2, 100.0, 1.05, 1.55});

        G4VSolid* geo_Lv2VacLERUp_shape = nullptr;
        double lerVacFirstCenter = 0.0;
        for (size_t k = 0; k < lerVacSegs.size(); ++k) {
          double halfLen = (lerVacSegs[k].zHi - lerVacSegs[k].zLo) / 2.0 * (Unit::cm / Unit::mm);
          double center  = (lerVacSegs[k].zLo + lerVacSegs[k].zHi) / 2.0 * (Unit::cm / Unit::mm);
          double dx = lerVacSegs[k].H * (Unit::cm / Unit::mm);
          double dy = lerVacSegs[k].V * (Unit::cm / Unit::mm);
          std::ostringstream nm;
          nm << "geo_Lv2VacLERUp_seg" << k;
          G4EllipticalTube* tube = new G4EllipticalTube(nm.str(), dx, dy, halfLen);
          if (k == 0) {
            geo_Lv2VacLERUp_shape = tube;
            lerVacFirstCenter = center;
          } else {
            G4Transform3D rel = G4Translate3D(0, 0, center - lerVacFirstCenter);
            geo_Lv2VacLERUp_shape = new G4UnionSolid(nm.str() + "_u", geo_Lv2VacLERUp_shape, tube, rel);
          }
        }
        // The union above is anchored at segment 0's own local origin (its
        // center, lerVacFirstCenter), not at s=0 -- shift it back so this
        // solid's local z means the same thing Lv1TaLERUp_Z[] does.
        G4VSolid* geo_Lv2VacLERUppcon = new G4DisplacedSolid("geo_Lv2VacLERUp_aligned", geo_Lv2VacLERUp_shape,
                                                             G4Translate3D(0, 0, lerVacFirstCenter));

        //define geometry
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
        const int Lv1TaHERDwn_num = 12;
        //
        double Lv1TaHERDwn_Z[Lv1TaHERDwn_num];
        double Lv1TaHERDwn_rO[Lv1TaHERDwn_num];
        for (int i = 0; i < Lv1TaHERDwn_num; i++) {
          ostringstream ossZ_Lv1TaHERDwn;
          ossZ_Lv1TaHERDwn << "L" << i + 1;

          ostringstream ossR_Lv1TaHERDwn;
          ossR_Lv1TaHERDwn << "R" << i + 1;

          Lv1TaHERDwn_Z[i] = m_config.getParameter(prep + ossZ_Lv1TaHERDwn.str()) * Unit::cm / Unit::mm;
          Lv1TaHERDwn_rO[i] = m_config.getParameter(prep + ossR_Lv1TaHERDwn.str()) * Unit::cm / Unit::mm;
        }
        //
        double Lv1TaHERDwn_rI[Lv1TaHERDwn_num];
        for (int i = 0; i < Lv1TaHERDwn_num; i++)
        { Lv1TaHERDwn_rI[i] = 0.0; }
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
        logi_Lv1TaHERDwn = new G4LogicalVolume(geo_Lv1TaHERDwn, mat_Lv1TaHERDwn, "logi_Lv1TaHERDwn_name");

        //-   put volume
        setColor(*logi_Lv1TaHERDwn, "#00CC00");
        G4Transform3D transform_Lv1TaHERDwn = G4Translate3D(0., 0., 0.);
        transform_Lv1TaHERDwn = transform_Lv1TaHERDwn * G4RotateY3D(Lv1TaHERDwn_A1);
        new G4PVPlacement(transform_Lv1TaHERDwn, logi_Lv1TaHERDwn, "phys_Lv1TaHERDwn_name", &topVolume, false, 0);

        //----------
        //-Lv1SUSHERDwn
        prep = "Lv1SUSHERDwn.";
        const int Lv1SUSHERDwn_num = 6;
        double Lv1SUSHERDwn_Z[Lv1SUSHERDwn_num];
        double Lv1SUSHERDwn_rO[Lv1SUSHERDwn_num];
        double Lv1SUSHERDwn_rI[Lv1SUSHERDwn_num];

        for (int i = 0; i < Lv1SUSHERDwn_num; i++) {
          ostringstream ossZ_Lv1SUSHERDwn;
          ossZ_Lv1SUSHERDwn << "Z" << i + 1;
          ostringstream ossRI_Lv1SUSHERDwn;
          ossRI_Lv1SUSHERDwn << "RI" << i + 1;
          ostringstream ossRO_Lv1SUSHERDwn;
          ossRO_Lv1SUSHERDwn << "RO" << i + 1;

          Lv1SUSHERDwn_Z[i] = m_config.getParameter(prep + ossZ_Lv1SUSHERDwn.str()) * Unit::cm / Unit::mm;
          Lv1SUSHERDwn_rI[i] = m_config.getParameter(prep + ossRI_Lv1SUSHERDwn.str()) * Unit::cm / Unit::mm;
          Lv1SUSHERDwn_rO[i] = m_config.getParameter(prep + ossRO_Lv1SUSHERDwn.str()) * Unit::cm / Unit::mm;
        }

        string strMat_Lv1SUSHERDwn = m_config.getParameterStr(prep + "Material");
        G4Material* mat_Lv1SUSHERDwn = Materials::get(strMat_Lv1SUSHERDwn);
        //G4Material* mat_Lv1SUSHERDwn = mat_Lv1SUS;

        G4Polycone* geo_Lv1SUSHERDwnpcon = new G4Polycone("geo_Lv1SUSHERDwnpcon_name", 0, 2 * M_PI, Lv1SUSHERDwn_num, Lv1SUSHERDwn_Z,
                                                          Lv1SUSHERDwn_rI, Lv1SUSHERDwn_rO);
        G4IntersectionSolid* geo_Lv1SUSHERDwn = new G4IntersectionSolid("", geo_Lv1SUSHERDwnpcon, geo_AreaTubeFwdpcon,
            transform_AreaTubeFwdForHER);
        logi_Lv1SUSHERDwn = new G4LogicalVolume(geo_Lv1SUSHERDwn, mat_Lv1SUSHERDwn, "logi_Lv1SUSHERDwn_name");

        //-put volume
        setColor(*logi_Lv1SUSHERDwn, "#666666");
        new G4PVPlacement(transform_Lv1TaHERDwn, logi_Lv1SUSHERDwn, "phys_Lv1SUSHERDwn_name", &topVolume, false, 0);

        //----------
        //- Lv2VacHERDwn

        //get parameters from .xml file
        prep = "Lv2VacHERDwn.";
        //
        double Lv2VacHERDwn_rO[Lv1TaHERDwn_num];
        for (int i = 0; i < Lv1TaHERDwn_num; i++) {
          Lv2VacHERDwn_rO[i] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
        }
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
        const int Lv1TaHERUp_num = 12;
        double Lv1TaHERUp_Z[Lv1TaHERUp_num];
        double Lv1TaHERUp_rO[Lv1TaHERUp_num];
        for (int i = 0; i < Lv1TaHERUp_num; i++) {
          ostringstream ossZ_Lv1TaHERUp;
          ossZ_Lv1TaHERUp << "L" << i + 1;

          ostringstream ossR_Lv1TaHERUp;
          ossR_Lv1TaHERUp << "R" << i + 1;

          Lv1TaHERUp_Z[i] = -m_config.getParameter(prep + ossZ_Lv1TaHERUp.str()) * Unit::cm / Unit::mm;
          Lv1TaHERUp_rO[i] = m_config.getParameter(prep + ossR_Lv1TaHERUp.str()) * Unit::cm / Unit::mm;
        }
        //
        double Lv1TaHERUp_rI[Lv1TaHERUp_num];
        for (int i = 0; i < Lv1TaHERUp_num; i++)
        { Lv1TaHERUp_rI[i] = 0.0; }
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
        logi_Lv1TaHERUp = new G4LogicalVolume(geo_Lv1TaHERUp, mat_Lv1TaHERUp, "logi_Lv1TaHERUp_name");

        //-   put volume
        setColor(*logi_Lv1TaHERUp, "#00CC00");
        G4Transform3D transform_Lv1TaHERUp = G4Translate3D(0., 0., 0.);
        transform_Lv1TaHERUp = transform_Lv1TaHERUp * G4RotateY3D(Lv1TaHERUp_A1);
        new G4PVPlacement(transform_Lv1TaHERUp, logi_Lv1TaHERUp, "phys_Lv1TaHERUp_name", &topVolume, false, 0);

        //----------
        //-Lv1SUSHERUp
        prep = "Lv1SUSHERUp.";
        const int Lv1SUSHERUp_num = 6;
        double Lv1SUSHERUp_Z[Lv1SUSHERUp_num];
        double Lv1SUSHERUp_rO[Lv1SUSHERUp_num];
        double Lv1SUSHERUp_rI[Lv1SUSHERUp_num];

        for (int i = 0; i < Lv1SUSHERUp_num; i++) {
          ostringstream ossZ_Lv1SUSHERUp;
          ossZ_Lv1SUSHERUp << "Z" << i + 1;
          ostringstream ossRI_Lv1SUSHERUp;
          ossRI_Lv1SUSHERUp << "RI" << i + 1;
          ostringstream ossRO_Lv1SUSHERUp;
          ossRO_Lv1SUSHERUp << "RO" << i + 1;

          Lv1SUSHERUp_Z[i] = -m_config.getParameter(prep + ossZ_Lv1SUSHERUp.str()) * Unit::cm / Unit::mm;
          Lv1SUSHERUp_rI[i] = m_config.getParameter(prep + ossRI_Lv1SUSHERUp.str()) * Unit::cm / Unit::mm;
          Lv1SUSHERUp_rO[i] = m_config.getParameter(prep + ossRO_Lv1SUSHERUp.str()) * Unit::cm / Unit::mm;
        }

        string strMat_Lv1SUSHERUp = m_config.getParameterStr(prep + "Material");
        G4Material* mat_Lv1SUSHERUp = Materials::get(strMat_Lv1SUSHERUp);

        G4Polycone* geo_Lv1SUSHERUppcon = new G4Polycone("geo_Lv1SUSHERUppcon_name", 0, 2 * M_PI, Lv1SUSHERUp_num, Lv1SUSHERUp_Z,
                                                         Lv1SUSHERUp_rI, Lv1SUSHERUp_rO);
        G4IntersectionSolid* geo_Lv1SUSHERUp = new G4IntersectionSolid("", geo_Lv1SUSHERUppcon, geo_AreaTubeBwdpcon,
            transform_AreaTubeFwdForHER);
        logi_Lv1SUSHERUp = new G4LogicalVolume(geo_Lv1SUSHERUp, mat_Lv1SUSHERUp, "logi_Lv1SUSHERUp_name");

        //-put volume
        setColor(*logi_Lv1SUSHERUp, "#666666");
        new G4PVPlacement(transform_Lv1TaHERUp, logi_Lv1SUSHERUp, "phys_Lv1SUSHERUp_name", &topVolume, false, 0);

        //----------
        //- Lv2VacHERUp

        //get parameters from .xml file
        prep =  "Lv2VacHERUp.";
        //
        double Lv2VacHERUp_rO[Lv1TaHERUp_num];
        for (int i = 0; i < Lv1TaHERUp_num; i++) {
          Lv2VacHERUp_rO[i] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
        }
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
        const int Lv1TaLERDwn_num = 12;
        //
        double Lv1TaLERDwn_Z[Lv1TaLERDwn_num];
        double Lv1TaLERDwn_rO[Lv1TaLERDwn_num];
        for (int i = 0; i < Lv1TaLERDwn_num; i++) {
          ostringstream ossZ_Lv1TaLERDwn;
          ossZ_Lv1TaLERDwn << "L" << i + 1;

          ostringstream ossR_Lv1TaLERDwn;
          ossR_Lv1TaLERDwn << "R" << i + 1;

          Lv1TaLERDwn_Z[i] = -m_config.getParameter(prep + ossZ_Lv1TaLERDwn.str()) * Unit::cm / Unit::mm;
          Lv1TaLERDwn_rO[i] = m_config.getParameter(prep + ossR_Lv1TaLERDwn.str()) * Unit::cm / Unit::mm;
        }
        //
        double Lv1TaLERDwn_rI[Lv1TaLERDwn_num];
        for (int i = 0; i < Lv1TaLERDwn_num; i++)
        { Lv1TaLERDwn_rI[i] = 0.0; }
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
        logi_Lv1TaLERDwn = new G4LogicalVolume(geo_Lv1TaLERDwn, mat_Lv1TaLERDwn, "logi_Lv1TaLERDwn_name");

        //-   put volume
        setColor(*logi_Lv1TaLERDwn, "#0000CC");
        G4Transform3D transform_Lv1TaLERDwn = G4Translate3D(0., 0., 0.);
        transform_Lv1TaLERDwn = transform_Lv1TaLERDwn * G4RotateY3D(Lv1TaLERDwn_A1);
        new G4PVPlacement(transform_Lv1TaLERDwn, logi_Lv1TaLERDwn, "phys_Lv1TaLERDwn_name", &topVolume, false, 0);

        //----------
        //-Lv1SUSLERDwn
        prep = "Lv1SUSLERDwn.";
        const int Lv1SUSLERDwn_num = 6;
        double Lv1SUSLERDwn_Z[Lv1SUSLERDwn_num];
        double Lv1SUSLERDwn_rO[Lv1SUSLERDwn_num];
        double Lv1SUSLERDwn_rI[Lv1SUSLERDwn_num];

        for (int i = 0; i < Lv1SUSLERDwn_num; i++) {
          ostringstream ossZ_Lv1SUSLERDwn;
          ossZ_Lv1SUSLERDwn << "Z" << i + 1;
          ostringstream ossRI_Lv1SUSLERDwn;
          ossRI_Lv1SUSLERDwn << "RI" << i + 1;
          ostringstream ossRO_Lv1SUSLERDwn;
          ossRO_Lv1SUSLERDwn << "RO" << i + 1;

          Lv1SUSLERDwn_Z[i] = -m_config.getParameter(prep + ossZ_Lv1SUSLERDwn.str()) * Unit::cm / Unit::mm;
          Lv1SUSLERDwn_rI[i] = m_config.getParameter(prep + ossRI_Lv1SUSLERDwn.str()) * Unit::cm / Unit::mm;
          Lv1SUSLERDwn_rO[i] = m_config.getParameter(prep + ossRO_Lv1SUSLERDwn.str()) * Unit::cm / Unit::mm;
        }

        string strMat_Lv1SUSLERDwn = m_config.getParameterStr(prep + "Material");
        G4Material* mat_Lv1SUSLERDwn = Materials::get(strMat_Lv1SUSLERDwn);

        G4Polycone* geo_Lv1SUSLERDwnpcon = new G4Polycone("geo_Lv1SUSLERDwnpcon_name", 0, 2 * M_PI, Lv1SUSLERDwn_num, Lv1SUSLERDwn_Z,
                                                          Lv1SUSLERDwn_rI, Lv1SUSLERDwn_rO);
        G4IntersectionSolid* geo_Lv1SUSLERDwn = new G4IntersectionSolid("", geo_Lv1SUSLERDwnpcon, geo_AreaTubeBwdpcon,
            transform_AreaTubeFwdForHER);
        logi_Lv1SUSLERDwn = new G4LogicalVolume(geo_Lv1SUSLERDwn, mat_Lv1SUSLERDwn, "logi_Lv1SUSLERDwn_name");

        //-put volume
        setColor(*logi_Lv1SUSLERDwn, "#666666");
        new G4PVPlacement(transform_Lv1TaLERDwn, logi_Lv1SUSLERDwn, "phys_Lv1SUSLERDwn_name", &topVolume, false, 0);

        //----------
        //- Lv2VacLERDwn

        //get parameters from .xml file
        prep = "Lv2VacLERDwn.";
        //
        double Lv2VacLERDwn_rO[Lv1TaLERDwn_num];
        for (int i = 0; i < Lv1TaLERDwn_num; i++) {
          Lv2VacLERDwn_rO[i] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
        }
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
        G4SubtractionSolid* geo_CuFlangeFwd_x3 = new G4SubtractionSolid("geo_CuFlangeFwd_x3_name", geo_CuFlangeFwd_x2, geo_Lv1TaLERUp,
            transform_Lv1TaLERUp);
        G4SubtractionSolid* geo_CuFlangeFwd_x4 = new G4SubtractionSolid("geo_CuFlangeFwd_x4_name",  geo_CuFlangeFwd_x3,  geo_Lv1TaHERDwn,
            transform_Lv1TaHERDwn);
        G4SubtractionSolid* geo_CuFlangeFwd_x5 = new G4SubtractionSolid("geo_CuFlangeFwd_x5_name",  geo_CuFlangeFwd_x4,
            geo_BellowsShield_FWD,
            G4Translate3D(0, 0, 0));
        G4SubtractionSolid* geo_CuFlangeFwd   = new G4SubtractionSolid("geo_CuFlangeFwd_name",  geo_CuFlangeFwd_x5,
            geo_AdditionalShield_FWD,
            G4Translate3D(0, 0, 0));

        logi_CuFlangeFwd = new G4LogicalVolume(geo_CuFlangeFwd, mat_Lv1TaLERUp, "logi_CuFlangeFwd_name");

        //-   put volume
        setColor(*logi_CuFlangeFwd, "#CCCCCC");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_CuFlangeFwd, "phys_CuFlangeFwd_name", &topVolume, false, 0);




        G4IntersectionSolid* geo_CuFlangeBwd_x2 = new G4IntersectionSolid("geo_CuFlangeBwd_x2_name", geo_AreaTubeBwdpcon, geo_Flange,
            G4Translate3D(0, 0, -Flange_D - Flange_T * 2));
        G4SubtractionSolid* geo_CuFlangeBwd_x = new G4SubtractionSolid("geo_CuFlangeBwd_x_name", geo_CuFlangeBwd_x2, geo_Lv1TaHERUp,
            transform_Lv1TaHERUp);
        G4SubtractionSolid* geo_CuFlangeBwd   = new G4SubtractionSolid("geo_CuFlangeBwd_name",  geo_CuFlangeBwd_x,  geo_Lv1TaLERDwn,
            transform_Lv1TaLERDwn);

        logi_CuFlangeBwd = new G4LogicalVolume(geo_CuFlangeBwd, mat_Lv1TaLERUp, "logi_CuFlangeBwd_name");

        //-   put volume
        setColor(*logi_CuFlangeBwd, "#CCCCCC");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_CuFlangeBwd, "phys_CuFlangeBwd_name", &topVolume, false, 0);

      } // enableCrotchAbsorber

      ////==========
      ////= Tangusten End Mounts

#if 0
      //----------
      //- PXDMountFwd

      //get parameters from .xml file
      prep = "PXDMountFwd.";
      //
      double PXDMountFwd_Z1 = m_config.getParameter(prep + "Z1") * Unit::cm / Unit::mm;
      double PXDMountFwd_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double PXDMountFwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double PXDMountFwd_R2 = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      double PXDMountFwd_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double PXDMountFwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //
      string strMat_PXDMountFwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_PXDMountFwd = Materials::get(strMat_PXDMountFwd);

      //define geometry
      G4VSolid* geo_PXDMountFwd_a = new G4Tubs("geo_PXDMountFwd_a", 0, PXDMountFwd_R1, PXDMountFwd_D1 / 2, 0, 2 * M_PI);
      G4VSolid* geo_PXDMountFwd_b = new G4SubtractionSolid("geo_PXDMountFwd_b", geo_PXDMountFwd_a, geo_IPBeamPipe_FWD, G4Translate3D(0.,
                                                           0., -PXDMountFwd_Z1 - PXDMountFwd_D1 / 2.0));
      G4VSolid* geo_PXDMountFwd_c = new G4Tubs("geo_PXDMountFwd_c", 0, PXDMountFwd_R2, 100, 0, 2 * M_PI);
      G4VSolid* geo_PXDMountFwd_d = new G4Box("geo_PXDMountFwd_d", PXDMountFwd_R1 - PXDMountFwd_L1, 100, PXDMountFwd_R2);

      G4VSolid* geo_PXDMountFwd_p1 = new G4SubtractionSolid("geo_PXDMountFwd_p1", geo_PXDMountFwd_b, geo_PXDMountFwd_c,
                                                            G4Translate3D(+PXDMountFwd_L1, 0., +(PXDMountFwd_D1 / 2 - PXDMountFwd_L2))*G4RotateX3D(M_PI / 2));
      G4VSolid* geo_PXDMountFwd_p2 = new G4SubtractionSolid("geo_PXDMountFwd_p2", geo_PXDMountFwd_p1, geo_PXDMountFwd_c,
                                                            G4Translate3D(+PXDMountFwd_L1, 0., -(PXDMountFwd_D1 / 2 - PXDMountFwd_L2))*G4RotateX3D(M_PI / 2));
      G4VSolid* geo_PXDMountFwd_p3 = new G4SubtractionSolid("geo_PXDMountFwd_p3", geo_PXDMountFwd_p2, geo_PXDMountFwd_c,
                                                            G4Translate3D(-PXDMountFwd_L1, 0., +(PXDMountFwd_D1 / 2 - PXDMountFwd_L2))*G4RotateX3D(M_PI / 2));
      G4VSolid* geo_PXDMountFwd_p4 = new G4SubtractionSolid("geo_PXDMountFwd_p4", geo_PXDMountFwd_p3, geo_PXDMountFwd_c,
                                                            G4Translate3D(-PXDMountFwd_L1, 0., -(PXDMountFwd_D1 / 2 - PXDMountFwd_L2))*G4RotateX3D(M_PI / 2));

      G4VSolid* geo_PXDMountFwd_q1 = new G4SubtractionSolid("geo_PXDMountFwd_q1", geo_PXDMountFwd_p4, geo_PXDMountFwd_d,
                                                            G4Translate3D(+PXDMountFwd_R1, 0., +(PXDMountFwd_D1 / 2 - PXDMountFwd_L2)));
      G4VSolid* geo_PXDMountFwd_q2 = new G4SubtractionSolid("geo_PXDMountFwd_q2", geo_PXDMountFwd_q1, geo_PXDMountFwd_d,
                                                            G4Translate3D(+PXDMountFwd_R1, 0., -(PXDMountFwd_D1 / 2 - PXDMountFwd_L2)));
      G4VSolid* geo_PXDMountFwd_q3 = new G4SubtractionSolid("geo_PXDMountFwd_q3", geo_PXDMountFwd_q2, geo_PXDMountFwd_d,
                                                            G4Translate3D(-PXDMountFwd_R1, 0., +(PXDMountFwd_D1 / 2 - PXDMountFwd_L2)));
      G4VSolid* geo_PXDMountFwd_q4 = new G4SubtractionSolid("geo_PXDMountFwd_q4", geo_PXDMountFwd_q3, geo_PXDMountFwd_d,
                                                            G4Translate3D(-PXDMountFwd_R1, 0., -(PXDMountFwd_D1 / 2 - PXDMountFwd_L2)));

      G4VSolid* geo_PXDMountFwd = geo_PXDMountFwd_q4;

      G4LogicalVolume* logi_PXDMountFwd = new G4LogicalVolume(geo_PXDMountFwd, mat_PXDMountFwd, "logi_PXDMountFwd_name");
      setColor(*logi_PXDMountFwd, "#333333");

//       new G4PVPlacement(0, G4ThreeVector(0, 0, +PXDMountFwd_D1 / 2 + PXDMountFwd_Z1), logi_PXDMountFwd, "phys_PXDMountFwd_name",
//                         &topVolume, false, 0);


      //adding the screws
      double PXDMountFixture_screw_radius = 0.2 * Unit::cm / Unit::mm;
      double PXDMountFixture_screw_length = 0.5 * Unit::cm / Unit::mm; //half z
      G4VSolid* geo_PXDMountFwd_s1 = new G4Tubs("geo_PXDMountFwd_s1", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);
      G4VSolid* geo_PXDMountFwd_s2 = new G4Tubs("geo_PXDMountFwd_s2", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);
      G4VSolid* geo_PXDMountFwd_s3 = new G4Tubs("geo_PXDMountFwd_s3", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);
      G4VSolid* geo_PXDMountFwd_s4 = new G4Tubs("geo_PXDMountFwd_s4", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);

      G4Material* mat_PXDMountFwd_s = Materials::get("Cu");

      G4Rotate3D rotate_PXDMountFwd = G4RotateX3D(-M_PI / 2.0 / Unit::rad);
      G4Transform3D transform_PXDMountFwd_s1 = G4Translate3D(+PXDMountFwd_L1, 0,
                                                             PXDMountFwd_Z1 + PXDMountFwd_D1 - PXDMountFwd_L2) * rotate_PXDMountFwd;
      G4Transform3D transform_PXDMountFwd_s2 = G4Translate3D(+PXDMountFwd_L1, 0, PXDMountFwd_Z1 + PXDMountFwd_L2) * rotate_PXDMountFwd;
      G4Transform3D transform_PXDMountFwd_s3 = G4Translate3D(-PXDMountFwd_L1, 0,
                                                             PXDMountFwd_Z1 + PXDMountFwd_D1 - PXDMountFwd_L2) * rotate_PXDMountFwd;
      G4Transform3D transform_PXDMountFwd_s4 = G4Translate3D(-PXDMountFwd_L1, 0, PXDMountFwd_Z1 + PXDMountFwd_L2) * rotate_PXDMountFwd;

      G4LogicalVolume* logi_PXDMountFwd_s1 = new G4LogicalVolume(geo_PXDMountFwd_s1, mat_PXDMountFwd_s, "logi_PXDMountFwd_name_s1");
      G4LogicalVolume* logi_PXDMountFwd_s2 = new G4LogicalVolume(geo_PXDMountFwd_s2, mat_PXDMountFwd_s, "logi_PXDMountFwd_name_s2");
      G4LogicalVolume* logi_PXDMountFwd_s3 = new G4LogicalVolume(geo_PXDMountFwd_s3, mat_PXDMountFwd_s, "logi_PXDMountFwd_name_s3");
      G4LogicalVolume* logi_PXDMountFwd_s4 = new G4LogicalVolume(geo_PXDMountFwd_s4, mat_PXDMountFwd_s, "logi_PXDMountFwd_name_s4");

//       new G4PVPlacement(transform_PXDMountFwd_s1, logi_PXDMountFwd_s1, "phys_PXDMountFwd_name_s1", &topVolume, false, 0);
//       new G4PVPlacement(transform_PXDMountFwd_s2, logi_PXDMountFwd_s2, "phys_PXDMountFwd_name_s2", &topVolume, false, 0);
//       new G4PVPlacement(transform_PXDMountFwd_s3, logi_PXDMountFwd_s3, "phys_PXDMountFwd_name_s3", &topVolume, false, 0);
//       new G4PVPlacement(transform_PXDMountFwd_s4, logi_PXDMountFwd_s4, "phys_PXDMountFwd_name_s4", &topVolume, false, 0);

      //----------
      //- PXDMountFixtureFwd
      prep = "PXDMountFixtureFwd.";
      //
      double PXDMountFixtureFwd_Z1 = m_config.getParameter(prep + "Z1") * Unit::cm / Unit::mm;
      double PXDMountFixtureFwd_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double PXDMountFixtureFwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double PXDMountFixtureFwd_T1 = m_config.getParameter(prep + "T1") * Unit::cm / Unit::mm;
      double PXDMountFixtureFwd_T2 = m_config.getParameter(prep + "T2") * Unit::cm / Unit::mm;
      double PXDMountFixtureFwd_FL = m_config.getParameter(prep + "I1") * Unit::cm / Unit::mm;
      double PXDMountFixtureFwd_inner_cut_phi = PXDMountFixtureFwd_FL / (PXDMountFwd_R1 + PXDMountFixtureFwd_T1);
      double PXDMountFixtureFwd_outter_uni_phi = (1 / 6.0) * M_PI - PXDMountFixtureFwd_FL / PXDMountFixtureFwd_R1;

      string strMat_PXDMountFixtureFwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_PXDMountFixtureFwd = Materials::get(strMat_PXDMountFixtureFwd);

      //define geometry
      G4VSolid* geo_PXDMountFixtureFwd_a = new G4Tubs("geo_PXDMountFixtureFwd_a", 0, PXDMountFixtureFwd_R1 -  PXDMountFixtureFwd_T2,
                                                      PXDMountFixtureFwd_D1 / 2, 0,
                                                      2 * M_PI);
      G4VSolid* geo_PXDMountFixtureFwd_b = new G4Box("geo_PXDMountFixtureFwd_b", PXDMountFixtureFwd_R1 * 0.5, 0.1 * Unit::cm / Unit::mm,
                                                     100);

      G4VSolid* geo_PXDMountFixtureFwd_b1 = new G4Box("geo_PXDMountFixtureFwd_b1",   PXDMountFwd_R1 + PXDMountFixtureFwd_T1,
                                                      PXDMountFixtureFwd_FL * 0.5, 100);
      G4VSolid* geo_PXDMountFixtureFwd_b2 = new G4Tubs("geo_PXDMountFixtureFwd_b2", PXDMountFwd_R1 + PXDMountFixtureFwd_T1,
                                                       PXDMountFixtureFwd_R1,  PXDMountFixtureFwd_D1 / 2,
                                                       0, PXDMountFixtureFwd_outter_uni_phi);
      G4VSolid* geo_PXDMountFixtureFwd_b3 = new G4Box("geo_PXDMountFixtureFwd_b3", 0.2 * Unit::cm / Unit::mm, 0.35 * Unit::cm / Unit::mm,
                                                      100 * Unit::cm / Unit::mm);

      G4VSolid* geo_PXDMountFixtureFwd_a1 = new G4Tubs("geo_PXDMountFixtureFwd_a1", 0, PXDMountFwd_R1, 100, 0, 2 * M_PI);

      G4VSolid* geo_PXDMountFixtureFwd_c1 = new G4SubtractionSolid("geo_PXDMountFixtureFwd_c1", geo_PXDMountFixtureFwd_a,
          geo_PXDMountFixtureFwd_a1);


      G4VSolid* geo_PXDMountFixtureFwd_d1 = geo_PXDMountFixtureFwd_c1;
      for (int i = 0; i < 4; ++i) {

        geo_PXDMountFixtureFwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureFwd_d1", geo_PXDMountFixtureFwd_d1,
                                                           geo_PXDMountFixtureFwd_b1, G4RotateZ3D(i * 0.25 * M_PI));
      }
      geo_PXDMountFixtureFwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureFwd_d1", geo_PXDMountFixtureFwd_d1,
                                                         geo_PXDMountFixtureFwd_b, G4Translate3D(PXDMountFixtureFwd_R1, - PXDMountFixtureFwd_FL * 0.5, 0));
      geo_PXDMountFixtureFwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureFwd_d1", geo_PXDMountFixtureFwd_d1,
                                                         geo_PXDMountFixtureFwd_b, G4Translate3D(-PXDMountFixtureFwd_R1,  PXDMountFixtureFwd_FL * 0.5, 0));


      double PXDMountFixtureFwd_R1_temp = PXDMountFixtureFwd_R1 -  PXDMountFixtureFwd_T2;
      for (int i = 2; i < 7; i += 4) {

        geo_PXDMountFixtureFwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureFwd_d1", geo_PXDMountFixtureFwd_d1,
                                                           geo_PXDMountFixtureFwd_b3,
                                                           G4Translate3D(PXDMountFixtureFwd_R1_temp * cos(i * 0.25 * M_PI - 0.5 * PXDMountFixtureFwd_inner_cut_phi),
                                                               PXDMountFixtureFwd_R1_temp * sin(i * 0.25 * M_PI - 0.5 * PXDMountFixtureFwd_inner_cut_phi),
                                                               0)*G4RotateZ3D(i * 0.25 * M_PI - 0.5 * PXDMountFixtureFwd_inner_cut_phi));

        geo_PXDMountFixtureFwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureFwd_d1", geo_PXDMountFixtureFwd_d1,
                                                           geo_PXDMountFixtureFwd_b3,
                                                           G4Translate3D(PXDMountFixtureFwd_R1_temp * cos(i * 0.25 * M_PI + 0.5 * PXDMountFixtureFwd_inner_cut_phi),
                                                               PXDMountFixtureFwd_R1_temp * sin(i * 0.25 * M_PI + 0.5 * PXDMountFixtureFwd_inner_cut_phi),
                                                               0)*G4RotateZ3D(i * 0.25 * M_PI + 0.5 * PXDMountFixtureFwd_inner_cut_phi));

      }
      G4VSolid* geo_PXDMountFixtureFwd_d2 = geo_PXDMountFixtureFwd_d1;
      for (int i = 0; i < 12; ++i) {
        if (i == 2 || i == 3 || i == 8 || i == 9) continue;
        geo_PXDMountFixtureFwd_d2 = new G4UnionSolid("geo_PXDMountFixtureFwd_d1", geo_PXDMountFixtureFwd_d2,
                                                     geo_PXDMountFixtureFwd_b2, G4RotateZ3D(i * (1 / 6.0) * M_PI + ((1 / 12.0)*M_PI - 0.5 * PXDMountFixtureFwd_outter_uni_phi)));

      }

      G4VSolid* geo_PXDMountFixtureFwd = geo_PXDMountFixtureFwd_d2;
      G4LogicalVolume* logi_PXDMountFixtureFwd = new G4LogicalVolume(geo_PXDMountFixtureFwd, mat_PXDMountFixtureFwd,
          "logi_PXDMountFixtureFwd_name");
      setColor(*logi_PXDMountFixtureFwd, "#333333");

//       new G4PVPlacement(0, G4ThreeVector(0, 0, +PXDMountFixtureFwd_D1 / 2 + PXDMountFixtureFwd_Z1), logi_PXDMountFixtureFwd,
// //                         "phys_PXDMountFixtureFwd_name",
//                         &topVolume, false, 0);
      //----------
      //- PXDMountBwd

      //get parameters from .xml file
      prep = "PXDMountBwd.";
      //
      double PXDMountBwd_Z1 = m_config.getParameter(prep + "Z1") * Unit::cm / Unit::mm;
      double PXDMountBwd_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double PXDMountBwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double PXDMountBwd_R2 = m_config.getParameter(prep + "R2") * Unit::cm / Unit::mm;
      double PXDMountBwd_L1 = m_config.getParameter(prep + "L1") * Unit::cm / Unit::mm;
      double PXDMountBwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //
      string strMat_PXDMountBwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_PXDMountBwd = Materials::get(strMat_PXDMountBwd);

      //define geometry
      G4VSolid* geo_PXDMountBwd_a = new G4Tubs("geo_PXDMountBwd_a", 0, PXDMountBwd_R1, PXDMountBwd_D1 / 2, 0, 2 * M_PI);
      G4VSolid* geo_PXDMountBwd_b = new G4SubtractionSolid("geo_PXDMountBwd_b", geo_PXDMountBwd_a, geo_IPBeamPipe_BWD, G4Translate3D(0.,
                                                           0., PXDMountBwd_Z1 + PXDMountBwd_D1 / 2.0));
      G4VSolid* geo_PXDMountBwd_c = new G4Tubs("geo_PXDMountBwd_c", 0, PXDMountBwd_R2, 100, 0, 2 * M_PI);
      G4VSolid* geo_PXDMountBwd_d = new G4Box("geo_PXDMountBwd_d", PXDMountBwd_R1 - PXDMountBwd_L1, 100, PXDMountBwd_R2);

      G4VSolid* geo_PXDMountBwd_p1 = new G4SubtractionSolid("geo_PXDMountBwd_p1", geo_PXDMountBwd_b, geo_PXDMountBwd_c,
                                                            G4Translate3D(+PXDMountBwd_L1, 0., +(PXDMountBwd_D1 / 2 - PXDMountBwd_L2))*G4RotateX3D(M_PI / 2));
      G4VSolid* geo_PXDMountBwd_p2 = new G4SubtractionSolid("geo_PXDMountBwd_p2", geo_PXDMountBwd_p1, geo_PXDMountBwd_c,
                                                            G4Translate3D(+PXDMountBwd_L1, 0., -(PXDMountBwd_D1 / 2 - PXDMountBwd_L2))*G4RotateX3D(M_PI / 2));
      G4VSolid* geo_PXDMountBwd_p3 = new G4SubtractionSolid("geo_PXDMountBwd_p3", geo_PXDMountBwd_p2, geo_PXDMountBwd_c,
                                                            G4Translate3D(-PXDMountBwd_L1, 0., +(PXDMountBwd_D1 / 2 - PXDMountBwd_L2))*G4RotateX3D(M_PI / 2));
      G4VSolid* geo_PXDMountBwd_p4 = new G4SubtractionSolid("geo_PXDMountBwd_p4", geo_PXDMountBwd_p3, geo_PXDMountBwd_c,
                                                            G4Translate3D(-PXDMountBwd_L1, 0., -(PXDMountBwd_D1 / 2 - PXDMountBwd_L2))*G4RotateX3D(M_PI / 2));

      G4VSolid* geo_PXDMountBwd_q1 = new G4SubtractionSolid("geo_PXDMountBwd_q1", geo_PXDMountBwd_p4, geo_PXDMountBwd_d,
                                                            G4Translate3D(+PXDMountBwd_R1, 0., +(PXDMountBwd_D1 / 2 - PXDMountBwd_L2)));
      G4VSolid* geo_PXDMountBwd_q2 = new G4SubtractionSolid("geo_PXDMountBwd_q2", geo_PXDMountBwd_q1, geo_PXDMountBwd_d,
                                                            G4Translate3D(+PXDMountBwd_R1, 0., -(PXDMountBwd_D1 / 2 - PXDMountBwd_L2)));
      G4VSolid* geo_PXDMountBwd_q3 = new G4SubtractionSolid("geo_PXDMountBwd_q3", geo_PXDMountBwd_q2, geo_PXDMountBwd_d,
                                                            G4Translate3D(-PXDMountBwd_R1, 0., +(PXDMountBwd_D1 / 2 - PXDMountBwd_L2)));
      G4VSolid* geo_PXDMountBwd_q4 = new G4SubtractionSolid("geo_PXDMountBwd_q4", geo_PXDMountBwd_q3, geo_PXDMountBwd_d,
                                                            G4Translate3D(-PXDMountBwd_R1, 0., -(PXDMountBwd_D1 / 2 - PXDMountBwd_L2)));

      G4VSolid* geo_PXDMountBwd = geo_PXDMountBwd_q4;

      G4LogicalVolume* logi_PXDMountBwd = new G4LogicalVolume(geo_PXDMountBwd, mat_PXDMountBwd, "logi_PXDMountBwd_name");
      setColor(*logi_PXDMountBwd, "#333333");

//       new G4PVPlacement(0, G4ThreeVector(0, 0, -PXDMountBwd_D1 / 2 - PXDMountBwd_Z1), logi_PXDMountBwd, "phys_PXDMountBwd_name",
//                         &topVolume, false, 0);

      // adding screws
      //
      G4VSolid* geo_PXDMountBwd_s1 = new G4Tubs("geo_PXDMountBwd_s1", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);
      G4VSolid* geo_PXDMountBwd_s2 = new G4Tubs("geo_PXDMountBwd_s2", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);
      G4VSolid* geo_PXDMountBwd_s3 = new G4Tubs("geo_PXDMountBwd_s3", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);
      G4VSolid* geo_PXDMountBwd_s4 = new G4Tubs("geo_PXDMountBwd_s4", 0, PXDMountFixture_screw_radius, PXDMountFixture_screw_length, 0,
                                                2 * M_PI);

      G4Material* mat_PXDMountBwd_s = Materials::get("Cu");

      G4Rotate3D rotate_PXDMountBwd = G4RotateX3D(-M_PI / 2.0 / Unit::rad);
      G4Transform3D transform_PXDMountBwd_s1 = G4Translate3D(+PXDMountBwd_L1, 0,
                                                             -PXDMountBwd_Z1 - PXDMountBwd_D1  + PXDMountBwd_L2) * rotate_PXDMountBwd;
      G4Transform3D transform_PXDMountBwd_s2 = G4Translate3D(+PXDMountBwd_L1, 0, -PXDMountBwd_Z1 - PXDMountBwd_L2) * rotate_PXDMountBwd;
      G4Transform3D transform_PXDMountBwd_s3 = G4Translate3D(-PXDMountBwd_L1, 0,
                                                             -PXDMountBwd_Z1 - PXDMountBwd_D1 + PXDMountBwd_L2) * rotate_PXDMountBwd;
      G4Transform3D transform_PXDMountBwd_s4 = G4Translate3D(-PXDMountBwd_L1, 0, -PXDMountBwd_Z1 - PXDMountBwd_L2) * rotate_PXDMountBwd;

      G4LogicalVolume* logi_PXDMountBwd_s1 = new G4LogicalVolume(geo_PXDMountBwd_s1, mat_PXDMountBwd_s, "logi_PXDMountBwd_name_s1");
      G4LogicalVolume* logi_PXDMountBwd_s2 = new G4LogicalVolume(geo_PXDMountBwd_s2, mat_PXDMountBwd_s, "logi_PXDMountBwd_name_s2");
      G4LogicalVolume* logi_PXDMountBwd_s3 = new G4LogicalVolume(geo_PXDMountBwd_s3, mat_PXDMountBwd_s, "logi_PXDMountBwd_name_s3");
      G4LogicalVolume* logi_PXDMountBwd_s4 = new G4LogicalVolume(geo_PXDMountBwd_s4, mat_PXDMountBwd_s, "logi_PXDMountBwd_name_s4");

//       new G4PVPlacement(transform_PXDMountBwd_s1, logi_PXDMountBwd_s1, "phys_PXDMountBwd_name_s1", &topVolume, false, 0);
//       new G4PVPlacement(transform_PXDMountBwd_s2, logi_PXDMountBwd_s2, "phys_PXDMountBwd_name_s2", &topVolume, false, 0);
//       new G4PVPlacement(transform_PXDMountBwd_s3, logi_PXDMountBwd_s3, "phys_PXDMountBwd_name_s3", &topVolume, false, 0);
//       new G4PVPlacement(transform_PXDMountBwd_s4, logi_PXDMountBwd_s4, "phys_PXDMountBwd_name_s4", &topVolume, false, 0);

      //----------
      //- PXDMountFixtureBwd
      prep = "PXDMountFixtureBwd.";
      //
      double PXDMountFixtureBwd_Z1 = m_config.getParameter(prep + "Z1") * Unit::cm / Unit::mm;
      double PXDMountFixtureBwd_R1 = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      double PXDMountFixtureBwd_D1 = m_config.getParameter(prep + "D1") * Unit::cm / Unit::mm;
      double PXDMountFixtureBwd_T1 = m_config.getParameter(prep + "T1") * Unit::cm / Unit::mm;
      double PXDMountFixtureBwd_T2 = m_config.getParameter(prep + "T2") * Unit::cm / Unit::mm;
      double PXDMountFixtureBwd_FL = m_config.getParameter(prep + "I1") * Unit::cm / Unit::mm;
      double PXDMountFixtureBwd_inner_cut_phi = PXDMountFixtureBwd_FL / (PXDMountBwd_R1 + PXDMountFixtureBwd_T1);
      double PXDMountFixtureBwd_outter_uni_phi = (1 / 6.0) * M_PI - PXDMountFixtureBwd_FL / PXDMountFixtureBwd_R1;

      string strMat_PXDMountFixtureBwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_PXDMountFixtureBwd = Materials::get(strMat_PXDMountFixtureBwd);

      //define geometry
      G4VSolid* geo_PXDMountFixtureBwd_a = new G4Tubs("geo_PXDMountFixtureBwd_a", 0, PXDMountFixtureBwd_R1 -  PXDMountFixtureBwd_T2,
                                                      PXDMountFixtureBwd_D1 / 2, 0,
                                                      2 * M_PI);
      G4VSolid* geo_PXDMountFixtureBwd_b = new G4Box("geo_PXDMountFixtureBwd_b", PXDMountFixtureBwd_R1 * 0.5, 0.1 * Unit::cm / Unit::mm,
                                                     100);

      G4VSolid* geo_PXDMountFixtureBwd_b1 = new G4Box("geo_PXDMountFixtureBwd_b1",   PXDMountBwd_R1 + PXDMountFixtureBwd_T1,
                                                      PXDMountFixtureBwd_FL * 0.5, 100);
      G4VSolid* geo_PXDMountFixtureBwd_b2 = new G4Tubs("geo_PXDMountFixtureBwd_b2", PXDMountBwd_R1 + PXDMountFixtureBwd_T1,
                                                       PXDMountFixtureBwd_R1,  PXDMountFixtureBwd_D1 / 2,
                                                       0, PXDMountFixtureBwd_outter_uni_phi);
      G4VSolid* geo_PXDMountFixtureBwd_b3 = new G4Box("geo_PXDMountFixtureBwd_b3", 0.2 * Unit::cm / Unit::mm, 0.35 * Unit::cm / Unit::mm,
                                                      100 * Unit::cm / Unit::mm);

      G4VSolid* geo_PXDMountFixtureBwd_a1 = new G4Tubs("geo_PXDMountFixtureBwd_a1", 0, PXDMountBwd_R1, 100, 0, 2 * M_PI);

      G4VSolid* geo_PXDMountFixtureBwd_c1 = new G4SubtractionSolid("geo_PXDMountFixtureBwd_c1", geo_PXDMountFixtureBwd_a,
          geo_PXDMountFixtureBwd_a1);


      G4VSolid* geo_PXDMountFixtureBwd_d1 = geo_PXDMountFixtureBwd_c1;
      for (int i = 0; i < 4; ++i) {

        geo_PXDMountFixtureBwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureBwd_d1", geo_PXDMountFixtureBwd_d1,
                                                           geo_PXDMountFixtureBwd_b1, G4RotateZ3D(i * 0.25 * M_PI));
      }
      geo_PXDMountFixtureBwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureBwd_d1", geo_PXDMountFixtureBwd_d1,
                                                         geo_PXDMountFixtureBwd_b, G4Translate3D(PXDMountFixtureBwd_R1, - PXDMountFixtureBwd_FL * 0.5, 0));
      geo_PXDMountFixtureBwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureBwd_d1", geo_PXDMountFixtureBwd_d1,
                                                         geo_PXDMountFixtureBwd_b, G4Translate3D(-PXDMountFixtureBwd_R1,  PXDMountFixtureBwd_FL * 0.5, 0));


      double PXDMountFixtureBwd_R1_temp = PXDMountFixtureBwd_R1 -  PXDMountFixtureBwd_T2;
      for (int i = 2; i < 7; i += 4) {

        geo_PXDMountFixtureBwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureBwd_d1", geo_PXDMountFixtureBwd_d1,
                                                           geo_PXDMountFixtureBwd_b3,
                                                           G4Translate3D(PXDMountFixtureBwd_R1_temp * cos(i * 0.25 * M_PI - 0.5 * PXDMountFixtureBwd_inner_cut_phi),
                                                               PXDMountFixtureBwd_R1_temp * sin(i * 0.25 * M_PI - 0.5 * PXDMountFixtureBwd_inner_cut_phi),
                                                               0)*G4RotateZ3D(i * 0.25 * M_PI - 0.5 * PXDMountFixtureBwd_inner_cut_phi));

        geo_PXDMountFixtureBwd_d1 = new G4SubtractionSolid("geo_PXDMountFixtureBwd_d1", geo_PXDMountFixtureBwd_d1,
                                                           geo_PXDMountFixtureBwd_b3,
                                                           G4Translate3D(PXDMountFixtureBwd_R1_temp * cos(i * 0.25 * M_PI + 0.5 * PXDMountFixtureBwd_inner_cut_phi),
                                                               PXDMountFixtureBwd_R1_temp * sin(i * 0.25 * M_PI + 0.5 * PXDMountFixtureBwd_inner_cut_phi),
                                                               0)*G4RotateZ3D(i * 0.25 * M_PI + 0.5 * PXDMountFixtureBwd_inner_cut_phi));

      }
      G4VSolid* geo_PXDMountFixtureBwd_d2 = geo_PXDMountFixtureBwd_d1;
      for (int i = 0; i < 12; ++i) {
        if (i == 2 || i == 3 || i == 8 || i == 9) continue;
        geo_PXDMountFixtureBwd_d2 = new G4UnionSolid("geo_PXDMountFixtureBwd_d1", geo_PXDMountFixtureBwd_d2,
                                                     geo_PXDMountFixtureBwd_b2, G4RotateZ3D(i * (1 / 6.0) * M_PI + ((1 / 12.0)*M_PI - 0.5 * PXDMountFixtureBwd_outter_uni_phi)));

      }

      G4VSolid* geo_PXDMountFixtureBwd = geo_PXDMountFixtureBwd_d2;
      G4LogicalVolume* logi_PXDMountFixtureBwd = new G4LogicalVolume(geo_PXDMountFixtureBwd, mat_PXDMountFixtureBwd,
          "logi_PXDMountFixtureBwd_name");
      setColor(*logi_PXDMountFixtureBwd, "#333333");

//       new G4PVPlacement(0, G4ThreeVector(0, 0, -PXDMountFixtureBwd_D1 / 2 - PXDMountFixtureBwd_Z1), logi_PXDMountFixtureBwd,
// //                         "phys_PXDMountFixtureBwd_name",
//                         &topVolume, false, 0);
#endif

      //---------------------------
      // for dose simulation
      //---------------------------

      int Index_sensi = 11;
      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv3AuCoat) logi_Lv3AuCoat->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_IPChamber_FWD) logi_IPChamber_FWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_IPChamber_BWD) logi_IPChamber_BWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_IPBeamPipe_FWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_IPBeamPipe_BWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1TaLERUp) logi_Lv1TaLERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1SUSLERUp) logi_Lv1SUSLERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1TaHERDwn) logi_Lv1TaHERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1SUSHERDwn) logi_Lv1SUSHERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1TaHERUp) logi_Lv1TaHERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1SUSHERUp) logi_Lv1SUSHERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1TaLERDwn) logi_Lv1TaLERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_Lv1SUSLERDwn) logi_Lv1SUSLERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_CuFlangeFwd) logi_CuFlangeFwd->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_CuFlangeBwd) logi_CuFlangeBwd->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_BellowsPipe_FWD) logi_BellowsPipe_FWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_BellowsPipe_BWD) logi_BellowsPipe_BWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_BellowsShield_FWD) logi_BellowsShield_FWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_BellowsShield_BWD) logi_BellowsShield_BWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_AdditionalShield_FWD) logi_AdditionalShield_FWD->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      if (logi_AdditionalShield_BWD) logi_AdditionalShield_BWD->SetSensitiveDetector(m_sensitive.back());

      //-
      //----------

      ////=
      ////==========
    }
  }
}
