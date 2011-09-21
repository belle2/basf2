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

#include <ir/geometry/GeoBeamPipeCreator.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <ir/simulation/SensitiveDetector.h>

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
      BOOST_FOREACH(SensitiveDetector* sensitive, m_sensitive) {
        delete sensitive;
      }
      m_sensitive.clear();
    }

    void GeoBeamPipeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
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

      //==========
      //= IP pipe

      //----------
      //- Lv1SUS

      //get parameters from .xml file
      GearDir cLv1SUS(content, "Lv1SUS/");
      //
      const int Lv1SUS_num = 21;
      //
      double Lv1SUS_Z[Lv1SUS_num];
      Lv1SUS_Z[0] = 0.0;
      for (int tmpn = 0; tmpn < 8; tmpn++) {
        Lv1SUS_Z[0] -= cLv1SUS.getLength((format("L%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      Lv1SUS_Z[1] = Lv1SUS_Z[0] + cLv1SUS.getLength("L1") / Unit::mm;
      Lv1SUS_Z[2] = Lv1SUS_Z[1]; Lv1SUS_Z[3] = Lv1SUS_Z[2] + cLv1SUS.getLength("L2") / Unit::mm;
      Lv1SUS_Z[4] = Lv1SUS_Z[3] + cLv1SUS.getLength("L3") / Unit::mm;
      Lv1SUS_Z[5] = Lv1SUS_Z[4];
      Lv1SUS_Z[6] = Lv1SUS_Z[5] + cLv1SUS.getLength("L4") / Unit::mm;
      Lv1SUS_Z[7] = Lv1SUS_Z[6] + cLv1SUS.getLength("L5") / Unit::mm;
      Lv1SUS_Z[8] = Lv1SUS_Z[7] + cLv1SUS.getLength("L6") / Unit::mm;
      Lv1SUS_Z[9] = Lv1SUS_Z[8] + cLv1SUS.getLength("L7") / Unit::mm;
      Lv1SUS_Z[10] = Lv1SUS_Z[9] + cLv1SUS.getLength("L8") / Unit::mm;
      Lv1SUS_Z[11] = Lv1SUS_Z[10] + cLv1SUS.getLength("L9") / Unit::mm;
      Lv1SUS_Z[12] = Lv1SUS_Z[11] + cLv1SUS.getLength("L10") / Unit::mm;
      Lv1SUS_Z[13] = Lv1SUS_Z[12] + cLv1SUS.getLength("L11") / Unit::mm;
      Lv1SUS_Z[14] = Lv1SUS_Z[13] + cLv1SUS.getLength("L12") / Unit::mm;
      Lv1SUS_Z[15] = Lv1SUS_Z[14] + cLv1SUS.getLength("L13") / Unit::mm;
      Lv1SUS_Z[16] = Lv1SUS_Z[15];
      Lv1SUS_Z[17] = Lv1SUS_Z[16] + cLv1SUS.getLength("L14") / Unit::mm;
      Lv1SUS_Z[18] = Lv1SUS_Z[17] + cLv1SUS.getLength("L15") / Unit::mm;
      Lv1SUS_Z[19] = Lv1SUS_Z[18];
      Lv1SUS_Z[20] = Lv1SUS_Z[19] + cLv1SUS.getLength("L16") / Unit::mm;
      //
      double Lv1SUS_rI[Lv1SUS_num];
      for (int tmpn = 0; tmpn < Lv1SUS_num; tmpn++)
        { Lv1SUS_rI[tmpn] = 0.0; }
      //
      double Lv1SUS_rO[Lv1SUS_num];
      Lv1SUS_rO[0] = cLv1SUS.getLength("R1") / Unit::mm;
      Lv1SUS_rO[1] = Lv1SUS_rO[0];
      Lv1SUS_rO[2] = cLv1SUS.getLength("R2") / Unit::mm;
      Lv1SUS_rO[3] = Lv1SUS_rO[2];
      Lv1SUS_rO[4] = cLv1SUS.getLength("R3") / Unit::mm;
      Lv1SUS_rO[5] = cLv1SUS.getLength("R4") / Unit::mm;
      Lv1SUS_rO[6] = Lv1SUS_rO[5];
      Lv1SUS_rO[7] = cLv1SUS.getLength("R5") / Unit::mm;
      Lv1SUS_rO[8] = Lv1SUS_rO[7];
      Lv1SUS_rO[9] = cLv1SUS.getLength("R6") / Unit::mm;
      Lv1SUS_rO[10] = Lv1SUS_rO[9];
      Lv1SUS_rO[11] = Lv1SUS_rO[10];
      Lv1SUS_rO[12] = cLv1SUS.getLength("R7") / Unit::mm;
      Lv1SUS_rO[13] = Lv1SUS_rO[12];
      Lv1SUS_rO[14] = cLv1SUS.getLength("R8") / Unit::mm;
      Lv1SUS_rO[15] = Lv1SUS_rO[14];
      Lv1SUS_rO[16] = cLv1SUS.getLength("R9") / Unit::mm;
      Lv1SUS_rO[17] = cLv1SUS.getLength("R10") / Unit::mm;
      Lv1SUS_rO[18] = Lv1SUS_rO[17];
      Lv1SUS_rO[19] = cLv1SUS.getLength("R11") / Unit::mm;
      Lv1SUS_rO[20] = Lv1SUS_rO[19];
      //
      string strMat_Lv1SUS = cLv1SUS.getString("Material");
      G4Material* mat_Lv1SUS = Materials::get(strMat_Lv1SUS);

      //define geometry
      G4Polycone* geo_Lv1SUS = new G4Polycone("geo_Lv1SUS_name", 0, 2*M_PI, Lv1SUS_num, Lv1SUS_Z, Lv1SUS_rI, Lv1SUS_rO);
      G4LogicalVolume *logi_Lv1SUS = new G4LogicalVolume(geo_Lv1SUS, mat_Lv1SUS, "logi_Lv1SUS_name");

      //-   put volume
      setColor(*logi_Lv1SUS, cLv1SUS.getString("Color", "#666666"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv1SUS, "phys_Lv1SUS_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2OutBe

      //get parameters from .xml file
      GearDir cLv2OutBe(content, "Lv2OutBe/");
      //
      const int Lv2OutBe_num = 2;
      //
      double Lv2OutBe_Z[Lv2OutBe_num];
      Lv2OutBe_Z[0] = -cLv2OutBe.getLength("L1") / Unit::mm;
      Lv2OutBe_Z[1] = cLv2OutBe.getLength("L2") / Unit::mm;
      //
      double Lv2OutBe_rI[Lv2OutBe_num];
      Lv2OutBe_rI[0] = cLv2OutBe.getLength("R1") / Unit::mm;
      Lv2OutBe_rI[1] = Lv2OutBe_rI[0];
      //
      double Lv2OutBe_rO[Lv2OutBe_num];
      Lv2OutBe_rO[0] = cLv2OutBe.getLength("R2") / Unit::mm;
      Lv2OutBe_rO[1] = Lv2OutBe_rO[0];
      //
      string strMat_Lv2OutBe = cLv2OutBe.getString("Material");
      G4Material* mat_Lv2OutBe = Materials::get(strMat_Lv2OutBe);

      //define geometry
      G4Polycone* geo_Lv2OutBe = new G4Polycone("geo_Lv2OutBe_name", 0, 2*M_PI, Lv2OutBe_num, Lv2OutBe_Z, Lv2OutBe_rI, Lv2OutBe_rO);
      G4LogicalVolume *logi_Lv2OutBe = new G4LogicalVolume(geo_Lv2OutBe, mat_Lv2OutBe, "logi_Lv2OutBe_name");

      //-   put volume
      setColor(*logi_Lv2OutBe, cLv2OutBe.getString("Color", "#666666"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2OutBe, "phys_Lv2OutBe_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv2InBe
      //-
      //----------

      //get parameters from .xml file
      GearDir cLv2InBe(content, "Lv2InBe/");
      //
      const int Lv2InBe_num = 2;
      //
      double Lv2InBe_Z[Lv2InBe_num];
      Lv2InBe_Z[0] = -cLv2InBe.getLength("L1") / Unit::mm;
      Lv2InBe_Z[1] = cLv2InBe.getLength("L2") / Unit::mm;
      //
      double Lv2InBe_rI[Lv2InBe_num];
      Lv2InBe_rI[0] = cLv2InBe.getLength("R1") / Unit::mm;
      Lv2InBe_rI[1] = Lv2InBe_rI[0];
      //
      double Lv2InBe_rO[Lv2InBe_num];
      Lv2InBe_rO[0] = cLv2InBe.getLength("R2") / Unit::mm;
      Lv2InBe_rO[1] = Lv2InBe_rO[0];
      //
      string strMat_Lv2InBe = cLv2InBe.getString("Material");
      G4Material* mat_Lv2InBe = Materials::get(strMat_Lv2InBe);

      //define geometry
      G4Polycone* geo_Lv2InBe = new G4Polycone("geo_Lv2InBe_name", 0, 2*M_PI, Lv2InBe_num, Lv2InBe_Z, Lv2InBe_rI, Lv2InBe_rO);
      G4LogicalVolume *logi_Lv2InBe = new G4LogicalVolume(geo_Lv2InBe, mat_Lv2InBe, "logi_Lv2InBe_name");

      //-   put volume
      setColor(*logi_Lv2InBe, cLv2InBe.getString("Color", "#666666"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2InBe, "phys_Lv2InBe_name", logi_Lv1SUS, false, 0);


      //----------
      //- Lv2Vacuum

      //get parameters from .xml file
      GearDir cLv2Vacuum(content, "Lv2Vacuum/");
      //
      double Lv2Vacuum_L1 = cLv2Vacuum.getLength("L1") / Unit::mm;
      double Lv2Vacuum_L2 = cLv2Vacuum.getLength("L2") / Unit::mm;
      double Lv2Vacuum_L3 = cLv2Vacuum.getLength("L3") / Unit::mm;
      double Lv2Vacuum_L4 = cLv2Vacuum.getLength("L4") / Unit::mm;
      double Lv2Vacuum_R1 = cLv2Vacuum.getLength("R1") / Unit::mm;
      double Lv2Vacuum_R2 = cLv2Vacuum.getLength("R2") / Unit::mm;
      double Lv2Vacuum_R3 = cLv2Vacuum.getLength("R3") / Unit::mm;
      double Lv2Vacuum_A1 = cLv2Vacuum.getAngle("A1");
      double Lv2Vacuum_A2 = cLv2Vacuum.getAngle("A2");
      //
      string strMat_Lv2Vacuum = cLv2Vacuum.getString("Material");
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
      double Lv2Vacuum2_Z2  = Lv2Vacuum_L3 * cos(Lv2Vacuum_A2);
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
      G4Polycone* geo_Lv2Vacuumpcon1 = new G4Polycone("geo_Lv2Vacuumpcon1_name", 0, 2*M_PI, Lv2Vacuum1_num, Lv2Vacuum1_Z, Lv2Vacuum1_rI, Lv2Vacuum1_rO);
      // Part 2
      G4Tubs* geo_Lv2Vacuumpcon2_1 = new G4Tubs("geo_Lv2Vacuumpcon2_1_name", Lv2Vacuum2_rI1, Lv2Vacuum2_rO1, Lv2Vacuum2_Z1, 0, 2*M_PI);
      G4Tubs* geo_Lv2Vacuumpcon2_2 = new G4Tubs("geo_Lv2Vacuumpcon2_2_name", Lv2Vacuum2_rI2, Lv2Vacuum2_rO2, Lv2Vacuum2_Z2, 0, 2*M_PI);
      G4Transform3D transform_Lv2Vacuumpcon2_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2Vacuumpcon2_2 = transform_Lv2Vacuumpcon2_2 * G4RotateY3D(-Lv2Vacuum_A2);
      G4IntersectionSolid* geo_Lv2Vacuumpcon2 = new G4IntersectionSolid("", geo_Lv2Vacuumpcon2_1, geo_Lv2Vacuumpcon2_2, transform_Lv2Vacuumpcon2_2);
      // Part 3
      G4Polycone* geo_Lv2Vacuumpcon3 = new G4Polycone("geo_Lv2Vacuumpcon3_name", 0, 2*M_PI, Lv2Vacuum3_num, Lv2Vacuum3_Z, Lv2Vacuum3_rI, Lv2Vacuum3_rO);
      // Part1+2+3
      G4Transform3D transform_Lv2Vacuumpcon3 = G4Translate3D(0., 0., 0.);
      transform_Lv2Vacuumpcon3 = transform_Lv2Vacuumpcon3 * G4RotateY3D(-Lv2Vacuum_A1);
      G4UnionSolid* geo_Lv2Vacuum = new G4UnionSolid("", geo_Lv2Vacuumpcon1, geo_Lv2Vacuumpcon3, transform_Lv2Vacuumpcon3);
      G4Transform3D transform_Lv2Vacuumpcon2 = G4Translate3D(Lv2Vacuum_L3 * sin(Lv2Vacuum_A2), 0., Lv2Vacuum_L2 + Lv2Vacuum_L3 * cos(Lv2Vacuum_A2));
      transform_Lv2Vacuumpcon2 = transform_Lv2Vacuumpcon2 * G4RotateY3D(Lv2Vacuum_A2);
      geo_Lv2Vacuum = new G4UnionSolid("", geo_Lv2Vacuum, geo_Lv2Vacuumpcon2, transform_Lv2Vacuumpcon2);
      G4LogicalVolume *logi_Lv2Vacuum = new G4LogicalVolume(geo_Lv2Vacuum, mat_Lv2Vacuum, "logi_Lv2Vacuum_name");

      //-   put volume
      setColor(*logi_Lv2Vacuum, cLv2Vacuum.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2Vacuum, "phys_Lv2Vacuum_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv2Paraf

      //get parameters from .xml file
      GearDir cLv2Paraf(content, "Lv2Paraf/");
      //
      const int Lv2Paraf1_num = 20;
      const int Lv2Paraf2_num = 3;
      //
      double Lv2Paraf1_Z[Lv2Paraf1_num];
      Lv2Paraf1_Z[0] = 0.0;
      for (int tmpn = 0; tmpn < 9; tmpn++) {
        Lv2Paraf1_Z[0] -= cLv2Paraf.getLength((format("L%1%") % (tmpn + 1)).str().c_str()) / Unit::mm;
      }
      Lv2Paraf1_Z[1] = Lv2Paraf1_Z[0] + cLv2Paraf.getLength("L1") / Unit::mm;
      Lv2Paraf1_Z[2] = Lv2Paraf1_Z[1];
      Lv2Paraf1_Z[3] = Lv2Paraf1_Z[2] + cLv2Paraf.getLength("L2") / Unit::mm;
      Lv2Paraf1_Z[4] = Lv2Paraf1_Z[3] + cLv2Paraf.getLength("L3") / Unit::mm;
      Lv2Paraf1_Z[5] = Lv2Paraf1_Z[4] + cLv2Paraf.getLength("L4") / Unit::mm;
      Lv2Paraf1_Z[6] = Lv2Paraf1_Z[5] + cLv2Paraf.getLength("L5") / Unit::mm;
      Lv2Paraf1_Z[7] = Lv2Paraf1_Z[6] + cLv2Paraf.getLength("L6") / Unit::mm;
      Lv2Paraf1_Z[8] = Lv2Paraf1_Z[7] + cLv2Paraf.getLength("L7") / Unit::mm;
      Lv2Paraf1_Z[9] = Lv2Paraf1_Z[8] + cLv2Paraf.getLength("L8") / Unit::mm;
      Lv2Paraf1_Z[10] = Lv2Paraf1_Z[9] + cLv2Paraf.getLength("L9") / Unit::mm + cLv2Paraf.getLength("L10") / Unit::mm;
      Lv2Paraf1_Z[11] = Lv2Paraf1_Z[10] + cLv2Paraf.getLength("L11") / Unit::mm;
      Lv2Paraf1_Z[12] = Lv2Paraf1_Z[11] + cLv2Paraf.getLength("L12") / Unit::mm;
      Lv2Paraf1_Z[13] = Lv2Paraf1_Z[12] + cLv2Paraf.getLength("L13") / Unit::mm;
      Lv2Paraf1_Z[14] = Lv2Paraf1_Z[13] + cLv2Paraf.getLength("L14") / Unit::mm;
      Lv2Paraf1_Z[15] = Lv2Paraf1_Z[14] + cLv2Paraf.getLength("L15") / Unit::mm + cLv2Paraf.getLength("L16") / Unit::mm;
      Lv2Paraf1_Z[16] = Lv2Paraf1_Z[15] + cLv2Paraf.getLength("L17") / Unit::mm + cLv2Paraf.getLength("L18") / Unit::mm;
      Lv2Paraf1_Z[17] = Lv2Paraf1_Z[16] + cLv2Paraf.getLength("L19") / Unit::mm;
      Lv2Paraf1_Z[18] = Lv2Paraf1_Z[17];
      Lv2Paraf1_Z[19] = Lv2Paraf1_Z[18] + cLv2Paraf.getLength("L20") / Unit::mm;
      //
      double Lv2Paraf1_rI[Lv2Paraf1_num];
      Lv2Paraf1_rI[0] = cLv2Paraf.getLength("R1") / Unit::mm;
      Lv2Paraf1_rI[1] = Lv2Paraf1_rI[0];
      Lv2Paraf1_rI[2] = Lv2Paraf1_rI[1];
      Lv2Paraf1_rI[3] = Lv2Paraf1_rI[2];
      Lv2Paraf1_rI[4] = Lv2Paraf1_rI[3];
      Lv2Paraf1_rI[5] = Lv2Paraf1_rI[4];
      Lv2Paraf1_rI[6] = Lv2Paraf1_rI[5];
      Lv2Paraf1_rI[7] = cLv2Paraf.getLength("R6") / Unit::mm;
      Lv2Paraf1_rI[8] = Lv2Paraf1_rI[7];
      Lv2Paraf1_rI[9] = Lv2Paraf1_rI[8];
      Lv2Paraf1_rI[10] = Lv2Paraf1_rI[9];
      Lv2Paraf1_rI[11] = Lv2Paraf1_rI[10];
      Lv2Paraf1_rI[12] = Lv2Paraf1_rI[11];
      Lv2Paraf1_rI[13] = cLv2Paraf.getLength("R9") / Unit::mm;
      Lv2Paraf1_rI[14] = Lv2Paraf1_rI[13];
      Lv2Paraf1_rI[15] = Lv2Paraf1_rI[14];
      Lv2Paraf1_rI[16] = Lv2Paraf1_rI[15];
      Lv2Paraf1_rI[17] = Lv2Paraf1_rI[16];
      Lv2Paraf1_rI[18] = Lv2Paraf1_rI[17];
      Lv2Paraf1_rI[19] = Lv2Paraf1_rI[18];
      //
      double Lv2Paraf1_rO[Lv2Paraf1_num];
      Lv2Paraf1_rO[0] = cLv2Paraf.getLength("R2") / Unit::mm;
      Lv2Paraf1_rO[1] = Lv2Paraf1_rO[0];
      Lv2Paraf1_rO[2] = cLv2Paraf.getLength("R3") / Unit::mm;
      Lv2Paraf1_rO[3] = cLv2Paraf.getLength("R4") / Unit::mm;
      Lv2Paraf1_rO[4] = Lv2Paraf1_rO[3];
      Lv2Paraf1_rO[5] = cLv2Paraf.getLength("R5") / Unit::mm;
      Lv2Paraf1_rO[6] = Lv2Paraf1_rO[5];
      Lv2Paraf1_rO[7] = Lv2Paraf1_rO[6];
      Lv2Paraf1_rO[8] = Lv2Paraf1_rO[7];
      Lv2Paraf1_rO[9] = cLv2Paraf.getLength("R7") / Unit::mm;
      Lv2Paraf1_rO[10] = Lv2Paraf1_rO[9];
      Lv2Paraf1_rO[11] = cLv2Paraf.getLength("R8") / Unit::mm;
      Lv2Paraf1_rO[12] = Lv2Paraf1_rO[11];
      Lv2Paraf1_rO[13] = Lv2Paraf1_rO[12];
      Lv2Paraf1_rO[14] = Lv2Paraf1_rO[13];
      Lv2Paraf1_rO[15] = cLv2Paraf.getLength("R10") / Unit::mm;
      Lv2Paraf1_rO[16] = Lv2Paraf1_rO[15];
      Lv2Paraf1_rO[17] = cLv2Paraf.getLength("R12") / Unit::mm;
      Lv2Paraf1_rO[18] = cLv2Paraf.getLength("R13") / Unit::mm;
      Lv2Paraf1_rO[19] = Lv2Paraf1_rO[18];
      //
      //
      double Lv2Paraf2_Z[Lv2Paraf2_num];
      Lv2Paraf1_Z[0] = 0.0;
      for (int tmpn = 10; tmpn <= 15; tmpn++) {
        Lv2Paraf2_Z[0] += cLv2Paraf.getLength((format("L%1%") % tmpn).str().c_str()) / Unit::mm;
      }
      Lv2Paraf2_Z[1] = Lv2Paraf2_Z[0] + cLv2Paraf.getLength("L16") / Unit::mm + cLv2Paraf.getLength("L17") / Unit::mm;
      Lv2Paraf2_Z[2] = Lv2Paraf2_Z[1] + cLv2Paraf.getLength("L18") / Unit::mm + cLv2Paraf.getLength("L19") / Unit::mm + cLv2Paraf.getLength("L20") / Unit::mm + 1.0;
      //
      double Lv2Paraf2_rI[Lv2Paraf2_num];
      for (int tmpn = 0; tmpn < Lv2Paraf2_num; tmpn++)
        { Lv2Paraf2_rI[tmpn] = 0.0; }
      //
      double Lv2Paraf2_rO[Lv2Paraf2_num];
      Lv2Paraf2_rO[0] = cLv2Paraf.getLength("R9") / Unit::mm;
      Lv2Paraf2_rO[1] = cLv2Paraf.getLength("R11") / Unit::mm;
      Lv2Paraf2_rO[2] = Lv2Paraf2_rO[1];
      //
      string strMat_Lv2Paraf = cLv2Paraf.getString("Material");
      G4Material* mat_Lv2Paraf = Materials::get(strMat_Lv2Paraf);

      //define geometry
      G4Polycone* geo_Lv2Parafpcon1 = new G4Polycone("geo_Lv2Parafpcon1_name", 0, 2*M_PI, Lv2Paraf1_num, Lv2Paraf1_Z, Lv2Paraf1_rI, Lv2Paraf1_rO);
      G4Polycone* geo_Lv2Parafpcon2 = new G4Polycone("geo_Lv2Parafpcon2_name", 0, 2*M_PI, Lv2Paraf2_num, Lv2Paraf2_Z, Lv2Paraf2_rI, Lv2Paraf2_rO);
      G4SubtractionSolid* geo_Lv2Paraf = new G4SubtractionSolid("", geo_Lv2Parafpcon1, geo_Lv2Parafpcon2);
      G4LogicalVolume *logi_Lv2Paraf = new G4LogicalVolume(geo_Lv2Paraf, mat_Lv2Paraf, "logi_Lv2Paraf_name");

      //-   put volume
      setColor(*logi_Lv2Paraf, cLv2Paraf.getString("Color", "#00CCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2Paraf, "phys_Lv2Paraf_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv3AuCoat

      //get parameters from .xml file
      GearDir cLv3AuCoat(content, "Lv3AuCoat/");
      //
      const int Lv3AuCoat_num = 2;
      //
      double Lv3AuCoat_Z[Lv3AuCoat_num];
      Lv3AuCoat_Z[0] = -cLv3AuCoat.getLength("L1") / Unit::mm;
      Lv3AuCoat_Z[1] = cLv3AuCoat.getLength("L2") / Unit::mm;
      //
      double Lv3AuCoat_rI[Lv3AuCoat_num];
      Lv3AuCoat_rI[0] = cLv3AuCoat.getLength("R1") / Unit::mm;
      Lv3AuCoat_rI[1] = Lv3AuCoat_rI[0];
      //
      double Lv3AuCoat_rO[Lv3AuCoat_num];
      Lv3AuCoat_rO[0] = cLv3AuCoat.getLength("R2") / Unit::mm;
      Lv3AuCoat_rO[1] = Lv3AuCoat_rO[0];
      //
      string strMat_Lv3AuCoat = cLv3AuCoat.getString("Material");
      G4Material* mat_Lv3AuCoat = Materials::get(strMat_Lv3AuCoat);

      //define geometry
      G4Polycone* geo_Lv3AuCoat = new G4Polycone("geo_Lv3AuCoat_name", 0, 2*M_PI, Lv3AuCoat_num, Lv3AuCoat_Z, Lv3AuCoat_rI, Lv3AuCoat_rO);
      G4LogicalVolume *logi_Lv3AuCoat = new G4LogicalVolume(geo_Lv3AuCoat, mat_Lv3AuCoat, "logi_Lv3AuCoat_name");

      //-   put volume
      setColor(*logi_Lv3AuCoat, cLv3AuCoat.getString("Color", "#666666"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv3AuCoat, "phys_Lv3AuCoat_name", logi_Lv2Vacuum, false, 0);

      //-
      //----------

      //=
      //==========

      //==========
      //= Ta pipe Forward

      //----------
      //- Lv1TaFwd

      //get parameters from .xml file
      GearDir cLv1TaFwd(content, "Lv1TaFwd/");
      //
      double Lv1TaFwd_D1 = cLv1TaFwd.getLength("D1") / Unit::mm;
      double Lv1TaFwd_L1 = cLv1TaFwd.getLength("L1") / Unit::mm;
      double Lv1TaFwd_L2 = cLv1TaFwd.getLength("L2") / Unit::mm;
      double Lv1TaFwd_L3 = cLv1TaFwd.getLength("L3") / Unit::mm;
      double Lv1TaFwd_T1 = cLv1TaFwd.getLength("T1") / Unit::mm;
      //
      string strMat_Lv1TaFwd = cLv1TaFwd.getString("Material");
      G4Material* mat_Lv1TaFwd = Materials::get(strMat_Lv1TaFwd);

      //define geometry
      G4Trd* geo_Lv1TaFwd = new G4Trd("geo_Lv1TaFwd_name", Lv1TaFwd_L3, Lv1TaFwd_L2, Lv1TaFwd_T1, Lv1TaFwd_T1, Lv1TaFwd_L1 / 2.0);
      G4LogicalVolume *logi_Lv1TaFwd = new G4LogicalVolume(geo_Lv1TaFwd, mat_Lv1TaFwd, "logi_Lv1TaFwd_name");

      //-   put volume
      setColor(*logi_Lv1TaFwd, cLv1TaFwd.getString("Color", "#666666"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, Lv1TaFwd_D1 + Lv1TaFwd_L1 / 2.0), logi_Lv1TaFwd, "phys_Lv1TaFwd_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacFwd

      //get parameters from .xml file
      GearDir cLv2VacFwd(content, "Lv2VacFwd/");
      //
      double Lv2VacFwd_D1 = cLv2VacFwd.getLength("D1") / Unit::mm;
      double Lv2VacFwd_D2 = cLv2VacFwd.getLength("D2") / Unit::mm;
      double Lv2VacFwd_D3 = cLv2VacFwd.getLength("D3") / Unit::mm;
      double Lv2VacFwd_L1 = cLv2VacFwd.getLength("L1") / Unit::mm;
      double Lv2VacFwd_L2 = cLv2VacFwd.getLength("L2") / Unit::mm;
      double Lv2VacFwd_L3 = cLv2VacFwd.getLength("L3") / Unit::mm;
      double Lv2VacFwd_R1 = cLv2VacFwd.getLength("R1") / Unit::mm;
      double Lv2VacFwd_R2 = cLv2VacFwd.getLength("R2") / Unit::mm;
      double Lv2VacFwd_R3 = cLv2VacFwd.getLength("R3") / Unit::mm;
      double Lv2VacFwd_R4 = cLv2VacFwd.getLength("R4") / Unit::mm;
      double Lv2VacFwd_A1 = cLv2VacFwd.getAngle("A1");
      double Lv2VacFwd_A2 = cLv2VacFwd.getAngle("A2");
      //
      string strMat_Lv2VacFwd = cLv2VacFwd.getString("Material");
      G4Material* mat_Lv2VacFwd = Materials::get(strMat_Lv2VacFwd);
      //
      // Part 1
      double Lv2VacFwd1_Z1  = sqrt(Lv2VacFwd_D1 * Lv2VacFwd_D1 + Lv2VacFwd_D2 * Lv2VacFwd_D2 - 2.*Lv2VacFwd_D1 * Lv2VacFwd_D2 * cos(Lv2VacFwd_A1));
      double Lv2VacFwd1_rI1 = 0.0;
      double Lv2VacFwd1_rO1 = Lv2VacFwd_R1;
      //
      double Lv2VacFwd1_Z2  = Lv2VacFwd1_Z1 / 2. * cos(Lv2VacFwd_A2);
      double Lv2VacFwd1_rI2 = 0.0;
      double Lv2VacFwd1_rO2 = 2 * Lv2VacFwd_R1;
      // Part 2
      double Lv2VacFwd2_Z1  = Lv2VacFwd_L1;
      double Lv2VacFwd2_rI1 = 0.0;
      double Lv2VacFwd2_rO1 = Lv2VacFwd_R2;
      //
      double Lv2VacFwd2_Z2  = Lv2VacFwd_L1 / 2. * cos(Lv2VacFwd_A2);
      double Lv2VacFwd2_rI2 = 0.0;
      double Lv2VacFwd2_rO2 = 2 * Lv2VacFwd_R2;
      // Part 3
      const int Lv2VacFwd3_num = 4;
      //
      double Lv2VacFwd_Z[Lv2VacFwd3_num];
      Lv2VacFwd_Z[0] = 0.0;
      Lv2VacFwd_Z[1] = Lv2VacFwd_D3 - Lv2VacFwd_L2;
      Lv2VacFwd_Z[2] = Lv2VacFwd_D3;
      Lv2VacFwd_Z[3] = Lv2VacFwd_Z[2] + Lv2VacFwd_L3;
      double Lv2VacFwd_rI[Lv2VacFwd3_num];
      for (int tmpn = 0; tmpn < Lv2VacFwd3_num; tmpn++)
        { Lv2VacFwd_rI[tmpn] = 0.0; }
      double Lv2VacFwd_rO[Lv2VacFwd3_num];
      Lv2VacFwd_rO[0] = Lv2VacFwd_R3;
      Lv2VacFwd_rO[1] = Lv2VacFwd_R3;
      Lv2VacFwd_rO[2] = Lv2VacFwd_R4;
      Lv2VacFwd_rO[3] = Lv2VacFwd_R4;

      //define geometry
      // Part 1
      G4Tubs* geo_Lv2VacFwdpcon1_1 = new G4Tubs("geo_Lv2VacFwdpcon1_1_name", Lv2VacFwd1_rI1, Lv2VacFwd1_rO1, Lv2VacFwd1_Z1, 0, 2*M_PI);
      G4Tubs* geo_Lv2VacFwdpcon1_2 = new G4Tubs("geo_Lv2VacFwdpcon1_2_name", Lv2VacFwd1_rI2, Lv2VacFwd1_rO2, Lv2VacFwd1_Z2, 0, 2*M_PI);
      G4Transform3D transform_Lv2VacFwdpcon1_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacFwdpcon1_2 = transform_Lv2VacFwdpcon1_2 * G4RotateY3D(-Lv2VacFwd_A2);
      G4IntersectionSolid* geo_Lv2VacFwdpcon1 = new G4IntersectionSolid("", geo_Lv2VacFwdpcon1_1, geo_Lv2VacFwdpcon1_2, transform_Lv2VacFwdpcon1_2);
      // Part 2
      G4Tubs* geo_Lv2VacFwdpcon2_1 = new G4Tubs("geo_Lv2VacFwdpcon2_1_name", Lv2VacFwd2_rI1, Lv2VacFwd2_rO1, Lv2VacFwd2_Z1, 0, 2*M_PI);
      G4Tubs* geo_Lv2VacFwdpcon2_2 = new G4Tubs("geo_Lv2VacFwdpcon2_2_name", Lv2VacFwd2_rI2, Lv2VacFwd2_rO2, Lv2VacFwd2_Z2, 0, 2*M_PI);
      G4Transform3D transform_Lv2VacFwdpcon2_2 = G4Translate3D(0., 0., 0.);
      transform_Lv2VacFwdpcon2_2 = transform_Lv2VacFwdpcon2_2 * G4RotateY3D(-Lv2VacFwd_A2);
      G4IntersectionSolid* geo_Lv2VacFwdpcon2 = new G4IntersectionSolid("", geo_Lv2VacFwdpcon2_1, geo_Lv2VacFwdpcon2_2, transform_Lv2VacFwdpcon2_2);
      // Part 3
      G4Polycone* geo_Lv2VacFwdpcon3 = new G4Polycone("geo_Lv2VacFwdpcon3", 0, 2*M_PI, Lv2VacFwd3_num, Lv2VacFwd_Z, Lv2VacFwd_rI, Lv2VacFwd_rO);
      // Part1+2+3
      G4Transform3D transform_Lv2VacFwdpcon1
      = G4Translate3D((Lv2VacFwd_D1 * sin(Lv2VacFwd_A1) + Lv2VacFwd_D2 * sin(2 * Lv2VacFwd_A1)) / 2.,
                      0.,
                      (Lv2VacFwd_D1 * cos(Lv2VacFwd_A1) + Lv2VacFwd_D2 * cos(2 * Lv2VacFwd_A1)) / 2.);
      transform_Lv2VacFwdpcon1 = transform_Lv2VacFwdpcon1 * G4RotateY3D(Lv2VacFwd_A1 + Lv2VacFwd_A2);
      G4UnionSolid* geo_Lv2VacFwd = new G4UnionSolid("", geo_Lv2VacFwdpcon3, geo_Lv2VacFwdpcon1, transform_Lv2VacFwdpcon1);
      G4Transform3D transform_Lv2VacFwdpcon2
      = G4Translate3D((Lv2VacFwd_D2 + Lv2VacFwd_L1 / 2.) * sin(2.*Lv2VacFwd_A1),
                      0.,
                      (Lv2VacFwd_D2 + Lv2VacFwd_L1 / 2.) * cos(2.*Lv2VacFwd_A1));
      transform_Lv2VacFwdpcon2 = transform_Lv2VacFwdpcon2 * G4RotateY3D(2.*Lv2VacFwd_A1);
      geo_Lv2VacFwd = new G4UnionSolid("", geo_Lv2VacFwd, geo_Lv2VacFwdpcon2, transform_Lv2VacFwdpcon2);
      G4LogicalVolume *logi_Lv2VacFwd = new G4LogicalVolume(geo_Lv2VacFwd, mat_Lv2VacFwd, "logi_Lv2VacFwd_name");

      //-   put volume
      setColor(*logi_Lv2VacFwd, cLv2VacFwd.getString("Color", "#CCCCCC"));
      G4Transform3D transform_Lv2VacFwd = G4Translate3D(0., 0., 0.);
      transform_Lv2VacFwd = transform_Lv2VacFwd * G4RotateY3D(-Lv2VacFwd_A1);
      new G4PVPlacement(transform_Lv2VacFwd, logi_Lv2VacFwd, "phys_Lv2VacFwd_name", logi_Lv1TaFwd, false, 0);

      //-
      //----------

      //=
      //==========

      //==========
      //= Ta pipe Backward

      //----------
      //- Lv1TaBwd

      //get parameters from .xml file
      GearDir cLv1TaBwd(content, "Lv1TaBwd/");
      //
      double Lv1TaBwd_D1 = cLv1TaBwd.getLength("D1") / Unit::mm;
      double Lv1TaBwd_L1 = cLv1TaBwd.getLength("L1") / Unit::mm;
      double Lv1TaBwd_L2 = cLv1TaBwd.getLength("L2") / Unit::mm;
      double Lv1TaBwd_L3 = cLv1TaBwd.getLength("L3") / Unit::mm;
      double Lv1TaBwd_T1 = cLv1TaBwd.getLength("T1") / Unit::mm;
      //
      string strMat_Lv1TaBwd = cLv1TaBwd.getString("Material");
      G4Material* mat_Lv1TaBwd = Materials::get(strMat_Lv1TaBwd);

      //define geometry
      G4Trd* geo_Lv1TaBwd = new G4Trd("geo_Lv1TaBwd_name", Lv1TaBwd_L3, Lv1TaBwd_L2, Lv1TaBwd_T1, Lv1TaBwd_T1, Lv1TaBwd_L1 / 2.0);
      G4LogicalVolume *logi_Lv1TaBwd = new G4LogicalVolume(geo_Lv1TaBwd, mat_Lv1TaBwd, "logi_Lv1TaBwd_name");

      //-   put volume
      setColor(*logi_Lv1TaBwd, cLv1TaBwd.getString("Color", "#666666"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, -Lv1TaBwd_D1 - Lv1TaBwd_L1 / 2.0), logi_Lv1TaBwd, "phys_Lv1TaBwd_name", logi_Lv1SUS, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacBwd

      //-
      //----------

      //=
      //==========

      ////==========
      ////= beam pipe Forward Forward

      //get parameters from .xml file
      GearDir cAreaTubeFwd(content, "AreaTubeFwd/");
      //
      const int AreaTubeFwd_num = 2;
      //
      double AreaTubeFwd_Z[AreaTubeFwd_num];
      AreaTubeFwd_Z[0] = cAreaTubeFwd.getLength("D1") / Unit::mm;
      AreaTubeFwd_Z[1] = cAreaTubeFwd.getLength("D2") / Unit::mm;
      //
      double AreaTubeFwd_rI[AreaTubeFwd_num];
      for (int i = 0; i < AreaTubeFwd_num; i++)
        { AreaTubeFwd_rI[i] = 0.0; }
      //
      double AreaTubeFwd_rO[AreaTubeFwd_num];
      AreaTubeFwd_rO[0] = cAreaTubeFwd.getLength("R1") / Unit::mm;
      AreaTubeFwd_rO[1] = AreaTubeFwd_rO[0];

      //define geometry
      G4Polycone* geo_AreaTubeFwdpcon = new G4Polycone("geo_AreaTubeFwdpcon_name", 0, 2*M_PI, AreaTubeFwd_num, AreaTubeFwd_Z, AreaTubeFwd_rI, AreaTubeFwd_rO);

      //----------
      //- Lv1TaLERDwn

      //get parameters from .xml file
      GearDir cLv1TaLERDwn(content, "Lv1TaLERDwn/");
      //
      double Lv1TaLERDwn_A1 = cLv1TaLERDwn.getAngle("A1");
      //
      const int Lv1TaLERDwn_num = 2;
      //
      double Lv1TaLERDwn_Z[Lv1TaLERDwn_num];
      Lv1TaLERDwn_Z[0] = cLv1TaLERDwn.getLength("L1") / Unit::mm;
      Lv1TaLERDwn_Z[1] = Lv1TaLERDwn_Z[0] + cLv1TaLERDwn.getLength("L2") / Unit::mm;
      //
      double Lv1TaLERDwn_rI[Lv1TaLERDwn_num];
      for (int i = 0; i < Lv1TaLERDwn_num; i++)
        { Lv1TaLERDwn_rI[i] = 0.0; }
      //
      double Lv1TaLERDwn_rO[Lv1TaLERDwn_num];
      Lv1TaLERDwn_rO[0] = cLv1TaLERDwn.getLength("R1") / Unit::mm;
      Lv1TaLERDwn_rO[1] = Lv1TaLERDwn_rO[0];
      //
      string strMat_Lv1TaLERDwn = cLv1TaLERDwn.getString("Material");
      G4Material* mat_Lv1TaLERDwn = Materials::get(strMat_Lv1TaLERDwn);

      //define geometry
      G4Polycone* geo_Lv1TaLERDwnpcon = new G4Polycone("geo_Lv1TaLERDwnpcon_name", 0, 2*M_PI, Lv1TaLERDwn_num, Lv1TaLERDwn_Z, Lv1TaLERDwn_rI, Lv1TaLERDwn_rO);
      G4Transform3D transform_AreaTubeFwdForLER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeFwdForLER = transform_AreaTubeFwdForLER * G4RotateY3D(-Lv1TaLERDwn_A1);
      G4IntersectionSolid* geo_Lv1TaLERDwn = new G4IntersectionSolid("", geo_Lv1TaLERDwnpcon, geo_AreaTubeFwdpcon, transform_AreaTubeFwdForLER);
      G4LogicalVolume *logi_Lv1TaLERDwn = new G4LogicalVolume(geo_Lv1TaLERDwn, mat_Lv1TaLERDwn, "logi_Lv1TaLERDwn_name");

      //-   put volume
      setColor(*logi_Lv1TaLERDwn, cLv1TaLERDwn.getString("Color", "#00CC00"));
      G4Transform3D transform_Lv1TaLERDwn = G4Translate3D(0., 0., 0.);
      transform_Lv1TaLERDwn = transform_Lv1TaLERDwn * G4RotateY3D(Lv1TaLERDwn_A1);
      new G4PVPlacement(transform_Lv1TaLERDwn, logi_Lv1TaLERDwn, "phys_Lv1TaLERDwn_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacLERDwn

      //get parameters from .xml file
      GearDir cLv2VacLERDwn(content, "Lv2VacLERDwn/");
      //
      double Lv2VacLERDwn_rO[Lv1TaLERDwn_num];
      Lv2VacLERDwn_rO[0] = cLv2VacLERDwn.getLength("R1") / Unit::mm;
      Lv2VacLERDwn_rO[1] = Lv2VacLERDwn_rO[0];
      //
      string strMat_Lv2VacLERDwn = cLv2VacLERDwn.getString("Material");
      G4Material* mat_Lv2VacLERDwn = Materials::get(strMat_Lv2VacLERDwn);

      //define geometry
      G4Polycone* geo_Lv2VacLERDwnpcon = new G4Polycone("geo_Lv2VacLERDwnpcon_name", 0, 2*M_PI, Lv1TaLERDwn_num, Lv1TaLERDwn_Z, Lv1TaLERDwn_rI, Lv2VacLERDwn_rO);
      G4IntersectionSolid* geo_Lv2VacLERDwn = new G4IntersectionSolid("", geo_Lv2VacLERDwnpcon, geo_AreaTubeFwdpcon, transform_AreaTubeFwdForLER);
      G4LogicalVolume *logi_Lv2VacLERDwn = new G4LogicalVolume(geo_Lv2VacLERDwn, mat_Lv2VacLERDwn, "logi_Lv2VacLERDwn_name");

      //-   put volume
      setColor(*logi_Lv2VacLERDwn, cLv2VacLERDwn.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacLERDwn, "phys_Lv2VacLERDwn_name", logi_Lv1TaLERDwn, false, 0);
      //-
      //----------

      //----------
      //- Lv1TaHERUp

      //get parameters from .xml file
      GearDir cLv1TaHERUp(content, "Lv1TaHERUp/");
      //
      double Lv1TaHERUp_A1 = cLv1TaHERUp.getAngle("A1");
      //
      const int Lv1TaHERUp_num = 2;
      //
      double Lv1TaHERUp_Z[Lv1TaHERUp_num];
      Lv1TaHERUp_Z[0] = cLv1TaHERUp.getLength("L1") / Unit::mm;
      Lv1TaHERUp_Z[1] = Lv1TaHERUp_Z[0] + cLv1TaHERUp.getLength("L2") / Unit::mm;
      //
      double Lv1TaHERUp_rI[Lv1TaHERUp_num];
      for (int i = 0; i < Lv1TaHERUp_num; i++)
        { Lv1TaHERUp_rI[i] = 0.0; }
      //
      double Lv1TaHERUp_rO[Lv1TaHERUp_num];
      Lv1TaHERUp_rO[0] = cLv1TaHERUp.getLength("R1") / Unit::mm;
      Lv1TaHERUp_rO[1] = Lv1TaHERUp_rO[0];
      //
      string strMat_Lv1TaHERUp = cLv1TaHERUp.getString("Material");
      G4Material* mat_Lv1TaHERUp = Materials::get(strMat_Lv1TaHERUp);

      //define geometry
      G4Polycone* geo_Lv1TaHERUppcon = new G4Polycone("geo_Lv1TaHERUppcon_name", 0, 2*M_PI, Lv1TaHERUp_num, Lv1TaHERUp_Z, Lv1TaHERUp_rI, Lv1TaHERUp_rO);
      G4Transform3D transform_AreaTubeFwdForHER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeFwdForHER = transform_AreaTubeFwdForHER * G4RotateY3D(-Lv1TaHERUp_A1);
      G4IntersectionSolid* geo_Lv1TaHERUp = new G4IntersectionSolid("", geo_Lv1TaHERUppcon, geo_AreaTubeFwdpcon, transform_AreaTubeFwdForHER);
      G4LogicalVolume *logi_Lv1TaHERUp = new G4LogicalVolume(geo_Lv1TaHERUp, mat_Lv1TaHERUp, "logi_Lv1TaHERUp_name");

      //-   put volume
      setColor(*logi_Lv1TaHERUp, cLv1TaHERUp.getString("Color", "#00CC00"));
      G4Transform3D transform_Lv1TaHERUp = G4Translate3D(0., 0., 0.);
      transform_Lv1TaHERUp = transform_Lv1TaHERUp * G4RotateY3D(Lv1TaHERUp_A1);
      new G4PVPlacement(transform_Lv1TaHERUp, logi_Lv1TaHERUp, "phys_Lv1TaHERUp_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacHERUp

      //get parameters from .xml file
      GearDir cLv2VacHERUp(content, "Lv2VacHERUp/");
      //
      double Lv2VacHERUp_rO[Lv1TaHERUp_num];
      Lv2VacHERUp_rO[0] = cLv2VacHERUp.getLength("R1") / Unit::mm;
      Lv2VacHERUp_rO[1] = Lv2VacHERUp_rO[0];
      //
      string strMat_Lv2VacHERUp = cLv2VacHERUp.getString("Material");
      G4Material* mat_Lv2VacHERUp = Materials::get(strMat_Lv2VacHERUp);

      //define geometry
      G4Polycone* geo_Lv2VacHERUppcon = new G4Polycone("geo_Lv2VacHERUppcon_name", 0, 2*M_PI, Lv1TaHERUp_num, Lv1TaHERUp_Z, Lv1TaHERUp_rI, Lv2VacHERUp_rO);
      G4IntersectionSolid* geo_Lv2VacHERUp = new G4IntersectionSolid("", geo_Lv2VacHERUppcon, geo_AreaTubeFwdpcon, transform_AreaTubeFwdForHER);
      G4LogicalVolume *logi_Lv2VacHERUp = new G4LogicalVolume(geo_Lv2VacHERUp, mat_Lv2VacHERUp, "logi_Lv2VacHERUp_name");

      //-   put volume
      setColor(*logi_Lv2VacHERUp, cLv2VacHERUp.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacHERUp, "phys_Lv2VacHERUp_name", logi_Lv1TaHERUp, false, 0);

      //-
      //----------

      ////=
      ////==========

      ////==========
      ////= beam pipe Backward Backward

      //get parameters from .xml file
      GearDir cAreaTubeBwd(content, "AreaTubeBwd/");
      //
      const int AreaTubeBwd_num = 2;
      //
      double AreaTubeBwd_Z[AreaTubeBwd_num];
      AreaTubeBwd_Z[0] = cAreaTubeBwd.getLength("D1") / Unit::mm;
      AreaTubeBwd_Z[1] = cAreaTubeBwd.getLength("D2") / Unit::mm;
      //
      double AreaTubeBwd_rI[AreaTubeBwd_num];
      for (int i = 0; i < AreaTubeBwd_num; i++)
        { AreaTubeBwd_rI[i] = 0.0; }
      //
      double AreaTubeBwd_rO[AreaTubeBwd_num];
      AreaTubeBwd_rO[0] = cAreaTubeBwd.getLength("R1") / Unit::mm;
      AreaTubeBwd_rO[1] = AreaTubeBwd_rO[0];

      //define geometry
      G4Polycone* geo_AreaTubeBwdpcon = new G4Polycone("geo_AreaTubeBwdpcon_name", 0, 2*M_PI, AreaTubeBwd_num, AreaTubeBwd_Z, AreaTubeBwd_rI, AreaTubeBwd_rO);

      //----------
      //- Lv1TaHERDwn

      //get parameters from .xml file
      GearDir cLv1TaHERDwn(content, "Lv1TaHERDwn/");
      //
      double Lv1TaHERDwn_A1 = cLv1TaHERDwn.getAngle("A1");
      //
      const int Lv1TaHERDwn_num = 2;
      //
      double Lv1TaHERDwn_Z[Lv1TaHERDwn_num];
      Lv1TaHERDwn_Z[0] = cLv1TaHERDwn.getLength("L1") / Unit::mm;
      Lv1TaHERDwn_Z[1] = Lv1TaHERDwn_Z[0] + cLv1TaHERDwn.getLength("L2") / Unit::mm;
      //
      double Lv1TaHERDwn_rI[Lv1TaHERDwn_num];
      for (int i = 0; i < Lv1TaHERDwn_num; i++)
        { Lv1TaHERDwn_rI[i] = 0.0; }
      //
      double Lv1TaHERDwn_rO[Lv1TaHERDwn_num];
      Lv1TaHERDwn_rO[0] = cLv1TaHERDwn.getLength("R1") / Unit::mm;
      Lv1TaHERDwn_rO[1] = Lv1TaHERDwn_rO[0];
      //
      string strMat_Lv1TaHERDwn = cLv1TaHERDwn.getString("Material");
      G4Material* mat_Lv1TaHERDwn = Materials::get(strMat_Lv1TaHERDwn);

      //define geometry
      G4Polycone* geo_Lv1TaHERDwnpcon = new G4Polycone("geo_Lv1TaHERDwnpcon_name", 0, 2*M_PI, Lv1TaHERDwn_num, Lv1TaHERDwn_Z, Lv1TaHERDwn_rI, Lv1TaHERDwn_rO);
      G4Transform3D transform_AreaTubeBwdForHER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeBwdForHER = transform_AreaTubeBwdForHER * G4RotateY3D(Lv1TaHERDwn_A1);
      G4IntersectionSolid* geo_Lv1TaHERDwn = new G4IntersectionSolid("", geo_Lv1TaHERDwnpcon, geo_AreaTubeBwdpcon, transform_AreaTubeBwdForHER);
      G4LogicalVolume *logi_Lv1TaHERDwn = new G4LogicalVolume(geo_Lv1TaHERDwn, mat_Lv1TaHERDwn, "logi_Lv1TaHERDwn_name");

      //-   put volume
      setColor(*logi_Lv1TaHERDwn, cLv1TaHERDwn.getString("Color", "#00CC00"));
      G4Transform3D transform_Lv1TaHERDwn = G4Translate3D(0., 0., 0.);
      transform_Lv1TaHERDwn = transform_Lv1TaHERDwn * G4RotateY3D(-Lv1TaHERDwn_A1);
      new G4PVPlacement(transform_Lv1TaHERDwn, logi_Lv1TaHERDwn, "phys_Lv1TaHERDwn_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacHERDwn

      //get parameters from .xml file
      GearDir cLv2VacHERDwn(content, "Lv2VacHERDwn/");
      //
      double Lv2VacHERDwn_rO[Lv1TaHERDwn_num];
      Lv2VacHERDwn_rO[0] = cLv2VacHERDwn.getLength("R1") / Unit::mm;
      Lv2VacHERDwn_rO[1] = Lv2VacHERDwn_rO[0];
      //
      string strMat_Lv2VacHERDwn = cLv2VacHERDwn.getString("Material");
      G4Material* mat_Lv2VacHERDwn = Materials::get(strMat_Lv2VacHERDwn);

      //define geometry
      G4Polycone* geo_Lv2VacHERDwnpcon = new G4Polycone("geo_Lv2VacHERDwnpcon_name", 0, 2*M_PI, Lv1TaHERDwn_num, Lv1TaHERDwn_Z, Lv1TaHERDwn_rI, Lv2VacHERDwn_rO);
      G4IntersectionSolid* geo_Lv2VacHERDwn = new G4IntersectionSolid("", geo_Lv2VacHERDwnpcon, geo_AreaTubeFwdpcon, transform_AreaTubeFwdForHER);
      G4LogicalVolume *logi_Lv2VacHERDwn = new G4LogicalVolume(geo_Lv2VacHERDwn, mat_Lv2VacHERDwn, "logi_Lv2VacHERDwn_name");

      //-   put volume
      setColor(*logi_Lv2VacHERDwn, cLv2VacHERDwn.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacHERDwn, "phys_Lv2VacHERDwn_name", logi_Lv1TaHERDwn, false, 0);

      //-
      //----------

      //----------
      //- Lv1TaLERUp

      //get parameters from .xml file
      GearDir cLv1TaLERUp(content, "Lv1TaLERUp/");
      //
      double Lv1TaLERUp_A1 = cLv1TaLERUp.getAngle("A1");
      //
      const int Lv1TaLERUp_num = 2;
      //
      double Lv1TaLERUp_Z[Lv1TaLERUp_num];
      Lv1TaLERUp_Z[0] = cLv1TaLERUp.getLength("L1") / Unit::mm;
      Lv1TaLERUp_Z[1] = Lv1TaLERUp_Z[0] + cLv1TaLERUp.getLength("L2") / Unit::mm;
      //
      double Lv1TaLERUp_rI[Lv1TaLERUp_num];
      for (int i = 0; i < Lv1TaLERUp_num; i++)
        { Lv1TaLERUp_rI[i] = 0.0; }
      //
      double Lv1TaLERUp_rO[Lv1TaLERUp_num];
      Lv1TaLERUp_rO[0] = cLv1TaLERUp.getLength("R1") / Unit::mm;
      Lv1TaLERUp_rO[1] = Lv1TaLERUp_rO[0];
      //
      string strMat_Lv1TaLERUp = cLv1TaLERUp.getString("Material");
      G4Material* mat_Lv1TaLERUp = Materials::get(strMat_Lv1TaLERUp);

      //define geometry
      G4Polycone* geo_Lv1TaLERUppcon = new G4Polycone("geo_Lv1TaLERUppcon_name", 0, 2*M_PI, Lv1TaLERUp_num, Lv1TaLERUp_Z, Lv1TaLERUp_rI, Lv1TaLERUp_rO);
      G4Transform3D transform_AreaTubeBwdForLER = G4Translate3D(0., 0., 0.);
      transform_AreaTubeBwdForLER = transform_AreaTubeBwdForLER * G4RotateY3D(-Lv1TaLERUp_A1);
      G4IntersectionSolid* geo_Lv1TaLERUp = new G4IntersectionSolid("", geo_Lv1TaLERUppcon, geo_AreaTubeBwdpcon, transform_AreaTubeBwdForLER);
      G4LogicalVolume *logi_Lv1TaLERUp = new G4LogicalVolume(geo_Lv1TaLERUp, mat_Lv1TaLERUp, "logi_Lv1TaLERUp_name");

      //-   put volume
      setColor(*logi_Lv1TaLERUp, cLv1TaLERUp.getString("Color", "#00CC00"));
      G4Transform3D transform_Lv1TaLERUp = G4Translate3D(0., 0., 0.);
      transform_Lv1TaLERUp = transform_Lv1TaLERUp * G4RotateY3D(Lv1TaLERUp_A1);
      new G4PVPlacement(transform_Lv1TaLERUp, logi_Lv1TaLERUp, "phys_Lv1TaLERUp_name", &topVolume, false, 0);

      //-
      //----------

      //----------
      //- Lv2VacLERUp

      //get parameters from .xml file
      GearDir cLv2VacLERUp(content, "Lv2VacLERUp/");
      //
      double Lv2VacLERUp_rO[Lv1TaLERUp_num];
      Lv2VacLERUp_rO[0] = cLv2VacLERUp.getLength("R1") / Unit::mm;
      Lv2VacLERUp_rO[1] = Lv2VacLERUp_rO[0];
      //
      string strMat_Lv2VacLERUp = cLv2VacLERUp.getString("Material");
      G4Material* mat_Lv2VacLERUp = Materials::get(strMat_Lv2VacLERUp);

      //define geometry
      G4Polycone* geo_Lv2VacLERUppcon = new G4Polycone("geo_Lv2VacLERUppcon_name", 0, 2*M_PI, Lv1TaLERUp_num, Lv1TaLERUp_Z, Lv1TaLERUp_rI, Lv2VacLERUp_rO);
      G4IntersectionSolid* geo_Lv2VacLERUp = new G4IntersectionSolid("", geo_Lv2VacLERUppcon, geo_AreaTubeFwdpcon, transform_AreaTubeBwdForLER);
      G4LogicalVolume *logi_Lv2VacLERUp = new G4LogicalVolume(geo_Lv2VacLERUp, mat_Lv2VacLERUp, "logi_Lv2VacLERUp_name");

      //-   put volume
      setColor(*logi_Lv2VacLERUp, cLv2VacLERUp.getString("Color", "#CCCCCC"));
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logi_Lv2VacLERUp, "phys_Lv2VacLERUp_name", logi_Lv1TaLERUp, false, 0);

      //-
      //----------

      ////=
      ////==========
    }
  }
}
