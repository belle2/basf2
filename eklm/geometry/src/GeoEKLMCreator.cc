/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin,         *
 *               Takanori Hara                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <errno.h>
#include <math.h>

/* External headers. */
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <CLHEP/Units/PhysicalConstants.h>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4ReflectedSolid.hh>

/* Belle2 headers. */
#include <eklm/geometry/G4TriangularPrism.h>
#include <eklm/geometry/GeoEKLMCreator.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

/* Register the creator */
geometry::CreatorFactory<EKLM::GeoEKLMCreator> GeoEKLMFactory("EKLMCreator");

/******************************* CONSTRUCTORS ********************************/

EKLM::GeoEKLMCreator::GeoEKLMCreator()
{
  m_Materials.air = NULL;
  m_CurVol.endcap = 1;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  if (readESTRData(&m_ESTRPar) == ENOMEM)
    B2FATAL(MemErr);
  try {
    m_TransformData = new EKLM::TransformData(false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  newVolumes();
  newTransforms();
  newSensitive();
}

EKLM::GeoEKLMCreator::~GeoEKLMCreator()
{
  delete m_TransformData;
  free(m_ESTRPar.z);
  free(m_ESTRPar.rmin);
  free(m_ESTRPar.rmax);
  deleteVolumes();
  deleteTransforms();
  deleteSensitive();
}

/***************************** MEMORY ALLOCATION *****************************/

void EKLM::GeoEKLMCreator::newVolumes()
{
  int i, nDiff;
  m_Solids.plane =
    (G4VSolid**)malloc(m_GeoDat->getNPlanes() * sizeof(G4VSolid*));
  if (m_Solids.plane == NULL)
    B2FATAL(MemErr);
  m_Solids.psheet =
    (G4VSolid**)malloc(m_GeoDat->getNSegments() * sizeof(G4VSolid*));
  if (m_Solids.psheet == NULL)
    B2FATAL(MemErr);
  m_LogVol.psheet = (G4LogicalVolume**)
                    malloc(m_GeoDat->getNSegments() * sizeof(G4LogicalVolume*));
  if (m_LogVol.psheet == NULL)
    B2FATAL(MemErr);
  nDiff = m_GeoDat->getNStripsDifferentLength();
  m_LogVol.stripvol = (G4LogicalVolume**)
                      malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.stripvol == NULL)
    B2FATAL(MemErr);
  m_LogVol.strip = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.strip == NULL)
    B2FATAL(MemErr);
  m_Solids.groove = (G4VSolid**)malloc(nDiff * sizeof(G4Box*));
  if (m_Solids.groove == NULL)
    B2FATAL(MemErr);
  m_LogVol.groove = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.groove == NULL)
    B2FATAL(MemErr);
  m_LogVol.scint = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (m_LogVol.scint == NULL)
    B2FATAL(MemErr);
  m_LogVol.segmentsup =
    (G4LogicalVolume***)malloc(m_GeoDat->getNPlanes() *
                               sizeof(G4LogicalVolume**));
  if (m_LogVol.segmentsup == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
    m_LogVol.segmentsup[i] =
      (G4LogicalVolume**)malloc((m_GeoDat->getNSegments() + 1) *
                                sizeof(G4LogicalVolume*));
    if (m_LogVol.segmentsup[i] == NULL)
      B2FATAL(MemErr);
  }
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    m_LogVol.psheet[i] = NULL;
}

void EKLM::GeoEKLMCreator::newTransforms()
{
  int i;
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
      try {
        m_BoardTransform[i] = new HepGeom::Transform3D[m_GeoDat->getNBoards()];
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
    }
  }
}

