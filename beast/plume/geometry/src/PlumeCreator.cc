/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
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

        G4double ElecBandY = activeParams.getLength("ElecBandY");
        G4double SensorDistance = activeParams.getLength("SensorDistance") * CLHEP::cm;
        G4double SensorLengthX = activeParams.getInt("nPixelsX") * activeParams.getLength("pitchX") * CLHEP::cm;
        G4double SensorLengthY = activeParams.getInt("nPixelsY") * activeParams.getLength("pitchY") * CLHEP::cm;

        G4double SubstrateThickness = activeParams.getLength("SubstrateThickness") * CLHEP::cm;
        G4double MetalizedThickness = activeParams.getLength("MetalThickness") * CLHEP::cm;
        G4double EpitaxialThickness = activeParams.getLength("EpitaxialThickness") * CLHEP::cm;

        //create foam layer
        G4double dx_foam = SensorLengthY / 2. + ElecBandY / 2.;
        G4double dy_foam = (SensorLengthX * 6. + 5.*SensorDistance) / 2.;
        G4double DistanceFromFoamCenter = activeParams.getLength("DistanceFromFoamCenter") * CLHEP::cm;
        G4double GlueThickness = activeParams.getLength("GlueThickness") / 2.*CLHEP::cm;
        G4double KaptonThickness = activeParams.getLength("KaptonThickness") / 2.*CLHEP::cm;
        G4double CupperThickness = activeParams.getLength("CupperThickness") / 2.*CLHEP::cm;

        G4Box* s_foam = new G4Box("Foam", dx_foam, dy_foam, DistanceFromFoamCenter);
        G4LogicalVolume* l_foam = new G4LogicalVolume(s_foam, geometry::Materials::get("SiC"), "l_foam");
        G4Transform3D transform = G4RotateZ3D(phi) * G4Translate3D(0, r, z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_foam, "p_foam", &topVolume, false, 0);

        //create glue layers
        G4Box* s_glue = new G4Box("s_glue", dx_foam, dy_foam, GlueThickness);
        G4LogicalVolume* l_glue = new G4LogicalVolume(s_glue, geometry::Materials::get("Glue"), "s_glue");
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r - (DistanceFromFoamCenter + 1e-5 * CLHEP::cm + GlueThickness), z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_glue, "p_glue_0", &topVolume, false, 0);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r + (DistanceFromFoamCenter + 1e-5 * CLHEP::cm + GlueThickness), z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_glue, "p_glue_1", &topVolume, false, 0);

        //create Kapton layers
        G4Box* s_Kapton = new G4Box("s_Kapton", dx_foam, dy_foam, KaptonThickness);
        G4LogicalVolume* l_Kapton = new G4LogicalVolume(s_Kapton, geometry::Materials::get("Kapton") , "l_Kapton");
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r - (DistanceFromFoamCenter + 2e-5 * CLHEP::cm + 2.*GlueThickness + KaptonThickness), z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_Kapton, "p_Kapton_0", &topVolume, false, 0);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r + (DistanceFromFoamCenter + 2e-5 * CLHEP::cm + 2.*GlueThickness + KaptonThickness), z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_Kapton, "p_Kapton_1", &topVolume, false, 0);

        //create metal layers
        G4Box* s_metal = new G4Box("s_metal", dx_foam, dy_foam, CupperThickness);
        G4LogicalVolume* l_metal = new G4LogicalVolume(s_metal, geometry::Materials::get("Cupper"), "l_metal");
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r - (DistanceFromFoamCenter + 3e-5 * CLHEP::cm + 2.*GlueThickness + 2.*KaptonThickness + CupperThickness), z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_metal, "p_metal_0", &topVolume, false, 0);
        transform = G4RotateZ3D(phi) * G4Translate3D(0, r + (DistanceFromFoamCenter + 3e-5 * CLHEP::cm + 2.*GlueThickness + 2.*KaptonThickness + CupperThickness), z) * G4RotateX3D(-M_PI / 2 - thetaZ);
        new G4PVPlacement(transform, l_metal, "p_metal_1", &topVolume, false, 0);

        l_foam->SetVisAttributes(FoamVisAtt);
        l_Kapton->SetVisAttributes(KaptonVisAtt);
        l_glue->SetVisAttributes(GlueVisAtt);
        l_metal->SetVisAttributes(MetalVisAtt);

        //create Mimosa Sensors, 12 per ladder, made of 3 layers
        G4double x_array[] = { -2.5, -1.5, -0.5, 0.5, 1.5, 2.5};
        G4double DistanceFromFoam = DistanceFromFoamCenter + KaptonThickness * 2. + GlueThickness * 2. + CupperThickness * 2. + 3.6e-3 * CLHEP::cm + 4.5e-4 * CLHEP::cm + 5e-5 * CLHEP::cm;
        //epi -> G4double Z1 = -distanceFromFoam; // =  1000 + sensorWidth/2 (+2.0um : no overlaping)
        //metal -> Z1 -= fMetalizedLengthZ/2. + fEpitaxialLengthZ/2. + 0.1*um;
        //substrate -> Z1 += fMetalizedLengthZ/2. + fEpitaxialLengthZ + 0.2*um + fSubstrateLengthZ/2.;
        //epi -> G4double Z7 = distanceFromFoam;
        //metal -> Z7 += fMetalizedLengthZ/2. + fEpitaxialLengthZ/2. + 0.1*um;
        //substrate -> Z7 -= fMetalizedLengthZ/2. + fEpitaxialLengthZ + 0.2*um + fSubstrateLengthZ/2.;
        G4double r_epitaxial1 = - DistanceFromFoam;
        G4double r_metalized1 = r_epitaxial1 - (MetalizedThickness / 2. + EpitaxialThickness / 2. + 1e-5 * CLHEP::cm);
        G4double r_substrate1 = r_metalized1 + MetalizedThickness / 2. + EpitaxialThickness + 2e-5 * CLHEP::cm + SubstrateThickness / 2;
        G4double r_epitaxial2 = DistanceFromFoam;
        G4double r_metalized2 = r_epitaxial2 + (MetalizedThickness / 2. + EpitaxialThickness / 2. + 1e-5 * CLHEP::cm);
        G4double r_substrate2 = r_metalized2 - MetalizedThickness / 2. + EpitaxialThickness + 2e-5 * CLHEP::cm + SubstrateThickness / 2;
        // before 1024.5um = dist min from foam + 5um = security distance for no overlaping.
        for (int i = 0; i < 12; i++) {
          //----------------------------------------------------------------------------//
          //                            Substrate Layer                                 //
          //----------------------------------------------------------------------------//

          G4Box* s_substrate = new G4Box("s_substrate", SensorLengthY / 2. , SensorLengthX / 2. , SubstrateThickness / 2.);
          G4LogicalVolume* l_substrate = new G4LogicalVolume(s_substrate, geometry::Materials::get("Silicon"), "l_substrate");
          if (i < 6) transform = G4RotateZ3D(phi) * G4Translate3D(0, r + r_substrate1, z - x_array[i] * (SensorLengthX + SensorDistance)) * G4RotateX3D(-M_PI / 2 - thetaZ);
          else transform = G4RotateZ3D(phi) * G4Translate3D(0, r + r_substrate2, z - x_array[i - 6] * (SensorLengthX + SensorDistance)) * G4RotateX3D(-M_PI / 2 - thetaZ);
          new G4PVPlacement(transform, l_substrate, "p_substrate",  &topVolume, false, 0);

          //----------------------------------------------------------------------------//
          //                            Metalized Layer                                 //
          //----------------------------------------------------------------------------//

          G4Box* s_metalized = new G4Box("s_metalized", SensorLengthY / 2. , SensorLengthX / 2. , MetalizedThickness / 2.);
          G4LogicalVolume* l_metalized = new G4LogicalVolume(s_metalized, geometry::Materials::get("Quartz"), "l_metalized");
          if (i < 6) transform = G4RotateZ3D(phi) * G4Translate3D(0, r + r_metalized1, z - x_array[i] * (SensorLengthX + SensorDistance)) * G4RotateX3D(-M_PI / 2 - thetaZ);
          else transform = G4RotateZ3D(phi) * G4Translate3D(0, r + r_metalized2, z - x_array[i - 6] * (SensorLengthX + SensorDistance)) * G4RotateX3D(-M_PI / 2 - thetaZ);
          new G4PVPlacement(transform, l_metalized, "p_metalized", &topVolume, false, 0);

          //----------------------------------------------------------------------------//
          //                            Epitaxial Layer                                 //
          //----------------------------------------------------------------------------//

          G4Box* s_epitaxial = new G4Box("s_epitaxial", SensorLengthY / 2. , SensorLengthX / 2., EpitaxialThickness / 2.);
          G4LogicalVolume* l_epitaxial = new G4LogicalVolume(s_epitaxial, geometry::Materials::get("Silicon"), "l_epitaxial", 0, m_sensitive);
          l_epitaxial->SetUserLimits(new G4UserLimits(stepSize));
          if (i < 6) transform = G4RotateZ3D(phi) * G4Translate3D(0, r + r_epitaxial1, z - x_array[i] * (SensorLengthX + SensorDistance)) * G4RotateX3D(-M_PI / 2 - thetaZ);
          else transform = G4RotateZ3D(phi) * G4Translate3D(0, r + r_epitaxial2, z - x_array[i - 6] * (SensorLengthX + SensorDistance)) * G4RotateX3D(-M_PI / 2 - thetaZ);
          new G4PVPlacement(transform, l_epitaxial, "p_epitaxial", &topVolume, false, LadderID * 12 + i);

          l_substrate->SetVisAttributes(SubstrateVisAtt);
          l_epitaxial->SetVisAttributes(EpitaxialVisAtt);
          l_metalized->SetVisAttributes(MetalizedVisAtt);

        }
        LadderID++;
      }
    }
  } // plume namespace
} // Belle2 namespace
