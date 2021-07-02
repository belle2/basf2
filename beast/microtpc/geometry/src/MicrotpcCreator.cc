/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/microtpc/geometry/MicrotpcCreator.h>
#include <beast/microtpc/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Box.hh>
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include "G4Tubs.hh"

//Visualization
#include "G4Colour.hh"
#include <G4VisAttributes.hh>

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
      //m_sensitive = new SensitiveDetector();
    }

    MicrotpcCreator::~MicrotpcCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void MicrotpcCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      m_sensitive = new SensitiveDetector();

      G4String symbol;
      G4double a, z;
      G4double density, fractionmass;
      G4int ncomponents, natoms;

      G4Element* H  = new G4Element("Hydrogen",  symbol = "H"  , z =  1., a =  1.00794 * CLHEP::g / CLHEP::mole);
      G4Element* He = new G4Element("Helium",    symbol = "He" , z =  2., a =  4.002602 * CLHEP::g / CLHEP::mole);
      G4Element* C  = new G4Element("Carbon",    symbol = "C"  , z =  6., a = 12.01    * CLHEP::g / CLHEP::mole);
      G4Element* O  = new G4Element("Oxygen",    symbol = "O",   z =  8., a = 16.00    * CLHEP::g / CLHEP::mole);
      G4Element* Si = new G4Element("Silicon",   symbol = "Si" , z = 14., a = 28.09    * CLHEP::g / CLHEP::mole);
      G4Element* Cl = new G4Element("Chlore",    symbol = "Cl" , z = 17., a = 35.453   * CLHEP::g / CLHEP::mole);
      G4Element* Cu = new G4Element("Copper",    symbol = "Cu" , z = 29., a = 63.546   * CLHEP::g / CLHEP::mole);
      G4Element* Zn = new G4Element("Zinc",      symbol = "Zn" , z = 30., a = 65.38    * CLHEP::g / CLHEP::mole);

      //Helium 4
      G4Material* gas_4He = new G4Material("gas_4He", density = 0.0001664 * CLHEP::g / CLHEP::cm3, ncomponents = 1, kStateGas,
                                           293.15 * CLHEP::kelvin,  1.*CLHEP::atmosphere);
      gas_4He->AddElement(He, natoms = 1);
      //C02
      G4Material* gas_CO2 = new G4Material("gas_CO2", density = 0.001842 * CLHEP::g / CLHEP::cm3, ncomponents = 2, kStateGas,
                                           293.15 * CLHEP::kelvin, 1.*CLHEP::atmosphere);
      gas_CO2->AddElement(C, natoms = 1);
      gas_CO2->AddElement(O, natoms = 2);

      //70/30 - 4He/ CO2
      G4Material* gasmix_4HeCO2 = new G4Material("gasmix_4HeCO2", density = 0.00066908 * CLHEP::g / CLHEP::cm3, ncomponents = 2,
                                                 kStateGas,  293.15 * CLHEP::kelvin, 1.*CLHEP::atmosphere);
      gasmix_4HeCO2->AddMaterial(gas_4He, fractionmass = 17.409 * CLHEP::perCent);
      gasmix_4HeCO2->AddMaterial(gas_CO2, fractionmass = 82.591 * CLHEP::perCent);

      //c8h7cl
      G4Material* TPC_ParylenC = new G4Material("TPC_ParylenC", density = 1.298 * CLHEP::g / CLHEP::cm3, ncomponents = 3);
      TPC_ParylenC->AddElement(H, fractionmass = 0.050908 * CLHEP::perCent);
      TPC_ParylenC->AddElement(C, fractionmass = 0.693276 * CLHEP::perCent);
      TPC_ParylenC->AddElement(Cl, fractionmass = 0.255816 * CLHEP::perCent);

      //G10
      G4Material* TPC_G10 = new G4Material("TPC_G10", density = 1.700 * CLHEP::g / CLHEP::cm3, ncomponents = 4);
      TPC_G10->AddElement(Si, natoms = 1);
      TPC_G10->AddElement(O , natoms = 2);
      TPC_G10->AddElement(C , natoms = 3);
      TPC_G10->AddElement(H , natoms = 3);

      //Cu - copper
      G4Material* metalCu = new G4Material("MetalCopper", density = 8.960 * CLHEP::g / CLHEP::cm3, ncomponents = 1);
      metalCu->AddElement(Cu, 1);

      //Copper screen
      G4Material* TPC_metaCuScreen = new G4Material("TPC_metaCuScreen", density = 8.95 * CLHEP::g / CLHEP::cm3, ncomponents = 2);
      TPC_metaCuScreen->AddElement(Cu, fractionmass = 90.*CLHEP::perCent);
      TPC_metaCuScreen->AddElement(Zn, fractionmass = 10.*CLHEP::perCent);


      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);

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

        G4double inch = 2.54 * CLHEP::cm;

        //create vessel volume inner volume will be subtracted by "inactive" gas
        G4double dx_Vessel = activeParams.getLength("dx_Vessel") * CLHEP::cm / 2.;
        G4double dy_Vessel = activeParams.getLength("dy_Vessel") * CLHEP::cm / 2.;
        G4double dz_VesselEndCap = 1. / 8. / 2.*inch;
        G4double dz_Vessel = activeParams.getLength("dz_Vessel") * CLHEP::cm / 2. + dz_VesselEndCap;
        G4VSolid* s_Vessel = new G4Box("s_Vessel_tmp", dx_Vessel, dy_Vessel, dz_Vessel);

        G4double width = 1. / 8.*inch;
        G4double dx_iGasTPC = (dx_Vessel - width);
        G4double dy_iGasTPC = (dy_Vessel - width);
        G4double dz_iGasTPC = (dz_Vessel);

        //create "inactive" gas volume
        G4VSolid* s_iGasTPC = new G4Box("s_iGasTPC", dx_iGasTPC, dy_iGasTPC, dz_iGasTPC);

        //create subtraction ie vessel
        s_Vessel = new G4SubtractionSolid("s_Vessel", s_Vessel, s_iGasTPC, 0, G4ThreeVector(0, 0, 0));

        G4LogicalVolume* l_Vessel = new G4LogicalVolume(s_Vessel, geometry::Materials::get("TPC_Al6061"), "l_Vessel");
        G4LogicalVolume* l_iGasTPC = new G4LogicalVolume(s_iGasTPC, gasmix_4HeCO2, "l_iGasTPC");

        G4RotationMatrix* rotXx = new G4RotationMatrix();
        G4double AngleX = activeParams.getAngle("AngleX");
        G4double AngleZ = activeParams.getAngle("AngleZ");
        rotXx->rotateX(AngleX);
        rotXx->rotateZ(AngleZ);
        G4ThreeVector TPCpos = G4ThreeVector(
                                 activeParams.getLength("TPCpos_x") * CLHEP::cm,
                                 activeParams.getLength("TPCpos_y") * CLHEP::cm,
                                 activeParams.getLength("TPCpos_z") * CLHEP::cm
                               );

        new G4PVPlacement(rotXx, TPCpos, l_Vessel, "p_Vessel", &topVolume, false, 1);
        new G4PVPlacement(rotXx, TPCpos, l_iGasTPC, "p_iGasTPC", &topVolume, false, 1);

        B2INFO("Micro-TPC-" << detID << " placed at: " << TPCpos << " mm");

        /*
              //create endcap top and bottom
              G4double dx_VesselEndCap = dx_Vessel;
              G4double dy_VesselEndCap = dy_Vessel;
              G4double dz_VesselEndCap = 1. / 8. / 2.*inch;
              G4VSolid* s_VesselEndCap = new G4Box("s_VesselEndCap", dx_VesselEndCap, dy_VesselEndCap, dz_VesselEndCap);

              string matEndCap = activeParams.getString("MaterialEndCap");
              G4LogicalVolume* l_VesselEndCap = new G4LogicalVolume(s_VesselEndCap, geometry::Materials::get(matEndCap), "l_VesselEndCap");

              G4double x_VesselEndCap[2] = {0, 0};
              //G4double y_VesselEndCap[2] = {(dz_Vessel + dz_VesselEndCap)* sin(AngleX * CLHEP::deg), (-dz_VesselEndCap - dz_Vessel - 0.00001)* sin(AngleX * CLHEP::deg)};
              //G4double z_VesselEndCap[2] = {(dz_Vessel + dz_VesselEndCap)* cos(AngleX * CLHEP::deg), (-dz_VesselEndCap - dz_Vessel - 0.00001)* cos(AngleX * CLHEP::deg)};
        G4double y_VesselEndCap[2] = {(dz_Vessel + dz_VesselEndCap) * sin(AngleX * CLHEP::deg),
                    (-dz_VesselEndCap - dz_Vessel) * sin(AngleX * CLHEP::deg)};
              G4double z_VesselEndCap[2] = {(dz_Vessel + dz_VesselEndCap) * cos(AngleX * CLHEP::deg),
                    (-dz_VesselEndCap - dz_Vessel) * cos(AngleX * CLHEP::deg)};

              new G4PVPlacement(rotXx, G4ThreeVector(x_VesselEndCap[0], y_VesselEndCap[0], z_VesselEndCap[0]) + TPCpos, l_VesselEndCap,
                                "p_VesselEndCapTop", &topVolume, false, 1);
              new G4PVPlacement(rotXx, G4ThreeVector(x_VesselEndCap[1], y_VesselEndCap[1], z_VesselEndCap[1]) + TPCpos, l_VesselEndCap,
                                "p_VesselEndCapBottom", &topVolume, false, 1);
        */
        G4VisAttributes* orange = new G4VisAttributes(G4Colour(1, 2, 0));
        orange->SetForceAuxEdgeVisible(true);
        //l_VesselEndCap->SetVisAttributes(orange);
        l_Vessel->SetVisAttributes(orange);

        G4double dx_parC1 = dx_iGasTPC;
        G4double dy_parC1 = dy_iGasTPC;
        G4double dz_parC1 = dz_iGasTPC;
        G4double cwidth = 0.001 / 2. * inch;
        G4double dx_parC2 = dx_parC1 - cwidth;
        G4double dy_parC2 = dy_parC1 - cwidth;
        G4double dz_parC2 = dz_parC1 - cwidth;
        G4Box* s_parC1 = new G4Box("s_parC1", dx_parC1, dy_parC1, dz_parC1);
        G4Box* s_parC2 = new G4Box("s_parC2", dx_parC2, dy_parC2, dz_parC2);
        G4VSolid* s_parylenC = new G4SubtractionSolid("s_parylenC", s_parC1, s_parC2, 0, G4ThreeVector(0, 0, 0));
        G4LogicalVolume* l_parylenC = new G4LogicalVolume(s_parylenC, TPC_ParylenC, "l_parylenC");
        new G4PVPlacement(0, G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 0 * CLHEP::cm), l_parylenC, "p_parylenC", l_iGasTPC, false, 1);

        G4double dx_kap1 = dx_parC2;
        G4double dy_kap1 = dy_parC2;
        G4double dz_kap1 = dz_parC2;
        G4double kwidth = 0.05 / 2. * CLHEP::cm;
        G4double dx_kap2 = dx_kap1 - kwidth;
        G4double dy_kap2 = dy_kap1 - kwidth;
        G4double dz_kap2 = dz_kap1 - kwidth;
        G4Box* s_kap1 = new G4Box("s_kap1", dx_kap1, dy_kap1, dz_kap1);
        G4Box* s_kap2 = new G4Box("s_kap2", dx_kap2, dy_kap2, dz_kap2);
        G4VSolid* s_kapton = new G4SubtractionSolid("s_kapton", s_kap1, s_kap2, 0, G4ThreeVector(0, 0, 0));
        G4LogicalVolume* l_kapton = new G4LogicalVolume(s_kapton, geometry::Materials::get("TPC_Kapton"), "l_kapton");
        new G4PVPlacement(0, G4ThreeVector(0 * CLHEP::cm, 0 * CLHEP::cm, 0 * CLHEP::cm), l_kapton, "p_kapton", l_iGasTPC, false, 1);

        //ring
        G4double w = 1.4 * CLHEP::cm;
        G4double rodx = 9.476 * CLHEP::cm;
        G4double rody = 6.976 * CLHEP::cm;
        G4double ridx = rodx - w;
        G4double ridy = rody - w;
        //rod hole position
        G4double xrodh = ridx / 2. + w / 8.;
        G4double yrodh = ridy / 2. + w / 8.;

        //create rods
        G4double iR_Rod = 0.*CLHEP::mm;
        G4double oR_Rod = 5. / 2.*CLHEP::mm;
        G4double h_Rod  = 20. / 2. * CLHEP::cm;
        G4double sA_Rod = 0.*CLHEP::deg;
        G4double spA_Rod = 360.*CLHEP::deg;
        /*cout << "h_Rod " << h_Rod / CLHEP::cm
             << " dx " << dx_kap2 / CLHEP::cm
             << " dy " << dy_kap2 / CLHEP::cm
             << " dz " << dz_kap2 / CLHEP::cm
             << endl;*/
        //G4double x_Rod[4] = {3.2 * CLHEP::cm, 3.2 * CLHEP::cm, -3.2 * CLHEP::cm, -3.2 * CLHEP::cm};
        //G4double y_Rod[4] = {3.2 * CLHEP::cm, -3.2 * CLHEP::cm, 3.2 * CLHEP::cm, -3.2 * CLHEP::cm};
        G4double x_Rod[4] = {xrodh, xrodh, -xrodh, -xrodh};
        G4double y_Rod[4] = {yrodh, -yrodh, yrodh, -yrodh};
        //G4double z_Rod = -dz_iGasTPC + h_Rod;
        G4double z_Rod = 0. * CLHEP::cm;

        G4Tubs* s_Rod = new G4Tubs("s_Rod", iR_Rod, oR_Rod, h_Rod, sA_Rod, spA_Rod);
        G4LogicalVolume* l_Rod = new G4LogicalVolume(s_Rod, geometry::Materials::get("G4_POLYVINYL_ACETATE"), "l_Rod");

        new G4PVPlacement(0, G4ThreeVector(x_Rod[0], y_Rod[0], z_Rod), l_Rod, "p_Rod_0", l_iGasTPC, false, 1);
        new G4PVPlacement(0, G4ThreeVector(x_Rod[1], y_Rod[1], z_Rod), l_Rod, "p_Rod_1", l_iGasTPC, false, 1);
        new G4PVPlacement(0, G4ThreeVector(x_Rod[2], y_Rod[2], z_Rod), l_Rod, "p_Rod_2", l_iGasTPC, false, 1);
        new G4PVPlacement(0, G4ThreeVector(x_Rod[3], y_Rod[3], z_Rod), l_Rod, "p_Rod_3", l_iGasTPC, false, 1);

        //create the rings
        //G4double dx_Ring = 7.4 / 2.*CLHEP::cm;
        //G4double dy_Ring = 7.4 / 2.*CLHEP::cm;
        G4double dx_Ring = rodx / 2.;
        G4double dy_Ring = rody / 2.;
        G4double dz_Ring = 0.127 / 2.*CLHEP::cm;

        G4VSolid* s_Ring = new G4Box("s_RingFilled", dx_Ring, dy_Ring, dz_Ring);
        //G4double dx_RingHole = 5.4 / 2.*CLHEP::cm;
        //G4double dy_RingHole = 5.4 / 2.*CLHEP::cm;
        G4double dx_RingHole = xrodh / 2.;
        G4double dy_RingHole = yrodh / 2.;
        G4double dz_RingHole = dz_Ring;
        G4VSolid* s_RingHole = new G4Box("s_RingHole", dx_RingHole, dy_RingHole, dz_RingHole);
        s_Ring = new G4SubtractionSolid("s_Ring", s_Ring, s_RingHole, 0, G4ThreeVector(0, 0, 0));

        G4double iR_RingHoles = 0.*CLHEP::mm;
        G4double oR_RingHoles = oR_Rod;
        G4double h_RingHoles = dz_Ring;
        G4double sA_RingHoles = 0.*CLHEP::deg;
        G4double spA_RingHoles = 360.*CLHEP::deg;
        G4VSolid* s_RingHoles = new G4Tubs("s_RingHoles", iR_RingHoles, oR_RingHoles, h_RingHoles, sA_RingHoles, spA_RingHoles);
        char Name[400];
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_Ring_%d", i);
          s_Ring = new G4SubtractionSolid(Name, s_Ring, s_RingHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }

        G4LogicalVolume* l_Ring = new G4LogicalVolume(s_Ring, geometry::Materials::get("TPC_Al6061"), "l_Ring");
        G4int RingNb = 10;
        G4double hspacer = 1.*CLHEP::cm;
        G4double offset = dz_iGasTPC - 5.*CLHEP::cm;

        G4double x_Ring[40];
        G4double y_Ring[40];
        G4double z_Ring[40];
        for (G4int i = 0; i < RingNb; i++) {
          x_Ring[i] = 0;
          y_Ring[i] = 0;
          z_Ring[i] = -dz_iGasTPC + offset + (hspacer + 2. * dz_Ring) * i;
          //cout << "z ring # " << i << " pos. " << z_Ring[i] / CLHEP::cm << endl;
          sprintf(Name, "p_Ring_%d", i);
          new G4PVPlacement(0, G4ThreeVector(x_Ring[i], y_Ring[i], z_Ring[i]), l_Ring, Name, l_iGasTPC, false, 1);
        }

        //create anode
        G4VSolid* s_Anode = new G4Box("s_Anode", dx_Ring, dy_Ring, dz_Ring);
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_Anode_%d", i);
          s_Anode = new G4SubtractionSolid(Name, s_Anode, s_RingHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }
        G4LogicalVolume* l_Anode = new G4LogicalVolume(s_Anode, geometry::Materials::get("TPC_Al6061"), "l_Anode");
        x_Ring[10] = 0;
        y_Ring[10] = 0;
        z_Ring[10] = -dz_iGasTPC + offset + (hspacer + 2.*dz_Ring) * RingNb;
        new G4PVPlacement(0, G4ThreeVector(x_Ring[RingNb], y_Ring[RingNb], z_Ring[RingNb]), l_Anode, "p_Anode", l_iGasTPC, false, 1);

        //create ring spacer
        G4double iR_RSpacer = oR_Rod;
        G4double oR_RSpacer = oR_Rod + 2.*CLHEP::mm;
        G4double h_RSpacer = (z_Ring[1] - dz_Ring - (z_Ring[0] + dz_Ring)) / 2.;
        G4double sA_RSpacer = 0.*CLHEP::deg;
        G4double spA_RSpacer = 360.*CLHEP::deg;
        G4Tubs* s_RSpacer = new G4Tubs("s_RSpacer1", iR_RSpacer, oR_RSpacer, h_RSpacer, sA_RSpacer, spA_RSpacer);
        G4LogicalVolume* l_RSpacer = new G4LogicalVolume(s_RSpacer, geometry::Materials::get("G4_POLYVINYL_ACETATE"), "l_RSpacer");

        G4double x_RSpacer[40];
        G4double y_RSpacer[40];
        G4double z_RSpacer[40];
        for (G4int i = 0; i < 10; i++) {
          for (G4int k = 0; k < 4; k++) {
            x_RSpacer[i] = x_Rod[k];
            y_RSpacer[i] = y_Rod[k];
            z_RSpacer[i] = z_Ring[i] + dz_Ring + h_RSpacer;
            sprintf(Name, "p_RSpacer_%d_%d", i, k);
            new G4PVPlacement(0, G4ThreeVector(x_RSpacer[i], y_RSpacer[i], z_RSpacer[i]), l_RSpacer, Name, l_iGasTPC, false, 1);
          }
        }

        //create GEM
        G4double dx_GEM = 50. / 2.*CLHEP::mm;
        G4double dy_GEM = 50. / 2.*CLHEP::mm;
        G4double dz_GEM = 1.6 / 2.*CLHEP::mm;

        G4double x_GEM = 0.*CLHEP::cm;
        G4double y_GEM = 0.*CLHEP::cm;
        //G4double z_GEM[] = { -dz_iGasTPC + 2.*CLHEP::cm - 0.21 * CLHEP::cm - dz_GEM, -dz_iGasTPC + 2.*CLHEP::cm - dz_GEM};
        G4double z_GEM[] = { z_Ring[0] - dz_Ring - dz_GEM, z_Ring[0] - dz_Ring - 2. * dz_GEM - 0.28 * CLHEP::cm};
        /*cout << "ring 1 " << z_Ring[1] / CLHEP::cm  << " ring 0 " << z_Ring[0] / CLHEP::cm << " gem 1 " << z_GEM[0] / CLHEP::cm << " gem 2 "
        << z_GEM[1] / CLHEP::cm << endl;*/

        G4VSolid* s_GEM = new G4Box("s_GEM", dx_GEM, dy_GEM, dz_GEM);
        G4LogicalVolume* l_GEM = new G4LogicalVolume(s_GEM, geometry::Materials::get("TPC_Kovar"), "l_GEM");
        for (G4int i = 0; i < 2; i++) {
          sprintf(Name, "p_GEM_%d", i);
          new G4PVPlacement(0, G4ThreeVector(x_GEM, y_GEM, z_GEM[i]), l_GEM, Name, l_iGasTPC, false, 1);
        }

        //create GEM support holder
        G4double dx_GEMSupport = dx_Ring;
        G4double dy_GEMSupport = dy_Ring;
        G4double dz_GEMSupport = dz_GEM;
        G4VSolid* s_GEMSupport = new G4Box("s_GEMSupport1", dx_GEMSupport, dy_GEMSupport, dz_GEMSupport);

        G4double iR_GEMSupportHoles = 0.*CLHEP::mm;
        G4double oR_GEMSupportHoles = oR_Rod;
        G4double h_GEMSupportHoles = dz_GEMSupport;
        G4double sA_GEMSupportHoles = 0.*CLHEP::deg;
        G4double spA_GEMSupportHoles = 360.*CLHEP::deg;
        G4VSolid* s_GEMSupportHoles = new G4Tubs("s_GEMSupportHoles", iR_GEMSupportHoles, oR_GEMSupportHoles, h_GEMSupportHoles,
                                                 sA_GEMSupportHoles, spA_GEMSupportHoles);
        s_GEMSupport = new G4SubtractionSolid("s_GEMSupport2", s_GEMSupport, s_GEM, 0, G4ThreeVector(0, 0, 0));
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_GEMSupport_%d", i);
          s_GEMSupport = new G4SubtractionSolid(Name, s_GEMSupport, s_GEMSupportHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }

        G4LogicalVolume* l_GEMSupport = new G4LogicalVolume(s_GEMSupport, TPC_G10, "l_GEMSupport");
        for (G4int i = 0; i < 2; i++) {
          sprintf(Name, "p_GEMSupport_%d", i);
          new G4PVPlacement(0, G4ThreeVector(x_GEM, y_GEM, z_GEM[i]), l_GEMSupport, Name, l_iGasTPC, false, 1);
        }
        //cout <<"gem 1 " << z_GEM[0] << " gem 2 " << z_GEM[1] << " ring 22 " << z_Ring[22] << " anode " << z_Ring[23] << " dz_GEM " << dz_GEM << " dz_Ring " << dz_Ring << endl;
        //create sensitive volume
        G4double dx_GasTPC = 2.95 / 2. * CLHEP::cm;
        G4double dy_GasTPC = 2.95 / 2. * CLHEP::cm;
        G4double dz_GasTPC = (z_Ring[10] - dz_Ring - z_GEM[0] - dz_GEM) / 2.; //13.5 * CLHEP::cm;
        cout << " dz_GasTPC " << dz_GasTPC / CLHEP::cm << endl;
        G4Box* s_GasTPC = new G4Box("s_GasTPC", dx_GasTPC, dy_GasTPC, dz_GasTPC);
        G4LogicalVolume* l_GasTPC = new G4LogicalVolume(s_GasTPC, gasmix_4HeCO2, "l_GasTPC", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_GasTPC->SetUserLimits(new G4UserLimits(stepSize));

        G4double x_GasTPC = 0;
        G4double y_GasTPC = 0;
        G4double z_GasTPC = z_GEM[0] + dz_GEM + dz_GasTPC;

        new G4PVPlacement(0, G4ThreeVector(x_GasTPC, y_GasTPC, z_GasTPC), l_GasTPC, "p_GasTPC", l_iGasTPC, false, detID);

        B2INFO("Micro-TPC-Sensitive-Volume-" << detID << " placed at: (" << TPCpos.getX() + x_GasTPC << "," << TPCpos.getY() + y_GasTPC <<
               "," << TPCpos.getZ() + z_GasTPC << ") mm");

        //create pixel chip
        G4double dx_PixelChip = dx_GasTPC;
        G4double dy_PixelChip = dy_GasTPC;
        G4double dz_PixelChip = 1. / 2.*CLHEP::mm;

        G4double x_PixelChip = 0.*CLHEP::mm;
        G4double y_PixelChip = 0.*CLHEP::mm;
        G4double z_PixelChip = z_GEM[1] - 0.3 * CLHEP::cm;

        G4Box* s_PixelChip = new G4Box("s_PixelChip", dx_PixelChip, dy_PixelChip, dz_PixelChip);
        G4LogicalVolume* l_PixelChip = new G4LogicalVolume(s_PixelChip, geometry::Materials::get("G4_PLASTIC_SC_VINYLTOLUENE"),
                                                           "l_PixelChip");
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


        G4LogicalVolume* l_CuPlate = new G4LogicalVolume(s_CuPlate, TPC_metaCuScreen, "l_CuPlate");
        new G4PVPlacement(0, G4ThreeVector(x_PixelChip, y_PixelChip, z_PixelChip), l_CuPlate, "p_CuPlate", l_iGasTPC, false, 1);

        //create pixel board
        G4double dx_PixelBoard = dx_Ring;
        G4double dy_PixelBoard = dy_Ring;
        G4double dz_PixelBoard = 2.*CLHEP::mm;

        G4VSolid* s_PixelBoard = new G4Box("s_PixelBoard1", dx_PixelBoard, dy_PixelBoard, dz_PixelBoard);
        G4VSolid* s_PixelBoardHoles = new G4Tubs("s_PixelBoardHoles", iR_RingHoles, oR_RingHoles, dz_PixelBoard, sA_RingHoles,
                                                 spA_RingHoles);
        for (G4int i = 0; i < 4; i++) {
          sprintf(Name, "s_PixelBoard_%d", i);
          s_PixelBoard = new G4SubtractionSolid(Name, s_PixelBoard, s_PixelBoardHoles, 0, G4ThreeVector(x_Rod[i], y_Rod[i], 0));
        }

        G4double x_PixelBoard = 0.*CLHEP::mm;
        G4double y_PixelBoard = 0.*CLHEP::mm;
        G4double z_PixelBoard = z_PixelChip - dz_PixelBoard - dz_PixelChip;

        G4LogicalVolume* l_PixelBoard = new G4LogicalVolume(s_PixelBoard, TPC_G10, "l_PixelBoard");

        new G4PVPlacement(0, G4ThreeVector(x_PixelBoard, y_PixelBoard, z_PixelBoard), l_PixelBoard, "p_PixelBoard", l_iGasTPC, false, 1);
        detID++;
      }
    }
  } // microtpc namespace
} // Belle2 namespace
