/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/GeoEKLMCreator.h>

/* KLM headers. */
#include <klm/eklm/geometry/G4TriangularPrism.h>
#include <klm/eklm/simulation/EKLMSensitiveDetector.h>

/* Belle 2 headers. */
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

/* Geant4 headers. */
#include <G4Box.hh>
#include <G4Polyhedra.hh>
#include <G4PVPlacement.hh>
#include <G4Region.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>

/* C++ headers. */
#include <cmath>
#include <string>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

/* Register the creator */
geometry::CreatorFactory<EKLM::GeoEKLMCreator> GeoEKLMFactory("EKLMCreator");

/******************************* CONSTRUCTORS ********************************/

EKLM::GeoEKLMCreator::GeoEKLMCreator()
{
  m_Materials.air = nullptr;
  m_CurVol.section = 1;
  m_GeoDat = nullptr;
  m_TransformData = nullptr;
  m_Solids.plane = nullptr;
  m_Solids.psheet = nullptr;
  m_LogVol.psheet = nullptr;
  m_LogVol.strip = nullptr;
  m_Solids.groove = nullptr;
  m_LogVol.groove = nullptr;
  m_LogVol.scint = nullptr;
  m_LogVol.segmentsup = nullptr;
  m_ElementNumbers = &(EKLMElementNumbers::Instance());
  m_Sensitive = nullptr;
}

EKLM::GeoEKLMCreator::~GeoEKLMCreator()
{
  delete m_TransformData;
  if (m_Solids.plane != nullptr)
    deleteVolumes();
  if (m_Sensitive != nullptr)
    deleteSensitive();
}

/***************************** MEMORY ALLOCATION *****************************/

void EKLM::GeoEKLMCreator::newVolumes()
{
  int i, nDiff;
  m_Solids.plane =
    (G4VSolid**)malloc(m_GeoDat->getNPlanes() * sizeof(G4VSolid*));
  if (m_Solids.plane == nullptr)
    B2FATAL(MemErr);
  m_Solids.plasticSheetElement =
    (G4VSolid**)malloc(m_GeoDat->getNStrips() * sizeof(G4VSolid*));
  if (m_Solids.plasticSheetElement == nullptr)
    B2FATAL(MemErr);
  m_Solids.psheet =
    (G4VSolid**)malloc(m_GeoDat->getNSegments() * sizeof(G4VSolid*));
  if (m_Solids.psheet == nullptr)
    B2FATAL(MemErr);
  m_LogVol.psheet = (G4LogicalVolume**)
                    malloc(m_GeoDat->getNSegments() * sizeof(G4LogicalVolume*));
  if (m_LogVol.psheet == nullptr)
    B2FATAL(MemErr);
  m_LogVol.segment =
    (G4LogicalVolume**)malloc(m_GeoDat->getNSegments() *
                              sizeof(G4LogicalVolume*));
  if (m_LogVol.segment == nullptr)
    B2FATAL(MemErr);
  m_Solids.stripSegment =
    (G4VSolid**)malloc(m_GeoDat->getNSegments() * sizeof(G4VSolid*));
  if (m_Solids.stripSegment == nullptr)
    B2FATAL(MemErr);
  m_LogVol.stripSegment =
    (G4LogicalVolume**)malloc(m_GeoDat->getNSegments() *
                              sizeof(G4LogicalVolume*));
  if (m_LogVol.stripSegment == nullptr)
    B2FATAL(MemErr);
  nDiff = m_GeoDat->getNStripsDifferentLength();
  m_Solids.strip = (G4VSolid**)malloc(nDiff * sizeof(G4VSolid*));
  if (m_Solids.strip == nullptr)
    B2FATAL(MemErr);
  m_LogVol.strip = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.strip == nullptr)
    B2FATAL(MemErr);
  m_Solids.groove = (G4VSolid**)malloc(nDiff * sizeof(G4VSolid*));
  if (m_Solids.groove == nullptr)
    B2FATAL(MemErr);
  m_LogVol.groove = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.groove == nullptr)
    B2FATAL(MemErr);
  m_LogVol.scint = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.scint == nullptr)
    B2FATAL(MemErr);
  m_LogVol.segmentsup =
    (G4LogicalVolume***)malloc(m_GeoDat->getNPlanes() *
                               sizeof(G4LogicalVolume**));
  if (m_LogVol.segmentsup == nullptr)
    B2FATAL(MemErr);
  for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
    m_LogVol.segmentsup[i] =
      (G4LogicalVolume**)malloc((m_GeoDat->getNSegments() + 1) *
                                sizeof(G4LogicalVolume*));
    if (m_LogVol.segmentsup[i] == nullptr)
      B2FATAL(MemErr);
  }
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    m_LogVol.psheet[i] = nullptr;
}

