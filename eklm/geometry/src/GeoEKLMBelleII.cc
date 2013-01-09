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
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4ReflectedSolid.hh>

/* Belle2 headers. */
#include <eklm/geometry/G4TriangularPrism.h>
#include <eklm/geometry/GeoEKLMBelleII.h>
#include <eklm/simulation/EKLMSensitiveDetector.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

/* Register the creator */
geometry::CreatorFactory<EKLM::GeoEKLMBelleII> GeoEKLMFactory("EKLMBelleII");

/******************************* CONSTRUCTORS ********************************/

void EKLM::GeoEKLMBelleII::constructor(bool geo)
{
  SectorSupportSize.CornerAngle = -1;
  haveGeoDat = false;
  haveGeo = false;
  transf = NULL;
  if (geo) {
    try {
      transf = new EKLM::TransformData;
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    if (readTransforms(transf) != 0)
      B2FATAL("Cannot read transformation data file.");
  }
}

EKLM::GeoEKLMBelleII::GeoEKLMBelleII()
{
  constructor(true);
}

EKLM::GeoEKLMBelleII::GeoEKLMBelleII(bool geo)
{
  constructor(geo);
}

EKLM::GeoEKLMBelleII::~GeoEKLMBelleII()
{
  int i, j;
  if (transf != NULL)
    delete transf;
  if (haveGeoDat) {
    for (i = 0; i < nPlane; i++) {
      free(BoardPosition[i]);
      free(SectionSupportPosition[i]);
    }
    free(StripPosition);
    free(StripBoardPosition);
    free(ESTRPar.z);
    free(ESTRPar.rmin);
    free(ESTRPar.rmax);
  }
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

void EKLM::GeoEKLMBelleII::mallocVolumes()
{
  int i;
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
  solids.stripvol = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.stripvol == NULL)
    B2FATAL(MemErr);
  logvol.stripvol =
    (G4LogicalVolume**)malloc(nStrip * sizeof(G4LogicalVolume*));
  if (logvol.stripvol == NULL)
    B2FATAL(MemErr);
  solids.strip = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.strip == NULL)
    B2FATAL(MemErr);
  solids.groove = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.groove == NULL)
    B2FATAL(MemErr);
  solids.scint = (struct EKLM::ScintillatorSolids*)
                 malloc(nStrip * sizeof(struct EKLM::ScintillatorSolids));
  if (solids.scint == NULL)
    B2FATAL(MemErr);
  solids.secsup = (struct EKLM::SectionSupportSolids**)
                  malloc(nPlane * sizeof(struct EKLM::SectionSupportSolids*));
  if (solids.secsup == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < nPlane; i++) {
    solids.secsup[i] = (struct EKLM::SectionSupportSolids*)
                       malloc((nSection + 1) *
                              sizeof(struct EKLM::SectionSupportSolids));
    if (solids.secsup[i] == NULL)
      B2FATAL(MemErr);
  }
  for (i = 0; i < nStrip; i++)
    logvol.stripvol[i] = NULL;
  for (i = 0; i < nSection; i++)
    logvol.psheet[i] = NULL;
  for (i = 0; i < nPlane; i++)
    memset(solids.secsup[i], 0,
           (nSection + 1) * sizeof(struct EKLM::SectionSupportSolids));
}

void EKLM::GeoEKLMBelleII::freeVolumes()
{
  int i;
  free(solids.plane);
  free(solids.psheet);
  free(logvol.psheet);
  free(solids.stripvol);
  free(logvol.stripvol);
  free(solids.strip);
  free(solids.groove);
  free(solids.scint);
  for (i = 0; i < nPlane; i++)
    free(solids.secsup[i]);
  free(solids.secsup);
}

/********************************** XML DATA *********************************/

void EKLM::GeoEKLMBelleII::createMaterials()
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
  epos->innerR = gd->getLength("InnerR") * cm;
  epos->outerR = gd->getLength("OuterR") * cm;
  epos->length = gd->getLength("Length") * cm;
  epos->X = gd->getLength("PositionX") * cm;
  epos->Y = gd->getLength("PositionY") * cm;
  epos->Z = gd->getLength("PositionZ") * cm;
}

/**
 * Read sector support size data.
 * @param sss Sector support size data.
 * @param gd  XML data directory.
 */
static void readSectorSupportData(struct EKLM::SectorSupportSize* sss,
                                  GearDir* gd)
{
  sss->Thickness = gd->getLength("Thickness") * cm;
  sss->DeltaLY = gd->getLength("DeltaLY") * cm;
  sss->CornerX = gd->getLength("CornerX") * cm;
  sss->TopCornerHeight = gd->getLength("TopCornerHeight") * cm;
  sss->Corner1LX = gd->getLength("Corner1LX") * cm;
  sss->Corner1Width = gd->getLength("Corner1Width") * cm;
  sss->Corner1Thickness = gd->getLength("Corner1Thickness") * cm;
  sss->Corner1Z = gd->getLength("Corner1Z") * cm;
  sss->Corner2LX = gd->getLength("Corner2LX") * cm;
  sss->Corner2LY = gd->getLength("Corner2LY") * cm;
  sss->Corner2Thickness = gd->getLength("Corner2Thickness") * cm;
  sss->Corner2Z = gd->getLength("Corner2Z") * cm;
  sss->Corner3LX = gd->getLength("Corner3LX") * cm;
  sss->Corner3LY = gd->getLength("Corner3LY") * cm;
  sss->Corner3Thickness = gd->getLength("Corner3Thickness") * cm;
  sss->Corner3Z = gd->getLength("Corner3Z") * cm;
  sss->Corner4LX = gd->getLength("Corner4LX") * cm;
  sss->Corner4LY = gd->getLength("Corner4LY") * cm;
  sss->Corner4Thickness = gd->getLength("Corner4Thickness") * cm;
  sss->Corner4Z = gd->getLength("Corner4Z") * cm;
}

