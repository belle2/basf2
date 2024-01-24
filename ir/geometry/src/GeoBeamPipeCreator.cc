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
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

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
      //double Lv1TaFwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //double Lv1TaFwd_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv1TaFwd_T1 = m_config.getParameter(prep + "T1") * Unit::cm / Unit::mm;
      double Lv1TaFwd_aR1 = m_config.getParameter(prep + "aR1") * Unit::cm / Unit::mm;
      double Lv1TaFwd_aR2 = m_config.getParameter(prep + "aR2") * Unit::cm / Unit::mm;
      double Lv1TaFwd_aL1 = m_config.getParameter(prep + "aL1") * Unit::cm / Unit::mm;
      double Lv1TaFwd_aL2 = m_config.getParameter(prep + "aL2") * Unit::cm / Unit::mm;
      double Lv1TaFwd_aL3 = m_config.getParameter(prep + "aL3") * Unit::cm / Unit::mm;
      double Lv1TaFwd_bL1 = Lv1TaFwd_L1 - Lv1TaFwd_aL1 - Lv1TaFwd_aL2 - Lv1TaFwd_aL3;
      double Lv1TaFwd_bL2 = m_config.getParameter(prep + "bL2") * Unit::cm / Unit::mm;
      double Lv1TaFwd_bL3 = m_config.getParameter(prep + "bL3") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaFwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaFwd = Materials::get(strMat_Lv1TaFwd);

      //define geometry
      //G4Trd* geo_Lv1TaFwd_xx = new G4Trd("geo_Lv1TaFwd_xx_name", Lv1TaFwd_L2, Lv1TaFwd_L3, Lv1TaFwd_T1, Lv1TaFwd_T1, Lv1TaFwd_L1 / 2.0);
      double Lv1TaFwd_aR[4] = {Lv1TaFwd_aR1, Lv1TaFwd_aR1, Lv1TaFwd_aR2, Lv1TaFwd_aR2};
      double Lv1TaFwd_ar[4] = {0, 0, 0, 0};
      double Lv1TaFwd_aL[4] = { -Lv1TaFwd_L1 / 2.0 + 0,
                                -Lv1TaFwd_L1 / 2.0 + Lv1TaFwd_aL1,
                                -Lv1TaFwd_L1 / 2.0 + Lv1TaFwd_aL1 + Lv1TaFwd_aL2,
                                -Lv1TaFwd_L1 / 2.0 + Lv1TaFwd_aL1 + Lv1TaFwd_aL2 + Lv1TaFwd_aL3
                              };
      G4VSolid* geo_Lv1TaFwd_a = new G4Polycone("geo_Lv1TaFwd_a_name", 0, 2 * M_PI, 4, Lv1TaFwd_aL, Lv1TaFwd_ar, Lv1TaFwd_aR);
      G4VSolid* geo_Lv1TaFwd_b = new G4Trd("geo_Lv1TaFwd_b_name", Lv1TaFwd_bL2, Lv1TaFwd_bL3, Lv1TaFwd_T1, Lv1TaFwd_T1,
                                           Lv1TaFwd_bL1 / 2.0);
      G4VSolid* geo_Lv1TaFwd_c = new G4Box("geo_Lv1TaFwd_c_name", 200, Lv1TaFwd_T1, (Lv1TaFwd_aL1 + Lv1TaFwd_aL2 + Lv1TaFwd_aL3) / 2.0);
      G4VSolid* geo_Lv1TaFwd_d = new G4IntersectionSolid("geo_Lv1TaFwd_d_name", geo_Lv1TaFwd_a, geo_Lv1TaFwd_c, G4Translate3D(0, 0,
                                                         -Lv1TaFwd_L1 / 2.0 + (Lv1TaFwd_aL1 + Lv1TaFwd_aL2 + Lv1TaFwd_aL3) / 2.0));
      G4VSolid* geo_Lv1TaFwd_xx = new G4UnionSolid("geo_Lv1TaFwd_xx_name", geo_Lv1TaFwd_d, geo_Lv1TaFwd_b, G4Translate3D(0, 0,
                                                   Lv1TaFwd_L1 / 2.0 - Lv1TaFwd_bL1 / 2.0));
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
                                                                2.*Lv2VacFwd_A1)) / 2.,
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
      //double Lv1TaBwd_L2 = m_config.getParameter(prep + "L2") * Unit::cm / Unit::mm;
      //double Lv1TaBwd_L3 = m_config.getParameter(prep + "L3") * Unit::cm / Unit::mm;
      double Lv1TaBwd_T1 = m_config.getParameter(prep + "T1") * Unit::cm / Unit::mm;
      double Lv1TaBwd_aR1 = m_config.getParameter(prep + "aR1") * Unit::cm / Unit::mm;
      double Lv1TaBwd_aR2 = m_config.getParameter(prep + "aR2") * Unit::cm / Unit::mm;
      double Lv1TaBwd_aL1 = m_config.getParameter(prep + "aL1") * Unit::cm / Unit::mm;
      double Lv1TaBwd_aL2 = m_config.getParameter(prep + "aL2") * Unit::cm / Unit::mm;
      double Lv1TaBwd_aL3 = m_config.getParameter(prep + "aL3") * Unit::cm / Unit::mm;
      double Lv1TaBwd_bL1 = Lv1TaBwd_L1 - Lv1TaBwd_aL1 - Lv1TaBwd_aL2 - Lv1TaBwd_aL3;
      double Lv1TaBwd_bL2 = m_config.getParameter(prep + "bL2") * Unit::cm / Unit::mm;
      double Lv1TaBwd_bL3 = m_config.getParameter(prep + "bL3") * Unit::cm / Unit::mm;
      //
      string strMat_Lv1TaBwd = m_config.getParameterStr(prep + "Material");
      G4Material* mat_Lv1TaBwd = Materials::get(strMat_Lv1TaBwd);

      //define geometry
      //G4Trd* geo_Lv1TaBwd_x = new G4Trd("geo_Lv1TaBwd_x_name", Lv1TaBwd_L2, Lv1TaBwd_L3, Lv1TaBwd_T1, Lv1TaBwd_T1, Lv1TaBwd_L1 / 2.0);
      double Lv1TaBwd_aR[4] = {Lv1TaBwd_aR2, Lv1TaBwd_aR2, Lv1TaBwd_aR1, Lv1TaBwd_aR1};
      double Lv1TaBwd_ar[4] = {0, 0, 0, 0};
      double Lv1TaBwd_aL[4] = { +Lv1TaBwd_L1 / 2.0 - (Lv1TaBwd_aL1 + Lv1TaBwd_aL2 + Lv1TaBwd_aL3),
                                +Lv1TaBwd_L1 / 2.0 - (Lv1TaBwd_aL1 + Lv1TaBwd_aL2),
                                +Lv1TaBwd_L1 / 2.0 - (Lv1TaBwd_aL1),
                                +Lv1TaBwd_L1 / 2.0 - 0
                              };
      G4VSolid* geo_Lv1TaBwd_a = new G4Polycone("geo_Lv1TaBwd_a_name", 0, 2 * M_PI, 4, Lv1TaBwd_aL, Lv1TaBwd_ar, Lv1TaBwd_aR);
      G4VSolid* geo_Lv1TaBwd_b = new G4Trd("geo_Lv1TaBwd_b_name", Lv1TaBwd_bL2, Lv1TaBwd_bL3, Lv1TaBwd_T1, Lv1TaBwd_T1,
                                           Lv1TaBwd_bL1 / 2.0);
      G4VSolid* geo_Lv1TaBwd_c = new G4Box("geo_Lv1TaBwd_c_name", 200, Lv1TaBwd_T1, (Lv1TaBwd_aL1 + Lv1TaBwd_aL2 + Lv1TaBwd_aL3) / 2.0);
      G4VSolid* geo_Lv1TaBwd_d = new G4IntersectionSolid("geo_Lv1TaBwd_d_name", geo_Lv1TaBwd_a, geo_Lv1TaBwd_c, G4Translate3D(0, 0,
                                                         +Lv1TaBwd_L1 / 2.0 - (Lv1TaBwd_aL1 + Lv1TaBwd_aL2 + Lv1TaBwd_aL3) / 2.0));
      G4VSolid* geo_Lv1TaBwd_x = new G4UnionSolid("geo_Lv1TaBwd_x_name", geo_Lv1TaBwd_d, geo_Lv1TaBwd_b, G4Translate3D(0, 0,
                                                  -Lv1TaBwd_L1 / 2.0 + Lv1TaBwd_bL1 / 2.0));
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
                                                                2.*Lv2VacBwd_A1)) / 2.,
                                                             0.,
                                                             -(Lv2VacBwd_D1 * cos(Lv2VacBwd_A1) + Lv2VacBwd_D2 * cos(2.*Lv2VacBwd_A1)) / 2.);
      //G4Transform3D transform_Lv2VacBwdPart1 = G4Translate3D((Lv2VacBwd_D1 * sin(Lv2VacBwd_A1) + Lv2VacBwd_D2 * sin(2.*Lv2VacBwd_A1)) / 2.05,
      //                                                       0.,
      //                                                       -(Lv2VacBwd_D1 * cos(Lv2VacBwd_A1) + Lv2VacBwd_D2 * cos(2.*Lv2VacBwd_A1)) / 2.05);
      transform_Lv2VacBwdPart1 = transform_Lv2VacBwdPart1 * G4RotateY3D(-Lv2VacBwd_A1 - Lv2VacBwd_A2);
      //tmp end
      //
      G4Transform3D transform_Lv2VacBwdPart2 = G4Translate3D((Lv2VacBwd_D2 + Lv2VacBwd_L1 / 2.0) * sin(2.*Lv2VacBwd_A1),
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
      G4LogicalVolume* logi_Lv1TaLERUp = new G4LogicalVolume(geo_Lv1TaLERUp, mat_Lv1TaLERUp, "logi_Lv1TaLERUp_name");

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
      G4LogicalVolume* logi_Lv1SUSLERUp = new G4LogicalVolume(geo_Lv1SUSLERUp, mat_Lv1SUSLERUp, "logi_Lv1SUSLERUp_name");

      //-put volumn
      setColor(*logi_Lv1SUSLERUp, "#666666");
      new G4PVPlacement(transform_Lv1TaLERUp, logi_Lv1SUSLERUp, "phys_Lv1SUSLERUp_name", &topVolume, false, 0);

      //----------
      //- Lv2VacLERUp

      //get parameters from .xml file
      prep = "Lv2VacLERUp.";
      //
      double Lv2VacLERUp_rO[Lv1TaLERUp_num];
      for (int i = 0; i < Lv1TaLERUp_num; i++) {
        Lv2VacLERUp_rO[i] = m_config.getParameter(prep + "R1") * Unit::cm / Unit::mm;
      }
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
      G4LogicalVolume* logi_Lv1TaHERDwn = new G4LogicalVolume(geo_Lv1TaHERDwn, mat_Lv1TaHERDwn, "logi_Lv1TaHERDwn_name");

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
      G4LogicalVolume* logi_Lv1SUSHERDwn = new G4LogicalVolume(geo_Lv1SUSHERDwn, mat_Lv1SUSHERDwn, "logi_Lv1SUSHERDwn_name");

      //-put volumn
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
      G4LogicalVolume* logi_Lv1TaHERUp = new G4LogicalVolume(geo_Lv1TaHERUp, mat_Lv1TaHERUp, "logi_Lv1TaHERUp_name");

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
      G4LogicalVolume* logi_Lv1SUSHERUp = new G4LogicalVolume(geo_Lv1SUSHERUp, mat_Lv1SUSHERUp, "logi_Lv1SUSHERUp_name");

      //-put volumn
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
      G4LogicalVolume* logi_Lv1TaLERDwn = new G4LogicalVolume(geo_Lv1TaLERDwn, mat_Lv1TaLERDwn, "logi_Lv1TaLERDwn_name");

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
      G4LogicalVolume* logi_Lv1SUSLERDwn = new G4LogicalVolume(geo_Lv1SUSLERDwn, mat_Lv1SUSLERDwn, "logi_Lv1SUSLERDwn_name");

      //-put volumn
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

      ////==========
      ////= Tangusten End Mounts

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
      G4VSolid* geo_PXDMountFwd_b = new G4SubtractionSolid("geo_PXDMountFwd_b", geo_PXDMountFwd_a, geo_Lv1TaFwd_d, G4Translate3D(0., 0.,
                                                           -(-Lv1TaFwd_L1 / 2.0 + Lv1TaFwd_aL1 + Lv1TaFwd_aL2 + Lv1TaFwd_aL3) + PXDMountFwd_D1 / 2));
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

      new G4PVPlacement(0, G4ThreeVector(0, 0, +PXDMountFwd_D1 / 2 + PXDMountFwd_Z1), logi_PXDMountFwd, "phys_PXDMountFwd_name",
                        &topVolume, false, 0);


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

      new G4PVPlacement(transform_PXDMountFwd_s1, logi_PXDMountFwd_s1, "phys_PXDMountFwd_name_s1", &topVolume, false, 0);
      new G4PVPlacement(transform_PXDMountFwd_s2, logi_PXDMountFwd_s2, "phys_PXDMountFwd_name_s2", &topVolume, false, 0);
      new G4PVPlacement(transform_PXDMountFwd_s3, logi_PXDMountFwd_s3, "phys_PXDMountFwd_name_s3", &topVolume, false, 0);
      new G4PVPlacement(transform_PXDMountFwd_s4, logi_PXDMountFwd_s4, "phys_PXDMountFwd_name_s4", &topVolume, false, 0);

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

      new G4PVPlacement(0, G4ThreeVector(0, 0, +PXDMountFixtureFwd_D1 / 2 + PXDMountFixtureFwd_Z1), logi_PXDMountFixtureFwd,
                        "phys_PXDMountFixtureFwd_name",
                        &topVolume, false, 0);
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
      G4VSolid* geo_PXDMountBwd_b = new G4SubtractionSolid("geo_PXDMountBwd_b", geo_PXDMountBwd_a, geo_Lv1TaBwd_d, G4Translate3D(0., 0.,
                                                           -(+Lv1TaBwd_L1 / 2.0 - (Lv1TaBwd_aL1 + Lv1TaBwd_aL2 + Lv1TaBwd_aL3)) - PXDMountBwd_D1 / 2));
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

      new G4PVPlacement(0, G4ThreeVector(0, 0, -PXDMountBwd_D1 / 2 - PXDMountBwd_Z1), logi_PXDMountBwd, "phys_PXDMountBwd_name",
                        &topVolume, false, 0);

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

      new G4PVPlacement(transform_PXDMountBwd_s1, logi_PXDMountBwd_s1, "phys_PXDMountBwd_name_s1", &topVolume, false, 0);
      new G4PVPlacement(transform_PXDMountBwd_s2, logi_PXDMountBwd_s2, "phys_PXDMountBwd_name_s2", &topVolume, false, 0);
      new G4PVPlacement(transform_PXDMountBwd_s3, logi_PXDMountBwd_s3, "phys_PXDMountBwd_name_s3", &topVolume, false, 0);
      new G4PVPlacement(transform_PXDMountBwd_s4, logi_PXDMountBwd_s4, "phys_PXDMountBwd_name_s4", &topVolume, false, 0);

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

      new G4PVPlacement(0, G4ThreeVector(0, 0, -PXDMountFixtureBwd_D1 / 2 - PXDMountFixtureBwd_Z1), logi_PXDMountFixtureBwd,
                        "phys_PXDMountFixtureBwd_name",
                        &topVolume, false, 0);


      //---------------------------
      // for dose simulation
      //---------------------------

      int Index_sensi = 11;
      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv3AuCoat->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1TaFwd->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1TaBwd->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1TaLERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1SUSLERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1TaHERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1SUSHERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1TaHERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1SUSHERUp->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1TaLERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_Lv1SUSLERDwn->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_CuFlangeFwd->SetSensitiveDetector(m_sensitive.back());

      m_sensitive.push_back((SensitiveDetector*)(new BkgSensitiveDetector("IR", Index_sensi++)));
      logi_CuFlangeBwd->SetSensitiveDetector(m_sensitive.back());

      //-
      //----------

      ////=
      ////==========
    }
  }
}
