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
#include <simulation/background/BkgSensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4ReflectionFactory.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Transform3D.hh>
#include <G4UserLimits.hh>
#include <G4VisAttributes.hh>

using namespace std;
using namespace CLHEP;

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
      m_Sensitive = dynamic_cast<G4VSensitiveDetector*>(new SensitiveDetector());
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
      m_BracketsLogical = NULL;
      for (int layer = 0; layer <= NLAYER; ++layer) {
        m_LayerIronSolid[layer] = NULL;
        m_LayerIronLogical[layer] = NULL;
        m_LayerIronLogical[layer + NLAYER + 1] = NULL;
      }
      m_SectorTube = NULL;
      for (int sector = 0; sector < NSECTOR; ++sector) {
        m_SectorLogical[sector] = NULL;
      }
      m_SolenoidTube = NULL;
      m_ScintLogicals.clear();
      m_VisAttributes.push_back(new G4VisAttributes(false));
    }

    GeoBKLMCreator::~GeoBKLMCreator()
    {
      delete m_Sensitive;
      std::cout << "GeoBKLMCreator::~GeoBKLMCreator(): " << m_VisAttributes.size() << " G4VisAttributes will be deleted ..." << std::endl;
      for (std::vector<G4VisAttributes*>::iterator j = m_VisAttributes.begin();
           j != m_VisAttributes.end(); ++j) {
        std::cout << "  V" << (*j)->IsVisible() << ", A" << (*j)->GetColour().GetAlpha() << ", R" << (*j)->GetColour().GetRed() << ", G" << (*j)->GetColour().GetGreen() << ", B" << (*j)->GetColour().GetBlue() << std::endl;
        delete *j;
      }
      m_VisAttributes.clear();
      // Geant4 will delete the solids and physical/logical volumes.
    }

    //-----------------------------------------------------------------
    //                 Build and place the BKLM
    //-----------------------------------------------------------------

    void GeoBKLMCreator::create(const GearDir& content, G4LogicalVolume& motherLogical, GeometryTypes)
    {

      if (content.getInt("BeamBackgroundStudy") != 0) {
        m_Sensitive = dynamic_cast<G4VSensitiveDetector*>(new BkgSensitiveDetector("BKLM"));
      }

      m_GeoPar = GeometryPar::instance(content);
      m_SectorDphi = 2.0 * M_PI / m_GeoPar->getNSector();
      m_SectorDz = 0.5 * m_GeoPar->getHalfLength() * cm;
      m_RibShift = 0.5 * m_GeoPar->getRibThickness() * cm / sin(0.5 * m_SectorDphi);

      // Place BKLM envelope in mother volume
      G4Tubs* envelopeSolid =
        new G4Tubs("BKLM.EnvelopeSolid",
                   m_GeoPar->getSolenoidOuterRadius() * cm,
                   m_GeoPar->getOuterRadius() * cm / cos(0.5 * m_SectorDphi),
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
      new G4PVPlacement(G4TranslateZ3D(m_GeoPar->getOffsetZ() * cm) * G4RotateZ3D(m_GeoPar->getRotation() * rad),
                        envelopeLogical,
                        "BKLM.EnvelopePhysical",
                        &motherLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );

    }

    void GeoBKLMCreator::putEndsInEnvelope(G4LogicalVolume* envelopeLogical)
    {
      G4Tubs* endSolid =
        new G4Tubs("BKLM.EndSolid",
                   m_GeoPar->getSolenoidOuterRadius() * cm,
                   m_GeoPar->getOuterRadius() * cm / cos(0.5 * m_SectorDphi),
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
                        m_GeoPar->doOverlapCheck()
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
                        m_GeoPar->doOverlapCheck()
                       );

    }

    void GeoBKLMCreator::putSectorsInEnd(G4LogicalVolume* endLogical, int fb)
    {
      for (int s = 0; s < m_GeoPar->getNSector(); ++s) {
        int sector = (fb == BKLM_FORWARD ? s : ((12 - s) % 8)) + 1;
        bool hasChimney = (fb == BKLM_BACKWARD) && (sector == 3);
        bool hasInnerSupport = (sector <= m_GeoPar->getNSector() / 2 + 1);
        G4LogicalVolume* sectorLogical = getSectorLogical(fb, hasChimney, hasInnerSupport);
        new G4PVPlacement(G4RotateZ3D(m_SectorDphi * s),
                          sectorLogical,
                          physicalName(sectorLogical),
                          endLogical,
                          false,
                          sector,
                          m_GeoPar->doOverlapCheck()
                         );
      }
    }

    void GeoBKLMCreator::putCapInSector(G4LogicalVolume* sectorLogical, bool hasChimney)
    {

      // Fill cap with iron and (aluminum) cables
      const Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(0, false) * cm;
      const double dyBrace = (hasChimney ? m_GeoPar->getBraceWidthChimney() : m_GeoPar->getBraceWidth()) * cm;
      const double dy = 0.25 * (m_GeoPar->getCablesWidth() * cm - dyBrace);
      const double dz = m_SectorDz - gapHalfSize.z();
      const double ri = m_GeoPar->getLayerInnerRadius(1) * cm + 2.0 * gapHalfSize.x();
      const double ro = m_GeoPar->getOuterRadius() * cm;
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
                          physicalName(cablesLogical) + "_L",
                          m_CapLogical[newLvol],
                          false,
                          1,
                          m_GeoPar->doOverlapCheck()
                         );
        new G4PVPlacement(G4Translate3D(0.5 * (ri + ro), +(0.5 * dyBrace + dy), 0.0),
                          cablesLogical,
                          physicalName(cablesLogical) + "_R",
                          m_CapLogical[newLvol],
                          false,
                          2,
                          m_GeoPar->doOverlapCheck()
                         );
      }
      new G4PVPlacement(G4TranslateZ3D(m_SectorDz - dz),
                        m_CapLogical[newLvol],
                        physicalName(m_CapLogical[newLvol]),
                        sectorLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putInnerRegionInSector(G4LogicalVolume* sectorLogical, bool hasInnerSupport, bool hasChimney)
    {

      // Fill inner region with iron
      if (m_InnerIronSolid == NULL) {
        const double r = m_GeoPar->getLayerInnerRadius(1) * cm;
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
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putVoidInInnerRegion(G4LogicalVolume* innerIronLogical, bool hasInnerSupport, bool hasChimney)
    {

      // Carve out an air void from the inner-region iron, leaving only the ribs
      // at the azimuthal edges
      if (m_InnerAirSolid == NULL) {
        const double r = m_GeoPar->getLayerInnerRadius(1) * cm - m_RibShift;
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
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putLayer1SupportInInnerVoid(G4LogicalVolume* innerAirLogical, bool hasChimney)
    {

      int newLvol = (hasChimney ? 1 : 0);
      const Hep3Vector size = m_GeoPar->getSupportPlateHalfSize(hasChimney) * cm;
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
      double dx = m_GeoPar->getLayerInnerRadius(1) * cm - size.x() - m_RibShift;
      double dz = size.z() - m_SectorDz;
      new G4PVPlacement(G4Translate3D(dx, 0.0, dz),
                        m_SupportLogical[newLvol],
                        physicalName(m_SupportLogical[newLvol]),
                        innerAirLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putLayer1BracketsInInnerVoid(G4LogicalVolume* innerAirLogical, bool hasChimney)
    {

      if (m_BracketsLogical == NULL) {
        const Hep3Vector size = m_GeoPar->getSupportPlateHalfSize(hasChimney) * cm;
        const double dz = 0.5 * m_GeoPar->getBracketLength() * cm;
        const double r = m_GeoPar->getLayerInnerRadius(1) * cm - m_RibShift - 2.0 * size.x();
        const double bracketShift = m_GeoPar->getBracketRibThickness() * cm / sin(0.5 * m_SectorDphi);
        const double z[2] = { -dz, +dz};
        const double rInner[2] = {0.0, 0.0};
        const double rOuter[2] = {r, r};
        const double r1 = m_GeoPar->getBracketInnerRadius() * cm - m_RibShift;
        const double z1[2] = { -(dz + 0.5 * cm), dz + 0.5 * cm};
        const double rOuter1[2] = {r1, r1};
        const double z2[2] = { -(dz + 1.0 * cm), dz + 1.0 * cm};
        const double dzBracket = m_GeoPar->getBracketThickness() * cm;
        const double drBracket = dzBracket + bracketShift;
        const double rOuter2[2] = {r - drBracket, r - drBracket};
        const double z3[2] = { -dzBracket, dzBracket};
        const double cutoutDphi = m_GeoPar->getBracketCutoutDphi() * rad;
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
                    rOuter[1] + 1.0 * cm,
                    rOuter[1] * tan(0.5 * m_SectorDphi) - m_GeoPar->getBracketWidth() * cm,
                    z[1] + 1.5 * cm
                   );
        G4VSolid* temp1 =
          new G4SubtractionSolid("BKLM.BracketsTemp1",
                                 bracketPolygon,
                                 bracketCutout4
                                );
        G4VSolid* temp2 =
          new G4SubtractionSolid("BKLM.BracketsTemp2",
                                 bracketCutout2,
                                 bracketCutout3
                                );
        G4VSolid* temp3 =
          new G4SubtractionSolid("BKLM.BracketsTemp3",
                                 temp1,
                                 temp2,
                                 G4TranslateX3D(bracketShift)
                                );
        G4VSolid* bracketsSolid =
          new G4SubtractionSolid("BKLM.BracketsSolid",
                                 temp3,
                                 bracketCutout1
                                );
        m_BracketsLogical =
          new G4LogicalVolume(bracketsSolid,
                              Materials::get("G4_Al"),
                              "BKLM.BracketsLogical"
                             );
        m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0.6, 0.6, 0.6)));
        m_BracketsLogical->SetVisAttributes(m_VisAttributes.back());
      }
      char name[80] = "";
      for (int bracket = 0; bracket < (hasChimney ? 2 : 3); ++bracket) {
        sprintf(name, "%s%d", (hasChimney ? "Chimney" : ""), bracket);
        new G4PVPlacement(G4TranslateZ3D(m_GeoPar->getBracketZPosition(bracket, hasChimney) * cm),
                          m_BracketsLogical,
                          physicalName(m_BracketsLogical) + name,
                          innerAirLogical,
                          false,
                          bracket,
                          m_GeoPar->doOverlapCheck()
                         );
      }
    }

    void GeoBKLMCreator::putLayersInSector(G4LogicalVolume* sectorLogical, bool isForward, bool hasChimney)
    {

      const double dz = 0.5 * m_GeoPar->getGapLength() * cm;
      const double z[2] = { -dz, +dz};
      char name[80] = "";
      for (int layer = 1; layer <= m_GeoPar->getNLayer(); ++layer) {
        // Fill layer with iron
        if (m_LayerIronSolid[layer] == NULL) {
          const double ri = m_GeoPar->getLayerInnerRadius(layer) * cm;
          const double ro = m_GeoPar->getLayerOuterRadius(layer) * cm;
          const double rInner[2] = {ri, ri};
          const double rOuter[2] = {ro, ro};
          sprintf(name, "BKLM.Layer%02dIronSolid", layer);
          m_LayerIronSolid[layer] =
            new G4Polyhedra(name,
                            -0.5 * m_SectorDphi,
                            m_SectorDphi,
                            1,
                            2, z, rInner, rOuter
                           );
        }
        int newLvol = (hasChimney ? (NLAYER + 1) : 0) + layer;
        if (m_LayerIronLogical[newLvol] == NULL) {
          int sector = (hasChimney ? 3 : 1);
          sprintf(name, "BKLM.Layer%02d%sIronLogical", layer, (hasChimney ? "Chimney" : ""));
          m_LayerIronLogical[newLvol] =
            new G4LogicalVolume(m_LayerIronSolid[layer],
                                Materials::get("G4_Fe"),
                                name
                               );
          m_LayerIronLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
          putModuleInLayer(m_LayerIronLogical[newLvol], m_GeoPar->findModule(isForward, sector, layer), layer, hasChimney);
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
                          m_GeoPar->doOverlapCheck()
                         );

      }
    }

    void GeoBKLMCreator::putChimneyInLayer(G4LogicalVolume* layerIronLogical, int layer)
    {
      Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, true) * cm;
      Hep3Vector chimneyHalfSize = m_GeoPar->getChimneyHalfSize(layer) * cm;
      Hep3Vector chimneyPosition = m_GeoPar->getChimneyPosition(layer) * cm;
      gapHalfSize.setY(0.5 * (gapHalfSize.y() - chimneyHalfSize.y()));
      gapHalfSize.setZ(chimneyHalfSize.z() + 0.5 * m_GeoPar->getChimneyCoverThickness() * cm);
      double dx = m_GeoPar->getGapMiddleRadius(layer) * cm;
      double dy = gapHalfSize.y() + chimneyHalfSize.y();
      double dz = 0.5 * m_GeoPar->getGapLength() * cm - gapHalfSize.z();
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
                        m_GeoPar->doOverlapCheck()
                       );
      sprintf(name, "BKLM.Layer%02dRightGapChimneyPhysical", layer);
      new G4PVPlacement(G4Translate3D(dx, +dy, dz),
                        gapLogical,
                        name,
                        layerIronLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
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
                   m_GeoPar->getChimneyHousingInnerRadius() * cm,
                   m_GeoPar->getChimneyHousingOuterRadius() * cm,
                   chimneyHalfSize.x(),
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* housingLogical =
        new G4LogicalVolume(housingTube,
                            Materials::get("G4_Fe"),
                            "BKLM.ChimneyHousingLogical"
                           );
      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0.4, 0.4, 0.4)));
      housingLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4RotateY3D(M_PI_2),
                        housingLogical,
                        "BKLM.ChimneyHousingPhysical",
                        chimneyLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      G4Tubs* shieldTube =
        new G4Tubs("BKLM.ChimneyShieldTube",
                   m_GeoPar->getChimneyShieldInnerRadius() * cm,
                   m_GeoPar->getChimneyShieldOuterRadius() * cm,
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
                        m_GeoPar->doOverlapCheck()
                       );
      G4Tubs* pipeTube =
        new G4Tubs("BKLM.ChimneyPipeTube",
                   m_GeoPar->getChimneyPipeInnerRadius() * cm,
                   m_GeoPar->getChimneyPipeOuterRadius() * cm,
                   chimneyHalfSize.x(),
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* pipeLogical =
        new G4LogicalVolume(pipeTube,
                            Materials::get("G4_Fe"),
                            "BKLM.ChimneyPipeLogical"
                           );
      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0.6, 0.6, 0.6)));
      pipeLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4RotateY3D(M_PI_2),
                        pipeLogical,
                        "BKLM.ChimneyPipePhysical",
                        chimneyLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      sprintf(name, "BKLM.Layer%02dChimneyPhysical", layer);
      new G4PVPlacement(G4Translate3D(chimneyPosition),
                        chimneyLogical,
                        name,
                        layerIronLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putModuleInLayer(G4LogicalVolume* layerIronLogical, const Module* module, int layer, bool hasChimney)
    {
      const Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, hasChimney) * cm;
      char name[80] = "";
      // Fill gap with air
      sprintf(name, "BKLM.Layer%02d%sGapSolid", layer, (hasChimney ? "Chimney" : ""));
      G4Box* gapBox =
        new G4Box(name,
                  gapHalfSize.x(), gapHalfSize.y(), gapHalfSize.z()
                 );
      G4LogicalVolume* gapLogical =
        new G4LogicalVolume(gapBox,
                            Materials::get("G4_AIR"),
                            logicalName(gapBox)
                           );
      gapLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      // Module is aluminum (but interior will be filled)
      const Hep3Vector moduleHalfSize = m_GeoPar->getModuleHalfSize(layer, hasChimney) * cm;
      sprintf(name, "BKLM.Layer%02d%sModuleSolid", layer, (hasChimney ? "Chimney" : ""));
      G4Box* moduleBox =
        new G4Box(name,
                  moduleHalfSize.x(), moduleHalfSize.y(), moduleHalfSize.z()
                 );
      G4LogicalVolume* moduleLogical =
        new G4LogicalVolume(moduleBox,
                            Materials::get("G4_Al"),
                            logicalName(moduleBox)
                           );
      moduleLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      sprintf(name, "BKLM.Layer%02d%sModuleInteriorSolid1", layer, (hasChimney ? "Chimney" : ""));
      const Hep3Vector interiorHalfSize1 = m_GeoPar->getModuleInteriorHalfSize1(layer, hasChimney) * cm;
      G4Box* interiorBox1 =
        new G4Box(name,
                  interiorHalfSize1.x(), interiorHalfSize1.y(), interiorHalfSize1.z()
                 );
      sprintf(name, "BKLM.Layer%02d%sModuleInteriorSolid2", layer, (hasChimney ? "Chimney" : ""));
      const Hep3Vector interiorHalfSize2 = m_GeoPar->getModuleInteriorHalfSize2(layer, hasChimney) * cm;
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
        putScintsInInterior(interiorLogical, module, layer, hasChimney);
      }
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        interiorLogical,
                        physicalName(interiorLogical),
                        moduleLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      double dx = (m_GeoPar->getModuleMiddleRadius(layer) - m_GeoPar->getGapMiddleRadius(layer)) * cm;
      double dz = moduleHalfSize.z() - gapHalfSize.z();
      new G4PVPlacement(G4Translate3D(dx, 0.0, dz),
                        moduleLogical,
                        physicalName(moduleLogical),
                        gapLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      dz = gapHalfSize.z() - 0.5 * m_GeoPar->getGapLength() * cm;
      new G4PVPlacement(G4Translate3D(m_GeoPar->getGapMiddleRadius(layer) * cm, 0.0, dz),
                        gapLogical,
                        physicalName(gapLogical),
                        layerIronLogical,
                        false,
                        layer,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putRPCsInInterior(G4LogicalVolume* interiorLogical, int layer, bool hasChimney)
    {
      char name[80] = "";
      // Place electrode inside the module's interior
      sprintf(name, "BKLM.Layer%02d%sElectrodeSolid", layer, (hasChimney ? "Chimney" : ""));
      const Hep3Vector electrodeHalfSize = m_GeoPar->getElectrodeHalfSize(layer, hasChimney) * cm;
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
      const Hep3Vector gasHalfSize = m_GeoPar->getGasHalfSize(layer, hasChimney) * cm;
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
      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(1.0, 0.5, 0.0)));
      gasLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4TranslateX3D(-0.5 * electrodeHalfSize.x()),
                        gasLogical,
                        physicalName(gasLogical),
                        electrodeLogical,
                        false,
                        BKLM_INNER,
                        m_GeoPar->doOverlapCheck()
                       );

      new G4PVPlacement(G4TranslateX3D(+0.5 * electrodeHalfSize.x()),
                        gasLogical,
                        physicalName(gasLogical),
                        electrodeLogical,
                        false,
                        BKLM_OUTER,
                        m_GeoPar->doOverlapCheck()
                       );
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        electrodeLogical,
                        physicalName(electrodeLogical),
                        interiorLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putScintsInInterior(G4LogicalVolume* interiorLogical, const Module* module, int layer, bool hasChimney)
    {
      char name[80] = "";
      sprintf(name, "BKLM.Layer%02d%sAirSolid", layer, (hasChimney ? "Chimney" : ""));
      const Hep3Vector airHalfSize = m_GeoPar->getAirHalfSize(layer, hasChimney) * cm;
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
      const Hep3Vector scintEnvelopeHalfSize = m_GeoPar->getScintEnvelopeHalfSize(layer, hasChimney) * cm;
      G4Box* scintEnvelopeBox =
        new G4Box(name,
                  scintEnvelopeHalfSize.x(), scintEnvelopeHalfSize.y(), scintEnvelopeHalfSize.z()
                 );
      G4LogicalVolume* innerEnvelopeLogical =
        new G4LogicalVolume(scintEnvelopeBox,
                            Materials::get("G4_POLYSTYRENE"),
                            logicalName(scintEnvelopeBox)
                           );
      innerEnvelopeLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      double scintHalfHeight  = m_GeoPar->getScintHalfHeight() * cm;
      double scintHalfWidth   = m_GeoPar->getScintHalfWidth() * cm;

      for (int scint = 1; scint <= m_GeoPar->getNPhiScints(layer); ++scint) {
        double scintHalfLength = module->getPhiScintHalfLength(scint) * cm;
        double scintOffset     = module->getPhiScintOffset(scint) * cm;
        double scintPosition   = module->getPhiScintPosition(scint) * cm;
        G4LogicalVolume* scintLogical = getScintLogical(scintHalfHeight, scintHalfWidth, scintHalfLength);
        new G4PVPlacement(G4Translate3D(0.0, scintPosition, scintOffset),
                          scintLogical,
                          physicalName(scintLogical),
                          innerEnvelopeLogical,
                          false,
                          scint,
                          m_GeoPar->doOverlapCheck()
                         );
      }
      G4LogicalVolume* outerEnvelopeLogical =
        new G4LogicalVolume(scintEnvelopeBox,
                            Materials::get("G4_POLYSTYRENE"),
                            logicalName(scintEnvelopeBox)
                           );
      outerEnvelopeLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      for (int scint = 1; scint <= m_GeoPar->getNZScints(hasChimney); ++scint) {
        double scintHalfLength = module->getZScintHalfLength(scint) * cm;
        double scintOffset     = module->getZScintOffset(scint) * cm;
        double scintPosition   = module->getZScintPosition(scint) * cm;
        G4LogicalVolume* scintLogical = getScintLogical(scintHalfHeight, scintHalfWidth, scintHalfLength);
        new G4PVPlacement(G4Translate3D(0.0, scintOffset, scintPosition) * G4RotateX3D(M_PI_2),
                          scintLogical,
                          physicalName(scintLogical),
                          outerEnvelopeLogical,
                          false,
                          scint,
                          m_GeoPar->doOverlapCheck()
                         );
      }
      Hep3Vector envelopeOffset = m_GeoPar->getScintEnvelopeOffset(layer, hasChimney) * cm;
      new G4PVPlacement(G4Translate3D(-scintEnvelopeHalfSize.x(), envelopeOffset.y(), envelopeOffset.z()),
                        innerEnvelopeLogical,
                        physicalName(innerEnvelopeLogical),
                        airLogical,
                        false,
                        BKLM_INNER,
                        m_GeoPar->doOverlapCheck()
                       );
      new G4PVPlacement(G4Translate3D(+scintEnvelopeHalfSize.x(), envelopeOffset.y(), envelopeOffset.z()),
                        outerEnvelopeLogical,
                        physicalName(outerEnvelopeLogical),
                        airLogical,
                        false,
                        BKLM_OUTER,
                        m_GeoPar->doOverlapCheck()
                       );
      new G4PVPlacement(G4TranslateX3D(m_GeoPar->getPolystyreneOffsetX() * cm),
                        airLogical,
                        physicalName(airLogical),
                        interiorLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    G4LogicalVolume* GeoBKLMCreator::getSectorLogical(int fb, bool hasChimney, bool hasInnerSupport)
    {

      if (m_SectorTube == NULL) {
        m_SectorTube =
          new G4Tubs("BKLM.SectorSolid",
                     m_GeoPar->getSolenoidOuterRadius() * cm,
                     m_GeoPar->getOuterRadius() * cm / cos(0.5 * m_SectorDphi),
                     m_SectorDz,
                     -0.5 * m_SectorDphi,
                     m_SectorDphi
                    );
      }
      int newLvol = (hasInnerSupport ? 4 : 0) + (hasChimney ? 2 : 0) + (fb - BKLM_FORWARD);
      if (m_SectorLogical[newLvol] == NULL) {
        char name[80] = "";
        sprintf(name, "BKLM.SectorType%dLogical", newLvol);
        m_SectorLogical[newLvol] =
          new G4LogicalVolume(m_SectorTube,
                              Materials::get("G4_AIR"),
                              name
                             );
        m_SectorLogical[newLvol]->SetVisAttributes(m_VisAttributes.front()); // invisible
        putCapInSector(m_SectorLogical[newLvol], hasChimney);
        putInnerRegionInSector(m_SectorLogical[newLvol], hasInnerSupport, hasChimney);
        putLayersInSector(m_SectorLogical[newLvol], (fb == BKLM_FORWARD), hasChimney);
      }
      return m_SectorLogical[newLvol];
    }

    G4LogicalVolume* GeoBKLMCreator::getScintLogical(double dx, double dy, double dz)
    {

      int newLvol = 1;
      for (std::vector<G4LogicalVolume*>::iterator iLvol = m_ScintLogicals.begin(); iLvol != m_ScintLogicals.end(); ++iLvol) {
        G4Box* box = (G4Box*)((*iLvol)->GetSolid());
        if ((std::fabs(box->GetXHalfLength() - dx) < 1.0E-4 * cm) &&
            (std::fabs(box->GetYHalfLength() - dy) < 1.0E-4 * cm) &&
            (std::fabs(box->GetZHalfLength() - dz) < 1.0E-4 * cm)) { return *iLvol; }
        newLvol++;
      }
      char name[80] = "";
      sprintf(name, "BKLM.ScintType%dSolid", newLvol);
      G4Box* box = new G4Box(name, dx, dy, dz);
      G4LogicalVolume* scintLogical =
        new G4LogicalVolume(box, Materials::get("G4_POLYSTYRENE"), logicalName(box), 0, m_Sensitive, 0);
      m_ScintLogicals.push_back(scintLogical);
      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(1.0, 0.5, 0.0)));
      scintLogical->SetVisAttributes(m_VisAttributes.back());
      sprintf(name, "BKLM.ScintBoreType%dSolid", newLvol);
      G4Tubs* boreTube = new G4Tubs(name, 0.0, m_GeoPar->getScintBoreRadius() * cm, dz, 0.0, 2.0 * M_PI);
      G4LogicalVolume* scintBoreLogical =
        new G4LogicalVolume(boreTube, Materials::get("G4_AIR"), logicalName(boreTube));
      scintBoreLogical->SetVisAttributes(m_VisAttributes.front()); // invisible
      sprintf(name, "BKLM.ScintFiberType%dSolid", newLvol);
      G4Tubs* fiberTube = new G4Tubs(name, 0.0, m_GeoPar->getScintFiberRadius() * cm, dz, 0.0, 2.0 * M_PI);
      G4LogicalVolume* scintFiberLogical = new G4LogicalVolume(fiberTube, Materials::get("G4_POLYSTYRENE"), logicalName(fiberTube));
      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0.0, 1.0, 0.0)));
      scintFiberLogical->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        scintFiberLogical,
                        physicalName(scintFiberLogical),
                        scintBoreLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        scintBoreLogical,
                        physicalName(scintBoreLogical),
                        scintLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
      return scintLogical;
    }

    G4Tubs* GeoBKLMCreator::getSolenoidTube(void)
    {

      if (m_SolenoidTube == NULL) {
        m_SolenoidTube =
          new G4Tubs("BKLM.SolenoidTube",
                     0.0,
                     m_GeoPar->getSolenoidOuterRadius() * cm,
                     2.0 * m_GeoPar->getHalfLength() * cm,
                     0.0,
                     2.0 * M_PI
                    );
      }
      return m_SolenoidTube;
    }

    G4String GeoBKLMCreator::logicalName(G4VSolid* solid)
    {
      G4String name = solid->GetName();
      name.resize(name.size() - 5); // strip "Solid" from end
      name.append("Logical");
      return name;
    }

    G4String GeoBKLMCreator::physicalName(G4LogicalVolume* lvol)
    {
      G4String name = lvol->GetName();
      name.resize(name.size() - 7); // strip "Logical" from end
      name.append("Physical");
      return name;
    }

  } // namespace bklm

} // namespace Belle2