void EKLM::GeoEKLMBelleII::readXMLData()
{
  int i;
  int j;
  if (readESTRData(&ESTRPar) == ENOMEM)
    B2FATAL(MemErr);
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
  m_mode = (enum EKLMDetectorMode)gd.getInt("Mode");
  if (m_mode < 0 || m_mode > 2)
    B2FATAL("EKLM started with unknown geometry mode " << m_mode << ".");
  GearDir EndCap(gd);
  EndCap.append("/EndCap");
  readPositionData(&EndcapPosition, &EndCap);
  EndcapPosition.Z = EndcapPosition.Z + EndcapPosition.length / 2.0;
  nLayer = EndCap.getInt("nLayer");
  GearDir Layer(EndCap);
  Layer.append("/Layer");
  readPositionData(&LayerPosition, &Layer);
  Layer_shiftZ = Layer.getLength("ShiftZ") * cm;
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readPositionData(&SectorPosition, &Sector);
  nPlane = Sector.getInt("nPlane");
  nBoard = Sector.getInt("nBoard");
  GearDir Boards(Sector);
  Boards.append("/Boards");
  BoardSize.length = Boards.getLength("Length") * cm;
  BoardSize.width = Boards.getLength("Width") * cm;
  BoardSize.height = Boards.getLength("Height") * cm;
  BoardSize.base_width = Boards.getLength("BaseWidth") * cm;
  BoardSize.base_height = Boards.getLength("BaseHeight") * cm;
  BoardSize.strip_length = Boards.getLength("StripLength") * cm;
  BoardSize.strip_width = Boards.getLength("StripWidth") * cm;
  BoardSize.strip_height = Boards.getLength("StripHeight") * cm;
  nStripBoard = Boards.getInt("nStripBoard");
  for (j = 0; j < nPlane; j++) {
    BoardPosition[j] = (struct EKLM::BoardPosition*)
                       malloc(nBoard * sizeof(struct EKLM::BoardPosition));
    if (BoardPosition[j] == NULL)
      B2FATAL(MemErr);
    for (i = 0; i < nBoard; i++) {
      GearDir BoardContent(Boards);
      BoardContent.append((boost::format("/BoardData[%1%]") % (j + 1)).str());
      BoardContent.append((boost::format("/Board[%1%]") % (i + 1)).str());
      BoardPosition[j][i].phi = BoardContent.getLength("Phi") * rad;
      BoardPosition[j][i].r = BoardContent.getLength("Radius") * cm;
    }
  }
  StripBoardPosition = (struct EKLM::StripBoardPosition*)
                       malloc(nStripBoard *
                              sizeof(struct EKLM::StripBoardPosition));
  if (StripBoardPosition == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < nStripBoard; i++) {
    GearDir StripBoardContent(Boards);
    StripBoardContent.append((boost::format("/StripBoardData/Board[%1%]") %
                              (i + 1)).str());
    StripBoardPosition[i].x = StripBoardContent.getLength("PositionX") * cm;
  }
  GearDir SectorSupport(Sector);
  SectorSupport.append("/SectorSupport");
  readPositionData(&SectorSupportPosition, &SectorSupport);
  readSectorSupportData(&SectorSupportSize, &SectorSupport);
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(&PlanePosition, &Plane);
  nSection = Plane.getInt("nSection");
  PlasticSheetWidth = Plane.getLength("PlasticSheetWidth") * cm;
  PlasticSheetDeltaL = Plane.getLength("PlasticSheetDeltaL") * cm;
  GearDir Strips(Plane);
  Strips.append("/Strips");
  nStrip = Strips.getNumberNodes("Strip");
  StripSize.width  = Strips.getLength("Width") * cm;
  StripSize.thickness = Strips.getLength("Thickness") * cm;
  StripSize.groove_depth = Strips.getLength("GrooveDepth") * cm;
  StripSize.groove_width = Strips.getLength("GrooveWidth") * cm;
  StripSize.no_scintillation_thickness = Strips.
                                         getLength("NoScintillationThickness")
                                         * cm;
  StripSize.rss_size = Strips.getLength("RSSSize") * cm;
  StripPosition = (struct EKLM::ElementPosition*)
                  malloc(nStrip * sizeof(struct EKLM::ElementPosition));
  if (StripPosition == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < nStrip; i++) {
    GearDir StripContent(Strips);
    StripContent.append((boost::format("/Strip[%1%]") % (i + 1)).str());
    StripPosition[i].length = StripContent.getLength("Length") * cm;
    StripPosition[i].X = StripContent.getLength("PositionX") * cm;
    StripPosition[i].Y = StripContent.getLength("PositionY") * cm;
    StripPosition[i].Z = StripContent.getLength("PositionZ") * cm;
  }
  GearDir Sections(Plane);
  Sections.append("/Sections");
  SectionSupportTopWidth = Sections.getLength("TopWidth") * cm;
  SectionSupportTopThickness = Sections.getLength("TopThickness") * cm;
  SectionSupportMiddleWidth = Sections.getLength("MiddleWidth") * cm;
  SectionSupportMiddleThickness = Sections.getLength("MiddleThickness") * cm;
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
                                            getLength("Length") * cm;
      SectionSupportPosition[j][i].x = SectionSupportContent.
                                       getLength("PositionX") * cm;
      SectionSupportPosition[j][i].y = SectionSupportContent.
                                       getLength("PositionY") * cm;
      SectionSupportPosition[j][i].z = SectionSupportContent.
                                       getLength("PositionZ") * cm;
      SectionSupportPosition[j][i].deltal_right = SectionSupportContent.
                                                  getLength("DeltaLRight") *
                                                  cm;
      SectionSupportPosition[j][i].deltal_left = SectionSupportContent.
                                                 getLength("DeltaLLeft") * cm;
    }
  }
  haveGeoDat = true;
}

/****************************** TRANSFORMATIONS ******************************/

void EKLM::GeoEKLMBelleII::getEndcapTransform(HepGeom::Transform3D* t, int n)
{
  if (!haveGeoDat)
    readXMLData();
  if (n == 0)
    *t = HepGeom::Translate3D(EndcapPosition.X, EndcapPosition.Y,
                              -EndcapPosition.Z + 94.0 * CLHEP::cm);
  else
    *t = HepGeom::Translate3D(EndcapPosition.X, EndcapPosition.Y,
                              EndcapPosition.Z) *
         HepGeom::RotateY3D(180.*CLHEP::deg);
}

