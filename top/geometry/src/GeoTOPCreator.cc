/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/GeoTOPCreator.h>
#include <top/geometry/TOPGeometryPar.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <top/simulation/SensitivePMT.h>
#include <top/simulation/SensitiveBar.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include <simulation/kernel/RunManager.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4OpticalSurface.hh>
//Shapes
#include <G4Trd.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Polycone.hh>
#include <G4SubtractionSolid.hh>
#include <G4UserLimits.hh>
#include <G4Material.hh>

#include <G4TwoVector.hh>
#include <G4ThreeVector.hh>
#include <G4ExtrudedSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Sphere.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Colour.hh>

using namespace std;
using namespace boost;


namespace Belle2 {

  using namespace geometry;

  namespace TOP {

    /** Register the creator */
    geometry::CreatorFactory<GeoTOPCreator> GeoTOPFactory("TOPCreator");


    GeoTOPCreator::GeoTOPCreator()
    {
      m_sensitivePMT = new SensitivePMT();
      m_sensitiveBar = new SensitiveBar();
    }


    GeoTOPCreator::~GeoTOPCreator()
    {
      if (m_sensitivePMT) delete m_sensitivePMT;
      if (m_sensitiveBar) delete m_sensitiveBar;
      if (m_sensitivePCB1) delete m_sensitivePCB1;
      if (m_sensitivePCB2) delete m_sensitivePCB2;
      if (m_topgp) delete m_topgp;
      if (m_geometry) delete m_geometry;
      G4LogicalSkinSurface::CleanSurfaceTable();
    }


