/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/geometry/GeoARICHCreator.h>

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
#include <G4LogicalVolumeStore.hh>

// Geant4 Shapes
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Trap.hh>
#include <G4TwoVector.hh>
#include <G4ExtrudedSolid.hh>

#include <G4Polyhedra.hh>
#include <G4SubtractionSolid.hh>
#include <G4Material.hh>
#include <TVector2.h>
#include <TVector3.h>
#include <TGraph2D.h>


using namespace std;
using namespace boost;
using namespace CLHEP;

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

    GeoARICHCreator::GeoARICHCreator(): m_isBeamBkgStudy(0)
    {
      m_sensitive = NULL;
      m_sensitiveAero = NULL;
    }

    GeoARICHCreator::~GeoARICHCreator()
    {
      delete m_sensitive;
      delete m_sensitiveAero;
      G4LogicalSkinSurface::CleanSurfaceTable();

    }


    void GeoARICHCreator::createGeometry(G4LogicalVolume& topVolume, GeometryTypes type)
    {

      if (type == 0) {
        B2INFO("Geometry type: Full");
      } else if (type == 1) {
        B2INFO("Geometry type: Tracking");
      } else if (type == 2) {
        B2INFO("Geometry type: Display");
      }

      m_config.useGeantUnits();
      m_sensitive = new SensitiveDetector();
      m_sensitiveAero = new SensitiveAero();

      // print geometry configuration
      // m_config.print();

      //Build envelope
      G4Tubs* envelopeTube = new G4Tubs("Envelope", m_config.getMasterVolume().getInnerRadius(),
                                        m_config.getMasterVolume().getOuterRadius(), m_config.getMasterVolume().getLength() / 2., 0, 2 * M_PI);
      G4Material* material = Materials::get(m_config.getMasterVolume().getMaterial());

      // check of material and its refractive index
      if (!material) { B2FATAL("Material ARICH_Air required for ARICH master volume could not be found");}
      if (!getAvgRINDEX(material))
        B2WARNING("Material ARICH_Air required by ARICH master volume has no specified refractive index. Continuing, but no photons in ARICH will be propagated.")
        ;

      // create and place
      G4LogicalVolume* masterLV = new G4LogicalVolume(envelopeTube, material, "ARICH.masterVolume");
      setVisibility(*masterLV, false);

      G4RotationMatrix rotMaster;
      rotMaster.rotateX(m_config.getMasterVolume().getRotationX());
      rotMaster.rotateY(m_config.getMasterVolume().getRotationY());
      rotMaster.rotateZ(m_config.getMasterVolume().getRotationZ());

      G4ThreeVector transMaster(m_config.getMasterVolume().getPosition().X(), m_config.getMasterVolume().getPosition().Y(),
                                m_config.getMasterVolume().getPosition().Z());

      new G4PVPlacement(G4Transform3D(rotMaster, transMaster), masterLV, "ARICH.MasterVolume", &topVolume, false, 1);

      m_isBeamBkgStudy = m_config.doBeamBackgroundStudy();

      // Z shift for placing volumes in ARICH master volume
      double zShift = 0;

      // build photon detector logical volume
      G4LogicalVolume* detPlaneLV = buildDetectorPlane(m_config);
      G4LogicalVolume* detSupportPlateLV = buildDetectorSupportPlate(m_config);

      G4LogicalVolume* aeroPlaneLV;
      if (!m_config.getAerogelPlane().isSimple()) {
        aeroPlaneLV = buildAerogelPlane(m_config);
      } else {
        B2INFO("GeoARICHCreator: using simple cosmic test geometry.");
        aeroPlaneLV = buildSimpleAerogelPlane(m_config);
      }

      G4RotationMatrix rotDetPlane;
      rotDetPlane.rotateX(m_config.getDetectorPlane().getRotationX());
      rotDetPlane.rotateY(m_config.getDetectorPlane().getRotationY());
      rotDetPlane.rotateZ(m_config.getDetectorPlane().getRotationZ());

      G4RotationMatrix rotAeroPlane;
      rotAeroPlane.rotateX(m_config.getAerogelPlane().getRotationX());
      rotAeroPlane.rotateY(m_config.getAerogelPlane().getRotationY());
      rotAeroPlane.rotateZ(m_config.getAerogelPlane().getRotationZ());

      G4ThreeVector transDetPlane(m_config.getDetectorPlane().getPosition().X(), m_config.getDetectorPlane().getPosition().Y(),
                                  m_config.getDetectorPlane().getPosition().Z() + zShift);

      G4ThreeVector transDetSupportPlate(m_config.getDetectorPlane().getPosition().X(), m_config.getDetectorPlane().getPosition().Y(),
                                         m_config.getDetectorPlane().getSupportZPosition() + zShift);

      G4ThreeVector transAeroPlane(m_config.getAerogelPlane().getPosition().X(), m_config.getAerogelPlane().getPosition().Y(),
                                   m_config.getAerogelPlane().getPosition().Z() + zShift);

      new G4PVPlacement(G4Transform3D(rotDetPlane, transDetPlane), detPlaneLV, "ARICH.detPlane", masterLV, false, 1);
      new G4PVPlacement(G4Transform3D(rotDetPlane, transDetSupportPlate), detSupportPlateLV, "ARICH.detSupportPlane", masterLV, false, 1);
      new G4PVPlacement(G4Transform3D(rotAeroPlane, transAeroPlane), aeroPlaneLV, "ARICH.aeroPlane", masterLV, false, 1);

      // build and place mirrors
      G4LogicalVolume* mirrorLV = buildMirror(m_config);
      int nMirrors = m_config.getMirrors().getNMirrors();
      int mirStart = m_config.getMirrors().getStartAngle();
      int mirZPos = m_config.getMirrors().getZPosition();
      int mirRad = m_config.getMirrors().getRadius();

      double angl = mirStart;
      double dphi = 2 * M_PI / nMirrors;
      for (int i = 1; i < nMirrors + 1; i++) {
        G4RotationMatrix rotMirror;
        rotMirror.rotateZ(angl);
        G4ThreeVector transMirror(mirRad * cos(angl), mirRad * sin(angl), mirZPos + zShift);
        new G4PVPlacement(G4Transform3D(rotMirror, transMirror), mirrorLV, "ARICH.mirrorPlate", masterLV, false, i);
        angl += dphi;
      }


      // build additional components of support structure

      const ARICHGeoSupport& supportPar = m_config.getSupportStructure();
      std::vector<G4LogicalVolume*> shieldLV(2);
      std::vector<double> shieldZ(2);
      int nTubes = supportPar.getNTubes();
      for (int i = 0; i < nTubes; i++) {
        G4Tubs*  tube = new G4Tubs(supportPar.getTubeName(i), supportPar.getTubeInnerR(i), supportPar.getTubeOuterR(i),
                                   supportPar.getTubeLength(i) / 2., 0, 2.*M_PI);
        G4Material* tubeMaterial = Materials::get(supportPar.getTubeMaterial(i));

        if (supportPar.getTubeName(i) == "ARICH.NeutronShield1") {
          shieldLV[0] = new G4LogicalVolume(tube, tubeMaterial, supportPar.getTubeName(i));
          shieldZ[0] = supportPar.getTubeZPosition(i) + supportPar.getTubeLength(i) / 2.;
          continue;
        } else if (supportPar.getTubeName(i) == "ARICH.NeutronShield2") {
          shieldLV[1] = new G4LogicalVolume(tube, tubeMaterial, supportPar.getTubeName(i));
          shieldZ[1] = supportPar.getTubeZPosition(i) + supportPar.getTubeLength(i) / 2.;
          continue;
        }

        G4LogicalVolume* tubeLV = new G4LogicalVolume(tube, tubeMaterial, supportPar.getTubeName(i));

        new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0,
                                        supportPar.getTubeZPosition(i) + supportPar.getTubeLength(i) / 2. + zShift)), tubeLV, supportPar.getTubeName(i), masterLV, false,
                          1);
      }

      const std::vector<double> wedge1 = supportPar.getWedge(1);
      const std::vector<double> wedge2 = supportPar.getWedge(2);
      G4Material* wedge1Material = Materials::get(supportPar.getWedgeMaterial(1));
      G4Material* wedge2Material = Materials::get(supportPar.getWedgeMaterial(2));
      G4AssemblyVolume* assemblyWedge1 = makeJoint(wedge1Material, wedge1);
      G4AssemblyVolume* assemblyWedge2 = makeJoint(wedge2Material, wedge2);

      G4Transform3D Tr;
      int nWedge = supportPar.getNWedges();
      for (int i = 0; i < nWedge; i++) {
        G4RotationMatrix Rx;
        int type = supportPar.getWedgeType(i);
        double phi = supportPar.getWedgePhi(i);
        Rx.rotateZ(phi);
        double r = supportPar.getWedgeR(i);
        double z = supportPar.getWedgeZ(i);
        G4ThreeVector Tb(r * cos(phi), r * sin(phi), z);

        if (shieldLV[0]) {
          z -= shieldZ[0];
          Tb.setZ(z);
          Tr = G4Transform3D(Rx, Tb);
          if (type == 1) assemblyWedge1->MakeImprint(shieldLV[0], Tr);
          else if (type == 2) assemblyWedge2->MakeImprint(shieldLV[0], Tr);
          continue;
        }

        Tr = G4Transform3D(Rx, Tb);
        if (type == 1) assemblyWedge1->MakeImprint(masterLV, Tr);
        else if (type == 2) assemblyWedge2->MakeImprint(masterLV, Tr);
        else B2ERROR("GeoARICHCreator: invalid support wedge type!");
      }

      // place neutron shield volumes
      if (shieldLV[0])
        new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0, shieldZ[0])), shieldLV[0], "ARICH.NeutronShield1", masterLV,
                          false, 1);
      if (shieldLV[1])
        new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0, shieldZ[1])), shieldLV[1], "ARICH.NeutronShield2", masterLV,
                          false, 1);

      // place mirror holders, for now skip
      /*
      double mirSupX = 17.;
      double mirSupY = 14.;
      double mirSupLen = 131;
      double mirSupLen1 = 201;

      std::vector<G4TwoVector> polygon;
      polygon.assign(12, G4TwoVector());
      polygon[11] = G4TwoVector(-mirSupX / 2., -mirSupY / 2.);
      polygon[10] = G4TwoVector(-mirSupX / 2., 2.);
      polygon[9] = G4TwoVector(-mirSupX / 4., 2. - 0.75);
      polygon[8] = G4TwoVector(-mirSupX / 4., 5. - 0.75);
      polygon[7] = G4TwoVector(-mirSupX / 2., 5);
      polygon[6] = G4TwoVector(-mirSupX / 2., mirSupY / 2.);
      polygon[5] = G4TwoVector(mirSupX / 2., mirSupY / 2.);
      polygon[4] = G4TwoVector(mirSupX / 2., 6);
      polygon[3] = G4TwoVector(mirSupX / 4., 6. - 0.75);
      polygon[2] = G4TwoVector(mirSupX / 4., 2. - 0.75);
      polygon[1] = G4TwoVector(mirSupX / 2., 2.);
      polygon[0] = G4TwoVector(mirSupX / 2., -mirSupY / 2.);

      std::vector<G4ExtrudedSolid::ZSection> zsections;
      zsections.push_back(G4ExtrudedSolid::ZSection(-mirSupLen / 2., G4TwoVector(0, 0), 1));
      zsections.push_back(G4ExtrudedSolid::ZSection(mirSupLen / 2., G4TwoVector(0, 0), 1));

      G4ExtrudedSolid* shape = new G4ExtrudedSolid("bla", polygon, zsections);
      G4LogicalVolume* shapeLV = new G4LogicalVolume(shape, wedge1Material, "mirrorsupport");

      G4Box* shape1 = new G4Box("shape1", mirSupX / 2., 2. / 2., mirSupLen1 / 2.);
      G4LogicalVolume* shape1LV = new G4LogicalVolume(shape1, wedge1Material, "mirrorholder");

      G4AssemblyVolume* mirroHolder = new G4AssemblyVolume();
      G4RotationMatrix Rh;
      G4ThreeVector Th(0, 0, 0);
      Tr = G4Transform3D(Rh, Th);
      mirroHolder->AddPlacedVolume(shapeLV, Tr);

      Th.setZ(-35.0);
      Th.setY(-mirSupY / 2. - 1.);
      Tr = G4Transform3D(Rh, Th);
      mirroHolder->AddPlacedVolume(shape1LV, Tr);

      double rholder = m_config.getDetectorPlane().getSupportOuterR() - mirSupY / 2.;

      angl = dphi / 2.;
      for (int i = 1; i < nMirrors + 1; i++) {
        G4RotationMatrix rotMirror;
        rotMirror.rotateX(M_PI);
        rotMirror.rotateZ(-M_PI / 2. + angl);
        G4ThreeVector transMirror(rholder * cos(angl), rholder * sin(angl), mirZPos + zShift);
        Tr = G4Transform3D(rotMirror, transMirror);
        mirroHolder->MakeImprint(masterLV, Tr);
        angl += dphi;
      }
      */

      // temporary for simple cosmic test
      // if using simple configuration, place scinitilators
      if (m_config.getAerogelPlane().isSimple()) {
        int nBoxes = supportPar.getNBoxes();
        for (int i = 0; i < nBoxes; i++) {
          ARICHGeoSupport::box box = supportPar.getBox(i);
          G4Box* scintBox = new G4Box("scintBox", box.size[0] * 10. / 2., box.size[1] * 10. / 2., box.size[2] * 10. / 2.);
          G4Material* scintMaterial = Materials::get(box.material);
          G4LogicalVolume* scintLV = new G4LogicalVolume(scintBox, scintMaterial, box.name);
          scintLV->SetSensitiveDetector(m_sensitiveAero);
          G4RotationMatrix rotScint;
          rotScint.rotateX(box.rotation[0]);
          rotScint.rotateY(box.rotation[1]);
          rotScint.rotateZ(box.rotation[2]);
          TVector3 transScintTV(box.position[0], box.position[1], box.position[2]);
          transScintTV = m_config.getMasterVolume().pointToGlobal(transScintTV);
          B2INFO("GeoARICHCreator: Scintilator " << box.name << " placed at global: " << transScintTV.X() << " " << transScintTV.Y() << " " <<
                 transScintTV.Z());
          G4ThreeVector transScint(box.position[0] * 10., box.position[1] * 10., box.position[2] * 10.);
          new G4PVPlacement(G4Transform3D(rotScint, transScint), scintLV, "scintilator", masterLV, false, 1);
        }
      }

      m_config.useBasf2Units();

      return;

    }


    G4LogicalVolume* GeoARICHCreator::buildSimpleAerogelPlane(const ARICHGeometryConfig& detectorGeo)
    {

      const ARICHGeoAerogelPlane& aeroGeo = detectorGeo.getAerogelPlane();

      const std::vector<double>& params  = aeroGeo.getSimpleParams();

      // support plane
      double rin = aeroGeo.getSupportInnerR();
      double rout = aeroGeo.getSupportOuterR();
      double thick = aeroGeo.getSupportThickness();
      string supportMat = aeroGeo.getSupportMaterial();
      double wallHeight = aeroGeo.getWallHeight();
      G4Material* supportMaterial = Materials::get(supportMat);
      G4Material* gapMaterial = Materials::get("Air"); // Air without refractive index (to kill photons, to mimic black paper around tile)

      // master volume
      G4Tubs* aerogelTube = new G4Tubs("aerogelTube", rin, rout, (thick + wallHeight) / 2., 0, 2 * M_PI);
      G4LogicalVolume* aerogelPlaneLV = new G4LogicalVolume(aerogelTube, gapMaterial, "ARICH.AaerogelPlane");

      // support plate
      G4Tubs* supportTube = new G4Tubs("aeroSupportTube", rin, rout, thick / 2., 0, 2 * M_PI);
      G4LogicalVolume*  supportTubeLV = new G4LogicalVolume(supportTube, supportMaterial, "ARICH.AerogelSupportPlate");
      //supportTubeLV->SetSensitiveDetector(m_sensitiveAero);

      unsigned nLayer = aeroGeo.getNLayers();
      G4Transform3D transform = G4Translate3D(0., 0., thick / 2.);

      // loop over layers
      double zLayer = 0;
      for (unsigned iLayer = 1; iLayer < nLayer + 1; iLayer++) {
        double layerThick = aeroGeo.getLayerThickness(iLayer);
        std::stringstream tileName;
        tileName << "aerogelTile_" << iLayer;

        G4Box* tileShape = new G4Box(tileName.str(), params[0] * 10. / 2., params[1] * 10. / 2., layerThick / 2.);

        G4Material* aeroMaterial = Materials::get(aeroGeo.getLayerMaterial(iLayer));

        G4LogicalVolume* tileLV = new G4LogicalVolume(tileShape, aeroMaterial, string("ARICH.") + tileName.str());

        G4ThreeVector transTile(params[2] * 10., params[3] * 10., (thick + layerThick - wallHeight) / 2. + zLayer);
        G4RotationMatrix Ra;
        Ra.rotateZ(params[4]);
        new G4PVPlacement(G4Transform3D(Ra, transTile), tileLV, string("ARICH.") + tileName.str(), aerogelPlaneLV, false, iLayer);

        zLayer += layerThick;
      }

      new G4PVPlacement(G4Translate3D(0., 0., -wallHeight / 2.), supportTubeLV, "ARICH.AerogelSupportPlate", aerogelPlaneLV, false, 1);

      return aerogelPlaneLV;
    }


    G4LogicalVolume* GeoARICHCreator::buildAerogelPlane(const ARICHGeometryConfig& detectorGeo)
    {

      const ARICHGeoAerogelPlane& aeroGeo = detectorGeo.getAerogelPlane();

      // support plane
      double rin = aeroGeo.getSupportInnerR() + 15.0; // remove when fix!!
      double rout = aeroGeo.getSupportOuterR() - 3.0; // remove when fix!!
      double thick = aeroGeo.getSupportThickness();
      double wallThick = aeroGeo.getWallThickness();
      double wallHeight = aeroGeo.getWallHeight();
      string supportMat = aeroGeo.getSupportMaterial();
      G4Material* supportMaterial = Materials::get(supportMat);
      G4Material* gapMaterial = Materials::get("Air"); // Air without refractive index (to kill photons, to mimic black paper around tile)
      G4Material* imgMaterial = Materials::get("ARICH_Air");
      // master volume
      G4Tubs* aerogelTube = new G4Tubs("aerogelTube", rin, rout, (thick + wallHeight + 0.5) / 2., 0, 2 * M_PI);
      G4LogicalVolume* aerogelPlaneLV = new G4LogicalVolume(aerogelTube, gapMaterial, "ARICH.AaerogelPlane");

      // support plate
      G4Tubs* supportTube = new G4Tubs("aeroSupportTube", rin, rout, thick / 2., 0, 2 * M_PI);
      G4LogicalVolume*  supportTubeLV = new G4LogicalVolume(supportTube, supportMaterial, "ARICH.AerogelSupportPlate");
      //supportTubeLV->SetSensitiveDetector(m_sensitiveAero);

      // imaginary tube after aerogel layers (used as volume to which tracks are extrapolated by ext module)
      double imgTubeLen = 0.5;
      G4Tubs* imgTube = new G4Tubs("imgTube", rin, rout, imgTubeLen / 2., 0, 2 * M_PI);
      G4LogicalVolume*  imgTubeLV = new G4LogicalVolume(imgTube, imgMaterial, "ARICH.AerogelImgPlate");
      imgTubeLV->SetSensitiveDetector(m_sensitiveAero);


      // read radiuses of aerogel slot aluminum walls
      std::vector<double> wallR;
      unsigned nRing = aeroGeo.getNRings();
      for (unsigned iRing = 1; iRing < nRing + 1; iRing++) {
        wallR.push_back(aeroGeo.getRingRadius(iRing));
      }

      unsigned nLayer = aeroGeo.getNLayers();
      double tileGap = aeroGeo.getTileGap();
      G4Transform3D transform = G4Translate3D(0., 0., (thick - imgTubeLen) / 2.);

      for (unsigned iRing = 0; iRing < nRing; iRing++) {

        // aluminum walls between tile rings (r wall)
        std::stringstream wallName;
        wallName << "supportWallR_" << iRing + 1;
        G4Tubs* supportWall = new G4Tubs(wallName.str().c_str(), wallR[iRing], wallR[iRing] + wallThick, wallHeight / 2., 0, 2 * M_PI);
        G4LogicalVolume* supportWallLV  = new G4LogicalVolume(supportWall, supportMaterial, string("ARICH.") + wallName.str().c_str());

        new G4PVPlacement(transform, supportWallLV, string("ARICH.") + wallName.str().c_str(), aerogelPlaneLV, false, 0);

        if (iRing == 0) continue;

        // place phi aluminum walls
        double dphi = aeroGeo.getRingDPhi(iRing);

        wallName.str("");
        wallName << "supportWallPhi_" << iRing + 1;
        G4Box* wall = new G4Box(wallName.str(), (wallR[iRing] - wallR[iRing - 1] - wallThick) / 2. - 1., thick / 2., wallHeight / 2.);
        G4LogicalVolume* wallLV = new G4LogicalVolume(wall, supportMaterial, string("ARICH.") + wallName.str());
        double r = (wallR[iRing - 1] + wallThick + wallR[iRing]) / 2.;
        double zLayer = 0;

        // loop over layers
        int iSlot = 1;
        for (unsigned iLayer = 1; iLayer < nLayer + 1; iLayer++) {
          double iphi = 0;
          double layerThick = aeroGeo.getLayerThickness(iLayer);

          std::stringstream tileName;
          tileName << "aerogelTile_" << iRing << "_" << iLayer;

          G4Tubs* tileShape = new G4Tubs(tileName.str(), wallR[iRing - 1] + wallThick + tileGap, wallR[iRing] - tileGap, layerThick / 2.,
                                         (tileGap + wallThick / 2.) / wallR[iRing], dphi - (2.*tileGap + wallThick) / wallR[iRing]);

          G4Material* aeroMaterial = Materials::get(aeroGeo.getLayerMaterial(iLayer));
          G4LogicalVolume* tileLV = new G4LogicalVolume(tileShape, aeroMaterial, string("ARICH.") + tileName.str());

          while (iphi < 2 * M_PI - 0.0001) {
            G4ThreeVector trans(r * cos(iphi), r * sin(iphi), (thick - imgTubeLen) / 2.);
            G4RotationMatrix Ra;
            Ra.rotateZ(iphi);

            if (iLayer == 1) new G4PVPlacement(G4Transform3D(Ra, trans), wallLV, string("ARICH.") + wallName.str(), aerogelPlaneLV, false,
                                                 iSlot);

            G4ThreeVector transTile(0, 0, (thick + layerThick - wallHeight - imgTubeLen) / 2. + zLayer);
            new G4PVPlacement(G4Transform3D(Ra, transTile), tileLV, string("ARICH.") + tileName.str(), aerogelPlaneLV, false, iSlot);
            iphi += dphi;
            iSlot++;
          }
          zLayer += layerThick;
        }
      }

      new G4PVPlacement(G4Translate3D(0., 0., -(wallHeight + imgTubeLen) / 2.), supportTubeLV, "ARICH.AerogelSupportPlate",
                        aerogelPlaneLV,
                        false, 1);

      new G4PVPlacement(G4Translate3D(0., 0., (wallHeight + thick) / 2.), imgTubeLV, "ARICH.AerogelImgPlate", aerogelPlaneLV, false, 1);

      return aerogelPlaneLV;

    }


    G4LogicalVolume* GeoARICHCreator::buildHAPD(const ARICHGeoHAPD& hapdGeo)
    {

      // get module materials
      string wallMat =  hapdGeo.getWallMaterial();
      string winMat =  hapdGeo.getWinMaterial();
      string apdMat =  hapdGeo.getAPDMaterial();
      string fillMat =  hapdGeo.getFillMaterial();
      string febMat =  hapdGeo.getFEBMaterial();
      G4Material* wallMaterial = Materials::get(wallMat);
      G4Material* windowMaterial = Materials::get(winMat);
      G4Material* apdMaterial = Materials::get(apdMat);
      G4Material* fillMaterial = Materials::get(fillMat);
      G4Material* febMaterial = Materials::get(febMat);
      G4Material* moduleFill = Materials::get("ARICH_Air");

      // check that module window material has specified refractive index
      double wref = getAvgRINDEX(windowMaterial);
      if (!wref) B2WARNING("Material '" << winMat <<
                             "', required for ARICH photon detector window as no specified refractive index. Continuing, but no photons in ARICH will be detected.");

      // get module dimensions
      const double hapdSizeX = hapdGeo.getSizeX();
      const double hapdSizeY = hapdGeo.getSizeY();
      const double hapdSizeZ = hapdGeo.getSizeZ();
      const double wallThick = hapdGeo.getWallThickness();
      const double winThick = hapdGeo.getWinThickness();
      const double apdSizeX = hapdGeo.getAPDSizeX();
      const double apdSizeY = hapdGeo.getAPDSizeY();
      const double apdSizeZ = hapdGeo.getAPDSizeZ();
      const double botThick =  wallThick;
      const double modHeight = hapdGeo.getModuleSizeZ();

      // module master volume
      G4Box* moduleBox = new G4Box("moduleBox", hapdSizeX / 2., hapdSizeY / 2., modHeight / 2.);
      G4LogicalVolume* lmoduleBox = new G4LogicalVolume(moduleBox, moduleFill, "ARICH.HAPDModule");

      // build HAPD box
      G4Box* hapdBox = new G4Box("hapdBox", hapdSizeX / 2., hapdSizeY / 2., hapdSizeZ / 2.);
      G4LogicalVolume* lhapdBox = new G4LogicalVolume(hapdBox, fillMaterial, "ARICH.HAPD");

      // build HAPD walls
      G4Box* tempBox2 = new G4Box("tempBox2", hapdSizeX / 2. - wallThick, hapdSizeY / 2. - wallThick,
                                  hapdSizeZ / 2. + 0.1); // Dont't care about "+0.1", needs to be there.
      G4SubtractionSolid* moduleWall = new G4SubtractionSolid("Box-tempBox", hapdBox, tempBox2);
      G4LogicalVolume* lmoduleWall = new G4LogicalVolume(moduleWall, wallMaterial, "ARICH.HAPDWall");
      setColor(*lmoduleWall, "rgb(1.0,0.0,0.0,1.0)");
      new G4PVPlacement(G4Transform3D(), lmoduleWall, "ARICH.HAPDWall", lhapdBox, false, 1);

      // build HAPD window
      G4Box* winBox = new G4Box("winBox", hapdSizeX / 2. - wallThick, hapdSizeY / 2. - wallThick, winThick / 2.);
      G4LogicalVolume* lmoduleWin = new G4LogicalVolume(winBox, windowMaterial, "ARICH.HAPDWindow");
      setColor(*lmoduleWin, "rgb(0.7,0.7,0.7,1.0)");
      lmoduleWin->SetSensitiveDetector(m_sensitive);
      G4Transform3D transform = G4Translate3D(0., 0., (-hapdSizeZ + winThick) / 2.);
      new G4PVPlacement(transform, lmoduleWin, "ARICH.HAPDWindow", lhapdBox, false, 1);

      // build module bottom
      G4Box* botBox = new G4Box("botBox", hapdSizeX / 2. - wallThick, hapdSizeY / 2. - wallThick, botThick / 2.);
      G4LogicalVolume* lmoduleBot = new G4LogicalVolume(botBox, wallMaterial, "ARICH.HAPDBottom");
      setColor(*lmoduleBot, "rgb(0.0,1.0,0.0,1.0)");
      G4Transform3D transform1 = G4Translate3D(0., 0., (hapdSizeZ - botThick) / 2.);
      new G4PVPlacement(transform1, lmoduleBot, "ARICH.HAPDBottom", lhapdBox, false, 1);

      // build apd
      G4Box* apdBox = new G4Box("apdBox", apdSizeX / 2., apdSizeY / 2., apdSizeZ / 2.);
      G4LogicalVolume* lApd = new G4LogicalVolume(apdBox, apdMaterial, "ARICH.HAPDApd");
      if (m_isBeamBkgStudy) lApd->SetSensitiveDetector(new BkgSensitiveDetector("ARICH", 1));

      // add APD surface optical properties
      Materials& materials = Materials::getInstance();

      G4OpticalSurface* optSurf = materials.createOpticalSurface(hapdGeo.getAPDSurface());

      new G4LogicalSkinSurface("apdSurface", lApd, optSurf);
      G4Transform3D transform2 = G4Translate3D(0., 0., (hapdSizeZ - apdSizeZ) / 2. - botThick);
      new G4PVPlacement(transform2, lApd, "ARICH.HAPDApd", lhapdBox, false, 1);

      // build FEB
      double febSizeX = hapdGeo.getFEBSizeX();
      double febSizeY = hapdGeo.getFEBSizeY();
      double febSizeZ = hapdGeo.getFEBSizeZ();
      G4Box* febBox = new G4Box("febBox", febSizeX / 2., febSizeY / 2., febSizeZ / 2.);
      G4LogicalVolume* lfeb = new G4LogicalVolume(febBox, febMaterial, "ARICH.HAPDFeb");
      if (m_isBeamBkgStudy) lfeb->SetSensitiveDetector(new BkgSensitiveDetector("ARICH"));
      setColor(*lfeb, "rgb(0.0,0.6,0.0,1.0)");
      G4Transform3D transform3 = G4Translate3D(0., 0., (modHeight - febSizeZ) / 2.);
      new G4PVPlacement(transform3, lfeb, "ARICH.HAPDFeb", lmoduleBox, false, 1);
      G4Transform3D transform4 = G4Translate3D(0., 0., - (modHeight - hapdSizeZ) / 2.);
      new G4PVPlacement(transform4, lhapdBox, "ARICH.HAPD", lmoduleBox, false, 1);

      return lmoduleBox;

    }

    G4LogicalVolume* GeoARICHCreator::buildDetectorPlane(const ARICHGeometryConfig& detectorGeo)
    {

      const ARICHGeoHAPD& hapdGeo =  detectorGeo.getHAPDGeometry();
      G4LogicalVolume* hapdLV = buildHAPD(hapdGeo);

      const ARICHGeoDetectorPlane& detGeo =  detectorGeo.getDetectorPlane();

      G4Tubs* detTube = new G4Tubs("detTube", detGeo.getRingR(1) - hapdGeo.getSizeX() * 1.4 / 2.,
                                   detGeo.getRingR(detGeo.getNRings()) + hapdGeo.getSizeX() * 1.4 / 2. , hapdGeo.getModuleSizeZ() / 2., 0, 2 * M_PI);
      G4LogicalVolume* detPlaneLV = new G4LogicalVolume(detTube, Materials::get("ARICH_Air"), "ARICH.detectorPlane");

      unsigned nSlots = detGeo.getNSlots();

      for (unsigned iSlot = 1; iSlot < nSlots + 1; iSlot++) {
        if (!m_modInfo->isInstalled(iSlot)) continue;
        double r = detGeo.getSlotR(iSlot);
        double phi = detGeo.getSlotPhi(iSlot);
        G4ThreeVector trans(r * cos(phi), r * sin(phi), 0);
        G4RotationMatrix Ra;
        Ra.rotateZ(phi);
        G4ThreeVector trans1(r * cos(phi), r * sin(phi), 0.0);
        new G4PVPlacement(G4Transform3D(Ra, trans1),  hapdLV, "ARICH.HAPDModule", detPlaneLV, false, iSlot);
      }

      return detPlaneLV;

    }

    G4LogicalVolume* GeoARICHCreator::buildDetectorSupportPlate(const ARICHGeometryConfig& detectorGeo)
    {

      const ARICHGeoDetectorPlane& detGeo =  detectorGeo.getDetectorPlane();

      G4Tubs* supportTube = new G4Tubs("supportTube", detGeo.getSupportInnerR(), detGeo.getSupportOuterR(),
                                       (detGeo.getSupportThickness() +  detGeo.getSupportBackWallHeight()) / 2., 0, 2 * M_PI);
      G4Material* supportMaterial = Materials::get(detGeo.getSupportMaterial());
      G4LogicalVolume* detSupportLV = new G4LogicalVolume(supportTube, supportMaterial, "ARICH.detectorSupportPlate");

      G4Tubs* supportPlate = new G4Tubs("supportPlate", detGeo.getSupportInnerR(), detGeo.getSupportOuterR(),
                                        detGeo.getSupportThickness() / 2., 0, 2 * M_PI);

      G4Box* hole = new G4Box("hole", detGeo.getModuleHoleSize() / 2., detGeo.getModuleHoleSize() / 2.,
                              detGeo.getSupportThickness() / 2.); // +1 for thickness for subtraction solid
      G4LogicalVolume* holeLV = new G4LogicalVolume(hole, Materials::get("Air"), "ARICH.detectorSupportHole");

      int nRings = detGeo.getNRings();
      std::vector<G4LogicalVolume*> hapdBackRadialWallLV;
      double backWallThick = detGeo.getSupportBackWallThickness();
      double backWallHeight =  detGeo.getSupportBackWallHeight();

      G4LogicalVolume* hapdSupportPlateLV = new G4LogicalVolume(supportPlate, supportMaterial, "hapdSupport");

      std::vector<double> wallR;
      wallR.assign(nRings + 1, 0);
      std::vector<double> thickR;
      thickR.assign(nRings + 1, 0);

      for (int i = 1; i < nRings; i++) {
        double rm1 = detGeo.getRingR(i);
        double rp1 = detGeo.getRingR(i + 1);
        wallR[i] = (rp1 + rm1) / 2.;
        if (i == 1) {
          wallR[0] = rm1 - (rp1 - rm1) / 2.;
        }
        if (i == nRings - 1) {
          wallR[i + 1] = rp1 + (rp1 - rm1) / 2.;
        }
      }

      for (int i = 0; i < nRings + 1; i++) {
        std::stringstream ringName1;
        ringName1 << "backWall_" << i;
        thickR[i] = backWallThick;
        if (i == nRings) {
          thickR[i] = 2.*backWallThick;
          wallR[i] = detGeo.getSupportOuterR() - thickR[i] / 2.;
        }
        G4Tubs* backTube = new G4Tubs("hapdBackRing", wallR[i] - thickR[i] / 2., wallR[i] + thickR[i] / 2., backWallHeight / 2., 0,
                                      2 * M_PI);
        G4LogicalVolume* hapdBackTubeLV = new G4LogicalVolume(backTube, supportMaterial, "backTube");
        G4Transform3D transform3 = G4Translate3D(0., 0., detGeo.getSupportThickness() / 2.);
        new G4PVPlacement(transform3,  hapdBackTubeLV, "backTube", detSupportLV, false, 1);
        if (i == 0) continue;

        G4Box* backRadial = new G4Box("backRadialBox", (wallR[i] - wallR[i - 1] - thickR[i] / 2. - thickR[i - 1] / 2.) / 2. - 1.,
                                      backWallThick / 2., backWallHeight / 2.);
        hapdBackRadialWallLV.push_back(new G4LogicalVolume(backRadial, supportMaterial, ringName1.str().c_str()));
      }

      G4SubtractionSolid* substraction = NULL;
      unsigned nSlots = detGeo.getNSlots();

      // drill holes in support plate
      for (unsigned iSlot = 1; iSlot < nSlots + 1; iSlot++) {
        unsigned iRing = detGeo.getSlotRing(iSlot);
        double r = (wallR[iRing] + wallR[iRing - 1]) / 2. - (thickR[iRing] - thickR[iRing - 1]) / 2.;

        double phi = detGeo.getSlotPhi(iSlot);
        G4ThreeVector trans(r * cos(phi), r * sin(phi), 0);
        G4RotationMatrix Ra;
        Ra.rotateZ(phi);
        new G4PVPlacement(G4Transform3D(Ra, trans),  holeLV, "hole", hapdSupportPlateLV, false, iSlot);
        if (substraction) substraction = new G4SubtractionSolid("Box+CylinderMoved", substraction, hole, G4Transform3D(Ra, trans));
        else substraction = new G4SubtractionSolid("Box+CylinderMoved", supportPlate, hole, G4Transform3D(Ra, trans));

        phi = phi + detGeo.getRingDPhi(iRing) / 2.;
        G4ThreeVector transBack(r * cos(phi), r * sin(phi), detGeo.getSupportThickness() / 2.);
        G4RotationMatrix RaBack;
        RaBack.rotateZ(phi);
        new G4PVPlacement(G4Transform3D(RaBack, transBack), hapdBackRadialWallLV[iRing - 1], "hapdBack", detSupportLV, false, iSlot);
      }

      // G4LogicalVolume* hapdSupportPlateLV = new G4LogicalVolume(substraction, supportMaterial, "hapdSupport");

      G4Transform3D transform3 = G4Translate3D(0., 0., - backWallHeight / 2.);
      new G4PVPlacement(transform3, hapdSupportPlateLV, "supportPlate",  detSupportLV, false, 1);

      // place electronics side neutron shield - for now hardcoded here, pending for more proper implementation!
      G4Box* shieldBox1 = new G4Box("shieldBox1", 20. / 2., 75. / 2.,  backWallHeight / 2.);
      G4Box* shieldBox2 = new G4Box("shieldBox2", 55. / 2., 40. / 2.,  backWallHeight / 2.);
      G4LogicalVolume* shield1 = new G4LogicalVolume(shieldBox1,  Materials::get("BoratedPoly"), "ARICH.FWDShield1");
      G4LogicalVolume* shield2 = new G4LogicalVolume(shieldBox2,  Materials::get("BoratedPoly"), "ARICH.FWDShield2");
      double dphi = 2 * M_PI / 36.;
      double r1 = wallR[0] - 15.;
      double r2 = wallR[0] - 15. - 20. / 2. - 55. / 2.;
      for (int i = 0; i < 36; i++) {
        double phi = (i + 0.5) * dphi;
        G4RotationMatrix rot;
        rot.rotateZ(phi);
        G4ThreeVector trans(r1 * cos(phi), r1 * sin(phi), detGeo.getSupportThickness() / 2.);
        G4ThreeVector trans1(r2 * cos(phi), r2 * sin(phi), detGeo.getSupportThickness() / 2.);
        new G4PVPlacement(G4Transform3D(rot, trans), shield1, "ARICH.FWDShield1", detSupportLV, false, i);
        new G4PVPlacement(G4Transform3D(rot, trans1), shield2, "ARICH.FWDShield2", detSupportLV, false, i);
      }

      return detSupportLV;
    }


    G4LogicalVolume* GeoARICHCreator::buildMirror(const ARICHGeometryConfig& detectorGeo)
    {

      const ARICHGeoMirrors& mirrGeo =  detectorGeo.getMirrors();

      // read m_config
      string mirrMat = mirrGeo.getMaterial();
      G4Material* mirrorMaterial = Materials::get(mirrMat);

      double mThick = mirrGeo.getPlateThickness();
      double mLength = mirrGeo.getPlateLength();
      double mWidth =  mirrGeo.getPlateWidth();

      G4Box* mirrPlate = new G4Box("mirrPlate", mThick / 2., mLength / 2., mWidth / 2.);

      G4LogicalVolume* lmirror = new G4LogicalVolume(mirrPlate, mirrorMaterial, "ARICH.mirrorPlate");

      Materials& materials = Materials::getInstance();

      G4OpticalSurface* optSurf = materials.createOpticalSurface(mirrGeo.getMirrorSurface());
      new G4LogicalSkinSurface("mirrorSurface", lmirror, optSurf);

      return lmirror;

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

    G4AssemblyVolume* GeoARICHCreator::makeJoint(G4Material* supportMaterial, const std::vector<double>& par)
    {

      int size = par.size();
      if (size < 4 || size > 8) B2ERROR("GeoARICHCreator::makeJoint: invalid number of joint wedge parameters");
      double lenx = par.at(0);
      double leny = par.at(1);
      double lenz = par.at(2);
      double thick = par.at(3);

      G4Box* wedgeBox1 = new G4Box("wedgeBox1", thick / 2., lenx / 2., leny / 2.);
      G4Box* wedgeBox2 = new G4Box("wedgeBox2", lenz / 2.,  lenx / 2., thick / 2.);

      G4LogicalVolume* wedgeBox1LV = new G4LogicalVolume(wedgeBox1, supportMaterial, "ARICH.supportWedge");
      G4LogicalVolume* wedgeBox2LV = new G4LogicalVolume(wedgeBox2, supportMaterial, "ARICH.supportWedge");

      G4AssemblyVolume* assemblyWedge = new G4AssemblyVolume();

      G4RotationMatrix Rm;
      G4ThreeVector Ta(0, 0, 0);
      G4Transform3D Tr;
      Tr = G4Transform3D(Rm, Ta);

      assemblyWedge->AddPlacedVolume(wedgeBox1LV, Tr);

      Ta.setX(lenz / 2. + thick / 2.);
      Ta.setZ(leny / 2. - thick / 2.);
      Tr = G4Transform3D(Rm, Ta);
      assemblyWedge->AddPlacedVolume(wedgeBox2LV, Tr);

      if (size == 4) return assemblyWedge;
      double edge = par.at(4);

      G4Box* wedgeBox3 = new G4Box("wedgeBox3", lenz / 2., edge / 2., thick / 2.);
      G4LogicalVolume* wedgeBox3LV = new G4LogicalVolume(wedgeBox3, supportMaterial, "ARICH.supportWedge");

      Ta.setZ(leny / 2. - thick - thick / 2.);
      Tr = G4Transform3D(Rm, Ta);
      assemblyWedge->AddPlacedVolume(wedgeBox3LV, Tr);

      G4Trap*  wedgeBoxTmp = new G4Trap("wedgeBoxTmp", thick, leny - 2 * thick, lenz, edge);
      G4LogicalVolume* wedgeBox4LV;
      if (size == 8) {
        G4Tubs*  wedgeBoxTmp1 = new G4Tubs("wedgeBoxTmp1", 0.0, par.at(5), thick, 0, 2.*M_PI);
        G4RotationMatrix rotHole;
        G4ThreeVector transHole(par.at(6), par.at(7), 0);
        G4SubtractionSolid* wedgeBox4 = new G4SubtractionSolid("wedgeBox4", wedgeBoxTmp, wedgeBoxTmp1, G4Transform3D(rotHole, transHole));
        wedgeBox4LV = new G4LogicalVolume(wedgeBox4, supportMaterial, "ARICH.supportWedge");
      } else wedgeBox4LV = new G4LogicalVolume(wedgeBoxTmp, supportMaterial, "ARICH.supportWedge");

      Rm.rotateX(-M_PI / 2.);
      Ta.setX(thick / 2. + edge / 4. + lenz / 4.);
      Ta.setZ(-thick / 2. - edge / 2.);
      Tr = G4Transform3D(Rm, Ta);
      assemblyWedge->AddPlacedVolume(wedgeBox4LV, Tr);

      return assemblyWedge;

    }

    // simple geometry for beamtest setup
    /*    void GeoARICHCreator::createSimple(const GearDir& content, G4LogicalVolume& topVolume)
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
    */

  } // namespace aricih
} // namespace belleII
