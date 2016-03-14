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

/* C++ headers. */
#include <new>

/* External headers. */
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "CLHEP/Units/PhysicalConstants.h"
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

void EKLM::GeoEKLMCreator::constructor(bool geo)
{
  haveGeo = false;
  m_geoDat = NULL;
  readXMLData();
  if (geo) {
    try {
      m_geoDat = new EKLM::GeometryData;
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    if (m_geoDat->read() != 0)
      B2FATAL("Cannot read geometry data file.");
  }
}

EKLM::GeoEKLMCreator::GeoEKLMCreator()
{
  constructor(true);
}

EKLM::GeoEKLMCreator::GeoEKLMCreator(bool geo)
{
  constructor(geo);
}

EKLM::GeoEKLMCreator::~GeoEKLMCreator()
{
  int i, j;
  if (m_geoDat != NULL)
    delete m_geoDat;
  /* Free geometry data. */
  for (i = 0; i < nPlane; i++) {
    free(BoardPosition[i]);
    free(SectionSupportPosition[i]);
  }
  free(StripPosition);
  free(StripBoardPosition);
  free(ESTRPar.z);
  free(ESTRPar.rmin);
  free(ESTRPar.rmax);
  if (haveGeo) {
    for (i = 0; i < nPlane; i++) {
      for (j = 0; j < nBoard; j++)
        delete BoardTransform[i][j];
      free(BoardTransform[i]);
    }
    delete m_sensitive[0];
    if (m_mode == EKLM_DETECTOR_BACKGROUND) {
      delete m_sensitive[1];
      delete m_sensitive[2];
    }
    freeVolumes();
  }
}

/***************************** MEMORY ALLOCATION *****************************/

void EKLM::GeoEKLMCreator::mallocVolumes()
{
  int i, nDiff;
  solids.plane = (G4VSolid**)malloc(nPlane * sizeof(G4VSolid*));
  if (solids.plane == NULL)
    B2FATAL(MemErr);
  solids.psheet = (G4VSolid**)malloc(nSection * sizeof(G4VSolid*));
  if (solids.psheet == NULL)
    B2FATAL(MemErr);
  logvol.psheet = (G4LogicalVolume**)
                  malloc(nSection * sizeof(G4LogicalVolume*));
  if (logvol.psheet == NULL)
    B2FATAL(MemErr);
  nDiff = m_geoDat->getNStripsDifferentLength();
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
                 malloc(m_nStrip * sizeof(struct EKLM::ScintillatorSolids));
  if (solids.scint == NULL)
    B2FATAL(MemErr);
  logvol.scint = (G4LogicalVolume**)malloc(nDiff * sizeof(G4LogicalVolume*));
  if (logvol.scint == NULL)
    B2FATAL(MemErr);
  solids.sectionsup = (struct EKLM::SectionSupportSolids**)
                      malloc(nPlane * sizeof(struct EKLM::SectionSupportSolids*));
  if (solids.sectionsup == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < nPlane; i++) {
    solids.sectionsup[i] = (struct EKLM::SectionSupportSolids*)
                           malloc((nSection + 1) *
                                  sizeof(struct EKLM::SectionSupportSolids));
    if (solids.sectionsup[i] == NULL)
      B2FATAL(MemErr);
  }
  for (i = 0; i < nSection; i++)
    logvol.psheet[i] = NULL;
  for (i = 0; i < nPlane; i++)
    memset(solids.sectionsup[i], 0,
           (nSection + 1) * sizeof(struct EKLM::SectionSupportSolids));
}

void EKLM::GeoEKLMCreator::freeVolumes()
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
  for (i = 0; i < nPlane; i++)
    free(solids.sectionsup[i]);
  free(solids.sectionsup);
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

/**
 * Read position data.
 * @param epos Position data.
 * @param gd   XML data directory.
 */
static void readPositionData(struct EKLM::ElementPosition* epos, GearDir* gd)
{
  epos->X = gd->getLength("PositionX") * CLHEP::cm;
  epos->Y = gd->getLength("PositionY") * CLHEP::cm;
  epos->Z = gd->getLength("PositionZ") * CLHEP::cm;
}

/**
 * Read size data.
 * @param epos Position data.
 * @param gd   XML data directory.
 */
static void readSizeData(struct EKLM::ElementPosition* epos, GearDir* gd)
{
  epos->innerR = gd->getLength("InnerR") * CLHEP::cm;
  epos->outerR = gd->getLength("OuterR") * CLHEP::cm;
  epos->length = gd->getLength("Length") * CLHEP::cm;
}

/**
 * Read sector support geometry data.
 * @param ssg Sector support size data.
 * @param gd  XML data directory.
 */
static void readSectorSupportData(struct EKLM::SectorSupportGeometry* ssg,
                                  GearDir* gd)
{
  ssg->Thickness = gd->getLength("Thickness") * CLHEP::cm;
  ssg->DeltaLY = gd->getLength("DeltaLY") * CLHEP::cm;
  ssg->CornerX = gd->getLength("CornerX") * CLHEP::cm;
  ssg->Corner1LX = gd->getLength("Corner1LX") * CLHEP::cm;
  ssg->Corner1Width = gd->getLength("Corner1Width") * CLHEP::cm;
  ssg->Corner1Thickness = gd->getLength("Corner1Thickness") * CLHEP::cm;
  ssg->Corner1Z = gd->getLength("Corner1Z") * CLHEP::cm;
  ssg->Corner2LX = gd->getLength("Corner2LX") * CLHEP::cm;
  ssg->Corner2LY = gd->getLength("Corner2LY") * CLHEP::cm;
  ssg->Corner2Thickness = gd->getLength("Corner2Thickness") * CLHEP::cm;
  ssg->Corner2Z = gd->getLength("Corner2Z") * CLHEP::cm;
  ssg->Corner3LX = gd->getLength("Corner3LX") * CLHEP::cm;
  ssg->Corner3LY = gd->getLength("Corner3LY") * CLHEP::cm;
  ssg->Corner3Thickness = gd->getLength("Corner3Thickness") * CLHEP::cm;
  ssg->Corner3Z = gd->getLength("Corner3Z") * CLHEP::cm;
  ssg->Corner4LX = gd->getLength("Corner4LX") * CLHEP::cm;
  ssg->Corner4LY = gd->getLength("Corner4LY") * CLHEP::cm;
  ssg->Corner4Thickness = gd->getLength("Corner4Thickness") * CLHEP::cm;
  ssg->Corner4Z = gd->getLength("Corner4Z") * CLHEP::cm;
}

/**
 * Calculate sector support geometry data.
 * @param ssg Sector support geometry data.
 */
void EKLM::GeoEKLMCreator::calculateSectorSupportData()
{
  /* Corner 1. */
  m_SectorSupportData.Corner1A.X = SectorSupportPosition.X;
  m_SectorSupportData.Corner1A.Y = SectorSupportPosition.outerR -
                                   m_SectorSupportData.DeltaLY;
  m_SectorSupportData.Corner1B.X = SectorSupportPosition.X +
                                   m_SectorSupportData.CornerX;
  m_SectorSupportData.Corner1B.Y = sqrt(SectorSupportPosition.outerR *
                                        SectorSupportPosition.outerR -
                                        m_SectorSupportData.Corner1B.X *
                                        m_SectorSupportData.Corner1B.X);
  m_SectorSupportData.CornerAngle = atan2(m_SectorSupportData.Corner1B.Y -
                                          m_SectorSupportData.Corner1A.Y,
                                          m_SectorSupportData.Corner1B.X -
                                          m_SectorSupportData.Corner1A.X) *
                                    CLHEP::rad;
  m_SectorSupportData.Corner1AInner.X = SectorSupportPosition.X +
                                        m_SectorSupportData.Thickness;
  m_SectorSupportData.Corner1AInner.Y =
    m_SectorSupportData.Corner1A.Y -
    m_SectorSupportData.Thickness *
    (1.0 / cos(m_SectorSupportData.CornerAngle) -
     tan(m_SectorSupportData.CornerAngle));
  /* Corner 3. */
  m_SectorSupportData.Corner3.X = sqrt(SectorSupportPosition.innerR *
                                       SectorSupportPosition.innerR -
                                       SectorSupportPosition.Y *
                                       SectorSupportPosition.Y);
  m_SectorSupportData.Corner3.Y = SectorSupportPosition.Y;
  /* Corner 4. */
  m_SectorSupportData.Corner4.X = SectorSupportPosition.X;
  m_SectorSupportData.Corner4.Y = sqrt(SectorSupportPosition.innerR *
                                       SectorSupportPosition.innerR -
                                       SectorSupportPosition.X *
                                       SectorSupportPosition.X);
}

void EKLM::GeoEKLMCreator::readXMLDataStrips()
{
  int i;
  GearDir Strips("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
                 "Layer/Sector/Plane/Strips");
  m_nStrip = Strips.getNumberNodes("Strip");
  StripSize.width  = Strips.getLength("Width") * CLHEP::cm;
  StripSize.thickness = Strips.getLength("Thickness") * CLHEP::cm;
  StripSize.groove_depth = Strips.getLength("GrooveDepth") * CLHEP::cm;
  StripSize.groove_width = Strips.getLength("GrooveWidth") * CLHEP::cm;
  StripSize.no_scintillation_thickness = Strips.
                                         getLength("NoScintillationThickness")
                                         * CLHEP::cm;
  StripSize.rss_size = Strips.getLength("RSSSize") * CLHEP::cm;
  StripPosition = (struct EKLM::ElementPosition*)
                  malloc(m_nStrip * sizeof(struct EKLM::ElementPosition));
  if (StripPosition == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_nStrip; i++) {
    GearDir StripContent(Strips);
    StripContent.append((boost::format("/Strip[%1%]") % (i + 1)).str());
    StripPosition[i].length = StripContent.getLength("Length") * CLHEP::cm;
    StripPosition[i].X = StripContent.getLength("PositionX") * CLHEP::cm;
    StripPosition[i].Y = StripContent.getLength("PositionY") * CLHEP::cm;
    StripPosition[i].Z = StripContent.getLength("PositionZ") * CLHEP::cm;
  }
}

void EKLM::GeoEKLMCreator::readXMLData()
{
  int i;
  int j;
  if (readESTRData(&ESTRPar) == ENOMEM)
    B2FATAL(MemErr);
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
  m_mode = (enum EKLMDetectorMode)gd.getInt("Mode");
  if (m_mode < 0 || m_mode > 2)
    B2FATAL("EKLM started with unknown geometry mode " << m_mode << ".");
  m_solenoidZ = gd.getLength("SolenoidZ") * CLHEP::cm;
  GearDir EndCap(gd);
  EndCap.append("/Endcap");
  readPositionData(&EndcapPosition, &EndCap);
  readSizeData(&EndcapPosition, &EndCap);
  m_nLayer = EndCap.getInt("nLayer");
  if (m_nLayer <= 0 || m_nLayer > 14)
    B2FATAL("Number of layers must be from 1 to 14");
  m_nLayerForward = EndCap.getInt("nLayerForward");
  if (m_nLayerForward < 0)
    B2FATAL("Number of detector layers in the forward endcap "
            "must be nonnegative.");
  if (m_nLayerForward > m_nLayer)
    B2FATAL("Number of detector layers in the forward endcap "
            "must be less than or equal to the number of layers.");
  m_nLayerBackward = EndCap.getInt("nLayerBackward");
  if (m_nLayerBackward < 0)
    B2FATAL("Number of detector layers in the backward endcap "
            "must be nonnegative.");
  if (m_nLayerBackward > m_nLayer)
    B2FATAL("Number of detector layers in the backward endcap "
            "must be less than or equal to the number of layers.");
  GearDir Layer(EndCap);
  Layer.append("/Layer");
  readSizeData(&LayerPosition, &Layer);
  Layer_shiftZ = Layer.getLength("ShiftZ") * CLHEP::cm;
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readSizeData(&SectorPosition, &Sector);
  nPlane = Sector.getInt("nPlane");
  nBoard = Sector.getInt("nBoard");
  GearDir Boards(Sector);
  Boards.append("/Boards");
  BoardSize.length = Boards.getLength("Length") * CLHEP::cm;
  BoardSize.width = Boards.getLength("Width") * CLHEP::cm;
  BoardSize.height = Boards.getLength("Height") * CLHEP::cm;
  BoardSize.base_width = Boards.getLength("BaseWidth") * CLHEP::cm;
  BoardSize.base_height = Boards.getLength("BaseHeight") * CLHEP::cm;
  BoardSize.strip_length = Boards.getLength("StripLength") * CLHEP::cm;
  BoardSize.strip_width = Boards.getLength("StripWidth") * CLHEP::cm;
  BoardSize.strip_height = Boards.getLength("StripHeight") * CLHEP::cm;
  m_nStripBoard = Boards.getInt("nStripBoard");
  for (j = 0; j < nPlane; j++) {
    BoardPosition[j] = (struct EKLM::BoardPosition*)
                       malloc(nBoard * sizeof(struct EKLM::BoardPosition));
    if (BoardPosition[j] == NULL)
      B2FATAL(MemErr);
    for (i = 0; i < nBoard; i++) {
      GearDir BoardContent(Boards);
      BoardContent.append((boost::format("/BoardData[%1%]") % (j + 1)).str());
      BoardContent.append((boost::format("/Board[%1%]") % (i + 1)).str());
      BoardPosition[j][i].phi = BoardContent.getLength("Phi") * CLHEP::rad;
      BoardPosition[j][i].r = BoardContent.getLength("Radius") * CLHEP::cm;
    }
  }
  StripBoardPosition = (struct EKLM::StripBoardPosition*)
                       malloc(m_nStripBoard *
                              sizeof(struct EKLM::StripBoardPosition));
  if (StripBoardPosition == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_nStripBoard; i++) {
    GearDir StripBoardContent(Boards);
    StripBoardContent.append((boost::format("/StripBoardData/Board[%1%]") %
                              (i + 1)).str());
    StripBoardPosition[i].x = StripBoardContent.getLength("PositionX") * CLHEP::cm;
  }
  GearDir SectorSupport(Sector);
  SectorSupport.append("/SectorSupport");
  readPositionData(&SectorSupportPosition, &SectorSupport);
  readSizeData(&SectorSupportPosition, &SectorSupport);
  readSectorSupportData(&m_SectorSupportData, &SectorSupport);
  calculateSectorSupportData();
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(&PlanePosition, &Plane);
  readSizeData(&PlanePosition, &Plane);
  readXMLDataStrips();
  nSection = Plane.getInt("nSection");
  PlasticSheetWidth = Plane.getLength("PlasticSheetWidth") * CLHEP::cm;
  PlasticSheetDeltaL = Plane.getLength("PlasticSheetDeltaL") * CLHEP::cm;
  GearDir Sections(Plane);
  Sections.append("/Sections");
  SectionSupportTopWidth = Sections.getLength("TopWidth") * CLHEP::cm;
  SectionSupportTopThickness = Sections.getLength("TopThickness") * CLHEP::cm;
  SectionSupportMiddleWidth = Sections.getLength("MiddleWidth") * CLHEP::cm;
  SectionSupportMiddleThickness = Sections.getLength("MiddleThickness") * CLHEP::cm;
  for (j = 0; j < nPlane; j++) {
    SectionSupportPosition[j] =
      (struct EKLM::SectionSupportPosition*)
      malloc((nSection + 1) * sizeof(struct EKLM::SectionSupportPosition));
    if (SectionSupportPosition[j] == NULL)
      B2FATAL(MemErr);
    for (i = 0; i <= nSection; i++) {
      GearDir SectionSupportContent(Sections);
      SectionSupportContent.append((boost::format(
                                      "/SectionSupportData[%1%]") %
                                    (j + 1)).str());
      SectionSupportContent.append((boost::format(
                                      "/SectionSupport[%1%]") %
                                    (i +  1)).str());
      SectionSupportPosition[j][i].length = SectionSupportContent.
                                            getLength("Length") * CLHEP::cm;
      SectionSupportPosition[j][i].x = SectionSupportContent.
                                       getLength("PositionX") * CLHEP::cm;
      SectionSupportPosition[j][i].y = SectionSupportContent.
                                       getLength("PositionY") * CLHEP::cm;
      SectionSupportPosition[j][i].z = SectionSupportContent.
                                       getLength("PositionZ") * CLHEP::cm;
      SectionSupportPosition[j][i].deltal_right = SectionSupportContent.
                                                  getLength("DeltaLRight") *
                                                  CLHEP::cm;
      SectionSupportPosition[j][i].deltal_left = SectionSupportContent.
                                                 getLength("DeltaLLeft") * CLHEP::cm;
    }
  }
}

/****************************** TRANSFORMATIONS ******************************/

void EKLM::GeoEKLMCreator::getEndcapTransform(HepGeom::Transform3D* t, int n)
{
  if (n == 0)
    *t = HepGeom::Translate3D(EndcapPosition.X, EndcapPosition.Y,
                              -EndcapPosition.Z + m_solenoidZ);
  else
    *t = HepGeom::Translate3D(EndcapPosition.X, EndcapPosition.Y,
                              EndcapPosition.Z + m_solenoidZ) *
         HepGeom::RotateY3D(180.*CLHEP::deg);
}

void EKLM::GeoEKLMCreator::getLayerTransform(HepGeom::Transform3D* t, int n)
{
  *t = HepGeom::Translate3D(0.0, 0.0, EndcapPosition.length / 2.0 -
                            (n + 1) * Layer_shiftZ +
                            0.5 * LayerPosition.length);
}

void EKLM::GeoEKLMCreator::getSectorTransform(HepGeom::Transform3D* t, int n)
{
  switch (n) {
    case 0:
      *t = HepGeom::Translate3D(0., 0., 0.);
      break;
    case 1:
      *t = HepGeom::RotateY3D(180.0 * CLHEP::deg);
      break;
    case 2:
      *t = HepGeom::RotateZ3D(90.0 * CLHEP::deg) *
           HepGeom::RotateY3D(180.0 * CLHEP::deg);
      break;
    case 3:
      *t = HepGeom::RotateZ3D(-90.0 * CLHEP::deg);
      break;
  }
}

void EKLM::GeoEKLMCreator::getPlaneTransform(HepGeom::Transform3D* t, int n)
{
  if (n == 0)
    *t = HepGeom::Translate3D(PlanePosition.X, PlanePosition.Y,
                              PlanePosition.Z);
  else
    *t = HepGeom::Translate3D(PlanePosition.X, PlanePosition.Y,
                              -PlanePosition.Z) *
         HepGeom::Rotate3D(180. * CLHEP::deg,
                           HepGeom::Vector3D<double>(1., 1., 0.));
}

void EKLM::GeoEKLMCreator::getStripTransform(HepGeom::Transform3D* t, int n)
{
  *t = HepGeom::Translate3D(StripPosition[n].X, StripPosition[n].Y, 0.0);
}

void EKLM::GeoEKLMCreator::getSheetTransform(HepGeom::Transform3D* t, int n)
{
  double y;
  y = StripPosition[n].Y;
  if (n % 15 == 0)
    y = y + 0.5 * PlasticSheetDeltaL;
  else if (n % 15 == 14)
    y = y - 0.5 * PlasticSheetDeltaL;
  *t = HepGeom::Translate3D(StripPosition[n].X, y, 0.0);
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
  lz = 0.5 * (SectorPosition.length - SectorSupportPosition.length);
  try {
    solidCoverTube = new G4Tubs("Cover_Tube",
                                SectorSupportPosition.innerR,
                                SectorSupportPosition.outerR, 0.5 * lz, 0.0,
                                90.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCoverBox = new G4Box("Cover_Box",
                              0.5 * SectorSupportPosition.outerR,
                              0.5 * SectorSupportPosition.outerR, 0.5 * lz);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    box = new G4Box("Cover_SubtractionBox",
                    0.5 * SectorSupportPosition.outerR,
                    0.5 * SectorSupportPosition.outerR, lz);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = G4Translate3D(0.5 * PlanePosition.outerR + SectorSupportPosition.X,
                     0.5 * PlanePosition.outerR + SectorSupportPosition.Y,
                     0.);
  t2 = G4Translate3D(SectorSupportPosition.X +
                     0.5 * SectorSupportPosition.outerR *
                     cos(m_SectorSupportData.CornerAngle) -
                     0.5 * SectorSupportPosition.outerR *
                     sin(m_SectorSupportData.CornerAngle),
                     SectorSupportPosition.outerR -
                     m_SectorSupportData.DeltaLY +
                     0.5 * SectorSupportPosition.outerR *
                     cos(m_SectorSupportData.CornerAngle) +
                     0.5 * SectorSupportPosition.outerR *
                     sin(m_SectorSupportData.CornerAngle),
                     0.) * G4RotateZ3D(m_SectorSupportData.CornerAngle);
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
  x = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
           SectorSupportPosition.Y * SectorSupportPosition.Y);
  t = G4Translate3D(0.5 * (x + m_SectorSupportData.Corner3.X),
                    SectorSupportPosition.Y +
                    0.5 * m_SectorSupportData.Thickness, 0.);
  try {
    res = new G4Box("SectorSupport_BoxX",
                    0.5 * (x - m_SectorSupportData.Corner3.X),
                    0.5 * m_SectorSupportData.Thickness,
                    0.5 * SectorSupportPosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Box* EKLM::GeoEKLMCreator::createSectorSupportBoxY(G4Transform3D& t)
{
  G4Box* res = NULL;
  t = G4Translate3D(SectorSupportPosition.X +
                    0.5 * m_SectorSupportData.Thickness,
                    0.5 * (m_SectorSupportData.Corner4.Y +
                           m_SectorSupportData.Corner1A.Y), 0.) *
      G4RotateZ3D(90. * CLHEP::deg);
  try {
    res = new G4Box("SectorSupport_BoxY",
                    0.5 * (m_SectorSupportData.Corner1A.Y -
                           m_SectorSupportData.Corner4.Y),
                    0.5 * m_SectorSupportData.Thickness,
                    0.5 * SectorSupportPosition.length);
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
  t = G4Translate3D(0.5 * (m_SectorSupportData.Corner1A.X +
                           m_SectorSupportData.Corner1B.X +
                           m_SectorSupportData.Thickness *
                           sin(m_SectorSupportData.CornerAngle)),
                    0.5 * (m_SectorSupportData.Corner1A.Y +
                           m_SectorSupportData.Corner1B.Y -
                           m_SectorSupportData.Thickness *
                           cos(m_SectorSupportData.CornerAngle)),
                    0.) * G4RotateZ3D(m_SectorSupportData.CornerAngle);
  dx = m_SectorSupportData.Corner1B.X - m_SectorSupportData.Corner1A.X;
  dy = m_SectorSupportData.Corner1B.Y - m_SectorSupportData.Corner1B.Y;
  try {
    res = new G4Box("SectorSupport_BoxTop", 0.5 * sqrt(dx * dx + dy * dy),
                    0.5 * m_SectorSupportData.Thickness,
                    0.5 * SectorSupportPosition.length);
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
  ang1 = atan2(m_SectorSupportData.Corner3.Y, m_SectorSupportData.Corner3.X);
  ang2 = atan2(m_SectorSupportData.Corner4.Y, m_SectorSupportData.Corner4.X);
  try {
    res = new G4Tubs("SectorSupport_InnerTube",
                     SectorSupportPosition.innerR,
                     SectorSupportPosition.innerR +
                     m_SectorSupportData.Thickness,
                     0.5 * SectorSupportPosition.length,
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
  r = SectorSupportPosition.outerR - m_SectorSupportData.Thickness;
  x = sqrt(r * r - SectorSupportPosition.Y * SectorSupportPosition.Y);
  ang1 = atan2(SectorSupportPosition.Y, x);
  ang2 = atan2(m_SectorSupportData.Corner1B.Y, m_SectorSupportData.Corner1B.X);
  try {
    res = new G4Tubs("SectorSupport_OuterTube", r,
                     SectorSupportPosition.outerR,
                     0.5 * SectorSupportPosition.length,
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
  lx = m_SectorSupportData.CornerX + m_SectorSupportData.Corner1LX -
       m_SectorSupportData.Thickness;
  try {
    solidCorner1Tube = new G4Tubs("SectorSupport_Corner1_Tube", 0.,
                                  SectorSupportPosition.outerR -
                                  m_SectorSupportData.Thickness,
                                  0.5 * m_SectorSupportData.Corner1Thickness,
                                  0., 90. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1Box1 = new G4Box("SectorSupport_Corner1_Box1", 0.5 * lx,
                                 0.5 * SectorSupportPosition.outerR,
                                 0.5 * m_SectorSupportData.Corner1Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solidCorner1Box2 =
      new G4Box("SectorSupport_Corner1_Box2",
                0.5 * (lx / cos(m_SectorSupportData.CornerAngle) +
                       m_SectorSupportData.Corner1Width *
                       tan(m_SectorSupportData.CornerAngle)),
                0.5 * m_SectorSupportData.Corner1Width,
                0.5 * m_SectorSupportData.Corner1Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  x = SectorSupportPosition.X + 0.5 * (m_SectorSupportData.CornerX +
                                       m_SectorSupportData.Corner1LX +
                                       m_SectorSupportData.Thickness);
  t1 = G4Translate3D(x, 0.5 * SectorSupportPosition.outerR, 0.);
  t2 = G4Translate3D(x, m_SectorSupportData.Corner1AInner.Y -
                     0.5 * m_SectorSupportData.Corner1Width /
                     cos(m_SectorSupportData.CornerAngle) +
                     0.5 * lx * tan(m_SectorSupportData.CornerAngle), 0.) *
       G4RotateZ3D(m_SectorSupportData.CornerAngle);
  try {
    is1 = new G4IntersectionSolid("SectorSupport_Corner1_Intersection1",
                                  solidCorner1Tube, solidCorner1Box1, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solids.sectorsup.corn1 = new G4IntersectionSolid("SectorSupport_Corner1", is1,
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
  r = SectorSupportPosition.outerR - m_SectorSupportData.Thickness;
  solids.sectorsup.c2y = SectorSupportPosition.Y + m_SectorSupportData.Thickness;
  solids.sectorsup.c2x = sqrt(r * r - solids.sectorsup.c2y * solids.sectorsup.c2y);
  try {
    solidCorner2Prism = new G4TriangularPrism("SectorSupport_Corner2_Prism",
                                              m_SectorSupportData.Corner2LY,
                                              90. * CLHEP::deg,
                                              m_SectorSupportData.Corner2LX,
                                              180. * CLHEP::deg,
                                              0.5 * m_SectorSupportData.
                                              Corner2Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = G4Translate3D(-solids.sectorsup.c2x, -solids.sectorsup.c2y, 0.);
  try {
    solids.sectorsup.corn2 = new G4SubtractionSolid("SectorSupport_Corner2",
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
  r = SectorSupportPosition.innerR + m_SectorSupportData.Thickness;
  y = SectorSupportPosition.Y + m_SectorSupportData.Thickness +
      m_SectorSupportData.Corner3LY;
  solids.sectorsup.c3x = sqrt(r * r - y * y);
  solids.sectorsup.c3y = SectorSupportPosition.Y + m_SectorSupportData.Thickness;
  try {
    solidCorner3Prism = new G4TriangularPrism("SectorSupport_Corner3_Prism",
                                              m_SectorSupportData.Corner3LX, 0.,
                                              m_SectorSupportData.Corner3LY,
                                              90. * CLHEP::deg,
                                              0.5 * m_SectorSupportData.
                                              Corner3Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = G4Translate3D(-solids.sectorsup.c3x, -solids.sectorsup.c3y, 0.);
  try {
    solids.sectorsup.corn3 = new G4SubtractionSolid("SectorSupport_Corner3",
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
  r = SectorSupportPosition.innerR + m_SectorSupportData.Thickness;
  x = SectorSupportPosition.X + m_SectorSupportData.Thickness +
      m_SectorSupportData.Corner4LX;
  solids.sectorsup.c4y = sqrt(r * r - x * x);
  solids.sectorsup.c4x = SectorSupportPosition.X + m_SectorSupportData.Thickness;
  try {
    solidCorner4Prism = new G4TriangularPrism("SectorSupport_Corner4_Prism",
                                              m_SectorSupportData.Corner4LX, 0.,
                                              m_SectorSupportData.Corner4LY,
                                              90. * CLHEP::deg,
                                              0.5 * m_SectorSupportData.
                                              Corner4Thickness);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t1 = G4Translate3D(-solids.sectorsup.c4x, -solids.sectorsup.c4y, 0.);
  try {
    solids.sectorsup.corn4 = new G4SubtractionSolid("SectorSupport_Corner4",
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
  solidBoxX = createSectorSupportBoxX(tbx);
  solidBoxY = createSectorSupportBoxY(tby);
  solidBoxTop = createSectorSupportBoxTop(tbt);
  solids.sectorsup.otube = createSectorSupportOuterTube();
  solids.sectorsup.itube = createSectorSupportInnerTube();
  try {
    solidLimitationTube = new G4Tubs("SectorSupport_LimitationTube",
                                     0., SectorSupportPosition.outerR,
                                     0.5 * SectorSupportPosition.length,
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
subtractBoardSolids(G4SubtractionSolid* plane, int n)
{
  int i;
  int j;
  G4Transform3D t;
  G4Box* solidBoardBox = NULL;
  G4SubtractionSolid** ss[2];
  G4SubtractionSolid* prev_solid = NULL;
  G4SubtractionSolid* res = NULL;
  /* If there are no boards, it is not necessary to subtract their solids. */
  if (m_mode != EKLM_DETECTOR_BACKGROUND)
    return plane;
  /* Subtraction. */
  try {
    solidBoardBox = new G4Box("PlateBox", 0.5 * BoardSize.length,
                              0.5 * BoardSize.height,
                              0.5 * (PlanePosition.length + PlanePosition.Z));
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < 2; i++) {
    ss[i] = (G4SubtractionSolid**)malloc(sizeof(G4SubtractionSolid*) * nBoard);
    if (ss[i] == NULL)
      B2FATAL(MemErr);
    for (j = 0; j < nBoard; j++) {
      t = *BoardTransform[i][j];
      if (n == 1)
        t = G4Rotate3D(180. * CLHEP::deg, G4ThreeVector(1., 1., 0.)) * t;
      if (i == 0) {
        if (j == 0)
          prev_solid = plane;
        else
          prev_solid = ss[0][j - 1];
      } else {
        if (j == 0)
          prev_solid = ss[0][nBoard - 1];
        else
          prev_solid = ss[1][j - 1];
      }
      try {
        ss[i][j] = new G4SubtractionSolid("BoardSubtraction_" +
                                          boost::lexical_cast<std::string>(i) + "_" +
                                          boost::lexical_cast<std::string>(j),
                                          prev_solid, solidBoardBox, t);
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
    }
  }
  res = ss[1][nBoard - 1];
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
  HepGeom::Transform3D t2;
  HepGeom::Transform3D t3;
  HepGeom::Transform3D t4;
  HepGeom::Transform3D t5;
  char name[128];
  G4Tubs* tb = NULL;
  G4Box* b1 = NULL;
  G4Box* b2 = NULL;
  G4TriangularPrism* pr1 = NULL;
  G4TriangularPrism* pr2 = NULL;
  G4TriangularPrism* pr3 = NULL;
  G4IntersectionSolid* is = NULL;
  G4SubtractionSolid* ss1 = NULL;
  G4SubtractionSolid* ss2 = NULL;
  G4SubtractionSolid* ss3 = NULL;
  G4SubtractionSolid* ss4 = NULL;
  /* Basic solids. */
  snprintf(name, 128, "Plane_%d_Tube", n + 1);
  try {
    tb = new G4Tubs(name, PlanePosition.innerR, PlanePosition.outerR,
                    0.5 * PlanePosition.length, 0.0, 90.0 * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Box_1", n + 1);
  box_x = SectorSupportPosition.X + m_SectorSupportData.Thickness;
  box_y = SectorSupportPosition.Y + m_SectorSupportData.Thickness;
  box_lx =  PlanePosition.outerR;
  try {
    b1 = new G4Box(name, 0.5 * box_lx, 0.5 * box_lx,
                   0.5 * PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Triangular_Prism_1", n + 1);
  try {
    pr1 = new G4TriangularPrism(name, m_SectorSupportData.Corner2LY, 90. * CLHEP::deg,
                                m_SectorSupportData.Corner2LX,
                                180. * CLHEP::deg, PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Triangular_Prism_2", n + 1);
  try {
    pr2 = new G4TriangularPrism(name, m_SectorSupportData.Corner3LX, 0.,
                                m_SectorSupportData.Corner3LY,
                                90. * CLHEP::deg, PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Triangular_Prism_3", n + 1);
  try {
    pr3 = new G4TriangularPrism(name, m_SectorSupportData.Corner4LX, 0.,
                                m_SectorSupportData.Corner4LY,
                                90. * CLHEP::deg, PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Box_2", n + 1);
  try {
    b2 = new G4Box(name, 0.5 * box_lx, 0.5 * box_lx, PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  /* Calculate transformations for boolean solids. */
  t1 = HepGeom::Translate3D(0.5 * PlanePosition.outerR + box_x,
                            0.5 * PlanePosition.outerR + box_y, 0.);
  t2 = HepGeom::Translate3D(
         m_SectorSupportData.Corner1AInner.X +
         0.5 * box_lx * cos(m_SectorSupportData.CornerAngle) -
         0.5 * box_lx * sin(m_SectorSupportData.CornerAngle),
         m_SectorSupportData.Corner1AInner.Y +
         0.5 * box_lx * cos(m_SectorSupportData.CornerAngle) +
         0.5 * box_lx * sin(m_SectorSupportData.CornerAngle),
         0.) * HepGeom::RotateZ3D(m_SectorSupportData.CornerAngle);
  t3 = HepGeom::Translate3D(solids.sectorsup.c2x, solids.sectorsup.c2y, 0.);
  t4 = HepGeom::Translate3D(solids.sectorsup.c3x, solids.sectorsup.c3y, 0.);
  t5 = HepGeom::Translate3D(solids.sectorsup.c4x, solids.sectorsup.c4y, 0.);
  /* For rotated plane. */
  if (n == 1) {
    t = HepGeom::Rotate3D(180. * CLHEP::deg,
                          HepGeom::Vector3D<double>(1., 1., 0.));
    t1 = t * t1;
    t2 = t * t2;
    t3 = t * t3;
    t4 = t * t4;
    t5 = t * t5;
  }
  /* Boolean solids. */
  snprintf(name, 128, "Plane_%d_Intersection", n + 1);
  try {
    is = new G4IntersectionSolid(name, tb, b1, t1);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Subtraction_1", n + 1);
  try {
    ss1 = new G4SubtractionSolid(name, is, b2, t2);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Subtraction_2", n + 1);
  try {
    ss2 = new G4SubtractionSolid(name, ss1, pr1->getSolid(), t3);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Subtraction_3", n + 1);
  try {
    ss3 = new G4SubtractionSolid(name, ss2, pr2->getSolid(), t4);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Subtraction_4", n + 1);
  try {
    ss4 = new G4SubtractionSolid(name, ss3, pr3->getSolid(), t5);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d", n + 1);
  solids.plane[n] = subtractBoardSolids(ss4, n);
  delete pr1;
  delete pr2;
  delete pr3;
}

void EKLM::GeoEKLMCreator::createPlasticSheetSolid(int n)
{
  int i;
  int m;
  double ly;
  char name[128];
  G4Box* b[15];
  G4UnionSolid* u1[8];
  G4UnionSolid* u2[4];
  G4UnionSolid* u3[2];
  HepGeom::Transform3D tl[15];
  HepGeom::Transform3D tli[15];
  HepGeom::Transform3D t;
  /* Transformations. */
  for (i = 0; i < 15; i++) {
    m = 15 * n + i;
    getSheetTransform(&(tl[i]), m);
    tli[i] = tl[i].inverse();
  }
  /* Sheet elements. */
  for (i = 0; i < 15; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Element_%d", n + 1, i + 1);
    ly = StripSize.width;
    if (i == 0 || i == 14)
      ly = ly - PlasticSheetDeltaL;
    m = 15 * n + i;
    try {
      b[i] = new G4Box(name, 0.5 * StripPosition[m].length, 0.5 * ly,
                       0.5 * PlasticSheetWidth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* First level unions (7 * 2 + 1). */
  for (i = 0; i < 7; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Union1_%d", n + 1, i + 1);
    t = tli[2 * i] * tl[2 * i + 1];
    try {
      u1[i] = new G4UnionSolid(name, b[2 * i], b[2 * i + 1], t);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  u1[7] = (G4UnionSolid*)b[14];
  /* Second level unions (3 * 4 + 1 * 3). */
  for (i = 0; i < 4; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Union2_%d", n + 1, i + 1);
    t = tli[4 * i] * tl[4 * i + 2];
    try {
      u2[i] = new G4UnionSolid(name, u1[2 * i], u1[2 * i + 1], t);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* Third level unions (1 * 8 + 1 * 7). */
  for (i = 0; i < 2; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Union3_%d", n + 1, i + 1);
    t = tli[8 * i] * tl[8 * i + 4];
    try {
      u3[i] = new G4UnionSolid(name, u2[2 * i], u2[2 * i + 1], t);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* Plastic sheet. */
  snprintf(name, 128, "PlasticSheet_%d", n + 1);
  t = tli[0] * tl[8];
  try {
    solids.psheet[n] = new G4UnionSolid(name, u3[0], u3[1], t);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSolids()
{
  int i, iPos, n;
  char name[128];
  HepGeom::Transform3D t;
  /* Endcap. */
  createEndcapSolid();
  /* Layer. */
  try {
    solids.layer = new G4Tubs("Layer", LayerPosition.innerR,
                              LayerPosition.outerR, LayerPosition.length / 2.0,
                              0. * CLHEP::deg, 360. * CLHEP::deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  /* Sector. */
  try {
    solids.sector = new G4Tubs("Sector", SectorPosition.innerR,
                               SectorPosition.outerR,
                               0.5 * SectorPosition.length,
                               0.0, 90.0 * CLHEP::deg);
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
  calcBoardTransform();
  for (i = 0; i < nPlane; i++)
    createPlaneSolid(i);
  /* Strips. */
  n = m_geoDat->getNStripsDifferentLength();
  for (i = 0; i < n; i++) {
    iPos = m_geoDat->getStripPositionIndex(i);
    /* Strip volumes. */
    snprintf(name, 128, "StripVolume_%d", i + 1);
    try {
      solids.stripvol[i] =
        new G4Box(name, 0.5 * (StripPosition[iPos].length + StripSize.rss_size),
                  0.5 * StripSize.width, 0.5 * StripSize.thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strips. */
    snprintf(name, 128, "Strip_%d", i + 1);
    try {
      solids.strip[i] = new G4Box(name, 0.5 * StripPosition[iPos].length,
                                  0.5 * StripSize.width,
                                  0.5 * StripSize.thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strip grooves. */
    snprintf(name, 128, "Groove_%d", i + 1);
    try {
      solids.groove[i] = new G4Box(name, 0.5 * StripPosition[iPos].length,
                                   0.5 * StripSize.groove_width,
                                   0.5 * StripSize.groove_depth);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strip sensitive volumes (scintillator). */
    snprintf(name, 128, "StripSensitive_%d_Box", i + 1);
    try {
      solids.scint[i].box =
        new G4Box(name,
                  0.5 * StripPosition[iPos].length -
                  StripSize.no_scintillation_thickness,
                  0.5 * StripSize.width -
                  StripSize.no_scintillation_thickness,
                  0.5 * StripSize.thickness -
                  StripSize.no_scintillation_thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    snprintf(name, 128, "StripSensitive_%d", i + 1);
    t = HepGeom::Translate3D(0., 0., 0.5 * (StripSize.thickness -
                                            StripSize.groove_depth));
    try {
      solids.scint[i].sens = new G4SubtractionSolid(name, solids.scint[i].box,
                                                    solids.groove[i], t);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  /* Plastic sheet elements. */
  for (i = 0; i < nSection; i++)
    createPlasticSheetSolid(i);
  /* For background mode. */
  if (m_mode == EKLM_DETECTOR_BACKGROUND) {
    /* Readout board. */
    try {
      solids.board = new G4Box("Board", 0.5 * BoardSize.length,
                               0.5 * BoardSize.height,
                               0.5 * BoardSize.width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.baseboard = new G4Box("BaseBoard", 0.5 * BoardSize.length,
                                   0.5 * BoardSize.base_height,
                                   0.5 * BoardSize.base_width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.stripboard = new G4Box("StripBoard", 0.5 * BoardSize.strip_length,
                                    0.5 * BoardSize.strip_height,
                                    0.5 * BoardSize.strip_width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* SiPM (not really a SiPM; a cube in the place of SiPM) */
    try {
      solids.sipm = new G4Box("SiPM", 0.5 * StripSize.rss_size,
                              0.5 * StripSize.rss_size,
                              0.5 * StripSize.rss_size);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
}

/************************** CREATION OF VOLUMES ******************************/

void EKLM::GeoEKLMCreator::createEndcap(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicEndcap = NULL;
  G4Transform3D* t;
  std::string Endcap_Name = "Endcap_" +
                            boost::lexical_cast<std::string>(curvol.endcap);
  try {
    logicEndcap = new G4LogicalVolume(solids.endcap, mat.iron, Endcap_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicEndcap, true);
  geometry::setColor(*logicEndcap, "#ffffff22");
  t = &m_geoDat->transf.endcap[curvol.endcap - 1];
  try {
    new G4PVPlacement(*t, logicEndcap, Endcap_Name, mlv, false,
                      curvol.endcap, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (curvol.layer = 1; curvol.layer <= m_nLayer; curvol.layer++)
    createLayer(logicEndcap);
}

void EKLM::GeoEKLMCreator::createLayer(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicLayer = NULL;
  std::string Layer_Name = "Layer_" +
                           boost::lexical_cast<std::string>(curvol.layer) +
                           "_" + mlv->GetName();
  try {
    logicLayer = new G4LogicalVolume(solids.layer, mat.air, Layer_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicLayer, false);
  try {
    new G4PVPlacement(m_geoDat->transf.layer[curvol.endcap - 1][curvol.layer - 1],
                      logicLayer, Layer_Name, mlv, false, curvol.layer, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (curvol.sector = 1; curvol.sector <= 4; curvol.sector++)
    createSector(logicLayer);
}

void EKLM::GeoEKLMCreator::createSector(G4LogicalVolume* mlv)
{
  int i;
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
  try {
    new G4PVPlacement(m_geoDat->transf.sector[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1],
                      logicSector, Sector_Name, mlv, false,
                      curvol.sector, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createSectorSupport(logicSector);
  for (i = 1; i <= 2; i++)
    createSectorCover(i, logicSector);
  if ((curvol.endcap == 1 && curvol.layer <= m_nLayerBackward) ||
      (curvol.endcap == 2 && curvol.layer <= m_nLayerForward)) {
    /* Detector layer. */
    for (curvol.plane = 1; curvol.plane <= nPlane; curvol.plane++)
      createPlane(logicSector);
    if (m_mode == EKLM_DETECTOR_BACKGROUND)
      for (curvol.plane = 1; curvol.plane <= nPlane; curvol.plane++)
        for (curvol.board = 1; curvol.board <= nBoard; curvol.board++)
          createSectionReadoutBoard(logicSector);
  } else {
    /* Shield layer. */
  }
}

void EKLM::GeoEKLMCreator::calcBoardTransform()
{
  int i;
  int j;
  for (i = 0; i < nPlane; i++) {
    BoardTransform[i] = (G4Transform3D**)
                        malloc(sizeof(G4Transform3D*) * nBoard);
    if (BoardTransform[i] == NULL)
      B2FATAL(MemErr);
    for (j = 0; j < nBoard; j++) {
      try {
        BoardTransform[i][j] = new G4Transform3D(
          G4RotateZ3D(BoardPosition[i][j].phi) *
          G4Translate3D(BoardPosition[i][j].r -
                        0.5 * BoardSize.height, 0., 0.) *
          G4RotateZ3D(90.0 * CLHEP::deg));
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
    }
  }
}

void EKLM::GeoEKLMCreator::createSectorCover(int iCover, G4LogicalVolume* mlv)
{
  double z;
  G4LogicalVolume* logicCover = NULL;
  G4Transform3D t;
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
  z = 0.25 * (SectorPosition.length + SectorSupportPosition.length);
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
  std::string Corner1_Name = "Corner1_" + mlv->GetName();
  try {
    logicCorner1 =
      new G4LogicalVolume(solids.sectorsup.corn1, mat.duralumin, Corner1_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner1, true);
  geometry::setColor(*logicCorner1, "#ff0000ff");
  t = G4Translate3D(0., 0., m_SectorSupportData.Corner1Z);
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
                    m_SectorSupportData.Corner2Z);
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
                    m_SectorSupportData.Corner3Z);
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
                    m_SectorSupportData.Corner4Z);
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
  t = G4Translate3D(0., 0., SectorSupportPosition.Z);
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
  G4LogicalVolume* logicPlane = NULL;
  std::string Plane_Name = "Plane_" +
                           boost::lexical_cast<std::string>(curvol.plane) + "_" +
                           mlv->GetName();
  try {
    logicPlane = new G4LogicalVolume(solids.plane[curvol.plane - 1],
                                     mat.air, Plane_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicPlane, false);
  try {
    new G4PVPlacement(m_geoDat->transf.plane[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1][curvol.plane - 1],
                      logicPlane, Plane_Name, mlv, false, curvol.plane, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 1; i <= nSection + 1; i++)
    createSectionSupport(i, logicPlane);
  for (i = 1; i <= 2; i++)
    for (j = 1; j <= nSection; j++)
      createPlasticSheetElement(i, j, logicPlane);
  if (curvol.endcap == 1 && curvol.layer == 1 && curvol.sector == 1 &&
      curvol.plane == 1)
    for (i = 0; i < m_geoDat->getNStripsDifferentLength(); i++) {
      createStripLogicalVolumes(i, logicPlane);
      createStripPhysicalVolumes(i);
    }
  for (curvol.strip = 1; curvol.strip <= m_nStrip; curvol.strip++)
    createStripVolume(logicPlane);
}

void EKLM::GeoEKLMCreator::createSectionReadoutBoard(G4LogicalVolume* mlv)
{
  int i;
  G4LogicalVolume* logicSectionReadoutBoard = NULL;
  std::string Board_Name = "SectionReadoutBoard_" +
                           boost::lexical_cast<std::string>(curvol.board) +
                           "_Plane_" +
                           boost::lexical_cast<std::string>(curvol.plane) +
                           "_" +
                           mlv->GetName();
  try {
    logicSectionReadoutBoard = new G4LogicalVolume(solids.board,
                                                   mat.air, Board_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSectionReadoutBoard, false);
  try {
    new G4PVPlacement(*BoardTransform[curvol.plane - 1][curvol.board - 1],
                      logicSectionReadoutBoard, Board_Name, mlv, false,
                      (curvol.plane - 1) * 5 + curvol.board, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createBaseBoard(logicSectionReadoutBoard);
  for (i = 1; i <= m_nStripBoard; i++)
    createStripBoard(i, logicSectionReadoutBoard);
}

void EKLM::GeoEKLMCreator::createBaseBoard(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicBaseBoard = NULL;
  G4Transform3D t;
  std::string Board_Name = "BaseBoard_" + mlv->GetName();
  try {
    logicBaseBoard = new G4LogicalVolume(solids.baseboard, mat.silicon,
                                         Board_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicBaseBoard, true);
  geometry::setColor(*logicBaseBoard, "#0000ffff");
  t = G4Translate3D(0., -0.5 * BoardSize.height + 0.5 * BoardSize.base_height,
                    0.);
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
  t = G4Translate3D(-0.5 * BoardSize.length + StripBoardPosition[iBoard - 1].x,
                    -0.5 * BoardSize.height + BoardSize.base_height +
                    0.5 * BoardSize.strip_height, 0.);
  try {
    new G4PVPlacement(t, logicStripBoard, Board_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicStripBoard);
}

void EKLM::GeoEKLMCreator::createSectionSupport(int iSectionSupport,
                                                G4LogicalVolume* mlv)
{
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4LogicalVolume* logicSectionSupport = NULL;
  std::string SectionSupportName;
  SectionSupportName = "SectionSupport_" +
                       boost::lexical_cast<std::string>(iSectionSupport) +
                       "_" + mlv->GetName();
  if (solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].secsup == NULL) {
    try {
      solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].topbox =
        new G4Box("BoxTop_" + SectionSupportName,
                  0.5 * (SectionSupportPosition[curvol.plane - 1]
                         [iSectionSupport - 1].length -
                         SectionSupportPosition[curvol.plane - 1]
                         [iSectionSupport - 1].deltal_left -
                         SectionSupportPosition[curvol.plane - 1]
                         [iSectionSupport - 1].deltal_right),
                  0.5 * SectionSupportTopWidth,
                  0.5 * SectionSupportTopThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].midbox =
        new G4Box("BoxMiddle_" + SectionSupportName,
                  0.5 * (SectionSupportPosition[curvol.plane - 1]
                         [iSectionSupport - 1].length -
                         SectionSupportPosition[curvol.plane - 1]
                         [iSectionSupport - 1].deltal_left -
                         SectionSupportPosition[curvol.plane - 1]
                         [iSectionSupport - 1].deltal_right),
                  0.5 * SectionSupportMiddleWidth,
                  0.5 * SectionSupportMiddleThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].botbox =
        new G4Box("BoxBottom_" + SectionSupportName,
                  0.5 * SectionSupportPosition[curvol.plane - 1]
                  [iSectionSupport - 1].length,
                  0.5 * SectionSupportTopWidth,
                  0.5 * SectionSupportTopThickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    t1 = G4Translate3D(0., 0., 0.5 * (SectionSupportMiddleThickness +
                                      SectionSupportTopThickness));
    t2 = G4Translate3D(0.5 * (SectionSupportPosition[curvol.plane - 1]
                              [iSectionSupport - 1].deltal_right -
                              SectionSupportPosition[curvol.plane - 1]
                              [iSectionSupport - 1].deltal_left),
                       0., -0.5 * (SectionSupportMiddleThickness +
                                   SectionSupportTopThickness));
    try {
      solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].us =
        new G4UnionSolid("Union1_" + SectionSupportName,
                         solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].midbox,
                         solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].topbox,
                         t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].secsup =
        new G4UnionSolid(SectionSupportName,
                         solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].us,
                         solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].botbox,
                         t2);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  try {
    logicSectionSupport =
      new G4LogicalVolume(solids.sectionsup[curvol.plane - 1][iSectionSupport - 1].secsup,
                          mat.duralumin, SectionSupportName);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSectionSupport, true);
  geometry::setColor(*logicSectionSupport, "#ff0000ff");
  t = G4Translate3D(0.5 * (SectionSupportPosition[curvol.plane - 1]
                           [iSectionSupport - 1].deltal_left -
                           SectionSupportPosition[curvol.plane - 1]
                           [iSectionSupport - 1].deltal_right) +
                    SectionSupportPosition[curvol.plane - 1][iSectionSupport - 1].x,
                    SectionSupportPosition[curvol.plane - 1][iSectionSupport - 1].y,
                    SectionSupportPosition[curvol.plane - 1][iSectionSupport - 1].z);
  try {
    new G4PVPlacement(t, logicSectionSupport, SectionSupportName,
                      mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicSectionSupport);
}

void EKLM::GeoEKLMCreator::createPlasticSheetElement(int iSheetPlane, int iSheet,
                                                     G4LogicalVolume* mlv)
{
  double z;
  G4Transform3D t;
  std::string Sheet_Name = "Sheet_" +
                           boost::lexical_cast<std::string>(iSheet) +
                           "_SheetPlane_" +
                           boost::lexical_cast<std::string>(iSheetPlane) +
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
  z = 0.5 * (StripSize.thickness + PlasticSheetWidth);
  if (iSheetPlane == 2)
    z = -z;
  getSheetTransform(&t, (iSheet - 1) * 15);
  t = t * G4Translate3D(0, 0, z);
  try {
    new G4PVPlacement(t, logvol.psheet[iSheet - 1], Sheet_Name,
                      mlv, false, (iSheetPlane - 1) * nSection + iSheet, false);
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
  t = G4Translate3D(-0.5 * StripSize.rss_size, 0., 0.);
  lvm = logvol.stripvol[iStrip];
  lv = logvol.strip[iStrip];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), lvm, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  t = G4Translate3D(0., 0.,
                    0.5 * (StripSize.thickness - StripSize.groove_depth));
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
  if (m_mode == EKLM_DETECTOR_BACKGROUND)
    createSiPM(logvol.stripvol[iStrip]);
}

void EKLM::GeoEKLMCreator::createStripVolume(G4LogicalVolume* mlv)
{
  int n;
  G4Transform3D t;
  G4LogicalVolume* lv;
  n = m_geoDat->getStripLengthIndex(curvol.strip - 1);
  t = m_geoDat->transf.strip[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1][curvol.plane - 1][curvol.strip - 1] *
      G4Translate3D(0.5 * StripSize.rss_size, 0.0, 0.0);
  lv = logvol.stripvol[n];
  try {
    new G4PVPlacement(t, lv, lv->GetName(), mlv, false, curvol.strip, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMCreator::createSiPM(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicSiPM = NULL;
  G4Transform3D t;
  std::string SiPM_Name = "SiPM_" + mlv->GetName();
  try {
    logicSiPM = new G4LogicalVolume(solids.sipm, mat.silicon, SiPM_Name, 0,
                                    m_sensitive[1], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSiPM, true);
  geometry::setColor(*logicSiPM, "#0000ffff");
  t = G4Translate3D(0.5 * StripPosition[curvol.strip - 1].length, 0., 0.);
  try {
    new G4PVPlacement(t, logicSiPM, SiPM_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicSiPM);
}

void EKLM::GeoEKLMCreator::printVolumeMass(G4LogicalVolume* lv)
{
  if (m_mode == EKLM_DETECTOR_PRINTMASSES)
    printf("Volume %s: mass = %g g\n", lv->GetName().c_str(),
           lv->GetMass() / CLHEP::g);
}

void EKLM::GeoEKLMCreator::create(const GearDir& content,
                                  G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes type)
{
  (void)content;
  (void)type;
  try {
    m_sensitive[0] =
      new EKLMSensitiveDetector("EKLMSensitiveStrip",
                                EKLM_SENSITIVE_STRIP);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  if (m_mode == EKLM_DETECTOR_BACKGROUND) {
    try {
      m_sensitive[1] =
        new EKLMSensitiveDetector("EKLMSensitiveSiPM",
                                  EKLM_SENSITIVE_SIPM);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      m_sensitive[2] =
        new EKLMSensitiveDetector("EKLMSensitiveBoard",
                                  EKLM_SENSITIVE_BOARD);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  createMaterials();
  mallocVolumes();
  createSolids();
  for (curvol.endcap = 1; curvol.endcap <= 2; curvol.endcap++)
    createEndcap(&topVolume);
  if (m_mode == EKLM_DETECTOR_PRINTMASSES) {
    printf("EKLM started in mode EKLM_DETECTOR_PRINTMASSES. Exiting now.\n");
    exit(0);
  }
  haveGeo = true;
}