    const TOPGeometry* GeoTOPCreator::createConfiguration(const GearDir& content)
    {
      TOPGeometry* geo = new TOPGeometry();

      // PMT array

      GearDir pmtParams(content, "PMTs/Module");
      TOPGeoPMT pmt(pmtParams.getLength("ModuleXSize"),
                    pmtParams.getLength("ModuleYSize"),
                    pmtParams.getLength("ModuleZSize") +
                    pmtParams.getLength("WindowThickness") +
                    pmtParams.getLength("BottomThickness"));
      pmt.setWallThickness(pmtParams.getLength("ModuleWall"));
      pmt.setWallMaterial(pmtParams.getString("wallMaterial"));
      pmt.setFillMaterial(pmtParams.getString("fillMaterial"));
      pmt.setSensVolume(pmtParams.getLength("SensXSize"),
                        pmtParams.getLength("SensYSize"),
                        pmtParams.getLength("SensThickness"),
                        pmtParams.getString("sensMaterial"));
      pmt.setNumPixels(pmtParams.getInt("PadXNum"),
                       pmtParams.getInt("PadYNum"));
      pmt.setWindow(pmtParams.getLength("WindowThickness"),
                    pmtParams.getString("winMaterial"));
      pmt.setBottom(pmtParams.getLength("BottomThickness"),
                    pmtParams.getString("botMaterial"));

      auto& materials = geometry::Materials::getInstance();
      GearDir reflEdgeSurfParams(pmtParams, "reflectiveEdge/Surface");
      pmt.setReflEdge(pmtParams.getLength("reflectiveEdge/width"),
                      pmtParams.getLength("reflectiveEdge/thickness"),
                      materials.createOpticalSurfaceConfig(reflEdgeSurfParams));

      GearDir arrayParams(content, "PMTs");
      TOPGeoPMTArray pmtArray(arrayParams.getInt("nPMTx"),
                              arrayParams.getInt("nPMTy"),
                              arrayParams.getLength("Xgap"),
                              arrayParams.getLength("Ygap"),
                              arrayParams.getString("stackMaterial"),
                              pmt);
      geo->setPMTArray(pmtArray);

      // modules

      GearDir barParams(content, "Bars");
      GearDir barSurfParams(barParams, "Surface");
      auto barSurface = materials.createOpticalSurfaceConfig(barSurfParams);
      double sigmaAlpha = barSurfParams.getDouble("SigmaAlpha");

      TOPGeoBarSegment bar1(barParams.getLength("QWidth"),
                            barParams.getLength("QThickness"),
                            barParams.getLength("QBar1Length"),
                            barParams.getString("BarMaterial"));
      bar1.setGlue(barParams.getLength("Glue/Thicknes2"),
                   barParams.getString("Glue/GlueMaterial"));
      bar1.setSurface(barSurface, sigmaAlpha);

      TOPGeoBarSegment bar2(barParams.getLength("QWidth"),
                            barParams.getLength("QThickness"),
                            barParams.getLength("QBar2Length"),
                            barParams.getString("BarMaterial"));
      bar2.setGlue(barParams.getLength("Glue/Thicknes1"),
                   barParams.getString("Glue/GlueMaterial"));
      bar2.setSurface(barSurface, sigmaAlpha);

      TOPGeoMirrorSegment mirror(barParams.getLength("QWidth"),
                                 barParams.getLength("QThickness"),
                                 barParams.getLength("QBarMirror"),
                                 barParams.getString("BarMaterial"));
      mirror.setGlue(barParams.getLength("Glue/Thicknes3"),
                     barParams.getString("Glue/GlueMaterial"));
      mirror.setSurface(barSurface, sigmaAlpha);
      GearDir mirrorParams(content, "Mirror");
      mirror.setRadius(mirrorParams.getLength("Radius"));
      mirror.setCenterOfCurvature(mirrorParams.getLength("Xpos"),
                                  mirrorParams.getLength("Ypos"));
      GearDir mirrorSurfParams(mirrorParams, "Surface");
      mirror.setCoating(mirrorParams.getLength("mirrorThickness"),
                        mirrorParams.getString("Material"),
                        materials.createOpticalSurfaceConfig(mirrorSurfParams));

      TOPGeoPrism prism(barParams.getLength("QWedgeWidth"),
                        barParams.getLength("QThickness"),
                        barParams.getLength("QWedgeLength"),
                        barParams.getLength("QWedgeDown") +
                        barParams.getLength("QThickness"),
                        barParams.getLength("QWedgeFlat"),
                        barParams.getString("BarMaterial"));
      prism.setGlue(arrayParams.getLength("dGlue"),
                    arrayParams.getString("glueMaterial"));
      prism.setSurface(barSurface, sigmaAlpha);

      double radius = barParams.getLength("Radius") + barParams.getLength("QThickness") / 2;
      double phi = barParams.getLength("Phi0");
      double backwardZ = barParams.getLength("QZBackward");
      int numModules = barParams.getInt("Nbar");
      for (int i = 0; i < numModules; i++) {
        TOPGeoModule module(i + 1, radius, phi, backwardZ);
        module.setBarSegment1(bar1);
        module.setBarSegment2(bar2);
        module.setMirrorSegment(mirror);
        module.setPrism(prism);
        // module.setModuleCNumber(num);
        // module.setPMTArrayDisplacement(arrayDispl);
        // module.setModuleDisplacement(moduleDispl);
        geo->appendModule(module);
        phi += 2 * M_PI / numModules;

      }

      // boardstack

      TOPGeoBoardStack bs; // TODO
      geo->setBoardStack(bs);

      // QBB

      TOPGeoQBB qbb; // TODO
      geo->setQBB(qbb);

      // nominal QE

      GearDir qeParams(content, "QE");
      std::vector<float> qeData;
      for (const GearDir& Qeffi : qeParams.getNodes("Qeffi")) {
        qeData.push_back(Qeffi.getDouble(""));
      }
      TOPNominalQE nominalQE(qeParams.getLength("LambdaFirst") / Unit::nm,
                             qeParams.getLength("LambdaStep") / Unit::nm,
                             qeParams.getDouble("ColEffi"),
                             qeData);
      geo->setNominalQE(nominalQE);

      // nominal TTS

      GearDir ttsParams(content, "PMTs/TTS");
      TOPNominalTTS nominalTTS;
      for (const GearDir& Gauss : ttsParams.getNodes("Gauss")) {
        nominalTTS.appendGaussian(Gauss.getDouble("fraction"),
                                  Gauss.getTime("mean"),
                                  Gauss.getTime("sigma"));
      }
      nominalTTS.normalize();
      geo->setNominalTTS(nominalTTS);

      // nominal TDC

      GearDir tdcParams(content, "TDC");
      if (tdcParams) {
        TOPNominalTDC nominalTDC(tdcParams.getInt("numWindows"),
                                 tdcParams.getInt("subBits"),
                                 tdcParams.getTime("syncTimeBase"),
                                 tdcParams.getTime("offset"),
                                 tdcParams.getTime("pileupTime"),
                                 tdcParams.getTime("doubleHitResolution"),
                                 tdcParams.getTime("timeJitter"),
                                 tdcParams.getDouble("efficiency"));
        geo->setNominalTDC(nominalTDC);
      } else {
        TOPNominalTDC nominalTDC(pmtParams.getInt("TDCbits"),
                                 pmtParams.getTime("TDCbitwidth"),
                                 pmtParams.getTime("TDCoffset", 0),
                                 pmtParams.getTime("TDCpileupTime", 0),
                                 pmtParams.getTime("TDCdoubleHitResolution", 0),
                                 pmtParams.getTime("TDCtimeJitter", 50e-3),
                                 pmtParams.getDouble("TDCefficiency", 1));
        geo->setNominalTDC(nominalTDC);
      }

      // check for consistency

      if (!geo->isConsistent())
        B2ERROR("GeoTOPCreator::createConfiguration: geometry not consistently defined");

      return geo;
    }




    void GeoTOPCreator::create(const GearDir& content, G4LogicalVolume& topVolume,
                               GeometryTypes)
    {

      m_isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      m_geometry = createConfiguration(content);
      m_topgp->setGeometry(m_geometry);

      /*! Build detector segment */

      //! Initialize parameters
      m_topgp->Initialize(content);
      m_topgp->setGeanUnits();

      G4int moduleID = 0; // this is dummy
      // build one module
      G4LogicalVolume* module = buildTOPModule(content, moduleID);

      G4double Radius = m_topgp->getRadius() + m_topgp->getQthickness() / 2.0;
      G4double phi0 = m_topgp->getPhi0();
      G4int Nbars = m_topgp->getNbars();

      //! position the segment
      for (G4int i = 0; i < Nbars; i++) {

        G4double phi = i * 2 * M_PI / ((double)Nbars) + phi0;

        G4RotationMatrix rot(M_PI / 2.0, M_PI / 2.0, -phi);
        G4ThreeVector trans(Radius * cos(phi), Radius * sin(phi), 0);

        new G4PVPlacement(G4Transform3D(rot, trans), module, "PlacedTOPModule",
                          &topVolume, false, i + 1);

      }
    }



