/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Leo Piilonen                                            *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
*************************************************************************/

#include <bklm/geometry/GeoBKLMCreator.h>
#include <bklm/simulation/SensitiveDetector.h>
#include "bklm/dbobjects/BKLMGeometryPar.h"

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Transform3D.hh>
#include <G4UserLimits.hh>
#include <G4VisAttributes.hh>
#include <G4String.hh>

using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace bklm {

    //-----------------------------------------------------------------
    //                 Register the Creator
    //-----------------------------------------------------------------

    geometry::CreatorFactory<GeoBKLMCreator> GeoBKLMFactory("BKLMCreator");

    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    GeoBKLMCreator::GeoBKLMCreator()
    {
      m_Sensitive = dynamic_cast<G4VSensitiveDetector*>(new SensitiveDetector(G4String("BKLM")));
      m_GeoPar = NULL;
      m_SectorDphi = 0.0;
      m_SectorDz = 0.0;
      m_RibShift = 0.0;
      m_CapSolid = NULL;
      m_CapLogical[0] = m_CapLogical[1] = NULL;
      m_InnerIronSolid = NULL;
      m_InnerIronLogical[0] = m_InnerIronLogical[1] = m_InnerIronLogical[2] = m_InnerIronLogical[3] = NULL;
      m_InnerAirSolid = NULL;
      m_InnerAirLogical[0] = m_InnerAirLogical[1] = m_InnerAirLogical[2] = m_InnerAirLogical[3] = NULL;
      m_SupportLogical[0] = m_SupportLogical[1] = NULL;
      m_BracketLogical = NULL;
      for (int j = 0; j < NLAYER; ++j) {
        m_LayerIronSolid[j] = NULL;
      }
      for (int j = 0; j < 2 * NLAYER; ++j) {
        m_LayerModuleLogical[j] = NULL;
        m_LayerGapSolid[j] = NULL;
      }
      for (int j = 0; j < 12 * NLAYER; ++j) {
        m_LayerIronLogical[j] = NULL;
        m_LayerGapLogical[j] = NULL;
      }
      m_SectorTube = NULL;
      for (int sector = 0; sector < NSECTOR; ++sector) {
        m_SectorLogical[0][sector] = NULL;
        m_SectorLogical[1][sector] = NULL;
      }
      m_MPPCHousingLogical = NULL;
      m_ReadoutContainerLogical = NULL;
      m_SolenoidTube = NULL;
      m_ScintLogicals.clear();
      m_VisAttributes.clear();
      m_VisAttributes.push_back(new G4VisAttributes(false)); // for "invisible"
      m_Names.clear();
    }

    GeoBKLMCreator::~GeoBKLMCreator()
    {
      delete m_Sensitive;
      m_ScintLogicals.clear();
      for (G4VisAttributes* visAttr : m_VisAttributes) delete visAttr;
      m_VisAttributes.clear();
      for (G4String* name : m_Names) delete name;
      m_Names.clear();
    }

    //-----------------------------------------------------------------
    //                 Build and place the BKLM
    //-----------------------------------------------------------------

    void GeoBKLMCreator::createGeometry(const BKLMGeometryPar& parameters, G4LogicalVolume& motherLogical, GeometryTypes)
    {

      m_GeoPar = GeometryPar::instance(parameters);
      m_SectorDphi = 2.0 * M_PI / m_GeoPar->getNSector();
      m_SectorDz = 0.5 * m_GeoPar->getHalfLength() * CLHEP::cm;
      m_RibShift = 0.5 * m_GeoPar->getRibThickness() * CLHEP::cm / sin(0.5 * m_SectorDphi);

      // Place BKLM envelope in mother volume
      G4Tubs* envelopeSolid =
        new G4Tubs("BKLM.EnvelopeSolid",
                   m_GeoPar->getSolenoidOuterRadius() * CLHEP::cm,
                   m_GeoPar->getOuterRadius() * CLHEP::cm / cos(0.5 * m_SectorDphi),
                   2.0 * m_SectorDz,
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* envelopeLogical =
        new G4LogicalVolume(envelopeSolid,
                            Materials::get("G4_AIR"),
                            "BKLM.EnvelopeLogical"
                           );
      envelopeLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      putEndsInEnvelope(envelopeLogical);
      new G4PVPlacement(G4TranslateZ3D(m_GeoPar->getOffsetZ() * CLHEP::cm) * G4RotateZ3D(m_GeoPar->getRotation() * CLHEP::rad),
                        envelopeLogical,
                        "BKLM.EnvelopePhysical",
                        &motherLogical,
                        false,
                        1,
                        false
                       );

    }

    void GeoBKLMCreator::putEndsInEnvelope(G4LogicalVolume* envelopeLogical)
    {
      G4Tubs* endSolid =
        new G4Tubs("BKLM.EndSolid",
                   m_GeoPar->getSolenoidOuterRadius() * CLHEP::cm,
                   m_GeoPar->getOuterRadius() * CLHEP::cm / cos(0.5 * m_SectorDphi),
                   m_SectorDz,
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* frontLogical =
        new G4LogicalVolume(endSolid,
                            Materials::get("G4_AIR"),
                            "BKLM.F_Logical"
                           );
      frontLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      putSectorsInEnd(frontLogical, BKLM_FORWARD);
      new G4PVPlacement(G4TranslateZ3D(m_SectorDz),
                        frontLogical,
                        "BKLM.F_Physical",
                        envelopeLogical,
                        false,
                        BKLM_FORWARD,
                        false
                       );
      G4LogicalVolume* backLogical =
        new G4LogicalVolume(endSolid,
                            Materials::get("G4_AIR"),
                            "BKLM.B_Logical"
                           );
      backLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      putSectorsInEnd(backLogical, BKLM_BACKWARD);
      new G4PVPlacement(G4TranslateZ3D(-m_SectorDz) * G4RotateY3D(M_PI),
                        backLogical,
                        "BKLM.B_Physical",
                        envelopeLogical,
                        false,
                        BKLM_BACKWARD,
                        false
                       );

    }

    void GeoBKLMCreator::putSectorsInEnd(G4LogicalVolume* endLogical, int fb)
    {
      if (m_SectorTube == NULL) {
        m_SectorTube =
          new G4Tubs("BKLM.SectorSolid",
                     m_GeoPar->getSolenoidOuterRadius() * CLHEP::cm,
                     m_GeoPar->getOuterRadius() * CLHEP::cm / cos(0.5 * m_SectorDphi),
                     m_SectorDz,
                     -0.5 * m_SectorDphi,
                     m_SectorDphi
                    );
      }
      char name[80] = "";
      for (int s = 0; s < m_GeoPar->getNSector(); ++s) {
        int sector = (fb == BKLM_FORWARD ? s : ((12 - s) % 8)) + 1;
        bool hasChimney = (fb == BKLM_BACKWARD) && (sector == CHIMNEY_SECTOR);
        bool hasInnerSupport = (sector <= m_GeoPar->getNSector() / 2 + 1);
        sprintf(name, "BKLM.%sSector%dLogical", (fb == BKLM_FORWARD ? "Forward" : "Backward"), sector);
        m_SectorLogical[fb - 1][sector - 1] =
          new G4LogicalVolume(m_SectorTube,
                              Materials::get("G4_AIR"),
                              name
                             );
        m_SectorLogical[fb - 1][sector - 1]->SetVisAttributes(m_VisAttributes.front()); // invisible
        putCapInSector(m_SectorLogical[fb - 1][sector - 1], hasChimney);
        putInnerRegionInSector(m_SectorLogical[fb - 1][sector - 1], hasInnerSupport, hasChimney);
        putLayersInSector(m_SectorLogical[fb - 1][sector - 1], fb, sector, hasChimney);
        new G4PVPlacement(G4RotateZ3D(m_SectorDphi * s),
                          m_SectorLogical[fb - 1][sector - 1],
                          physicalName(m_SectorLogical[fb - 1][sector - 1]),
                          endLogical,
                          false,
                          sector,
                          false
                         );
      }
    }

    void GeoBKLMCreator::putCapInSector(G4LogicalVolume* sectorLogical, bool hasChimney)
    {

      // Fill cap with iron and (aluminum) cables
      const CLHEP::Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(0, false) * CLHEP::cm;
      const double dyBrace = (hasChimney ? m_GeoPar->getBraceWidthChimney() : m_GeoPar->getBraceWidth()) * CLHEP::cm;
      const double dy = 0.25 * (m_GeoPar->getCablesWidth() * CLHEP::cm - dyBrace);
      const double dz = m_SectorDz - gapHalfSize.z();
      const double ri = m_GeoPar->getLayerInnerRadius(1) * CLHEP::cm + 2.0 * gapHalfSize.x();
      const double ro = m_GeoPar->getOuterRadius() * CLHEP::cm;
      if (m_CapSolid == NULL) {
        const double z[2] = { -dz, dz};
        const double rInner[2] = {ri, ri};
        const double rOuter[2] = {ro, ro};
        m_CapSolid =
          new G4Polyhedra("BKLM.CapSolid",
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z, rInner, rOuter
                         );
      }
      int newLvol = (hasChimney ? 1 : 0);
      G4String name = (hasChimney ? "BKLM.ChimneyCapLogical" : "BKLM.CapLogical");
      if (m_CapLogical[newLvol] == NULL) {
        m_CapLogical[newLvol] =
          new G4LogicalVolume(m_CapSolid,
                              Materials::get("G4_Fe"),
                              name
                             );
        m_CapLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
        name = (hasChimney ? "BKLM.ChimneyCablesSolid" : "BKLM.CablesSolid");
        G4Box* cablesSolid =
          new G4Box(name, 0.5 * (ro - ri), dy, dz);
        G4LogicalVolume* cablesLogical =
          new G4LogicalVolume(cablesSolid,
                              Materials::get("G4_Al"),
                              logicalName(cablesSolid)
                             );
        cablesLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
        new G4PVPlacement(G4Translate3D(0.5 * (ri + ro), -(0.5 * dyBrace + dy), 0.0),
                          cablesLogical,
                          physicalName(cablesLogical).append(G4String("_L")),
                          m_CapLogical[newLvol],
                          false,
                          1,
                          false
                         );
        new G4PVPlacement(G4Translate3D(0.5 * (ri + ro), +(0.5 * dyBrace + dy), 0.0),
                          cablesLogical,
                          physicalName(cablesLogical).append(G4String("_R")),
                          m_CapLogical[newLvol],
                          false,
                          2,
                          false
                         );
      }
      new G4PVPlacement(G4TranslateZ3D(m_SectorDz - dz),
                        m_CapLogical[newLvol],
                        physicalName(m_CapLogical[newLvol]),
                        sectorLogical,
                        false,
                        1,
                        false
                       );
    }

    void GeoBKLMCreator::putInnerRegionInSector(G4LogicalVolume* sectorLogical, bool hasInnerSupport, bool hasChimney)
    {

      // Fill inner region with iron
      if (m_InnerIronSolid == NULL) {
        const double r = m_GeoPar->getLayerInnerRadius(1) * CLHEP::cm;
        const double z[2] = { -m_SectorDz, +m_SectorDz};
        const double rInner[2] = {0.0, 0.0};
        const double rOuter[2] = {r, r};
        G4Polyhedra* innerIronPolygon =
          new G4Polyhedra("BKLM.InnerIronPolygon",
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z, rInner, rOuter
                         );
        m_InnerIronSolid =
          new G4SubtractionSolid("BKLM.InnerIronSolid",
                                 innerIronPolygon,
                                 getSolenoidTube()
                                );
      }
      int newLvol = (hasInnerSupport ? 2 : 0) + (hasChimney ? 1 : 0);
      if (m_InnerIronLogical[newLvol] == NULL) {
        m_InnerIronLogical[newLvol] =
          new G4LogicalVolume(m_InnerIronSolid,
                              Materials::get("G4_Fe"),
                              (hasChimney ? "BKLM.InnerIronChimneyLogical" : "BKLM.InnerIronLogical")
                             );
        m_InnerIronLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
        putVoidInInnerRegion(m_InnerIronLogical[newLvol], hasInnerSupport, hasChimney);
      }
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        m_InnerIronLogical[newLvol],
                        physicalName(m_InnerIronLogical[newLvol]),
                        sectorLogical,
                        false,
                        1,
                        false
                       );
    }

    void GeoBKLMCreator::putVoidInInnerRegion(G4LogicalVolume* innerIronLogical, bool hasInnerSupport, bool hasChimney)
    {

      // Carve out an air void from the inner-region iron, leaving only the ribs
      // at the azimuthal edges
      if (m_InnerAirSolid == NULL) {
        const double r = m_GeoPar->getLayerInnerRadius(1) * CLHEP::cm - m_RibShift;
        const double z[2] = { -m_SectorDz, +m_SectorDz};
        const double rInner[2] = {0.0, 0.0};
        const double rOuter[2] = {r, r};
        G4Polyhedra* innerAirPolygon =
          new G4Polyhedra("BKLM.InnerAirPolygon",
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z, rInner, rOuter
                         );
        m_InnerAirSolid =
          new G4SubtractionSolid("BKLM.InnerAirSolid",
                                 innerAirPolygon,
                                 getSolenoidTube(),
                                 G4TranslateX3D(-m_RibShift)
                                );
      }
      int newLvol = (hasInnerSupport ? 2 : 0) + (hasChimney ? 1 : 0);
      if (m_InnerAirLogical[newLvol] == NULL) {
        m_InnerAirLogical[newLvol] =
          new G4LogicalVolume(m_InnerAirSolid,
                              Materials::get("G4_AIR"),
                              (hasChimney ? "BKLM.InnerAirChimneyLogical" : "BKLM.InnerAirLogical")
                             );
        m_InnerAirLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
        if (hasInnerSupport) {
          putLayer1SupportInInnerVoid(m_InnerAirLogical[newLvol], hasChimney);
          putLayer1BracketsInInnerVoid(m_InnerAirLogical[newLvol], hasChimney);
        }
      }
      new G4PVPlacement(G4TranslateX3D(m_RibShift),
                        m_InnerAirLogical[newLvol],
                        physicalName(m_InnerAirLogical[newLvol]),
                        innerIronLogical,
                        false,
                        1,
                        false
                       );
    }

    void GeoBKLMCreator::putLayer1SupportInInnerVoid(G4LogicalVolume* innerAirLogical, bool hasChimney)
    {

      int newLvol = (hasChimney ? 1 : 0);
      const CLHEP::Hep3Vector size = m_GeoPar->getSupportPlateHalfSize(hasChimney) * CLHEP::cm;
      if (m_SupportLogical[newLvol] == NULL) {
        G4Box* supportBox =
          new G4Box((hasChimney ? "BKLM.ChimneySupportSolid" : "BKLM.SupportSolid"),
                    size.x(),
                    size.y(),
                    size.z()
                   );
        m_SupportLogical[newLvol] =
          new G4LogicalVolume(supportBox,
                              Materials::get("G4_Al"),
                              logicalName(supportBox)
                             );
        m_SupportLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
      }
      double dx = m_GeoPar->getLayerInnerRadius(1) * CLHEP::cm - size.x() - m_RibShift;
      double dz = size.z() - m_SectorDz;
      new G4PVPlacement(G4Translate3D(dx, 0.0, dz),
                        m_SupportLogical[newLvol],
                        physicalName(m_SupportLogical[newLvol]),
                        innerAirLogical,
                        false,
                        1,
                        false
                       );
    }

    void GeoBKLMCreator::putLayer1BracketsInInnerVoid(G4LogicalVolume* innerAirLogical, bool hasChimney)
    {

      if (m_BracketLogical == NULL) {
        const CLHEP::Hep3Vector size = m_GeoPar->getSupportPlateHalfSize(hasChimney) * CLHEP::cm;
        const double dz = 0.5 * m_GeoPar->getBracketLength() * CLHEP::cm;
        const double r = m_GeoPar->getLayerInnerRadius(1) * CLHEP::cm - m_RibShift - 2.0 * size.x();
        const double bracketShift = m_GeoPar->getBracketRibThickness() * CLHEP::cm / sin(0.5 * m_SectorDphi);
        const double z[2] = { -dz, +dz};
        const double rInner[2] = {0.0, 0.0};
        const double rOuter[2] = {r, r};
        const double r1 = m_GeoPar->getBracketInnerRadius() * CLHEP::cm - m_RibShift;
        const double z1[2] = { -(dz + 0.5 * CLHEP::cm), dz + 0.5 * CLHEP::cm};
        const double rOuter1[2] = {r1, r1};
        const double z2[2] = { -(dz + 1.0 * CLHEP::cm), dz + 1.0 * CLHEP::cm};
        const double dzBracket = m_GeoPar->getBracketThickness() * CLHEP::cm;
        const double drBracket = dzBracket + bracketShift;
        const double rOuter2[2] = {r - drBracket, r - drBracket};
        const double z3[2] = { -dzBracket, dzBracket};
        const double cutoutDphi = m_GeoPar->getBracketCutoutDphi() * CLHEP::rad;
        G4Polyhedra* bracketPolygon =
          new G4Polyhedra("BKLM.BracketPolygon",
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z, rInner, rOuter
                         );
        G4Polyhedra* bracketCutout1 =
          new G4Polyhedra("BKLM.BracketCutout1",
                          -0.5 * cutoutDphi,
                          cutoutDphi,
                          2,  // two sides
                          2, z1, rInner, rOuter1
                         );
        G4Polyhedra* bracketCutout2 =
          new G4Polyhedra("BKLM.BracketCutout2",
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z2, rInner, rOuter2
                         );
        G4Polyhedra* bracketCutout3 =
          new G4Polyhedra("BKLM.BracketCutout3",
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z3, rInner, rOuter2
                         );
        G4Box* bracketCutout4 =
          new G4Box("BKLM.BracketCutout4",
                    rOuter[1] + 1.0 * CLHEP::cm,
                    rOuter[1] * tan(0.5 * m_SectorDphi) - m_GeoPar->getBracketWidth() * CLHEP::cm,
                    z[1] + 1.5 * CLHEP::cm
                   );
        G4VSolid* polygon1 =
          new G4SubtractionSolid("BKLM.BracketPolygon1",
                                 bracketPolygon,
                                 bracketCutout4
                                );
        G4VSolid* polygon2 =
          new G4SubtractionSolid("BKLM.BracketPolygon2",
                                 bracketCutout2,
                                 bracketCutout3
                                );
        G4VSolid* polygon3 =
          new G4SubtractionSolid("BKLM.BracketPolygon3",
                                 polygon1,
                                 polygon2,
                                 G4TranslateX3D(bracketShift)
                                );
        G4VSolid* bracketSolid =
          new G4SubtractionSolid("BKLM.BracketSolid",
                                 polygon3,
                                 bracketCutout1
                                );
        m_BracketLogical =
          new G4LogicalVolume(bracketSolid,
                              Materials::get("G4_Al"),
                              "BKLM.BracketLogical"
                             );
        m_VisAttributes.push_back(new G4VisAttributes(true));
        m_VisAttributes.back()->SetColour(0.6, 0.6, 0.6);
        m_BracketLogical->SetVisAttributes(m_VisAttributes.back());
      }
      char name[80] = "";
      for (int bracket = 0; bracket < (hasChimney ? 2 : 3); ++bracket) {
        sprintf(name, "BKLM.Bracket%d%sPhysical", bracket, (hasChimney ? "Chimney" : ""));
        new G4PVPlacement(G4TranslateZ3D(m_GeoPar->getBracketZPosition(bracket, hasChimney) * CLHEP::cm),
                          m_BracketLogical,
                          name,
                          innerAirLogical,
                          false,
                          bracket,
                          false
                         );
      }
    }

    void GeoBKLMCreator::putLayersInSector(G4LogicalVolume* sectorLogical, int fb, int sector, bool hasChimney)
    {

      const double dz = 0.5 * m_GeoPar->getGapLength() * CLHEP::cm;
      const double z[2] = { -dz, +dz };
      char name[80] = "";
      for (int layer = 1; layer <= m_GeoPar->getNLayer(); ++layer) {
        // Fill layer with iron
        if (m_LayerIronSolid[layer - 1] == NULL) {
          const double ri = m_GeoPar->getLayerInnerRadius(layer) * CLHEP::cm;
          const double ro = m_GeoPar->getLayerOuterRadius(layer) * CLHEP::cm;
          const double rInner[2] = {ri, ri};
          const double rOuter[2] = {ro, ro};
          sprintf(name, "BKLM.Layer%02dIronSolid", layer);
          m_LayerIronSolid[layer - 1] =
            new G4Polyhedra(name,
                            -0.5 * m_SectorDphi,
                            m_SectorDphi,
                            1,
                            2, z, rInner, rOuter
                           );
        }
        const Module* module = m_GeoPar->findModule(fb == BKLM_FORWARD, sector, layer);
        bool isFlipped = module->isFlipped();
        int newLvol = NLAYER * ((isFlipped ? 2 : 0) + (hasChimney ? 1 : 0)) + (layer - 1);
        int s1 = sector - 1;
        int s2 = m_GeoPar->getNSector() / 2;
        if (s1 % s2 == 0) {
        } else if (s1 > s2) {
          newLvol += NLAYER * 4;
        } else {
          newLvol += NLAYER * 8;
        }
        if (m_LayerIronLogical[newLvol] == NULL) {
          sprintf(name, "BKLM.Layer%02d%s%sIronLogical", layer, (isFlipped ? "Flipped" : ""), (hasChimney ? "Chimney" : ""));
          m_LayerIronLogical[newLvol] =
            new G4LogicalVolume(m_LayerIronSolid[layer - 1],
                                Materials::get("G4_Fe"),
                                name
                               );
          m_LayerIronLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
          putModuleInLayer(m_LayerIronLogical[newLvol], fb, sector, layer, hasChimney, isFlipped, newLvol);
          if (hasChimney) {
            putChimneyInLayer(m_LayerIronLogical[newLvol], layer);
          }
        }
        new G4PVPlacement(G4TranslateZ3D(dz - m_SectorDz),
                          m_LayerIronLogical[newLvol],
                          physicalName(m_LayerIronLogical[newLvol]),
                          sectorLogical,
                          false,
                          layer,
                          false
                         );

      }
    }

    void GeoBKLMCreator::putChimneyInLayer(G4LogicalVolume* layerIronLogical, int layer)
    {
      CLHEP::Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, true) * CLHEP::cm;
      CLHEP::Hep3Vector chimneyHalfSize = m_GeoPar->getChimneyHalfSize(layer) * CLHEP::cm;
      CLHEP::Hep3Vector chimneyPosition = m_GeoPar->getChimneyPosition(layer) * CLHEP::cm;
      gapHalfSize.setY(0.5 * (gapHalfSize.y() - chimneyHalfSize.y()));
      gapHalfSize.setZ(chimneyHalfSize.z() + 0.5 * m_GeoPar->getChimneyCoverThickness() * CLHEP::cm);
      double dx = m_GeoPar->getGapMiddleRadius(layer) * CLHEP::cm;
      double dy = gapHalfSize.y() + chimneyHalfSize.y();
      double dz = 0.5 * m_GeoPar->getGapLength() * CLHEP::cm - gapHalfSize.z();
      char name[80] = "";
      // Fill the two chimney gaps with air
      sprintf(name, "BKLM.Layer%02dGapChimneyBox", layer);
      G4Box* gapBox =
        new G4Box(name,
                  gapHalfSize.x(), gapHalfSize.y(), gapHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dGapChimneyLogical", layer);
      G4LogicalVolume* gapLogical =
        new G4LogicalVolume(gapBox,
                            Materials::get("G4_AIR"),
                            name
                           );
      gapLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      sprintf(name, "BKLM.Layer%02dLeftGapChimneyPhysical", layer);
      new G4PVPlacement(G4Translate3D(dx, -dy, dz),
                        gapLogical,
                        name,
                        layerIronLogical,
                        false,
                        0,
                        false
                       );
      sprintf(name, "BKLM.Layer%02dRightGapChimneyPhysical", layer);
      new G4PVPlacement(G4Translate3D(dx, +dy, dz),
                        gapLogical,
                        name,
                        layerIronLogical,
                        false,
                        1,
                        false
                       );
      // Fill chimney with air
      sprintf(name, "BKLM.Layer%02dChimneyBox", layer);
      G4Box* chimneyBox =
        new G4Box(name,
                  chimneyHalfSize.x(), chimneyHalfSize.y(), chimneyHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dChimneyLogical", layer);
      G4LogicalVolume* chimneyLogical =
        new G4LogicalVolume(chimneyBox,
                            Materials::get("G4_AIR"),
                            name
                           );
      chimneyLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      // Place coaxial tubes in chimney
      G4Tubs* housingTube =
        new G4Tubs("BKLM.ChimneyHousingTube",
                   m_GeoPar->getChimneyHousingInnerRadius() * CLHEP::cm,
                   m_GeoPar->getChimneyHousingOuterRadius() * CLHEP::cm,
                   chimneyHalfSize.x(),
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* housingLogical =
        new G4LogicalVolume(housingTube,
                            Materials::get("G4_Fe"),
                            "BKLM.ChimneyHousingLogical"
                           );
      m_VisAttributes.push_back(new G4VisAttributes(true));
      m_VisAttributes.back()->SetColour(0.4, 0.4, 0.4);
      housingLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4RotateY3D(M_PI_2),
                        housingLogical,
                        "BKLM.ChimneyHousingPhysical",
                        chimneyLogical,
                        false,
                        0,
                        false
                       );
      G4Tubs* shieldTube =
        new G4Tubs("BKLM.ChimneyShieldTube",
                   m_GeoPar->getChimneyShieldInnerRadius() * CLHEP::cm,
                   m_GeoPar->getChimneyShieldOuterRadius() * CLHEP::cm,
                   chimneyHalfSize.x(),
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* shieldLogical =
        new G4LogicalVolume(shieldTube,
                            Materials::get("G4_Fe"),
                            "BKLM.ChimneyShieldLogical"
                           );
      shieldLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4RotateY3D(M_PI_2),
                        shieldLogical,
                        "BKLM.ChimneyShieldPhysical",
                        chimneyLogical,
                        false,
                        0,
                        false
                       );
      G4Tubs* pipeTube =
        new G4Tubs("BKLM.ChimneyPipeTube",
                   m_GeoPar->getChimneyPipeInnerRadius() * CLHEP::cm,
                   m_GeoPar->getChimneyPipeOuterRadius() * CLHEP::cm,
                   chimneyHalfSize.x(),
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* pipeLogical =
        new G4LogicalVolume(pipeTube,
                            Materials::get("G4_Fe"),
                            "BKLM.ChimneyPipeLogical"
                           );
      m_VisAttributes.push_back(new G4VisAttributes(true));
      m_VisAttributes.back()->SetColour(0.6, 0.6, 0.6);
      pipeLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4RotateY3D(M_PI_2),
                        pipeLogical,
                        "BKLM.ChimneyPipePhysical",
                        chimneyLogical,
                        false,
                        0,
                        false
                       );
      sprintf(name, "BKLM.Layer%02dChimneyPhysical", layer);
      new G4PVPlacement(G4Translate3D(chimneyPosition),
                        chimneyLogical,
                        name,
                        layerIronLogical,
                        false,
                        0,
                        false
                       );
    }

    void GeoBKLMCreator::putModuleInLayer(G4LogicalVolume* layerIronLogical, int fb, int sector, int layer, bool hasChimney,
                                          bool isFlipped, int newLvol)
    {
      const CLHEP::Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, hasChimney) * CLHEP::cm;
      const CLHEP::Hep3Vector moduleHalfSize = m_GeoPar->getModuleHalfSize(layer, hasChimney) * CLHEP::cm;
      char name[80] = "";
      // Fill gap with air
      int modLvol = (hasChimney ? NLAYER : 0) + (layer - 1);
      if (m_LayerModuleLogical[modLvol] == NULL) {
        // Module is aluminum (but interior will be filled)
        sprintf(name, "BKLM.Layer%02d%sModuleSolid", layer, (hasChimney ? "Chimney" : ""));
        G4Box* moduleBox =
          new G4Box(name,
                    moduleHalfSize.x(), moduleHalfSize.y(), moduleHalfSize.z()
                   );
        m_LayerModuleLogical[modLvol] =
          new G4LogicalVolume(moduleBox,
                              Materials::get("G4_Al"),
                              logicalName(moduleBox)
                             );
        m_LayerModuleLogical[modLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
        sprintf(name, "BKLM.Layer%02d%sModuleInteriorSolid1", layer, (hasChimney ? "Chimney" : ""));
        const CLHEP::Hep3Vector interiorHalfSize1 = m_GeoPar->getModuleInteriorHalfSize1(layer, hasChimney) * CLHEP::cm;
        G4Box* interiorBox1 =
          new G4Box(name,
                    interiorHalfSize1.x(), interiorHalfSize1.y(), interiorHalfSize1.z()
                   );
        sprintf(name, "BKLM.Layer%02d%sModuleInteriorSolid2", layer, (hasChimney ? "Chimney" : ""));
        const CLHEP::Hep3Vector interiorHalfSize2 = m_GeoPar->getModuleInteriorHalfSize2(layer, hasChimney) * CLHEP::cm;
        G4Box* interiorBox2 =
          new G4Box(name,
                    interiorHalfSize2.x(), interiorHalfSize2.y(), interiorHalfSize2.z()
                   );
        sprintf(name, "BKLM.Layer%02d%sModuleInteriorSolid", layer, (hasChimney ? "Chimney" : ""));
        G4UnionSolid* interiorUnion =
          new G4UnionSolid(name, interiorBox1, interiorBox2);
        G4LogicalVolume* interiorLogical =
          new G4LogicalVolume(interiorUnion,
                              Materials::get((m_GeoPar->hasRPCs(layer) ? "RPCReadout" : "G4_POLYSTYRENE")),
                              logicalName(interiorUnion)
                             );
        interiorLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
        if (m_GeoPar->hasRPCs(layer)) {
          putRPCsInInterior(interiorLogical, layer, hasChimney);
        } else {
          putScintsInInterior(interiorLogical, fb, sector, layer, hasChimney);
        }
        new G4PVPlacement(G4TranslateZ3D(0.0),
                          interiorLogical,
                          physicalName(interiorLogical),
                          m_LayerModuleLogical[modLvol],
                          false,
                          0,
                          false
                         );
        sprintf(name, "BKLM.Layer%02d%sGapSolid", layer, (hasChimney ? "Chimney" : ""));
        m_LayerGapSolid[modLvol] =
          new G4Box(name,
                    gapHalfSize.x(), gapHalfSize.y(), gapHalfSize.z()
                   );
      }
      if (m_LayerGapLogical[newLvol] == NULL) {
        sprintf(name, "BKLM.Layer%02d%s%sGapLogical", layer, (isFlipped ? "Flipped" : ""), (hasChimney ? "Chimney" : ""));
        m_LayerGapLogical[newLvol] =
          new G4LogicalVolume(m_LayerGapSolid[modLvol],
                              Materials::get("G4_AIR"),
                              name
                             );
        m_LayerGapLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
      }
      double dx = (m_GeoPar->getModuleMiddleRadius(layer) - m_GeoPar->getGapMiddleRadius(layer)) * CLHEP::cm;
      // Module is closer to IP within gap if in the upper octants, farther from IP within gap if in the lower octants,
      // or in the middle of the gap if in the side octants.
      int s1 = sector - 1;
      int s2 = m_GeoPar->getNSector() / 2;
      if (s1 % s2 == 0) {
        dx = 0.0;
      } else if (s1 > s2) {
        dx = -dx;
      }
      double dz = moduleHalfSize.z() - gapHalfSize.z();
      G4Transform3D displacedGeo = m_GeoPar->getModuleDisplacedGeo(fb == BKLM_FORWARD, sector, layer);
      new G4PVPlacement(G4Translate3D(dx, 0.0, dz) * G4RotateZ3D(isFlipped ? M_PI : 0.0) * displacedGeo,
                        m_LayerModuleLogical[modLvol],
                        physicalName(m_LayerModuleLogical[modLvol]),
                        m_LayerGapLogical[newLvol],
                        false,
                        0,
                        false
                       );
      dz = gapHalfSize.z() - 0.5 * m_GeoPar->getGapLength() * CLHEP::cm;
      new G4PVPlacement(G4Translate3D(m_GeoPar->getGapMiddleRadius(layer) * CLHEP::cm, 0.0, dz),
                        m_LayerGapLogical[newLvol],
                        physicalName(m_LayerGapLogical[newLvol]),
                        layerIronLogical,
                        false,
                        layer,
                        false
                       );
    }

    void GeoBKLMCreator::putRPCsInInterior(G4LogicalVolume* interiorLogical, int layer, bool hasChimney)
    {
      char name[80] = "";
      // Place electrode inside the module's interior
      sprintf(name, "BKLM.Layer%02d%sElectrodeSolid", layer, (hasChimney ? "Chimney" : ""));
      const CLHEP::Hep3Vector electrodeHalfSize = m_GeoPar->getElectrodeHalfSize(layer, hasChimney) * CLHEP::cm;
      G4Box* electrodeBox =
        new G4Box(name,
                  electrodeHalfSize.x(), electrodeHalfSize.y(), electrodeHalfSize.z()
                 );
      G4LogicalVolume* electrodeLogical =
        new G4LogicalVolume(electrodeBox,
                            Materials::get("G4_GLASS_PLATE"),
                            logicalName(electrodeBox)
                           );
      electrodeLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      // Place two gas volumes inside electrodes
      sprintf(name, "BKLM.Layer%02d%sGasSolid", layer, (hasChimney ? "Chimney" : ""));
      const CLHEP::Hep3Vector gasHalfSize = m_GeoPar->getGasHalfSize(layer, hasChimney) * CLHEP::cm;
      G4Box* gasBox =
        new G4Box(name,
                  gasHalfSize.x(), gasHalfSize.y(), gasHalfSize.z()
                 );
      G4LogicalVolume* gasLogical =
        new G4LogicalVolume(gasBox,
                            Materials::get("RPCGas"),
                            logicalName(gasBox),
                            0, // use global field manager
                            m_Sensitive, // this is the only sensitive volume in BKLM
                            0 // no user limits
                           );
      m_VisAttributes.push_back(new G4VisAttributes(true));
      m_VisAttributes.back()->SetColour(1.0, 0.5, 0.0);
      gasLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4TranslateX3D(-0.5 * electrodeHalfSize.x()),
                        gasLogical,
                        physicalName(gasLogical),
                        electrodeLogical,
                        false,
                        BKLM_INNER,
                        false
                       );

      new G4PVPlacement(G4TranslateX3D(+0.5 * electrodeHalfSize.x()),
                        gasLogical,
                        physicalName(gasLogical),
                        electrodeLogical,
                        false,
                        BKLM_OUTER,
                        false
                       );
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        electrodeLogical,
                        physicalName(electrodeLogical),
                        interiorLogical,
                        false,
                        1,
                        false
                       );
    }

    void GeoBKLMCreator::putScintsInInterior(G4LogicalVolume* interiorLogical, int fb, int sector, int layer, bool hasChimney)
    {
      char name[80] = "";
      sprintf(name, "BKLM.Layer%02d%sAirSolid", layer, (hasChimney ? "Chimney" : ""));
      const CLHEP::Hep3Vector airHalfSize = m_GeoPar->getAirHalfSize(layer, hasChimney) * CLHEP::cm;
      G4Box* airBox =
        new G4Box(name,
                  airHalfSize.x(), airHalfSize.y(), airHalfSize.z()
                 );
      G4LogicalVolume* airLogical =
        new G4LogicalVolume(airBox,
                            Materials::get("G4_AIR"),
                            logicalName(airBox)
                           );
      airLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      sprintf(name, "BKLM.Layer%02d%sScintEnvelopeSolid", layer, (hasChimney ? "Chimney" : ""));
      double mppcHousingHalfLength = m_GeoPar->getMPPCHousingHalfLength() * CLHEP::cm;
      const CLHEP::Hep3Vector envelopeHalfSize = m_GeoPar->getScintEnvelopeHalfSize(layer, hasChimney) * CLHEP::cm +
                                                 CLHEP::Hep3Vector(0.0, mppcHousingHalfLength, mppcHousingHalfLength);
      G4Box* scintEnvelopeBox =
        new G4Box(name,
                  envelopeHalfSize.x(), envelopeHalfSize.y(), envelopeHalfSize.z()
                 );
      G4LogicalVolume* innerEnvelopeLogical =
        new G4LogicalVolume(scintEnvelopeBox,
                            Materials::get("G4_AIR"),
                            logicalName(scintEnvelopeBox)
                           );
      innerEnvelopeLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      double scintHalfHeight = m_GeoPar->getScintHalfHeight() * CLHEP::cm;
      double scintHalfWidth  = m_GeoPar->getScintHalfWidth() * CLHEP::cm;

      int envelopeOffsetSign = m_GeoPar->getScintEnvelopeOffsetSign(layer);
      const Module* module = m_GeoPar->findModule(fb == BKLM_FORWARD, sector, layer);
      for (int scint = 1; scint <= m_GeoPar->getNPhiScints(layer); ++scint) {
        double scintHalfLength = module->getPhiScintHalfLength(scint) * CLHEP::cm;
        double scintPosition   = module->getPhiScintPosition(scint) * CLHEP::cm;
        G4LogicalVolume* scintLogical = getScintLogical(scintHalfHeight, scintHalfWidth, scintHalfLength,
                                                        mppcHousingHalfLength);
        new G4PVPlacement(G4Translate3D(0.0, scintPosition + mppcHousingHalfLength * envelopeOffsetSign, 0.0),
                          scintLogical,
                          physicalName(scintLogical),
                          innerEnvelopeLogical,
                          false,
                          scint,
                          false
                         );
      }
      G4LogicalVolume* outerEnvelopeLogical =
        new G4LogicalVolume(scintEnvelopeBox,
                            Materials::get("G4_AIR"),
                            logicalName(scintEnvelopeBox)
                           );
      outerEnvelopeLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      for (int scint = 1; scint <= m_GeoPar->getNZScints(hasChimney); ++scint) {
        double scintHalfLength = module->getZScintHalfLength(scint) * CLHEP::cm;
        double scintOffset     = module->getZScintOffset(scint) * CLHEP::cm;
        double scintPosition   = module->getZScintPosition(scint) * CLHEP::cm;
        G4LogicalVolume* scintLogical = getScintLogical(scintHalfHeight, scintHalfWidth, scintHalfLength,
                                                        mppcHousingHalfLength);
        new G4PVPlacement(G4Translate3D(0.0, scintOffset, scintPosition - mppcHousingHalfLength) * G4RotateX3D(M_PI_2 * envelopeOffsetSign),
                          scintLogical,
                          physicalName(scintLogical),
                          outerEnvelopeLogical,
                          false,
                          scint,
                          false
                         );
      }
      CLHEP::Hep3Vector envelopeOffset = m_GeoPar->getScintEnvelopeOffset(layer, hasChimney) * CLHEP::cm +
                                         CLHEP::Hep3Vector(0.0, -mppcHousingHalfLength, mppcHousingHalfLength);
      new G4PVPlacement(G4Translate3D(-envelopeHalfSize.x(), envelopeOffset.y() * envelopeOffsetSign, envelopeOffset.z()),
                        innerEnvelopeLogical,
                        physicalName(innerEnvelopeLogical),
                        airLogical,
                        false,
                        BKLM_INNER,
                        false
                       );
      new G4PVPlacement(G4Translate3D(+envelopeHalfSize.x(), envelopeOffset.y() * envelopeOffsetSign, envelopeOffset.z()),
                        outerEnvelopeLogical,
                        physicalName(outerEnvelopeLogical),
                        airLogical,
                        false,
                        BKLM_OUTER,
                        false
                       );

      // Place readout carriers and preamplifiers along module perimeter
      double containerHalfSizeZ = m_GeoPar->getReadoutContainerHalfSize().z() * CLHEP::cm;
      G4LogicalVolume* readoutContainerLogical = getReadoutContainerLogical();
      for (int station = 1; station <= m_GeoPar->getNReadoutStation(); ++station) {
        double stationPosition = m_GeoPar->getReadoutStationPosition(station) * CLHEP::cm;
        G4Transform3D xform;
        if (m_GeoPar->getReadoutStationIsPhi(station)) {
          xform = G4Translate3D(0.0, stationPosition, airHalfSize.z() - containerHalfSizeZ);
        } else {
          xform = G4Translate3D(0.0,
                                (containerHalfSizeZ - airHalfSize.y()) * envelopeOffsetSign,
                                airHalfSize.z() + stationPosition
                               ) * G4RotateX3D(M_PI_2 * envelopeOffsetSign);
          if (fabs(xform.getTranslation().z()) > airHalfSize.z()) continue; // don't place all z-readout stations in chimney module
        }
        sprintf(name, "BKLM.ReadoutContainer%dLayer%02d%sPhysical", station, layer, (hasChimney ? "Chimney" : ""));
        new G4PVPlacement(xform,
                          readoutContainerLogical,
                          name,
                          airLogical,
                          false,
                          station,
                          false
                         );
      }

      // Place the air container with scints, MPPCs and preamps in the interior container
      new G4PVPlacement(G4TranslateX3D(m_GeoPar->getPolystyreneOffsetX() * CLHEP::cm),
                        airLogical,
                        physicalName(airLogical),
                        interiorLogical,
                        false,
                        1,
                        false
                       );
    }

    G4LogicalVolume* GeoBKLMCreator::getScintLogical(double dx, double dy, double dz, double dzMPPC)
    {

      int newLvol = 1;
      for (std::vector<G4LogicalVolume*>::iterator iLvol = m_ScintLogicals.begin(); iLvol != m_ScintLogicals.end(); ++iLvol) {
        G4Box* box = (G4Box*)((*iLvol)->GetSolid());
        if ((std::fabs(box->GetXHalfLength() - dx) < 1.0E-4 * CLHEP::cm) &&
            (std::fabs(box->GetYHalfLength() - dy) < 1.0E-4 * CLHEP::cm) &&
            (std::fabs(box->GetZHalfLength() - dz - dzMPPC) < 1.0E-4 * CLHEP::cm)) { return *iLvol; }
        newLvol++;
      }
      char name[80] = "";
      sprintf(name, "BKLM.ScintType%dSolid", newLvol);
      G4Box* scintBox = new G4Box(name, dx, dy, dz + dzMPPC);
      G4LogicalVolume* scintLogical =
        new G4LogicalVolume(scintBox, Materials::get("G4_POLYSTYRENE"), logicalName(scintBox));
      m_ScintLogicals.push_back(scintLogical);
      scintLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      sprintf(name, "BKLM.ScintType%dAirSolid", newLvol);
      G4Box* scintAirBox = new G4Box(name, dx, dy, dzMPPC);
      G4LogicalVolume* scintAirLogical =
        new G4LogicalVolume(scintAirBox, Materials::get("G4_AIR"), logicalName(scintAirBox));
      scintAirLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      double dxTiO2 = m_GeoPar->getScintTiO2ThicknessTop() * CLHEP::cm;
      double dyTiO2 = m_GeoPar->getScintTiO2ThicknessSide() * CLHEP::cm;
      sprintf(name, "BKLM.ScintActiveType%dSolid", newLvol);
      G4Box* activeBox = new G4Box(name, dx - dxTiO2, dy - dyTiO2, dz);
      G4LogicalVolume* activeLogical =
        new G4LogicalVolume(activeBox, Materials::get("G4_POLYSTYRENE"), logicalName(activeBox), 0, m_Sensitive, 0);
      m_VisAttributes.push_back(new G4VisAttributes(true));
      m_VisAttributes.back()->SetColour(1.0, 0.5, 0.0);
      activeLogical->SetVisAttributes(m_VisAttributes.back());
      sprintf(name, "BKLM.ScintBoreType%dSolid", newLvol);
      G4Tubs* boreTube = new G4Tubs(name, 0.0, m_GeoPar->getScintBoreRadius() * CLHEP::cm, dz, 0.0, 2.0 * M_PI);
      G4LogicalVolume* scintBoreLogical =
        new G4LogicalVolume(boreTube, Materials::get("G4_AIR"), logicalName(boreTube));
      scintBoreLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      sprintf(name, "BKLM.ScintFiberType%dSolid", newLvol);
      G4Tubs* fiberTube = new G4Tubs(name, 0.0, m_GeoPar->getScintFiberRadius() * CLHEP::cm, dz, 0.0, 2.0 * M_PI);
      G4LogicalVolume* scintFiberLogical = new G4LogicalVolume(fiberTube, Materials::get("G4_POLYSTYRENE"), logicalName(fiberTube));
      m_VisAttributes.push_back(new G4VisAttributes(true));
      m_VisAttributes.back()->SetColour(0.0, 1.0, 0.0);
      scintFiberLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        scintFiberLogical,
                        physicalName(scintFiberLogical),
                        scintBoreLogical,
                        false,
                        1,
                        false
                       );
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        scintBoreLogical,
                        physicalName(scintBoreLogical),
                        activeLogical,
                        false,
                        1,
                        false
                       );
      new G4PVPlacement(G4TranslateZ3D(-dzMPPC),
                        activeLogical,
                        physicalName(activeLogical),
                        scintLogical,
                        false,
                        1,
                        false
                       );
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        getMPPCHousingLogical(),
                        "BKLM.MPPCHousingPhysical",
                        scintAirLogical,
                        false,
                        1,
                        false
                       );
      new G4PVPlacement(G4TranslateZ3D(dz),
                        scintAirLogical,
                        physicalName(scintAirLogical),
                        scintLogical,
                        false,
                        1,
                        false
                       );
      return scintLogical;
    }

    G4LogicalVolume* GeoBKLMCreator::getMPPCHousingLogical()
    {
      if (m_MPPCHousingLogical == NULL) {
        G4Tubs* mppcHousingSolid =
          new G4Tubs("BKLM.MPPCHousingSolid",
                     0.0,
                     m_GeoPar->getMPPCHousingRadius() * CLHEP::cm,
                     m_GeoPar->getMPPCHousingHalfLength() * CLHEP::cm,
                     0.0,
                     2.0 * M_PI
                    );
        m_MPPCHousingLogical =
          new G4LogicalVolume(mppcHousingSolid,
                              Materials::get("G4_POLYCARBONATE"),
                              "BKLM>MPPCHousingLogical"
                             );
        m_VisAttributes.push_back(new G4VisAttributes(true));
        m_VisAttributes.back()->SetColour(0.5, 0.5, 0.5); // gray
        m_MPPCHousingLogical->SetVisAttributes(m_VisAttributes.back());
        G4Box* mppcBox =
          new G4Box("BKLM.MPPCSolid",
                    m_GeoPar->getMPPCHalfLength() * CLHEP::cm,
                    m_GeoPar->getMPPCHalfWidth() * CLHEP::cm,
                    m_GeoPar->getMPPCHalfHeight() * CLHEP::cm
                   );
        G4LogicalVolume* mppcLogical =
          new G4LogicalVolume(mppcBox,
                              Materials::get("G4_Si"),
                              "BKLM.MPPCLogical"
                             );
        m_VisAttributes.push_back(new G4VisAttributes(true));
        m_VisAttributes.back()->SetColour(1.0, 1.0, 1.0); // white
        mppcLogical->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(G4TranslateX3D(0.0),
                          mppcLogical,
                          "BKLM.MPPCPhysical",
                          m_MPPCHousingLogical,
                          false,
                          1,
                          false
                         );
      }
      return m_MPPCHousingLogical;
    }

    G4LogicalVolume* GeoBKLMCreator::getReadoutContainerLogical()
    {
      if (m_ReadoutContainerLogical == NULL) {
        const CLHEP::Hep3Vector containerHalfSize = m_GeoPar->getReadoutContainerHalfSize() * CLHEP::cm;
        G4Box* containerBox =
          new G4Box("BKLM.ReadoutContainerSolid",
                    containerHalfSize.x(), containerHalfSize.y(), containerHalfSize.z()
                   );
        m_ReadoutContainerLogical =
          new G4LogicalVolume(containerBox,
                              Materials::get("G4_AIR"),
                              logicalName(containerBox)
                             );
        m_ReadoutContainerLogical->SetVisAttributes(m_VisAttributes.front()); // invisible

        const CLHEP::Hep3Vector carrierHalfSize = m_GeoPar->getReadoutCarrierHalfSize() * CLHEP::cm;
        G4Box* carrierBox =
          new G4Box("BKLM.ReadoutCarrierSolid",
                    carrierHalfSize.x(), carrierHalfSize.y(), carrierHalfSize.z()
                   );
        G4LogicalVolume* carrierLogical =
          new G4LogicalVolume(carrierBox,
                              Materials::get("NEMA_G10_Plate"), // defined in CDC
                              logicalName(carrierBox)
                             );
        m_VisAttributes.push_back(new G4VisAttributes(true));
        m_VisAttributes.back()->SetColour(0.0, 1.0, 0.0);
        carrierLogical->SetVisAttributes(m_VisAttributes.back());
        const CLHEP::Hep3Vector preamplifierHalfSize = m_GeoPar->getReadoutPreamplifierHalfSize() * CLHEP::cm;
        G4Box* preamplifierBox =
          new G4Box("BKLM.ReadoutPreamplifierSolid",
                    preamplifierHalfSize.x(), preamplifierHalfSize.y(), preamplifierHalfSize.z()
                   );
        G4LogicalVolume* preamplifierLogical =
          new G4LogicalVolume(preamplifierBox,
                              Materials::get("NEMA_G10_Plate"), // defined in CDC
                              logicalName(preamplifierBox)
                             );
        preamplifierLogical->SetVisAttributes(m_VisAttributes.back());
        const CLHEP::Hep3Vector connectorsHalfSize = m_GeoPar->getReadoutConnectorsHalfSize() * CLHEP::cm;
        G4Box* connectorsBox =
          new G4Box("BKLM.ReadoutConnectorsSolid",
                    connectorsHalfSize.x(), connectorsHalfSize.y(), connectorsHalfSize.z()
                   );
        G4LogicalVolume* connectorsLogical =
          new G4LogicalVolume(connectorsBox,
                              Materials::get("G4_POLYCARBONATE"),
                              logicalName(connectorsBox)
                             );
        m_VisAttributes.push_back(new G4VisAttributes(true));
        m_VisAttributes.back()->SetColour(0.5, 0.5, 0.5);
        connectorsLogical->SetVisAttributes(m_VisAttributes.back());
        new G4PVPlacement(G4TranslateZ3D(preamplifierHalfSize.z()),
                          carrierLogical,
                          physicalName(carrierLogical),
                          m_ReadoutContainerLogical,
                          false,
                          1,
                          false
                         );
        new G4PVPlacement(G4Translate3D(0.0, m_GeoPar->getReadoutConnectorsPosition(), -carrierHalfSize.z()),
                          connectorsLogical,
                          physicalName(connectorsLogical),
                          m_ReadoutContainerLogical,
                          false,
                          1,
                          false
                         );
        for (int preamp = 1; preamp <= m_GeoPar->getNReadoutPreamplifierPosition(); ++preamp) {
          new G4PVPlacement(G4Translate3D(0.0,
                                          m_GeoPar->getReadoutPreamplifierPosition(preamp) * CLHEP::cm,
                                          -carrierHalfSize.z()),
                            preamplifierLogical,
                            physicalName(preamplifierLogical),
                            m_ReadoutContainerLogical,
                            false,
                            1,
                            false
                           );
        }
      }
      return m_ReadoutContainerLogical;
    }

    G4Tubs* GeoBKLMCreator::getSolenoidTube(void)
    {

      if (m_SolenoidTube == NULL) {
        m_SolenoidTube =
          new G4Tubs("BKLM.SolenoidTube",
                     0.0,
                     m_GeoPar->getSolenoidOuterRadius() * CLHEP::cm,
                     2.0 * m_GeoPar->getHalfLength() * CLHEP::cm,
                     0.0,
                     2.0 * M_PI
                    );
      }
      return m_SolenoidTube;
    }

    G4String GeoBKLMCreator::logicalName(G4VSolid* solid)
    {
      G4String* name = new G4String(solid->GetName().substr(0, solid->GetName().size() - 5) + "Logical");
      m_Names.push_back(name);
      return *name;
    }

    G4String GeoBKLMCreator::physicalName(G4LogicalVolume* lvol)
    {
      G4String* name = new G4String(lvol->GetName().substr(0, lvol->GetName().size() - 7) + "Physical");
      m_Names.push_back(name);
      return *name;
    }

  } // namespace bklm

} // namespace Belle2
