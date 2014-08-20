/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/geometry/GeoARICHCreator.h>
#include <arich/geometry/ARICHGeometryPar.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <arich/simulation/SensitiveDetector.h>
#include <arich/simulation/SensitiveAero.h>
#include <simulation/background/BkgSensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

// Geant4
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4AssemblyVolume.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4OpticalSurface.hh>
// Geant4 Shapes
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4SubtractionSolid.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace geometry;

  namespace arich {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoARICHCreator> GeoARICHFactory("ARICHCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoARICHCreator::GeoARICHCreator(): isBeamBkgStudy(0)
    {
      m_sensitive = new SensitiveDetector();
      m_sensitiveAero = new SensitiveAero();
    }

    GeoARICHCreator::~GeoARICHCreator()
    {
      delete m_sensitive;
      delete m_sensitiveAero;
    }


    void GeoARICHCreator::create(const GearDir& content, G4LogicalVolume& topVolume, GeometryTypes type)
    {

      if (type == 0) {
        B2INFO("Geometry type: Full")
      } else if (type == 1) {
        B2INFO("Geometry type: Tracking")
      } else if (type == 2) {
        B2INFO("Geometry type: Display")
      }
      string Type = content.getString("@type", "");
      if (Type == "beamtest") {
        createSimple(content, topVolume);
        return;
      }


      //Build envelope
      G4LogicalVolume* envelope(0);
      GearDir envelopeParams(content, "Envelope");
      G4ThreeVector envOrigin(0, 0, 0);
      if (!envelopeParams) {
        B2WARNING("Could not find definition for ARICH Envelope, continuing without envelope (note that no photons will be propagated)");
        envelope = &topVolume;
      } else {
        double zEnvFront = envelopeParams.getLength("zFront") / Unit::mm;
        double zEnvBack = envelopeParams.getLength("zBack") / Unit::mm;
        double innerR = envelopeParams.getLength("innerRadius") / Unit::mm;
        double outerR = envelopeParams.getLength("outerRadius") / Unit::mm;
        G4Tubs* envelopeTube = new G4Tubs("Envelope", innerR, outerR, (zEnvBack - zEnvFront) / 2., 0, 2 * M_PI);
        string materialName = envelopeParams.getString("material", "Air");
        G4Material* material = Materials::get(materialName);
        // check of material and its refractive index
        if (!material) { B2FATAL("Material '" << materialName << "', required by ARICH Envelope could not be found");}
        if (!getAvgRINDEX(material)) B2WARNING("Material '" << materialName << "', required by ARICH Envelope has no specified refractive index. Continuing, but no photons in ARICH will be propagated.") ;
        // create and place
        envelope = new G4LogicalVolume(envelopeTube, material, "Envelope");
        setVisibility(*envelope, false);
        envOrigin.setZ((zEnvFront + zEnvBack) / 2.);
        G4Transform3D transform = G4Translate3D(envOrigin);
        new G4PVPlacement(transform, envelope, "ARICH.Envelope", &topVolume, false, 1);

        G4NistManager* man = G4NistManager::Instance();
        G4Material* medPoly = man->FindOrBuildMaterial("G4_POLYETHYLENE");
        G4Material* boron   = man->FindOrBuildMaterial("G4_B");
        // 30% borated polyethylene = SWX210
        G4Material* boratedpoly30 = new G4Material("BoratedPoly30", 1.19 * CLHEP::g / CLHEP::cm3, 2);
        boratedpoly30->AddMaterial(boron, 0.30);
        boratedpoly30->AddMaterial(medPoly, 0.70);

        double zfrr = 1670;
        double zbkk = 1902;
        double zfrr1 = 1906;
        double zbkk1 = 1950;
        double radij1 = 440;
        double radij2 = 500;


        G4ThreeVector nshieldOrigin(0, 0, (zbkk + zfrr) / 2. - envOrigin.z());
        G4ThreeVector nshieldOrigin1(0, 0, (zbkk1 + zfrr1) / 2. - envOrigin.z());
        G4Transform3D transform_nshield = G4Translate3D(nshieldOrigin);
        G4Transform3D transform_nshield1 = G4Translate3D(nshieldOrigin1);
        G4Tubs* nshield_shape = new G4Tubs("nshield_shape", radij1, radij2, (zbkk - zfrr) / 2., 0, 2 * M_PI);
        G4Tubs* nshield_shape1 = new G4Tubs("nshield_shape1", radij1, radij2, (zbkk1 - zfrr1) / 2., 0, 2 * M_PI);
        // please change here boratedpoly30 to boratedpoly05
        G4LogicalVolume* nshield = new G4LogicalVolume(nshield_shape, boratedpoly30, "nShield");
        G4LogicalVolume* nshield1 = new G4LogicalVolume(nshield_shape1, boratedpoly30, "nShield1");
        new G4PVPlacement(transform_nshield , nshield, "ARICH.nShield", envelope, false, 1);
        new G4PVPlacement(transform_nshield1 , nshield1, "ARICH.nShield1", envelope, false, 1);

      }

      isBeamBkgStudy = content.getInt("BeamBackgroundStudy");

      ARICHGeometryPar* m_arichgp = ARICHGeometryPar::Instance();

      // Initializing geometry parametrisation; m_arichgp contains photon detectors positions, ...
      m_arichgp->Initialize(content);

      double boardThick(0), moduleThick(0), detectorZ(0), detectorRout(0), detectorRin(0);

      detectorZ = content.getLength("Detector/Plane/zPosition") / Unit::mm;
      detectorRout = content.getLength("Detector/Plane/tubeOuterRadius") / Unit::mm;
      detectorRin = content.getLength("Detector/Plane/tubeInnerRadius") / Unit::mm;
      GearDir moduleParam(content, "Detector/Module");
      if (!moduleParam) { B2FATAL("Could not find definition parameters for ARICH photon detector module.");} else moduleThick = moduleParam.getLength("moduleZSize") / Unit::mm;

      // build photon detector module
      G4LogicalVolume* lmodule = buildModule(moduleParam);
      if (!lmodule) B2FATAL("ARICH photon detector module was not build.");

      // build photon detector pcb
      G4LogicalVolume* lboard = buildModulePCB(moduleParam);
      if (!lboard) { B2WARNING("ARICH photon detector modules will be placed without PCBs. No parameters specified.");} else boardThick = moduleParam.getLength("Board/thickness") / Unit::mm;

      // build photon detectors support plate
      GearDir supportParam(content, "Detector/SupportPlate");
      G4LogicalVolume* lsupport = buildModuleSupportPlate(supportParam);

      // place photon detector modules and their pcbs
      G4Tubs* detectorsTube = new G4Tubs("detectorsTube", detectorRin, detectorRout, (moduleThick + boardThick) / 2., 0, 2 * M_PI);
      G4Material* dTubeMaterial = Materials::get("ARICH_Air");
      G4LogicalVolume* ldetectorsTube = new G4LogicalVolume(detectorsTube, dTubeMaterial, "detectorsTube");
      setVisibility(*ldetectorsTube, false);
      G4Transform3D dtubeTrans = G4Translate3D(0, 0, detectorZ + (moduleThick + boardThick) / 2. - envOrigin.z());
      int nModules = m_arichgp->getNMCopies();
      for (int i = 1; i <= nModules; i++) {
        G4ThreeVector origin = m_arichgp->getOriginG4(i); origin.setZ(-boardThick / 2.);
        G4ThreeVector pcbOrigin = origin; pcbOrigin.setZ(moduleThick / 2.);
        double angle = m_arichgp->getModAngle(i);
        G4RotationMatrix Ra;
        Ra.rotateZ(angle);
        new G4PVPlacement(G4Transform3D(Ra, origin), lmodule, "detectorModule", ldetectorsTube, false, i);
        if (lboard) new G4PVPlacement(G4Transform3D(Ra, pcbOrigin), lboard, "detectorBoard", ldetectorsTube, false, i);
      }
      new G4PVPlacement(dtubeTrans, ldetectorsTube, "ARICH.DetectorModules", envelope, false, 1);
      B2INFO("ARICH photon detector modules were placed at z=" << detectorZ << "mm.");

      // place detectors support plate
      if (lsupport) {
        double supportThick =  supportParam.getLength("thickness") / Unit::mm;
        G4ThreeVector supportPos = G4ThreeVector(0, 0, detectorZ + moduleThick + boardThick + supportThick / 2.) - envOrigin;
        G4Transform3D transform3 = G4Translate3D(supportPos);
        new G4PVPlacement(transform3, lsupport, "ARICH.DetectorSupportPlate", envelope, false, 1);
        B2INFO("ARICH detector plane support plate is placed.");
      } else B2WARNING("ARICH detector modules will be placed without support plate. No parameters specified.");

      // build aerogel layers
      GearDir aeroParam(content, "Aerogel");

      // get aerogel parameters
      double rin = aeroParam.getLength("tubeInnerRadius") / Unit::mm;
      double rout = aeroParam.getLength("tubeOuterRadius") / Unit::mm;
      double tileX = aeroParam.getLength("tileXSize") / Unit::mm;
      double tileY = aeroParam.getLength("tileYSize") / Unit::mm;
      double tileGap = aeroParam.getLength("tileGap") / Unit::mm;
      double supportZ(0);

      string gapsMat = aeroParam.getString("tubeMaterial");
      G4Material* gapsMaterial = Materials::get(gapsMat);
      int ilayer = 0;
      // build and place aerogel layers
      BOOST_FOREACH(const GearDir & layer, aeroParam.getNodes("Layers/Layer")) {

        double thick = layer.getLength("thickness");
        m_arichgp->setAerogelThickness(ilayer, thick);
        string aeroMat = layer.getString("material");
        double zPos = layer.getLength("zPosition");
        m_arichgp->setAerogelZPosition(ilayer, zPos);
        G4Material* aeroMaterial = Materials::get(aeroMat);
        // here we check that aerogel has specified optical properties, and set their values in m_arichgp to be available for reconstruction later on
        G4MaterialPropertiesTable* mTable = aeroMaterial->GetMaterialPropertiesTable();
        if (!mTable) { B2WARNING("Material '" << aeroMat << "', for ARICH aerogel layer has no specified optical properties");} else {
          G4MaterialPropertyVector* mVector =  mTable->GetProperty("RAYLEIGH");
          if (!mVector) { B2WARNING("Material '" << aeroMat << "', for ARICH aerogel layer has no specified Rayleigh attenuation length.");} else {
            double lambda0 = 400;
            double e0 = 1240. / lambda0 * Unit::eV / Unit::MeV;

            G4bool b;
            m_arichgp->setAeroTransLength(ilayer, mVector->GetValue(e0, b)*Unit::mm);
          }
          double rindex = getAvgRINDEX(aeroMaterial);
          if (!rindex) B2WARNING("Material '" << aeroMat << "', for ARICH aerogel layer has no specified refractive index. No Cherenkov photons will be produced.");
          m_arichgp->setAeroRefIndex(ilayer, rindex);
        }

        if (supportZ == 0) supportZ = zPos / Unit::mm;

        G4Tubs* aerogelTube = new G4Tubs("aerogelTube", rin, rout, thick / 2. / Unit::mm, 0, 2 * M_PI);
        std::stringstream tubeName;
        tubeName << "AerogelTube_" << ilayer;
        G4LogicalVolume* laerogelTube = new G4LogicalVolume(aerogelTube, gapsMaterial, tubeName.str().c_str());

        // build and place aerogel tiles in tube

        // number of tiles in radial direction
        int nRTiles = int((rout - rin) / (tileX + tileGap)) + 1;
        // size of the tile in radial direction
        double rSize = (rout - rin) / nRTiles - tileGap;
        for (int iRad = 0; iRad < nRTiles; iRad++) {
          double routT = rin + tileGap / 2. + rSize + iRad * (rSize + tileGap);
          double circ = 2.*M_PI * routT;
          int nPhiTiles = int(circ / (tileY + tileGap)) + 1;
          double dphi = 2.*M_PI / double(nPhiTiles);
          G4Tubs* tileShape = new G4Tubs("tileShape", routT - rSize, routT, thick / 2. / Unit::mm, -dphi / 2. + tileGap / 2. / routT, dphi  - tileGap / routT);
          std::stringstream tileName;
          tileName << "AeroTile_" << iRad << "_" << ilayer;
          G4LogicalVolume* ltile = new G4LogicalVolume(tileShape, aeroMaterial, tileName.str().c_str());
          setColor(*ltile, "rgb(0.0, 1.0, 1.0,1.0)");
          for (int iPhi = 0; iPhi < nPhiTiles; iPhi++) {
            double phi = dphi / 2. + iPhi * dphi;
            G4RotationMatrix Ra;
            Ra.rotateZ(phi);
            new G4PVPlacement(G4Transform3D(Ra, G4ThreeVector(0, 0, 0)), ltile, "AeroTile", laerogelTube, false, iPhi);
          }
        }
        // place aerogel layer
        G4ThreeVector aeroPos = G4ThreeVector(0, 0, zPos / Unit::mm + thick / 2. / Unit::mm) - envOrigin;
        G4Transform3D transform4 = G4Translate3D(aeroPos);
        string id = layer.getString("@id", "");
        new G4PVPlacement(transform4, laerogelTube, "ARICH.AerogelTube_" + id, envelope, false, 1);
        B2INFO("ARICH aerogel layer " << id << " with thickness=" << thick / Unit::mm  << "mm was placed at z = " << zPos / Unit::mm << "mm.");
        ilayer++;
      }

      // place aerogel layer support plate
      if (aeroParam.exists("SupportPlate")) {
        double asupportThick = aeroParam.getLength("SupportPlate/thickness") / Unit::mm;
        string asupMat = aeroParam.getString("SupportPlate/material");
        G4Material* asupportMaterial = Materials::get(asupMat);

        G4Tubs* asupportTube = new G4Tubs("asupportTube", rin, rout, asupportThick / 2., 0, 2 * M_PI);
        G4LogicalVolume* lasupportTube = new G4LogicalVolume(asupportTube, asupportMaterial, "AerogelSupportPlate");
        lasupportTube->SetSensitiveDetector(m_sensitiveAero);
        G4ThreeVector asupPos = G4ThreeVector(0, 0, supportZ - asupportThick / 2.) - envOrigin;
        G4Transform3D transform5 = G4Translate3D(asupPos);
        new G4PVPlacement(transform5, lasupportTube, "ARICH.AerogelSupportPlate", envelope, false, 1);
        B2INFO("ARICH aerogel layers support plate was placed.")
      } else B2WARNING("ARICH aerogel layers will be placed without support plate. No parameters specified.");

      // build mirrors

      GearDir mirrorParam(content, "Mirrors");
      G4LogicalVolume* lMirrors = buildMirrors(mirrorParam);

      if (lMirrors) {
        double mLength = mirrorParam.getLength("mirrorLength") / Unit::mm;
        double mZpos = mirrorParam.getLength("Zposition") / Unit::mm;
        G4ThreeVector mirrPos = G4ThreeVector(0, 0, mZpos + mLength / 2.) - envOrigin;
        G4Transform3D transform6 =  G4Translate3D(mirrPos);
        new G4PVPlacement(transform6, lMirrors, "ARICH.Mirrors", envelope, false, 1);
        B2INFO("ARICH mirrors were placed.");
      } else B2INFO("ARICH: mirrors will not be placed. No parameters specified.");

    }

    G4LogicalVolume* GeoARICHCreator::buildModule(GearDir Module)
    {

      // get detector module parameters

      // get module materials
      string wallMat =  Module.getString("wallMaterial");
      string winMat =  Module.getString("windowMaterial");
      string botMat =  Module.getString("Bottom/material");
      G4Material* wallMaterial = Materials::get(wallMat);
      G4Material* windowMaterial = Materials::get(winMat);
      G4Material* bottomMaterial = Materials::get(botMat);
      G4Material* boxFill = Materials::get("ARICH_Vacuum");

      // check that module window material has specified refractive index
      double wref = getAvgRINDEX(windowMaterial);
      if (!wref) B2WARNING("Material '" << winMat << "', required for ARICH photon detector window as no specified refractive index. Continuing, but no photons in ARICH will be detected.");
      ARICHGeometryPar* m_arichgp = ARICHGeometryPar::Instance();
      m_arichgp->setWindowRefIndex(wref);
      // get module dimensions
      double modXsize = Module.getLength("moduleXSize") / Unit::mm;
      double modZsize = Module.getLength("moduleZSize") / Unit::mm;
      double wallThick = Module.getLength("moduleWallThickness") / Unit::mm;
      double winThick = Module.getLength("windowThickness") / Unit::mm ;
      double sensXsize = m_arichgp->getSensitiveSurfaceSize() / Unit::mm;
      double botThick =  Module.getLength("Bottom/thickness") / Unit::mm;

      // some trivial checks of overlaps
      if (sensXsize > modXsize - 2 * wallThick) B2FATAL("ARICH photon detector module: Sensitive surface is too big. Doesn't fit into module box.");
      if (winThick + botThick > modZsize) B2FATAL("ARICH photon detector module: window + bottom thickness larger than module thickness.");

      // module master volume
      G4Box* moduleBox = new G4Box("Box", modXsize / 2., modXsize / 2., modZsize / 2.);
      G4LogicalVolume* lmoduleBox = new G4LogicalVolume(moduleBox, boxFill, "moduleBox");

      // build and place module wall
      G4Box* tempBox = new G4Box("tempBox", modXsize / 2. - wallThick, modXsize / 2. - wallThick, modZsize / 2. + 0.1); // Dont't care about "+0.1", needs to be there.
      G4SubtractionSolid* moduleWall = new G4SubtractionSolid("Box-tempBox", moduleBox, tempBox);
      G4LogicalVolume* lmoduleWall = new G4LogicalVolume(moduleWall, wallMaterial, "moduleWall");
      setColor(*lmoduleWall, "rgb(1.0,0.0,0.0,1.0)");
      new G4PVPlacement(G4Transform3D(), lmoduleWall, "moduleWall", lmoduleBox, false, 1);

      // build module window
      G4Box* winBox = new G4Box("winBox", modXsize / 2. - wallThick, modXsize / 2. - wallThick, winThick / 2.);
      G4LogicalVolume* lmoduleWin = new G4LogicalVolume(winBox, windowMaterial, "moduleWindow");
      setColor(*lmoduleWin, "rgb(0.7,0.7,0.7,1.0)");
      G4Transform3D transform = G4Translate3D(0., 0., (-modZsize + winThick) / 2.);
      new G4PVPlacement(transform, lmoduleWin, "moduleWindow", lmoduleBox, false, 1);

      // build module bottom
      G4Box* botBox = new G4Box("botBox", modXsize / 2. - wallThick, modXsize / 2. - wallThick, botThick / 2.);
      G4LogicalVolume* lmoduleBot = new G4LogicalVolume(botBox, bottomMaterial, "moduleBottom");
      if (isBeamBkgStudy) lmoduleBot->SetSensitiveDetector(new BkgSensitiveDetector("ARICH", 1));
      setColor(*lmoduleBot, "rgb(0.0,1.0,0.0,1.0)");
      G4Transform3D transform1 = G4Translate3D(0., 0., (modZsize - botThick) / 2.);
      // add surface optical properties if specified
      Materials& materials = Materials::getInstance();
      GearDir bottomParam(Module, "Bottom/Surface");
      if (bottomParam) {
        G4OpticalSurface* optSurf = materials.createOpticalSurface(bottomParam);
        new G4LogicalSkinSurface("bottomSurface", lmoduleBot, optSurf);
      } else B2INFO("ARICH: No optical properties are specified for detector module bottom surface.");
      new G4PVPlacement(transform1, lmoduleBot, "moduleBottom", lmoduleBox, false, 1);

      // build sensitive surface
      G4Box* sensBox = new G4Box("sensBox", sensXsize / 2., sensXsize / 2., 0.1 * Unit::mm);
      G4LogicalVolume* lmoduleSens = new G4LogicalVolume(sensBox, boxFill, "moduleSensitive");
      lmoduleSens->SetSensitiveDetector(m_sensitive);
      setColor(*lmoduleSens, "rgb(0.5,0.5,0.5,1.0)");
      G4Transform3D transform2 = G4Translate3D(0., 0., (-modZsize + 0.1) / 2. + winThick);
      new G4PVPlacement(transform2, lmoduleSens, "moduleSensitive", lmoduleBox, false, 1);

      // module is build, return module logical volume
      return lmoduleBox;
    }

    G4LogicalVolume* GeoARICHCreator::buildModulePCB(GearDir Module)
    {
      if (!Module.exists("Board")) return 0;

      // read parameters
      string pcbMat = Module.getString("Board/material");
      G4Material* pcbMaterial = Materials::get(pcbMat);
      double pcbSize = Module.getLength("Board/size") / Unit::mm;
      double pcbThick = Module.getLength("Board/thickness") / Unit::mm;

      // build board
      G4Box* pcbBox = new G4Box("pcbBox", pcbSize / 2., pcbSize / 2., pcbThick / 2.);
      G4LogicalVolume* lpcb = new G4LogicalVolume(pcbBox, pcbMaterial, "modulePcb");
      if (isBeamBkgStudy) lpcb->SetSensitiveDetector(new BkgSensitiveDetector("ARICH"));
      setColor(*lpcb, "rgb(0.0,0.6,0.0,1.0)");
      // return pcb logical volume
      return lpcb;
    }

    G4LogicalVolume* GeoARICHCreator::buildModuleSupportPlate(GearDir Support)
    {
      if (!Support) return 0;

      // read parameters
      string supportMat = Support.getString("material");
      G4Material* supportMaterial = Materials::get(supportMat);
      G4Material* holeMaterial = Materials::get("Air");
      double supportThick = Support.getLength("thickness") / Unit::mm;
      double supportOutRad = Support.getLength("outerRadius") / Unit::mm;
      double supportInRad = Support.getLength("innerRadius") / Unit::mm;
      double holeY = Support.getLength("holeYSize") / Unit::mm;
      double holeX = Support.getLength("holeXSize") / Unit::mm;

      ARICHGeometryPar* m_arichgp = ARICHGeometryPar::Instance();

      // build plate
      G4Tubs* supportTube = new G4Tubs("supportTube", supportInRad, supportOutRad, supportThick / 2., 0, 2 * M_PI);
      G4LogicalVolume* lsupportTube = new G4LogicalVolume(supportTube, supportMaterial, "supportPlate");
      // build hole
      G4Box* hole = new G4Box("hole", holeX / 2., holeY / 2., supportThick / 2.);
      G4LogicalVolume* lhole = new G4LogicalVolume(hole, holeMaterial, "supportHole");
      setColor(*lsupportTube, "rgb(0.5,0.5,0.5,1.0)");
      setVisibility(*lhole, false);
      // drill holes in support plate
      int nholes = m_arichgp->getNMCopies();
      for (int i = 1; i <= nholes; i++) {
        G4ThreeVector originXY = m_arichgp->getOriginG4(i);
        originXY.setZ(0);
        double angle = m_arichgp->getModAngle(i);
        G4RotationMatrix Ra;
        Ra.rotateZ(angle);
        new G4PVPlacement(G4Transform3D(Ra, originXY), lhole, "supportHole", lsupportTube, false, i);
      }

      // return support logical volume
      return lsupportTube;
    }

    G4LogicalVolume* GeoARICHCreator::buildMirrors(GearDir Mirrors)
    {
      if (!Mirrors) return 0;

      // read parameters
      string mirrMat = Mirrors.getString("Material");
      G4Material* mirrorMaterial = Materials::get(mirrMat);

      int nMirrors = Mirrors.getInt("nMirrors");
      double outr = Mirrors.getLength("outerRadius") / Unit::mm * cos(M_PI / nMirrors);
      double mThick = Mirrors.getLength("mirrorThickness") / Unit::mm;
      double mLength = Mirrors.getLength("mirrorLength") / Unit::mm;
      double startAngle = Mirrors.getAngle("startAngle");

      double zPlane[2] = { -mLength / 2., mLength / 2.};
      double innerr[2] = {outr - mThick, outr - mThick};
      double outerr[2] = {outr, outr};

      G4Polyhedra* mirrShape = new G4Polyhedra("mirrShape", startAngle, 2 * M_PI, nMirrors, 2, zPlane, innerr, outerr);
      G4LogicalVolume* lmirrors = new G4LogicalVolume(mirrShape, mirrorMaterial, "ARICH.Mirrors");

      GearDir surface(Mirrors, "Surface");
      if (surface) {
        Materials& materials = Materials::getInstance();
        G4OpticalSurface* optSurf = materials.createOpticalSurface(surface);
        new G4LogicalSkinSurface("mirrorsSurface", lmirrors, optSurf);
      } else B2WARNING("ARICH mirrors surface has no specified optical properties. No photons will be reflected.");

      return lmirrors;
    }

    double GeoARICHCreator::getAvgRINDEX(G4Material* material)
    {
      G4MaterialPropertiesTable* mTable = material->GetMaterialPropertiesTable();
      if (!mTable) return 0;
      G4MaterialPropertyVector* mVector =  mTable->GetProperty("RINDEX");
      if (!mVector) return 0;
      G4bool b;
      return mVector->GetValue(2 * Unit::eV / Unit::MeV, b);
    }

    void GeoARICHCreator::createSimple(const GearDir& content, G4LogicalVolume& topVolume)
    {

      B2INFO("ARICH simple (beamtest) geometry will be built.")
      GearDir envelopeParams(content, "Envelope");
      double xSize = envelopeParams.getLength("xSize") / Unit::mm;
      double ySize = envelopeParams.getLength("ySize") / Unit::mm;
      double zSize = envelopeParams.getLength("zSize") / Unit::mm;
      string envMat = envelopeParams.getString("material");
      G4Material* envMaterial = Materials::get(envMat);

      G4Box* envBox = new G4Box("envBox", xSize / 2., ySize / 2., zSize / 2.);
      G4LogicalVolume* lenvBox = new G4LogicalVolume(envBox, envMaterial, "ARICH.envelope");
      new G4PVPlacement(G4Transform3D(), lenvBox, "ARICH.envelope", &topVolume, false, 1);
      setVisibility(*lenvBox, false);
      ARICHGeometryPar* m_arichgp = ARICHGeometryPar::Instance();
      m_arichgp->Initialize(content);
      GearDir moduleParam(content, "Detector/Module");
      G4LogicalVolume* detModule = buildModule(moduleParam);

      double detZpos = content.getLength("Detector/Plane/zPosition") / Unit::mm;
      double detThick = content.getLength("Detector/Module/moduleZSize") / Unit::mm;
      int nModules = m_arichgp->getNMCopies();
      for (int i = 1; i <= nModules; i++) {
        G4ThreeVector origin = m_arichgp->getOriginG4(i); origin.setZ(detZpos + detThick / 2.);
        double angle = m_arichgp->getModAngle(i);
        G4RotationMatrix Ra;
        Ra.rotateZ(angle);
        G4Transform3D trans = G4Transform3D(Ra, origin);
        new G4PVPlacement(G4Transform3D(Ra, origin), detModule, "detModule", lenvBox, false, i);
      }

      // place aerogel tiles
      GearDir aerogelParam(content, "Aerogel");
      double sizeX = aerogelParam.getLength("tileXSize") / Unit::mm;
      double sizeY = aerogelParam.getLength("tileYSize") / Unit::mm;
      double posX = aerogelParam.getLength("tileXPos") / Unit::mm;
      double posY = aerogelParam.getLength("tileYPos") / Unit::mm;
      int ilayer = 0;
      BOOST_FOREACH(const GearDir & layer, aerogelParam.getNodes("Layers/Layer")) {
        double posZ = layer.getLength("zPosition");
        double sizeZ = layer.getLength("thickness");
        string tileMat = layer.getString("material");
        G4Material* tileMaterial = Materials::get(tileMat);
        double rInd = getAvgRINDEX(tileMaterial);
        m_arichgp->setAeroRefIndex(ilayer, rInd);
        m_arichgp->setAerogelZPosition(ilayer, posZ);
        m_arichgp->setAerogelThickness(ilayer, sizeZ);

        G4MaterialPropertiesTable* mTable = tileMaterial->GetMaterialPropertiesTable();
        G4MaterialPropertyVector* mVector =  mTable->GetProperty("RAYLEIGH");
        double lambda0 = 400;
        double e0 = 1240. / lambda0 * Unit::eV / Unit::MeV;
        G4bool b;
        m_arichgp->setAeroTransLength(ilayer, mVector->GetValue(e0, b)*Unit::mm);
        G4Box* tileBox = new G4Box("tileBox", sizeX / 2., sizeY / 2., sizeZ / 2. / Unit::mm);
        G4LogicalVolume* lTile = new G4LogicalVolume(tileBox, tileMaterial, "Tile", 0, m_sensitiveAero);
        setColor(*lTile, "rgb(0.0, 1.0, 1.0,1.0)");
        G4Transform3D trans = G4Translate3D(posX, posY, posZ / Unit::mm + sizeZ / 2. / Unit::mm);
        ilayer++;
        new G4PVPlacement(trans, lTile, "ARICH.tile", lenvBox, false, ilayer);
      }

      // place mirrors
      GearDir mirrorsParam(content, "Mirrors");
      double height = mirrorsParam.getLength("height") / Unit::mm;
      double width = mirrorsParam.getLength("width") / Unit::mm;
      double thickness = mirrorsParam.getLength("thickness") / Unit::mm;
      string mirrMat = mirrorsParam.getString("material");
      G4Material* mirrMaterial = Materials::get(mirrMat);
      G4Box* mirrBox = new G4Box("mirrBox", thickness / 2., height / 2., width / 2.);
      G4LogicalVolume* lmirror = new G4LogicalVolume(mirrBox, mirrMaterial, "mirror");

      Materials& materials = Materials::getInstance();
      GearDir surface(mirrorsParam, "Surface");
      G4OpticalSurface* optSurf = materials.createOpticalSurface(surface);
      new G4LogicalSkinSurface("mirrorsSurface", lmirror, optSurf);
      int iMirror = 0;
      BOOST_FOREACH(const GearDir & mirror, mirrorsParam.getNodes("Mirror")) {
        double xpos = mirror.getLength("xPos") / Unit::mm;
        double ypos = mirror.getLength("yPos") / Unit::mm;
        double zpos = mirror.getLength("zPos") / Unit::mm;
        double angle = mirror.getAngle("angle") / Unit::rad;
        G4ThreeVector origin(xpos, ypos, zpos + width / 2.);
        G4RotationMatrix Ra;
        Ra.rotateZ(angle);
        G4Transform3D trans = G4Transform3D(Ra, origin);
        new G4PVPlacement(G4Transform3D(Ra, origin), lmirror, "ARICH.mirror", lenvBox, false, iMirror);
        iMirror++;
      }
    }
  }
}