    G4AssemblyVolume* GeoTOPCreator::buildBar(const GearDir& content, const int)
    {

      /*!  Read parameters  */

      G4double Qwidth = m_topgp->getQwidth();
      G4double Qthickness = m_topgp->getQthickness();
      G4double Bposition = m_topgp->getBposition();
      G4double Length1 = m_topgp->getLength1();
      G4double Length2 = m_topgp->getLength2();
      G4double Length3 = m_topgp->getLength3();
      G4double WLength = m_topgp->getWLength();
      G4double Wwidth = m_topgp->getWwidth();
      G4double Wextdown = m_topgp->getWextdown();
      G4double Wflat = m_topgp->getWflat();
      G4double Gwidth1 = m_topgp->getGwidth1();
      G4double Gwidth2 = m_topgp->getGwidth2();
      G4double Gwidth3 = m_topgp->getGwidth3();

      G4double Mirposx = m_topgp->getMirposx();
      G4double Mirposy = m_topgp->getMirposy();
      G4double Mirthickness = m_topgp->getMirthickness();
      G4double Mirradius = m_topgp->getMirradius();

      G4double dGlue = m_topgp->getdGlue();
      G4double SigmaAlpha = m_topgp->getSigmaAlpha();

      //!calculate the total length and width of the qurtz bar

      G4double length = Length1 + Length2 + Length3 + Gwidth2 + Gwidth3;

      /*!  Read materials  */

      //! read bar material names
      GearDir materialNames(content, "Bars");
      string quartzName = materialNames.getString("BarMaterial");
      string glueName = materialNames.getString("Glue/GlueMaterial");

      G4Material* quartzMaterial = Materials::get(quartzName);
      G4Material* glueMaterial = Materials::get(glueName);

      //! read PMT glueing parameter name
      GearDir gluematerialNames(content, "PMTs");
      string gluePMTName = gluematerialNames.getString("glueMaterial");
      G4Material* gluePMTMaterial = Materials::get(gluePMTName);
      if (!gluePMTMaterial) { B2FATAL("Material '" << gluePMTName << "', required for gluing of PMTs to quartz not found");}


      /*!  Build raw bar  */

      G4Box* bar = new G4Box("bar", length / 2.0, Qthickness / 2.0 , Qwidth / 2.0);

      G4LogicalVolume* qbar = new G4LogicalVolume(bar, quartzMaterial, "cuttest");

      //!make quartz bar sensitive for tracking.
      qbar->SetSensitiveDetector(m_sensitiveBar);

      //! set the color of the bar
      setColor(*qbar, "rgb(0.0,0.75,1.0,1.0)");

      GearDir barSurface(content, "Bars/Surface");
      if (barSurface) {
        Materials& materials = Materials::getInstance();
        G4OpticalSurface* optSurf = materials.createOpticalSurface(barSurface);
        optSurf->SetSigmaAlpha(SigmaAlpha);
        new G4LogicalSkinSurface("barSurface", qbar, optSurf);
      } else B2WARNING("TOP bar: no surface properties defined!");

      /*!  Build shapes of mirror  */
      G4double   pRmin = Mirradius;
      G4double   pRmax = Mirradius + Mirthickness * 100.0;

      G4double   phimax = atan(Qthickness / Mirradius); // cca 0.2 deg
      G4double   thetamax = atan(Qwidth / 2.0 / Mirradius); // cca 2deg


      G4double   pSPhi = M_PI / 2.0 - phimax * 0.5;
      G4double   pDPhi = phimax * 2.0;
      G4double   pSTheta = M_PI / 2.0 - thetamax * 1.5;
      G4double   pDTheta = 3 * thetamax ;

      G4Sphere* mirsphere = new G4Sphere("mirror_sphere_segment",
                                         pRmin, pRmax, pSPhi, pDPhi, pSTheta, pDTheta);

      G4RotationMatrix rotmir(M_PI / 2.0 + phimax, 0, 0);
      G4ThreeVector transmir(-Mirradius + length / 2.0, Mirposy, Mirposx);

      G4IntersectionSolid* mirrorshape = new G4IntersectionSolid("bar*mirsphere", bar, mirsphere, G4Transform3D(rotmir, transmir));


      /*!  Position mirror  */

      G4LogicalVolume* mirror = new G4LogicalVolume(mirrorshape, quartzMaterial, "mirror");

      GearDir surface(content, "Mirror/Surface");
      if (surface) {
        Materials& materials = Materials::getInstance();
        G4OpticalSurface* optSurf = materials.createOpticalSurface(surface);
        new G4LogicalSkinSurface("mirrorsSurface", mirror, optSurf);
      } else B2WARNING("TOP mirrors surface has no specified optical properties. No photons will be reflected.");

      G4Transform3D dumb = G4Translate3D(0, 0, 0);
      new G4PVPlacement(dumb, mirror, "TOP.mirror", qbar, false, 1);


      /*!  Build glue joints and assign material  */

      //! for quartz qluing
      G4Box* box2 = new G4Box("glue_2", Gwidth2 / 2.0, Qthickness / 2.0, Qwidth / 2.0);
      G4Box* box3 = new G4Box("glue_3", Gwidth3 / 2.0, Qthickness / 2.0, Qwidth / 2.0);

      G4LogicalVolume* gbox2 = new G4LogicalVolume(box2, glueMaterial, "gbox2");
      G4LogicalVolume* gbox3 = new G4LogicalVolume(box3, glueMaterial, "gbox3");

      //! color the glue
      setColor(*gbox2, "rgb(0.0,1.0,1.0,1.0)");
      setColor(*gbox3, "rgb(0.0,1.0,1.0,1.0)");

      /*!  Place glue joints */

      G4Transform3D tglue2 = G4Translate3D(-length / 2.0 + Length1 + Gwidth2 / 2.0 , 0, 0);
      G4Transform3D tglue3 = G4Translate3D(-length / 2.0 + Length1 + Gwidth2 + Length2 + Gwidth3 / 2.0 , 0, 0);


      new G4PVPlacement(tglue2, gbox2, "TOP.gbox2", qbar, false, 1);
      new G4PVPlacement(tglue3, gbox3, "TOP.gbox3", qbar, false, 1);



      /*!  Build wedge  */


      std::vector<G4TwoVector> polygon;

      polygon.push_back(G4TwoVector(-length / 2.0,
                                    Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0,
                                    -Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1,
                                    -Qthickness / 2.0));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1 - WLength + Wflat,
                                    -Qthickness / 2.0 - Wextdown));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1 - WLength - dGlue,
                                    -Qthickness / 2.0 - Wextdown));
      polygon.push_back(G4TwoVector(-length / 2.0 - Gwidth1 - WLength - dGlue,
                                    Qthickness / 2.0));

      G4ExtrudedSolid* wedge = new  G4ExtrudedSolid("wedge", polygon, Wwidth / 2.0, G4TwoVector(0.0, 0.0), 1.0, G4TwoVector(0.0, 0.0),
                                                    1.0);

      G4LogicalVolume* qwedge = new G4LogicalVolume(wedge, quartzMaterial, "qwedge");

      //!color the glue

      setColor(*qwedge, "rgb(0.0,0.75,1.0,1.0)");


      /*! Add glue to wedge */

      double brokenFraction = content.getDouble("Bars/BrokenJointFraction", 0);

      if (brokenFraction <= 0) {
        //! for quartz gluing
        G4Box* box1 = new G4Box("glue_1", Gwidth3 / 2.0, Qthickness / 2.0, Wwidth / 2.0);
        G4LogicalVolume* gbox1 = new G4LogicalVolume(box1, glueMaterial, "gbox1");

        //! color the glue
        setColor(*gbox1, "rgb(0.0,1.0,1.0,1.0)");

        //! place the glue
        G4Transform3D tglue1 = G4Translate3D(-length / 2.0 - Gwidth1 / 2.0 , 0, 0);
        new G4PVPlacement(tglue1, gbox1, "TOP.gbox1", qwedge, false, 1);

      } else if (brokenFraction >= 1) {
        B2WARNING("GeoTOPCreator: simulating totaly broken glue joint at prism");

        G4Material* brokenMaterial = Materials::get("TOPAir");
        G4Box* box1 = new G4Box("glue_1", Gwidth3 / 2.0, Qthickness / 2.0, Wwidth / 2.0);
        G4LogicalVolume* gbox1 = new G4LogicalVolume(box1, brokenMaterial, "gbox1");
        G4Transform3D tglue1 = G4Translate3D(-length / 2.0 - Gwidth1 / 2.0 , 0, 0);
        new G4PVPlacement(tglue1, gbox1, "TOP.gbox1", qwedge, false, 1);

      } else {
        B2WARNING("GeoTOPCreator: simulating broken glue joint at prism with fraction "
                  << brokenFraction);
        double glueWidth = Wwidth * (1 - brokenFraction);
        double brokenWidth = Wwidth * brokenFraction;
        double glueZc = Wwidth * brokenFraction / 2;
        double brokenZc = Wwidth * (brokenFraction - 1) / 2;
        G4Material* brokenMaterial = Materials::get("TOPAir");

        G4Box* box1 = new G4Box("glue_1", Gwidth3 / 2, Qthickness / 2, glueWidth / 2);
        G4LogicalVolume* gbox1 = new G4LogicalVolume(box1, glueMaterial, "gbox1");
        G4Transform3D tglue1 = G4Translate3D(-length / 2.0 - Gwidth1 / 2.0 , 0, glueZc);
        new G4PVPlacement(tglue1, gbox1, "TOP.gbox1", qwedge, false, 1);

        G4Box* box1b = new G4Box("glue_1b", Gwidth3 / 2, Qthickness / 2, brokenWidth / 2);
        G4LogicalVolume* gbox1b = new G4LogicalVolume(box1b, brokenMaterial, "gbox1b");
        G4Transform3D tglue1b = G4Translate3D(-length / 2 - Gwidth1 / 2 , 0, brokenZc);
        new G4PVPlacement(tglue1b, gbox1b, "TOP.gbox1b", qwedge, false, 1);
      }


      //! for PMT gluing

      G4Box* box4 = new G4Box("glue_4", dGlue / 2.0, (Qthickness + Wextdown) / 2.0, Wwidth / 2.0);
      G4LogicalVolume* gbox4 = new G4LogicalVolume(box4, gluePMTMaterial, "PMTglue");

      //!color the glue
      setColor(*gbox4, "rgb(0.0,1.0,1.0,1.0)");

      //!place the glue
      G4Transform3D tglue4 = G4Translate3D(-length / 2.0 - Gwidth1 - WLength - dGlue / 2.0 , -Wextdown / 2.0, 0);


      new G4PVPlacement(tglue4, gbox4, "TOP.gbox4", qwedge, false, 1);

      /*! Put all segment into the Assembly volume */

      //- G4RotationMatrix* rotasem = new G4RotationMatrix(0, 0, 0); // Identity matrix not needed
      G4ThreeVector trnsasem(length / 2.0 + Bposition, 0, 0);

      G4AssemblyVolume* assemblyDetector = new G4AssemblyVolume();
      Simulation::RunManager::Instance().addAssemblyVolume(assemblyDetector);
      assemblyDetector->AddPlacedVolume(qbar, trnsasem, 0);
      assemblyDetector->AddPlacedVolume(qwedge, trnsasem, 0);

      //! get the PMT stack and position it
      G4LogicalVolume* stack = buildPMTstack(content);

      G4double dz = Bposition - WLength - Gwidth1 - m_topgp->getdGlue() - (m_topgp->getWinthickness() + m_topgp->getMsizez() +
                    m_topgp->getBotthickness()) / 2.0;
      G4double dx = (-Wextdown) / 2.0;

      G4RotationMatrix* rotsta = new G4RotationMatrix(M_PI / 2.0, -M_PI / 2.0, -M_PI / 2.0);
      G4ThreeVector trnssta(dz, dx, 0);


      assemblyDetector->AddPlacedVolume(stack, trnssta, rotsta);
      delete rotsta; // AddPlacedVolume() made an internal copy of the rotation matrix

      return assemblyDetector;



    }



    G4LogicalVolume* GeoTOPCreator::buildTOPModule(const GearDir& content, const int moduleID)
    {

      /*!  Read parameters  */

      G4double Qwidth = m_topgp->getQwidth();
      if (m_topgp->getWwidth() > Qwidth) Qwidth = m_topgp->getWwidth();
      G4double Qthickness = m_topgp->getQthickness();
      G4double Z1 = m_topgp->getZ1();
      G4double WLength = m_topgp->getWLength();
      G4double Wextdown = m_topgp->getWextdown();
      //      G4double Wflat = m_topgp->getWflat();
      G4double PannelThickness = m_topgp->getPannelThickness();
      G4double PlateThickness = m_topgp->getPlateThickness();
      G4double LowerGap = m_topgp->getLowerGap();
      G4double UpperGap = m_topgp->getUpperGap();
      G4double SideGap = m_topgp->getSideGap();
      G4double ZForward = m_topgp->getZForward();
      G4double ZBackward = m_topgp->getZBackward();
      G4double dx = m_topgp->getMsizex();
      G4double dy = m_topgp->getMsizey();
      G4double Xgap = m_topgp->getXgap();
      G4double Ygap = m_topgp->getYgap();
      G4int  Npmtx = m_topgp->getNpmtx();
      G4int Npmty = m_topgp->getNpmty();

      //! size of the whole stack
      G4double x = dx * Npmtx + Xgap * (Npmtx - 1);
      G4double y = dy * Npmty + Ygap * (Npmty - 1);

      //! Check which is higher the wedge or the PMT stack
      G4double upside = 0;
      G4double downside = 0;

      if ((y - Wextdown) / 2.0 > Qthickness / 2.0) {
        upside = (y - Wextdown) / 2.0;
        downside = -Wextdown / 2.0 - y / 2.0;
      } else {
        upside = Qthickness / 2.0;
        downside = -Wextdown - Qthickness / 2.0;
      }

      //! Check which is wider bar or PMTs
      G4double side = 0;

      if (x / 2.0 > Qwidth / 2.0) {
        side = x / 2.0;
      } else {
        side = Qwidth / 2.0;
      }

      /*!  Build wedge  */

      //! Build the outer wall
      std::vector<G4TwoVector> polygon;

      polygon.push_back(G4TwoVector(ZBackward,
                                    upside + UpperGap + PannelThickness));
      polygon.push_back(G4TwoVector(ZForward,
                                    upside + UpperGap + PannelThickness));
      polygon.push_back(G4TwoVector(ZForward,
                                    -Qthickness / 2.0 - LowerGap - PannelThickness));
      /*
      polygon.push_back(G4TwoVector(Z1,
                                    -Qthickness / 2.0 - LowerGap - PannelThickness));
      polygon.push_back(G4TwoVector(Z1 - WLength + Wflat,
                                    downside - LowerGap - PlateThickness));
      */
      polygon.push_back(G4TwoVector(Z1 + PlateThickness,
                                    -Qthickness / 2.0 - LowerGap - PannelThickness));
      polygon.push_back(G4TwoVector(Z1 + PlateThickness,
                                    downside - LowerGap - PlateThickness));
      polygon.push_back(G4TwoVector(ZBackward,
                                    downside - LowerGap - PlateThickness));

      G4ExtrudedSolid* supportShape = new  G4ExtrudedSolid("support", polygon, side + SideGap + PlateThickness, G4TwoVector(0.0, 0.0),
                                                           1.0, G4TwoVector(0.0, 0.0), 1.0);

      //! Build inside air
      std::vector<G4TwoVector> polygon2;

      polygon2.push_back(G4TwoVector(ZBackward + PlateThickness,
                                     upside + UpperGap));
      polygon2.push_back(G4TwoVector(ZForward - PlateThickness,
                                     upside + UpperGap));
      polygon2.push_back(G4TwoVector(ZForward - PlateThickness,
                                     -Qthickness / 2.0 - LowerGap));
      /*
      polygon2.push_back(G4TwoVector(Z1,
                                     -Qthickness / 2.0 - LowerGap));
      polygon2.push_back(G4TwoVector(Z1 - WLength + Wflat,
                                     downside - LowerGap));
      */
      polygon2.push_back(G4TwoVector(Z1,
                                     -Qthickness / 2.0 - LowerGap));
      polygon2.push_back(G4TwoVector(Z1,
                                     downside - LowerGap));
      polygon2.push_back(G4TwoVector(ZBackward + PlateThickness,
                                     downside - LowerGap));

      /*
      cout<<"Inner QBB"<<endl;
      for(const auto& point: polygon2){
      cout<<point.x()<<" "<<point.y()<<endl;
      }
      cout<<endl;
      */

      G4ExtrudedSolid* airShape = new  G4ExtrudedSolid("air", polygon2, side + SideGap, G4TwoVector(0.0, 0.0), 1.0, G4TwoVector(0.0, 0.0),
                                                       1.0);

      /*! Read support and fill materials */

      GearDir supportNames(content, "Support");
      string suportName = supportNames.getString("PannelMaterial");
      string fillName = supportNames.getString("FillMaterial");
      G4Material* supportMaterial = Materials::get(suportName);
      G4Material* fillMaterial = Materials::get(fillName);
      if (!supportMaterial) { B2FATAL("Material '" << suportName << "' missing!");}
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}

      /*! Place air inside support and thus create the structure */


      //! build physical volumes
      G4LogicalVolume* support = new G4LogicalVolume(supportShape, supportMaterial, "Support");
      G4LogicalVolume* air = new G4LogicalVolume(airShape, fillMaterial, "Air");

      /*! Build quartz bar and insert it into the air */

      //- G4RotationMatrix* rota = new G4RotationMatrix(0, 0, 0); // Identity matrix not needed
      G4ThreeVector transa(0, 0, 0);
      //      G4AssemblyVolume* bar = buildBar(content, moduleID);
      //      bar->MakeImprint(air, transa, rota, 100, false);
      buildBar(content, moduleID)->MakeImprint(air, transa, 0, 100, false);

      //! Add electronics, this part is not finished, just for backgound studies.
      G4Material* PCBMaterial = Materials::get("TOPPCB");
      G4Box* el1 = new G4Box("el1box", m_topgp->getBotthickness() / 2.0, y / 2.0, x / 2.0);
      G4Box* el2 = new G4Box("el1box", y / 2.0, m_topgp->getBotthickness() / 2.0, x / 2.0);
      G4LogicalVolume* pcb1 = new G4LogicalVolume(el1, PCBMaterial, "Board.1");
      G4LogicalVolume* pcb2 = new G4LogicalVolume(el2, PCBMaterial, "Board.2");

      if (m_isBeamBkgStudy) {
        m_sensitivePCB1 = new BkgSensitiveDetector("TOP", 1);
        pcb1->SetSensitiveDetector(m_sensitivePCB1);
        m_sensitivePCB2 = new BkgSensitiveDetector("TOP", 2);
        pcb2->SetSensitiveDetector(m_sensitivePCB2);
      }


      setColor(*pcb1, "rgb(0.0,1.0,0.0,1.0)");
      setColor(*pcb2, "rgb(0.0,1.0,0.0,1.0)");


      G4ThreeVector transPCB(Z1 - WLength - 2 * m_topgp->getdGlue() - (m_topgp->getWinthickness() + m_topgp->getMsizez() +
                             m_topgp->getBotthickness()) - m_topgp->getBotthickness() / 2.0, -Wextdown / 2.0, 0.);
      G4ThreeVector transPCB2(Z1 - WLength - 3 * m_topgp->getdGlue() - (m_topgp->getWinthickness() + m_topgp->getMsizez() +
                              m_topgp->getBotthickness()) - m_topgp->getBotthickness() - y / 2.0, -Wextdown / 2.0, 0.);

      new G4PVPlacement(0, transPCB, pcb1, "TOP.electronics", air, false, 0);
      new G4PVPlacement(0, transPCB2, pcb2, "TOP.electronics", air, false, 0);



      //! Needed dumy variable to place air inside support shape
      G4Transform3D temp = G4Translate3D(0.0, 0.0, 0.0);

      new G4PVPlacement(temp, air, "TOP.support", support, false, 0);

      /*! return support shape */

      return support;

    }



    G4LogicalVolume* GeoTOPCreator::buildPMTstack(const GearDir& content)
    {

      /*! Get PMT dimensions */

      //! get module side X
      G4double dx = m_topgp->getMsizex();
      //! get module side Y
      G4double dy = m_topgp->getMsizey();
      //! get module wall size in Z
      G4double MWsizez = m_topgp->getMsizez();
      //! get window thickness
      G4double Winthickness = m_topgp->getWinthickness();
      //! get window thickness
      G4double Botthickness = m_topgp->getBotthickness();

      //! get gap between PMTs in x direction
      G4double Xgap = m_topgp->getXgap();
      //! get gap between PMTs in x direction
      G4double Ygap = m_topgp->getYgap();
      //! get number of PMTs in one row
      G4int  Npmtx = m_topgp->getNpmtx();
      //! get number of PMTs in one column
      G4int Npmty = m_topgp->getNpmty();

      //! total z size of a PMT
      G4double dz = Winthickness + MWsizez + Botthickness;

      /*! Get materials for PMT stack that houses the PMTs*/
      GearDir materialNames(content, "PMTs");
      string airName = materialNames.getString("stackMaterial");
      G4Material* airMaterial = Materials::get(airName);
      if (!airMaterial) { B2FATAL("Material '" << airName << "' missing!");}

      /*! Build stack */
      //! size of the whole stack
      G4double x = dx * Npmtx + Xgap * (Npmtx - 1);
      G4double y = dy * Npmty + Ygap * (Npmty - 1);

      //! define the stack
      G4Box* box = new G4Box("stack", x / 2.0, y / 2.0, dz / 2.0);
      G4LogicalVolume* stack = new G4LogicalVolume(box, airMaterial, "stack");

      //! Build one PMT
      G4LogicalVolume* PMT = buildPMT(content);

      //! Place the PMTs inside the stack
      G4double x0 = m_topgp->getPMToffsetX();
      G4double y0 = m_topgp->getPMToffsetY();

      for (G4int iy = 0; iy < Npmty; iy++) {
        for (G4int ix = 0; ix < Npmtx; ix++) {

          G4Transform3D tpmt = G4Translate3D(-x / 2.0 + dx / 2.0 + (dx + Xgap) * ix + x0,
                                             -y / 2.0 + dy / 2.0 + (dy + Ygap) * iy + y0,
                                             0.0);

          new G4PVPlacement(tpmt, PMT, "TOP.window", stack, false,
                            (Npmtx - ix) + (iy * Npmtx)); // numbers run opposite to x

        }
      }

      return stack;

    }




    G4LogicalVolume* GeoTOPCreator::buildPMT(const GearDir& content)
    {

      /*! Get PMT dimensions */

      //! get module side X
      G4double Msizex = m_topgp->getMsizex();
      //! get module side Y
      G4double Msizey = m_topgp->getMsizey();
      //! get module wall size in Z
      G4double MWsizez = m_topgp->getMsizez();
      //! get module wall thicness
      G4double WThickness = m_topgp->getMWallThickness();
      //! get active area size in X
      G4double Asizex = m_topgp->getAsizex();
      //! get active area size in y
      G4double Asizey = m_topgp->getAsizey();
      //! get active area size in z
      G4double Asizez = m_topgp->getAsizez();
      //! get window thickness
      G4double Winthickness = m_topgp->getWinthickness();
      //! get window thickness
      G4double Botthickness = m_topgp->getBotthickness();

      //! total z size of a PMT
      G4double Msizez = Botthickness + MWsizez + Winthickness;

      /*! Get PMT material */

      GearDir materialNames(content, "PMTs/Module");
      string caseName = materialNames.getString("wallMaterial");
      string sensName = materialNames.getString("sensMaterial");
      string winName = materialNames.getString("winMaterial");
      string botName = materialNames.getString("botMaterial");
      string fillName = materialNames.getString("fillMaterial");

      G4Material* caseMaterial = Materials::get(caseName);
      G4Material* sensMaterial = Materials::get(sensName);
      G4Material* winMaterial = Materials::get(winName);
      G4Material* botMaterial = Materials::get(botName);
      G4Material* fillMaterial = Materials::get(fillName);

      //! Check if material is present in xml file
      if (!caseMaterial) { B2FATAL("Material '" << caseName << "' missing!");}
      if (!sensMaterial) { B2FATAL("Material '" << sensName << "' missing!");}
      if (!winMaterial) { B2FATAL("Material '" << winName << "' missing!");}
      if (!botMaterial) { B2FATAL("Material '" << botName << "' missing!");}
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}

      /*! Build PMT segments */

      //! The whole PMT box in which everything will be placed
      G4Box* box = new G4Box("box", Msizex / 2., Msizey / 2., Msizez / 2.);
      G4LogicalVolume* PMT = new G4LogicalVolume(box, caseMaterial, "PMT");

      //! The window of the PMT
      G4Box* box1 = new G4Box("box1", Msizex / 2., Msizey / 2., Winthickness / 2.);
      G4LogicalVolume* window = new G4LogicalVolume(box1, winMaterial, "window");

      //! The sensitive area of the PMT (placed into window bottom)
      G4Box* sensBox = new G4Box("sensbox", Asizex / 2., Asizey / 2., Asizez  / 2.);
      G4LogicalVolume* lmoduleSens = new G4LogicalVolume(sensBox, sensMaterial,
                                                         "SensitivePMT");
      lmoduleSens->SetSensitiveDetector(m_sensitivePMT); // Activate sensitive area
      setColor(*lmoduleSens, "rgb(0.0,0.0,0.0,1.0)");
      G4Transform3D tsens = G4Translate3D(0 , 0, -(Winthickness - Asizez) / 2.);
      new G4PVPlacement(tsens, lmoduleSens, "TOP.moduleSensitive", window, false, 1);

      //! The bottom material of the PMT
      G4Box* box2 = new G4Box("box2", Msizex / 2., Msizey / 2., Botthickness / 2.);
      G4LogicalVolume* bottom = new G4LogicalVolume(box2, botMaterial, "bottom");

      //! The vacuum gap inside the PMT
      G4Box* box3 = new G4Box("box3", Msizex / 2. - WThickness,
                              Msizey / 2. - WThickness , (MWsizez - WThickness) / 2.);
      G4LogicalVolume* vacuum = new G4LogicalVolume(box3, fillMaterial, "vacuum");

      // Reflective edge
      GearDir reflectiveEdge(content, "PMTs/Module/reflectiveEdge");
      if (!reflectiveEdge) B2FATAL("reflectiveEdge: definition is missing");
      double reflEdgeWidth = reflectiveEdge.getLength("width") / Unit::mm;
      double reflEdgeThick = reflectiveEdge.getLength("thickness") / Unit::mm;
      G4Box* box4 = new G4Box("box4", Msizex / 2, Msizey / 2, reflEdgeThick / 2);
      G4LogicalVolume* reflEdge = new G4LogicalVolume(box4, caseMaterial, "reflEdge");
      G4Box* box5 = new G4Box("box5", Msizex / 2 - reflEdgeWidth,
                              Msizey / 2 - reflEdgeWidth, reflEdgeThick / 2.);
      G4LogicalVolume* reflHole = new G4LogicalVolume(box5, fillMaterial, "vacuum");
      G4Transform3D temp = G4Translate3D(0.0, 0.0, 0.0);
      new G4PVPlacement(temp, reflHole, "TOP.vacuum", reflEdge, false, 1);

      GearDir surface(reflectiveEdge, "Surface");
      if (!surface) B2FATAL("reflectiveEdge/Surface: definition is missing");
      Materials& materials = Materials::getInstance();
      G4OpticalSurface* optSurf = materials.createOpticalSurface(surface);
      new G4LogicalSkinSurface("reflectiveEdgeSurface", reflEdge, optSurf);

      /*! Place PMT segments */

      //! Calculate the shifts need for the composition of the PMT
      G4Transform3D twin = G4Translate3D(0 , 0, (Msizez - Winthickness) / 2.);
      G4Transform3D tbot = G4Translate3D(0 , 0, (-Msizez + Botthickness) / 2.);
      G4Transform3D tvacuum = G4Translate3D(0 , 0,
                                            (Msizez - 2 * Winthickness - 2 * WThickness
                                             - MWsizez + WThickness) / 2.);
      G4Transform3D trefl = G4Translate3D(0 , 0,
                                          (Msizez - 2 * Winthickness - reflEdgeThick)
                                          / 2.);

      //! Apply coloring and place all parts into PMT

      setColor(*window, "rgb(0.776,0.886,1.0,1.0)");
      new G4PVPlacement(twin, window, "TOP.window", PMT, false, 1);

      setColor(*bottom, "rgb(1.0,1.0,1.0,1.0)");
      new G4PVPlacement(tbot, bottom, "TOP.bottom", PMT, false, 1);

      new G4PVPlacement(tvacuum, vacuum, "TOP.vacuum", PMT, false, 1);

      new G4PVPlacement(trefl, reflEdge, "TOP.reflEdge", PMT, false, 1);

      setColor(*PMT, "rgb(0.47,0.47,0.47,1.0)");

      //! One piece of PMT is finished

      return PMT;
    }

  }
}
