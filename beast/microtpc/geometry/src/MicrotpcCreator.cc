/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/geometry/MicrotpcCreator.h>
#include <beast/microtpc/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <microtpc/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include <G4Polycone.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the MICROTPC detector */
  namespace microtpc {

    // Register the creator
    /** Creator creates the micro-tpc geometry */
    geometry::CreatorFactory<MicrotpcCreator> MicrotpcFactory("MICROTPCCreator");

    MicrotpcCreator::MicrotpcCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    MicrotpcCreator::~MicrotpcCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void MicrotpcCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * Unit::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create vessel volume inner volume will be subtracted by "inactive" gas
        G4double dx_Vessel = activeParams.getLength("dx_Vessel") / 2.*Unit::cm;
        G4double dy_Vessel = activeParams.getLength("dy_Vessel") / 2.*Unit::cm;
        G4double dz_Vessel = activeParams.getLength("dz_Vessel") / 2.*Unit::cm;
        G4VSolid* s_Vessel = new G4Box("s_Vessel_tmp", dx_Vessel, dy_Vessel, dz_Vessel);

        G4double inch = 2.54 * Unit::cm;
        G4double width = 1. / 8.*inch;
        G4double dx_iGasTPC = (dx_Vessel - width);
        G4double dy_iGasTPC = (dy_Vessel - width);
        G4double dz_iGasTPC = (dz_Vessel);

        //create "inactive" gas volume
        G4VSolid* s_iGasTPC = new G4Box("s_iGasTPC", dx_iGasTPC, dy_iGasTPC, dz_iGasTPC);

        //create subtraction ie vessel
        s_Vessel = new G4SubtractionSolid("s_Vessel", s_Vessel, s_iGasTPC, 0, G4ThreeVector(0, 0, 0));

        string matVessel = activeParams.getString("MaterialVessel");
        G4LogicalVolume* l_Vessel = new G4LogicalVolume(s_Vessel, geometry::Materials::get(matVessel), "l_Vessel");
        string matGas = activeParams.getString("MaterialGas");
        G4LogicalVolume* l_iGasTPC = new G4LogicalVolume(s_iGasTPC, geometry::Materials::get(matGas), "l_iGasTPC");

        G4RotationMatrix* rotXx = new G4RotationMatrix();
        G4double AngleX = activeParams.getLength("AngleX");
        G4double AngleZ = activeParams.getLength("AngleZ");
        rotXx->rotateX(AngleX);
        rotXx->rotateZ(AngleZ);
        G4ThreeVector TPCpos = G4ThreeVector(
                                 activeParams.getLength("TPCpos_x") * Unit::cm,
                                 activeParams.getLength("TPCpos_y") * Unit::cm,
                                 activeParams.getLength("TPCpos_z") * Unit::cm
                               );

        new G4PVPlacement(rotXx, TPCpos, l_Vessel, "p_Vessel", &topVolume, false, 1);
        new G4PVPlacement(rotXx, TPCpos, l_iGasTPC, "p_iGasTPC", &topVolume, false, 1);

        //create endcap top and bottom
        G4double dx_VesselEndCap = dx_Vessel;
        G4double dy_VesselEndCap = dy_Vessel;
        G4double dz_VesselEndCap = 1. / 8. / 2.*inch;
        G4VSolid* s_VesselEndCap = new G4Box("s_VesselEndCap", dx_VesselEndCap, dy_VesselEndCap, dz_VesselEndCap);

        string matEndCap = activeParams.getString("MaterialEndCap");
        G4LogicalVolume* l_VesselEndCap = new G4LogicalVolume(s_VesselEndCap, geometry::Materials::get(matEndCap), "l_VesselEndCap");

        G4double x_VesselEndCap[2] = {0, 0};
        G4double y_VesselEndCap[2] = {(dz_Vessel + dz_VesselEndCap)* sin(AngleX * Unit::deg), (-dz_VesselEndCap - dz_Vessel - 0.00001)* sin(AngleX * Unit::deg)};
        G4double z_VesselEndCap[2] = {(dz_Vessel + dz_VesselEndCap)* cos(AngleX * Unit::deg), (-dz_VesselEndCap - dz_Vessel - 0.00001)* cos(AngleX * Unit::deg)};

        new G4PVPlacement(rotXx, G4ThreeVector(x_VesselEndCap[0], y_VesselEndCap[0], z_VesselEndCap[0]) + TPCpos, l_VesselEndCap, "p_VesselEndCapTop", &topVolume, false, 1);
        new G4PVPlacement(rotXx, G4ThreeVector(x_VesselEndCap[1], y_VesselEndCap[1], z_VesselEndCap[1]) + TPCpos, l_VesselEndCap, "p_VesselEndCapBottom", &topVolume, false, 1);

        //create rods
        G4double iR_Rod = 0.*Unit::mm;
        G4double oR_Rod = 5. / 2.*Unit::mm;
        G4double h_Rod  = dz_Vessel - 0.5 * Unit::cm;
        G4double sA_Rod = 0.*Unit::deg;
        G4double spA_Rod = 360.*Unit::deg;

        G4double x_Rod[4] = {3.2 * Unit::cm, 3.2 * Unit::cm, -3.2 * Unit::cm, -3.2 * Unit::cm};
        G4double y_Rod[4] = {3.2 * Unit::cm, -3.2 * Unit::cm, 3.2 * Unit::cm, -3.2 * Unit::cm};
        G4double z_Rod = -dz_iGasTPC + h_Rod;

        G4Tubs* s_Rod = new G4Tubs("s_Rod", iR_Rod, oR_Rod, h_Rod, sA_Rod, spA_Rod);
        G4LogicalVolume* l_Rod = new G4LogicalVolume(s_Rod, geometry::Materials::get("G4_POLYVINYL_ACETATE"), "l_Rod");

        new G4PVPlacement(0, G4ThreeVector(x_Rod[0], y_Rod[0], z_Rod), l_Rod, "p_Rod_0", l_iGasTPC, false, 1);
        new G4PVPlacement(0, G4ThreeVector(x_Rod[1], y_Rod[1], z_Rod), l_Rod, "p_Rod_1", l_iGasTPC, false, 1);
        new G4PVPlacement(0, G4ThreeVector(x_Rod[2], y_Rod[2], z_Rod), l_Rod, "p_Rod_2", l_iGasTPC, false, 1);
        new G4PVPlacement(0, G4ThreeVector(x_Rod[3], y_Rod[3], z_Rod), l_Rod, "p_Rod_3", l_iGasTPC, false, 1);

        //create the rings
        G4double dx_Ring = 7.4 / 2.*Unit::cm;
        G4double dy_Ring = 7.4 / 2.*Unit::cm;
        G4double dz_Ring = 1.27 / 2.*Unit::mm;

        G4VSolid* s_Ring = new G4Box("s_RingFilled", dx_Ring, dy_Ring, dz_Ring);
        G4double dx_RingHole = 5.4 / 2.*Unit::cm;
        G4double dy_RingHole = 5.4 / 2.*Unit::cm;
        G4double dz_RingHole = dz_Ring;
        G4VSolid* s_RingHole = new G4Box("s_RingHole", dx_RingHole, dy_RingHole, dz_RingHole);
        s_Ring = new G4SubtractionSolid("s_Ring", s_Ring, s_RingHole, 0, G4ThreeVector(0, 0, 0));

        G4double iR_RingHoles = 0.*Unit::mm;
        G4double oR_RingHoles = oR_Rod;
        G4double h_RingHoles = dz_Ring;
        G4double sA_RingHoles = 0.*Unit::deg;
        G4double spA_RingHoles = 360.*Unit::deg;
        G4VSolid* s_RingHoles = new G4Tubs("s_RingHoles", iR_RingHoles, oR_RingHoles, h_RingHoles, sA_RingHoles, spA_RingHoles);
        char Name[400];
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_Ring_%d", i);
          s_Ring = new G4SubtractionSolid(Name, s_Ring, s_RingHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }

        G4LogicalVolume* l_Ring = new G4LogicalVolume(s_Ring, geometry::Materials::get("MetalCopper"), "l_Ring");

        G4double x_Ring[40];
        G4double y_Ring[40];
        G4double z_Ring[40];
        for (G4int i = 0; i < 23; i++) {
          x_Ring[i] = 0;
          y_Ring[i] = 0;
          z_Ring[i] = -dz_iGasTPC + 3.*Unit::cm + 1.*i * Unit::cm + 2.*dz_Ring * i * Unit::mm;
          sprintf(Name, "p_Ring_%d", i);
          new G4PVPlacement(0, G4ThreeVector(x_Ring[i], y_Ring[i], z_Ring[i]), l_Ring, Name, l_iGasTPC, false, 1);
        }

        //create anode
        G4VSolid* s_Anode = new G4Box("s_Anode", dx_Ring, dy_Ring, dz_Ring);
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_Anode_%d", i);
          s_Anode = new G4SubtractionSolid(Name, s_Anode, s_RingHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }
        G4LogicalVolume* l_Anode = new G4LogicalVolume(s_Anode, geometry::Materials::get("MetalCopper"), "l_Anode");
        x_Ring[23] = 0;
        y_Ring[23] = 0;
        z_Ring[23] = -dz_iGasTPC + 3.*Unit::cm + 1.*23.*Unit::cm  + 2.*dz_Ring * 23.*Unit::mm;
        new G4PVPlacement(0, G4ThreeVector(x_Ring[23], y_Ring[23], z_Ring[23]), l_Anode, "p_Anode", l_iGasTPC, false, 1);

        //create ring spacer
        G4double iR_RSpacer = oR_Rod;
        G4double oR_RSpacer = oR_Rod + 2.*Unit::mm;
        G4double h_RSpacer = (z_Ring[1] - dz_Ring - (z_Ring[0] + dz_Ring)) / 2.;
        G4double sA_RSpacer = 0.*Unit::deg;
        G4double spA_RSpacer = 360.*Unit::deg;
        G4Tubs* s_RSpacer = new G4Tubs("s_RSpacer1", iR_RSpacer, oR_RSpacer, h_RSpacer, sA_RSpacer, spA_RSpacer);
        G4LogicalVolume* l_RSpacer = new G4LogicalVolume(s_RSpacer, geometry::Materials::get("G4_POLYVINYL_ACETATE"), "l_RSpacer");

        G4double x_RSpacer[40];
        G4double y_RSpacer[40];
        G4double z_RSpacer[40];
        for (G4int i = 0; i < 23; i++) {
          for (G4int k = 0; k < 4; k++) {
            x_RSpacer[i] = x_Rod[k];
            y_RSpacer[i] = y_Rod[k];
            z_RSpacer[i] = z_Ring[i] + dz_Ring + h_RSpacer;
            sprintf(Name, "p_RSpacer_%d_%d", i, k);
            new G4PVPlacement(0, G4ThreeVector(x_RSpacer[i], y_RSpacer[i], z_RSpacer[i]), l_RSpacer, Name, l_iGasTPC, false, 1);
          }
        }

        //create GEM
        G4double dx_GEM = 50. / 2.*Unit::mm;
        G4double dy_GEM = 50. / 2.*Unit::mm;
        G4double dz_GEM = 1.6 / 2.*Unit::mm;

        G4double x_GEM = 0.*Unit::mm;
        G4double y_GEM = 0.*Unit::mm;
        G4double z_GEM[] = { -dz_iGasTPC + 2.*Unit::cm - 0.21 * Unit::cm - dz_GEM, -dz_iGasTPC + 2.*Unit::cm - dz_GEM};

        G4VSolid* s_GEM = new G4Box("s_GEM", dx_GEM, dy_GEM, dz_GEM);
        G4LogicalVolume* l_GEM = new G4LogicalVolume(s_GEM, geometry::Materials::get("Kovar"), "l_GEM");
        for (G4int i = 0; i < 2; i++) {
          sprintf(Name, "p_GEM_%d", i);
          new G4PVPlacement(0, G4ThreeVector(x_GEM, y_GEM, z_GEM[i]), l_GEM, Name, l_iGasTPC, false, 1);
        }

        //create GEM support holder
        G4double dx_GEMSupport = dx_Ring;
        G4double dy_GEMSupport = dy_Ring;
        G4double dz_GEMSupport = dz_GEM;
        G4VSolid* s_GEMSupport = new G4Box("s_GEMSupport1", dx_GEMSupport, dy_GEMSupport, dz_GEMSupport);

        G4double iR_GEMSupportHoles = 0.*Unit::mm;
        G4double oR_GEMSupportHoles = oR_Rod;
        G4double h_GEMSupportHoles = dz_GEMSupport;
        G4double sA_GEMSupportHoles = 0.*Unit::deg;
        G4double spA_GEMSupportHoles = 360.*Unit::deg;
        G4VSolid* s_GEMSupportHoles = new G4Tubs("s_GEMSupportHoles", iR_GEMSupportHoles, oR_GEMSupportHoles, h_GEMSupportHoles, sA_GEMSupportHoles, spA_GEMSupportHoles);
        s_GEMSupport = new G4SubtractionSolid("s_GEMSupport2", s_GEMSupport, s_GEM, 0, G4ThreeVector(0, 0, 0));
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_GEMSupport_%d", i);
          s_GEMSupport = new G4SubtractionSolid(Name, s_GEMSupport, s_GEMSupportHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }

        G4LogicalVolume* l_GEMSupport = new G4LogicalVolume(s_GEMSupport, geometry::Materials::get("G10"), "l_GEMSupport");
        for (G4int i = 0; i < 2; i++) {
          sprintf(Name, "p_GEMSupport_%d", i);
          new G4PVPlacement(0, G4ThreeVector(x_GEM, y_GEM, z_GEM[i]), l_GEMSupport, Name, l_iGasTPC, false, 1);
        }
        //cout <<"gem 1 " << z_GEM[0] << " gem 2 " << z_GEM[1] << " ring 22 " << z_Ring[22] << " anode " << z_Ring[23] << " dz_GEM " << dz_GEM << " dz_Ring " << dz_Ring << endl;
        //create sensitive volume
        G4double dx_GasTPC = 2.5 * Unit::cm;
        G4double dy_GasTPC = 2.5 * Unit::cm;
        G4double dz_GasTPC = (z_Ring[23] - dz_Ring - z_GEM[1] - dz_GEM) / 2. * Unit::cm; //13.5 * Unit::cm;
        //cout << " dz_GasTPC " << dz_GasTPC << endl;
        G4Box* s_GasTPC = new G4Box("s_GasTPC", dx_GasTPC, dy_GasTPC, dz_GasTPC);
        G4LogicalVolume* l_GasTPC = new G4LogicalVolume(s_GasTPC, geometry::Materials::get(matGas), "l_GasTPC", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_GasTPC->SetUserLimits(new G4UserLimits(stepSize));

        G4double x_GasTPC = 0;
        G4double y_GasTPC = 0;
        G4double z_GasTPC = z_GEM[1] + dz_GEM + dz_GasTPC;

        new G4PVPlacement(0, G4ThreeVector(x_GasTPC, y_GasTPC, z_GasTPC), l_GasTPC, "p_GasTPC", l_iGasTPC, false, detID);

        //create pixel chip
        G4double dx_PixelChip = dx_GasTPC;
        G4double dy_PixelChip = dy_GasTPC;
        G4double dz_PixelChip = 1. / 2.*Unit::mm;

        G4double x_PixelChip = 0.*Unit::mm;
        G4double y_PixelChip = 0.*Unit::mm;
        G4double z_PixelChip = z_GEM[0] - 0.3 * Unit::cm;

        G4Box* s_PixelChip = new G4Box("s_PixelChip", dx_PixelChip, dy_PixelChip, dz_PixelChip);
        G4LogicalVolume* l_PixelChip = new G4LogicalVolume(s_PixelChip, geometry::Materials::get("G4_PLASTIC_SC_VINYLTOLUENE"), "l_PixelChip");
        new G4PVPlacement(0, G4ThreeVector(x_PixelChip, y_PixelChip, z_PixelChip), l_PixelChip, "p_PixelChip", l_iGasTPC, false, 1);

        //create cu plate
        G4double dx_CuPlate = dx_Ring;
        G4double dy_CuPlate = dy_Ring;
        G4double dz_CuPlate = dz_PixelChip;

        G4VSolid* s_CuPlate = new G4Box("s_CuPlate", dx_CuPlate, dy_CuPlate, dz_CuPlate);
        G4VSolid* s_HolesInCuPlate = new G4Tubs("HolesInCuPlate", iR_RingHoles, oR_RingHoles, dz_PixelChip, sA_RingHoles, spA_RingHoles);
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_CuPlate_%d", i);
          s_CuPlate = new G4SubtractionSolid(Name, s_CuPlate, s_HolesInCuPlate, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }
        s_CuPlate = new G4SubtractionSolid("s_CuPlate", s_CuPlate, s_PixelChip, 0, G4ThreeVector(0, 0, 0));


        G4LogicalVolume* l_CuPlate = new G4LogicalVolume(s_CuPlate, geometry::Materials::get("MetalCopper"), "l_CuPlate");
        new G4PVPlacement(0, G4ThreeVector(x_PixelChip, y_PixelChip, z_PixelChip), l_CuPlate, "p_CuPlate", l_iGasTPC, false, 1);

        //create pixel board
        G4double dx_PixelBoard = dx_Ring;
        G4double dy_PixelBoard = dy_Ring;
        G4double dz_PixelBoard = 2.*Unit::mm;

        G4VSolid* s_PixelBoard = new G4Box("s_PixelBoard1", dx_PixelBoard, dy_PixelBoard, dz_PixelBoard);
        G4VSolid* s_PixelBoardHoles = new G4Tubs("s_PixelBoardHoles", iR_RingHoles, oR_RingHoles, dz_PixelBoard, sA_RingHoles, spA_RingHoles);
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_PixelBoard_%d", i);
          s_PixelBoard = new G4SubtractionSolid(Name, s_PixelBoard, s_PixelBoardHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }

        G4double x_PixelBoard = 0.*Unit::mm;
        G4double y_PixelBoard = 0.*Unit::mm;
        G4double z_PixelBoard = z_PixelChip - dz_PixelBoard - dz_PixelChip;

        G4LogicalVolume* l_PixelBoard = new G4LogicalVolume(s_PixelBoard, geometry::Materials::get("G10"), "l_PixelBoard");

        new G4PVPlacement(0, G4ThreeVector(x_PixelBoard, y_PixelBoard, z_PixelBoard), l_PixelBoard, "p_PixelBoard", l_iGasTPC, false, 1);
        detID++;
      }
    }
  } // microtpc namespace
} // Belle2 namespace
