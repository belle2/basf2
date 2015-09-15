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
  m_GeoDat = &(EKLM::GeometryData::Instance());
  if (readESTRData(&ESTRPar) == ENOMEM)
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
  free(ESTRPar.z);
  free(ESTRPar.rmin);
  free(ESTRPar.rmax);
  deleteVolumes();
  deleteTransforms();
  deleteSensitive();
}

/***************************** MEMORY ALLOCATION *****************************/

void EKLM::GeoEKLMCreator::newVolumes()
{
  int i, nDiff;
  solids.plane =
    (G4VSolid**)malloc(m_GeoDat->getNPlanes() * sizeof(G4VSolid*));
  if (solids.plane == NULL)
    B2FATAL(MemErr);
  solids.psheet =
    (G4VSolid**)malloc(m_GeoDat->getNSegments() * sizeof(G4VSolid*));
  if (solids.psheet == NULL)
    B2FATAL(MemErr);
  logvol.psheet = (G4LogicalVolume**)
                  malloc(m_GeoDat->getNSegments() * sizeof(G4LogicalVolume*));
  if (logvol.psheet == NULL)
    B2FATAL(MemErr);
  nDiff = m_GeoDat->getNStripsDifferentLength();
  solids.stripvol = (G4VSolid**)malloc(nDiff * sizeof(G4Box*));
  if (solids.stripvol == NULL)
    B2FATAL(MemErr);
  logvol.stripvol = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (logvol.stripvol == NULL)
    B2FATAL(MemErr);
  solids.strip = (G4VSolid**)malloc(nDiff * sizeof(G4Box*));
  if (solids.strip == NULL)
    B2FATAL(MemErr);
  logvol.strip = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (logvol.strip == NULL)
    B2FATAL(MemErr);
  solids.groove = (G4VSolid**)malloc(nDiff * sizeof(G4Box*));
  if (solids.groove == NULL)
    B2FATAL(MemErr);
  logvol.groove = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (logvol.groove == NULL)
    B2FATAL(MemErr);
  solids.scint = (struct EKLM::ScintillatorSolids*)
                 malloc(m_GeoDat->getNStrips() *
                        sizeof(struct EKLM::ScintillatorSolids));
  if (solids.scint == NULL)
    B2FATAL(MemErr);
  logvol.scint = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (logvol.scint == NULL)
    B2FATAL(MemErr);
  solids.segmentsup =
    (struct EKLM::SegmentSupportSolids**)
    malloc(m_GeoDat->getNPlanes() *
           sizeof(struct EKLM::SegmentSupportSolids*));
  if (solids.segmentsup == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
    solids.segmentsup[i] =
      (struct EKLM::SegmentSupportSolids*)
      malloc((m_GeoDat->getNSegments() + 1) *
             sizeof(struct EKLM::SegmentSupportSolids));
    if (solids.segmentsup[i] == NULL)
      B2FATAL(MemErr);
  }
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    logvol.psheet[i] = NULL;
  for (i = 0; i < m_GeoDat->getNPlanes(); i++)
    memset(solids.segmentsup[i], 0,
           (m_GeoDat->getNSegments() + 1) *
           sizeof(struct EKLM::SegmentSupportSolids));
}

void EKLM::GeoEKLMCreator::newTransforms()
{
  int i;
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    for (i = 0; i < m_GeoDat->getNPlanes(); i++) {
      try {
        BoardTransform[i] = new G4Transform3D[m_GeoDat->getNBoards()];
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
    }
  }
}

void EKLM::GeoEKLMCreator::newSensitive()
{
  try {
    m_sensitive[0] =
      new EKLMSensitiveDetector("EKLMSensitiveStrip",
                                c_SensitiveStrip);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    try {
      m_sensitive[1] =
        new EKLMSensitiveDetector("EKLMSensitiveSiPM",
                                  c_SensitiveSiPM);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      m_sensitive[2] =
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
  free(solids.plane);
  free(solids.psheet);
  free(logvol.psheet);
  free(solids.stripvol);
  free(logvol.stripvol);
  free(solids.strip);
  free(logvol.strip);
  free(solids.groove);
  free(logvol.groove);
  free(solids.scint);
  free(logvol.scint);
  for (i = 0; i < m_GeoDat->getNPlanes(); i++)
    free(solids.segmentsup[i]);
  free(solids.segmentsup);
}

void EKLM::GeoEKLMCreator::deleteTransforms()
{
  int i;
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    for (i = 0; i < m_GeoDat->getNPlanes(); i++)
      delete[] BoardTransform[i];
  }
}

void EKLM::GeoEKLMCreator::deleteSensitive()
{
  delete m_sensitive[0];
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    delete m_sensitive[1];
    delete m_sensitive[2];
  }
}

/********************************** XML DATA *********************************/

void EKLM::GeoEKLMCreator::createMaterials()
{
  mat.air = geometry::Materials::get("Air");
  mat.polystyrene = geometry::Materials::get("EKLMPolystyrene");
  mat.polystyrol = geometry::Materials::get("EKLMPolystyrol");
  mat.gel = geometry::Materials::get("EKLMGel");
  mat.iron = geometry::Materials::get("EKLMIron");
  mat.duralumin = geometry::Materials::get("EKLMDuralumin");
  mat.silicon = geometry::Materials::get("EKLMSilicon");
}

/*************************** CREATION OF SOLIDS ******************************/

