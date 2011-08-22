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
#include <bklm/geometry/GeometryPar.h>
#include <bklm/simulation/SensitiveDetector.h>

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
      m_sensitive = new SensitiveDetector();
    }

    GeoBKLMCreator::~GeoBKLMCreator()
    {
      delete m_sensitive;
    }

    //-----------------------------------------------------------------
    //                 Build and place the BKLM
    //-----------------------------------------------------------------

    void GeoBKLMCreator::create(const GearDir& content, G4LogicalVolume& motherLogical, GeometryTypes type)
    {

      m_GeoPar = GeometryPar::instance(content);
      m_SectorDphi = 2.0 * M_PI / m_GeoPar->getNSector();
      m_SectorDz = 0.5 * m_GeoPar->getHalfLength() * cm;
      m_RibShift = 0.5 * m_GeoPar->getRibThickness() * cm / sin(0.5 * m_SectorDphi);

      // Place BKLM envelope in mother volume
      G4Tubs* envelopeShape =
        new G4Tubs("BKLM.EnvelopeShape",
                   m_GeoPar->getSolenoidOuterRadius() * cm,
                   m_GeoPar->getOuterRadius() * cm / cos(0.5 * m_SectorDphi),
                   2.0 * m_SectorDz,
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* envelopeLogical =
        new G4LogicalVolume(envelopeShape,
                            Materials::get("G4_AIR"),
                            "BKLM.EnvelopeLogical"
                           );
      envelopeLogical->SetVisAttributes(G4VisAttributes(false));
      putEndsInEnvelope(envelopeLogical);
      new G4PVPlacement(G4TranslateZ3D(m_GeoPar->getOffsetZ() * cm) * G4RotateZ3D(m_GeoPar->getRotation() * rad),
                        envelopeLogical,
                        "BKLM.EnvelopePhysical",
                        &motherLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );

    }

    void GeoBKLMCreator::putEndsInEnvelope(G4LogicalVolume* envelopeLogical)
    {
      G4Tubs* endShape =
        new G4Tubs("BKLM.EndShape",
                   m_GeoPar->getSolenoidOuterRadius() * cm,
                   m_GeoPar->getOuterRadius() * cm / cos(0.5 * m_SectorDphi),
                   m_SectorDz,
                   0.0,
                   2.0 * M_PI
                  );
      G4LogicalVolume* frontLogical =
        new G4LogicalVolume(endShape,
                            Materials::get("G4_AIR"),
                            "BKLM.FrontLogical"
                           );
      frontLogical->SetVisAttributes(G4VisAttributes(false));
      putSectorsInEnd(frontLogical, 0);
      new G4PVPlacement(G4TranslateZ3D(m_SectorDz),
                        frontLogical,
                        "BKLM.FrontPhysical",
                        envelopeLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      G4LogicalVolume* backLogical =
        new G4LogicalVolume(endShape,
                            Materials::get("G4_AIR"),
                            "BKLM.BackLogical"
                           );
      backLogical->SetVisAttributes(G4VisAttributes(false));
      putSectorsInEnd(backLogical, 1);
      G4ReflectionFactory::Instance()->Place(G4TranslateZ3D(-m_SectorDz) * G4ReflectZ3D(),
                                             "BKLM.BackPhysical",
                                             backLogical,
                                             envelopeLogical,
                                             false,
                                             0
                                            );

    }

    void GeoBKLMCreator::putSectorsInEnd(G4LogicalVolume* endLogical, int fb)
    {
      G4Tubs* sectorShape =
        new G4Tubs("BKLM.SectorShape",
                   m_GeoPar->getSolenoidOuterRadius() * cm,
                   m_GeoPar->getOuterRadius() * cm / cos(0.5 * m_SectorDphi),
                   m_SectorDz,
                   -0.5 * m_SectorDphi,
                   m_SectorDphi
                  );
      char name[40] = "";
      for (int sector = 0; sector < m_GeoPar->getNSector(); ++sector) {
        bool hasChimney = (fb == 1) && (sector == 2);
        sprintf(name, "BKLM.Sector%dLogical", sector);
        G4LogicalVolume* sectorLogical =
          new G4LogicalVolume(sectorShape,
                              Materials::get("G4_AIR"),
                              name
                             );
        sectorLogical->SetVisAttributes(G4VisAttributes(false));
        putCapInSector(sectorLogical, hasChimney);
        putInnerRegionInSector(sectorLogical, sector, hasChimney);
        putLayersInSector(sectorLogical, fb, sector, hasChimney);
        sprintf(name, "BKLM.Sector%dPhysical", sector);
        new G4PVPlacement(G4RotateZ3D(m_SectorDphi * sector),
                          sectorLogical,
                          name,
                          endLogical,
                          false,
                          0,
                          m_GeoPar->doOverlapCheck()
                         );
      }
    }

    void GeoBKLMCreator::putCapInSector(G4LogicalVolume* sectorLogical, bool hasChimney)
    {
      const Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(0, false) * cm;
      const double dyBrace = (hasChimney ? m_GeoPar->getBraceWidthChimney() : m_GeoPar->getBraceWidth()) * cm;
      const double dy = 0.25 * (m_GeoPar->getCablesWidth() * cm - dyBrace);
      const double dz = m_SectorDz - gapHalfSize.z();
      const double ri = m_GeoPar->getLayerInnerRadius(1) * cm + 2.0 * gapHalfSize.x();
      const double ro = m_GeoPar->getOuterRadius() * cm;
      const double z[2] = { -dz, dz};
      const double rInner[2] = {ri, ri};
      const double rOuter[2] = {ro, ro};
      // Fill cap with iron
      G4Polyhedra* capShape =
        new G4Polyhedra("BKLM.CapShape",
                        -0.5 * m_SectorDphi,
                        m_SectorDphi,
                        1,
                        2, z, rInner, rOuter
                       );
      G4LogicalVolume* capLogical =
        new G4LogicalVolume(capShape,
                            Materials::get("G4_Fe"),
                            "BKLM.CapLogical"
                           );
      capLogical->SetVisAttributes(G4VisAttributes(false));
      G4Box* cablesShape =
        new G4Box("BKLM.CablesShape", 0.5 *(ro - ri), dy, dz);
      G4LogicalVolume* cablesLogical =
        new G4LogicalVolume(cablesShape,
                            Materials::get("G4_Al"),
                            "BKLM.LeftCablesLogical"
                           );
      cablesLogical->SetVisAttributes(G4VisAttributes(false));
      new G4PVPlacement(G4Translate3D(0.5 *(ri + ro), -(0.5 * dyBrace + dy), 0.0),
                        cablesLogical,
                        "BKLM.LeftCablesPhysical",
                        capLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      new G4PVPlacement(G4Translate3D(0.5 *(ri + ro), +(0.5 * dyBrace + dy), 0.0),
                        cablesLogical,
                        "BKLM.RightCablesPhysical",
                        capLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
      new G4PVPlacement(G4TranslateZ3D(m_SectorDz - dz),
                        capLogical,
                        "BKLM.CapPhysical",
                        sectorLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putInnerRegionInSector(G4LogicalVolume* sectorLogical, int sector, bool hasChimney)
    {
      const double r = m_GeoPar->getLayerInnerRadius(0) * cm;
      const double z[2] = { -m_SectorDz, +m_SectorDz};
      const double rInner[2] = {0.0, 0.0};
      const double rOuter[2] = {r, r};
      // Fill inner region with iron
      G4Polyhedra* innerIronPolygon =
        new G4Polyhedra("BKLM.InnerIronPolygon",
                        -0.5 * m_SectorDphi,
                        m_SectorDphi,
                        1,
                        2, z, rInner, rOuter
                       );
      G4VSolid* innerIronShape =
        new G4SubtractionSolid("BKLM.InnerIronShape",
                               innerIronPolygon,
                               solenoidCutout()
                              );
      G4LogicalVolume* innerIronLogical =
        new G4LogicalVolume(innerIronShape,
                            Materials::get("G4_Fe"),
                            "BKLM.InnerIronLogical"
                           );
      innerIronLogical->SetVisAttributes(G4VisAttributes(false));
      putVoidInInnerRegion(innerIronLogical, sector, hasChimney);
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        innerIronLogical,
                        "BKLM.InnerIronPhysical",
                        sectorLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putVoidInInnerRegion(G4LogicalVolume* innerIronLogical, int sector, bool hasChimney)
    {
      const double r = m_GeoPar->getLayerInnerRadius(0) * cm - m_RibShift;
      const double z[2] = { -m_SectorDz, +m_SectorDz};
      const double rInner[2] = {0.0, 0.0};
      const double rOuter[2] = {r, r};
      // Carve out an air void from the inner-region iron, leaving only the ribs
      // at the azimuthal edges
      G4Polyhedra* innerAirPolygon =
        new G4Polyhedra("BKLM.InnerAirPolygon",
                        -0.5 * m_SectorDphi,
                        m_SectorDphi,
                        1,
                        2, z, rInner, rOuter
                       );
      G4VSolid* innerAirShape =
        new G4SubtractionSolid("BKLM.InnerAirShape",
                               innerAirPolygon,
                               solenoidCutout(),
                               G4TranslateX3D(-m_RibShift)
                              );
      G4LogicalVolume* innerAirLogical =
        new G4LogicalVolume(innerAirShape,
                            Materials::get("G4_AIR"),
                            "BKLM.InnerAirLogical"
                           );
      innerAirLogical->SetVisAttributes(G4VisAttributes(false));
      if (sector <= m_GeoPar->getNSector() / 2) {
        putLayer0SupportInInnerVoid(innerAirLogical, hasChimney);
        putLayer0BracketsInInnerVoid(innerAirLogical, hasChimney);
      }
      new G4PVPlacement(G4TranslateX3D(m_RibShift),
                        innerAirLogical,
                        "BKLM.InnerAirPhysical",
                        innerIronLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putLayer0SupportInInnerVoid(G4LogicalVolume* innerAirLogical, bool hasChimney)
    {
      const Hep3Vector size = m_GeoPar->getSupportPlateHalfSize(hasChimney) * cm;
      G4Box* supportBox =
        new G4Box("BKLM.SupportBox",
                  size.x(),
                  size.y(),
                  size.z()
                 );
      G4LogicalVolume* supportLogical =
        new G4LogicalVolume(supportBox,
                            Materials::get("G4_Al"),
                            "BKLM.SupportLogical"
                           );
      supportLogical->SetVisAttributes(G4VisAttributes(false));
      new G4PVPlacement(G4TranslateX3D(m_GeoPar->getLayerInnerRadius(0) * cm - size.x() - m_RibShift),
                        supportLogical,
                        "BKLM.SupportPhysical",
                        innerAirLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }
    void GeoBKLMCreator::putLayer0BracketsInInnerVoid(G4LogicalVolume* innerAirLogical, bool hasChimney)
    {
      const Hep3Vector size = m_GeoPar->getSupportPlateHalfSize(hasChimney) * cm;
      const double dz = 0.5 * m_GeoPar->getBracketLength() * cm;
      const double r = m_GeoPar->getLayerInnerRadius(0) * cm - m_RibShift - 2.0 * size.x();
      const double bracketShift = m_GeoPar->getBracketRibThickness() * cm / sin(0.5 * m_SectorDphi);
      const double z[2] = { -dz, +dz};
      const double rInner[2] = {0.0, 0.0};
      const double rOuter[2] = {r, r};
      const double r1 = m_GeoPar->getBracketInnerRadius() * cm - m_RibShift;
      const double z1[2] = { -(dz + 0.5*cm), dz + 0.5*cm};
      const double rOuter1[2] = {r1, r1};
      const double z2[2] = { -(dz + 1.0*cm), dz + 1.0*cm};
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
                  rOuter[1] + 1.0*cm,
                  rOuter[1] * tan(0.5 * m_SectorDphi) - m_GeoPar->getBracketWidth() * cm,
                  z[1] + 1.5*cm
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

      G4LogicalVolume* bracketsLogical =
        new G4LogicalVolume(bracketsSolid,
                            Materials::get("G4_Al"),
                            "BKLM.BracketsLogical"
                           );
      bracketsLogical->SetVisAttributes(G4VisAttributes(true, G4Colour(0.6, 0.6, 0.6)));
      for (int bracket = 0; bracket < (hasChimney ? 2 : 3); ++bracket) {
        new G4PVPlacement(G4TranslateZ3D(m_GeoPar->getBracketZPosition(bracket, hasChimney) * cm),
                          bracketsLogical,
                          "BKLM.BracketsPhysical",
                          innerAirLogical,
                          false,
                          bracket,
                          m_GeoPar->doOverlapCheck()
                         );
      }
    }

    void GeoBKLMCreator::putLayersInSector(G4LogicalVolume* sectorLogical, int fb, int sector, bool hasChimney)
    {
      const double dz = 0.5 * m_GeoPar->getGapLength() * cm;
      const double z[2] = { -dz, +dz};
      char name[40];
      for (int layer = 0; layer < m_GeoPar->getNLayer(); ++layer) {
        const double ri = m_GeoPar->getLayerInnerRadius(layer) * cm;
        const double ro = m_GeoPar->getLayerOuterRadius(layer) * cm;
        const double rInner[2] = {ri, ri};
        const double rOuter[2] = {ro, ro};
        // Fill layer with iron
        sprintf(name, "BKLM.Layer%02dIronPolygon", layer);
        G4Polyhedra* layerIronPolygon =
          new G4Polyhedra(name,
                          -0.5 * m_SectorDphi,
                          m_SectorDphi,
                          1,
                          2, z, rInner, rOuter
                         );
        sprintf(name, "BKLM.Layer%02dIronLogical", layer);
        G4LogicalVolume* layerIronLogical =
          new G4LogicalVolume(layerIronPolygon,
                              Materials::get("G4_Fe"),
                              name
                             );
        layerIronLogical->SetVisAttributes(G4VisAttributes(false));
        putGapInLayer(layerIronLogical, fb, sector, layer, hasChimney);
        if (hasChimney) {
          putChimneyInLayer(layerIronLogical, layer);
        }
        sprintf(name, "BKLM.Layer%02dIronPhysical", layer);
        new G4PVPlacement(G4TranslateZ3D(dz - m_SectorDz),
                          layerIronLogical,
                          name,
                          sectorLogical,
                          false,
                          0,
                          m_GeoPar->doOverlapCheck()
                         );

      }
    }

    void GeoBKLMCreator::putChimneyInLayer(G4LogicalVolume* layerIronLogical, int layer)
    {
      Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, true) * cm;
      Hep3Vector chimneyHalfSize = m_GeoPar->getChimneyHalfSize(layer) * cm;
      Hep3Vector chimneyPosition = m_GeoPar->getChimneyPosition(layer) * cm;
      gapHalfSize.setY(0.5 *(gapHalfSize.y() - chimneyHalfSize.y()));
      gapHalfSize.setZ(chimneyHalfSize.z() + 0.5 * m_GeoPar->getChimneyCoverThickness() * cm);
      double dx = m_GeoPar->getGapMiddleRadius(layer) * cm;
      double dy = gapHalfSize.y() + chimneyHalfSize.y();
      double dz = 0.5 * m_GeoPar->getGapLength() * cm - gapHalfSize.z();
      char name[40];
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
      gapLogical->SetVisAttributes(G4VisAttributes(false));
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
      chimneyLogical->SetVisAttributes(G4VisAttributes(false));
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
      housingLogical->SetVisAttributes(G4VisAttributes(true, G4Colour(0.4, 0.4, 0.4)));
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
      shieldLogical->SetVisAttributes(G4VisAttributes(true, G4Colour(0.4, 0.4, 0.4)));
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
      pipeLogical->SetVisAttributes(G4VisAttributes(true, G4Colour(0.6, 0.6, 0.6)));
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

    void GeoBKLMCreator::putGapInLayer(G4LogicalVolume* layerIronLogical, int fb, int sector, int layer, bool hasChimney)
    {
      const double dz = 0.5 * m_GeoPar->getGapLength() * cm;
      char name[40];
      // Fill gap with air
      sprintf(name, "BKLM.Layer%02dGapBox", layer);
      const Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, hasChimney) * cm;
      G4Box* gapBox =
        new G4Box(name,
                  gapHalfSize.x(), gapHalfSize.y(), gapHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dGapLogical", layer);
      G4LogicalVolume* gapLogical =
        new G4LogicalVolume(gapBox,
                            Materials::get("G4_AIR"),
                            name
                           );
      gapLogical->SetVisAttributes(G4VisAttributes(false));
      if (m_GeoPar->hasRPCs(layer)) {
        putRPCModuleInGap(gapLogical, fb, sector, layer, hasChimney);
      } else {
        putScintModuleInGap(gapLogical, fb, sector, layer, hasChimney);
      }
      sprintf(name, "BKLM.Layer%02dGapPhysical", layer);
      new G4PVPlacement(G4Translate3D(m_GeoPar->getGapMiddleRadius(layer) * cm, 0.0, gapHalfSize.z() - dz),
                        gapLogical,
                        name,
                        layerIronLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putRPCModuleInGap(G4LogicalVolume* gapLogical, int fb, int sector, int layer, bool hasChimney)
    {
      char name[40];
      // Module is aluminum (but interior will be filled)
      sprintf(name, "BKLM.Layer%02dModuleBox", layer);
      const Hep3Vector gapHalfSize = m_GeoPar->getGapHalfSize(layer, hasChimney) * cm;
      const Hep3Vector moduleHalfSize = m_GeoPar->getModuleHalfSize(layer, hasChimney) * cm;
      double dx = (m_GeoPar->getModuleMiddleRadius(layer) - m_GeoPar->getGapMiddleRadius(layer)) * cm;
      double dz = moduleHalfSize.z() - gapHalfSize.z();
      G4Box* moduleBox =
        new G4Box(name,
                  moduleHalfSize.x(), moduleHalfSize.y(), moduleHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dModuleLogical", layer);
      G4LogicalVolume* moduleLogical =
        new G4LogicalVolume(moduleBox,
                            Materials::get("G4_Al"),
                            name
                           );
      moduleLogical->SetVisAttributes(G4VisAttributes(false));
      // Place readout inside aluminum frame
      sprintf(name, "BKLM.Layer%02dReadoutBox", layer);
      const Hep3Vector readoutHalfSize = m_GeoPar->getReadoutHalfSize(layer, hasChimney) * cm;
      G4Box* readoutBox =
        new G4Box(name,
                  readoutHalfSize.x(), readoutHalfSize.y(), readoutHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dReadoutLogical", layer);
      G4LogicalVolume* readoutLogical =
        new G4LogicalVolume(readoutBox,
                            Materials::get("RPCReadout"),
                            name
                           );
      readoutLogical->SetVisAttributes(G4VisAttributes(false));
      // Place electrode inside readout
      sprintf(name, "BKLM.Layer%02dElectrodeBox", layer);
      const Hep3Vector electrodeHalfSize = m_GeoPar->getElectrodeHalfSize(layer, hasChimney) * cm;
      G4Box* electrodeBox =
        new G4Box(name,
                  electrodeHalfSize.x(), electrodeHalfSize.y(), electrodeHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dElectrodeLogical", layer);
      G4LogicalVolume* electrodeLogical =
        new G4LogicalVolume(electrodeBox,
                            Materials::get("G4_GLASS_PLATE"),
                            name
                           );
      electrodeLogical->SetVisAttributes(G4VisAttributes(false));
      // Place two gas volumes inside electrodes
      sprintf(name, "BKLM.Layer%02dGasBox", layer);
      const Hep3Vector gasHalfSize = m_GeoPar->getGasHalfSize(layer, hasChimney) * cm;
      double dxGas = (m_GeoPar->getModuleGlassHeight() + 0.5 * m_GeoPar->getModuleGasHeight()) * cm;
      G4Box* gasBox =
        new G4Box(name,
                  gasHalfSize.x(), gasHalfSize.y(), gasHalfSize.z()
                 );
      sprintf(name, "BKLM.Layer%02dGasLogical", layer);
      G4LogicalVolume* gasLogical =
        new G4LogicalVolume(gasBox,
                            Materials::get("RPCGas"),
                            name,
                            0, // use global field manager
                            m_sensitive, // this is the only sensitive volume in BKLM
                            0 // no user limits
                           );
      gasLogical->SetVisAttributes(G4VisAttributes(true, G4Colour(1.0, 0.5, 0.0)));
      sprintf(name, "BKLM.Gas_%d_%d_%02d_0", fb, sector, layer);
      new G4PVPlacement(G4TranslateX3D(-dxGas),
                        gasLogical,
                        name,
                        electrodeLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      sprintf(name, "BKLM.Gas_%d_%d_%02d_1", fb, sector, layer);
      new G4PVPlacement(G4TranslateX3D(dxGas),
                        gasLogical,
                        name,
                        electrodeLogical,
                        false,
                        1,
                        m_GeoPar->doOverlapCheck()
                       );
      sprintf(name, "BKLM.Layer%02dElectrodePhysical", layer);
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        electrodeLogical,
                        name,
                        readoutLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      sprintf(name, "BKLM.Layer%02dReadoutPhysical", layer);
      new G4PVPlacement(G4TranslateZ3D(0.0),
                        readoutLogical,
                        name,
                        moduleLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
      sprintf(name, "BKLM.Layer%02dModulePhysical", layer);
      new G4PVPlacement(G4Translate3D(dx, 0.0, dz),
                        moduleLogical,
                        name,
                        gapLogical,
                        false,
                        0,
                        m_GeoPar->doOverlapCheck()
                       );
    }

    void GeoBKLMCreator::putScintModuleInGap(G4LogicalVolume* gapLogical, int fb, int sector, int layer, bool hasChimney)
    {
      // stub
    }

    G4Tubs* GeoBKLMCreator::solenoidCutout(void)
    {
      static G4Tubs* cutout = 0;

      if (cutout == 0) {
        cutout =
          new G4Tubs("BKLM.SolenoidCutout",
                     0.0,
                     m_GeoPar->getSolenoidOuterRadius() * cm,
                     2.0 * m_GeoPar->getHalfLength() * cm,
                     0.0,
                     2.0 * M_PI
                    );
      }
      return cutout;
    }

  } // namespace bklm

} // namespace Belle2