void EKLM::GeoEKLMBelleII::getLayerTransform(HepGeom::Transform3D* t, int n)
{
  if (!haveGeoDat)
    readXMLData();
  *t = HepGeom::Translate3D(0.0, 0.0, EndcapPosition.length / 2.0 -
                            (n + 1) * Layer_shiftZ +
                            0.5 * LayerPosition.length);
}

void EKLM::GeoEKLMBelleII::getSectorTransform(HepGeom::Transform3D* t, int n)
{
  if (!haveGeoDat)
    readXMLData();
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

void EKLM::GeoEKLMBelleII::getPlaneTransform(HepGeom::Transform3D* t, int n)
{
  if (!haveGeoDat)
    readXMLData();
  if (n == 0)
    *t = HepGeom::Translate3D(PlanePosition.X, PlanePosition.Y,
                              PlanePosition.Z);
  else
    *t = HepGeom::Translate3D(PlanePosition.X, PlanePosition.Y,
                              -PlanePosition.Z) *
         HepGeom::Rotate3D(180. * CLHEP::deg,
                           HepGeom::Vector3D<double>(1., 1., 0.));
}

void EKLM::GeoEKLMBelleII::getStripTransform(HepGeom::Transform3D* t, int n)
{
  if (!haveGeoDat)
    readXMLData();
  *t = HepGeom::Translate3D(StripPosition[n].X, StripPosition[n].Y, 0.0);
}

void EKLM::GeoEKLMBelleII::getSheetTransform(HepGeom::Transform3D* t, int n)
{
  double y;
  if (!haveGeoDat)
    readXMLData();
  y = StripPosition[n].Y;
  if (n % 15 == 0)
    y = y + 0.5 * PlasticSheetDeltaL;
  else if (n % 15 == 14)
    y = y - 0.5 * PlasticSheetDeltaL;
  *t = HepGeom::Translate3D(StripPosition[n].X, y, 0.0);
}

/*************************** CREATION OF SOLIDS ******************************/

void EKLM::GeoEKLMBelleII::createEndcapSolid()
{
  G4Polyhedra* op;
  G4Tubs* tb;
  try {
    op = new G4Polyhedra("Endcap_Octagonal_Prism", ESTRPar.phi, ESTRPar.dphi,
                         ESTRPar.nsides, ESTRPar.nboundary, ESTRPar.z,
                         ESTRPar.rmin, ESTRPar.rmax);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    tb = new G4Tubs("Endcap_Tube",  ESTRPar.rminsub, ESTRPar.rmaxsub,
                    ESTRPar.zsub, 0.0, 360.0 * deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  try {
    solids.endcap = new G4SubtractionSolid("Endcap", op, tb);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMBelleII::createPlaneSolid(int n)
{
  double r;
  double x;
  double y;
  double box_x;
  double box_lx;
  double ang;
  HepGeom::Transform3D t1;
  HepGeom::Transform3D t2;
  HepGeom::Transform3D t3;
  HepGeom::Transform3D t4;
  HepGeom::Transform3D t5;
  char name[128];
  G4Tubs* tb;
  G4Box* b1;
  G4Box* b2;
  G4TriangularPrism* pr1;
  G4TriangularPrism* pr2;
  G4TriangularPrism* pr3;
  G4IntersectionSolid* is;
  G4SubtractionSolid* ss1;
  G4SubtractionSolid* ss2;
  G4SubtractionSolid* ss3;
  G4SubtractionSolid* ss4;
  /* Basic solids. */
  snprintf(name, 128, "Plane_%d_Tube", n + 1);
  try {
    tb = new G4Tubs(name, PlanePosition.innerR, PlanePosition.outerR,
                    0.5 * PlanePosition.length, 0.0, 90.0 * deg);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Box_1", n + 1);
  box_x = std::max(SectorSupportPosition.Y, SectorSupportPosition.X) +
          SectorSupportSize.Thickness;
  box_lx =  PlanePosition.outerR - box_x;
  try {
    b1 = new G4Box(name, 0.5 * box_lx, 0.5 * box_lx,
                   0.5 * PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Triangular_Prism_1", n + 1);
  try {
    pr1 = new G4TriangularPrism(name, SectorSupportSize.Corner2LY, 90. * deg,
                                SectorSupportSize.Corner2LX,
                                180. * deg, PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Triangular_Prism_2", n + 1);
  try {
    pr2 = new G4TriangularPrism(name, SectorSupportSize.Corner3LX, 0.,
                                SectorSupportSize.Corner3LY,
                                90. * deg, PlanePosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  snprintf(name, 128, "Plane_%d_Triangular_Prism_3", n + 1);
  try {
    pr3 = new G4TriangularPrism(name, SectorSupportSize.Corner4LX, 0.,
                                SectorSupportSize.Corner4LY,
                                90. * deg, PlanePosition.length);
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
  t1 = HepGeom::Translate3D(0.5 * (PlanePosition.outerR + box_x),
                            0.5 * (PlanePosition.outerR + box_x), 0.);
  if (n == 1)
    t1 = HepGeom::Rotate3D(180. * deg,
                           HepGeom::Vector3D<double>(1., 1., 0.)) * t1;
  ang = getSectorSupportCornerAngle();
  x = std::max(SectorSupportPosition.Y, SectorSupportPosition.X);
  y = SectorSupportPosition.outerR -
      SectorSupportSize.DeltaLY -
      SectorSupportSize.TopCornerHeight;
  if (n == 0) {
    t2 = HepGeom::Translate3D(x + 0.5 * box_lx * cos(ang) -
                              0.5 * box_lx * sin(ang),
                              y + 0.5 * box_lx * cos(ang) +
                              0.5 * box_lx * sin(ang),
                              0.) * HepGeom::RotateZ3D(ang);
  } else {
    t2 = HepGeom::Translate3D(y + 0.5 * box_lx * cos(ang) +
                              0.5 * box_lx * sin(ang),
                              x + 0.5 * box_lx * cos(ang) -
                              0.5 * box_lx * sin(ang),
                              0.) * HepGeom::RotateZ3D(-ang);
  }
  r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
  x = sqrt(r * r - y * y);
  t3 = HepGeom::Translate3D(x, y, 0.);
  if (n == 1)
    t3 = HepGeom::Rotate3D(180. * deg,
                           HepGeom::Vector3D<double>(1., 1., 0.)) * t3;
  r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
      SectorSupportSize.Corner3LY;
  x = sqrt(r * r - y * y);
  y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
  t4 = HepGeom::Translate3D(x, y, 0.);
  if (n == 1)
    t4 = HepGeom::Rotate3D(180. * deg,
                           HepGeom::Vector3D<double>(1., 1., 0.)) * t4;
  x = SectorSupportPosition.X + SectorSupportSize.Thickness +
      SectorSupportSize.Corner4LX;
  y = sqrt(r * r - x * x);
  x = SectorSupportPosition.X + SectorSupportSize.Thickness;
  t5 = HepGeom::Translate3D(x, y, 0.);
  if (n == 1)
    t5 = HepGeom::Rotate3D(180. * deg,
                           HepGeom::Vector3D<double>(1., 1., 0.)) * t5;
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

void EKLM::GeoEKLMBelleII::createPlasticSheetSolid(int n)
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

void EKLM::GeoEKLMBelleII::createSolids()
{
  int i;
  char name[128];
  HepGeom::Transform3D t;
  /* High level volumes. */
  createEndcapSolid();
  calcBoardTransform();
  for (i = 0; i < nPlane; i++)
    createPlaneSolid(i);
  /* Strips. */
  for (i = 0; i < nStrip; i++) {
    /* Strip volumes. */
    snprintf(name, 128, "StripVolume_%d", i + 1);
    try {
      solids.stripvol[i] =
        new G4Box(name, 0.5 * (StripPosition[i].length + StripSize.rss_size),
                  0.5 * StripSize.width, 0.5 * StripSize.thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strips. */
    snprintf(name, 128, "Strip_%d", i + 1);
    try {
      solids.strip[i] = new G4Box(name, 0.5 * StripPosition[i].length,
                                  0.5 * StripSize.width,
                                  0.5 * StripSize.thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    /* Strip grooves. */
    snprintf(name, 128, "Groove_%d", i + 1);
    try {
      solids.groove[i] = new G4Box(name, 0.5 * StripPosition[i].length,
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
                  0.5 * StripPosition[i].length -
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
  /* SiPM (not really a SiPM; a cube in the place of SiPM) */
  if (m_mode != EKLM_DETECTOR_NORMAL) {
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

void EKLM::GeoEKLMBelleII::createEndcap(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicEndcap;
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
  t = &transf->endcap[curvol.endcap - 1];
  try {
    new G4PVPlacement(*t, logicEndcap, Endcap_Name, mlv, false,
                      curvol.endcap, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (curvol.layer = 1; curvol.layer <= nLayer; curvol.layer++)
    createLayer(logicEndcap);
}

void EKLM::GeoEKLMBelleII::createLayer(G4LogicalVolume* mlv)
{
  static G4Tubs* solidLayer = NULL;
  G4LogicalVolume* logicLayer;
  std::string Layer_Name = "Layer_" +
                           boost::lexical_cast<std::string>(curvol.layer) +
                           "_" + mlv->GetName();
  if (solidLayer == NULL)
    try {
      solidLayer = new G4Tubs(Layer_Name, LayerPosition.innerR,
                              LayerPosition.outerR, LayerPosition.length / 2.0,
                              0. * deg, 360. * deg);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  try {
    logicLayer = new G4LogicalVolume(solidLayer, mat.air, Layer_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicLayer, false);
  try {
    new G4PVPlacement(transf->layer[curvol.endcap - 1][curvol.layer - 1],
                      logicLayer, Layer_Name, mlv, false, curvol.layer, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (curvol.sector = 1; curvol.sector <= 4; curvol.sector++)
    createSector(logicLayer);
}

void EKLM::GeoEKLMBelleII::createSector(G4LogicalVolume* mlv)
{
  int i;
  static G4Tubs* solidSector = NULL;
  G4LogicalVolume* logicSector;
  std::string Sector_Name = "Sector_" +
                            boost::lexical_cast<std::string>(curvol.sector) +
                            "_" + mlv->GetName();
  if (solidSector == NULL)
    try {
      solidSector = new G4Tubs(Sector_Name, SectorPosition.innerR,
                               SectorPosition.outerR,
                               0.5 * SectorPosition.length,
                               0.0, 90.0 * deg);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  try {
    logicSector = new G4LogicalVolume(solidSector, mat.air, Sector_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicSector, false);
  try {
    new G4PVPlacement(transf->sector[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1],
                      logicSector, Sector_Name, mlv, false,
                      curvol.sector, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  createSectorSupport(logicSector);
  for (i = 1; i <= 2; i++)
    createSectorCover(i, logicSector);
  for (curvol.plane = 1; curvol.plane <= nPlane; curvol.plane++)
    createPlane(logicSector);
  if (m_mode == EKLM_DETECTOR_BACKGROUND)
    for (curvol.plane = 1; curvol.plane <= nPlane; curvol.plane++)
      for (curvol.board = 1; curvol.board <= nBoard; curvol.board++)
        createSectionReadoutBoard(logicSector);
}

void EKLM::GeoEKLMBelleII::calcBoardTransform()
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
          G4RotateZ3D(90.0 * deg));
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
    }
  }
}

void EKLM::GeoEKLMBelleII::createSectorCover(int iCover, G4LogicalVolume* mlv)
{
  double z;
  double lz;
  double ang;
  static G4Tubs* solidCoverTube;
  static G4Box* solidCoverBox;
  static G4Box* box;
  static G4IntersectionSolid* is;
  static G4SubtractionSolid* solidCover = NULL;
  G4LogicalVolume* logicCover;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t;
  std::string Cover_Name = "Cover_" +
                           boost::lexical_cast<std::string>(iCover) + "_" +
                           mlv->GetName();
  if (solidCover == NULL) {
    lz = 0.5 * (SectorPosition.length - SectorSupportPosition.length);
    try {
      solidCoverTube = new G4Tubs("Tube_" + Cover_Name,
                                  SectorSupportPosition.innerR,
                                  SectorSupportPosition.outerR, 0.5 * lz, 0.0,
                                  90.0 * deg);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solidCoverBox = new G4Box("Box_" + Cover_Name,
                                0.5 * SectorSupportPosition.outerR,
                                0.5 * SectorSupportPosition.outerR, 0.5 * lz);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      box = new G4Box("SubtractionBox_" + Cover_Name,
                      0.5 * SectorSupportPosition.outerR,
                      0.5 * SectorSupportPosition.outerR, lz);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    ang = getSectorSupportCornerAngle();
    t1 = G4Translate3D(0.5 * PlanePosition.outerR + SectorSupportPosition.X,
                       0.5 * PlanePosition.outerR + SectorSupportPosition.Y,
                       0.);
    t2 = G4Translate3D(SectorSupportPosition.X +
                       0.5 * SectorSupportPosition.outerR * cos(ang) -
                       0.5 * SectorSupportPosition.outerR * sin(ang),
                       SectorSupportPosition.outerR - SectorSupportSize.DeltaLY +
                       0.5 * SectorSupportPosition.outerR * cos(ang) +
                       0.5 * SectorSupportPosition.outerR * sin(ang),
                       0.) * G4RotateZ3D(ang);
    try {
      is = new G4IntersectionSolid("Intersection_" + Cover_Name,
                                   solidCoverTube, solidCoverBox, t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solidCover = new G4SubtractionSolid(Cover_Name, is, box, t2);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  try {
    logicCover = new G4LogicalVolume(solidCover, mat.duralumin, Cover_Name);
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

G4Box* EKLM::GeoEKLMBelleII::createSectorSupportBoxX(G4LogicalVolume* mlv,
                                                     G4Transform3D& t)
{
  double x1;
  double x2;
  G4Box* res;
  x1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  x2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  t =  G4Translate3D(0.5 * (x1 + x2), SectorSupportPosition.Y +
                     0.5 * SectorSupportSize.Thickness, 0.);
  try {
    res = new G4Box("BoxX_Support_" + mlv->GetName(),
                    0.5 * (x2 - x1),
                    0.5 * SectorSupportSize.Thickness,
                    0.5 * SectorSupportPosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Box* EKLM::GeoEKLMBelleII::createSectorSupportBoxY(G4LogicalVolume* mlv,
                                                     G4Transform3D& t)
{
  double y1;
  double y2;
  G4Box* res;
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  y2 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  t = G4Translate3D(SectorSupportPosition.X + 0.5 * SectorSupportSize.Thickness,
                    0.5 * (y1 + y2), 0.) * G4RotateZ3D(90. * deg);
  try {
    res = new G4Box("BoxY_Support_" + mlv->GetName(),
                    0.5 * (y2 - y1),
                    0.5 * SectorSupportSize.Thickness,
                    0.5 * SectorSupportPosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

double EKLM::GeoEKLMBelleII::getSectorSupportCornerAngle()
{
  double x1;
  double y1;
  double x2;
  double y2;
  if (SectorSupportSize.CornerAngle < 0) {
    x1 = SectorSupportPosition.X;
    y1 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
    x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
    y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
              x2 * x2);
    SectorSupportSize.CornerAngle = atan2(y2 - y1, x2 - x1) * rad;
  }
  return SectorSupportSize.CornerAngle;
}

G4Box* EKLM::GeoEKLMBelleII::createSectorSupportBoxTop(G4LogicalVolume* mlv,
                                                       G4Transform3D& t)
{
  double x1;
  double y1;
  double x2;
  double y2;
  double ang;
  G4Box* res;
  x1 = SectorSupportPosition.X;
  y1 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang = getSectorSupportCornerAngle();
  t = G4Translate3D(0.5 * (x1 + x2 + SectorSupportSize.Thickness * sin(ang)),
                    0.5 * (y1 + y2 - SectorSupportSize.Thickness * cos(ang)),
                    0.) * G4RotateZ3D(ang);
  try {
    res = new G4Box("BoxTop_Support_" + mlv->GetName(),
                    0.5 * sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)),
                    0.5 * SectorSupportSize.Thickness,
                    0.5 * SectorSupportPosition.length);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Tubs* EKLM::GeoEKLMBelleII::createSectorSupportInnerTube(G4LogicalVolume* mlv)
{
  double x1;
  double y1;
  double ang1;
  double ang2;
  G4Tubs* res;
  x1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  ang1 = atan2(SectorSupportPosition.Y, x1);
  ang2 = atan2(y1, SectorSupportPosition.X);
  try {
    res = new G4Tubs("InnerTube_Support_" + mlv->GetName(),
                     SectorSupportPosition.innerR,
                     SectorSupportPosition.innerR +
                     SectorSupportSize.Thickness,
                     0.5 * SectorSupportPosition.length,
                     std::min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

G4Tubs* EKLM::GeoEKLMBelleII::createSectorSupportOuterTube(G4LogicalVolume* mlv)
{
  double x1;
  double y1;
  double x2;
  double y2;
  double ang1;
  double ang2;
  double r;
  G4Tubs* res;
  r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
  x1 = sqrt(r * r - SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = SectorSupportPosition.Y;
  x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang1 = atan2(y1, x1);
  ang2 = atan2(y2, x2);
  try {
    res = new G4Tubs("OuterTube_Support" + mlv->GetName(),
                     SectorSupportPosition.outerR -
                     SectorSupportSize.Thickness,
                     SectorSupportPosition.outerR,
                     0.5 * SectorSupportPosition.length,
                     std::min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  return res;
}

void EKLM::GeoEKLMBelleII::createSectorSupportCorner1(G4LogicalVolume* mlv)
{
  double lx;
  double x;
  static G4Tubs* solidCorner1Tube;
  static G4Box* solidCorner1Box1;
  static G4Box* solidCorner1Box2;
  static G4IntersectionSolid* is1;
  static G4IntersectionSolid* solidCorner1 = NULL;
  G4LogicalVolume* logicCorner1;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  std::string Corner1_Name = "Corner1_" + mlv->GetName();
  if (solidCorner1 == NULL) {
    lx = SectorSupportSize.CornerX + SectorSupportSize.Corner1LX -
         SectorSupportSize.Thickness;
    try {
      solidCorner1Tube = new G4Tubs("Tube_" + Corner1_Name, 0.,
                                    SectorSupportPosition.outerR -
                                    SectorSupportSize.Thickness,
                                    0.5 * SectorSupportSize.Corner1Thickness,
                                    0., 90. * deg);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solidCorner1Box1 = new G4Box("Box1_" + Corner1_Name, 0.5 * lx,
                                   0.5 * SectorSupportPosition.outerR,
                                   0.5 * SectorSupportSize.Corner1Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solidCorner1Box2 =
        new G4Box("Box2_" + Corner1_Name,
                  0.5 * (lx / cos(getSectorSupportCornerAngle()) +
                         SectorSupportSize.Corner1Width *
                         sin(getSectorSupportCornerAngle())),
                  0.5 * SectorSupportSize.Corner1Width,
                  0.5 * SectorSupportSize.Corner1Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    x = SectorSupportPosition.X + 0.5 * (SectorSupportSize.CornerX +
                                         SectorSupportSize.Corner1LX +
                                         SectorSupportSize.Thickness);
    t1 = G4Translate3D(x, 0.5 * SectorSupportPosition.outerR, 0.);
    t2 = G4Translate3D(x, SectorSupportPosition.outerR -
                       SectorSupportSize.DeltaLY -
                       (SectorSupportSize.Thickness +
                        0.5 * SectorSupportSize.Corner1Width) /
                       cos(getSectorSupportCornerAngle()) +
                       (SectorSupportSize.Thickness + 0.5 * lx) *
                       tan(getSectorSupportCornerAngle()), 0.) *
         G4RotateZ3D(getSectorSupportCornerAngle());
    try {
      is1 = new G4IntersectionSolid("Intersection1_" + Corner1_Name,
                                    solidCorner1Tube, solidCorner1Box1, t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solidCorner1 = new G4IntersectionSolid(Corner1_Name, is1,
                                             solidCorner1Box2, t2);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  try {
    logicCorner1 =
      new G4LogicalVolume(solidCorner1, mat.duralumin, Corner1_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner1, true);
  geometry::setColor(*logicCorner1, "#ff0000ff");
  t = G4Translate3D(0., 0., SectorSupportSize.Corner1Z);
  try {
    new G4PVPlacement(t, logicCorner1, Corner1_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner1);
}

void EKLM::GeoEKLMBelleII::createSectorSupportCorner2(G4LogicalVolume* mlv)
{
  static double r;
  static double x;
  static double y;
  static G4TriangularPrism* solidCorner2Prism;
  static G4Tubs* solidCorner2Tubs;
  static G4SubtractionSolid* solidCorner2 = NULL;
  G4LogicalVolume* logicCorner2;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner2_Name = "Corner2_" + mlv->GetName();
  if (solidCorner2 == NULL) {
    r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    x = sqrt(r * r - y * y);
    try {
      solidCorner2Prism = new G4TriangularPrism("Prism_" + Corner2_Name,
                                                SectorSupportSize.Corner2LY,
                                                90. * deg,
                                                SectorSupportSize.Corner2LX,
                                                180. * deg,
                                                0.5 * SectorSupportSize.
                                                Corner2Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    solidCorner2Tubs = createSectorSupportOuterTube(mlv);
    t1 = G4Translate3D(-x, -y, 0.);
    try {
      solidCorner2 = new G4SubtractionSolid(Corner2_Name,
                                            solidCorner2Prism->getSolid(),
                                            solidCorner2Tubs, t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    delete solidCorner2Prism;
  }
  try {
    logicCorner2 = new G4LogicalVolume(solidCorner2, mat.duralumin,
                                       Corner2_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner2, true);
  geometry::setColor(*logicCorner2, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner2Z);
  try {
    new G4PVPlacement(t, logicCorner2, Corner2_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner2);
}

void EKLM::GeoEKLMBelleII::createSectorSupportCorner3(G4LogicalVolume* mlv)
{
  static double r;
  static double x;
  static double y;
  static G4TriangularPrism* solidCorner3Prism;
  static G4Tubs* solidCorner3Tubs;
  static G4SubtractionSolid* solidCorner3 = NULL;
  G4LogicalVolume* logicCorner3;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner3_Name = "Corner3_" + mlv->GetName();
  if (solidCorner3 == NULL) {
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
        SectorSupportSize.Corner3LY;
    x = sqrt(r * r - y * y);
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    try {
      solidCorner3Prism = new G4TriangularPrism("Prism_" + Corner3_Name,
                                                SectorSupportSize.Corner3LX, 0.,
                                                SectorSupportSize.Corner3LY,
                                                90. * deg,
                                                0.5 * SectorSupportSize.
                                                Corner3Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    solidCorner3Tubs = createSectorSupportInnerTube(mlv);
    t1 = G4Translate3D(-x, -y, 0.);
    try {
      solidCorner3 = new G4SubtractionSolid(Corner3_Name,
                                            solidCorner3Prism->getSolid(),
                                            solidCorner3Tubs, t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    delete solidCorner3Prism;
  }
  try {
    logicCorner3 = new G4LogicalVolume(solidCorner3, mat.duralumin,
                                       Corner3_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner3, true);
  geometry::setColor(*logicCorner3, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner3Z);
  try {
    new G4PVPlacement(t, logicCorner3, Corner3_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner3);
}

void EKLM::GeoEKLMBelleII::createSectorSupportCorner4(G4LogicalVolume* mlv)
{
  static double r;
  static double x;
  static double y;
  static G4TriangularPrism* solidCorner4Prism;
  static G4Tubs* solidCorner4Tubs;
  static G4SubtractionSolid* solidCorner4 = NULL;
  G4LogicalVolume* logicCorner4;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner4_Name = "Corner4_" + mlv->GetName();
  if (solidCorner4 == NULL) {
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    x = SectorSupportPosition.X + SectorSupportSize.Thickness +
        SectorSupportSize.Corner4LX;
    y = sqrt(r * r - x * x);
    x = SectorSupportPosition.X + SectorSupportSize.Thickness;
    try {
      solidCorner4Prism = new G4TriangularPrism("Corner4_" + mlv->GetName(),
                                                SectorSupportSize.Corner4LX, 0.,
                                                SectorSupportSize.Corner4LY,
                                                90. * deg,
                                                0.5 * SectorSupportSize.
                                                Corner4Thickness);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    solidCorner4Tubs = createSectorSupportInnerTube(mlv);
    t1 = G4Translate3D(-x, -y, 0.);
    try {
      solidCorner4 = new G4SubtractionSolid(Corner4_Name,
                                            solidCorner4Prism->getSolid(),
                                            solidCorner4Tubs, t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    delete solidCorner4Prism;
  }
  try {
    logicCorner4 = new G4LogicalVolume(solidCorner4, mat.duralumin,
                                       Corner4_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicCorner4, true);
  geometry::setColor(*logicCorner4, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner4Z);
  try {
    new G4PVPlacement(t, logicCorner4, Corner4_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicCorner4);
}

void EKLM::GeoEKLMBelleII::createSectorSupport(G4LogicalVolume* mlv)
{
  static G4Box* solidBoxX;
  static G4Box* solidBoxY;
  static G4Box* solidBoxTop;
  static G4Tubs* solidOuterTube;
  static G4Tubs* solidInnerTube;
  static G4Tubs* solidLimitationTube;
  static G4UnionSolid* us1;
  static G4UnionSolid* us2;
  static G4UnionSolid* us3;
  static G4UnionSolid* us4;
  static G4IntersectionSolid* solidSectorSupport = NULL;
  G4LogicalVolume* logicSectorSupport;
  G4Transform3D t;
  G4Transform3D tbx;
  G4Transform3D tby;
  G4Transform3D tbt;
  std::string SectorSupportName = "Support_" + mlv->GetName();
  if (solidSectorSupport == NULL) {
    solidBoxX = createSectorSupportBoxX(mlv, tbx);
    solidBoxY = createSectorSupportBoxY(mlv, tby);
    solidBoxTop = createSectorSupportBoxTop(mlv, tbt);
    solidOuterTube = createSectorSupportOuterTube(mlv);
    solidInnerTube = createSectorSupportInnerTube(mlv);
    try {
      solidLimitationTube = new G4Tubs("LimitationTube_" + SectorSupportName,
                                       0., SectorSupportPosition.outerR,
                                       0.5 * SectorSupportPosition.length,
                                       0., 90.*deg);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    t = G4Translate3D(0., 0., SectorSupportPosition.Z);
    try {
      us1 = new G4UnionSolid("Union1_" + SectorSupportName,
                             solidInnerTube, solidBoxY, tby);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      us2 = new G4UnionSolid("Union2_" + SectorSupportName,
                             us1, solidBoxX, tbx);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      us3 = new G4UnionSolid("Union3_" + SectorSupportName, us2,
                             solidOuterTube, G4Translate3D(0., 0., 0.));
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      us4 = new G4UnionSolid("Union4_" + SectorSupportName,
                             us3, solidBoxTop, tbt);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solidSectorSupport = new G4IntersectionSolid(SectorSupportName, us4,
                                                   solidLimitationTube,
                                                   G4Translate3D(0., 0., 0.));
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  try {
    logicSectorSupport = new G4LogicalVolume(solidSectorSupport, mat.duralumin,
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

G4SubtractionSolid* EKLM::GeoEKLMBelleII::
subtractBoardSolids(G4SubtractionSolid* plane, int n)
{
  int i;
  int j;
  G4Transform3D t;
  G4Box* solidBoardBox;
  G4SubtractionSolid** ss[2];
  G4SubtractionSolid* prev_solid;
  G4SubtractionSolid* res;
  try {
    solidBoardBox = new G4Box("PlateBox", 0.5 * BoardSize.length,
                              0.5 * BoardSize.height,
                              0.5 * (PlanePosition.length + PlanePosition.Z));
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 0; i < nPlane; i++) {
    ss[i] = (G4SubtractionSolid**)malloc(sizeof(G4SubtractionSolid*) * nBoard);
    if (ss[i] == NULL)
      B2FATAL(MemErr);
    for (j = 0; j < nBoard; j++) {
      t = *BoardTransform[i][j];
      if (n == 1)
        t = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t;
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
  res = ss[nPlane - 1][nBoard - 1];
  for (i = 0; i < nPlane; i++)
    free(ss[i]);
  return res;
}

void EKLM::GeoEKLMBelleII::createPlane(G4LogicalVolume* mlv)
{
  int i;
  int j;
  G4LogicalVolume* logicPlane;
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
    new G4PVPlacement(transf->plane[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1][curvol.plane - 1],
                      logicPlane, Plane_Name, mlv, false, curvol.plane, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  for (i = 1; i <= nSection + 1; i++)
    createSectionSupport(i, logicPlane);
  for (i = 1; i <= 2; i++)
    for (j = 1; j <= nSection; j++)
      createPlasticSheetElement(i, j, logicPlane);
  for (curvol.strip = 1; curvol.strip <= nStrip; curvol.strip++)
    createStripVolume(logicPlane);
}

void EKLM::GeoEKLMBelleII::createSectionReadoutBoard(G4LogicalVolume* mlv)
{
  int i;
  static G4Box* solidSectionReadoutBoard = NULL;
  G4LogicalVolume* logicSectionReadoutBoard;
  std::string Board_Name = "SectionReadoutBoard_" +
                           boost::lexical_cast<std::string>(curvol.board) +
                           "_Plane_" +
                           boost::lexical_cast<std::string>(curvol.plane) +
                           "_" +
                           mlv->GetName();
  if (solidSectionReadoutBoard == NULL)
    try {
      solidSectionReadoutBoard = new G4Box(Board_Name, 0.5 * BoardSize.length,
                                           0.5 * BoardSize.height,
                                           0.5 * BoardSize.width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  try {
    logicSectionReadoutBoard = new G4LogicalVolume(solidSectionReadoutBoard,
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
  for (i = 1; i <= nStripBoard; i++)
    createStripBoard(i, logicSectionReadoutBoard);
}

void EKLM::GeoEKLMBelleII::createBaseBoard(G4LogicalVolume* mlv)
{
  static G4Box* solidBaseBoard = NULL;
  G4LogicalVolume* logicBaseBoard;
  G4Transform3D t;
  std::string Board_Name = "BaseBoard_" + mlv->GetName();
  if (solidBaseBoard == NULL)
    try {
      solidBaseBoard = new G4Box(Board_Name, 0.5 * BoardSize.length,
                                 0.5 * BoardSize.base_height,
                                 0.5 * BoardSize.base_width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  try {
    logicBaseBoard = new G4LogicalVolume(solidBaseBoard, mat.silicon,
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

void EKLM::GeoEKLMBelleII::createStripBoard(int iBoard, G4LogicalVolume* mlv)
{
  static G4Box* solidStripBoard = NULL;
  G4LogicalVolume* logicStripBoard;
  G4Transform3D t;
  std::string Board_Name = "StripBoard_" +
                           boost::lexical_cast<std::string>(iBoard) + "_" +
                           mlv->GetName();
  if (solidStripBoard == NULL)
    try {
      solidStripBoard = new G4Box(Board_Name, 0.5 * BoardSize.strip_length,
                                  0.5 * BoardSize.strip_height,
                                  0.5 * BoardSize.strip_width);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  try {
    logicStripBoard = new G4LogicalVolume(solidStripBoard, mat.silicon,
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

void EKLM::GeoEKLMBelleII::createSectionSupport(int iSectionSupport,
                                                G4LogicalVolume* mlv)
{
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4LogicalVolume* logicSectionSupport;
  std::string SectionSupportName;
  SectionSupportName = "SectionSupport_" +
                       boost::lexical_cast<std::string>(iSectionSupport) +
                       "_" + mlv->GetName();
  if (solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup == NULL) {
    try {
      solids.secsup[curvol.plane - 1][iSectionSupport - 1].topbox =
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
      solids.secsup[curvol.plane - 1][iSectionSupport - 1].midbox =
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
      solids.secsup[curvol.plane - 1][iSectionSupport - 1].botbox =
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
      solids.secsup[curvol.plane - 1][iSectionSupport - 1].us =
        new G4UnionSolid("Union1_" + SectionSupportName,
                         solids.secsup[curvol.plane - 1][iSectionSupport - 1].midbox,
                         solids.secsup[curvol.plane - 1][iSectionSupport - 1].topbox,
                         t1);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    try {
      solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup =
        new G4UnionSolid(SectionSupportName,
                         solids.secsup[curvol.plane - 1][iSectionSupport - 1].us,
                         solids.secsup[curvol.plane - 1][iSectionSupport - 1].botbox,
                         t2);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
  }
  try {
    logicSectionSupport =
      new G4LogicalVolume(solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup,
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

void EKLM::GeoEKLMBelleII::createPlasticSheetElement(int iSheetPlane, int iSheet,
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
                      mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logvol.psheet[iSheet - 1]);
}

void EKLM::GeoEKLMBelleII::createStripVolume(G4LogicalVolume* mlv)
{
  G4Transform3D t;
  std::string StripVolume_Name = "StripVolume_" +
                                 boost::lexical_cast<std::string>(curvol.strip)
                                 + "_" + mlv->GetName();
  if (logvol.stripvol[curvol.strip - 1] == NULL) {
    try {
      logvol.stripvol[curvol.strip - 1] =
        new G4LogicalVolume(solids.stripvol[curvol.strip - 1],
                            mat.air, StripVolume_Name);
    } catch (std::bad_alloc& ba) {
      B2FATAL(MemErr);
    }
    geometry::setVisibility(*logvol.stripvol[curvol.strip - 1], false);
    createStrip(logvol.stripvol[curvol.strip - 1]);
    if (m_mode == EKLM_DETECTOR_BACKGROUND)
      createSiPM(logvol.stripvol[curvol.strip - 1]);
  }
  t = transf->strip[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1][curvol.plane - 1][curvol.strip - 1] *
      G4Translate3D(0.5 * StripSize.rss_size, 0.0, 0.0);
  try {
    new G4PVPlacement(t, logvol.stripvol[curvol.strip - 1],
                      StripVolume_Name, mlv, false, curvol.strip, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
}

void EKLM::GeoEKLMBelleII::createStrip(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicStrip;
  G4Transform3D t;
  std::string Strip_Name = "Strip_" + mlv->GetName();
  try {
    logicStrip = new G4LogicalVolume(solids.strip[curvol.strip - 1],
                                     mat.polystyrene, Strip_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setVisibility(*logicStrip, true);
  geometry::setColor(*logicStrip, "#ffffffff");
  createStripGroove(logicStrip);
  createStripSensitive(logicStrip);
  t = G4Translate3D(-0.5 * StripSize.rss_size, 0., 0.);
  try {
    new G4PVPlacement(t, logicStrip, Strip_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicStrip);
}

void EKLM::GeoEKLMBelleII::createStripGroove(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicGroove;
  G4Transform3D t;
  std::string Groove_Name = "Groove_" + mlv->GetName();
  try {
    logicGroove = new G4LogicalVolume(solids.groove[curvol.strip - 1],
                                      mat.gel, Groove_Name);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setColor(*logicGroove, "#00ff00ff");
  geometry::setVisibility(*logicGroove, true);
  t = G4Translate3D(0., 0.,
                    0.5 * (StripSize.thickness - StripSize.groove_depth));
  try {
    new G4PVPlacement(t, logicGroove, Groove_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicGroove);
}

void EKLM::GeoEKLMBelleII::createStripSensitive(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicSensitive;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Sensitive_Name = "Sensitive_" + mlv->GetName();
  try {
    logicSensitive =
      new G4LogicalVolume(solids.scint[curvol.strip - 1].sens,
                          mat.polystyrene, Sensitive_Name,
                          0, m_sensitive[0], 0);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  geometry::setColor(*logicSensitive, "#ffffffff");
  geometry::setVisibility(*logicSensitive, false);
  t = G4Translate3D(0., 0., 0.);
  try {
    new G4PVPlacement(t, logicSensitive, Sensitive_Name, mlv, false, 1, false);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
  printVolumeMass(logicSensitive);
}

void EKLM::GeoEKLMBelleII::createSiPM(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicSiPM;
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

void EKLM::GeoEKLMBelleII::printVolumeMass(G4LogicalVolume* lv)
{
  if (m_mode == EKLM_DETECTOR_PRINTMASSES)
    printf("Volume %s: mass = %g g\n", lv->GetName().c_str(),
           lv->GetMass() / g);
}

void EKLM::GeoEKLMBelleII::create(const GearDir& content,
                                  G4LogicalVolume& topVolume,
                                  geometry::GeometryTypes type)
{
  (void)content;
  (void)type;
  readXMLData();
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