void EKLM::GeoEKLMCreator::newSensitive()
{
  try {
    m_Sensitive[0] =
      new EKLMSensitiveDetector("EKLMSensitiveStrip",
                                c_SensitiveStrip);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    try {
      m_Sensitive[1] =
        new EKLMSensitiveDetector("EKLMSensitiveSiPM",
                                  c_SensitiveSiPM);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      m_Sensitive[2] =
        new EKLMSensitiveDetector("EKLMSensitiveBoard",
                                  c_SensitiveBoard);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
}

void EKLM::GeoEKLMCreator::deleteVolumes()
{
  int i;
  free(m_Solids.plane);
  free(m_Solids.psheet);
  free(m_LogVol.psheet);
  free(m_LogVol.stripvol);
  free(m_LogVol.strip);
  free(m_Solids.groove);
  free(m_LogVol.groove);
  free(m_LogVol.scint);
  for (i = 0; i < m_GeoDat->getNPlanes(); i++)
    free(m_LogVol.segmentsup[i]);
  free(m_LogVol.segmentsup);
}

void EKLM::GeoEKLMCreator::deleteTransforms()
{
  int i;
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    for (i = 0; i < m_GeoDat->getNPlanes(); i++)
      delete[] m_BoardTransform[i];
  }
}

void EKLM::GeoEKLMCreator::deleteSensitive()
{
  delete m_Sensitive[0];
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    delete m_Sensitive[1];
    delete m_Sensitive[2];
  }
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

void EKLM::GeoEKLMCreator::createEndcapSolid()
{
  G4Polyhedra* op = NULL;
  G4Tubs* tb = NULL;
  try {
    op = new G4Polyhedra("Endcap_Octagonal_Prism", m_ESTRPar.phi,
                         m_ESTRPar.dphi, m_ESTRPar.nsides, m_ESTRPar.nboundary,
                         m_ESTRPar.z, m_ESTRPar.rmin, m_ESTRPar.rmax);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    tb = new G4Tubs("Endcap_Tube",  m_ESTRPar.rminsub, m_ESTRPar.rmaxsub,
                    m_ESTRPar.zsub, 0.0, 360.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_Solids.endcap = new G4SubtractionSolid("Endcap", op, tb);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4LogicalVolume*
EKLM::GeoEKLMCreator::createLayerLogicalVolume(const char* name) const
{
  G4LogicalVolume* logicLayer = NULL;
  try {
    logicLayer = new G4LogicalVolume(m_Solids.layer, m_Materials.air, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicLayer, false);
  return logicLayer;
}

void EKLM::GeoEKLMCreator::createLayerLogicalVolume()
{
  const struct ElementPosition* layerPos = m_GeoDat->getLayerPosition();
  try {
    m_Solids.layer = new G4Tubs("Layer", layerPos->InnerR, layerPos->OuterR,
                                layerPos->Length / 2.0, 0.0, 360. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  m_LogVol.shieldLayer = createLayerLogicalVolume("ShieldLayer");
}

G4LogicalVolume*
EKLM::GeoEKLMCreator::createSectorLogicalVolume(const char* name) const
{
  G4LogicalVolume* logicSector = NULL;
  try {
    logicSector = new G4LogicalVolume(m_Solids.sector, m_Materials.air, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSector, false);
  return logicSector;
}

void EKLM::GeoEKLMCreator::createSectorLogicalVolume()
{
  const struct ElementPosition* sectorPos = m_GeoDat->getSectorPosition();
  try {
    m_Solids.sector =
      new G4Tubs("Sector", sectorPos->InnerR, sectorPos->OuterR,
                 0.5 * sectorPos->Length, 0.0, 90. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  m_LogVol.shieldLayerSector = createSectorLogicalVolume("ShieldLayerSector");
}

void EKLM::GeoEKLMCreator::createSectorCoverLogicalVolume()
{
  double lz;
  HepGeom::Transform3D t1;
  HepGeom::Transform3D t2;
  G4Tubs* solidCoverTube = NULL;
  G4Box* solidCoverBox = NULL;
  G4Box* box = NULL;
  G4IntersectionSolid* is = NULL;
  G4SubtractionSolid* solidCover = NULL;
  const struct ElementPosition* sectorPos = m_GeoDat->getSectorPosition();
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const struct ElementPosition* planePos = m_GeoDat->getPlanePosition();
  lz = 0.5 * (sectorPos->Length - sectorSupportPos->Length);
  try {
    solidCoverTube = new G4Tubs("Cover_Tube", sectorSupportPos->InnerR,
                                sectorSupportPos->OuterR, 0.5 * lz, 0.0,
                                90.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCoverBox = new G4Box("Cover_Box", 0.5 * sectorSupportPos->OuterR,
                              0.5 * sectorSupportPos->OuterR, 0.5 * lz);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    box = new G4Box("Cover_SubtractionBox", 0.5 * sectorSupportPos->OuterR,
                    0.5 * sectorSupportPos->OuterR, lz);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(0.5 * planePos->OuterR + sectorSupportPos->X,
                            0.5 * planePos->OuterR + sectorSupportPos->Y, 0.);
  t2 = HepGeom::Translate3D(sectorSupportPos->X +
                            0.5 * sectorSupportPos->OuterR *
                            cos(sectorSupportGeometry->CornerAngle) -
                            0.5 * sectorSupportPos->OuterR *
                            sin(sectorSupportGeometry->CornerAngle),
                            sectorSupportPos->OuterR -
                            sectorSupportGeometry->DeltaLY +
                            0.5 * sectorSupportPos->OuterR *
                            cos(sectorSupportGeometry->CornerAngle) +
                            0.5 * sectorSupportPos->OuterR *
                            sin(sectorSupportGeometry->CornerAngle),
                            0.) * HepGeom::RotateZ3D(sectorSupportGeometry->CornerAngle);
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
  G4Box* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  x = sqrt(sectorSupportPos->OuterR * sectorSupportPos->OuterR -
           sectorSupportPos->Y * sectorSupportPos->Y);
  t = HepGeom::Translate3D(0.5 * (x + sectorSupportGeometry->Corner3.X),
                           sectorSupportPos->Y +
                           0.5 * sectorSupportGeometry->Thickness, 0.);
  try {
    res = new G4Box("SectorSupport_BoxX",
                    0.5 * (x - sectorSupportGeometry->Corner3.X),
                    0.5 * sectorSupportGeometry->Thickness,
                    0.5 * sectorSupportPos->Length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxY(HepGeom::Transform3D& t)
{
  G4Box* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(sectorSupportPos->X +
                           0.5 * sectorSupportGeometry->Thickness,
                           0.5 * (sectorSupportGeometry->Corner4.Y +
                                  sectorSupportGeometry->Corner1A.Y), 0.) *
      HepGeom::RotateZ3D(90. * CLHEP::deg);
  try {
    res = new G4Box("SectorSupport_BoxY",
                    0.5 * (sectorSupportGeometry->Corner1A.Y -
                           sectorSupportGeometry->Corner4.Y),
                    0.5 * sectorSupportGeometry->Thickness,
                    0.5 * sectorSupportPos->Length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxTop(HepGeom::Transform3D& t)
{
  double dx;
  double dy;
  G4Box* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(0.5 * (sectorSupportGeometry->Corner1A.X +
                                  sectorSupportGeometry->Corner1B.X +
                                  sectorSupportGeometry->Thickness *
                                  sin(sectorSupportGeometry->CornerAngle)),
                           0.5 * (sectorSupportGeometry->Corner1A.Y +
                                  sectorSupportGeometry->Corner1B.Y -
                                  sectorSupportGeometry->Thickness *
                                  cos(sectorSupportGeometry->CornerAngle)),
                           0.) * HepGeom::RotateZ3D(sectorSupportGeometry->CornerAngle);
  dx = sectorSupportGeometry->Corner1B.X - sectorSupportGeometry->Corner1A.X;
  dy = sectorSupportGeometry->Corner1B.Y - sectorSupportGeometry->Corner1B.Y;
  try {
    res = new G4Box("SectorSupport_BoxTop", 0.5 * sqrt(dx * dx + dy * dy),
                    0.5 * sectorSupportGeometry->Thickness,
                    0.5 * sectorSupportPos->Length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Tubs* EKLM::GeoEKLMCreator::createSectorSupportInnerTube()
{
  double ang1;
  double ang2;
  G4Tubs* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  ang1 = atan2(sectorSupportGeometry->Corner3.Y,
               sectorSupportGeometry->Corner3.X);
  ang2 = atan2(sectorSupportGeometry->Corner4.Y,
               sectorSupportGeometry->Corner4.X);
  try {
    res = new G4Tubs("SectorSupport_InnerTube", sectorSupportPos->InnerR,
                     sectorSupportPos->InnerR +
                     sectorSupportGeometry->Thickness,
                     0.5 * sectorSupportPos->Length,
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
  G4Tubs* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->OuterR - sectorSupportGeometry->Thickness;
  x = sqrt(r * r - sectorSupportPos->Y * sectorSupportPos->Y);
  ang1 = atan2(sectorSupportPos->Y, x);
  ang2 = atan2(sectorSupportGeometry->Corner1B.Y,
               sectorSupportGeometry->Corner1B.X);
  try {
    res = new G4Tubs("SectorSupport_OuterTube", r,
                     sectorSupportPos->OuterR,
                     0.5 * sectorSupportPos->Length,
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
  G4Tubs* solidCorner1Tube = NULL;
  G4Box* solidCorner1Box1 = NULL;
  G4Box* solidCorner1Box2 = NULL;
  G4IntersectionSolid* is1 = NULL;
  G4IntersectionSolid* solidCorner1 = NULL;
  HepGeom::Transform3D t1;
  HepGeom::Transform3D t2;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  lx = sectorSupportGeometry->CornerX + sectorSupportGeometry->Corner1LX -
       sectorSupportGeometry->Thickness;
  try {
    solidCorner1Tube = new G4Tubs("SectorSupport_Corner1_Tube", 0.,
                                  sectorSupportPos->OuterR -
                                  sectorSupportGeometry->Thickness,
                                  0.5 * sectorSupportGeometry->Corner1Thickness,
                                  0., 90. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1Box1 = new G4Box("SectorSupport_Corner1_Box1", 0.5 * lx,
                                 0.5 * sectorSupportPos->OuterR,
                                 0.5 * sectorSupportGeometry->Corner1Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1Box2 =
      new G4Box("SectorSupport_Corner1_Box2",
                0.5 * (lx / cos(sectorSupportGeometry->CornerAngle) +
                       sectorSupportGeometry->Corner1Width *
                       tan(sectorSupportGeometry->CornerAngle)),
                0.5 * sectorSupportGeometry->Corner1Width,
                0.5 * sectorSupportGeometry->Corner1Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  x = sectorSupportPos->X + 0.5 * (sectorSupportGeometry->CornerX +
                                   sectorSupportGeometry->Corner1LX +
                                   sectorSupportGeometry->Thickness);
  t1 = HepGeom::Translate3D(x, 0.5 * sectorSupportPos->OuterR, 0.);
  t2 = HepGeom::Translate3D(x, sectorSupportGeometry->Corner1AInner.Y -
                            0.5 * sectorSupportGeometry->Corner1Width /
                            cos(sectorSupportGeometry->CornerAngle) +
                            0.5 * lx * tan(sectorSupportGeometry->CornerAngle), 0.) *
       HepGeom::RotateZ3D(sectorSupportGeometry->CornerAngle);
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
  double r;
  G4TriangularPrism* solidCorner2Prism = NULL;
  G4SubtractionSolid* solidCorner2 = NULL;
  HepGeom::Transform3D t1;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->OuterR - sectorSupportGeometry->Thickness;
  m_Solids.sectorsup.c2y = sectorSupportPos->Y +
                           sectorSupportGeometry->Thickness;
  m_Solids.sectorsup.c2x =
    sqrt(r * r - m_Solids.sectorsup.c2y * m_Solids.sectorsup.c2y);
  try {
    solidCorner2Prism = new G4TriangularPrism("SectorSupport_Corner2_Prism",
                                              sectorSupportGeometry->Corner2LY,
                                              90. * CLHEP::deg,
                                              sectorSupportGeometry->Corner2LX,
                                              180. * CLHEP::deg,
                                              0.5 * sectorSupportGeometry->
                                              Corner2Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(-m_Solids.sectorsup.c2x, -m_Solids.sectorsup.c2y, 0.);
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
  double r;
  double y;
  G4TriangularPrism* solidCorner3Prism = NULL;
  G4SubtractionSolid* solidCorner3 = NULL;
  HepGeom::Transform3D t1;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->InnerR + sectorSupportGeometry->Thickness;
  y = sectorSupportPos->Y + sectorSupportGeometry->Thickness +
      sectorSupportGeometry->Corner3LY;
  m_Solids.sectorsup.c3x = sqrt(r * r - y * y);
  m_Solids.sectorsup.c3y = sectorSupportPos->Y +
                           sectorSupportGeometry->Thickness;
  try {
    solidCorner3Prism =
      new G4TriangularPrism("SectorSupport_Corner3_Prism",
                            sectorSupportGeometry->Corner3LX, 0.,
                            sectorSupportGeometry->Corner3LY,
                            90. * CLHEP::deg,
                            0.5 * sectorSupportGeometry->Corner3Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(-m_Solids.sectorsup.c3x, -m_Solids.sectorsup.c3y, 0.);
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
  double r;
  double x;
  G4TriangularPrism* solidCorner4Prism = NULL;
  G4SubtractionSolid* solidCorner4 = NULL;
  HepGeom::Transform3D t1;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->InnerR + sectorSupportGeometry->Thickness;
  x = sectorSupportPos->X + sectorSupportGeometry->Thickness +
      sectorSupportGeometry->Corner4LX;
  m_Solids.sectorsup.c4y = sqrt(r * r - x * x);
  m_Solids.sectorsup.c4x = sectorSupportPos->X +
                           sectorSupportGeometry->Thickness;
  try {
    solidCorner4Prism =
      new G4TriangularPrism("SectorSupport_Corner4_Prism",
                            sectorSupportGeometry->Corner4LX, 0.,
                            sectorSupportGeometry->Corner4LY,
                            90. * CLHEP::deg,
                            0.5 * sectorSupportGeometry->Corner4Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(-m_Solids.sectorsup.c4x, -m_Solids.sectorsup.c4y, 0.);
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
  G4Tubs* solidLimitationTube = NULL;
  G4UnionSolid* us1 = NULL;
  G4UnionSolid* us2 = NULL;
  G4UnionSolid* us3 = NULL;
  G4UnionSolid* us4 = NULL;
  G4IntersectionSolid* solidSectorSupport = NULL;
  HepGeom::Transform3D tbx;
  HepGeom::Transform3D tby;
  HepGeom::Transform3D tbt;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  solidBoxX = createSectorSupportBoxX(tbx);
  solidBoxY = createSectorSupportBoxY(tby);
  solidBoxTop = createSectorSupportBoxTop(tbt);
  m_Solids.sectorsup.otube = createSectorSupportOuterTube();
  m_Solids.sectorsup.itube = createSectorSupportInnerTube();
  try {
    solidLimitationTube = new G4Tubs("SectorSupport_LimitationTube",
                                     0., sectorSupportPos->OuterR,
                                     0.5 * sectorSupportPos->Length,
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
                           m_Solids.sectorsup.otube, HepGeom::Translate3D(0., 0., 0.));
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
  const struct ElementPosition* layerPos = m_GeoDat->getLayerPosition();
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  try {
    m_Solids.subtractionBox =
      new G4Box("SubractionBox", 0.5 * sectorSupportPos->OuterR,
                0.5 * sectorSupportPos->OuterR, layerPos->Length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4SubtractionSolid* EKLM::GeoEKLMCreator::
cutSolidCorner(const char* name, G4VSolid* solid, G4Box* subtractionBox,
               HepGeom::Transform3D& transf, bool largerAngles,
               double x1, double y1, double x2, double y2)
{
  double ang;
  ang = atan2(y2 - y1, x2 - x1);
  return cutSolidCorner(name, solid, subtractionBox, transf, largerAngles,
                        x1, y1, ang);
}

G4SubtractionSolid* EKLM::GeoEKLMCreator::
cutSolidCorner(const char* name, G4VSolid* solid, G4Box* subtractionBox,
               HepGeom::Transform3D& transf, bool largerAngles,
               double x, double y, double ang)
{
  double lx, ly;
  HepGeom::Transform3D t;
  G4SubtractionSolid* ss = NULL;
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

G4SubtractionSolid* EKLM::GeoEKLMCreator::
subtractBoardSolids(G4SubtractionSolid* plane, int n)
{
  int i;
  int j;
  HepGeom::Transform3D t;
  G4Box* solidBoardBox = NULL;
  G4SubtractionSolid** ss[2];
  G4SubtractionSolid* prev_solid = NULL;
  G4SubtractionSolid* res = NULL;
  const struct ElementPosition* planePos = m_GeoDat->getPlanePosition();
  const struct BoardGeometry* boardGeometry;
  /* If there are no boards, it is not necessary to subtract their solids. */
  if (m_GeoDat->getDetectorMode() != c_DetectorBackground)
    return plane;
  boardGeometry = m_GeoDat->getBoardGeometry();
  /* Subtraction. */
  try {
    solidBoardBox = new G4Box("PlateBox", 0.5 * boardGeometry->Length,
                              0.5 * boardGeometry->Height,
                              0.5 * (planePos->Length + planePos->Z));
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < 2; i++) {
    ss[i] =
      (G4SubtractionSolid**)malloc(sizeof(G4SubtractionSolid*) *
                                   m_GeoDat->getNBoards());
    if (ss[i] == NULL)
      B2FATAL(MemErr);
    for (j = 0; j < m_GeoDat->getNBoards(); j++) {
      t = m_BoardTransform[i][j];
      if (n == 0)
        t = G4Rotate3D(180. * CLHEP::deg, G4ThreeVector(1., 1., 0.)) * t;
      if (i == 0) {
        if (j == 0)
          prev_solid = plane;
        else
          prev_solid = ss[0][j - 1];
      } else {
        if (j == 0)
          prev_solid = ss[0][m_GeoDat->getNBoards() - 1];
        else
          prev_solid = ss[1][j - 1];
      }
      try {
        ss[i][j] = new G4SubtractionSolid(
          "BoardSubtraction_" + boost::lexical_cast<std::string>(i) + "_" +
          boost::lexical_cast<std::string>(j),
          prev_solid, solidBoardBox, t);
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
    }
  }
  res = ss[1][m_GeoDat->getNBoards() - 1];
  for (i = 0; i < 2; i++)
    free(ss[i]);
  return res;
}

void EKLM::GeoEKLMCreator::createPlaneSolid(int n)
{
  double box_x;
  double box_y;
  HepGeom::Transform3D t;
  HepGeom::Transform3D t1;
  char name[128];
  G4Tubs* tb = NULL;
  G4IntersectionSolid* is = NULL;
  G4SubtractionSolid* ss1, *ss2, *ss3, *ss4;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const struct ElementPosition* planePos = m_GeoDat->getPlanePosition();
  /* Basic solids. */
  snprintf(name, 128, "Plane_%d_Tube", n + 1);
  try {
    tb = new G4Tubs(name, planePos->InnerR, planePos->OuterR,
                    0.5 * planePos->Length, 0.0, 90.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Box", n + 1);
  box_x = sectorSupportPos->X + sectorSupportGeometry->Thickness;
  box_y = sectorSupportPos->Y + sectorSupportGeometry->Thickness;
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
                       sectorSupportGeometry->Corner1AInner.X,
                       sectorSupportGeometry->Corner1AInner.Y,
                       sectorSupportGeometry->CornerAngle);
  snprintf(name, 128, "Plane_%d_Subtraction_2", n + 1);
  ss2 = cutSolidCorner(
          name, ss1, m_Solids.subtractionBox, t, false,
          m_Solids.sectorsup.c2x - sectorSupportGeometry->Corner2LX,
          m_Solids.sectorsup.c2y, m_Solids.sectorsup.c2x,
          m_Solids.sectorsup.c2y + sectorSupportGeometry->Corner2LY);
  snprintf(name, 128, "Plane_%d_Subtraction_3", n + 1);
  ss3 = cutSolidCorner(
          name, ss2, m_Solids.subtractionBox, t, false, m_Solids.sectorsup.c3x,
          m_Solids.sectorsup.c3y + sectorSupportGeometry->Corner3LY,
          m_Solids.sectorsup.c3x + sectorSupportGeometry->Corner3LX,
          m_Solids.sectorsup.c3y);
  snprintf(name, 128, "Plane_%d_Subtraction_4", n + 1);
  ss4 = cutSolidCorner(
          name, ss3, m_Solids.subtractionBox, t, true,
          m_Solids.sectorsup.c4x + sectorSupportGeometry->Corner4LX,
          m_Solids.sectorsup.c4y, m_Solids.sectorsup.c4x,
          m_Solids.sectorsup.c4y + sectorSupportGeometry->Corner4LY);
  snprintf(name, 128, "Plane_%d", n + 1);
  m_Solids.plane[n] = subtractBoardSolids(ss4, n);
}

void EKLM::GeoEKLMCreator::
createSegmentSupportLogicalVolume(int iPlane, int iSegmentSupport)
{
  HepGeom::Transform3D t1, t2;
  G4Box* topBox = NULL;
  G4Box* midBox = NULL;
  G4Box* botBox = NULL;
  G4UnionSolid* us = NULL;
  G4UnionSolid* solidSegmentSupport = NULL;
  std::string segmentSupportName =
    "SegmentSupport_" + boost::lexical_cast<std::string>(iSegmentSupport) +
    "Plane_" + boost::lexical_cast<std::string>(iPlane);
  const struct SegmentSupportPosition* segmentSupportPos =
    m_GeoDat->getSegmentSupportPosition(iPlane, iSegmentSupport);
  const struct SegmentSupportGeometry* segmentSupportGeometry =
    m_GeoDat->getSegmentSupportGeometry();
  try {
    topBox = new G4Box("BoxTop_" + segmentSupportName,
                       0.5 * (segmentSupportPos->Length -
                              segmentSupportPos->DeltaLLeft -
                              segmentSupportPos->DeltaLRight),
                       0.5 * segmentSupportGeometry->TopWidth,
                       0.5 * segmentSupportGeometry->TopThickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    midBox = new G4Box("BoxMiddle_" + segmentSupportName,
                       0.5 * (segmentSupportPos->Length -
                              segmentSupportPos->DeltaLLeft -
                              segmentSupportPos->DeltaLRight),
                       0.5 * segmentSupportGeometry->MiddleWidth,
                       0.5 * segmentSupportGeometry->MiddleThickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    botBox = new G4Box("BoxBottom_" + segmentSupportName,
                       0.5 * segmentSupportPos->Length,
                       0.5 * segmentSupportGeometry->TopWidth,
                       0.5 * segmentSupportGeometry->TopThickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = HepGeom::Translate3D(0., 0., 0.5 * (segmentSupportGeometry->MiddleThickness +
                                           segmentSupportGeometry->TopThickness));
  t2 = HepGeom::Translate3D(0.5 * (segmentSupportPos->DeltaLRight -
                                   segmentSupportPos->DeltaLLeft),
                            0., -0.5 * (segmentSupportGeometry->MiddleThickness +
                                        segmentSupportGeometry->TopThickness));
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

G4UnionSolid* EKLM::GeoEKLMCreator::
unifySolids(G4VSolid** solids, HepGeom::Transform3D* transf,
            int nSolids, std::string name)
{
  G4UnionSolid** u;
  G4UnionSolid* res;
  G4VSolid** solidArray;
  HepGeom::Transform3D* inverseTransf;
  HepGeom::Transform3D t;
  char str[128];
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
  if (u == NULL)
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
  res = u[nUnions - 1];
  free(u);
  delete[] inverseTransf;
  return res;
}

void EKLM::GeoEKLMCreator::createPlasticSheetSolid(int n)
{
  int i;
  int m;
  double ly;
  char name[128];
  G4Box* b[15];
  HepGeom::Transform3D t[15];
  const struct PlasticSheetGeometry* plasticSheetGeometry =
    m_GeoDat->getPlasticSheetGeometry();
  const struct ElementPosition* stripPos;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  /* Transformations. */
  for (i = 0; i < 15; i++) {
    m = 15 * n + i;
    m_GeoDat->getSheetTransform(&(t[i]), m);
  }
  /* Sheet elements. */
  for (i = 0; i < 15; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Element_%d", n + 1, i + 1);
    ly = stripGeometry->Width;
    if (i == 0 || i == 14)
      ly = ly - plasticSheetGeometry->DeltaL;
    m = 15 * n + i;
    stripPos = m_GeoDat->getStripPosition(m + 1);
    try {
      b[i] = new G4Box(name, 0.5 * stripPos->Length, 0.5 * ly,
                       0.5 * plasticSheetGeometry->Width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* Union. */
  snprintf(name, 128, "PlasticSheet_%d_", n + 1);
  m_Solids.psheet[n] = unifySolids((G4VSolid**)b, t, 15, name);
}

void EKLM::GeoEKLMCreator::createStripVolumeLogicalVolume(int iStrip)
{
  int iPos;
  char name[128];
  const struct ElementPosition* stripPos;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  G4Box* stripVolumeSolid;
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "StripVolume_%d", iStrip + 1);
  try {
    stripVolumeSolid =
      new G4Box(name, 0.5 * (stripPos->Length + stripGeometry->RSSSize),
                0.5 * stripGeometry->Width, 0.5 * stripGeometry->Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.stripvol[iStrip] =
      new G4LogicalVolume(stripVolumeSolid, m_Materials.air, name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.stripvol[iStrip], false);
}

void EKLM::GeoEKLMCreator::createStripLogicalVolume(int iStrip)
{
  int iPos;
  char name[128];
  const struct ElementPosition* stripPos;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  G4Box* stripSolid;
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "Strip_%d", iStrip + 1);
  try {
    stripSolid = new G4Box(name, 0.5 * stripPos->Length,
                           0.5 * stripGeometry->Width,
                           0.5 * stripGeometry->Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.strip[iStrip] =
      new G4LogicalVolume(stripSolid, m_Materials.polystyrene, name);
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
  const struct ElementPosition* stripPos;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "Groove_%d", iStrip + 1);
  try {
    m_Solids.groove[iStrip] = new G4Box(name, 0.5 * stripPos->Length,
                                        0.5 * stripGeometry->GrooveWidth,
                                        0.5 * stripGeometry->GrooveDepth);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.groove[iStrip] =
      new G4LogicalVolume(m_Solids.groove[iStrip], m_Materials.gel,
                          name);
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
  const struct ElementPosition* stripPos;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  HepGeom::Transform3D t;
  G4Box* b;
  G4SubtractionSolid* scintillatorSolid;
  iPos = m_GeoDat->getStripPositionIndex(iStrip);
  stripPos = m_GeoDat->getStripPosition(iPos + 1);
  snprintf(name, 128, "StripSensitive_%d_Box", iStrip + 1);
  try {
    b = new G4Box(name,
                  0.5 * stripPos->Length -
                  stripGeometry->NoScintillationThickness,
                  0.5 * stripGeometry->Width -
                  stripGeometry->NoScintillationThickness,
                  0.5 * stripGeometry->Thickness -
                  stripGeometry->NoScintillationThickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "StripSensitive_%d", iStrip + 1);
  t = HepGeom::Translate3D(0., 0., 0.5 * (stripGeometry->Thickness -
                                          stripGeometry->GrooveDepth));
  try {
    scintillatorSolid =
      new G4SubtractionSolid(name, b, m_Solids.groove[iStrip], t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    m_LogVol.scint[iStrip] =
      new G4LogicalVolume(scintillatorSolid, m_Materials.polystyrene,
                          name, 0, m_Sensitive[0], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*m_LogVol.scint[iStrip], false);
  geometry::setColor(*m_LogVol.scint[iStrip], "#ffffffff");
}

void EKLM::GeoEKLMCreator::createShieldDetailALogicalVolume()
{
  G4Box* box = NULL;
  G4SubtractionSolid* ss1, *ss2, *ss3, *solidDetailA;
  double lx, ly;
  const struct ShieldGeometry* shieldGeometry = m_GeoDat->getShieldGeometry();
  HepGeom::Transform3D t = HepGeom::Translate3D(0, 0, 0);
  lx = shieldGeometry->DetailA.LengthX / 2;
  ly = shieldGeometry->DetailA.LengthY / 2;
  try {
    box = new G4Box("ShieldDetailA_Box", lx, ly, shieldGeometry->Thickness / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  ss1 = cutSolidCorner("ShieldDetailA_Subtraction_1", box,
                       m_Solids.subtractionBox, t, false,
                       shieldGeometry->DetailA.Points[0].X - lx,
                       shieldGeometry->DetailA.Points[0].Y - ly,
                       shieldGeometry->DetailA.Points[1].X - lx,
                       shieldGeometry->DetailA.Points[1].Y - ly);
  ss2 = cutSolidCorner("ShieldDetailA_Subtraction_2", ss1,
                       m_Solids.subtractionBox, t, true,
                       shieldGeometry->DetailA.Points[3].X - lx,
                       shieldGeometry->DetailA.Points[3].Y - ly,
                       shieldGeometry->DetailA.Points[2].X - lx,
                       shieldGeometry->DetailA.Points[2].Y - ly);
  ss3 = cutSolidCorner("ShieldDetailA_Subtraction_3", ss2,
                       m_Solids.subtractionBox, t, false,
                       shieldGeometry->DetailA.Points[3].X - lx,
                       shieldGeometry->DetailA.Points[3].Y - ly,
                       shieldGeometry->DetailA.Points[4].X - lx,
                       shieldGeometry->DetailA.Points[4].Y - ly);
  solidDetailA = cutSolidCorner("ShieldDetailA", ss3,
                                m_Solids.subtractionBox, t, false,
                                shieldGeometry->DetailA.Points[5].X - lx,
                                shieldGeometry->DetailA.Points[5].Y - ly,
                                shieldGeometry->DetailA.Points[6].X - lx,
                                shieldGeometry->DetailA.Points[6].Y - ly);
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
  G4Box* box = NULL;
  G4SubtractionSolid* ss1, *solidDetailB;
  double lx, ly;
  const struct ShieldGeometry* shieldGeometry = m_GeoDat->getShieldGeometry();
  HepGeom::Transform3D t = HepGeom::Translate3D(0, 0, 0);
  lx = shieldGeometry->DetailB.LengthX / 2;
  ly = shieldGeometry->DetailB.LengthY / 2;
  try {
    box = new G4Box("ShieldDetailB_Box", lx, ly, shieldGeometry->Thickness / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  ss1 = cutSolidCorner("ShieldDetailB_Subtraction_1", box,
                       m_Solids.subtractionBox, t, false,
                       shieldGeometry->DetailB.Points[0].X - lx,
                       shieldGeometry->DetailB.Points[0].Y - ly,
                       shieldGeometry->DetailB.Points[1].X - lx,
                       shieldGeometry->DetailB.Points[1].Y - ly);
  solidDetailB = cutSolidCorner("ShieldDetailB", ss1,
                                m_Solids.subtractionBox, t, false,
                                shieldGeometry->DetailB.Points[2].X - lx,
                                shieldGeometry->DetailB.Points[2].Y - ly,
                                shieldGeometry->DetailB.Points[3].X - lx,
                                shieldGeometry->DetailB.Points[3].Y - ly);
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
  G4Box* box = NULL;
  G4SubtractionSolid* ss1, *ss2, *ss3, *solidDetailC;
  double lx, ly;
  const struct ShieldGeometry* shieldGeometry = m_GeoDat->getShieldGeometry();
  HepGeom::Transform3D t = HepGeom::Translate3D(0, 0, 0);
  lx = shieldGeometry->DetailC.LengthX / 2;
  ly = shieldGeometry->DetailC.LengthY / 2;
  try {
    box = new G4Box("ShieldDetailC_Box", lx, ly, shieldGeometry->Thickness / 2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  ss1 = cutSolidCorner("ShieldDetailC_Subtraction_1", box,
                       m_Solids.subtractionBox, t, false,
                       shieldGeometry->DetailC.Points[0].X - lx,
                       shieldGeometry->DetailC.Points[0].Y - ly,
                       shieldGeometry->DetailC.Points[1].X - lx,
                       shieldGeometry->DetailC.Points[1].Y - ly);
  ss2 = cutSolidCorner("ShieldDetailC_Subtraction_2", ss1,
                       m_Solids.subtractionBox, t, true,
                       shieldGeometry->DetailC.Points[3].X - lx,
                       shieldGeometry->DetailC.Points[3].Y - ly,
                       shieldGeometry->DetailC.Points[2].X - lx,
                       shieldGeometry->DetailC.Points[2].Y - ly);
  ss3 = cutSolidCorner("ShieldDetailC_Subtraction_3", ss2,
                       m_Solids.subtractionBox, t, false,
                       shieldGeometry->DetailC.Points[3].X - lx,
                       shieldGeometry->DetailC.Points[3].Y - ly,
                       shieldGeometry->DetailC.Points[4].X - lx,
                       shieldGeometry->DetailC.Points[4].Y - ly);
  solidDetailC = cutSolidCorner("ShieldDetailC", ss3,
                                m_Solids.subtractionBox, t, false,
                                shieldGeometry->DetailC.Points[5].X - lx,
                                shieldGeometry->DetailC.Points[5].Y - ly,
                                shieldGeometry->DetailC.Points[6].X - lx,
                                shieldGeometry->DetailC.Points[6].Y - ly);
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
  const struct ShieldGeometry* shieldGeometry = m_GeoDat->getShieldGeometry();
  try {
    solidDetailDPrism =
      new G4TriangularPrism("ShieldDetailD_Prism",
                            shieldGeometry->DetailD.LengthX, 0.,
                            shieldGeometry->DetailD.LengthY, 90. * CLHEP::deg,
                            shieldGeometry->Thickness / 2);
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
  const struct BoardGeometry* boardGeometry;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  /* Endcap, layer, sector. */
  createEndcapSolid();
  createLayerLogicalVolume();
  createSectorLogicalVolume();
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
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground)
    calcBoardTransform();
  for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
    createPlaneSolid(i);
    /* Segment support. */
    for (j = 1; j <= m_GeoDat->getNSegments() + 1; j++)
      createSegmentSupportLogicalVolume(i + 1, j);
  }
  /* Strips. */
  n = m_GeoDat->getNStripsDifferentLength();
  for (i = 0; i < n; i++) {
    if (m_GeoDat->getDetectorMode() == c_DetectorBackground)
      createStripVolumeLogicalVolume(i);
    createStripLogicalVolume(i);
    createStripGrooveLogicalVolume(i);
    createScintillatorLogicalVolume(i);
  }
  /* Plastic sheet elements. */
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    createPlasticSheetSolid(i);
  /* For background mode. */
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    boardGeometry = m_GeoDat->getBoardGeometry();
    /* Readout board. */
    try {
      m_Solids.board = new G4Box("Board", 0.5 * boardGeometry->Length,
                                 0.5 * boardGeometry->Height,
                                 0.5 * boardGeometry->Width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      m_Solids.baseboard = new G4Box("BaseBoard", 0.5 * boardGeometry->Length,
                                     0.5 * boardGeometry->BaseHeight,
                                     0.5 * boardGeometry->BaseWidth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      m_Solids.stripboard =
        new G4Box("StripBoard", 0.5 * boardGeometry->StripLength,
                  0.5 * boardGeometry->StripHeight,
                  0.5 * boardGeometry->StripWidth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* SiPM (not really a SiPM; a cube in the place of SiPM) */
    try {
      m_Solids.sipm = new G4Box("SiPM", 0.5 * stripGeometry->RSSSize,
                                0.5 * stripGeometry->RSSSize,
                                0.5 * stripGeometry->RSSSize);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* Shield layer details. */
  createShieldDetailALogicalVolume();
  createShieldDetailBLogicalVolume();
  createShieldDetailCLogicalVolume();
  createShieldDetailDLogicalVolume();
}

/************************** CREATION OF VOLUMES ******************************/

G4LogicalVolume*
EKLM::GeoEKLMCreator::createEndcap(G4LogicalVolume* topVolume) const
{
  G4LogicalVolume* logicEndcap = NULL;
  const HepGeom::Transform3D* t;
  std::string endcapName = "Endcap_" +
                           boost::lexical_cast<std::string>(m_CurVol.endcap);
  try {
    logicEndcap = new G4LogicalVolume(m_Solids.endcap, m_Materials.iron,
                                      endcapName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicEndcap, true);
  geometry::setColor(*logicEndcap, "#ffffff22");
  t = m_TransformData->getEndcapTransform(m_CurVol.endcap);
  try {
    new G4PVPlacement(*t, logicEndcap, endcapName, topVolume, false,
                      m_CurVol.endcap, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicEndcap;
}

G4LogicalVolume* EKLM::GeoEKLMCreator::
createLayer(G4LogicalVolume* endcap, G4LogicalVolume* layer) const
{
  G4LogicalVolume* logicLayer;
  const HepGeom::Transform3D* t;
  if (layer == NULL) {
    std::string layerName = "Layer_" +
                            boost::lexical_cast<std::string>(m_CurVol.layer) +
                            "_" + endcap->GetName();
    logicLayer = createLayerLogicalVolume(layerName.c_str());
  } else
    logicLayer = layer;
  t = m_TransformData->getLayerTransform(m_CurVol.endcap, m_CurVol.layer);
  try {
    new G4PVPlacement(*t, logicLayer, logicLayer->GetName(), endcap, false,
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
  if (sector == NULL) {
    std::string sectorName = "Sector_" +
                             boost::lexical_cast<std::string>(m_CurVol.sector) +
                             "_" + layer->GetName();
    logicSector = createSectorLogicalVolume(sectorName.c_str());
  } else
    logicSector = sector;
  t = m_TransformData->getSectorTransform(m_CurVol.endcap, m_CurVol.layer,
                                          m_CurVol.sector);
  try {
    new G4PVPlacement(*t, logicSector, logicSector->GetName(), layer, false,
                      m_CurVol.sector, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicSector;
}

void EKLM::GeoEKLMCreator::calcBoardTransform()
{
  int i;
  int j;
  const struct BoardGeometry* boardGeometry = m_GeoDat->getBoardGeometry();
  const struct BoardPosition* boardPos;
  for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
    for (j = 0; j < m_GeoDat->getNBoards(); j++) {
      boardPos = m_GeoDat->getBoardPosition(i + 1, j + 1);
      m_BoardTransform[i][j] =
        HepGeom::Transform3D(
          HepGeom::RotateZ3D(boardPos->Phi) *
          HepGeom::Translate3D(boardPos->R - 0.5 * boardGeometry->Height, 0., 0.) *
          HepGeom::RotateZ3D(90.0 * CLHEP::deg));
    }
  }
}

void EKLM::GeoEKLMCreator::
createSectorCover(int iCover, G4LogicalVolume* sector) const
{
  double z;
  HepGeom::Transform3D t;
  G4LogicalVolume* lv = m_LogVol.cover;
  const struct ElementPosition* sectorPos = m_GeoDat->getSectorPosition();
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  z = 0.25 * (sectorPos->Length + sectorSupportPos->Length);
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
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(0., 0., sectorSupportGeometry->Corner1Z);
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
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(m_Solids.sectorsup.c2x, m_Solids.sectorsup.c2y,
                           sectorSupportGeometry->Corner2Z);
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
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(m_Solids.sectorsup.c3x, m_Solids.sectorsup.c3y,
                           sectorSupportGeometry->Corner3Z);
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
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = HepGeom::Translate3D(m_Solids.sectorsup.c4x, m_Solids.sectorsup.c4y,
                           sectorSupportGeometry->Corner4Z);
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
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  t = HepGeom::Translate3D(0., 0., sectorSupportPos->Z);
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
  G4LogicalVolume* logicPlane = NULL;
  const HepGeom::Transform3D* t;
  std::string planeName =
    "Plane_" + boost::lexical_cast<std::string>(m_CurVol.plane) + "_" +
    sector->GetName();
  try {
    logicPlane = new G4LogicalVolume(m_Solids.plane[m_CurVol.plane - 1],
                                     m_Materials.air, planeName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicPlane, false);
  t = m_TransformData->getPlaneTransform(m_CurVol.endcap, m_CurVol.layer,
                                         m_CurVol.sector, m_CurVol.plane);
  try {
    new G4PVPlacement(*t, logicPlane, planeName, sector, false,
                      m_CurVol.plane, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicPlane;
}

G4LogicalVolume*
EKLM::GeoEKLMCreator::createSegmentReadoutBoard(G4LogicalVolume* sector) const
{
  G4LogicalVolume* logicSegmentReadoutBoard = NULL;
  std::string boardName =
    "SegmentReadoutBoard_" + boost::lexical_cast<std::string>(m_CurVol.board) +
    "_Plane_" + boost::lexical_cast<std::string>(m_CurVol.plane) +
    "_" + sector->GetName();
  try {
    logicSegmentReadoutBoard = new G4LogicalVolume(m_Solids.board,
                                                   m_Materials.air, boardName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSegmentReadoutBoard, false);
  try {
    new G4PVPlacement(m_BoardTransform[m_CurVol.plane - 1][m_CurVol.board - 1],
                      logicSegmentReadoutBoard, boardName, sector, false,
                      (m_CurVol.plane - 1) * 5 + m_CurVol.board, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return logicSegmentReadoutBoard;
}

void EKLM::GeoEKLMCreator::
createBaseBoard(G4LogicalVolume* segmentReadoutBoard) const
{
  G4LogicalVolume* logicBaseBoard = NULL;
  HepGeom::Transform3D t;
  const struct BoardGeometry* boardGeometry = m_GeoDat->getBoardGeometry();
  std::string boardName = "BaseBoard_" + segmentReadoutBoard->GetName();
  try {
    logicBaseBoard =
      new G4LogicalVolume(m_Solids.baseboard, m_Materials.silicon,
                          boardName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicBaseBoard, true);
  geometry::setColor(*logicBaseBoard, "#0000ffff");
  t = HepGeom::Translate3D(0., -0.5 * boardGeometry->Height +
                           0.5 * boardGeometry->BaseHeight, 0.);
  try {
    new G4PVPlacement(t, logicBaseBoard, boardName, segmentReadoutBoard,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createStripBoard(int iBoard, G4LogicalVolume* segmentReadoutBoard) const
{
  G4LogicalVolume* logicStripBoard = NULL;
  HepGeom::Transform3D t;
  const struct BoardGeometry* boardGeometry = m_GeoDat->getBoardGeometry();
  const struct StripBoardPosition* stripBoardPos =
    m_GeoDat->getStripBoardPosition(iBoard);
  std::string boardName = "StripBoard_" +
                          boost::lexical_cast<std::string>(iBoard) + "_" +
                          segmentReadoutBoard->GetName();
  try {
    logicStripBoard =
      new G4LogicalVolume(m_Solids.stripboard, m_Materials.silicon,
                          boardName, 0, m_Sensitive[2], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicStripBoard, true);
  geometry::setColor(*logicStripBoard, "#0000ffff");
  t = HepGeom::Translate3D(-0.5 * boardGeometry->Length + stripBoardPos->X,
                           -0.5 * boardGeometry->Height + boardGeometry->BaseHeight +
                           0.5 * boardGeometry->StripHeight, 0.);
  try {
    new G4PVPlacement(t, logicStripBoard, boardName, segmentReadoutBoard,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createSegmentSupport(int iSegmentSupport, G4LogicalVolume* plane) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv =
    m_LogVol.segmentsup[m_CurVol.plane - 1][iSegmentSupport - 1];
  const struct SegmentSupportPosition* segmentSupportPos =
    m_GeoDat->getSegmentSupportPosition(m_CurVol.plane, iSegmentSupport);
  t = HepGeom::Translate3D(
        0.5 * (segmentSupportPos->DeltaLLeft -
               segmentSupportPos->DeltaLRight) +
        segmentSupportPos->X, segmentSupportPos->Y, segmentSupportPos->Z) *
      HepGeom::RotateX3D(180.0 * CLHEP::deg);
  try {
    new G4PVPlacement(t, lv, lv->GetName() + "_" + plane->GetName(), plane,
                      false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::
createPlasticSheetElement(int iSheetPlane, int iSheet,
                          G4LogicalVolume* plane) const
{
  double z;
  HepGeom::Transform3D t;
  const struct PlasticSheetGeometry* plasticSheetGeometry =
    m_GeoDat->getPlasticSheetGeometry();
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  std::string sheetName =
    "Sheet_" + boost::lexical_cast<std::string>(iSheet) +
    "_SheetPlane_" + boost::lexical_cast<std::string>(iSheetPlane) +
    "_" + plane->GetName();
  if (m_LogVol.psheet[iSheet - 1] == NULL) {
    try {
      m_LogVol.psheet[iSheet - 1] =
        new G4LogicalVolume(m_Solids.psheet[iSheet - 1],
                            m_Materials.polystyrol, sheetName);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    geometry::setVisibility(*m_LogVol.psheet[iSheet - 1], false);
    geometry::setColor(*m_LogVol.psheet[iSheet - 1], "#00ff00ff");
  }
  z = 0.5 * (stripGeometry->Thickness + plasticSheetGeometry->Width);
  if (iSheetPlane == 2)
    z = -z;
  m_GeoDat->getSheetTransform(&t, (iSheet - 1) * 15);
  t = t * HepGeom::Translate3D(0, 0, z);
  try {
    new G4PVPlacement(t, m_LogVol.psheet[iSheet - 1], sheetName, plane, false,
                      (iSheetPlane - 1) * m_GeoDat->getNSegments() + iSheet,
                      false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStripVolume(G4LogicalVolume* plane) const
{
  int n;
  const HepGeom::Transform3D* t;
  HepGeom::Transform3D t2;
  G4LogicalVolume* lv;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  n = m_GeoDat->getStripLengthIndex(m_CurVol.strip - 1);
  t = m_TransformData->getStripTransform(m_CurVol.endcap, m_CurVol.layer,
                                         m_CurVol.sector, m_CurVol.plane,
                                         m_CurVol.strip);
  t2 = (*t) * HepGeom::Translate3D(0.5 * stripGeometry->RSSSize, 0.0, 0.0) *
       HepGeom::RotateX3D(180.0 * CLHEP::deg);
  lv = m_LogVol.stripvol[n];
  try {
    new G4PVPlacement(t2, lv, lv->GetName(), plane, false, m_CurVol.strip,
                      false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStrip(G4LogicalVolume* plane) const
{
  int n;
  const HepGeom::Transform3D* t;
  HepGeom::Transform3D t2;
  G4LogicalVolume* lv;
  n = m_GeoDat->getStripLengthIndex(m_CurVol.strip - 1);
  t = m_TransformData->getStripTransform(m_CurVol.endcap, m_CurVol.layer,
                                         m_CurVol.sector, m_CurVol.plane,
                                         m_CurVol.strip);
  t2 = (*t) * HepGeom::RotateX3D(180.0 * CLHEP::deg);
  lv = m_LogVol.strip[n];
  try {
    new G4PVPlacement(t2, lv, lv->GetName(), plane, false, m_CurVol.strip,
                      false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStrip(int iStrip) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv;
  G4LogicalVolume* lvm;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  t = HepGeom::Translate3D(-0.5 * stripGeometry->RSSSize, 0., 0.);
  lvm = m_LogVol.stripvol[iStrip];
  lv = m_LogVol.strip[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createStripGroove(int iStrip) const
{
  HepGeom::Transform3D t;
  G4LogicalVolume* lv;
  G4LogicalVolume* lvm;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  t = HepGeom::Translate3D(0., 0., 0.5 * (stripGeometry->Thickness -
                                          stripGeometry->GrooveDepth));
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

void EKLM::GeoEKLMCreator::createSiPM(G4LogicalVolume* stripVolume) const
{
  G4LogicalVolume* logicSiPM = NULL;
  HepGeom::Transform3D t;
  const struct ElementPosition* stripPos =
    m_GeoDat->getStripPosition(m_CurVol.strip);
  std::string sipmName = "SiPM_" + stripVolume->GetName();
  try {
    logicSiPM = new G4LogicalVolume(m_Solids.sipm, m_Materials.silicon,
                                    sipmName, 0, m_Sensitive[1], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSiPM, true);
  geometry::setColor(*logicSiPM, "#0000ffff");
  t = HepGeom::Translate3D(0.5 * stripPos->Length, 0., 0.);
  try {
    new G4PVPlacement(t, logicSiPM, sipmName, stripVolume, false, 1, false);
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
  const struct ShieldGeometry* shieldGeometry = m_GeoDat->getShieldGeometry();
  lx = shieldGeometry->DetailB.LengthX / 2;
  ly = shieldGeometry->DetailB.LengthY / 2;
  ta = HepGeom::Translate3D(shieldGeometry->DetailACenter.X,
                            shieldGeometry->DetailACenter.Y, 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg);
  tb = HepGeom::Translate3D(shieldGeometry->DetailBCenter.X,
                            shieldGeometry->DetailBCenter.Y, 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg);
  tc = HepGeom::Translate3D(shieldGeometry->DetailCCenter.X,
                            shieldGeometry->DetailCCenter.Y, 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
       HepGeom::RotateY3D(180.0 * CLHEP::deg);
  td = HepGeom::Translate3D(
         shieldGeometry->DetailBCenter.X + asqrt2 * (-lx - ly),
         shieldGeometry->DetailBCenter.Y + asqrt2 * (lx - ly), 0) *
       HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
       HepGeom::RotateX3D(180.0 * CLHEP::deg);
  te = HepGeom::Translate3D(
         shieldGeometry->DetailBCenter.X + asqrt2 * (lx - ly),
         shieldGeometry->DetailBCenter.Y + asqrt2 * (-lx - ly), 0) *
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

bool EKLM::GeoEKLMCreator::detectorLayer(int endcap, int layer) const
{
  return ((endcap == 1 && layer <= m_GeoDat->getNDetectorLayers(1)) ||
          (endcap == 2 && layer <= m_GeoDat->getNDetectorLayers(2)));
}

void EKLM::GeoEKLMCreator::create(const GearDir& content,
                                  G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes type)
{
  (void)content;
  (void)type;
  int i, j;
  G4LogicalVolume* endcap, *layer, *sector, *plane, *segmentReadoutBoard;
  createMaterials();
  createSolids();
  /* Create physical volumes which are used only once. */
  for (i = 0; i < m_GeoDat->getNStripsDifferentLength(); i++) {
    if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
      createSiPM(m_LogVol.stripvol[i]);
      createStrip(i);
    }
    createStripGroove(i);
    createScintillator(i);
  }
  /* Create other volumes. */
  for (m_CurVol.endcap = 1; m_CurVol.endcap <= 2; m_CurVol.endcap++) {
    endcap = createEndcap(&topVolume);
    for (m_CurVol.layer = 1; m_CurVol.layer <= m_GeoDat->getNLayers();
         m_CurVol.layer++) {
      if (detectorLayer(m_CurVol.endcap, m_CurVol.layer)) {
        /* Detector layer. */
        layer = createLayer(endcap, NULL);
        for (m_CurVol.sector = 1; m_CurVol.sector <= 4; m_CurVol.sector++) {
          sector = createSector(layer, NULL);
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
            for (i = 1; i <= 2; i++) {
              for (j = 1; j <= m_GeoDat->getNSegments(); j++)
                createPlasticSheetElement(i, j, plane);
            }
            for (m_CurVol.strip = 1; m_CurVol.strip <= m_GeoDat->getNStrips();
                 m_CurVol.strip++)
              switch (m_GeoDat->getDetectorMode()) {
                case c_DetectorNormal:
                  createStrip(plane);
                  break;
                case c_DetectorBackground:
                  createStripVolume(plane);
                  break;
              }
          }
          if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
            for (m_CurVol.plane = 1; m_CurVol.plane <= m_GeoDat->getNPlanes();
                 m_CurVol.plane++) {
              for (m_CurVol.board = 1; m_CurVol.board <= m_GeoDat->getNBoards();
                   m_CurVol.board++) {
                segmentReadoutBoard = createSegmentReadoutBoard(sector);
                createBaseBoard(segmentReadoutBoard);
                for (i = 1; i <= m_GeoDat->getNStripBoards(); i++)
                  createStripBoard(i, segmentReadoutBoard);
              }
            }
          }
        }
      } else {
        /* Shield layer. */
        layer = createLayer(endcap, m_LogVol.shieldLayer);
        for (m_CurVol.sector = 1; m_CurVol.sector <= 4; m_CurVol.sector++)
          sector = createSector(layer, m_LogVol.shieldLayerSector);
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