void EKLM::GeoEKLMCreator::createEndcapSolid()
{
  G4Polyhedra* op = NULL;
  G4Tubs* tb = NULL;
  try {
    op = new G4Polyhedra("Endcap_Octagonal_Prism", ESTRPar.phi, ESTRPar.dphi,
                         ESTRPar.nsides, ESTRPar.nboundary, ESTRPar.z,
                         ESTRPar.rmin, ESTRPar.rmax);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    tb = new G4Tubs("Endcap_Tube",  ESTRPar.rminsub, ESTRPar.rmaxsub,
                    ESTRPar.zsub, 0.0, 360.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solids.endcap = new G4SubtractionSolid("Endcap", op, tb);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSectorCoverSolid()
{
  double lz;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Tubs* solidCoverTube = NULL;
  G4Box* solidCoverBox = NULL;
  G4Box* box = NULL;
  G4IntersectionSolid* is = NULL;
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
  t1 = G4Translate3D(0.5 * planePos->OuterR + sectorSupportPos->X,
                     0.5 * planePos->OuterR + sectorSupportPos->Y, 0.);
  t2 = G4Translate3D(sectorSupportPos->X +
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
                     0.) * G4RotateZ3D(sectorSupportGeometry->CornerAngle);
  try {
    is = new G4IntersectionSolid("Cover_Intersection",
                                 solidCoverTube, solidCoverBox, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solids.cover = new G4SubtractionSolid("Cover", is, box, t2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxX(G4Transform3D& t)
{
  double x;
  G4Box* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  x = sqrt(sectorSupportPos->OuterR * sectorSupportPos->OuterR -
           sectorSupportPos->Y * sectorSupportPos->Y);
  t = G4Translate3D(0.5 * (x + sectorSupportGeometry->Corner3.X),
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

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxY(G4Transform3D& t)
{
  G4Box* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = G4Translate3D(sectorSupportPos->X +
                    0.5 * sectorSupportGeometry->Thickness,
                    0.5 * (sectorSupportGeometry->Corner4.Y +
                           sectorSupportGeometry->Corner1A.Y), 0.) *
      G4RotateZ3D(90. * CLHEP::deg);
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

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxTop(G4Transform3D& t)
{
  double dx;
  double dy;
  G4Box* res = NULL;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  t = G4Translate3D(0.5 * (sectorSupportGeometry->Corner1A.X +
                           sectorSupportGeometry->Corner1B.X +
                           sectorSupportGeometry->Thickness *
                           sin(sectorSupportGeometry->CornerAngle)),
                    0.5 * (sectorSupportGeometry->Corner1A.Y +
                           sectorSupportGeometry->Corner1B.Y -
                           sectorSupportGeometry->Thickness *
                           cos(sectorSupportGeometry->CornerAngle)),
                    0.) * G4RotateZ3D(sectorSupportGeometry->CornerAngle);
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

void EKLM::GeoEKLMCreator::createSectorSupportCorner1Solid()
{
  double lx;
  double x;
  G4Tubs* solidCorner1Tube = NULL;
  G4Box* solidCorner1Box1 = NULL;
  G4Box* solidCorner1Box2 = NULL;
  G4IntersectionSolid* is1 = NULL;
  G4Transform3D t1;
  G4Transform3D t2;
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
  t1 = G4Translate3D(x, 0.5 * sectorSupportPos->OuterR, 0.);
  t2 = G4Translate3D(x, sectorSupportGeometry->Corner1AInner.Y -
                     0.5 * sectorSupportGeometry->Corner1Width /
                     cos(sectorSupportGeometry->CornerAngle) +
                     0.5 * lx * tan(sectorSupportGeometry->CornerAngle), 0.) *
       G4RotateZ3D(sectorSupportGeometry->CornerAngle);
  try {
    is1 = new G4IntersectionSolid("SectorSupport_Corner1_Intersection1",
                                  solidCorner1Tube, solidCorner1Box1, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solids.sectorsup.corn1 =
      new G4IntersectionSolid("SectorSupport_Corner1", is1,
                              solidCorner1Box2, t2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner2Solid()
{
  double r;
  G4TriangularPrism* solidCorner2Prism = NULL;
  G4Transform3D t1;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->OuterR - sectorSupportGeometry->Thickness;
  solids.sectorsup.c2y = sectorSupportPos->Y +
                         sectorSupportGeometry->Thickness;
  solids.sectorsup.c2x = sqrt(r * r -
                              solids.sectorsup.c2y * solids.sectorsup.c2y);
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
  t1 = G4Translate3D(-solids.sectorsup.c2x, -solids.sectorsup.c2y, 0.);
  try {
    solids.sectorsup.corn2 =
      new G4SubtractionSolid("SectorSupport_Corner2",
                             solidCorner2Prism->getSolid(),
                             solids.sectorsup.otube, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  delete solidCorner2Prism;
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner3Solid()
{
  double r;
  double y;
  G4TriangularPrism* solidCorner3Prism = NULL;
  G4Transform3D t1;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->InnerR + sectorSupportGeometry->Thickness;
  y = sectorSupportPos->Y + sectorSupportGeometry->Thickness +
      sectorSupportGeometry->Corner3LY;
  solids.sectorsup.c3x = sqrt(r * r - y * y);
  solids.sectorsup.c3y = sectorSupportPos->Y +
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
  t1 = G4Translate3D(-solids.sectorsup.c3x, -solids.sectorsup.c3y, 0.);
  try {
    solids.sectorsup.corn3 =
      new G4SubtractionSolid("SectorSupport_Corner3",
                             solidCorner3Prism->getSolid(),
                             solids.sectorsup.itube, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  delete solidCorner3Prism;
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner4Solid()
{
  double r;
  double x;
  G4TriangularPrism* solidCorner4Prism = NULL;
  G4Transform3D t1;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  r = sectorSupportPos->InnerR + sectorSupportGeometry->Thickness;
  x = sectorSupportPos->X + sectorSupportGeometry->Thickness +
      sectorSupportGeometry->Corner4LX;
  solids.sectorsup.c4y = sqrt(r * r - x * x);
  solids.sectorsup.c4x = sectorSupportPos->X +
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
  t1 = G4Translate3D(-solids.sectorsup.c4x, -solids.sectorsup.c4y, 0.);
  try {
    solids.sectorsup.corn4 =
      new G4SubtractionSolid("SectorSupport_Corner4",
                             solidCorner4Prism->getSolid(),
                             solids.sectorsup.itube, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  delete solidCorner4Prism;
}

void EKLM::GeoEKLMCreator::createSectorSupportSolid()
{
  G4Box* solidBoxX;
  G4Box* solidBoxY;
  G4Box* solidBoxTop;
  G4Tubs* solidLimitationTube = NULL;
  G4UnionSolid* us1 = NULL;
  G4UnionSolid* us2 = NULL;
  G4UnionSolid* us3 = NULL;
  G4UnionSolid* us4 = NULL;
  G4Transform3D tbx;
  G4Transform3D tby;
  G4Transform3D tbt;
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  solidBoxX = createSectorSupportBoxX(tbx);
  solidBoxY = createSectorSupportBoxY(tby);
  solidBoxTop = createSectorSupportBoxTop(tbt);
  solids.sectorsup.otube = createSectorSupportOuterTube();
  solids.sectorsup.itube = createSectorSupportInnerTube();
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
                           solids.sectorsup.itube, solidBoxY, tby);
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
                           solids.sectorsup.otube, G4Translate3D(0., 0., 0.));
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
    solids.sectorsup.supp = new G4IntersectionSolid("SectorSupport", us4,
                                                    solidLimitationTube,
                                                    G4Translate3D(0., 0., 0.));
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

G4SubtractionSolid* EKLM::GeoEKLMCreator::
cutSolidCorner(char* name, G4VSolid* solid, G4Box* subtractionBox,
               HepGeom::Transform3D& transf, bool largerAngles,
               double x1, double y1, double x2, double y2)
{
  double ang;
  ang = atan2(y2 - y1, x2 - x1);
  return cutSolidCorner(name, solid, subtractionBox, transf, largerAngles,
                        x1, y1, ang);
}

G4SubtractionSolid* EKLM::GeoEKLMCreator::
cutSolidCorner(char* name, G4VSolid* solid, G4Box* subtractionBox,
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
  G4Transform3D t;
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
      t = BoardTransform[i][j];
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
  double box_lx;
  HepGeom::Transform3D t;
  HepGeom::Transform3D t1;
  char name[128];
  G4Tubs* tb = NULL;
  G4Box* b1 = NULL;
  G4IntersectionSolid* is = NULL;
  G4SubtractionSolid* ss1 = NULL;
  G4SubtractionSolid* ss2 = NULL;
  G4SubtractionSolid* ss3 = NULL;
  G4SubtractionSolid* ss4 = NULL;
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
  box_lx = planePos->OuterR;
  try {
    b1 = new G4Box(name, 0.5 * box_lx, 0.5 * box_lx, planePos->Length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  /* Calculate transformations for boolean solids. */
  t1 = HepGeom::Translate3D(0.5 * planePos->OuterR + box_x,
                            0.5 * planePos->OuterR + box_y, 0.);
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
    is = new G4IntersectionSolid(name, tb, b1, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Subtraction_1", n + 1);
  ss1 = cutSolidCorner(name, is, b1, t, true,
                       sectorSupportGeometry->Corner1AInner.X,
                       sectorSupportGeometry->Corner1AInner.Y,
                       sectorSupportGeometry->CornerAngle);
  snprintf(name, 128, "Plane_%d_Subtraction_2", n + 1);
  ss2 = cutSolidCorner(name, ss1, b1, t, false,
                       solids.sectorsup.c2x - sectorSupportGeometry->Corner2LX,
                       solids.sectorsup.c2y, solids.sectorsup.c2x,
                       solids.sectorsup.c2y + sectorSupportGeometry->Corner2LY);
  snprintf(name, 128, "Plane_%d_Subtraction_3", n + 1);
  ss3 = cutSolidCorner(name, ss2, b1, t, false, solids.sectorsup.c3x,
                       solids.sectorsup.c3y + sectorSupportGeometry->Corner3LY,
                       solids.sectorsup.c3x + sectorSupportGeometry->Corner3LX,
                       solids.sectorsup.c3y);
  snprintf(name, 128, "Plane_%d_Subtraction_4", n + 1);
  ss4 = cutSolidCorner(name, ss3, b1, t, true,
                       solids.sectorsup.c4x + sectorSupportGeometry->Corner4LX,
                       solids.sectorsup.c4y, solids.sectorsup.c4x,
                       solids.sectorsup.c4y + sectorSupportGeometry->Corner4LY);
  snprintf(name, 128, "Plane_%d", n + 1);
  solids.plane[n] = subtractBoardSolids(ss4, n);
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
  solids.psheet[n] = unifySolids((G4VSolid**)b, t, 15, name);
}

void EKLM::GeoEKLMCreator::createSolids()
{
  int i, iPos, n;
  char name[128];
  HepGeom::Transform3D t;
  const struct ElementPosition* layerPos = m_GeoDat->getLayerPosition();
  const struct ElementPosition* sectorPos = m_GeoDat->getSectorPosition();
  const struct BoardGeometry* boardGeometry;
  const struct ElementPosition* stripPos;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  /* Endcap. */
  createEndcapSolid();
  /* Layer. */
  try {
    solids.layer = new G4Tubs("Layer", layerPos->InnerR, layerPos->OuterR,
                              layerPos->Length / 2.0, 0.0, 360. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  /* Sector. */
  try {
    solids.sector = new G4Tubs("Sector", sectorPos->InnerR, sectorPos->OuterR,
                               0.5 * sectorPos->Length, 0.0, 90. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createSectorCoverSolid();
  createSectorSupportSolid();
  /**
   * createSectorSupportCornerXSolid() must be called after
   * createSectorSupportSolid()
   */
  createSectorSupportCorner1Solid();
  createSectorSupportCorner2Solid();
  createSectorSupportCorner3Solid();
  createSectorSupportCorner4Solid();
  /* Plane. */
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground)
    calcBoardTransform();
  for (i = 0; i < m_GeoDat->getNPlanes(); i++)
    createPlaneSolid(i);
  /* Strips. */
  n = m_GeoDat->getNStripsDifferentLength();
  for (i = 0; i < n; i++) {
    iPos = m_GeoDat->getStripPositionIndex(i);
    stripPos = m_GeoDat->getStripPosition(iPos + 1);
    /* Strip volumes. */
    snprintf(name, 128, "StripVolume_%d", i + 1);
    try {
      solids.stripvol[i] =
        new G4Box(name, 0.5 * (stripPos->Length + stripGeometry->RSSSize),
                  0.5 * stripGeometry->Width, 0.5 * stripGeometry->Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strips. */
    snprintf(name, 128, "Strip_%d", i + 1);
    try {
      solids.strip[i] = new G4Box(name, 0.5 * stripPos->Length,
                                  0.5 * stripGeometry->Width,
                                  0.5 * stripGeometry->Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strip grooves. */
    snprintf(name, 128, "Groove_%d", i + 1);
    try {
      solids.groove[i] = new G4Box(name, 0.5 * stripPos->Length,
                                   0.5 * stripGeometry->GrooveWidth,
                                   0.5 * stripGeometry->GrooveDepth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strip sensitive volumes (scintillator). */
    snprintf(name, 128, "StripSensitive_%d_Box", i + 1);
    try {
      solids.scint[i].box =
        new G4Box(name,
                  0.5 * stripPos->Length -
                  stripGeometry->NoScintillationThickness,
                  0.5 * stripGeometry->Width -
                  stripGeometry->NoScintillationThickness,
                  0.5 * stripGeometry->Thickness -
                  stripGeometry->NoScintillationThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    snprintf(name, 128, "StripSensitive_%d", i + 1);
    t = HepGeom::Translate3D(0., 0., 0.5 * (stripGeometry->Thickness -
                                            stripGeometry->GrooveDepth));
    try {
      solids.scint[i].sens = new G4SubtractionSolid(name, solids.scint[i].box,
                                                    solids.groove[i], t);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* Plastic sheet elements. */
  for (i = 0; i < m_GeoDat->getNSegments(); i++)
    createPlasticSheetSolid(i);
  /* For background mode. */
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground) {
    boardGeometry = m_GeoDat->getBoardGeometry();
    /* Readout board. */
    try {
      solids.board = new G4Box("Board", 0.5 * boardGeometry->Length,
                               0.5 * boardGeometry->Height,
                               0.5 * boardGeometry->Width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.baseboard = new G4Box("BaseBoard", 0.5 * boardGeometry->Length,
                                   0.5 * boardGeometry->BaseHeight,
                                   0.5 * boardGeometry->BaseWidth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.stripboard =
        new G4Box("StripBoard", 0.5 * boardGeometry->StripLength,
                  0.5 * boardGeometry->StripHeight,
                  0.5 * boardGeometry->StripWidth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* SiPM (not really a SiPM; a cube in the place of SiPM) */
    try {
      solids.sipm = new G4Box("SiPM", 0.5 * stripGeometry->RSSSize,
                              0.5 * stripGeometry->RSSSize,
                              0.5 * stripGeometry->RSSSize);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
}

/************************** CREATION OF VOLUMES ******************************/

void EKLM::GeoEKLMCreator::createEndcap(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicEndcap = NULL;
  const HepGeom::Transform3D* t;
  std::string Endcap_Name = "Endcap_" +
                            boost::lexical_cast<std::string>(curvol.endcap);
  try {
    logicEndcap = new G4LogicalVolume(solids.endcap, mat.iron, Endcap_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicEndcap, true);
  geometry::setColor(*logicEndcap, "#ffffff22");
  t = m_TransformData->getEndcapTransform(curvol.endcap);
  try {
    new G4PVPlacement(*t, logicEndcap, Endcap_Name, mlv, false,
                      curvol.endcap, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (curvol.layer = 1; curvol.layer <= m_GeoDat->getNLayers();
       curvol.layer++)
    createLayer(logicEndcap);
}

void EKLM::GeoEKLMCreator::createLayer(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicLayer = NULL;
  const HepGeom::Transform3D* t;
  std::string Layer_Name = "Layer_" +
                           boost::lexical_cast<std::string>(curvol.layer) +
                           "_" + mlv->GetName();
  try {
    logicLayer = new G4LogicalVolume(solids.layer, mat.air, Layer_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicLayer, false);
  t = m_TransformData->getLayerTransform(curvol.endcap, curvol.layer);
  try {
    new G4PVPlacement(*t, logicLayer, Layer_Name, mlv, false,
                      curvol.layer, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (curvol.sector = 1; curvol.sector <= 4; curvol.sector++)
    createSector(logicLayer);
}

void EKLM::GeoEKLMCreator::createSector(G4LogicalVolume* mlv)
{
  int i;
  const HepGeom::Transform3D* t;
  G4LogicalVolume* logicSector = NULL;
  std::string Sector_Name = "Sector_" +
                            boost::lexical_cast<std::string>(curvol.sector) +
                            "_" + mlv->GetName();
  try {
    logicSector = new G4LogicalVolume(solids.sector, mat.air, Sector_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSector, false);
  t = m_TransformData->getSectorTransform(curvol.endcap, curvol.layer,
                                          curvol.sector);
  try {
    new G4PVPlacement(*t, logicSector, Sector_Name, mlv, false,
                      curvol.sector, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createSectorSupport(logicSector);
  for (i = 1; i <= 2; i++)
    createSectorCover(i, logicSector);
  if ((curvol.endcap == 1 &&
       curvol.layer <= m_GeoDat->getNDetectorLayers(1)) ||
      (curvol.endcap == 2 &&
       curvol.layer <= m_GeoDat->getNDetectorLayers(2))) {
    /* Detector layer. */
    for (curvol.plane = 1; curvol.plane <= m_GeoDat->getNPlanes();
         curvol.plane++)
      createPlane(logicSector);
    if (m_GeoDat->getDetectorMode() == c_DetectorBackground)
      for (curvol.plane = 1; curvol.plane <= m_GeoDat->getNPlanes();
           curvol.plane++)
        for (curvol.board = 1; curvol.board <= m_GeoDat->getNBoards();
             curvol.board++)
          createSegmentReadoutBoard(logicSector);
  } else {
    /* Shield layer. */
  }
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
      BoardTransform[i][j] =
        G4Transform3D(
          G4RotateZ3D(boardPos->Phi) *
          G4Translate3D(boardPos->R - 0.5 * boardGeometry->Height, 0., 0.) *
          G4RotateZ3D(90.0 * CLHEP::deg));
    }
  }
}

void EKLM::GeoEKLMCreator::createSectorCover(int iCover, G4LogicalVolume* mlv)
{
  double z;
  G4LogicalVolume* logicCover = NULL;
  G4Transform3D t;
  const struct ElementPosition* sectorPos = m_GeoDat->getSectorPosition();
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  std::string Cover_Name = "Cover_" +
                           boost::lexical_cast<std::string>(iCover) + "_" +
                           mlv->GetName();
  try {
    logicCover = new G4LogicalVolume(solids.cover, mat.duralumin, Cover_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCover, false);
  geometry::setColor(*logicCover, "#ff000022");
  z = 0.25 * (sectorPos->Length + sectorSupportPos->Length);
  if (iCover == 2)
    z = -z;
  t = G4Translate3D(0., 0., z);
  try {
    new G4PVPlacement(t, logicCover, Cover_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCover);
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner1(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicCorner1 = NULL;
  G4Transform3D t;
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  std::string Corner1_Name = "Corner1_" + mlv->GetName();
  try {
    logicCorner1 =
      new G4LogicalVolume(solids.sectorsup.corn1, mat.duralumin, Corner1_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner1, true);
  geometry::setColor(*logicCorner1, "#ff0000ff");
  t = G4Translate3D(0., 0., sectorSupportGeometry->Corner1Z);
  try {
    new G4PVPlacement(t, logicCorner1, Corner1_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner1);
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner2(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicCorner2 = NULL;
  G4Transform3D t;
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  std::string Corner2_Name = "Corner2_" + mlv->GetName();
  try {
    logicCorner2 = new G4LogicalVolume(solids.sectorsup.corn2, mat.duralumin,
                                       Corner2_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner2, true);
  geometry::setColor(*logicCorner2, "#ff0000ff");
  t = G4Translate3D(solids.sectorsup.c2x, solids.sectorsup.c2y,
                    sectorSupportGeometry->Corner2Z);
  try {
    new G4PVPlacement(t, logicCorner2, Corner2_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner2);
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner3(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicCorner3 = NULL;
  G4Transform3D t;
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  std::string Corner3_Name = "Corner3_" + mlv->GetName();
  try {
    logicCorner3 = new G4LogicalVolume(solids.sectorsup.corn3, mat.duralumin,
                                       Corner3_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner3, true);
  geometry::setColor(*logicCorner3, "#ff0000ff");
  t = G4Translate3D(solids.sectorsup.c3x, solids.sectorsup.c3y,
                    sectorSupportGeometry->Corner3Z);
  try {
    new G4PVPlacement(t, logicCorner3, Corner3_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner3);
}

void EKLM::GeoEKLMCreator::createSectorSupportCorner4(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicCorner4 = NULL;
  G4Transform3D t;
  const struct SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  std::string Corner4_Name = "Corner4_" + mlv->GetName();
  try {
    logicCorner4 = new G4LogicalVolume(solids.sectorsup.corn4, mat.duralumin,
                                       Corner4_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner4, true);
  geometry::setColor(*logicCorner4, "#ff0000ff");
  t = G4Translate3D(solids.sectorsup.c4x, solids.sectorsup.c4y,
                    sectorSupportGeometry->Corner4Z);
  try {
    new G4PVPlacement(t, logicCorner4, Corner4_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner4);
}

void EKLM::GeoEKLMCreator::createSectorSupport(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicSectorSupport = NULL;
  G4Transform3D t;
  std::string SectorSupportName = "Support_" + mlv->GetName();
  const struct ElementPosition* sectorSupportPos =
    m_GeoDat->getSectorSupportPosition();
  t = G4Translate3D(0., 0., sectorSupportPos->Z);
  try {
    logicSectorSupport = new G4LogicalVolume(solids.sectorsup.supp,
                                             mat.duralumin,
                                             SectorSupportName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSectorSupport, true);
  geometry::setColor(*logicSectorSupport, "#ff0000ff");
  try {
    new G4PVPlacement(t, logicSectorSupport, SectorSupportName,
                      mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createSectorSupportCorner1(mlv);
  createSectorSupportCorner2(mlv);
  createSectorSupportCorner3(mlv);
  createSectorSupportCorner4(mlv);
  printVolumeMass(logicSectorSupport);
}

void EKLM::GeoEKLMCreator::createPlane(G4LogicalVolume* mlv)
{
  int i;
  int j;
  const HepGeom::Transform3D* t;
  G4LogicalVolume* logicPlane = NULL;
  std::string Plane_Name =
    "Plane_" + boost::lexical_cast<std::string>(curvol.plane) + "_" +
    mlv->GetName();
  try {
    logicPlane = new G4LogicalVolume(solids.plane[curvol.plane - 1],
                                     mat.air, Plane_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicPlane, false);
  t = m_TransformData->getPlaneTransform(curvol.endcap, curvol.layer,
                                         curvol.sector, curvol.plane);
  try {
    new G4PVPlacement(*t, logicPlane, Plane_Name, mlv, false,
                      curvol.plane, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 1; i <= m_GeoDat->getNSegments() + 1; i++)
    createSegmentSupport(i, logicPlane);
  for (i = 1; i <= 2; i++)
    for (j = 1; j <= m_GeoDat->getNSegments(); j++)
      createPlasticSheetElement(i, j, logicPlane);
  if (curvol.endcap == 1 && curvol.layer == 1 && curvol.sector == 1 &&
      curvol.plane == 1)
    for (i = 0; i < m_GeoDat->getNStripsDifferentLength(); i++) {
      createStripLogicalVolumes(i, logicPlane);
      createStripPhysicalVolumes(i);
    }
  for (curvol.strip = 1; curvol.strip <= m_GeoDat->getNStrips();
       curvol.strip++)
    createStripVolume(logicPlane);
}

void EKLM::GeoEKLMCreator::createSegmentReadoutBoard(G4LogicalVolume* mlv)
{
  int i;
  G4LogicalVolume* logicSegmentReadoutBoard = NULL;
  std::string Board_Name =
    "SegmentReadoutBoard_" + boost::lexical_cast<std::string>(curvol.board) +
    "_Plane_" + boost::lexical_cast<std::string>(curvol.plane) +
    "_" + mlv->GetName();
  try {
    logicSegmentReadoutBoard = new G4LogicalVolume(solids.board,
                                                   mat.air, Board_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSegmentReadoutBoard, false);
  try {
    new G4PVPlacement(BoardTransform[curvol.plane - 1][curvol.board - 1],
                      logicSegmentReadoutBoard, Board_Name, mlv, false,
                      (curvol.plane - 1) * 5 + curvol.board, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createBaseBoard(logicSegmentReadoutBoard);
  for (i = 1; i <= m_GeoDat->getNStripBoards(); i++)
    createStripBoard(i, logicSegmentReadoutBoard);
}

void EKLM::GeoEKLMCreator::createBaseBoard(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicBaseBoard = NULL;
  G4Transform3D t;
  const struct BoardGeometry* boardGeometry = m_GeoDat->getBoardGeometry();
  std::string Board_Name = "BaseBoard_" + mlv->GetName();
  try {
    logicBaseBoard = new G4LogicalVolume(solids.baseboard, mat.silicon,
                                         Board_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicBaseBoard, true);
  geometry::setColor(*logicBaseBoard, "#0000ffff");
  t = G4Translate3D(0., -0.5 * boardGeometry->Height +
                    0.5 * boardGeometry->BaseHeight, 0.);
  try {
    new G4PVPlacement(t, logicBaseBoard, Board_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicBaseBoard);
}

void EKLM::GeoEKLMCreator::createStripBoard(int iBoard, G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicStripBoard = NULL;
  G4Transform3D t;
  const struct BoardGeometry* boardGeometry = m_GeoDat->getBoardGeometry();
  const struct StripBoardPosition* stripBoardPos =
    m_GeoDat->getStripBoardPosition(iBoard);
  std::string Board_Name = "StripBoard_" +
                           boost::lexical_cast<std::string>(iBoard) + "_" +
                           mlv->GetName();
  try {
    logicStripBoard = new G4LogicalVolume(solids.stripboard, mat.silicon,
                                          Board_Name, 0, m_sensitive[2], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicStripBoard, true);
  geometry::setColor(*logicStripBoard, "#0000ffff");
  t = G4Translate3D(-0.5 * boardGeometry->Length + stripBoardPos->X,
                    -0.5 * boardGeometry->Height + boardGeometry->BaseHeight +
                    0.5 * boardGeometry->StripHeight, 0.);
  try {
    new G4PVPlacement(t, logicStripBoard, Board_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicStripBoard);
}

void EKLM::GeoEKLMCreator::createSegmentSupport(int iSegmentSupport,
                                                G4LogicalVolume* mlv)
{
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4LogicalVolume* logicSegmentSupport = NULL;
  std::string SegmentSupportName;
  const struct SegmentSupportPosition* segmentSupportPos =
    m_GeoDat->getSegmentSupportPosition(curvol.plane, iSegmentSupport);
  const struct SegmentSupportGeometry* segmentSupportGeometry =
    m_GeoDat->getSegmentSupportGeometry();
  SegmentSupportName = "SegmentSupport_" +
                       boost::lexical_cast<std::string>(iSegmentSupport) +
                       "_" + mlv->GetName();
  if (solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].secsup == NULL) {
    try {
      solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].topbox =
        new G4Box("BoxTop_" + SegmentSupportName,
                  0.5 * (segmentSupportPos->Length -
                         segmentSupportPos->DeltaLLeft -
                         segmentSupportPos->DeltaLRight),
                  0.5 * segmentSupportGeometry->TopWidth,
                  0.5 * segmentSupportGeometry->TopThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].midbox =
        new G4Box("BoxMiddle_" + SegmentSupportName,
                  0.5 * (segmentSupportPos->Length -
                         segmentSupportPos->DeltaLLeft -
                         segmentSupportPos->DeltaLRight),
                  0.5 * segmentSupportGeometry->MiddleWidth,
                  0.5 * segmentSupportGeometry->MiddleThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].botbox =
        new G4Box("BoxBottom_" + SegmentSupportName,
                  0.5 * segmentSupportPos->Length,
                  0.5 * segmentSupportGeometry->TopWidth,
                  0.5 * segmentSupportGeometry->TopThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    t1 = G4Translate3D(0., 0., 0.5 * (segmentSupportGeometry->MiddleThickness +
                                      segmentSupportGeometry->TopThickness));
    t2 = G4Translate3D(0.5 * (segmentSupportPos->DeltaLRight -
                              segmentSupportPos->DeltaLLeft),
                       0., -0.5 * (segmentSupportGeometry->MiddleThickness +
                                   segmentSupportGeometry->TopThickness));
    try {
      solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].us =
        new G4UnionSolid(
        "Union1_" + SegmentSupportName,
        solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].midbox,
        solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].topbox,
        t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].secsup =
        new G4UnionSolid(
        SegmentSupportName,
        solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].us,
        solids.segmentsup[curvol.plane - 1][iSegmentSupport - 1].botbox,
        t2);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  try {
    logicSegmentSupport =
      new G4LogicalVolume(solids.segmentsup[curvol.plane - 1]
                          [iSegmentSupport - 1].secsup,
                          mat.duralumin, SegmentSupportName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSegmentSupport, true);
  geometry::setColor(*logicSegmentSupport, "#ff0000ff");
  t = G4Translate3D(
        0.5 * (segmentSupportPos->DeltaLLeft -
               segmentSupportPos->DeltaLRight) +
        segmentSupportPos->X, segmentSupportPos->Y, segmentSupportPos->Z);
  try {
    new G4PVPlacement(t, logicSegmentSupport, SegmentSupportName,
                      mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicSegmentSupport);
}

void EKLM::GeoEKLMCreator::createPlasticSheetElement(int iSheetPlane,
                                                     int iSheet,
                                                     G4LogicalVolume* mlv)
{
  double z;
  G4Transform3D t;
  const struct PlasticSheetGeometry* plasticSheetGeometry =
    m_GeoDat->getPlasticSheetGeometry();
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  std::string Sheet_Name =
    "Sheet_" + boost::lexical_cast<std::string>(iSheet) +
    "_SheetPlane_" + boost::lexical_cast<std::string>(iSheetPlane) +
    "_" + mlv->GetName();
  if (logvol.psheet[iSheet - 1] == NULL) {
    try {
      logvol.psheet[iSheet - 1] =
        new G4LogicalVolume(solids.psheet[iSheet - 1],
                            mat.polystyrol, Sheet_Name);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    geometry::setVisibility(*logvol.psheet[iSheet - 1], false);
    geometry::setColor(*logvol.psheet[iSheet - 1], "#00ff00ff");
  }
  z = 0.5 * (stripGeometry->Thickness + plasticSheetGeometry->Width);
  if (iSheetPlane == 2)
    z = -z;
  m_GeoDat->getSheetTransform(&t, (iSheet - 1) * 15);
  t = t * G4Translate3D(0, 0, z);
  try {
    new G4PVPlacement(t, logvol.psheet[iSheet - 1], Sheet_Name, mlv, false,
                      (iSheetPlane - 1) * m_GeoDat->getNSegments() + iSheet,
                      false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logvol.psheet[iSheet - 1]);
}

void EKLM::GeoEKLMCreator::createStripLogicalVolumes(int iStrip,
                                                     G4LogicalVolume* mlv)
{
  std::string StripVolume_Name = "StripVolume_" +
                                 boost::lexical_cast<std::string>(iStrip)
                                 + "_" + mlv->GetName();
  std::string Strip_Name = "Strip_" + StripVolume_Name;
  std::string Groove_Name = "Groove_" + Strip_Name;
  std::string Sensitive_Name = "Sensitive_" + Strip_Name;
  try {
    logvol.stripvol[iStrip] =
      new G4LogicalVolume(solids.stripvol[iStrip], mat.air, StripVolume_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    logvol.strip[iStrip] =
      new G4LogicalVolume(solids.strip[iStrip], mat.polystyrene, Strip_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    logvol.groove[iStrip] =
      new G4LogicalVolume(solids.groove[iStrip], mat.gel, Groove_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    logvol.scint[iStrip] =
      new G4LogicalVolume(solids.scint[iStrip].sens,
                          mat.polystyrene, Sensitive_Name,
                          0, m_sensitive[0], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logvol.stripvol[iStrip], false);
  geometry::setVisibility(*logvol.strip[iStrip], true);
  geometry::setColor(*logvol.strip[iStrip], "#ffffffff");
  geometry::setVisibility(*logvol.groove[iStrip], true);
  geometry::setColor(*logvol.groove[iStrip], "#00ff00ff");
  geometry::setVisibility(*logvol.scint[iStrip], false);
  geometry::setColor(*logvol.scint[iStrip], "#ffffffff");
  printVolumeMass(logvol.strip[iStrip]);
  printVolumeMass(logvol.groove[iStrip]);
  printVolumeMass(logvol.scint[iStrip]);
}

void EKLM::GeoEKLMCreator::createStripPhysicalVolumes(int iStrip)
{
  G4Transform3D t;
  G4LogicalVolume* lv;
  G4LogicalVolume* lvm;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  t = G4Translate3D(-0.5 * stripGeometry->RSSSize, 0., 0.);
  lvm = logvol.stripvol[iStrip];
  lv = logvol.strip[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t = G4Translate3D(0., 0., 0.5 * (stripGeometry->Thickness -
                                   stripGeometry->GrooveDepth));
  lvm = lv;
  lv = logvol.groove[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t = G4Translate3D(0., 0., 0.);
  lv = logvol.scint[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  if (m_GeoDat->getDetectorMode() == c_DetectorBackground)
    createSiPM(logvol.stripvol[iStrip]);
}

void EKLM::GeoEKLMCreator::createStripVolume(G4LogicalVolume* mlv)
{
  int n;
  const HepGeom::Transform3D* t;
  HepGeom::Transform3D t2;
  G4LogicalVolume* lv;
  const struct StripGeometry* stripGeometry = m_GeoDat->getStripGeometry();
  n = m_GeoDat->getStripLengthIndex(curvol.strip - 1);
  t = m_TransformData->getStripTransform(curvol.endcap, curvol.layer,
                                         curvol.sector, curvol.plane,
                                         curvol.strip);
  t2 = (*t) * G4Translate3D(0.5 * stripGeometry->RSSSize, 0.0, 0.0) *
       HepGeom::RotateX3D(180.0 * CLHEP::deg);
  lv = logvol.stripvol[n];
  try {
    new G4PVPlacement(t2, lv, lv->GetName(), mlv, false, curvol.strip, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSiPM(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicSiPM = NULL;
  G4Transform3D t;
  const struct ElementPosition* stripPos =
    m_GeoDat->getStripPosition(curvol.strip);
  std::string SiPM_Name = "SiPM_" + mlv->GetName();
  try {
    logicSiPM = new G4LogicalVolume(solids.sipm, mat.silicon, SiPM_Name, 0,
                                    m_sensitive[1], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSiPM, true);
  geometry::setColor(*logicSiPM, "#0000ffff");
  t = G4Translate3D(0.5 * stripPos->Length, 0., 0.);
  try {
    new G4PVPlacement(t, logicSiPM, SiPM_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicSiPM);
}

void EKLM::GeoEKLMCreator::printVolumeMass(G4LogicalVolume* lv)
{
  if (m_GeoDat->getDetectorMode() == c_DetectorPrintMasses)
    printf("Volume %s: mass = %g g\n", lv->GetName().c_str(),
           lv->GetMass() / CLHEP::g);
}

void EKLM::GeoEKLMCreator::create(const GearDir& content,
                                  G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes type)
{
  (void)content;
  (void)type;
  createMaterials();
  createSolids();
  for (curvol.endcap = 1; curvol.endcap <= 2; curvol.endcap++)
    createEndcap(&topVolume);
  if (m_GeoDat->getDetectorMode() == c_DetectorPrintMasses) {
    printf("EKLM started in mode c_DetectorPrintMasses. Exiting now.\n");
    exit(0);
  }
}

