/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 * Major revision: 2016                                                   *
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
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <top/simulation/SensitivePMT.h>
#include <top/simulation/SensitiveBar.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include <simulation/kernel/RunManager.h>

#include <cmath>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4OpticalSurface.hh>

#include <G4Box.hh>
#include <G4UserLimits.hh>
#include <G4Material.hh>
#include <G4ExtrudedSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Sphere.hh>
#include <G4IntersectionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Colour.hh>
#include <G4TwoVector.hh>
#include <G4ThreeVector.hh>

#include <sstream>

using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace TOP {

    /** Register the creator */
    geometry::CreatorFactory<GeoTOPCreator> GeoTOPFactory("TOPCreator");


    GeoTOPCreator::GeoTOPCreator()
    {}


    GeoTOPCreator::~GeoTOPCreator()
    {
      if (m_sensitivePMT) delete m_sensitivePMT;
      if (m_sensitiveBar) delete m_sensitiveBar;
      if (m_sensitivePCB1) delete m_sensitivePCB1;
      if (m_sensitivePCB2) delete m_sensitivePCB2;
      G4LogicalSkinSurface::CleanSurfaceTable();
    }


    void GeoTOPCreator::create(const GearDir& content, G4LogicalVolume& topVolume,
                               geometry::GeometryTypes type)
    {

      m_isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      m_topgp->Initialize(content);
      if (!m_topgp->isValid()) {
        B2ERROR("TOP: geometry or mappers not valid (gearbox) - geometry not created");
        return;
      }

      const auto* geo = m_topgp->getGeometry();
      createGeometry(*geo, topVolume, type);

    }


    void GeoTOPCreator::createPayloads(const GearDir& content,
                                       const IntervalOfValidity& iov)
    {
      m_topgp->Initialize(content);
      if (!m_topgp->isValid()) {
        B2ERROR("TOP: geometry or mappers not valid (gearbox) - no payloads imported");
        return;
      }

      DBImportObjPtr<TOPGeometry> importObj;
      const auto* geo = m_topgp->getGeometry();
      importObj.construct(*geo);
      importObj.import(iov);

      m_topgp->getChannelMapper().import(iov);
      m_topgp->getFrontEndMapper().import(iov);

      B2RESULT("TOP: geometry and mappers imported to database");

    }


    void GeoTOPCreator::createFromDB(const std::string& name, G4LogicalVolume& topVolume,
                                     geometry::GeometryTypes type)
    {

      m_topgp->Initialize();
      if (!m_topgp->isValid()) {
        B2ERROR("Cannot create Geometry from Database: no configuration found for "
                << name);
        return;
      }

      const auto* geo = m_topgp->getGeometry();
      createGeometry(*geo, topVolume, type);

    }


    void GeoTOPCreator::createGeometry(const TOPGeometry& geo,
                                       G4LogicalVolume& topVolume,
                                       geometry::GeometryTypes)
    {
      m_sensitivePMT = new SensitivePMT();
      m_sensitiveBar = new SensitiveBar();

      m_sensitivePMT->setModuleReplicaDepth(4);
      m_sensitiveBar->setReplicaDepth(1);

      geo.useGeantUnits();

      double backwardLength = geo.getQBB().getPrismEnclosure().getLength();
      double prismPosition = geo.getQBB().getPrismPosition();

      for (const auto& geoModule : geo.getModules()) {
        int moduleID = geoModule.getModuleID();
        double barLength = geoModule.getBarLength();

        // go to local (bar) frame

        G4Transform3D T1 =
          G4TranslateZ3D(backwardLength / 2 - prismPosition - barLength / 2);

        // displaced geometry

        const auto& displacement = geoModule.getModuleDisplacement();
        G4Transform3D dRx = G4RotateX3D(displacement.getAlpha());
        G4Transform3D dRy = G4RotateY3D(displacement.getBeta());
        G4Transform3D dRz = G4RotateZ3D(displacement.getGamma());
        G4Transform3D dtr = G4Translate3D(displacement.getX(),
                                          displacement.getY(),
                                          displacement.getZ());
        G4Transform3D D = dtr * dRz * dRy * dRx;

        // position the module

        double radius = geoModule.getRadius();
        double backwardZ = geoModule.getBackwardZ();
        G4Transform3D tr = G4Translate3D(0, radius, barLength / 2 + backwardZ);
        double phi = geoModule.getPhi() - M_PI / 2;
        G4Transform3D Rz = G4RotateZ3D(phi);
        G4Transform3D T = Rz * tr * D * T1;
        auto* module = createModule(geo, moduleID);
        std::string name = geoModule.getName();
        new G4PVPlacement(T, module, name, &topVolume, false, moduleID);
      }

      if (m_numDecoupledPMTs > 0) B2WARNING("GeoTOPCreator, " << m_numDecoupledPMTs
                                              << " PMT's are optically decoupled");
      if (m_numBrokenGlues > 0) B2WARNING("GeoTOPCreator, " << m_numBrokenGlues
                                            << " broken glues");
      if (m_numPeelOffRegions > 0) B2WARNING("GeoTOPCreator, " << m_numPeelOffRegions
                                               << " peel-off cookie regions");
    }


    G4LogicalVolume* GeoTOPCreator::createModule(const TOPGeometry& geo, int moduleID)
    {
      // note: z = 0 is at center of backward envelope

      G4ThreeVector move;
      G4RotationMatrix rot;

      // create module envelope volume

      const auto& geoQBB = geo.getQBB();
      auto* module = createModuleEnvelope(geoQBB, moduleID);

      // add quartz optics together with PMT array

      const auto& geoModule = geo.getModule(moduleID);
      auto* optics = assembleOptics(geoModule);

      move.setZ(geoQBB.getPrismPosition() - geoQBB.getPrismEnclosure().getLength() / 2);
      optics->MakeImprint(module, move, &rot);

      // add front-end electronics

      if (!m_frontEnd) {
        m_frontEnd = assembleFrontEnd(geo.getFrontEnd(), geo.getNumBoardStacks());
      }
      double Lprism = geoModule.getPrism().getFullLength();
      double Larray = geoModule.getPMTArray().getSizeZ();
      move.setZ(move.z() - Lprism - Larray);
      m_frontEnd->MakeImprint(module, move, &rot);

      // add QBB

      if (!m_qbb) m_qbb = assembleQBB(geoQBB);
      move.setZ(0);
      m_qbb->MakeImprint(module, move, &rot);

      return module;
    }


    G4LogicalVolume* GeoTOPCreator::createModuleEnvelope(const TOPGeoQBB& geo,
                                                         int moduleID)
    {
      // note: z = 0 is at center of backward envelope

      if (!m_moduleEnvelope) {
        double backwardLength = geo.getPrismEnclosure().getLength();
        double forwardLength = geo.getLength() - backwardLength;
        std::vector<G4TwoVector> polygon;
        for (auto& point : geo.getBackwardContour()) {
          polygon.push_back(G4TwoVector(point.first, point.second));
        }
        auto* backward = new G4ExtrudedSolid("backwardEnvelope",
                                             polygon, backwardLength / 2,
                                             G4TwoVector(), 1, G4TwoVector(), 1);
        polygon.clear();
        for (auto& point : geo.getForwardContour()) {
          polygon.push_back(G4TwoVector(point.first, point.second));
        }
        auto* forward = new G4ExtrudedSolid("forwardEnvelope",
                                            polygon, forwardLength / 2,
                                            G4TwoVector(), 1, G4TwoVector(), 1);

        G4Transform3D move = G4TranslateZ3D((backwardLength + forwardLength) / 2);
        m_moduleEnvelope = new G4UnionSolid("moduleEnvelope", backward, forward, move);
      }

      G4Material* material = Materials::get(geo.getMaterial());
      if (!material) B2FATAL("Material '" << geo.getMaterial() << "' not found");

      std::string name = addNumber("TOPEnvelopeModule", moduleID);
      return new G4LogicalVolume(m_moduleEnvelope, material, name);

    }


    G4AssemblyVolume* GeoTOPCreator::assembleFrontEnd(const TOPGeoFrontEnd& geo, int N)
    {
      auto* frontEnd = new G4AssemblyVolume();
      Simulation::RunManager::Instance().addAssemblyVolume(frontEnd);

      G4ThreeVector move;

      // front board

      double Z = -geo.getFrontBoardGap();
      double length = geo.getFrontBoardThickness();
      auto* frontBoard = createBox("TOPFrontBoard",
                                   geo.getFrontBoardWidth(),
                                   geo.getFrontBoardHeight(),
                                   length,
                                   geo.getFrontBoardMaterial());
      if (m_isBeamBkgStudy) {
        if (!m_sensitivePCB1) m_sensitivePCB1 = new BkgSensitiveDetector("TOP", 1);
        frontBoard->SetSensitiveDetector(m_sensitivePCB1);
      }
      move.setZ(Z - length / 2);
      move.setY(geo.getFrontBoardY());
      frontEnd->AddPlacedVolume(frontBoard, move, 0);
      Z -= length;

      // HV board

      length = geo.getHVBoardLength();
      auto* HVBoard = createBox("TOPHVBoard",
                                geo.getHVBoardWidth(),
                                geo.getHVBoardThickness(),
                                length,
                                geo.getHVBoardMaterial());
      if (m_isBeamBkgStudy) {
        if (!m_sensitivePCB2) m_sensitivePCB2 = new BkgSensitiveDetector("TOP", 2);
        HVBoard->SetSensitiveDetector(m_sensitivePCB2);
      }
      move.setZ(Z  - geo.getHVBoardGap() - length / 2);
      move.setY(geo.getHVBoardY());
      frontEnd->AddPlacedVolume(HVBoard, move, 0);

      // board stack

      length = geo.getBoardStackLength();
      move.setZ(Z  - geo.getBoardStackGap() - length / 2);
      move.setY(geo.getBoardStackY());
      auto* boardStack = createBoardStack(geo, N);
      frontEnd->AddPlacedVolume(boardStack, move, 0);

      return frontEnd;
    }


    G4LogicalVolume* GeoTOPCreator::createBoardStack(const TOPGeoFrontEnd& geo, int N)
    {
      double width = geo.getBoardStackWidth();
      double fullWidth = width * N;
      std::string name = "TOPBoardStack";
      auto* boardStack = createBox(name,
                                   fullWidth,
                                   geo.getBoardStackHeight(),
                                   geo.getBoardStackLength(),
                                   geo.getBoardStackMaterial());

      double spacerWidth = geo.getSpacerWidth();
      std::string name1 = name + "Spacer";
      auto* spacer = createBox(name1,
                               spacerWidth,
                               geo.getBoardStackHeight(),
                               geo.getBoardStackLength(),
                               geo.getSpacerMaterial());

      std::string name2 = name + "TwoSpacers";
      auto* twoSpacers = createBox(name2,
                                   spacerWidth * 2,
                                   geo.getBoardStackHeight(),
                                   geo.getBoardStackLength(),
                                   geo.getSpacerMaterial());

      G4Transform3D move;
      move = G4TranslateX3D(-(fullWidth - spacerWidth) / 2);
      new G4PVPlacement(move, spacer, name1, boardStack, false, 1);
      move = G4TranslateX3D((fullWidth - spacerWidth) / 2);
      new G4PVPlacement(move, spacer, name1, boardStack, false, 2);

      int n = N - 1;
      for (int i = 0; i < n; i++) {
        move = G4TranslateX3D(width * (2 * i - n + 1) / 2.0);
        new G4PVPlacement(move, twoSpacers, name2, boardStack, false, i + 1);
      }

      return boardStack;
    }


    G4AssemblyVolume* GeoTOPCreator::assembleQBB(const TOPGeoQBB& geo)
    {
      auto* qbb = new G4AssemblyVolume();
      Simulation::RunManager::Instance().addAssemblyVolume(qbb);

      double Zback = -geo.getPrismEnclosure().getLength() / 2;
      double Zfront = Zback + geo.getLength() - geo.getForwardEndPlate().getThickness();

      G4ThreeVector move;

      // outer panel

      auto* outerPanel = createHoneycombPanel(geo, c_Outer);
      move.setZ(Zfront - geo.getOuterPanel().getLength() / 2);
      qbb->AddPlacedVolume(outerPanel, move, 0);

      // inner panel

      auto* innerPanel = createHoneycombPanel(geo, c_Inner);
      move.setZ(Zfront - geo.getInnerPanel().getLength() / 2);
      qbb->AddPlacedVolume(innerPanel, move, 0);

      // forward end plate

      double length = geo.getForwardEndPlate().getThickness();
      auto* forwardEndPlate = createBox(geo.getForwardEndPlate().getName(),
                                        geo.getWidth(),
                                        geo.getForwardEndPlate().getHeight(),
                                        length,
                                        geo.getForwardEndPlate().getMaterial());
      move.setZ(Zfront + length / 2);
      qbb->AddPlacedVolume(forwardEndPlate, move, 0);

      // prism enclosure box

      std::string name = geo.getPrismEnclosure().getName();
      double Z = Zback;
      length = geo.getPrismEnclosure().getBackThickness();
      auto* backPlate = createExtrudedSolid(name + "BackPlate",
                                            geo.getBackPlateContour(),
                                            length,
                                            geo.getPrismEnclosure().getMaterial());
      move.setZ(Z + length / 2);
      qbb->AddPlacedVolume(backPlate, move, 0);
      Z += length;

      length = geo.getPrismEnclosure().getBodyLength();
      auto* prismEnclosure = createExtrudedSolid(name + "Body",
                                                 geo.getPrismEnclosureContour(),
                                                 length,
                                                 geo.getPrismEnclosure().getMaterial());
      move.setZ(Z + length / 2);
      qbb->AddPlacedVolume(prismEnclosure, move, 0);
      Z += length;

      length = geo.getPrismEnclosure().getFrontThickness();
      auto* frontPlate = createExtrudedSolid(name + "FrontPlate",
                                             geo.getFrontPlateContour(),
                                             length,
                                             geo.getPrismEnclosure().getMaterial());
      move.setZ(Z + length / 2);
      qbb->AddPlacedVolume(frontPlate, move, 0);
      Z += length;

      length = Zfront - Z - geo.getInnerPanel().getLength();
      if (length > 0) {
        double height = geo.getPrismEnclosure().getExtensionThickness();
        auto* extPlate = createBox(name + "ExtensionPlate",
                                   geo.getPanelWidth(),
                                   height,
                                   length,
                                   geo.getPrismEnclosure().getMaterial());

        G4ThreeVector movePlate;
        movePlate.setZ(Z + length / 2);
        movePlate.setY((height - geo.getSideRails().getHeight()) / 2);
        qbb->AddPlacedVolume(extPlate, movePlate, 0);
      }

      // side rails

      auto* leftRail = createSideRail(geo, c_Left);
      move.setZ(Zfront - geo.getSideRailsLength() / 2);
      move.setY(geo.getOuterPanel().getY() + geo.getOuterPanel().getMinThickness() -
                geo.getSideRails().getHeight() / 2);
      move.setX(-(geo.getWidth() - geo.getSideRails().getThickness()) / 2);
      qbb->AddPlacedVolume(leftRail, move, 0);

      auto* rightRail = createSideRail(geo, c_Right);
      move.setX((geo.getWidth() - geo.getSideRails().getThickness()) / 2);
      qbb->AddPlacedVolume(rightRail, move, 0);

      // cold plate

      length = geo.getLength() - geo.getPrismEnclosure().getBackThickness() -
               geo.getOuterPanel().getLength() - geo.getForwardEndPlate().getThickness();
      auto* coldPlateBase = createBox(geo.getColdPlate().getName() + "Base",
                                      geo.getPanelWidth(),
                                      geo.getColdPlate().getBaseThickness(),
                                      length,
                                      geo.getColdPlate().getBaseMaterial());
      move.setZ(Zback + geo.getPrismEnclosure().getBackThickness() + length / 2);
      move.setY(geo.getOuterPanel().getY() + geo.getOuterPanel().getMinThickness() -
                geo.getColdPlate().getBaseThickness() / 2);
      move.setX(0);
      qbb->AddPlacedVolume(coldPlateBase, move, 0);

      auto* coldPlateCool = createBox(geo.getColdPlate().getName() + "Cooler",
                                      geo.getColdPlate().getCoolWidth(),
                                      geo.getColdPlate().getCoolThickness(),
                                      length,
                                      geo.getColdPlate().getCoolMaterial());
      move.setY(geo.getOuterPanel().getY() + geo.getOuterPanel().getMinThickness() +
                geo.getColdPlate().getCoolThickness() / 2);
      qbb->AddPlacedVolume(coldPlateCool, move, 0);

      return qbb;
    }


    G4LogicalVolume* GeoTOPCreator::createHoneycombPanel(const TOPGeoQBB& geo,
                                                         EPanelType type)
    {
      G4Transform3D move;

      TOPGeoHoneycombPanel geoPanel;
      Polygon contour;
      double sideEdgeHeight = 0;
      double sideEdgeY = 0;

      if (type == c_Inner) {
        geoPanel = geo.getInnerPanel();
        contour = geo.getInnerPanelContour();
        sideEdgeHeight = geoPanel.getMaxThickness();
        sideEdgeY = geoPanel.getY() - sideEdgeHeight / 2;
      } else {
        geoPanel = geo.getOuterPanel();
        contour = geo.getOuterPanelContour();
        sideEdgeHeight = geoPanel.getMinThickness();
        sideEdgeY = geoPanel.getY() + sideEdgeHeight / 2;
      }

      // honeycomb panel

      auto* panel = createExtrudedSolid(geoPanel.getName(),
                                        contour,
                                        geoPanel.getLength(),
                                        geoPanel.getMaterial());

      // reinforced faces

      std::string faceEdgeName = geoPanel.getName() + "ReinforcedFace";
      auto* faceEdge = createExtrudedSolid(faceEdgeName,
                                           contour,
                                           geoPanel.getEdgeWidth(),
                                           geoPanel.getEdgeMaterial());

      move = G4TranslateZ3D((geoPanel.getLength() - geoPanel.getEdgeWidth()) / 2);
      new G4PVPlacement(move, faceEdge, faceEdgeName, panel, false, 1);
      move = G4TranslateZ3D(-(geoPanel.getLength() - geoPanel.getEdgeWidth()) / 2);
      new G4PVPlacement(move, faceEdge, faceEdgeName, panel, false, 2);

      // reinforced sides

      double width = geo.getPanelWidth();
      double sideEdgeWidth = geoPanel.getEdgeWidth() + (width - geoPanel.getWidth()) / 2;
      std::string sideEdgeName = geoPanel.getName() + "ReinforcedSide";
      auto* sideEdge = createBox(sideEdgeName,
                                 sideEdgeWidth,
                                 sideEdgeHeight,
                                 geoPanel.getLength() - 2 * geoPanel.getEdgeWidth(),
                                 geoPanel.getEdgeMaterial());


      move = G4Translate3D((width - sideEdgeWidth) / 2, sideEdgeY, 0);
      new G4PVPlacement(move, sideEdge, sideEdgeName, panel, false, 1);
      move = G4Translate3D(-(width - sideEdgeWidth) / 2, sideEdgeY, 0);
      new G4PVPlacement(move, sideEdge, sideEdgeName, panel, false, 2);

      return panel;
    }


    G4LogicalVolume* GeoTOPCreator::createSideRail(const TOPGeoQBB& geo,
                                                   ESideRailType type)
    {
      double A = geo.getSideRails().getThickness();
      double B = geo.getSideRails().getHeight();
      double C = geo.getSideRailsLength();
      double a = A - geo.getSideRails().getReducedThickness();
      double b = B - geo.getColdPlate().getBaseThickness();
      double c = geo.getPrismEnclosure().getLength() -
                 geo.getPrismEnclosure().getBackThickness();

      auto* box = new G4Box("box", A / 2, B / 2, C / 2);
      auto* subtrBox = new G4Box("subtrBox", a / 2, b / 2, c / 2);
      double x = 0;
      if (type == c_Left) {
        x = (A - a) / 2;
      } else {
        x = -(A - a) / 2;
      }
      G4Transform3D move = G4Translate3D(x, -(B - b) / 2, -(C - c) / 2);
      auto* solid = new G4SubtractionSolid("sideRail", box, subtrBox, move);

      G4Material* material = Materials::get(geo.getSideRails().getMaterial());
      if (!material) B2FATAL("Material '" << geo.getSideRails().getMaterial() <<
                               "' not found");

      std::string name = geo.getSideRails().getName();
      if (type == c_Left) {
        name += "Left";
      } else {
        name += "Rigth";
      }

      return new G4LogicalVolume(solid, material, name);
    }


    G4AssemblyVolume* GeoTOPCreator::assembleOptics(const TOPGeoModule& geo)
    {
      auto* optics = new G4AssemblyVolume();
      Simulation::RunManager::Instance().addAssemblyVolume(optics);

      double Lm = geo.getMirrorSegment().getFullLength();
      double L1 = geo.getBarSegment1().getFullLength();
      double L2 = geo.getBarSegment2().getFullLength();
      double Lp = geo.getPrism().getFullLength();
      double La = geo.getPMTArray().getSizeZ();

      // note: z = 0 is at prism-bar joint

      auto* pmtArray = createPMTArray(geo.getPMTArray(), geo.getModuleID());
      double Dy = (geo.getPrism().getThickness() - geo.getPrism().getExitThickness()) / 2;
      G4ThreeVector moveArray(geo.getPMTArrayDisplacement().getX(),
                              geo.getPMTArrayDisplacement().getY() + Dy,
                              -(Lp + La / 2));
      G4RotationMatrix rotArray;
      rotArray.rotateZ(geo.getPMTArrayDisplacement().getAlpha());
      optics->AddPlacedVolume(pmtArray, moveArray, &rotArray);

      G4ThreeVector move;
      G4RotationMatrix rot;

      auto* prism = createPrism(geo.getPrism(), geo.getModuleID());
      move.setZ(0);
      rot.rotateY(M_PI / 2);
      optics->AddPlacedVolume(prism, move, &rot);

      auto* barSegment2 = createBarSegment(geo.getBarSegment2(), geo.getModuleID());
      move.setZ(L2 / 2);
      optics->AddPlacedVolume(barSegment2, move, 0);

      auto* barSegment1 = createBarSegment(geo.getBarSegment1(), geo.getModuleID());
      move.setZ(L2 + L1 / 2);
      optics->AddPlacedVolume(barSegment1, move, 0);

      auto* mirrorSegment = createMirrorSegment(geo.getMirrorSegment(),
                                                geo.getModuleID());
      move.setZ(L2 + L1 + Lm / 2);
      optics->AddPlacedVolume(mirrorSegment, move, 0);

      return optics;
    }


    G4LogicalVolume* GeoTOPCreator::createBarSegment(const TOPGeoBarSegment& geo,
                                                     int moduleID)
    {
      G4Transform3D move;

      // mother volume
      auto* bar = createBox(geo.getName(),
                            geo.getWidth(), geo.getThickness(), geo.getFullLength(),
                            geo.getMaterial());
      // glue
      auto* glue = createBox(geo.getName() + "Glue",
                             geo.getWidth(), geo.getThickness(), geo.getGlueThickness(),
                             geo.getGlueMaterial());
      if (geo.getBrokenGlueFraction() > 0) {
        auto* brokenGlue = createExtrudedSolid(geo.getName() + "BrokenGlue",
                                               geo.getBrokenGlueContour(),
                                               geo.getGlueThickness(),
                                               geo.getBrokenGlueMaterial());
        G4Transform3D fix;
        new G4PVPlacement(fix, brokenGlue, geo.getName() + "BrokenGlue", glue, false, 1);
        B2RESULT("GeoTOPCreator, broken glue at " << geo.getName()
                 << addNumber(" of Slot", moduleID));
        m_numBrokenGlues++;
      }

      // place glue to -z side
      move = G4TranslateZ3D(-(geo.getFullLength() - geo.getGlueThickness()) / 2);
      new G4PVPlacement(move, glue, geo.getName() + "Glue", bar, false, 1);

      // optical surface
      auto& materials = Materials::getInstance();
      auto* optSurf = materials.createOpticalSurface(geo.getSurface());
      optSurf->SetSigmaAlpha(geo.getSigmaAlpha());
      new G4LogicalSkinSurface("opticalSurface", bar, optSurf);

      // Activate sensitive volume
      bar->SetSensitiveDetector(m_sensitiveBar);

      return bar;
    }


    G4LogicalVolume* GeoTOPCreator::createMirrorSegment(const TOPGeoMirrorSegment& geo,
                                                        int moduleID)
    {
      G4Transform3D move;

      // box of the bar
      auto* box = new G4Box(geo.getName(),
                            geo.getWidth() / 2, geo.getThickness() / 2,
                            geo.getFullLength() / 2);

      // mother volume
      auto* bar = createBoxSphereIntersection(geo.getName(),
                                              box,
                                              0, geo.getOuterRadius(),
                                              geo.getXc(), geo.getYc(), geo.getZc(),
                                              geo.getMaterial());

      // glue
      auto* glue = createBox(geo.getName() + "Glue",
                             geo.getWidth(), geo.getThickness(), geo.getGlueThickness(),
                             geo.getGlueMaterial());
      if (geo.getBrokenGlueFraction() > 0) {
        auto* brokenGlue = createExtrudedSolid(geo.getName() + "BrokenGlue",
                                               geo.getBrokenGlueContour(),
                                               geo.getGlueThickness(),
                                               geo.getBrokenGlueMaterial());
        G4Transform3D fix;
        new G4PVPlacement(fix, brokenGlue, geo.getName() + "BrokenGlue", glue, false, 1);
        B2RESULT("GeoTOPCreator, broken glue at " << geo.getName()
                 << addNumber(" of Slot", moduleID));
        m_numBrokenGlues++;
      }

      // place glue to -z side
      move = G4TranslateZ3D(-(geo.getFullLength() - geo.getGlueThickness()) / 2);
      new G4PVPlacement(move, glue, geo.getName() + "Glue", bar, false, 1);

      // bar optical surface
      auto& materials = Materials::getInstance();
      auto* optSurf = materials.createOpticalSurface(geo.getSurface());
      optSurf->SetSigmaAlpha(geo.getSigmaAlpha());
      new G4LogicalSkinSurface("opticalSurface", bar, optSurf);

      // mirror reflective coating
      auto* mirror = createBoxSphereIntersection(geo.getName() + "ReflectiveCoating",
                                                 box,
                                                 geo.getRadius(), geo.getOuterRadius(),
                                                 geo.getXc(), geo.getYc(), geo.getZc(),
                                                 geo.getCoatingMaterial());

      // mirror optical surface
      auto* mirrorSurf = materials.createOpticalSurface(geo.getCoatingSurface());
      new G4LogicalSkinSurface("mirrorSurface", mirror, mirrorSurf);

      // place reflective coating
      move = G4TranslateZ3D(0);
      new G4PVPlacement(move, mirror, geo.getName() + "ReflectiveCoating", bar, false, 1);

      // Activate sensitive volume
      bar->SetSensitiveDetector(m_sensitiveBar);

      return bar;
    }


    G4LogicalVolume* GeoTOPCreator::createPrism(const TOPGeoPrism& geo, int moduleID)

    {
      G4Transform3D move;

      // mother volume
      std::vector<G4TwoVector> polygon;
      polygon.push_back(G4TwoVector(0, geo.getThickness() / 2));
      polygon.push_back(G4TwoVector(geo.getFullLength(), geo.getThickness() / 2));
      polygon.push_back(G4TwoVector(geo.getFullLength(),
                                    geo.getThickness() / 2 - geo.getExitThickness()));
      polygon.push_back(G4TwoVector(geo.getLength() - geo.getFlatLength(),
                                    geo.getThickness() / 2 - geo.getExitThickness()));
      polygon.push_back(G4TwoVector(0, -geo.getThickness() / 2));

      auto* volume = new G4ExtrudedSolid(geo.getName(), polygon, geo.getWidth() / 2,
                                         G4TwoVector(), 1, G4TwoVector(), 1);
      G4Material* material = Materials::get(geo.getMaterial());
      if (!material) B2FATAL("Material '" << geo.getMaterial() << "' not found");
      auto* prism = new G4LogicalVolume(volume, material, geo.getName());

      // wavelenght filter
      auto* filter = createBox(geo.getName() + "Filter",
                               geo.getFilterThickness(),
                               geo.getExitThickness(),
                               geo.getWidth(),
                               geo.getFilterMaterial());
      // place peel-off regions (if any) into filter
      int numRegions = 0;
      std::string message = addNumber("peel-off cookie regions of Slot", moduleID) + ":";
      for (const auto& region : geo.getPeelOffRegions()) {
        if (region.fraction <= 0) continue;
        std::string name = addNumber(geo.getName() + "PeelOff", region.ID);
        double thickness = geo.getPeelOffThickness();
        auto* peelOff = createExtrudedSolid(name,
                                            geo.getPeelOffContour(region),
                                            thickness,
                                            geo.getPeelOffMaterial());
        G4Transform3D T = G4Translate3D((geo.getFilterThickness() - thickness) / 2,
                                        0,
                                        geo.getPeelOffCenter(region));
        G4Transform3D R = G4RotateY3D(-M_PI / 2);
        G4Transform3D moveRegion = T * R;
        new G4PVPlacement(moveRegion, peelOff, name, filter, false, 1);
        message += addNumber(" ", region.ID);
        numRegions++;
      }
      if (numRegions > 0) B2RESULT("GeoTOPCreator, " << message);
      m_numPeelOffRegions += numRegions;

      // place filter to +x side
      move = G4Translate3D(geo.getFullLength() - geo.getFilterThickness() / 2,
                           (geo.getThickness() - geo.getExitThickness()) / 2,
                           0);
      new G4PVPlacement(move, filter, geo.getName() + "Filter", prism, false, 1);

      // optical surface
      auto& materials = Materials::getInstance();
      auto* optSurf = materials.createOpticalSurface(geo.getSurface());
      optSurf->SetSigmaAlpha(geo.getSigmaAlpha());
      new G4LogicalSkinSurface("opticalSurface", prism, optSurf);

      // Activate sensitive volume
      prism->SetSensitiveDetector(m_sensitiveBar);

      return prism;
    }


    G4LogicalVolume* GeoTOPCreator::createPMTArray(const TOPGeoPMTArray& geo,
                                                   int moduleID)
    {
      // mother volume
      auto* pmtArray = createBox(geo.getName(),
                                 geo.getSizeX(), geo.getSizeY(),
                                 geo.getSizeZ(),
                                 geo.getMaterial());

      // single PMT
      auto* pmt = createPMT(geo.getPMT());

      // place PMT's
      double halfGap = geo.getAirGap() / 2;
      std::string message = addNumber("optically decoupled PMT's of Slot", moduleID) + ":";
      for (unsigned row = 1; row <= geo.getNumRows(); row++) {
        for (unsigned col = 1; col <= geo.getNumColumns(); col++) {
          auto id = geo.getPmtID(row, col);
          double z = halfGap;
          if (geo.isPMTDecoupled(id)) {
            z = -halfGap;
            message += addNumber(" ", id);
            m_numDecoupledPMTs++;
          }
          G4Transform3D move = G4Translate3D(geo.getX(col), geo.getY(row), z);
          new G4PVPlacement(move, pmt, addNumber(geo.getPMT().getName(), id), pmtArray,
                            false, id);
        }
      }

      if (!geo.getDecoupledPMTs().empty()) B2RESULT("GeoTOPCreator, " << message);

      return pmtArray;
    }


    G4LogicalVolume* GeoTOPCreator::createPMT(const TOPGeoPMT& geo)
    {
      G4Transform3D move;

      // mother volume
      auto* pmt = createBox(geo.getName(),
                            geo.getSizeX(), geo.getSizeY(), geo.getSizeZ(),
                            geo.getWallMaterial());

      // window + photocathode + reflective edge
      double winThickness = geo.getWinThickness() + geo.getReflEdgeThickness();
      auto* window = createBox(geo.getName() + "Window",
                               geo.getSizeX(), geo.getSizeY(),
                               winThickness,
                               geo.getWinMaterial());
      auto* photoCathode = createBox(geo.getName() + "PhotoCathode",
                                     geo.getSensSizeX(), geo.getSensSizeY(),
                                     geo.getSensThickness(),
                                     geo.getSensMaterial());
      photoCathode->SetSensitiveDetector(m_sensitivePMT); // Activate sensitive area

      move = G4TranslateZ3D(-(winThickness - geo.getSensThickness()) / 2
                            + geo.getReflEdgeThickness());
      new G4PVPlacement(move, photoCathode, geo.getName() + "PhotoCathode", window,
                        false, 1);

      auto* reflEdge = createBox(geo.getName() + "ReflectiveEdge",
                                 geo.getSizeX(), geo.getSizeY(),
                                 geo.getReflEdgeThickness(),
                                 geo.getWallMaterial());
      double holeSizeX = geo.getSizeX() - 2 * geo.getReflEdgeWidth();
      double holeSizeY = geo.getSizeY() - 2 * geo.getReflEdgeWidth();
      if (holeSizeX > 0 and holeSizeY > 0) {
        auto* hole = createBox(geo.getName() + "ReflectiveEdgeHole",
                               holeSizeX, holeSizeY,
                               geo.getReflEdgeThickness(),
                               geo.getFillMaterial());
        move = G4TranslateZ3D(0.0);
        new G4PVPlacement(move, hole, geo.getName() + "ReflectiveEdgeHole", reflEdge,
                          false, 1);
      }

      auto& materials = Materials::getInstance();
      auto* optSurf = materials.createOpticalSurface(geo.getReflEdgeSurface());
      new G4LogicalSkinSurface("reflectiveEdgeSurface", reflEdge, optSurf);

      move = G4TranslateZ3D(-(winThickness - geo.getReflEdgeThickness()) / 2);
      new G4PVPlacement(move, reflEdge, geo.getName() + "ReflectiveEdge", window,
                        false, 1);

      move = G4TranslateZ3D((geo.getSizeZ() - winThickness) / 2);
      new G4PVPlacement(move, window, geo.getName() + "Window", pmt, false, 1);

      // bottom
      if (geo.getBotMaterial() != geo.getWallMaterial()) {
        auto* bottom = createBox(geo.getName() + "Bottom",
                                 geo.getSizeX(), geo.getSizeY(), geo.getBotThickness(),
                                 geo.getBotMaterial());
        move = G4TranslateZ3D(-(geo.getSizeZ() - geo.getBotThickness()) / 2);
        new G4PVPlacement(move, bottom, geo.getName() + "Bottom", pmt, false, 1);
      }

      // interior
      double interiorSizeZ = geo.getSizeZ() - winThickness - geo.getBotThickness();
      auto* interior = createBox(geo.getName() + "Interior",
                                 geo.getSizeX() - 2 * geo.getWallThickness(),
                                 geo.getSizeY() - 2 * geo.getWallThickness(),
                                 interiorSizeZ,
                                 geo.getBotMaterial());
      move = G4TranslateZ3D(-(geo.getSizeZ() - interiorSizeZ) / 2
                            + geo.getBotThickness());
      new G4PVPlacement(move, interior, geo.getName() + "Interior", pmt, false, 1);

      return pmt;
    }


    G4LogicalVolume* GeoTOPCreator::createBox(const std::string& name,
                                              double A, double B, double C,
                                              const std::string& materialName)
    {
      G4Box* box = new G4Box(name, A / 2, B / 2, C / 2);
      G4Material* material = Materials::get(materialName);
      if (!material) B2FATAL("Material '" << materialName << "' not found");
      return new G4LogicalVolume(box, material, name);
    }


    G4LogicalVolume*
    GeoTOPCreator::createBoxSphereIntersection(const std::string& name,
                                               G4Box* box,
                                               double Rmin,
                                               double Rmax,
                                               double xc,
                                               double yc,
                                               double zc,
                                               const std::string& materialName)
    {
      // determine max theta - note: not valid generically!
      double x = box->GetXHalfLength();
      double y = box->GetYHalfLength();
      double z = box->GetZHalfLength();
      double dx = fmax(fabs(-x - xc), fabs(x - xc));
      double dy = fmax(fabs(-y - yc), fabs(y - yc));
      double dz = fmin(-z - zc, z - zc);
      double theta = atan(sqrt(dx * dx + dy * dy) / dz);

      auto* sphere = new G4Sphere(name + "Sphere",
                                  Rmin, Rmax, 0, 2 * M_PI, 0, theta);
      G4Translate3D move(xc, yc, zc);
      auto* shape = new G4IntersectionSolid(name, box, sphere, move);

      G4Material* material = Materials::get(materialName);
      if (!material) B2FATAL("Material '" << materialName << "' not found");
      return new G4LogicalVolume(shape, material, name);
    }


    G4LogicalVolume* GeoTOPCreator::createExtrudedSolid(const std::string& name,
                                                        const Polygon& shape,
                                                        double length,
                                                        const std::string& materialName)
    {
      std::vector<G4TwoVector> polygon;
      for (auto& point : shape) {
        polygon.push_back(G4TwoVector(point.first, point.second));
      }
      G4ExtrudedSolid* solid = new G4ExtrudedSolid(name, polygon, length / 2,
                                                   G4TwoVector(), 1, G4TwoVector(), 1);
      G4Material* material = Materials::get(materialName);
      if (!material) B2FATAL("Material '" << materialName << "' not found");
      return new G4LogicalVolume(solid, material, name);
    }


    std::string GeoTOPCreator::addNumber(const std::string& str, unsigned number)
    {
      stringstream ss;
      if (number < 10) {
        ss << "0" << number;
      } else {
        ss << number;
      }
      string out;
      ss >> out;
      return str + out;
    }

  } // name space TOP
} // name space Belle2