void EKLM::GeoEKLMCreator::newSensitive()
{
  try {
    m_Sensitive =
      new EKLMSensitiveDetector("EKLMSensitiveStrip");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::deleteVolumes()
{
  int i;
  free(m_Solids.plane);
  free(m_Solids.plasticSheetElement);
  free(m_Solids.psheet);
  free(m_LogVol.psheet);
  free(m_LogVol.segment);
  free(m_Solids.stripSegment);
  free(m_LogVol.stripSegment);
  free(m_Solids.strip);
  free(m_LogVol.strip);
  free(m_Solids.groove);
  free(m_LogVol.groove);
  free(m_LogVol.scint);
  for (i = 0; i < m_GeoDat->getNPlanes(); i++)
    free(m_LogVol.segmentsup[i]);
  free(m_LogVol.segmentsup);
}

void EKLM::GeoEKLMCreator::deleteSensitive()
{
  delete m_Sensitive;
}

/********************************** XML DATA *********************************/

void EKLM::GeoEKLMCreator::createMaterials()
{
  m_Materials.air = geometry::Materials::get("Air");
  m_Materials.polyethylene = geometry::Materials::get("EKLMPolyethylene");
  m_Materials.polystyrene = geometry::Materials::get("EKLMPolystyrene");
  m_Materials.polystyrol = geometry::Materials::get("EKLMPolystyrol");
  m_Materials.gel = geometry::Materials::get("EKLMGel");
  m_Materials.iron = geometry::Materials::get("EKLMIron");
  m_Materials.duralumin = geometry::Materials::get("EKLMDuralumin");
  m_Materials.silicon = geometry::Materials::get("EKLMSilicon");
}

/*************************** CREATION OF SOLIDS ******************************/

void EKLM::GeoEKLMCreator::createSectionSolid()
{
  const EKLMGeometry::EndcapStructureGeometry* sectionStructureGeometry =
    m_GeoDat->getEndcapStructureGeometry();
  const EKLMGeometry::ElementPosition* sectionPos =
    m_GeoDat->getSectionPosition();
  const double z[2] = { -sectionPos->getLength() / 2,
                        sectionPos->getLength() / 2
                      };
  const double rMin[2] = {0, 0};
  const double rMax[2] = {sectionPos->getOuterR(), sectionPos->getOuterR()};
  G4Polyhedra* op = nullptr;
  G4Tubs* tb = nullptr;
  try {
    op = new G4Polyhedra("Section_Octagonal_Prism",
                         sectionStructureGeometry->getPhi(),
                         360. * CLHEP::deg,
                         sectionStructureGeometry->getNSides(),
                         2, z, rMin, rMax);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  /* Subtraction tube has slightly larger Z size. */
  try {
    tb = new G4Tubs("Section_Tube", 0, sectionPos->getInnerR(),
                    sectionPos->getLength() / 2.0 + 1.0 * CLHEP::mm,
                    0.0, 360.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_Solids.section = new G4SubtractionSolid("Section", op, tb);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4LogicalVolume*
EKLM::GeoEKLMCreator::createLayerLogicalVolume(const char* name) const
{
  G4LogicalVolume* logicLayer = nullptr;
  try {
    logicLayer = new G4LogicalVolume(m_Solids.layer, m_Materials.air, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicLayer, false);
  return logicLayer;
}

void EKLM::GeoEKLMCreator::createLayerSolid()
{
  const EKLMGeometry::ElementPosition* layerPos = m_GeoDat->getLayerPosition();
  try {
    m_Solids.layer =
      new G4Tubs("Layer", layerPos->getInnerR(), layerPos->getOuterR(),
                 layerPos->getLength() / 2.0, 0.0, 360. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4LogicalVolume*
EKLM::GeoEKLMCreator::createSectorLogicalVolume(const char* name) const
{
  G4LogicalVolume* logicSector = nullptr;
  try {
    logicSector = new G4LogicalVolume(m_Solids.sector, m_Materials.air, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSector, false);
  return logicSector;
}

void EKLM::GeoEKLMCreator::createSectorSolid()
{
  const EKLMGeometry::ElementPosition* sectorPos =
    m_GeoDat->getSectorPosition();
  try {
    m_Solids.sector =
      new G4Tubs("Sector", sectorPos->getInnerR(), sectorPos->getOuterR(),
                 0.5 * sectorPos->getLength(), 0.0, 90. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSectorCoverLogicalVolume()
{
  double lz, ang;
  HepGeom::Transform3D t1;
  HepGeom::Transform3D t2;
  G4Tubs* solidCoverTube = nullptr;
  G4Box* solidCoverBox = nullptr;
  G4Box* box = nullptr;
  G4IntersectionSolid* is = nullptr;
  G4SubtractionSolid* solidCover = nullptr;
  const EKLMGeometry::ElementPosition* sectorPos =
    m_GeoDat->getSectorPosition();
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const EKLMGeometry::ElementPosition* planePos = m_GeoDat->getPlanePosition();
  lz = 0.5 * (sectorPos->getLength() - sectorSupportPos->getLength());
  try {
    solidCoverTube = new G4Tubs("Cover_Tube", sectorSupportPos->getInnerR(),
                                sectorSupportPos->getOuterR(), 0.5 * lz, 0.0,
                                90.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCoverBox = new G4Box("Cover_Box", 0.5 * sectorSupportPos->getOuterR(),
                              0.5 * sectorSupportPos->getOuterR(), 0.5 * lz);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    box = new G4Box("Cover_SubtractionBox", 0.5 * sectorSupportPos->getOuterR(),
                    0.5 * sectorSupportPos->getOuterR(), lz);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(
         0.5 * planePos->getOuterR() + sectorSupportPos->getX(),
         0.5 * planePos->getOuterR() + sectorSupportPos->getY(), 0.);
  ang = sectorSupportGeometry->getCornerAngle();
  t2 = HepGeom::Translate3D(
         sectorSupportPos->getX() +
         0.5 * sectorSupportPos->getOuterR() * cos(ang) -
         0.5 * sectorSupportPos->getOuterR() * sin(ang),
         sectorSupportPos->getOuterR() -
         sectorSupportGeometry->getDeltaLY() +
         0.5 * sectorSupportPos->getOuterR() * cos(ang) +
         0.5 * sectorSupportPos->getOuterR() * sin(ang),
         0.) * HepGeom::RotateZ3D(ang);
  try {
    is = new G4IntersectionSolid("Cover_Intersection",
                                 solidCoverTube, solidCoverBox, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCover = new G4SubtractionSolid("Cover", is, box, t2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.cover =
      new G4LogicalVolume(solidCover, m_Materials.duralumin, "Cover");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.cover, false);
  geometry::setColor(*m_LogVol.cover, "#ff000022");
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxX(HepGeom::Transform3D& t)
{
  double x;
  G4Box* res = nullptr;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  x = sqrt(sectorSupportPos->getOuterR() * sectorSupportPos->getOuterR() -
           sectorSupportPos->getY() * sectorSupportPos->getY());
  t = HepGeom::Translate3D(0.5 * (x + sectorSupportGeometry->getCorner3().x()),
                           sectorSupportPos->getY() +
                           0.5 * sectorSupportGeometry->getThickness(), 0.);
  try {
    res = new G4Box("SectorSupport_BoxX",
                    0.5 * (x - sectorSupportGeometry->getCorner3().x()),
                    0.5 * sectorSupportGeometry->getThickness(),
                    0.5 * sectorSupportPos->getLength());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxY(HepGeom::Transform3D& t)
{
  G4Box* res = nullptr;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(sectorSupportPos->getX() +
                           0.5 * sectorSupportGeometry->getThickness(),
                           0.5 * (sectorSupportGeometry->getCorner4().y() +
                                  sectorSupportGeometry->getCorner1A().y()),
                           0.) * HepGeom::RotateZ3D(90. * CLHEP::deg);
  try {
    res = new G4Box("SectorSupport_BoxY",
                    0.5 * (sectorSupportGeometry->getCorner1A().y() -
                           sectorSupportGeometry->getCorner4().y()),
                    0.5 * sectorSupportGeometry->getThickness(),
                    0.5 * sectorSupportPos->getLength());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxTop(HepGeom::Transform3D& t)
{
  double dx;
  double dy;
  G4Box* res = nullptr;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(
        0.5 * (sectorSupportGeometry->getCorner1A().x() +
               sectorSupportGeometry->getCorner1B().x() +
               sectorSupportGeometry->getThickness() *
               sin(sectorSupportGeometry->getCornerAngle())),
        0.5 * (sectorSupportGeometry->getCorner1A().y() +
               sectorSupportGeometry->getCorner1B().y() -
               sectorSupportGeometry->getThickness() *
               cos(sectorSupportGeometry->getCornerAngle())),
        0.) * HepGeom::RotateZ3D(sectorSupportGeometry->getCornerAngle());
  dx = sectorSupportGeometry->getCorner1B().x() -
       sectorSupportGeometry->getCorner1A().x();
  dy = sectorSupportGeometry->getCorner1B().y() -
       sectorSupportGeometry->getCorner1B().y();
  try {
    res = new G4Box("SectorSupport_BoxTop", 0.5 * sqrt(dx * dx + dy * dy),
                    0.5 * sectorSupportGeometry->getThickness(),
                    0.5 * sectorSupportPos->getLength());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Tubs* EKLM::GeoEKLMCreator::createSectorSupportInnerTube()
{
  double ang1;
  double ang2;
  G4Tubs* res = nullptr;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  ang1 = atan2(sectorSupportGeometry->getCorner3().y(),
               sectorSupportGeometry->getCorner3().x());
  ang2 = atan2(sectorSupportGeometry->getCorner4().y(),
               sectorSupportGeometry->getCorner4().x());
  try {
    res = new G4Tubs("SectorSupport_InnerTube", sectorSupportPos->getInnerR(),
                     sectorSupportPos->getInnerR() +
                     sectorSupportGeometry->getThickness(),
                     0.5 * sectorSupportPos->getLength(),
                     ang1 * CLHEP::rad, (ang2 - ang1) * CLHEP::rad);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Tubs* EKLM::GeoEKLMCreator::createSectorSupportOuterTube()
{
  double x;
  double ang1;
  double ang2;
  double r;
  G4Tubs* res = nullptr;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->getOuterR() - sectorSupportGeometry->getThickness();
  x = sqrt(r * r - sectorSupportPos->getY() * sectorSupportPos->getY());
  ang1 = atan2(sectorSupportPos->getY(), x);
  ang2 = atan2(sectorSupportGeometry->getCorner1B().y(),
               sectorSupportGeometry->getCorner1B().x());
  try {
    res = new G4Tubs("SectorSupport_OuterTube", r,
                     sectorSupportPos->getOuterR(),
                     0.5 * sectorSupportPos->getLength(),
                     ang1 * CLHEP::rad, (ang2 - ang1) * CLHEP::rad);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner1LogicalVolume()
{
  double lx;
  double x;
  G4Tubs* solidCorner1Tube = nullptr;
  G4Box* solidCorner1Box1 = nullptr;
  G4Box* solidCorner1Box2 = nullptr;
  G4IntersectionSolid* is1 = nullptr;
  G4IntersectionSolid* solidCorner1 = nullptr;
  HepGeom::Transform3D t1;
  HepGeom::Transform3D t2;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  lx = sectorSupportGeometry->getCornerX() +
       sectorSupportGeometry->getCorner1LX() -
       sectorSupportGeometry->getThickness();
  try {
    solidCorner1Tube = new G4Tubs(
      "SectorSupport_Corner1_Tube", 0.,
      sectorSupportPos->getOuterR() - sectorSupportGeometry->getThickness(),
      0.5 * sectorSupportGeometry->getCorner1Thickness(),
      0., 90. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1Box1 = new G4Box(
      "SectorSupport_Corner1_Box1", 0.5 * lx,
      0.5 * sectorSupportPos->getOuterR(),
      0.5 * sectorSupportGeometry->getCorner1Thickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1Box2 =
      new G4Box("SectorSupport_Corner1_Box2",
                0.5 * (lx / cos(sectorSupportGeometry->getCornerAngle()) +
                       sectorSupportGeometry->getCorner1Width() *
                       tan(sectorSupportGeometry->getCornerAngle())),
                0.5 * sectorSupportGeometry->getCorner1Width(),
                0.5 * sectorSupportGeometry->getCorner1Thickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  x = sectorSupportPos->getX() + 0.5 * (sectorSupportGeometry->getCornerX() +
                                        sectorSupportGeometry->getCorner1LX() +
                                        sectorSupportGeometry->getThickness());
  t1 = HepGeom::Translate3D(x, 0.5 * sectorSupportPos->getOuterR(), 0.);
  t2 = HepGeom::Translate3D(
         x, sectorSupportGeometry->getCorner1AInner().y() -
         0.5 * sectorSupportGeometry->getCorner1Width() /
         cos(sectorSupportGeometry->getCornerAngle()) +
         0.5 * lx * tan(sectorSupportGeometry->getCornerAngle()), 0.) *
       HepGeom::RotateZ3D(sectorSupportGeometry->getCornerAngle());
  try {
    is1 = new G4IntersectionSolid("SectorSupport_Corner1_Intersection1",
                                  solidCorner1Tube, solidCorner1Box1, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1 = new G4IntersectionSolid("SectorSupport_Corner1", is1,
                                           solidCorner1Box2, t2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.sectorsup.corn1 =
      new G4LogicalVolume(solidCorner1, m_Materials.duralumin, "Corner1");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.sectorsup.corn1, true);
  geometry::setColor(*m_LogVol.sectorsup.corn1, "#ff0000ff");
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner2LogicalVolume()
{
  G4TriangularPrism* solidCorner2Prism = nullptr;
  G4SubtractionSolid* solidCorner2 = nullptr;
  HepGeom::Transform3D t1;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  try {
    solidCorner2Prism =
      new G4TriangularPrism("SectorSupport_Corner2_Prism",
                            sectorSupportGeometry->getCorner2LY(),
                            90. * CLHEP::deg,
                            sectorSupportGeometry->getCorner2LX(),
                            180. * CLHEP::deg,
                            0.5 * sectorSupportGeometry->getCorner2Thickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(-sectorSupportGeometry->getCorner2Inner().x(),
                            -sectorSupportGeometry->getCorner2Inner().y(), 0);
  try {
    solidCorner2 = new G4SubtractionSolid("SectorSupport_Corner2",
                                          solidCorner2Prism->getSolid(),
                                          m_Solids.sectorsup.otube, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  delete solidCorner2Prism;
  try {
    m_LogVol.sectorsup.corn2 =
      new G4LogicalVolume(solidCorner2, m_Materials.duralumin, "Corner2");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.sectorsup.corn2, true);
  geometry::setColor(*m_LogVol.sectorsup.corn2, "#ff0000ff");
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner3LogicalVolume()
{
  G4TriangularPrism* solidCorner3Prism = nullptr;
  G4SubtractionSolid* solidCorner3 = nullptr;
  HepGeom::Transform3D t1;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  try {
    solidCorner3Prism =
      new G4TriangularPrism("SectorSupport_Corner3_Prism",
                            sectorSupportGeometry->getCorner3LX(), 0.,
                            sectorSupportGeometry->getCorner3LY(),
                            90. * CLHEP::deg,
                            0.5 * sectorSupportGeometry->getCorner3Thickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(-sectorSupportGeometry->getCorner3Prism().x(),
                            -sectorSupportGeometry->getCorner3Prism().y(), 0.);
  try {
    solidCorner3 = new G4SubtractionSolid("SectorSupport_Corner3",
                                          solidCorner3Prism->getSolid(),
                                          m_Solids.sectorsup.itube, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  delete solidCorner3Prism;
  try {
    m_LogVol.sectorsup.corn3 =
      new G4LogicalVolume(solidCorner3, m_Materials.duralumin, "Corner3");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.sectorsup.corn3, true);
  geometry::setColor(*m_LogVol.sectorsup.corn3, "#ff0000ff");
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner4LogicalVolume()
{
  G4TriangularPrism* solidCorner4Prism = nullptr;
  G4SubtractionSolid* solidCorner4 = nullptr;
  HepGeom::Transform3D t1;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  try {
    solidCorner4Prism =
      new G4TriangularPrism("SectorSupport_Corner4_Prism",
                            sectorSupportGeometry->getCorner4LX(), 0.,
                            sectorSupportGeometry->getCorner4LY(),
                            90. * CLHEP::deg,
                            0.5 * sectorSupportGeometry->getCorner4Thickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(-sectorSupportGeometry->getCorner4Prism().x(),
                            -sectorSupportGeometry->getCorner4Prism().y(), 0.);
  try {
    solidCorner4 = new G4SubtractionSolid("SectorSupport_Corner4",
                                          solidCorner4Prism->getSolid(),
                                          m_Solids.sectorsup.itube, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  delete solidCorner4Prism;
  try {
    m_LogVol.sectorsup.corn4 =
      new G4LogicalVolume(solidCorner4, m_Materials.duralumin, "Corner4");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.sectorsup.corn4, true);
  geometry::setColor(*m_LogVol.sectorsup.corn4, "#ff0000ff");
}

void EKLM::GeoEKLMCreator::createSectorSupportLogicalVolume()
{
  G4Box* solidBoxX;
  G4Box* solidBoxY;
  G4Box* solidBoxTop;
  G4Tubs* solidLimitationTube = nullptr;
  G4UnionSolid* us1 = nullptr;
  G4UnionSolid* us2 = nullptr;
  G4UnionSolid* us3 = nullptr;
  G4UnionSolid* us4 = nullptr;
  G4IntersectionSolid* solidSectorSupport = nullptr;
  HepGeom::Transform3D tbx;
  HepGeom::Transform3D tby;
  HepGeom::Transform3D tbt;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  solidBoxX = createSectorSupportBoxX(tbx);
  solidBoxY = createSectorSupportBoxY(tby);
  solidBoxTop = createSectorSupportBoxTop(tbt);
  m_Solids.sectorsup.otube = createSectorSupportOuterTube();
  m_Solids.sectorsup.itube = createSectorSupportInnerTube();
  try {
    solidLimitationTube = new G4Tubs("SectorSupport_LimitationTube",
                                     0., sectorSupportPos->getOuterR(),
                                     0.5 * sectorSupportPos->getLength(),
                                     0., 90.*CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    us1 = new G4UnionSolid("SectorSupport_Union1",
                           m_Solids.sectorsup.itube, solidBoxY, tby);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    us2 = new G4UnionSolid("SectorSupport_Union2",
                           us1, solidBoxX, tbx);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    us3 = new G4UnionSolid("SectorSupport_Union3", us2,
                           m_Solids.sectorsup.otube,
                           HepGeom::Translate3D(0., 0., 0.));
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    us4 = new G4UnionSolid("SectorSupport_Union4",
                           us3, solidBoxTop, tbt);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidSectorSupport =
      new G4IntersectionSolid("SectorSupport", us4, solidLimitationTube,
                              HepGeom::Translate3D(0., 0., 0.));
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.sectorsup.supp =
      new G4LogicalVolume(solidSectorSupport, m_Materials.duralumin, "Support");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.sectorsup.supp, true);
  geometry::setColor(*m_LogVol.sectorsup.supp, "#ff0000ff");
}

void EKLM::GeoEKLMCreator::createSubtractionBoxSolid()
{
  const EKLMGeometry::ElementPosition* layerPos = m_GeoDat->getLayerPosition();
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  try {
    m_Solids.subtractionBox =
      new G4Box("SubractionBox", 0.5 * sectorSupportPos->getOuterR(),
                0.5 * sectorSupportPos->getOuterR(), layerPos->getLength());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4SubtractionSolid* EKLM::GeoEKLMCreator::
cutSolidCorner(const char* name, G4VSolid* solid, G4Box* subtractionBox,
               const HepGeom::Transform3D& transf, bool largerAngles,
               double x1, double y1, double x2, double y2)
{
  double ang;
  ang = atan2(y2 - y1, x2 - x1);
  return cutSolidCorner(name, solid, subtractionBox, transf, largerAngles,
                        x1, y1, ang);
}

G4SubtractionSolid* EKLM::GeoEKLMCreator::
cutSolidCorner(const char* name, G4VSolid* solid, G4Box* subtractionBox,
               const HepGeom::Transform3D& transf, bool largerAngles,
               double x, double y, double ang)
{
  double lx, ly;
  HepGeom::Transform3D t;
  G4SubtractionSolid* ss = nullptr;
  lx = subtractionBox->GetXHalfLength();
  ly = subtractionBox->GetYHalfLength();
  if (largerAngles)
    t = HepGeom::Translate3D(x + lx * cos(ang) - ly * sin(ang),
                             y + lx * sin(ang) + ly * cos(ang), 0) *
        HepGeom::RotateZ3D(ang);
  else
    t = HepGeom::Translate3D(x + lx * cos(ang) + ly * sin(ang),
                             y + lx * sin(ang) - ly * cos(ang), 0) *
        HepGeom::RotateZ3D(ang);
  try {
    ss = new G4SubtractionSolid(name, solid, subtractionBox, transf * t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return ss;
}

void EKLM::GeoEKLMCreator::createPlaneSolid(int n)
{
  double box_x;
  double box_y;
  HepGeom::Transform3D t;
  HepGeom::Transform3D t1;
  char name[128];
  G4Tubs* tb = nullptr;
  G4IntersectionSolid* is = nullptr;
  G4SubtractionSolid* ss1, *ss2, *ss3, *ss4;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const EKLMGeometry::ElementPosition* planePos = m_GeoDat->getPlanePosition();
  /* Basic solids. */
  snprintf(name, 128, "Plane_%d_Tube", n + 1);
  try {
    tb = new G4Tubs(name, planePos->getInnerR(), planePos->getOuterR(),
                    0.5 * planePos->getLength(), 0.0, 90.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Box", n + 1);
  box_x = sectorSupportPos->getX() + sectorSupportGeometry->getThickness();
  box_y = sectorSupportPos->getY() + sectorSupportGeometry->getThickness();
  /* Calculate transformations for boolean solids. */
  t1 = HepGeom::Translate3D(m_Solids.subtractionBox->GetXHalfLength() + box_x,
                            m_Solids.subtractionBox->GetXHalfLength() + box_y,
                            0.);
  /* For rotated plane. */
  if (n == 0) {
    t = HepGeom::Rotate3D(180. * CLHEP::deg,
                          HepGeom::Vector3D<double>(1., 1., 0.));
    t1 = t * t1;
  } else
    t = HepGeom::Translate3D(0, 0, 0);
  /* Boolean solids. */
  snprintf(name, 128, "Plane_%d_Intersection", n + 1);
  try {
    is = new G4IntersectionSolid(name, tb, m_Solids.subtractionBox, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Subtraction_1", n + 1);
  ss1 = cutSolidCorner(name, is, m_Solids.subtractionBox, t, true,
                       sectorSupportGeometry->getCorner1AInner().x(),
                       sectorSupportGeometry->getCorner1AInner().y(),
                       sectorSupportGeometry->getCornerAngle());
  snprintf(name, 128, "Plane_%d_Subtraction_2", n + 1);
  ss2 = cutSolidCorner(
          name, ss1, m_Solids.subtractionBox, t, false,
          sectorSupportGeometry->getCorner2Inner().x() -
          sectorSupportGeometry->getCorner2LX(),
          sectorSupportGeometry->getCorner2Inner().y(),
          sectorSupportGeometry->getCorner2Inner().x(),
          sectorSupportGeometry->getCorner2Inner().y() +
          sectorSupportGeometry->getCorner2LY());
  snprintf(name, 128, "Plane_%d_Subtraction_3", n + 1);
  ss3 = cutSolidCorner(
          name, ss2, m_Solids.subtractionBox, t, false,
          sectorSupportGeometry->getCorner3Prism().x(),
          sectorSupportGeometry->getCorner3Prism().y() +
          sectorSupportGeometry->getCorner3LY(),
          sectorSupportGeometry->getCorner3Prism().x() +
          sectorSupportGeometry->getCorner3LX(),
          sectorSupportGeometry->getCorner3Prism().y());
  snprintf(name, 128, "Plane_%d_Subtraction_4", n + 1);
  ss4 = cutSolidCorner(
          name, ss3, m_Solids.subtractionBox, t, true,
          sectorSupportGeometry->getCorner4Prism().x() +
          sectorSupportGeometry->getCorner4LX(),
          sectorSupportGeometry->getCorner4Prism().y(),
          sectorSupportGeometry->getCorner4Prism().x(),
          sectorSupportGeometry->getCorner4Prism().y() +
          sectorSupportGeometry->getCorner4LY());
  snprintf(name, 128, "Plane_%d", n + 1);
  m_Solids.plane[n] = ss4;
}

void EKLM::GeoEKLMCreator::
createSegmentSupportLogicalVolume(int iPlane, int iSegmentSupport)
{
  HepGeom::Transform3D t1, t2;
  G4Box* topBox = nullptr;
  G4Box* midBox = nullptr;
  G4Box* botBox = nullptr;
  G4UnionSolid* us = nullptr;
  G4UnionSolid* solidSegmentSupport = nullptr;
  std::string segmentSupportName =
    "SegmentSupport_" + std::to_string(iSegmentSupport) +
    "_Plane_" + std::to_string(iPlane);
  const EKLMGeometry::SegmentSupportPosition* segmentSupportPos =
    m_GeoDat->getSegmentSupportPosition(iPlane, iSegmentSupport);
  const EKLMGeometry::SegmentSupportGeometry* segmentSupportGeometry =
    m_GeoDat->getSegmentSupportGeometry();
  try {
    topBox = new G4Box("BoxTop_" + segmentSupportName,
                       0.5 * (segmentSupportPos->getLength() -
                              segmentSupportPos->getDeltaLLeft() -
                              segmentSupportPos->getDeltaLRight()),
                       0.5 * segmentSupportGeometry->getTopWidth(),
                       0.5 * segmentSupportGeometry->getTopThickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    midBox = new G4Box("BoxMiddle_" + segmentSupportName,
                       0.5 * (segmentSupportPos->getLength() -
                              segmentSupportPos->getDeltaLLeft() -
                              segmentSupportPos->getDeltaLRight()),
                       0.5 * segmentSupportGeometry->getMiddleWidth(),
                       0.5 * segmentSupportGeometry->getMiddleThickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    botBox = new G4Box("BoxBottom_" + segmentSupportName,
                       0.5 * segmentSupportPos->getLength(),
                       0.5 * segmentSupportGeometry->getTopWidth(),
                       0.5 * segmentSupportGeometry->getTopThickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(
         0., 0., 0.5 * (segmentSupportGeometry->getMiddleThickness() +
                        segmentSupportGeometry->getTopThickness()));
  t2 = HepGeom::Translate3D(
         0.5 * (segmentSupportPos->getDeltaLRight() -
                segmentSupportPos->getDeltaLLeft()), 0.,
         -0.5 * (segmentSupportGeometry->getMiddleThickness() +
                 segmentSupportGeometry->getTopThickness()));
  try {
    us = new G4UnionSolid("Union1_" + segmentSupportName, midBox, topBox, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidSegmentSupport = new G4UnionSolid(segmentSupportName, us, botBox, t2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.segmentsup[iPlane - 1][iSegmentSupport - 1] =
      new G4LogicalVolume(solidSegmentSupport, m_Materials.duralumin,
                          segmentSupportName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.segmentsup[iPlane - 1][iSegmentSupport - 1],
                          true);
  geometry::setColor(*m_LogVol.segmentsup[iPlane - 1][iSegmentSupport - 1],
                     "#ff0000ff");
}

G4VSolid* EKLM::GeoEKLMCreator::
unifySolids(G4VSolid** solids, HepGeom::Transform3D* transf,
            int nSolids, const std::string& name)
{
  G4UnionSolid** u;
  G4DisplacedSolid* d;
  G4VSolid** solidArray;
  HepGeom::Transform3D* inverseTransf;
  HepGeom::Transform3D t;
  char str[128];
  /* cppcheck-suppress variableScope */
  int n, nUnions, i, i1, i2, k, k1, k2, l, dl;
  if (nSolids <= 1)
    B2FATAL("Number of solids to be unified must be greater than 1.");
  try {
    inverseTransf = new HepGeom::Transform3D[nSolids];
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < nSolids; i++)
    inverseTransf[i] = transf[i].inverse();
  n = nSolids;
  nUnions = 0;
  while (n > 1) {
    if (n % 2 == 0)
      n = n / 2;
    else
      n = n / 2 + 1;
    nUnions = nUnions + n;
  }
  u = (G4UnionSolid**)malloc(sizeof(G4UnionSolid*) * nUnions);
  if (u == nullptr)
    B2FATAL(MemErr);
  n = nSolids;
  i2 = 0;
  solidArray = solids;
  k1 = 0;
  k2 = nSolids;
  dl = 1;
  while (n > 1) {
    i1 = i2;
    if (n % 2 == 0)
      n = n / 2;
    else
      n = n / 2 + 1;
    i2 = i1 + n;
    k = k1;
    l = 0;
    for (i = i1; i < i2; i++) {
      if (k != k2 - 1) {
        /* Unify k and k + 1 -> i */
        t = inverseTransf[l] * transf[l + dl];
        try {
          snprintf(str, 128, "_Union_%d", i + 1);
          u[i] = new G4UnionSolid(name + str, solidArray[k],
                                  solidArray[k + 1], t);
        } catch (std::bad_alloc& ba) {
          B2FATAL(MemErr);
        }
      } else {
        /* Copy k -> i */
        u[i] = (G4UnionSolid*)solids[k];
      }
      k = k + 2;
      l = l + dl * 2;
    }
    solidArray = (G4VSolid**)u;
    k1 = i1;
    k2 = i2;
    dl = dl * 2;
  }
  d = new G4DisplacedSolid(name + "_Displaced", u[nUnions - 1], transf[0]);
  free(u);
  delete[] inverseTransf;
  return d;
}

void EKLM::GeoEKLMCreator::createPlasticSheetLogicalVolume(int iSegment)
{
  int i, m, nStrip;
  /* cppcheck-suppress variableScope */
  double ly;
  char name[128];
  G4VSolid** elements;
  HepGeom::Transform3D* t;
  const EKLMGeometry::PlasticSheetGeometry* plasticSheetGeometry =
    m_GeoDat->getPlasticSheetGeometry();
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  nStrip = m_ElementNumbers->getNStripsSegment();
  elements = new G4VSolid*[nStrip];
  t = new HepGeom::Transform3D[nStrip];
  /* Transformations. */
  for (i = 0; i < nStrip; i++) {
    m = nStrip * iSegment + i;
    m_GeoDat->getSheetTransform(&(t[i]), m);
  }
  /* Sheet elements. */
  for (i = 0; i < nStrip; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Element_%d", iSegment + 1, i + 1);
    ly = stripGeometry->getWidth();
    if (i == 0 || i == nStrip - 1)
      ly = ly - plasticSheetGeometry->getDeltaL();
    m = nStrip * iSegment + i;
    const EKLMGeometry::ElementPosition* stripPos =
      m_GeoDat->getStripPosition(m + 1);
    try {
      m_Solids.plasticSheetElement[m] =
        new G4Box(name, 0.5 * stripPos->getLength(), 0.5 * ly,
                  0.5 * plasticSheetGeometry->getWidth());
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    elements[i] = m_Solids.plasticSheetElement[m];
  }
  /* Union. */
  snprintf(name, 128, "PlasticSheet_%d", iSegment + 1);
  m_Solids.psheet[iSegment] = unifySolids(elements, t, nStrip, name);
  try {
    m_LogVol.psheet[iSegment] =
      new G4LogicalVolume(m_Solids.psheet[iSegment],
                          m_Materials.polystyrol, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.psheet[iSegment], false);
  geometry::setColor(*m_LogVol.psheet[iSegment], "#00ff00ff");
  delete[] elements;
  delete[] t;
}

void EKLM::GeoEKLMCreator::createStripSegmentLogicalVolume(int iSegment)
{
  /* cppcheck-suppress variableScope */
  int i, m, nStrip;
  char name[128];
  G4VSolid** strips;
  HepGeom::Transform3D* t;
  nStrip = m_ElementNumbers->getNStripsSegment();
  strips = new G4VSolid*[nStrip];
  t = new HepGeom::Transform3D[nStrip];
  for (i = 0; i < nStrip; i++) {
    m = nStrip * iSegment + i;
    m_GeoDat->getStripTransform(&(t[i]), m);
    strips[i] = m_Solids.strip[m_GeoDat->getStripLengthIndex(m)];
  }
  snprintf(name, 128, "StripSegment_%d", iSegment + 1);
  m_Solids.stripSegment[iSegment] = unifySolids(strips, t, nStrip, name);
  try {
    m_LogVol.stripSegment[iSegment] =
      new G4LogicalVolume(m_Solids.stripSegment[iSegment], m_Materials.air,
                          name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.stripSegment[iSegment], false);
  geometry::setColor(*m_LogVol.stripSegment[iSegment], "#00ff00ff");
  delete[] strips;
  delete[] t;
}

void EKLM::GeoEKLMCreator::createSegmentLogicalVolume(int iSegment)
{
  double z;
  char name[128];
  G4UnionSolid* u1, *u2;
  HepGeom::Transform3D t;
  const EKLMGeometry::PlasticSheetGeometry* plasticSheetGeometry =
    m_GeoDat->getPlasticSheetGeometry();
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  z = 0.5 * (stripGeometry->getThickness() + plasticSheetGeometry->getWidth());
  snprintf(name, 128, "Segment_%d_Union_1", iSegment + 1);
  t = HepGeom::Translate3D(0, 0, -z);
  try {
    u1 = new G4UnionSolid(name, m_Solids.stripSegment[iSegment],
                          m_Solids.psheet[iSegment], t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Segment_%d", iSegment + 1);
  t = HepGeom::Translate3D(0, 0, z);
  try {
    u2 = new G4UnionSolid(name, u1, m_Solids.psheet[iSegment], t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.segment[iSegment] =
      new G4LogicalVolume(u2, m_Materials.air, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.segment[iSegment], false);
  geometry::setColor(*m_LogVol.segment[iSegment], "#00ff00ff");
}

void EKLM::GeoEKLMCreator::createStripLogicalVolume(int iStrip)
{
  int iPos;
  char name[128];
  const EKLMGeometry::ElementPosition* stripPos;
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "Strip_%d", iStrip + 1);
  try {
    m_Solids.strip[iStrip] = new G4Box(name, 0.5 * stripPos->getLength(),
                                       0.5 * stripGeometry->getWidth(),
                                       0.5 * stripGeometry->getThickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.strip[iStrip] =
      new G4LogicalVolume(m_Solids.strip[iStrip], m_Materials.polystyrene,
                          name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.strip[iStrip], true);
  geometry::setColor(*m_LogVol.strip[iStrip], "#ffffffff");
}

void EKLM::GeoEKLMCreator::createStripGrooveLogicalVolume(int iStrip)
{
  int iPos;
  char name[128];
  const EKLMGeometry::ElementPosition* stripPos;
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "Groove_%d", iStrip + 1);
  try {
    m_Solids.groove[iStrip] = new G4Box(name, 0.5 * stripPos->getLength(),
                                        0.5 * stripGeometry->getGrooveWidth(),
                                        0.5 * stripGeometry->getGrooveDepth());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.groove[iStrip] =
      new G4LogicalVolume(m_Solids.groove[iStrip], m_Materials.gel, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.groove[iStrip], true);
  geometry::setColor(*m_LogVol.groove[iStrip], "#00ff00ff");
}

void EKLM::GeoEKLMCreator::createScintillatorLogicalVolume(int iStrip)
{
  int iPos;
  char name[128];
  const EKLMGeometry::ElementPosition* stripPos;
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  HepGeom::Transform3D t;
  G4Box* b;
  G4SubtractionSolid* scintillatorSolid;
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "StripSensitive_%d_Box", iStrip + 1);
  try {
    b = new G4Box(name,
                  0.5 * stripPos->getLength() -
                  stripGeometry->getNoScintillationThickness(),
                  0.5 * stripGeometry->getWidth() -
                  stripGeometry->getNoScintillationThickness(),
                  0.5 * stripGeometry->getThickness() -
                  stripGeometry->getNoScintillationThickness());
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "StripSensitive_%d", iStrip + 1);
  t = HepGeom::Translate3D(0., 0., 0.5 * (stripGeometry->getThickness() -
                                          stripGeometry->getGrooveDepth()));
  try {
    scintillatorSolid =
      new G4SubtractionSolid(name, b, m_Solids.groove[iStrip], t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.scint[iStrip] =
      new G4LogicalVolume(scintillatorSolid, m_Materials.polystyrene,
                          name, 0, m_Sensitive, 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.scint[iStrip], false);
  geometry::setColor(*m_LogVol.scint[iStrip], "#ffffffff");
}

void EKLM::GeoEKLMCreator::createShieldDetailALogicalVolume()
{
  G4Box* box = nullptr;
  G4SubtractionSolid* ss1, *ss2, *ss3, *solidDetailA;
  double lx, ly;
  const EKLMGeometry::ShieldGeometry* shieldGeometry =
    m_GeoDat->getShieldGeometry();
  const EKLMGeometry::ShieldDetailGeometry* detailA =
    shieldGeometry->getDetailA();
  HepGeom::Transform3D t = HepGeom::Translate3D(0, 0, 0);
  lx = detailA->getLengthX() / 2;
  ly = detailA->getLengthY() / 2;
  try {
    box = new G4Box("ShieldDetailA_Box", lx, ly,
                    shieldGeometry->getThickness() / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  ss1 = cutSolidCorner("ShieldDetailA_Subtraction_1", box,
                       m_Solids.subtractionBox, t, false,
                       detailA->getPoint(0)->getX() - lx,
                       detailA->getPoint(0)->getY() - ly,
                       detailA->getPoint(1)->getX() - lx,
                       detailA->getPoint(1)->getY() - ly);
  ss2 = cutSolidCorner("ShieldDetailA_Subtraction_2", ss1,
                       m_Solids.subtractionBox, t, true,
                       detailA->getPoint(3)->getX() - lx,
                       detailA->getPoint(3)->getY() - ly,
                       detailA->getPoint(2)->getX() - lx,
                       detailA->getPoint(2)->getY() - ly);
  ss3 = cutSolidCorner("ShieldDetailA_Subtraction_3", ss2,
                       m_Solids.subtractionBox, t, false,
                       detailA->getPoint(3)->getX() - lx,
                       detailA->getPoint(3)->getY() - ly,
                       detailA->getPoint(4)->getX() - lx,
                       detailA->getPoint(4)->getY() - ly);
  solidDetailA = cutSolidCorner("ShieldDetailA", ss3,
                                m_Solids.subtractionBox, t, false,
                                detailA->getPoint(5)->getX() - lx,
                                detailA->getPoint(5)->getY() - ly,
                                detailA->getPoint(6)->getX() - lx,
                                detailA->getPoint(6)->getY() - ly);
  try {
    m_LogVol.shield.detailA =
      new G4LogicalVolume(solidDetailA, m_Materials.polyethylene,
                          "ShieldDetailA");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.shield.detailA, true);
  geometry::setColor(*m_LogVol.shield.detailA, "#00ff00ff");
}

void EKLM::GeoEKLMCreator::createShieldDetailBLogicalVolume()
{
  G4Box* box = nullptr;
  G4SubtractionSolid* ss1, *solidDetailB;
  double lx, ly;
  const EKLMGeometry::ShieldGeometry* shieldGeometry =
    m_GeoDat->getShieldGeometry();
  const EKLMGeometry::ShieldDetailGeometry* detailB =
    shieldGeometry->getDetailB();
  HepGeom::Transform3D t = HepGeom::Translate3D(0, 0, 0);
  lx = detailB->getLengthX() / 2;
  ly = detailB->getLengthY() / 2;
  try {
    box = new G4Box("ShieldDetailB_Box", lx, ly,
                    shieldGeometry->getThickness() / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  ss1 = cutSolidCorner("ShieldDetailB_Subtraction_1", box,
                       m_Solids.subtractionBox, t, false,
                       detailB->getPoint(0)->getX() - lx,
                       detailB->getPoint(0)->getY() - ly,
                       detailB->getPoint(1)->getX() - lx,
                       detailB->getPoint(1)->getY() - ly);
  solidDetailB = cutSolidCorner("ShieldDetailB", ss1,
                                m_Solids.subtractionBox, t, false,
                                detailB->getPoint(2)->getX() - lx,
                                detailB->getPoint(2)->getY() - ly,
                                detailB->getPoint(3)->getX() - lx,
                                detailB->getPoint(3)->getY() - ly);
  try {
    m_LogVol.shield.detailB =
      new G4LogicalVolume(solidDetailB, m_Materials.polyethylene,
                          "ShieldDetailB");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.shield.detailB, true);
  geometry::setColor(*m_LogVol.shield.detailB, "#00ff00ff");
}

void EKLM::GeoEKLMCreator::createShieldDetailCLogicalVolume()
{
  G4Box* box = nullptr;
  G4SubtractionSolid* ss1, *ss2, *ss3, *solidDetailC;
  double lx, ly;
  const EKLMGeometry::ShieldGeometry* shieldGeometry =
    m_GeoDat->getShieldGeometry();
  const EKLMGeometry::ShieldDetailGeometry* detailC =
    shieldGeometry->getDetailC();
  HepGeom::Transform3D t = HepGeom::Translate3D(0, 0, 0);
  lx = detailC->getLengthX() / 2;
  ly = detailC->getLengthY() / 2;
  try {
    box = new G4Box("ShieldDetailC_Box", lx, ly,
                    shieldGeometry->getThickness() / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  ss1 = cutSolidCorner("ShieldDetailC_Subtraction_1", box,
                       m_Solids.subtractionBox, t, false,
                       detailC->getPoint(0)->getX() - lx,
                       detailC->getPoint(0)->getY() - ly,
                       detailC->getPoint(1)->getX() - lx,
                       detailC->getPoint(1)->getY() - ly);
  ss2 = cutSolidCorner("ShieldDetailC_Subtraction_2", ss1,
                       m_Solids.subtractionBox, t, true,
                       detailC->getPoint(3)->getX() - lx,
                       detailC->getPoint(3)->getY() - ly,
                       detailC->getPoint(2)->getX() - lx,
                       detailC->getPoint(2)->getY() - ly);
  ss3 = cutSolidCorner("ShieldDetailC_Subtraction_3", ss2,
                       m_Solids.subtractionBox, t, false,
                       detailC->getPoint(3)->getX() - lx,
                       detailC->getPoint(3)->getY() - ly,
                       detailC->getPoint(4)->getX() - lx,
                       detailC->getPoint(4)->getY() - ly);
  solidDetailC = cutSolidCorner("ShieldDetailC", ss3,
                                m_Solids.subtractionBox, t, false,
                                detailC->getPoint(5)->getX() - lx,
                                detailC->getPoint(5)->getY() - ly,
                                detailC->getPoint(6)->getX() - lx,
                                detailC->getPoint(6)->getY() - ly);
  try {
    m_LogVol.shield.detailC =
      new G4LogicalVolume(solidDetailC, m_Materials.polyethylene,
                          "ShieldDetailC");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.shield.detailC, true);
  geometry::setColor(*m_LogVol.shield.detailC, "#00ff00ff");
}

void EKLM::GeoEKLMCreator::createShieldDetailDLogicalVolume()
{
  G4TriangularPrism* solidDetailDPrism;
  const EKLMGeometry::ShieldGeometry* shieldGeometry =
    m_GeoDat->getShieldGeometry();
  const EKLMGeometry::ShieldDetailGeometry* detailD =
    shieldGeometry->getDetailD();
  try {
    solidDetailDPrism =
      new G4TriangularPrism("ShieldDetailD_Prism",
                            detailD->getLengthX(), 0.,
                            detailD->getLengthY(), 90. * CLHEP::deg,
                            shieldGeometry->getThickness() / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.shield.detailD =
      new G4LogicalVolume(solidDetailDPrism->getSolid(),
                          m_Materials.polyethylene, "ShieldDetailD");
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.shield.detailD, true);
  geometry::setColor(*m_LogVol.shield.detailD, "#00ff00ff");
  delete solidDetailDPrism;
}

void EKLM::GeoEKLMCreator::createSolids()
{
  int i, j, n;
  HepGeom::Transform3D t;
  /* Section, layer, sector. */
  createSectionSolid();
  createLayerSolid();
  m_LogVol.shieldLayer = createLayerLogicalVolume("ShieldLayer");
  createSectorSolid();
  m_LogVol.shieldLayerSector = createSectorLogicalVolume("ShieldLayerSector");
  createSectorCoverLogicalVolume();
  createSectorSupportLogicalVolume();
  /**
   * createSectorSupportCornerXSolid() must be called after
   * createSectorSupportSolid()
   */
  createSectorSupportCorner1LogicalVolume();
  createSectorSupportCorner2LogicalVolume();
  createSectorSupportCorner3LogicalVolume();
  createSectorSupportCorner4LogicalVolume();
  /* Plane. */
  createSubtractionBoxSolid();
  for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
    createPlaneSolid(i);
    /* Segment support. */
    for (j = 1; j <= m_GeoDat->getNSegments() + 1; j++)
      createSegmentSupportLogicalVolume(i + 1, j);
  }
  /* Strips. */
  n = m_GeoDat->getNStripsDifferentLength();
  for (i = 0; i < n; i++) {
    createStripLogicalVolume(i);
    createStripGrooveLogicalVolume(i);
    createScintillatorLogicalVolume(i);
  }
  /* Plastic sheet elements. */
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    createPlasticSheetLogicalVolume(i);
  /* Strip segments. */
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    createStripSegmentLogicalVolume(i);
  /* Segments. */
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    createSegmentLogicalVolume(i);
  /* Shield layer details. */
  createShieldDetailALogicalVolume();
  createShieldDetailBLogicalVolume();
  createShieldDetailCLogicalVolume();
  createShieldDetailDLogicalVolume();
}

/************************** CREATION OF VOLUMES ******************************/

G4LogicalVolume*
EKLM::GeoEKLMCreator::createSection(G4LogicalVolume* topVolume) const
{
  G4LogicalVolume* logicSection = nullptr;
  const HepGeom::Transform3D* t;
  std::string sectionName = "Section_" + std::to_string(m_CurVol.section);
  try {
    logicSection = new G4LogicalVolume(m_Solids.section, m_Materials.iron,
                                       sectionName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSection, true);
  geometry::setColor(*logicSection, "#ffffff22");
  t = m_TransformData->getSectionTransform(m_CurVol.section);
  try {
    new G4PVPlacement(*t, logicSection, sectionName, topVolume, false,
                      m_CurVol.section, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicSection;
}

G4LogicalVolume* EKLM::GeoEKLMCreator::
createLayer(G4LogicalVolume* section, G4LogicalVolume* layer) const
{
  G4LogicalVolume* logicLayer;
  const HepGeom::Transform3D* t;
  if (layer == nullptr) {
    std::string layerName = "Layer_" + std::to_string(m_CurVol.layer) +
                            "_" + section->GetName();
    logicLayer = createLayerLogicalVolume(layerName.c_str());
  } else
    logicLayer = layer;
  t = m_TransformData->getLayerTransform(m_CurVol.section, m_CurVol.layer);
  try {
    new G4PVPlacement(*t, logicLayer, logicLayer->GetName(), section, false,
                      m_CurVol.layer, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicLayer;
}

G4LogicalVolume* EKLM::GeoEKLMCreator::
createSector(G4LogicalVolume* layer, G4LogicalVolume* sector) const
{
  G4LogicalVolume* logicSector;
  const HepGeom::Transform3D* t;
  if (sector == nullptr) {
    std::string sectorName = "Sector_" + std::to_string(m_CurVol.sector) +
                             "_" + layer->GetName();
    logicSector = createSectorLogicalVolume(sectorName.c_str());
  } else
    logicSector = sector;
  t = m_TransformData->getSectorTransform(m_CurVol.section, m_CurVol.layer,
                                          m_CurVol.sector);
  try {
    new G4PVPlacement(*t, logicSector, logicSector->GetName(), layer, false,
                      m_CurVol.sector, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicSector;
}

void EKLM::GeoEKLMCreator::
createSectorCover(int iCover, G4LogicalVolume* sector) const
{
  double z;
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.cover;
  const EKLMGeometry::ElementPosition* sectorPos =
    m_GeoDat->getSectorPosition();
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  z = 0.25 * (sectorPos->getLength() + sectorSupportPos->getLength());
  if (iCover == 2)
    z = -z;
  t = HepGeom::Translate3D(0., 0., z);
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + sector->GetName(), sector,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createSectorSupportCorner1(G4LogicalVolume* sector) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.sectorsup.corn1;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(0., 0., sectorSupportGeometry->getCorner1Z());
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + sector->GetName(), sector,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createSectorSupportCorner2(G4LogicalVolume* sector) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.sectorsup.corn2;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(sectorSupportGeometry->getCorner2Inner().x(),
                           sectorSupportGeometry->getCorner2Inner().y(),
                           sectorSupportGeometry->getCorner2Z());
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + sector->GetName(), sector,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createSectorSupportCorner3(G4LogicalVolume* sector) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.sectorsup.corn3;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(sectorSupportGeometry->getCorner3Prism().x(),
                           sectorSupportGeometry->getCorner3Prism().y(),
                           sectorSupportGeometry->getCorner3Z());
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + sector->GetName(), sector,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createSectorSupportCorner4(G4LogicalVolume* sector) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.sectorsup.corn4;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(sectorSupportGeometry->getCorner4Prism().x(),
                           sectorSupportGeometry->getCorner4Prism().y(),
                           sectorSupportGeometry->getCorner4Z());
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + sector->GetName(), sector,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createSectorSupport(G4LogicalVolume* sector) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.sectorsup.supp;
  const EKLMGeometry::ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  t = HepGeom::Translate3D(0., 0., sectorSupportPos->getZ());
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + sector->GetName(), sector,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4LogicalVolume*
EKLM::GeoEKLMCreator::createPlane(G4LogicalVolume* sector) const
{
  G4LogicalVolume* logicPlane = nullptr;
  const HepGeom::Transform3D* t;
  std::string planeName =
    "Plane_" + std::to_string(m_CurVol.plane) + "_" + sector->GetName();
  try {
    logicPlane = new G4LogicalVolume(m_Solids.plane[m_CurVol.plane - 1],
                                     m_Materials.air, planeName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicPlane, false);
  t = m_TransformData->getPlaneTransform(m_CurVol.section, m_CurVol.layer,
                                         m_CurVol.sector, m_CurVol.plane);
  try {
    new G4PVPlacement(*t, logicPlane, planeName, sector, false,
                      m_CurVol.plane, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicPlane;
}

void EKLM::GeoEKLMCreator::
createSegmentSupport(int iSegmentSupport, G4LogicalVolume* plane) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv =
    m_LogVol.segmentsup[m_CurVol.plane - 1][iSegmentSupport - 1];
  const EKLMGeometry::SegmentSupportPosition* segmentSupportPos =
    m_GeoDat->getSegmentSupportPosition(m_CurVol.plane, iSegmentSupport);
  t = (*m_TransformData->getPlaneDisplacement(m_CurVol.section, m_CurVol.layer,
                                              m_CurVol.sector, m_CurVol.plane)) *
      HepGeom::Translate3D(
        0.5 * (segmentSupportPos->getDeltaLLeft() -
               segmentSupportPos->getDeltaLRight()) +
        segmentSupportPos->getX(), segmentSupportPos->getY(),
        segmentSupportPos->getZ()) *
      HepGeom::RotateX3D(180.0 * CLHEP::deg);
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + plane->GetName(), plane,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createPlasticSheet(int iSheetPlane, int iSheet) const
{
  double z;
  HepGeom::Transform3D t;
  const EKLMGeometry::PlasticSheetGeometry* plasticSheetGeometry =
    m_GeoDat->getPlasticSheetGeometry();
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  std::string sheetName =
    "Sheet_" + std::to_string(iSheet) +
    "_SheetPlane_" + std::to_string(iSheetPlane);
  z = 0.5 * (stripGeometry->getThickness() + plasticSheetGeometry->getWidth());
  if (iSheetPlane == 2)
    z = -z;
  t = HepGeom::Translate3D(0, 0, z);
  try {
    new G4PVPlacement(t, m_LogVol.psheet[iSheet - 1], sheetName,
                      m_LogVol.segment[iSheet - 1], false, (iSheetPlane - 1) *
                      m_GeoDat->getNSegments() + iSheet, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStripSegment(int iSegment) const
{
  HepGeom::Transform3D t;
  std::string segmentName = "StripSegment_" + std::to_string(iSegment);
  t = HepGeom::Translate3D(0, 0, 0);
  try {
    new G4PVPlacement(t, m_LogVol.stripSegment[iSegment - 1], segmentName,
                      m_LogVol.segment[iSegment - 1], false, iSegment, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSegment(G4LogicalVolume* plane) const
{
  HepGeom::Transform3D t;
  std::string segmentName =
    "Segment_" + std::to_string(m_CurVol.segment) + "_" + plane->GetName();
  t = (*m_TransformData->getPlaneDisplacement(m_CurVol.section, m_CurVol.layer,
                                              m_CurVol.sector, m_CurVol.plane)) *
      (*m_TransformData->getSegmentTransform(
         m_CurVol.section, m_CurVol.layer, m_CurVol.sector, m_CurVol.plane,
         m_CurVol.segment));
  try {
    new G4PVPlacement(t, m_LogVol.segment[m_CurVol.segment - 1], segmentName,
                      plane, false, m_CurVol.segment, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStrip(G4LogicalVolume* segment) const
{
  int n;
  HepGeom::Transform3D t, t2;
  G4LogicalVolume* lv;
  n = m_GeoDat->getStripLengthIndex(m_CurVol.strip - 1);
  m_GeoDat->getStripTransform(&t, m_CurVol.strip - 1);
  t2 = t * HepGeom::RotateX3D(180.0 * CLHEP::deg);
  lv = m_LogVol.strip[n];
  try {
    new G4PVPlacement(t2, lv, lv->GetName(), segment, false, m_CurVol.strip,
                      false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStripGroove(int iStrip) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv;
  G4LogicalVolume* lvm;
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  t = HepGeom::Translate3D(0., 0., 0.5 * (stripGeometry->getThickness() -
                                          stripGeometry->getGrooveDepth()));
  lvm = m_LogVol.strip[iStrip];
  lv = m_LogVol.groove[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createScintillator(int iStrip) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv;
  G4LogicalVolume* lvm;
  t = HepGeom::Translate3D(0., 0., 0.);
  lvm = m_LogVol.strip[iStrip];
  lv = m_LogVol.scint[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createShield(G4LogicalVolume* sector) const
{
  HepGeom::Transform3D ta, tb, tc, td, te;
  G4LogicalVolume* lv;
  double lx, ly;
  const double asqrt2 = 1.0 / sqrt(2.0);
  const EKLMGeometry::ShieldGeometry* shieldGeometry =
    m_GeoDat->getShieldGeometry();
  lx = shieldGeometry->getDetailB()->getLengthX() / 2;
  ly = shieldGeometry->getDetailB()->getLengthY() / 2;
  ta = HepGeom::Translate3D(shieldGeometry->getDetailACenter()->getX(),
                            shieldGeometry->getDetailACenter()->getY(), 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg);
  tb = HepGeom::Translate3D(shieldGeometry->getDetailBCenter()->getX(),
                            shieldGeometry->getDetailBCenter()->getY(), 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg);
  tc = HepGeom::Translate3D(shieldGeometry->getDetailCCenter()->getX(),
                            shieldGeometry->getDetailCCenter()->getY(), 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
       HepGeom::RotateY3D(180.0 * CLHEP::deg);
  td = HepGeom::Translate3D(
         shieldGeometry->getDetailBCenter()->getX() + asqrt2 * (-lx - ly),
         shieldGeometry->getDetailBCenter()->getY() + asqrt2 * (lx - ly), 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
       HepGeom::RotateX3D(180.0 * CLHEP::deg);
  te = HepGeom::Translate3D(
         shieldGeometry->getDetailBCenter()->getX() + asqrt2 * (lx - ly),
         shieldGeometry->getDetailBCenter()->getY() + asqrt2 * (-lx - ly), 0) *
       HepGeom::RotateZ3D(135.0 * CLHEP::deg);
  lv = m_LogVol.shield.detailA;
  try {
    new G4PVPlacement(ta, lv, lv->GetName(), sector, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  lv = m_LogVol.shield.detailB;
  try {
    new G4PVPlacement(tb, lv, lv->GetName(), sector, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  lv = m_LogVol.shield.detailC;
  try {
    new G4PVPlacement(tc, lv, lv->GetName(), sector, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  lv = m_LogVol.shield.detailD;
  try {
    new G4PVPlacement(td, lv, lv->GetName(), sector, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    new G4PVPlacement(te, lv, "ShieldDetailE", sector, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

bool EKLM::GeoEKLMCreator::detectorLayer(int section, int layer) const
{
  return ((section == 1 && layer <= m_GeoDat->getNDetectorLayers(1)) ||
          (section == 2 && layer <= m_GeoDat->getNDetectorLayers(2)));
}

void EKLM::GeoEKLMCreator::create(G4LogicalVolume& topVolume)
{
  /* cppcheck-suppress variableScope */
  int i, j, imin, imax;
  /* cppcheck-suppress variableScope */
  G4LogicalVolume* section, *layer, *sector, *plane;
  createMaterials();
  createSolids();
  /* Create physical volumes which are used only once. */
  for (i = 0; i < m_GeoDat->getNStripsDifferentLength(); i++) {
    createStripGroove(i);
    createScintillator(i);
  }
  for (i = 0; i < m_GeoDat->getNSegments(); i++) {
    imin = i * m_ElementNumbers->getNStripsSegment();
    imax = (i + 1) * m_ElementNumbers->getNStripsSegment();
    for (m_CurVol.strip = imin + 1; m_CurVol.strip <= imax; m_CurVol.strip++)
      createStrip(m_LogVol.stripSegment[i]);
  }
  for (i = 1; i <= m_GeoDat->getNSegments(); i++) {
    for (j = 1; j <= 2; j++)
      createPlasticSheet(j, i);
    createStripSegment(i);
  }
  /* Create other volumes. */
  /* Set up region for production cuts. */
  G4Region* aRegion = new G4Region("EKLMEnvelope");
  for (m_CurVol.section = 1; m_CurVol.section <= m_GeoDat->getNSections();
       m_CurVol.section++) {
    section = createSection(&topVolume);
    /* Assign same region to each section. */
    section->SetRegion(aRegion);
    aRegion->AddRootLogicalVolume(section);
    for (m_CurVol.layer = 1; m_CurVol.layer <= m_GeoDat->getNLayers();
         m_CurVol.layer++) {
      if (detectorLayer(m_CurVol.section, m_CurVol.layer)) {
        /* Detector layer. */
        layer = createLayer(section, nullptr);
        for (m_CurVol.sector = 1; m_CurVol.sector <= m_GeoDat->getNSectors();
             m_CurVol.sector++) {
          sector = createSector(layer, nullptr);
          createSectorSupport(sector);
          createSectorSupportCorner1(sector);
          createSectorSupportCorner2(sector);
          createSectorSupportCorner3(sector);
          createSectorSupportCorner4(sector);
          for (i = 1; i <= 2; i++)
            createSectorCover(i, sector);
          for (m_CurVol.plane = 1; m_CurVol.plane <= m_GeoDat->getNPlanes();
               m_CurVol.plane++) {
            plane = createPlane(sector);
            for (i = 1; i <= m_GeoDat->getNSegments() + 1; i++)
              createSegmentSupport(i, plane);
            for (m_CurVol.segment = 1;
                 m_CurVol.segment <= m_GeoDat->getNSegments();
                 m_CurVol.segment++)
              createSegment(plane);
          }
        }
      } else {
        /* Shield layer. */
        layer = createLayer(section, m_LogVol.shieldLayer);
        for (m_CurVol.sector = 1; m_CurVol.sector <= m_GeoDat->getNSectors();
             m_CurVol.sector++)
          createSector(layer, m_LogVol.shieldLayerSector);
        createSectorSupport(m_LogVol.shieldLayerSector);
        createSectorSupportCorner1(m_LogVol.shieldLayerSector);
        createSectorSupportCorner2(m_LogVol.shieldLayerSector);
        createSectorSupportCorner3(m_LogVol.shieldLayerSector);
        createSectorSupportCorner4(m_LogVol.shieldLayerSector);
        for (i = 1; i <= 2; i++)
          createSectorCover(i, m_LogVol.shieldLayerSector);
        createShield(m_LogVol.shieldLayerSector);
      }
    }
  }
}

void EKLM::GeoEKLMCreator::create(const GearDir& content,
                                  G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes type)
{
  (void)content;
  (void)type;
  m_GeoDat = &(EKLM::GeometryData::Instance(GeometryData::c_Gearbox, &content));
  try {
    m_TransformData =
      new EKLM::TransformData(false, EKLM::TransformData::c_Displacement);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  newVolumes();
  newSensitive();
  create(topVolume);
}

void EKLM::GeoEKLMCreator::createFromDB(const std::string& name,
                                        G4LogicalVolume& topVolume,
                                        geometry::GeometryTypes type)
{
  (void)name;
  (void)type;
  m_GeoDat = &(EKLM::GeometryData::Instance(GeometryData::c_Database));
  try {
    m_TransformData =
      new EKLM::TransformData(false, EKLM::TransformData::c_Displacement);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  newVolumes();
  newSensitive();
  create(topVolume);
}

void EKLM::GeoEKLMCreator::createPayloads(const GearDir& content,
                                          const IntervalOfValidity& iov)
{
  (void)content;
  m_GeoDat = &(EKLM::GeometryData::Instance(GeometryData::c_Gearbox, &content));
  try {
    m_TransformData =
      new EKLM::TransformData(false, EKLM::TransformData::c_Displacement);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  m_GeoDat->saveToDatabase(iov);
}

