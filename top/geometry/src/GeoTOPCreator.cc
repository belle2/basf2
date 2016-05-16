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
      if (m_geo) delete m_geo;
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

      m_geo = createConfiguration(content);
      m_topgp->setGeometry(m_geo);

      /* Build detector segment */

      // Initialize parameters
      m_topgp->Initialize(content);
      m_topgp->setGeanUnits();
      m_geo->useGeantUnits();

      // build one module
      G4LogicalVolume* module = buildTOPModule(*m_geo, 1);

      // position the modules
      int numModules = m_geo->getNumModules();
      for (int i = 0; i < numModules; i++) {
        int id = i + 1;
        double Radius = m_geo->getModule(id).getRadius();
        double phi = m_geo->getModule(id).getPhi();

        G4RotationMatrix rot(M_PI / 2.0, M_PI / 2.0, -phi);
        G4ThreeVector trans(Radius * cos(phi), Radius * sin(phi), 0);

        new G4PVPlacement(G4Transform3D(rot, trans), module, "PlacedTOPModule",
                          &topVolume, false, id);
      }
    }



    G4LogicalVolume* GeoTOPCreator::buildTOPModule(const TOPGeometry& geo, int moduleID)
    {
      /*  Read parameters  */

      double Qwidth = geo.getModule(moduleID).getBarWidth();
      double Wwidth = geo.getModule(moduleID).getPrism().getWidth();
      if (Wwidth > Qwidth) Qwidth = Wwidth;

      double Qthickness = geo.getModule(moduleID).getBarThickness();
      double Z1 = geo.getModule(moduleID).getBackwardZ();
      double WLength = geo.getModule(moduleID).getPrism().getLength();
      double Wextdown = geo.getModule(moduleID).getPrism().getExitThickness() -
                        geo.getModule(moduleID).getPrism().getThickness();

      double PannelThickness = m_topgp->getPannelThickness();
      double PlateThickness = m_topgp->getPlateThickness();
      double LowerGap = m_topgp->getLowerGap();
      double UpperGap = m_topgp->getUpperGap();
      double SideGap = m_topgp->getSideGap();
      double ZForward = geo.getModule(moduleID).getForwardZ() + m_topgp->getForwardGap()
                        + PannelThickness;
      double ZBackward = geo.getModule(moduleID).getBackwardZ() - WLength
                         - m_topgp->getBackwardGap() - PannelThickness;

      // size of the whole stack
      double x = geo.getPMTArray().getSizeX();
      double y = geo.getPMTArray().getSizeY();

      // Check which is higher the wedge or the PMT stack
      double upside = 0;
      double downside = 0;

      if ((y - Wextdown) / 2.0 > Qthickness / 2.0) {
        upside = (y - Wextdown) / 2.0;
        downside = -Wextdown / 2.0 - y / 2.0;
      } else {
        upside = Qthickness / 2.0;
        downside = -Wextdown - Qthickness / 2.0;
      }

      // Check which is wider bar or PMTs
      double side = 0;

      if (x / 2.0 > Qwidth / 2.0) {
        side = x / 2.0;
      } else {
        side = Qwidth / 2.0;
      }

      /*  Build wedge  */

      // Build the outer wall
      std::vector<G4TwoVector> polygon;

      polygon.push_back(G4TwoVector(ZBackward,
                                    upside + UpperGap + PannelThickness));
      polygon.push_back(G4TwoVector(ZForward,
                                    upside + UpperGap + PannelThickness));
      polygon.push_back(G4TwoVector(ZForward,
                                    -Qthickness / 2.0 - LowerGap - PannelThickness));
      polygon.push_back(G4TwoVector(Z1 + PlateThickness,
                                    -Qthickness / 2.0 - LowerGap - PannelThickness));
      polygon.push_back(G4TwoVector(Z1 + PlateThickness,
                                    downside - LowerGap - PlateThickness));
      polygon.push_back(G4TwoVector(ZBackward,
                                    downside - LowerGap - PlateThickness));

      G4ExtrudedSolid* supportShape = new G4ExtrudedSolid("support",
                                                          polygon,
                                                          side + SideGap + PlateThickness,
                                                          G4TwoVector(0.0, 0.0),
                                                          1.0,
                                                          G4TwoVector(0.0, 0.0),
                                                          1.0);

      // Build inside air
      std::vector<G4TwoVector> polygon2;

      polygon2.push_back(G4TwoVector(ZBackward + PlateThickness,
                                     upside + UpperGap));
      polygon2.push_back(G4TwoVector(ZForward - PlateThickness,
                                     upside + UpperGap));
      polygon2.push_back(G4TwoVector(ZForward - PlateThickness,
                                     -Qthickness / 2.0 - LowerGap));
      polygon2.push_back(G4TwoVector(Z1,
                                     -Qthickness / 2.0 - LowerGap));
      polygon2.push_back(G4TwoVector(Z1,
                                     downside - LowerGap));
      polygon2.push_back(G4TwoVector(ZBackward + PlateThickness,
                                     downside - LowerGap));

      G4ExtrudedSolid* airShape = new  G4ExtrudedSolid("air",
                                                       polygon2,
                                                       side + SideGap,
                                                       G4TwoVector(0.0, 0.0),
                                                       1.0,
                                                       G4TwoVector(0.0, 0.0),
                                                       1.0);

      /* Read support and fill materials */

      string suportName = m_topgp->getPannelMaterial();
      string fillName = m_topgp->getInsideMaterial();
      G4Material* supportMaterial = Materials::get(suportName);
      G4Material* fillMaterial = Materials::get(fillName);
      if (!supportMaterial) { B2FATAL("Material '" << suportName << "' missing!");}
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}

      /* Place air inside support and thus create the structure */

      // build physical volumes
      G4LogicalVolume* support = new G4LogicalVolume(supportShape,
                                                     supportMaterial, "Support");
      G4LogicalVolume* air = new G4LogicalVolume(airShape, fillMaterial, "Air");

      /* Build quartz bar and insert it into the air */

      G4ThreeVector transa(0, 0, 0);
      buildBar(geo, moduleID)->MakeImprint(air, transa, 0, 100, false);

      /* Add electronics, this part is not finished, just for backgound studies */

      double pcbThickness = geo.getPMTArray().getPMT().getBotThickness();
      G4Box* el1 = new G4Box("el1box", pcbThickness / 2.0, y / 2.0, x / 2.0);
      G4Box* el2 = new G4Box("el1box", y / 2.0, pcbThickness / 2.0, x / 2.0);
      G4Material* PCBMaterial = Materials::get("TOPPCB");
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

      double dGlue = geo.getModule(moduleID).getPrism().getFilterThickness();
      double pmtZ = geo.getPMTArray().getPMT().getSizeZ();
      G4ThreeVector transPCB(Z1 - WLength - 2 * dGlue - pmtZ - pcbThickness / 2.0,
                             -Wextdown / 2.0,
                             0.);
      G4ThreeVector transPCB2(Z1 - WLength - 3 * dGlue - pmtZ - pcbThickness - y / 2.0,
                              -Wextdown / 2.0,
                              0.);

      new G4PVPlacement(0, transPCB, pcb1, "TOP.electronics", air, false, 0);
      new G4PVPlacement(0, transPCB2, pcb2, "TOP.electronics", air, false, 0);


      // Needed dumy variable to place air inside support shape
      G4Transform3D temp = G4Translate3D(0.0, 0.0, 0.0);

      new G4PVPlacement(temp, air, "TOP.support", support, false, 0);

      /* return support shape */

      return support;

    }



    G4AssemblyVolume* GeoTOPCreator::buildBar(const TOPGeometry& geo, int moduleID)
    {
      /*  parameters  */

      const auto& geoModule = geo.getModule(moduleID);

      double Qwidth = geoModule.getBarWidth();
      double Qthickness = geoModule.getBarThickness();
      double Bposition = geoModule.getBackwardZ();
      double Length1 = geoModule.getBarSegment2().getLength();
      double Length2 = geoModule.getBarSegment1().getLength();
      double Length3 = geoModule.getMirrorSegment().getLength();
      double WLength = geoModule.getPrism().getLength();
      double Wwidth = geoModule.getPrism().getWidth();
      double Wextdown = geoModule.getPrism().getExitThickness() -
                        geoModule.getPrism().getThickness();
      double Wflat = geoModule.getPrism().getFlatLength();
      double Gwidth1 = geoModule.getBarSegment2().getGlueThickness();
      double Gwidth2 = geoModule.getBarSegment1().getGlueThickness();
      double Gwidth3 = geoModule.getMirrorSegment().getGlueThickness();

      double Mirposx = geoModule.getMirrorSegment().getXc();
      double Mirposy = geoModule.getMirrorSegment().getYc();
      double Mirthickness = geoModule.getMirrorSegment().getCoatingThickness();
      double Mirradius = geoModule.getMirrorSegment().getRadius();

      double dGlue = geoModule.getPrism().getFilterThickness();
      double SigmaAlpha = geoModule.getBarSegment1().getSigmaAlpha();

      // calculate total length of a bar
      double length = Length1 + Length2 + Length3 + Gwidth2 + Gwidth3;

      /*  materials  */

      string quartzName = geoModule.getBarSegment1().getMaterial();
      string glueName = geoModule.getBarSegment1().getGlueMaterial();
      string gluePMTName = geoModule.getPrism().getGlueMaterial();

      G4Material* quartzMaterial = Materials::get(quartzName);
      G4Material* glueMaterial = Materials::get(glueName);
      G4Material* gluePMTMaterial = Materials::get(gluePMTName);

      if (!quartzMaterial) { B2FATAL("Material '" << quartzName << "' not found");}
      if (!glueMaterial) { B2FATAL("Material '" << glueName << "' not found");}
      if (!gluePMTMaterial) { B2FATAL("Material '" << gluePMTName << "' not found");}

      /*  Build raw bar  */

      G4Box* bar = new G4Box("bar", length / 2.0, Qthickness / 2.0 , Qwidth / 2.0);
      G4LogicalVolume* qbar = new G4LogicalVolume(bar, quartzMaterial, "cuttest");

      // make quartz bar sensitive for tracking.
      qbar->SetSensitiveDetector(m_sensitiveBar);

      // set the color of the bar
      setColor(*qbar, "rgb(0.0,0.75,1.0,1.0)");

      // define optical surface
      const auto& barSurface = geoModule.getBarSegment1().getSurface();
      if (barSurface.hasProperties()) {
        Materials& materials = Materials::getInstance();
        G4OpticalSurface* optSurf = materials.createOpticalSurface(barSurface);
        optSurf->SetSigmaAlpha(SigmaAlpha);
        new G4LogicalSkinSurface("barSurface", qbar, optSurf);
      } else {
        B2WARNING("TOP bar: no surface properties defined");
      }

      /*  Build shapes of mirror  */

      double   pRmin = Mirradius;
      double   pRmax = Mirradius + Mirthickness * 100.0;

      double   phimax = atan(Qthickness / Mirradius); // cca 0.2 deg
      double   thetamax = atan(Qwidth / 2.0 / Mirradius); // cca 2deg


      double   pSPhi = M_PI / 2.0 - phimax * 0.5;
      double   pDPhi = phimax * 2.0;
      double   pSTheta = M_PI / 2.0 - thetamax * 1.5;
      double   pDTheta = 3 * thetamax ;

      G4Sphere* mirsphere = new G4Sphere("mirror_sphere_segment",
                                         pRmin, pRmax, pSPhi, pDPhi, pSTheta, pDTheta);

      G4RotationMatrix rotmir(M_PI / 2.0 + phimax, 0, 0);
      G4ThreeVector transmir(-Mirradius + length / 2.0, Mirposy, Mirposx);

      G4IntersectionSolid* mirrorshape = new G4IntersectionSolid("bar*mirsphere", bar, mirsphere, G4Transform3D(rotmir, transmir));


      /*  Position mirror  */

      G4LogicalVolume* mirror = new G4LogicalVolume(mirrorshape, quartzMaterial, "mirror");

      // define optical surface of the mirror
      const auto& mirrorSurface = geoModule.getMirrorSegment().getCoatingSurface();
      if (mirrorSurface.hasProperties()) {
        Materials& materials = Materials::getInstance();
        G4OpticalSurface* optSurf = materials.createOpticalSurface(mirrorSurface);
        new G4LogicalSkinSurface("mirrorsSurface", mirror, optSurf);
      } else {
        B2WARNING("TOP mirror: no surface properties defined - no photon reflection!");
      }

      G4Transform3D dumb = G4Translate3D(0, 0, 0);
      new G4PVPlacement(dumb, mirror, "TOP.mirror", qbar, false, 1);


      /*  Build glue joints and assign material  */

      // for quartz qluing
      G4Box* box2 = new G4Box("glue_2", Gwidth2 / 2.0, Qthickness / 2.0, Qwidth / 2.0);
      G4Box* box3 = new G4Box("glue_3", Gwidth3 / 2.0, Qthickness / 2.0, Qwidth / 2.0);

      G4LogicalVolume* gbox2 = new G4LogicalVolume(box2, glueMaterial, "gbox2");
      G4LogicalVolume* gbox3 = new G4LogicalVolume(box3, glueMaterial, "gbox3");

      // color the glue
      setColor(*gbox2, "rgb(0.0,1.0,1.0,1.0)");
      setColor(*gbox3, "rgb(0.0,1.0,1.0,1.0)");

      /*  Place glue joints */

      G4Transform3D tglue2 = G4Translate3D(-length / 2.0 + Length1 + Gwidth2 / 2.0 , 0, 0);
      G4Transform3D tglue3 = G4Translate3D(-length / 2.0 + Length1 + Gwidth2 + Length2 + Gwidth3 / 2.0 , 0, 0);

      new G4PVPlacement(tglue2, gbox2, "TOP.gbox2", qbar, false, 1);
      new G4PVPlacement(tglue3, gbox3, "TOP.gbox3", qbar, false, 1);

      /*  Build wedge  */

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

      G4ExtrudedSolid* wedge = new G4ExtrudedSolid("wedge",
                                                   polygon,
                                                   Wwidth / 2.0,
                                                   G4TwoVector(0.0, 0.0),
                                                   1.0,
                                                   G4TwoVector(0.0, 0.0),
                                                   1.0);

      G4LogicalVolume* qwedge = new G4LogicalVolume(wedge, quartzMaterial, "qwedge");

      //color the glue
      setColor(*qwedge, "rgb(0.0,0.75,1.0,1.0)");

      /* Add glue to wedge */

      double brokenFraction = m_topgp->getBrokenFraction();

      if (brokenFraction <= 0) {
        // for quartz gluing
        G4Box* box1 = new G4Box("glue_1", Gwidth3 / 2.0, Qthickness / 2.0, Wwidth / 2.0);
        G4LogicalVolume* gbox1 = new G4LogicalVolume(box1, glueMaterial, "gbox1");

        // color the glue
        setColor(*gbox1, "rgb(0.0,1.0,1.0,1.0)");

        // place the glue
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


      // for PMT gluing

      G4Box* box4 = new G4Box("glue_4", dGlue / 2.0, (Qthickness + Wextdown) / 2.0,
                              Wwidth / 2.0);
      G4LogicalVolume* gbox4 = new G4LogicalVolume(box4, gluePMTMaterial, "PMTglue");

      //color the glue
      setColor(*gbox4, "rgb(0.0,1.0,1.0,1.0)");

      //place the glue
      G4Transform3D tglue4 = G4Translate3D(-length / 2.0 - Gwidth1 - WLength - dGlue / 2.0,
                                           -Wextdown / 2.0,
                                           0);

      new G4PVPlacement(tglue4, gbox4, "TOP.gbox4", qwedge, false, 1);

      /* Put all segment into the Assembly volume */

      G4ThreeVector trnsasem(length / 2.0 + Bposition, 0, 0);

      G4AssemblyVolume* assemblyDetector = new G4AssemblyVolume();
      Simulation::RunManager::Instance().addAssemblyVolume(assemblyDetector);
      assemblyDetector->AddPlacedVolume(qbar, trnsasem, 0);
      assemblyDetector->AddPlacedVolume(qwedge, trnsasem, 0);

      // get the PMT stack and position it
      G4LogicalVolume* stack = buildPMTstack(geo.getPMTArray());

      double pmtSizez = geo.getPMTArray().getPMT().getSizeZ();
      double dz = Bposition - WLength - Gwidth1 - dGlue - pmtSizez / 2.0;
      double dx = (-Wextdown) / 2.0;

      G4RotationMatrix* rotsta = new G4RotationMatrix(M_PI / 2.0, -M_PI / 2.0, -M_PI / 2.0);
      G4ThreeVector trnssta(dz, dx, 0);

      assemblyDetector->AddPlacedVolume(stack, trnssta, rotsta);
      delete rotsta; // AddPlacedVolume() made an internal copy of the rotation matrix

      return assemblyDetector;
    }



    G4LogicalVolume* GeoTOPCreator::buildPMTstack(const TOPGeoPMTArray& geo)
    {
      // define material
      string materialName = geo.getMaterial();
      G4Material* material = Materials::get(materialName);
      if (!material) { B2FATAL("Material '" << materialName << "' missing!");}

      // define array volume
      G4Box* box = new G4Box("PMTarray", geo.getSizeX() / 2, geo.getSizeY() / 2,
                             geo.getPMT().getSizeZ() / 2);
      G4LogicalVolume* array = new G4LogicalVolume(box, material, "PMTarray");

      // Build one PMT
      G4LogicalVolume* PMT = buildPMT(geo.getPMT());

      // Place PMT's inside array volume
      for (unsigned row = 1; row <= geo.getNumRows(); row++) {
        for (unsigned col = 1; col <= geo.getNumColumns(); col++) {
          G4Transform3D tpmt = G4Translate3D(geo.getX(col), geo.getY(row), 0);
          new G4PVPlacement(tpmt, PMT, "TOP.PMT", array, false, geo.getPmtID(row, col));
        }
      }

      return array;
    }




    G4LogicalVolume* GeoTOPCreator::buildPMT(const TOPGeoPMT& geo)
    {

      /* Get PMT dimensions */

      double Msizex = geo.getSizeX();
      double Msizey = geo.getSizeY();
      double Msizez = geo.getSizeZ();
      double WThickness = geo.getWallThickness();
      double Asizex = geo.getSensSizeX();
      double Asizey = geo.getSensSizeY();
      double Asizez = geo.getSensThickness();
      double Winthickness = geo.getWinThickness();
      double Botthickness = geo.getBotThickness();
      double MWsizez = Msizez - Botthickness - Winthickness;

      /* Get PMT material */

      string caseName = geo.getWallMaterial();
      string sensName = geo.getSensMaterial();
      string winName = geo.getWinMaterial();
      string botName = geo.getBotMaterial();
      string fillName = geo.getFillMaterial();

      G4Material* caseMaterial = Materials::get(caseName);
      G4Material* sensMaterial = Materials::get(sensName);
      G4Material* winMaterial = Materials::get(winName);
      G4Material* botMaterial = Materials::get(botName);
      G4Material* fillMaterial = Materials::get(fillName);

      if (!caseMaterial) { B2FATAL("Material '" << caseName << "' missing!");}
      if (!sensMaterial) { B2FATAL("Material '" << sensName << "' missing!");}
      if (!winMaterial) { B2FATAL("Material '" << winName << "' missing!");}
      if (!botMaterial) { B2FATAL("Material '" << botName << "' missing!");}
      if (!fillMaterial) { B2FATAL("Material '" << fillName << "' missing!");}

      /* Build PMT segments */

      // The whole PMT box in which everything will be placed
      G4Box* box = new G4Box("box", Msizex / 2., Msizey / 2., Msizez / 2.);
      G4LogicalVolume* PMT = new G4LogicalVolume(box, caseMaterial, "PMT");

      // The window of the PMT
      G4Box* box1 = new G4Box("box1", Msizex / 2., Msizey / 2., Winthickness / 2.);
      G4LogicalVolume* window = new G4LogicalVolume(box1, winMaterial, "window");

      // The sensitive area of the PMT (placed into window bottom)
      G4Box* sensBox = new G4Box("sensbox", Asizex / 2., Asizey / 2., Asizez  / 2.);
      G4LogicalVolume* lmoduleSens = new G4LogicalVolume(sensBox, sensMaterial,
                                                         "SensitivePMT");
      lmoduleSens->SetSensitiveDetector(m_sensitivePMT); // Activate sensitive area
      setColor(*lmoduleSens, "rgb(0.0,0.0,0.0,1.0)");
      G4Transform3D tsens = G4Translate3D(0 , 0, -(Winthickness - Asizez) / 2.);
      new G4PVPlacement(tsens, lmoduleSens, "TOP.moduleSensitive", window, false, 1);

      // The PMT bottom
      G4Box* box2 = new G4Box("box2", Msizex / 2., Msizey / 2., Botthickness / 2.);
      G4LogicalVolume* bottom = new G4LogicalVolume(box2, botMaterial, "bottom");

      // The vacuum gap inside the PMT
      G4Box* box3 = new G4Box("box3", Msizex / 2. - WThickness,
                              Msizey / 2. - WThickness , (MWsizez - WThickness) / 2.);
      G4LogicalVolume* vacuum = new G4LogicalVolume(box3, fillMaterial, "vacuum");

      // Reflective edge
      double reflEdgeWidth = geo.getReflEdgeWidth();
      double reflEdgeThick = geo.getReflEdgeThickness();

      G4Box* box4 = new G4Box("box4", Msizex / 2, Msizey / 2, reflEdgeThick / 2);
      G4LogicalVolume* reflEdge = new G4LogicalVolume(box4, caseMaterial, "reflEdge");
      G4Box* box5 = new G4Box("box5", Msizex / 2 - reflEdgeWidth,
                              Msizey / 2 - reflEdgeWidth, reflEdgeThick / 2.);
      G4LogicalVolume* reflHole = new G4LogicalVolume(box5, fillMaterial, "vacuum");
      G4Transform3D temp = G4Translate3D(0.0, 0.0, 0.0);
      new G4PVPlacement(temp, reflHole, "TOP.vacuum", reflEdge, false, 1);

      auto& materials = Materials::getInstance();
      auto* optSurf = materials.createOpticalSurface(geo.getReflEdgeSurface());
      new G4LogicalSkinSurface("reflectiveEdgeSurface", reflEdge, optSurf);

      /* Place PMT segments */

      // Calculate the shifts need for the composition of the PMT
      G4Transform3D twin = G4Translate3D(0 , 0, (Msizez - Winthickness) / 2.);
      G4Transform3D tbot = G4Translate3D(0 , 0, (-Msizez + Botthickness) / 2.);
      G4Transform3D tvacuum = G4Translate3D(0 , 0,
                                            (Msizez - 2 * Winthickness - 2 * WThickness
                                             - MWsizez + WThickness) / 2.);
      G4Transform3D trefl = G4Translate3D(0 , 0,
                                          (Msizez - 2 * Winthickness - reflEdgeThick)
                                          / 2.);

      // Apply coloring and place all parts into PMT

      setColor(*window, "rgb(0.776,0.886,1.0,1.0)");
      new G4PVPlacement(twin, window, "TOP.window", PMT, false, 1);

      setColor(*bottom, "rgb(1.0,1.0,1.0,1.0)");
      new G4PVPlacement(tbot, bottom, "TOP.bottom", PMT, false, 1);

      new G4PVPlacement(tvacuum, vacuum, "TOP.vacuum", PMT, false, 1);

      new G4PVPlacement(trefl, reflEdge, "TOP.reflEdge", PMT, false, 1);

      setColor(*PMT, "rgb(0.47,0.47,0.47,1.0)");

      // One piece of PMT is finished

      return PMT;
    }

  }
}
