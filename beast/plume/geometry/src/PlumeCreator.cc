/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle, Isabelle Ripp-Baudot         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/plume/geometry/PlumeCreator.h>
#include <beast/plume/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <plume/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"

//Visualization Attributes
#include <G4VisAttributes.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PLUME detector */
  namespace plume {

    // Register the creator
    /** Creator creates the PLUME geometry */
    geometry::CreatorFactory<PlumeCreator> PlumeFactory("PLUMECreator");

    PlumeCreator::PlumeCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    PlumeCreator::~PlumeCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void PlumeCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {

      //color attributions
      G4VisAttributes* FoamVisAtt  = new G4VisAttributes(G4Colour::Blue());
      G4VisAttributes* KaptonVisAtt = new G4VisAttributes(G4Colour::Red());
      G4VisAttributes* GlueVisAtt = new G4VisAttributes(G4Colour::White());
      G4VisAttributes* MetalVisAtt = new G4VisAttributes(G4Colour::Gray());
      G4VisAttributes* SubstrateVisAtt  = new G4VisAttributes(G4Colour::Green());
      G4VisAttributes* MetalizedVisAtt  = new G4VisAttributes(G4Colour::Cyan());
      G4VisAttributes* EpitaxialVisAtt  = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0)); //yellow

      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * CLHEP::um);
      /*
      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      */
      int LadderID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //Create LADDER, LADDER is made of 7 layers
        G4double r = activeParams.getLength("r_plume") * CLHEP::cm;
        G4double z = activeParams.getLength("z_plume") * CLHEP::cm;
        G4double phi = activeParams.getAngle("Phi");
        G4double thetaZ = activeParams.getAngle("ThetaZ");

        G4double ElecBandY = activeParams.getLength("ElecBandY") * CLHEP::cm;
        G4double SensorDistance = activeParams.getLength("SensorDistance") * CLHEP::cm;
        G4double SensorLengthX = (activeParams.getInt("nPixelsX") * activeParams.getLength("pitchX")) * CLHEP::cm;
        G4double SensorLengthY = (activeParams.getInt("nPixelsY") * activeParams.getLength("pitchY")) * CLHEP::cm;

        G4double AirGap = activeParams.getLength("AirGap") * CLHEP::cm;

        //IRB:
        G4double SubstrateThickness = activeParams.getLength("SubstrateThickness") * CLHEP::cm;
        G4double EpitaxialThickness = activeParams.getLength("EpitaxialThickness") * CLHEP::cm;
        G4double MetalThickness = activeParams.getLength("MetalThickness") * CLHEP::cm;
        G4double GlueThickness = activeParams.getLength("GlueThickness") * CLHEP::cm;
        G4double KaptonThickness = activeParams.getLength("KaptonThickness") * CLHEP::cm;
        G4double AluminiumThickness = activeParams.getLength("AluminiumThickness") * CLHEP::cm;

        //create air volume that contains all layers
        G4double dx_foam = SensorLengthY / 2. + ElecBandY / 2.;
        G4double dy_foam = (SensorLengthX * 6. + 5.*SensorDistance) / 2.;
        G4double DistanceFromFoamCenter = activeParams.getLength("DistanceFromFoamCenter") * CLHEP::cm;
        //Envelop dimension Thickness not divided by 2 to take into account of the 2 sides of foam
        G4double dz_env = DistanceFromFoamCenter + KaptonThickness + GlueThickness +
                          AluminiumThickness + SubstrateThickness + EpitaxialThickness +
                          MetalThickness + 6. * AirGap;

        G4Box* s_env = new G4Box("s_env", dx_foam, dy_foam, dz_env);
        G4LogicalVolume* l_env = new G4LogicalVolume(s_env, geometry::Materials::get("G4_AIR"), "l_env");
        G4Transform3D transform = G4RotateZ3D(phi - M_PI / 2.0) * G4Translate3D(0, r, z) * G4RotateX3D(- M_PI / 2.0 - thetaZ);
        new G4PVPlacement(transform, l_env, "p_env", &topVolume, false, 1);

        //create foam layer
        G4Box* s_foam = new G4Box("Foam", dx_foam, dy_foam, DistanceFromFoamCenter);
        G4LogicalVolume* l_foam = new G4LogicalVolume(s_foam, geometry::Materials::get("SiC"), "l_foam");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), l_foam, "p_foam", l_env, false, 1);

        //create glue layers
        G4Box* s_glue = new G4Box("s_glue", dx_foam, dy_foam, GlueThickness / 2.);
        G4LogicalVolume* l_glue = new G4LogicalVolume(s_glue, geometry::Materials::get("Glue"), "s_glue");
        G4double r_glue = DistanceFromFoamCenter + AirGap + GlueThickness / 2.;
        new G4PVPlacement(0, G4ThreeVector(0, 0, -r_glue), l_glue, "p_glue_0", l_env, false, 1);
        new G4PVPlacement(0, G4ThreeVector(0, 0, r_glue), l_glue, "p_glue_1", l_env, false, 1);

        //create Kapton layers
        G4Box* s_Kapton = new G4Box("s_Kapton", dx_foam, dy_foam, KaptonThickness / 2.);
        G4LogicalVolume* l_Kapton = new G4LogicalVolume(s_Kapton, geometry::Materials::get("Kapton") , "l_Kapton");
        G4double r_Kapton = r_glue + AirGap + (GlueThickness + KaptonThickness) / 2.;
        new G4PVPlacement(0, G4ThreeVector(0, 0, -r_Kapton), l_Kapton, "p_Kapton_0", l_env, false, 1);
        new G4PVPlacement(0, G4ThreeVector(0, 0, r_Kapton), l_Kapton, "p_Kapton_1", l_env, false, 1);

        //create metal layers
        G4Box* s_metal = new G4Box("s_metal", dx_foam, dy_foam, AluminiumThickness / 2.);
        G4LogicalVolume* l_metal = new G4LogicalVolume(s_metal, geometry::Materials::get("Al"), "l_metal");
        G4double r_metal = r_Kapton + AirGap + (KaptonThickness + AluminiumThickness) / 2.;
        new G4PVPlacement(0, G4ThreeVector(0, 0, -r_metal), l_metal, "p_metal_0", l_env, false, 1);
        new G4PVPlacement(0, G4ThreeVector(0, 0, r_metal), l_metal, "p_metal_1", l_env, false, 1);

        l_foam->SetVisAttributes(FoamVisAtt);
        l_Kapton->SetVisAttributes(KaptonVisAtt);
        l_glue->SetVisAttributes(GlueVisAtt);
        l_metal->SetVisAttributes(MetalVisAtt);

        //create Mimosa Sensors, 12 per ladder, made of 3 layers
        G4double x_array[] = { -2.5, -1.5, -0.5, 0.5, 1.5, 2.5};
        G4double r_substrate = r_metal + AirGap + (AluminiumThickness + SubstrateThickness) / 2.;
        G4double r_epitaxial = r_substrate + AirGap + (SubstrateThickness + EpitaxialThickness) / 2.;
        G4double r_metalized = r_epitaxial + AirGap + (EpitaxialThickness + MetalThickness) / 2.;

        //----------------------------------------------------------------------------//
        //                            Substrate Layer                                 //
        //----------------------------------------------------------------------------//
        G4Box* s_substrate = new G4Box("s_substrate", SensorLengthY / 2. , SensorLengthX / 2. , SubstrateThickness / 2.);
        G4LogicalVolume* l_substrate = new G4LogicalVolume(s_substrate, geometry::Materials::get("Silicon"), "l_substrate");
        //----------------------------------------------------------------------------//
        //                            Epitaxial Layer                                 //
        //----------------------------------------------------------------------------//
        G4Box* s_epitaxial = new G4Box("s_epitaxial", SensorLengthY / 2. , SensorLengthX / 2., EpitaxialThickness / 2.);
        G4LogicalVolume* l_epitaxial = new G4LogicalVolume(s_epitaxial, geometry::Materials::get("Silicon"), "l_epitaxial", 0, m_sensitive);
        //----------------------------------------------------------------------------//
        //                            Metalized Layer                                 //
        //----------------------------------------------------------------------------//
        G4Box* s_metalized = new G4Box("s_metalized", SensorLengthY / 2. , SensorLengthX / 2. , MetalThickness / 2.);
        G4LogicalVolume* l_metalized = new G4LogicalVolume(s_metalized, geometry::Materials::get("SiO2Al"), "l_metalized");

        l_epitaxial->SetUserLimits(new G4UserLimits(stepSize));
        l_substrate->SetVisAttributes(SubstrateVisAtt);
        l_epitaxial->SetVisAttributes(EpitaxialVisAtt);
        l_metalized->SetVisAttributes(MetalizedVisAtt);

        for (int i = 0; i < 12; i++) {
          //----------------------------------------------------------------------------//
          //                            Substrate Layer                                 //
          //----------------------------------------------------------------------------//
          G4String name = "";
          G4ThreeVector MSpos = G4ThreeVector(0, 0, 0);
          if (i < 6) {
            name = "p_substrate_bot";
            MSpos = G4ThreeVector(0, -(SensorLengthX + SensorDistance) * x_array[i], -r_substrate);
          } else {
            name = "p_substrate_top";
            MSpos = G4ThreeVector(0, -(SensorLengthX + SensorDistance) * x_array[i - 6], r_substrate);
          }
          new G4PVPlacement(0, MSpos, l_substrate, name,  l_env, false, 1);
          //----------------------------------------------------------------------------//
          //                            Epitaxial Layer                                 //
          //----------------------------------------------------------------------------//
          if (i < 6) {
            name = "p_epitaxial_bot";
            MSpos = G4ThreeVector(0, -(SensorLengthX + SensorDistance) * x_array[i], -r_epitaxial);
          } else {
            name = "p_epitaxial_top";
            MSpos = G4ThreeVector(0, -(SensorLengthX + SensorDistance) * x_array[i - 6], r_epitaxial);
          }
          new G4PVPlacement(0, MSpos, l_epitaxial, name,  l_env, false, LadderID * 12 + i);
          //----------------------------------------------------------------------------//
          //                            Metalized Layer                                 //
          //----------------------------------------------------------------------------//
          if (i < 6) {
            name = "p_metalized_bot";
            MSpos = G4ThreeVector(0, -(SensorLengthX + SensorDistance) * x_array[i], -r_metalized);
          } else {
            name = "p_metalized_top";
            MSpos = G4ThreeVector(0, -(SensorLengthX + SensorDistance) * x_array[i - 6], r_metalized);
          }
          new G4PVPlacement(0, MSpos, l_metalized, name,  l_env, false, 1);
        }
        LadderID++;
      }
    }
  } // plume namespace
} // Belle2 namespace
