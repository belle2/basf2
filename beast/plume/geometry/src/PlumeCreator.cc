/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj,                                            *
 *               Isabelle Ripp-Baudot                                     *
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
#include <G4AssemblyVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4UnitsTable.hh>
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Trd.hh>
#include <G4Polycone.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"
#include <G4TwoVector.hh>
//Visualization Attributes
#include <G4VisAttributes.hh>
#include<G4ExtrudedSolid.hh>

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

      int LadderID = 0;

      GearDir activeParams(content, "Ladder");
      G4double SensorLengthX = (activeParams.getInt("nPixelsX") * activeParams.getLength("pitchX")) * CLHEP::cm;
      G4double SensorLengthY = (activeParams.getInt("nPixelsY") * activeParams.getLength("pitchY")) * CLHEP::cm;

      G4double AirGap = activeParams.getLength("AirGap") * CLHEP::cm;

      G4double SubstrateThickness = activeParams.getLength("SubstrateThickness") * CLHEP::cm;
      G4double EpitaxialThickness = activeParams.getLength("EpitaxialThickness") * CLHEP::cm;
      G4double MetalThickness = activeParams.getLength("MetalThickness") * CLHEP::cm;
      G4double GlueThickness = activeParams.getLength("GlueThickness") * CLHEP::cm;
      G4double KaptonThickness = activeParams.getLength("KaptonThickness") * CLHEP::cm;
      G4double AluminiumThickness = activeParams.getLength("AluminiumThickness") * CLHEP::cm;
      G4double foamlen = activeParams.getLength("Foam/length") * CLHEP::cm;
      G4double foamwid = activeParams.getLength("Foam/width") * CLHEP::cm;
      G4double foamthick = activeParams.getLength("Foam/thickness") * CLHEP::cm;
      G4double foamZ = activeParams.getLength("Foam/zPosition") * CLHEP::cm;
      G4double foamY = activeParams.getLength("Foam/yShift") * CLHEP::cm;
      G4double flexlen = activeParams.getLength("Flex/length") * CLHEP::cm;
      G4double flexwid = activeParams.getLength("Flex/width") * CLHEP::cm;
      G4double flexZ = activeParams.getLength("Flex/zPosition") * CLHEP::cm;
      G4double allen1 = activeParams.getLength("AlBat/length1") * CLHEP::cm;
      G4double allen2 = activeParams.getLength("AlBat/length2") * CLHEP::cm;
      G4double allen3 = activeParams.getLength("AlBat/length3") * CLHEP::cm;
      G4double alwid = activeParams.getLength("AlBat/width") * CLHEP::cm;
      G4double althick1 = activeParams.getLength("AlBat/thickness1") * CLHEP::cm;
      G4double althick2 = activeParams.getLength("AlBat/thickness2") * CLHEP::cm;
      G4double althick3 = activeParams.getLength("AlBat/thickness3") * CLHEP::cm;

      // old geometry
      /*
      G4double SensorDistance = activeParams.getLength("SensorDistance") * CLHEP::cm;
      G4double dx_foam = SensorLengthY / 2. + 0.15 * CLHEP::cm ;
      G4double dy_foam = (SensorLengthX * 6. + 5.*SensorDistance) / 2.;
      G4double DistanceFromFoamCenter = 0.1 * CLHEP::cm;
      //Envelop dimension Thickness not divided by 2 to take into account of the 2 sides of foam
      G4double dz_env = DistanceFromFoamCenter + KaptonThickness + GlueThickness +
      AluminiumThickness + SubstrateThickness + EpitaxialThickness +
      MetalThickness + 6. * AirGap;
      G4Box* s_env = new G4Box("s_env", dx_foam, dy_foam, dz_env);
      G4LogicalVolume* l_env = new G4LogicalVolume(s_env, geometry::Materials::get("Al"), "l_env");
      G4Transform3D transformt = G4RotateZ3D(135. / 180 * M_PI - M_PI / 2.0) * G4Translate3D(0, 61,
      42.2) * G4RotateX3D(- M_PI / 2.0 - 178. / 180 * M_PI);
      new G4PVPlacement(transformt, l_env, "p_env1", &topVolume, false, 1);
      transformt = G4RotateZ3D(225. / 180 * M_PI - M_PI / 2.0) * G4Translate3D(0, 84,
      -21.8) * G4RotateX3D(- M_PI / 2.0 - 20. / 180 * M_PI);
      new G4PVPlacement(transformt, l_env, "p_env2", &topVolume, false, 1);*/
      //---------------

      G4double dz_ladder = foamthick / 2. + KaptonThickness + GlueThickness +
                           AluminiumThickness + 3. * AirGap;
      G4double dz_sensor = (SubstrateThickness + EpitaxialThickness +
                            MetalThickness + 2. * AirGap) / 2.;

      G4double fullLength = allen1 + allen2 + allen3 - (foamZ - foamlen / 2.);
      double zshift = fullLength / 2. - (allen1 + allen2 + allen3);

      // Create support ladder
      G4AssemblyVolume* support = new G4AssemblyVolume();
      G4Transform3D transl;;

      //create foam layer
      G4Box* s_foam = new G4Box("s_foam", foamlen / 2., foamwid / 2., foamthick / 2.);
      G4LogicalVolume* l_foam = new G4LogicalVolume(s_foam, geometry::Materials::get("SiC"), "PLUME.l_foam");
      transl = G4Translate3D(foamZ + zshift, foamY, 0);
      support->AddPlacedVolume(l_foam, transl);

      //create glue layers
      G4Box* s_glue = new G4Box("s_glue", flexlen / 2., foamwid / 2. , GlueThickness / 2.);
      G4LogicalVolume* l_glue = new G4LogicalVolume(s_glue, geometry::Materials::get("Glue"), "PLUME.l_glue");
      G4double r_glue = foamthick / 2. + AirGap + GlueThickness / 2.;
      transl = G4Translate3D(flexZ + zshift, foamY, -r_glue);
      support->AddPlacedVolume(l_glue, transl);
      transl = G4Translate3D(flexZ + zshift, foamY, r_glue);
      support->AddPlacedVolume(l_glue, transl);

      //create Kapton layers
      G4Box* s_kapton = new G4Box("s_kapton", flexlen / 2., flexwid / 2., KaptonThickness / 2.);
      G4LogicalVolume* l_kapton = new G4LogicalVolume(s_kapton, geometry::Materials::get("Kapton") , "PLUME.l_kapton");
      G4double r_Kapton = r_glue + AirGap + (GlueThickness + KaptonThickness) / 2.;
      transl = G4Translate3D(flexZ + zshift, 0, -r_Kapton);
      support->AddPlacedVolume(l_kapton, transl);
      transl = G4Translate3D(flexZ + zshift, 0, r_Kapton);
      support->AddPlacedVolume(l_kapton, transl);

      //create metal layers
      G4Box* s_metal = new G4Box("s_metal", flexlen / 2., flexwid / 2., AluminiumThickness / 2.);
      G4LogicalVolume* l_metal = new G4LogicalVolume(s_metal, geometry::Materials::get("Al"), "PLUME.l_metal");
      G4double r_metal = r_Kapton + AirGap + (KaptonThickness + AluminiumThickness) / 2.;
      transl = G4Translate3D(flexZ + zshift, 0, -r_metal);
      support->AddPlacedVolume(l_metal, transl);
      transl = G4Translate3D(flexZ + zshift, 0, r_metal);
      support->AddPlacedVolume(l_metal, transl);

      //create aluminum bat
      G4Box* s_bat1p = new G4Box("s_bat1p", allen1 / 2., alwid / 2., althick1 / 2.);
      double incut = foamZ + foamlen / 2.;
      G4Box* s_bat1t = new G4Box("s_bat1t", incut, foamwid / 2., althick1 + 0.2 / 2.);

      G4Transform3D tt = G4Translate3D(-allen1 / 2., foamY, 0);
      G4SubtractionSolid* s_bat1 = new G4SubtractionSolid("s_bat1", s_bat1p, s_bat1t, tt);
      G4Box* s_bat2 = new G4Box("s_bat2", allen2 / 2., alwid / 2., althick2 / 2.);
      G4Box* s_bat3 = new G4Box("s_bat3", allen3 / 2., alwid / 2., althick3 / 2.);
      G4LogicalVolume* l_bat1 = new G4LogicalVolume(s_bat1, geometry::Materials::get("Al"), "PLUME.l_bat1");
      G4LogicalVolume* l_bat2 = new G4LogicalVolume(s_bat2, geometry::Materials::get("Al"), "PLUME.l_bat2");
      G4LogicalVolume* l_bat3 = new G4LogicalVolume(s_bat3, geometry::Materials::get("Al"), "PLUME.l_bat3");
      transl = G4Translate3D(allen1 / 2. + zshift, 0, 0);
      support->AddPlacedVolume(l_bat1, transl);
      transl = G4Translate3D(allen1 + allen2 / 2. + zshift, 0, 0);
      support->AddPlacedVolume(l_bat2, transl);
      transl = G4Translate3D(allen1 + allen2 + allen3 / 2. + zshift, 0, 0);
      support->AddPlacedVolume(l_bat3, transl);

      l_foam->SetVisAttributes(FoamVisAtt);
      l_kapton->SetVisAttributes(KaptonVisAtt);
      l_glue->SetVisAttributes(GlueVisAtt);
      l_metal->SetVisAttributes(MetalVisAtt);


      G4String symbol, name;
      G4double a, zz;
      G4double density;
      G4int ncomponents;
      G4double fractionmass;
      a = 1.01 * CLHEP::g / CLHEP::mole;
      G4Element* elH  = new G4Element(name = "Hydrogen", symbol = "H" , zz = 1., a);
      a = 12.01 * CLHEP::g / CLHEP::mole;
      G4Element* elC  = new G4Element(name = "Carbon"  , symbol = "C" , zz = 6., a);
      a = 16.00 * CLHEP::g / CLHEP::mole;
      G4Element* elO  = new G4Element(name = "Oxygen"  , symbol = "O" , zz = 8., a);
      density = 1.31 * CLHEP::g / CLHEP::cm3;
      G4Material* peekMat = new G4Material(name = "plumePeek"  , density, ncomponents = 3);
      peekMat->AddElement(elC, fractionmass = 0.76);
      peekMat->AddElement(elH, fractionmass = 0.08);
      peekMat->AddElement(elO, fractionmass = 0.16);
      density = 1.5 * CLHEP::g / CLHEP::cm3;
      G4Material* carbMat = new G4Material(name = "plumeCarb"  , density, ncomponents = 1);
      carbMat->AddElement(elC, fractionmass = 1.0);

      G4Box* s_sensor = new G4Box("s_sensor", SensorLengthX / 2., SensorLengthY / 2., dz_sensor);
      G4LogicalVolume* l_sensor = new G4LogicalVolume(s_sensor, geometry::Materials::get("G4_AIR"), "PLUME.l_sensor");

      //                            Substrate Layer                                 //
      G4Box* s_substrate = new G4Box("s_substrate", SensorLengthX / 2. , SensorLengthY / 2. , SubstrateThickness / 2.);
      G4LogicalVolume* l_substrate = new G4LogicalVolume(s_substrate, geometry::Materials::get("Silicon"), "PLUME.l_substrate");
      new G4PVPlacement(0, G4ThreeVector(0, 0, -dz_sensor + SubstrateThickness / 2.), l_substrate, "p_substrate", l_sensor, false, 1);

      //                            Epitaxial Layer                                 //
      G4Box* s_epitaxial = new G4Box("s_epitaxial", SensorLengthX / 2. , SensorLengthY / 2., EpitaxialThickness / 2.);
      G4LogicalVolume* l_epitaxial = new G4LogicalVolume(s_epitaxial, geometry::Materials::get("Silicon"), "PLUME.l_epitaxial", 0,
                                                         m_sensitive);
      new G4PVPlacement(0, G4ThreeVector(0, 0, -dz_sensor + AirGap + SubstrateThickness + EpitaxialThickness / 2.), l_epitaxial,
                        "p_epitaxial", l_sensor, false, 1);
      //                            Metalized Layer                                 //
      G4Box* s_metalized = new G4Box("s_metalized", SensorLengthX / 2. , SensorLengthY / 2. , MetalThickness / 2.);
      G4LogicalVolume* l_metalized = new G4LogicalVolume(s_metalized, geometry::Materials::get("SiO2Al"), "PLUME.l_metalized");
      new G4PVPlacement(0, G4ThreeVector(0, 0, -dz_sensor + 2 * AirGap + SubstrateThickness + EpitaxialThickness + MetalThickness / 2.),
                        l_metalized, "p_metalized", l_sensor, false, 1);

      // temporary
      /*G4LogicalVolume* l_sensorMir = new G4LogicalVolume(s_sensor, geometry::Materials::get("G4_AIR"), "l_sensorMir");
      new G4PVPlacement(0, G4ThreeVector(0, 0, -dz_sensor + MetalThickness / 2.), l_metalized, "p_metalized", l_sensorMir, false, 1);
      new G4PVPlacement(0, G4ThreeVector(0, 0, -dz_sensor + MetalThickness + AirGap + EpitaxialThickness / 2.), l_epitaxial,
      "p_epitaxial", l_sensorMir, false, 1);
      new G4PVPlacement(0, G4ThreeVector(0, 0, -dz_sensor + MetalThickness + 2 * AirGap + EpitaxialThickness + SubstrateThickness / 2.),
      l_substrate, "p_substrate", l_sensorMir, false, 1);
      */


      l_epitaxial->SetUserLimits(new G4UserLimits(stepSize));
      l_substrate->SetVisAttributes(SubstrateVisAtt);
      l_epitaxial->SetVisAttributes(EpitaxialVisAtt);
      l_metalized->SetVisAttributes(MetalizedVisAtt);

      // place 12 sensors on support ladder
      double zSens = dz_ladder + dz_sensor + AirGap;
      G4RotationMatrix ra;
      G4ThreeVector Ta(0, 0, 0);
      G4Transform3D tra(ra, Ta);

      // build cooling tubes
      GearDir pipeParams(content, "CoolingPipes");
      double pipeLen = pipeParams.getLength("length") * CLHEP::cm;
      double pipeInR = pipeParams.getLength("rIn") * CLHEP::cm;
      double pipeOutR = pipeParams.getLength("rOut") * CLHEP::cm;
      G4Tubs* pipe_s = new G4Tubs("pipe_s", pipeInR, pipeOutR, pipeLen / 2., 0, 2 * M_PI);

      G4LogicalVolume* pipe_l = new G4LogicalVolume(pipe_s, carbMat, "PLUME.l_pipe");
      double pipeX = pipeParams.getLength("x") * CLHEP::cm;
      double pipeY = pipeParams.getLength("y") * CLHEP::cm;
      double pipeZ = pipeParams.getLength("z") * CLHEP::cm;
      G4double alphaPipe = pipeParams.getAngle("alpha");

      GearDir alignPars;
      G4Box* s_sensors = new G4Box("s_sensors", flexlen / 2., flexwid / 2., dz_sensor + AirGap);

      for (auto ladder : content.getNodes("Placements/Ladder")) {

        std::string id = ladder.getString("@id");

        G4AssemblyVolume* assemblyLadder = new G4AssemblyVolume();
        assemblyLadder->AddPlacedAssembly(support, tra);

        for (auto pars : content.getNodes("SensorAlignment/ladder")) {
          if (pars.getString("@id") == id) alignPars = pars;
        }

        for (auto sidePars : alignPars.getNodes("side")) {
          unsigned mirror = 0;

          G4Transform3D transformSens;
          if (sidePars.getString("@id") == "mirror") mirror = 1;

          G4LogicalVolume* l_sensors = new G4LogicalVolume(s_sensors, geometry::Materials::get("G4_AIR"), "PLUME.l_sensors");

          for (auto sensorPars : sidePars.getNodes("sensor")) {
            double x = sensorPars.getLength("x") * CLHEP::cm + zshift;
            double y = sensorPars.getLength("y") * CLHEP::cm;
            int ids = sensorPars.getInt("id");
            y = mirror ? -y : y;
            y -= alwid / 2.;
            double alpha1 = sensorPars.getAngle("alpha") * CLHEP::rad;
            if (mirror) transformSens =  G4Translate3D(x, y,
                                                         0) * G4RotateZ3D(alpha1) * G4RotateX3D(M_PI); // think if this is correct, rotation and pos shift for mirror side
            else  transformSens =  G4Translate3D(x, y, 0) * G4RotateZ3D(-alpha1);

            new G4PVPlacement(transformSens, l_sensor, "p_sensor", l_sensors, true,
                              (ids + mirror * 6 + LadderID * 12));
          }
          transformSens = G4Translate3D(0, 0, mirror ? -zSens : zSens);
          assemblyLadder->AddPlacedVolume(l_sensors, transformSens);
        }

        G4double thetaZ = ladder.getAngle("ThetaZ");
        G4double r = ladder.getLength("r_plume") * CLHEP::cm - zshift * sin(thetaZ);
        G4double z = ladder.getLength("z_plume") * CLHEP::cm - zshift * cos(thetaZ);
        G4double phi = ladder.getAngle("Phi");
        G4double alpha = ladder.getAngle("Alpha");


        // place cooling pipes
        G4Transform3D transformPipe =  G4Translate3D(pipeZ + zshift, pipeY, pipeX) * G4RotateZ3D(alphaPipe) * G4RotateY3D(+ M_PI / 2.);
        assemblyLadder->AddPlacedVolume(pipe_l, transformPipe);
        transformPipe =  G4Translate3D(pipeZ + zshift
                                       , pipeY, -pipeX) * G4RotateZ3D(alphaPipe) * G4RotateY3D(+ M_PI / 2.);
        assemblyLadder->AddPlacedVolume(pipe_l, transformPipe);


        G4Transform3D transform1;
        if (LadderID == 1) {
          G4AssemblyVolume* sup1 =  buildSupport1();
          G4AssemblyVolume* sup2 = buildSupport2();
          transform1 = G4Translate3D(-fullLength / 2. - 18.7, 0., 1.2) * G4RotateY3D(thetaZ) * G4RotateX3D(M_PI / 2.) * G4RotateY3D(M_PI);
          assemblyLadder->AddPlacedAssembly(sup1, transform1);
          transform1 =  G4Translate3D(fullLength / 2. + 19.5, 0, -2.6) * G4RotateY3D(thetaZ) * G4RotateX3D(M_PI / 2.);
          assemblyLadder->AddPlacedAssembly(sup2, transform1);
        }

        if (LadderID == 0) {
          G4AssemblyVolume* sup1 =  buildSupport3();
          G4AssemblyVolume* sup2 = buildSupport4();
          transform1 = G4Translate3D(fullLength / 2. + 5, 0,
                                     7.8) * G4RotateY3D(-thetaZ) *  G4RotateZ3D(M_PI / 2.) * G4RotateX3D(M_PI / 2.) * G4RotateY3D(M_PI / 2.);
          assemblyLadder->AddPlacedAssembly(sup1, transform1);
          transform1 = G4Translate3D(-fullLength / 2. - 31, 0.,
                                     -21.7) * G4RotateY3D(-thetaZ) * G4RotateZ3D(M_PI / 2.) * G4RotateX3D(-M_PI / 2.) * G4RotateY3D(M_PI);
          assemblyLadder->AddPlacedAssembly(sup2, transform1);
        }

        G4Transform3D transform = G4RotateZ3D(phi) * G4Translate3D(r, 0,
                                                                   z) * G4RotateY3D(thetaZ) * G4RotateZ3D(alpha + M_PI) * G4RotateY3D(- M_PI / 2.0);

        assemblyLadder->MakeImprint(&topVolume, transform);

        LadderID += 1;

      }

    }


    G4AssemblyVolume* PlumeCreator::buildSupport1()
    {

      std::vector<G4TwoVector> points = {G4TwoVector(-11.6, 0.371), G4TwoVector(-11.504, -2.378), G4TwoVector(-1.51, -2.029), G4TwoVector(-1.51, -4.13), G4TwoVector(-11.561, -4.481), G4TwoVector(-11.686, -6.629), G4TwoVector(2.584, -6.629), G4TwoVector(4.974, -4.239), G4TwoVector(4.974, 1.761), G4TwoVector(2.0, 1.761), G4TwoVector(0.61, 0.371)};

      G4Material* peek = G4Material::GetMaterial("plumePeek");
      G4ExtrudedSolid* s1_peek0 = new G4ExtrudedSolid("s1_peek0", points, 10.0, G4TwoVector(0, 0), 1, G4TwoVector(0, 0), 1);
      G4LogicalVolume* l1_peek0 = new G4LogicalVolume(s1_peek0, peek, "PLUME.l1_peek0");

      G4Box* s1_peek1 = new G4Box("s1_peek1", 24. / 2., 6. / 2. , 20. / 2.);
      G4Box* s1_peek2 = new G4Box("s1_peek2", 19.4 / 2., 3. / 2. , 20. / 2.);
      G4LogicalVolume* l1_peek1 = new G4LogicalVolume(s1_peek1, peek, "PLUME.l1_peek1");
      G4LogicalVolume* l1_peek2 = new G4LogicalVolume(s1_peek2, peek, "PLUME.l1_peek2");

      G4AssemblyVolume* assemblySup = new G4AssemblyVolume();

      G4Transform3D transform = G4Translate3D(0, 0., 0);

      assemblySup->AddPlacedVolume(l1_peek1, transform);

      transform = G4Translate3D(-16.974, 1.239, 0);
      assemblySup->AddPlacedVolume(l1_peek0, transform);

      transform = G4Translate3D(21.7, 1.5, 0);
      assemblySup->AddPlacedVolume(l1_peek2, transform);

      G4Trd* s1_al0 = new G4Trd("s1_al0", 12. / 2. ,  20. / 2. , 3. / 2., 3. / 2., 94.5 / 2.);
      G4Box* s1_al1 = new G4Box("s1_al1", 12. / 2. ,  20.5 / 2. , 3.5 / 2.);
      G4Box* s1_al2 = new G4Box("s1_al2", 12. / 2. ,  4.0 / 2. , 10.5 / 2.);

      G4LogicalVolume* l1_al0 = new G4LogicalVolume(s1_al0, geometry::Materials::get("Al"), "PLUME.l1_al0");
      G4LogicalVolume* l1_al1 = new G4LogicalVolume(s1_al1, geometry::Materials::get("Al"), "PLUME.l1_al1");
      G4LogicalVolume* l1_al2 = new G4LogicalVolume(s1_al2, geometry::Materials::get("Al"), "PLUME.l1_al2");

      transform = G4Translate3D(59.25, -1.5, 0) * G4RotateY3D(-M_PI / 2.);
      assemblySup->AddPlacedVolume(l1_al0, transform);

      transform = G4Translate3D(108.25, -20.5 / 2., 0) * G4RotateY3D(-M_PI / 2.);
      assemblySup->AddPlacedVolume(l1_al1, transform);

      transform = G4Translate3D(115.25, -18.5, 0) * G4RotateY3D(-M_PI / 2.);
      assemblySup->AddPlacedVolume(l1_al2, transform);

      return assemblySup;
    }


    G4AssemblyVolume* PlumeCreator::buildSupport2()
    {

      std::vector<G4TwoVector> points = {G4TwoVector(-10.6, 0), G4TwoVector(-10.6, -1.7), G4TwoVector(-0.606, -2.049), G4TwoVector(-0.606, -4.15), G4TwoVector(-10.6, -3.801), G4TwoVector(-10.6, -6.11), G4TwoVector(2.28, -6.11), G4TwoVector(5.39, -3.0), G4TwoVector(23.4, -3), G4TwoVector(23.4, 0.)};
      G4Material* peek = G4Material::GetMaterial("plumePeek");
      G4ExtrudedSolid* s2_peek0 = new G4ExtrudedSolid("s2_peek0", points, 16.0, G4TwoVector(0, 0), 1, G4TwoVector(0, 0), 1);
      G4LogicalVolume* l2_peek0 = new G4LogicalVolume(s2_peek0, peek, "PLUME.l2_peek0");


      G4Box* s2_peek1 = new G4Box("s2_peek1", 10. / 2. ,  6.7 / 2. , 12.5 / 2.);
      G4Box* s2_peek2 = new G4Box("s2_peek2", 39.0 / 2.,  4. / 2. , 12.5 / 2.);
      G4LogicalVolume* l2_peek1 = new G4LogicalVolume(s2_peek1, peek, "PLUME.l2_peek1");
      G4LogicalVolume* l2_peek2 = new G4LogicalVolume(s2_peek2, peek, "PLUME.l2_peek2");

      G4AssemblyVolume* assemblySup = new G4AssemblyVolume();

      G4Transform3D transform = G4Translate3D(0, 0., 0);

      assemblySup->AddPlacedVolume(l2_peek1, transform);

      transform = G4Translate3D(-18.4, 6.35, 0);
      assemblySup->AddPlacedVolume(l2_peek0, transform);

      transform = G4Translate3D(24.5, -1.35, 0);
      assemblySup->AddPlacedVolume(l2_peek2, transform);

      G4Box* s2_al0 = new G4Box("s2_al0", 140. / 2. , 4. / 2. , 12. / 2.);
      G4LogicalVolume* l2_al0 = new G4LogicalVolume(s2_al0, geometry::Materials::get("Al"), "PLUME.l2_al0");

      transform = G4Translate3D(-26.0, -5.35, 0);
      assemblySup->AddPlacedVolume(l2_al0, transform);

      return assemblySup;
    }


    G4AssemblyVolume* PlumeCreator::buildSupport3()
    {

      std::vector<G4TwoVector> points = {G4TwoVector(4.7, -59.87), G4TwoVector(/*4.109*/4.7, -68.0), G4TwoVector(19.577, -62.97), G4TwoVector(18.96, -61.07), G4TwoVector(9.449, -64.162), G4TwoVector(8.8, -62.165), G4TwoVector(18.31, -59.075), G4TwoVector(17.693, -57.172), G4TwoVector(6.655, -59.873)};
      G4Material* peek = G4Material::GetMaterial("plumePeek");
      G4ExtrudedSolid* s3_peek0 = new G4ExtrudedSolid("s3_peek0", points, 16.0, G4TwoVector(0, 0), 1, G4TwoVector(0, 0), 1);
      G4LogicalVolume* l3_peek0 = new G4LogicalVolume(s3_peek0, peek, "PLUME.l3_peek0");

      G4Box* s3_tmp0 = new G4Box("s3_tmp0", 5. / 2. , 21.5 / 2., 32. / 2.);
      G4Box* s3_tmp1 = new G4Box("s3_tmp1", 5.2 / 2. , 6.5 / 2., 30. / 2.);
      G4Transform3D transform = G4Translate3D(0, -1.6, 0);
      G4SubtractionSolid* s3_peek1 = new G4SubtractionSolid("s3_peek1", s3_tmp0, s3_tmp1, transform);

      G4Trd* s3_peek2 = new G4Trd("s3_peek2", 5. / 2., 5. / 2., 16., 10.,  3.0);
      G4Box* s3_peek3 = new G4Box("s3_peek3", 3. / 2. , 15.5 / 2., 20. / 2.);
      G4LogicalVolume* l3_peek1 = new G4LogicalVolume(s3_peek1, peek, "PLUME.l3_peek1");
      G4LogicalVolume* l3_peek2 = new G4LogicalVolume(s3_peek2, peek, "PLUME.l3_peek2");
      G4LogicalVolume* l3_peek3 = new G4LogicalVolume(s3_peek3, peek, "PLUME.l3_peek3");

      G4AssemblyVolume* assemblySup = new G4AssemblyVolume();

      G4ThreeVector Ta(0, 0, 0);
      G4RotationMatrix ra;
      G4Transform3D tra(ra, Ta);
      assemblySup->AddPlacedVolume(l3_peek1, tra);
      Ta.setY(21.5 / 2. + 3.0);
      ra.rotateX(-M_PI / 2.);
      tra = G4Transform3D(ra, Ta);
      assemblySup->AddPlacedVolume(l3_peek2, tra);
      Ta.setY(21.5 / 2. + 6.0 + 15.5 / 2.); Ta.setX(-1.0);
      ra.rotateX(M_PI / 2.);
      tra = G4Transform3D(ra, Ta);
      assemblySup->AddPlacedVolume(l3_peek3, tra);

      Ta.setY(57.25); Ta.setX(-2.2);
      tra = G4Transform3D(ra, Ta);
      assemblySup->AddPlacedVolume(l3_peek0, tra);


      G4Trd* s3_al0 = new G4Trd("s3_al0", 3. / 2. , 3. / 2., 10.0, 6.0, 51.1566 / 2.);
      G4Box* s3_al1 = new G4Box("s3_al1", 12. / 2. ,  4. / 2. , 12. / 2.);
      G4LogicalVolume* l3_al0 = new G4LogicalVolume(s3_al0, geometry::Materials::get("Al"), "PLUME.l3_al0");
      G4LogicalVolume* l3_al1 = new G4LogicalVolume(s3_al1, geometry::Materials::get("Al"), "PLUME.l3_al1");
      ra.rotateX(-M_PI / 2.);
      Ta.setY(21.5 / 2. + 6.0 + 51.1566 / 2.); Ta.setX(+2.0);
      tra = G4Transform3D(ra, Ta);
      assemblySup->AddPlacedVolume(l3_al0, tra);
      ra.rotateX(M_PI / 2.);
      Ta.setY(21.5 / 2. + 6.0 + 51.1566 + 2.); Ta.setX(-2.5);
      tra = G4Transform3D(ra, Ta);
      assemblySup->AddPlacedVolume(l3_al1, tra);
      return assemblySup;
    }

    G4AssemblyVolume* PlumeCreator::buildSupport4()
    {


      std::vector<G4TwoVector> points = {G4TwoVector(-36.8, 6.645), G4TwoVector(-26.65, 9.94), G4TwoVector(-16.04, -3.0), G4TwoVector(3.5, -3.0), G4TwoVector(3.5, 0.0), G4TwoVector(-14.62, 0.0), G4TwoVector(-27.742, 16.0), G4TwoVector(-38.68, 12.446), G4TwoVector(-38.062, 10.544), G4TwoVector(-28.551, 13.634), G4TwoVector(-27.9, 11.637), G4TwoVector(-37.41, 8.547)};
      G4Material* peek = G4Material::GetMaterial("plumePeek");
      G4ExtrudedSolid* s4_peek0 = new G4ExtrudedSolid("s4_peek0", points, 10.0, G4TwoVector(0, 0), 1, G4TwoVector(0, 0), 1);
      G4LogicalVolume* l4_peek0 = new G4LogicalVolume(s4_peek0, peek, "PLUME.l4_peek0");

      G4Trd* s4_al0 = new G4Trd("s4_al0", 12. / 2. ,  20. / 2. , 3. / 2., 3. / 2., 36.5 / 2.);
      G4Box* s4_al1 = new G4Box("s4_al1", 12. / 2. ,  16. / 2. , 3.5 / 2.);
      G4Box* s4_al2 = new G4Box("s4_al2", 12. / 2. ,  4. / 2. , 14. / 2.);

      G4LogicalVolume* l4_al0 = new G4LogicalVolume(s4_al0, geometry::Materials::get("Al"), "PLUME.l4_al0");
      G4LogicalVolume* l4_al1 = new G4LogicalVolume(s4_al1, geometry::Materials::get("Al"), "PLUME.l4_al1");
      G4LogicalVolume* l4_al2 = new G4LogicalVolume(s4_al2, geometry::Materials::get("Al"), "PLUME.l4_al2");

      G4ThreeVector transl(0, 0, 0);
      G4RotationMatrix ra;
      G4Transform3D trans(ra, transl);
      G4AssemblyVolume* assem_al = new G4AssemblyVolume();
      assem_al->AddPlacedVolume(l4_al0, trans);
      transl.setY(-16. / 2. + 3. / 2.); transl.setZ(-36.5 / 2. - 3.5 / 2.);
      trans = G4Transform3D(ra, transl);
      assem_al->AddPlacedVolume(l4_al1, trans);
      transl.setZ(-36.5 / 2. - 14. / 2.); transl.setY(-16.5);
      trans = G4Transform3D(ra, transl);
      assem_al->AddPlacedVolume(l4_al2, trans);
      ra.rotateY(M_PI / 2.); ra.rotateZ(M_PI);
      transl.setZ(36.5 / 2. - 10.5); transl.setY(1.50);
      trans = G4Transform3D(ra, transl);
      assem_al->AddPlacedVolume(l4_peek0, trans);

      return assem_al;
    }

  } // plume namespace
} // Belle2 namespace
