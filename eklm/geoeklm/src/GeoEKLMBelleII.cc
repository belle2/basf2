/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Galina Pakhlova, Timofey Uglov, Kirill Chilikin,        *
*                Takanori Hara                                           *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

/* System headers. */
#include <errno.h>
#include <math.h>

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
#include <eklm/geoeklm/EKLMLogicalVolume.h>
#include <eklm/geoeklm/EKLMObjectNumbers.h>
#include <eklm/geoeklm/G4TriangularPrism.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <eklm/simeklm/EKLMSensitiveDetector.h>
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
    transf = new(std::nothrow) EKLM::TransformData;
    if (transf == NULL)
      B2FATAL(MemErr);
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
    delete m_sensitive;
    freeVolumes();
  }
}

/***************************** MEMORY ALLOCATION *****************************/

void EKLM::GeoEKLMBelleII::mallocVolumes()
{
  int i;
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
    (EKLMLogicalVolume**)malloc(nStrip * sizeof(EKLMLogicalVolume*));
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
  solids.plane = (struct EKLM::PlaneSolids*)
                 malloc(nPlane * sizeof(struct EKLM::PlaneSolids));
  if (solids.plane == NULL)
    B2FATAL(MemErr);
  solids.secsup = (struct EKLM::SectionSupportSolids**)
                  malloc(nPlane * sizeof(struct EKLM::SectionSupportSolids*));
  if (solids.secsup == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < nPlane; i++) {
    solids.secsup[i] = (struct EKLM::SectionSupportSolids*)
                       malloc((nSection + 1) *
                              sizeof(struct EKLM::SectionSupportSolids));
    if (solids.plane == NULL)
      B2FATAL(MemErr);
  }
  for (i = 0; i < nStrip; i++)
    logvol.stripvol[i] = NULL;
  for (i = 0; i < nSection; i++)
    logvol.psheet[i] = NULL;
  memset(solids.plane, 0, nPlane * sizeof(struct EKLM::PlaneSolids));
  for (i = 0; i < nPlane; i++)
    memset(solids.secsup[i], 0,
           (nSection + 1) * sizeof(struct EKLM::SectionSupportSolids));
}

void EKLM::GeoEKLMBelleII::freeVolumes()
{
  int i;
  free(solids.psheet);
  free(solids.stripvol);
  free(logvol.stripvol);
  free(solids.strip);
  free(solids.groove);
  free(solids.scint);
  for (i = 0; i < nPlane; i++) {
    free(solids.secsup[i]);
    delete solids.plane[i].prism1;
    delete solids.plane[i].prism2;
    delete solids.plane[i].prism3;
  }
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
  op = new(std::nothrow) G4Polyhedra("Endcap_Octagonal_Prism",
                                     ESTRPar.phi, ESTRPar.dphi,
                                     ESTRPar.nsides, ESTRPar.nboundary,
                                     ESTRPar.z,  ESTRPar.rmin, ESTRPar.rmax);
  tb = new(std::nothrow) G4Tubs("Endcap_Tube",  ESTRPar.rminsub,
                                ESTRPar.rmaxsub,  ESTRPar.zsub,
                                0.0, 360.0 * deg);
  if (op == NULL || tb == NULL)
    B2FATAL(MemErr);
  solids.endcap = new(std::nothrow) G4SubtractionSolid("Endcap", op, tb);
  if (solids.endcap == NULL)
    B2FATAL(MemErr);
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
    b[i] = new(std::nothrow) G4Box(name,
                                   0.5 * StripPosition[m].length,
                                   0.5 * ly, 0.5 * PlasticSheetWidth);
    if (b[i] == NULL)
      B2FATAL(MemErr);
  }
  /* First level unions (7 * 2 + 1). */
  for (i = 0; i < 7; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Union1_%d", n + 1, i + 1);
    t = tli[2 * i] * tl[2 * i + 1];
    u1[i] = new(std::nothrow) G4UnionSolid(name, b[2 * i], b[2 * i + 1], t);
    if (u1[i] == NULL)
      B2FATAL(MemErr);
  }
  u1[7] = (G4UnionSolid*)b[14];
  /* Second level unions (3 * 4 + 1 * 3). */
  for (i = 0; i < 4; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Union2_%d", n + 1, i + 1);
    t = tli[4 * i] * tl[4 * i + 2];
    u2[i] = new(std::nothrow) G4UnionSolid(name, u1[2 * i], u1[2 * i + 1], t);
    if (u2[i] == NULL)
      B2FATAL(MemErr);
  }
  /* Third level unions (1 * 8 + 1 * 7). */
  for (i = 0; i < 2; i++) {
    snprintf(name, 128, "PlasticSheet_%d_Union3_%d", n + 1, i + 1);
    t = tli[8 * i] * tl[8 * i + 4];
    u3[i] = new(std::nothrow) G4UnionSolid(name, u2[2 * i], u2[2 * i + 1], t);
    if (u3[i] == NULL)
      B2FATAL(MemErr);
  }
  /* Plastic sheet. */
  snprintf(name, 128, "PlasticSheet_%d", n + 1);
  t = tli[0] * tl[8];
  solids.psheet[n] = new(std::nothrow) G4UnionSolid(name, u3[0], u3[1], t);
  if (solids.psheet[n] == NULL)
    B2FATAL(MemErr);
}

void EKLM::GeoEKLMBelleII::createSolids()
{
  int i;
  char name[128];
  HepGeom::Transform3D t;
  createEndcapSolid();
  /* Strips. */
  for (i = 0; i < nStrip; i++) {
    /* Strip volumes. */
    snprintf(name, 128, "StripVolume_%d", i + 1);
    solids.stripvol[i] =
      new(std::nothrow) G4Box(name,
                              0.5 * (StripPosition[i].length +
                                     StripSize.rss_size),
                              0.5 * StripSize.width,
                              0.5 * StripSize.thickness);
    if (solids.stripvol[i] == NULL)
      B2FATAL(MemErr);
    /* Strips. */
    snprintf(name, 128, "Strip_%d", i + 1);
    solids.strip[i] =
      new(std::nothrow) G4Box(name,
                              0.5 * StripPosition[i].length,
                              0.5 * StripSize.width,
                              0.5 * StripSize.thickness);
    if (solids.strip[i] == NULL)
      B2FATAL(MemErr);
    /* Strip grooves. */
    snprintf(name, 128, "Groove_%d", i + 1);
    solids.groove[i] =
      new(std::nothrow) G4Box(name,
                              0.5 * StripPosition[i].length,
                              0.5 * StripSize.groove_width,
                              0.5 * StripSize.groove_depth);
    if (solids.groove[i] == NULL)
      B2FATAL(MemErr);
    /* Strip sensitive volumes (scintillator). */
    snprintf(name, 128, "StripSensitive_%d_Box", i + 1);
    solids.scint[i].box =
      new(std::nothrow) G4Box(name,
                              0.5 * StripPosition[i].length -
                              StripSize.no_scintillation_thickness,
                              0.5 * StripSize.width -
                              StripSize.no_scintillation_thickness,
                              0.5 * StripSize.thickness -
                              StripSize.no_scintillation_thickness);
    if (solids.scint[i].box == NULL)
      B2FATAL(MemErr);
    snprintf(name, 128, "StripSensitive_%d", i + 1);
    t = HepGeom::Translate3D(0., 0., 0.5 * (StripSize.thickness -
                                            StripSize.groove_depth));
    solids.scint[i].sens =
      new(std::nothrow) G4SubtractionSolid(name,
                                           solids.scint[i].box,
                                           solids.groove[i], t);
    if (solids.scint[i].sens == NULL)
      B2FATAL(MemErr);
  }
  /* Plastic sheet elements. */
  for (i = 0; i < nSection; i++)
    createPlasticSheetSolid(i);
  /* SiPM (not really a SiPM; a cube in the place of SiPM) */
  if (m_mode != EKLM_DETECTOR_NORMAL) {
    solids.sipm =
      new(std::nothrow) G4Box("SiPM", 0.5 * StripSize.rss_size,
                              0.5 * StripSize.rss_size,
                              0.5 * StripSize.rss_size);
    if (solids.sipm == NULL)
      B2FATAL(MemErr);
  }
}

/************************** CREATION OF VOLUMES ******************************/

void EKLM::GeoEKLMBelleII::createEndcap(G4LogicalVolume* mlv)
{
  EKLMLogicalVolume* logicEndcap;
  G4PVPlacement* physiEndcap;
  G4Transform3D* t;
  std::string Endcap_Name = "Endcap_" +
                            boost::lexical_cast<std::string>(curvol.endcap);
  logicEndcap =
    new(std::nothrow) EKLMLogicalVolume(solids.endcap, mat.iron, Endcap_Name,
                                        curvol.endcap);
  if (logicEndcap == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicEndcap, true);
  geometry::setColor(*logicEndcap, "#ffffff22");
  t = &transf->endcap[curvol.endcap - 1];
  physiEndcap =
    new(std::nothrow) G4PVPlacement(*t, logicEndcap, Endcap_Name, mlv, false,
                                    1, false);
  if (physiEndcap == NULL)
    B2FATAL(MemErr);
  for (curvol.layer = 1; curvol.layer <= nLayer; curvol.layer++)
    createLayer(logicEndcap);
}

void EKLM::GeoEKLMBelleII::createLayer(G4LogicalVolume* mlv)
{
  static G4Tubs* solidLayer = NULL;
  EKLMLogicalVolume* logicLayer;
  G4PVPlacement* physiLayer;
  std::string Layer_Name = "Layer_" +
                           boost::lexical_cast<std::string>(curvol.layer) +
                           "_" + mlv->GetName();
  if (solidLayer == NULL)
    solidLayer =
      new(std::nothrow) G4Tubs(Layer_Name, LayerPosition.innerR,
                               LayerPosition.outerR, LayerPosition.length / 2.0,
                               0. * deg, 360. * deg);
  if (solidLayer == NULL)
    B2FATAL(MemErr);
  logicLayer =
    new(std::nothrow) EKLMLogicalVolume(solidLayer, mat.air, Layer_Name,
                                        layerNumber(curvol.endcap, curvol.layer));
  if (logicLayer == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicLayer, false);
  physiLayer =
    new(std::nothrow) G4PVPlacement(transf->layer[curvol.endcap - 1][curvol.layer - 1],
                                    logicLayer, Layer_Name, mlv, false,
                                    1, false);
  if (physiLayer == NULL)
    B2FATAL(MemErr);
  for (curvol.sector = 1; curvol.sector <= 4; curvol.sector++)
    createSector(logicLayer);
}

void EKLM::GeoEKLMBelleII::createSector(G4LogicalVolume* mlv)
{
  int i;
  static G4Tubs* solidSector = NULL;
  EKLMLogicalVolume* logicSector;
  G4PVPlacement* physiSector;
  std::string Sector_Name = "Sector_" +
                            boost::lexical_cast<std::string>(curvol.sector) +
                            "_" + mlv->GetName();
  if (solidSector == NULL)
    solidSector =
      new(std::nothrow) G4Tubs(Sector_Name, SectorPosition.innerR,
                               SectorPosition.outerR, 0.5 * SectorPosition.length,
                               0.0, 90.0 * deg);
  if (solidSector == NULL)
    B2FATAL(MemErr);
  logicSector =
    new(std::nothrow) EKLMLogicalVolume(solidSector, mat.air, Sector_Name,
                                        sectorNumber(curvol.endcap, curvol.layer, curvol.sector));
  if (logicSector == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSector, false);
  physiSector =
    new(std::nothrow) G4PVPlacement(transf->sector[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1],
                                    logicSector, Sector_Name, mlv, false,
                                    1, false);
  if (physiSector == NULL)
    B2FATAL(MemErr);
  createSectorSupport(logicSector);
  for (i = 1; i <= 2; i++)
    createSectorCover(i, logicSector);
  calcBoardTransform();
  for (curvol.plane = 1; curvol.plane <= nPlane; curvol.plane++)
    createPlane(logicSector);
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
      BoardTransform[i][j] = new(std::nothrow) G4Transform3D(
        G4RotateZ3D(BoardPosition[i][j].phi) *
        G4Translate3D(BoardPosition[i][j].r -
                      0.5 * BoardSize.height, 0., 0.) *
        G4RotateZ3D(90.0 * deg));
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
  G4PVPlacement* physiCover;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t;
  std::string Cover_Name = "Cover_" +
                           boost::lexical_cast<std::string>(iCover) + "_" +
                           mlv->GetName();
  if (solidCover == NULL) {
    lz = 0.5 * (SectorPosition.length - SectorSupportPosition.length);
    solidCoverTube =
      new(std::nothrow) G4Tubs("Tube_" + Cover_Name,
                               SectorSupportPosition.innerR,
                               SectorSupportPosition.outerR, 0.5 * lz, 0.0,
                               90.0 * deg);
    solidCoverBox =
      new(std::nothrow) G4Box("Box_" + Cover_Name,
                              0.5 * SectorSupportPosition.outerR,
                              0.5 * SectorSupportPosition.outerR,
                              0.5 * lz);
    if (solidCoverTube == NULL || solidCoverBox == NULL)
      B2FATAL(MemErr);
    box = new(std::nothrow) G4Box("SubtractionBox_" + Cover_Name,
                                  0.5 * SectorSupportPosition.outerR,
                                  0.5 * SectorSupportPosition.outerR,
                                  lz);
    if (box == NULL)
      B2FATAL(MemErr);
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
    is = new(std::nothrow) G4IntersectionSolid("Intersection_" + Cover_Name,
                                               solidCoverTube,
                                               solidCoverBox, t1);
    if (is == NULL)
      B2FATAL(MemErr);
    solidCover = new(std::nothrow) G4SubtractionSolid(Cover_Name, is, box, t2);
  }
  if (solidCover == NULL)
    B2FATAL(MemErr);
  logicCover = new(std::nothrow) G4LogicalVolume(solidCover, mat.duralumin,
                                                 Cover_Name);
  if (logicCover == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCover, false);
  geometry::setColor(*logicCover, "#ff000022");
  z = 0.25 * (SectorPosition.length + SectorSupportPosition.length);
  if (iCover == 2)
    z = -z;
  t = G4Translate3D(0., 0., z);
  physiCover = new(std::nothrow) G4PVPlacement(t, logicCover, Cover_Name, mlv,
                                               false, 1, false);
  if (physiCover == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicCover);
}

G4Box* EKLM::GeoEKLMBelleII::createSectorSupportBoxX(G4LogicalVolume* mlv,
                                                     G4Transform3D& t)
{
  double x1;
  double x2;
  x1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  x2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  t =  G4Translate3D(0.5 * (x1 + x2), SectorSupportPosition.Y +
                     0.5 * SectorSupportSize.Thickness, 0.);
  return new(std::nothrow) G4Box("BoxX_Support_" + mlv->GetName(),
                                 0.5 * (x2 - x1),
                                 0.5 * SectorSupportSize.Thickness,
                                 0.5 * SectorSupportPosition.length);
}

G4Box* EKLM::GeoEKLMBelleII::createSectorSupportBoxY(G4LogicalVolume* mlv,
                                                     G4Transform3D& t)
{
  double y1;
  double y2;
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  y2 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  t = G4Translate3D(SectorSupportPosition.X + 0.5 * SectorSupportSize.Thickness,
                    0.5 * (y1 + y2), 0.) * G4RotateZ3D(90. * deg);
  return new(std::nothrow) G4Box("BoxY_Support_" + mlv->GetName(),
                                 0.5 * (y2 - y1),
                                 0.5 * SectorSupportSize.Thickness,
                                 0.5 * SectorSupportPosition.length);
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
  x1 = SectorSupportPosition.X;
  y1 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang = getSectorSupportCornerAngle();
  t = G4Translate3D(0.5 * (x1 + x2 + SectorSupportSize.Thickness * sin(ang)),
                    0.5 * (y1 + y2 - SectorSupportSize.Thickness * cos(ang)),
                    0.) * G4RotateZ3D(ang);
  return new(std::nothrow) G4Box("BoxTop_Support_" + mlv->GetName(),
                                 0.5 * sqrt((x2 - x1) *
                                            (x2 - x1) + (y2 - y1) * (y2 - y1)),
                                 0.5 * SectorSupportSize.Thickness,
                                 0.5 * SectorSupportPosition.length);
}

G4Tubs* EKLM::GeoEKLMBelleII::createSectorSupportInnerTube(G4LogicalVolume* mlv)
{
  double x1;
  double y1;
  double ang1;
  double ang2;
  x1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  ang1 = atan2(SectorSupportPosition.Y, x1);
  ang2 = atan2(y1, SectorSupportPosition.X);
  return new(std::nothrow) G4Tubs("InnerTube_Support_" + mlv->GetName(),
                                  SectorSupportPosition.innerR,
                                  SectorSupportPosition.innerR +
                                  SectorSupportSize.Thickness,
                                  0.5 * SectorSupportPosition.length,
                                  std::min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
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
  r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
  x1 = sqrt(r * r - SectorSupportPosition.Y * SectorSupportPosition.Y);
  y1 = SectorSupportPosition.Y;
  x2 = SectorSupportSize.CornerX + SectorSupportPosition.X;
  y2 = sqrt(SectorSupportPosition.outerR * SectorSupportPosition.outerR -
            x2 * x2);
  ang1 = atan2(y1, x1);
  ang2 = atan2(y2, x2);
  return new(std::nothrow) G4Tubs("OuterTube_Support" + mlv->GetName(),
                                  SectorSupportPosition.outerR -
                                  SectorSupportSize.Thickness,
                                  SectorSupportPosition.outerR,
                                  0.5 * SectorSupportPosition.length,
                                  std::min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
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
  G4PVPlacement* physiCorner1;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  std::string Corner1_Name = "Corner1_" + mlv->GetName();
  if (solidCorner1 == NULL) {
    lx = SectorSupportSize.CornerX + SectorSupportSize.Corner1LX -
         SectorSupportSize.Thickness;
    solidCorner1Tube =
      new(std::nothrow) G4Tubs("Tube_" + Corner1_Name, 0.,
                               SectorSupportPosition.outerR -
                               SectorSupportSize.Thickness,
                               0.5 * SectorSupportSize.Corner1Thickness,
                               0., 90. * deg);
    solidCorner1Box1 =
      new(std::nothrow) G4Box("Box1_" + Corner1_Name, 0.5 * lx,
                              0.5 * SectorSupportPosition.outerR,
                              0.5 * SectorSupportSize.Corner1Thickness);
    solidCorner1Box2 =
      new(std::nothrow) G4Box("Box2_" + Corner1_Name,
                              0.5 * (lx / cos(getSectorSupportCornerAngle()) +
                                     SectorSupportSize.Corner1Width *
                                     sin(getSectorSupportCornerAngle())),
                              0.5 * SectorSupportSize.Corner1Width,
                              0.5 * SectorSupportSize.Corner1Thickness);
    if (solidCorner1Tube == NULL || solidCorner1Box1 == NULL ||
        solidCorner1Box2 == NULL)
      B2FATAL(MemErr);
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
    is1 = new(std::nothrow) G4IntersectionSolid("Intersection1_" + Corner1_Name,
                                                solidCorner1Tube,
                                                solidCorner1Box1, t1);
    if (is1 == NULL)
      B2FATAL(MemErr);
    solidCorner1 =
      new(std::nothrow) G4IntersectionSolid(Corner1_Name, is1, solidCorner1Box2,
                                            t2);
  }
  if (solidCorner1 == NULL)
    B2FATAL(MemErr);
  logicCorner1 =
    new(std::nothrow) G4LogicalVolume(solidCorner1, mat.duralumin, Corner1_Name);
  if (logicCorner1 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner1, true);
  geometry::setColor(*logicCorner1, "#ff0000ff");
  t = G4Translate3D(0., 0., SectorSupportSize.Corner1Z);
  physiCorner1 =
    new(std::nothrow) G4PVPlacement(t, logicCorner1, Corner1_Name, mlv, false,
                                    1, false);
  if (physiCorner1 == NULL)
    B2FATAL(MemErr);
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
  G4PVPlacement* physiCorner2;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner2_Name = "Corner2_" + mlv->GetName();
  if (solidCorner2 == NULL) {
    r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    x = sqrt(r * r - y * y);
    solidCorner2Prism =
      new(std::nothrow) G4TriangularPrism("Prism_" + Corner2_Name,
                                          SectorSupportSize.Corner2LY,
                                          90. * deg,
                                          SectorSupportSize.Corner2LX,
                                          180. * deg,
                                          0.5 * SectorSupportSize.
                                          Corner2Thickness);
    solidCorner2Tubs = createSectorSupportOuterTube(mlv);
    if (solidCorner2Prism == NULL || solidCorner2Tubs == NULL)
      B2FATAL(MemErr);
    if (solidCorner2Prism->getSolid() == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(-x, -y, 0.);
    solidCorner2 =
      new(std::nothrow) G4SubtractionSolid(Corner2_Name,
                                           solidCorner2Prism->getSolid(),
                                           solidCorner2Tubs, t1);
    delete solidCorner2Prism;
  }
  if (solidCorner2 == NULL)
    B2FATAL(MemErr);
  logicCorner2 =
    new(std::nothrow) G4LogicalVolume(solidCorner2, mat.duralumin, Corner2_Name);
  if (logicCorner2 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner2, true);
  geometry::setColor(*logicCorner2, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner2Z);
  physiCorner2 =
    new(std::nothrow) G4PVPlacement(t, logicCorner2, Corner2_Name, mlv, false,
                                    1, false);
  if (physiCorner2 == NULL)
    B2FATAL(MemErr);
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
  G4PVPlacement* physiCorner3;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner3_Name = "Corner3_" + mlv->GetName();
  if (solidCorner3 == NULL) {
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
        SectorSupportSize.Corner3LY;
    x = sqrt(r * r - y * y);
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    solidCorner3Prism =
      new(std::nothrow) G4TriangularPrism("Prism_" + Corner3_Name,
                                          SectorSupportSize.Corner3LX, 0.,
                                          SectorSupportSize.Corner3LY,
                                          90. * deg,
                                          0.5 * SectorSupportSize.
                                          Corner3Thickness);
    solidCorner3Tubs = createSectorSupportInnerTube(mlv);
    if (solidCorner3Prism == NULL || solidCorner3Tubs == NULL)
      B2FATAL(MemErr);
    if (solidCorner3Prism->getSolid() == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(-x, -y, 0.);
    solidCorner3 =
      new(std::nothrow) G4SubtractionSolid(Corner3_Name,
                                           solidCorner3Prism->getSolid(),
                                           solidCorner3Tubs, t1);
    delete solidCorner3Prism;
  }
  if (solidCorner3 == NULL)
    B2FATAL(MemErr);
  logicCorner3 =
    new(std::nothrow) G4LogicalVolume(solidCorner3, mat.duralumin, Corner3_Name);
  if (logicCorner3 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner3, true);
  geometry::setColor(*logicCorner3, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner3Z);
  physiCorner3 =
    new(std::nothrow) G4PVPlacement(t, logicCorner3, Corner3_Name, mlv, false,
                                    1, false);
  if (physiCorner3 == NULL)
    B2FATAL(MemErr);
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
  G4PVPlacement* physiCorner4;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner4_Name = "Corner4_" + mlv->GetName();
  if (solidCorner4 == NULL) {
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    x = SectorSupportPosition.X + SectorSupportSize.Thickness +
        SectorSupportSize.Corner4LX;
    y = sqrt(r * r - x * x);
    x = SectorSupportPosition.X + SectorSupportSize.Thickness;
    solidCorner4Prism =
      new(std::nothrow) G4TriangularPrism("Corner4_" + mlv->GetName(),
                                          SectorSupportSize.Corner4LX, 0.,
                                          SectorSupportSize.Corner4LY,
                                          90. * deg,
                                          0.5 * SectorSupportSize.
                                          Corner4Thickness);
    solidCorner4Tubs = createSectorSupportInnerTube(mlv);
    if (solidCorner4Prism == NULL || solidCorner4Tubs == NULL)
      B2FATAL(MemErr);
    if (solidCorner4Prism->getSolid() == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(-x, -y, 0.);
    solidCorner4 =
      new(std::nothrow) G4SubtractionSolid(Corner4_Name,
                                           solidCorner4Prism->getSolid(),
                                           solidCorner4Tubs, t1);
    delete solidCorner4Prism;
  }
  if (solidCorner4 == NULL)
    B2FATAL(MemErr);
  logicCorner4 =
    new(std::nothrow) G4LogicalVolume(solidCorner4, mat.duralumin, Corner4_Name);
  if (logicCorner4 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner4, true);
  geometry::setColor(*logicCorner4, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner4Z);
  physiCorner4 =
    new(std::nothrow) G4PVPlacement(t, logicCorner4, Corner4_Name, mlv, false,
                                    1, false);
  if (physiCorner4 == NULL)
    B2FATAL(MemErr);
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
  G4PVPlacement* physiSectorSupport;
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
    solidLimitationTube =
      new(std::nothrow) G4Tubs("LimitationTube_" + SectorSupportName,
                               0., SectorSupportPosition.outerR,
                               0.5 * SectorSupportPosition.length,
                               0., 90.*deg);
    if (solidBoxX == NULL || solidBoxY == NULL || solidOuterTube == NULL ||
        solidInnerTube == NULL || solidLimitationTube == NULL)
      B2FATAL(MemErr);
    t = G4Translate3D(0., 0., SectorSupportPosition.Z);
    us1 = new(std::nothrow) G4UnionSolid("Union1_" + SectorSupportName,
                                         solidInnerTube,
                                         solidBoxY, tby);
    if (us1 == NULL)
      B2FATAL(MemErr);
    us2 = new(std::nothrow) G4UnionSolid("Union2_" + SectorSupportName,
                                         us1, solidBoxX, tbx);
    if (us2 == NULL)
      B2FATAL(MemErr);
    us3 = new(std::nothrow) G4UnionSolid("Union3_" + SectorSupportName, us2,
                                         solidOuterTube,
                                         G4Translate3D(0., 0., 0.));
    if (us3 == NULL)
      B2FATAL(MemErr);
    us4 = new(std::nothrow) G4UnionSolid("Union4_" + SectorSupportName,
                                         us3, solidBoxTop, tbt);
    if (us4 == NULL)
      B2FATAL(MemErr);
    solidSectorSupport =
      new(std::nothrow) G4IntersectionSolid(SectorSupportName, us4,
                                            solidLimitationTube,
                                            G4Translate3D(0., 0., 0.));
  }
  if (solidSectorSupport == NULL)
    B2FATAL(MemErr);
  logicSectorSupport =
    new(std::nothrow) G4LogicalVolume(solidSectorSupport, mat.duralumin,
                                      SectorSupportName);
  if (logicSectorSupport == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSectorSupport, true);
  geometry::setColor(*logicSectorSupport, "#ff0000ff");
  physiSectorSupport =
    new(std::nothrow) G4PVPlacement(t, logicSectorSupport, SectorSupportName,
                                    mlv, false, 1, false);
  if (physiSectorSupport == NULL)
    B2FATAL(MemErr);
  createSectorSupportCorner1(mlv);
  createSectorSupportCorner2(mlv);
  createSectorSupportCorner3(mlv);
  createSectorSupportCorner4(mlv);
  printVolumeMass(logicSectorSupport);
}

G4SubtractionSolid* EKLM::GeoEKLMBelleII::
subtractBoardSolids(G4SubtractionSolid* plane, std::string Plane_Name)
{
  int i;
  int j;
  G4Transform3D t;
  G4Box* solidBoardBox;
  G4SubtractionSolid** ss[2];
  G4SubtractionSolid* prev_solid;
  solidBoardBox =
    new(std::nothrow) G4Box("PlateBox_" + Plane_Name, 0.5 * BoardSize.length,
                            0.5 * BoardSize.height,
                            0.5 * (PlanePosition.length + PlanePosition.Z));
  for (i = 0; i < nPlane; i++) {
    ss[i] = new(std::nothrow) G4SubtractionSolid*[nBoard];
    if (ss[i] == NULL)
      return NULL;
    for (j = 0; j < nBoard; j++) {
      t = *BoardTransform[i][j];
      if (curvol.plane == 2)
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
      ss[i][j] =
        new(std::nothrow) G4SubtractionSolid("BoardSubtraction_" +
                                             boost::lexical_cast<std::string>(i) + "_" +
                                             boost::lexical_cast<std::string>(j) + Plane_Name,
                                             prev_solid, solidBoardBox, t);
    }
  }
  return ss[nPlane - 1][nBoard - 1];
}

void EKLM::GeoEKLMBelleII::createPlane(G4LogicalVolume* mlv)
{
  int i;
  int j;
  double r;
  double x;
  double y;
  double box_x;
  double box_lx;
  double ang;
  EKLMLogicalVolume* logicPlane;
  G4PVPlacement* physiPlane;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t3;
  G4Transform3D t4;
  G4Transform3D t5;
  std::string Plane_Name = "Plane_" +
                           boost::lexical_cast<std::string>(curvol.plane) + "_" +
                           mlv->GetName();
  if (solids.plane[curvol.plane - 1].plane == NULL) {
    solids.plane[curvol.plane - 1].tube =
      new(std::nothrow) G4Tubs("Tube_" + Plane_Name, PlanePosition.innerR,
                               PlanePosition.outerR, 0.5 * PlanePosition.length,
                               0.0, 90.0 * deg);
    box_x = std::max(SectorSupportPosition.Y, SectorSupportPosition.X) +
            SectorSupportSize.Thickness;
    box_lx =  PlanePosition.outerR - box_x;
    solids.plane[curvol.plane - 1].box1 =
      new(std::nothrow) G4Box("Box_" + Plane_Name, 0.5 * box_lx, 0.5 * box_lx,
                              0.5 * PlanePosition.length);
    solids.plane[curvol.plane - 1].prism1 =
      new(std::nothrow) G4TriangularPrism("TriangularPrism1_" + Plane_Name,
                                          SectorSupportSize.Corner2LY,
                                          90. * deg,
                                          SectorSupportSize.Corner2LX,
                                          180. * deg, PlanePosition.length);
    solids.plane[curvol.plane - 1].prism2 =
      new(std::nothrow) G4TriangularPrism("TriangularPrism2_" + Plane_Name,
                                          SectorSupportSize.Corner3LX, 0.,
                                          SectorSupportSize.Corner3LY,
                                          90. * deg, PlanePosition.length);
    solids.plane[curvol.plane - 1].prism3 =
      new(std::nothrow) G4TriangularPrism("TriangularPrism3_" + Plane_Name,
                                          SectorSupportSize.Corner4LX, 0.,
                                          SectorSupportSize.Corner4LY,
                                          90. * deg, PlanePosition.length);
    if (solids.plane[curvol.plane - 1].tube == NULL ||
        solids.plane[curvol.plane - 1].box1 == NULL ||
        solids.plane[curvol.plane - 1].prism1 == NULL ||
        solids.plane[curvol.plane - 1].prism2 == NULL ||
        solids.plane[curvol.plane - 1].prism3 == NULL)
      B2FATAL(MemErr);
    if (solids.plane[curvol.plane - 1].prism1->getSolid() == NULL ||
        solids.plane[curvol.plane - 1].prism2->getSolid() == NULL ||
        solids.plane[curvol.plane - 1].prism3->getSolid() == NULL)
      B2FATAL(MemErr);
    solids.plane[curvol.plane - 1].box2 =
      new(std::nothrow) G4Box("Box1_" + Plane_Name,
                              0.5 * box_lx, 0.5 * box_lx,
                              PlanePosition.length);
    if (solids.plane[curvol.plane - 1].box2 == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(0.5 * (PlanePosition.outerR + box_x),
                       0.5 * (PlanePosition.outerR + box_x), 0.);
    if (curvol.plane == 2)
      t1 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t1;
    ang = getSectorSupportCornerAngle();
    x = std::max(SectorSupportPosition.Y, SectorSupportPosition.X);
    y = SectorSupportPosition.outerR -
        SectorSupportSize.DeltaLY -
        SectorSupportSize.TopCornerHeight;
    if (curvol.plane == 1) {
      t2 = G4Translate3D(x + 0.5 * box_lx * cos(ang) - 0.5 * box_lx * sin(ang),
                         y + 0.5 * box_lx * cos(ang) + 0.5 * box_lx * sin(ang),
                         0.) * G4RotateZ3D(ang);
    } else {
      t2 = G4Translate3D(y + 0.5 * box_lx * cos(ang) + 0.5 * box_lx * sin(ang),
                         x + 0.5 * box_lx * cos(ang) - 0.5 * box_lx * sin(ang),
                         0.) * G4RotateZ3D(-ang);
    }
    r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    x = sqrt(r * r - y * y);
    t3 = G4Translate3D(x, y, 0.);
    if (curvol.plane == 2)
      t3 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t3;
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
        SectorSupportSize.Corner3LY;
    x = sqrt(r * r - y * y);
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    t4 = G4Translate3D(x, y, 0.);
    if (curvol.plane == 2)
      t4 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t4;
    x = SectorSupportPosition.X + SectorSupportSize.Thickness +
        SectorSupportSize.Corner4LX;
    y = sqrt(r * r - x * x);
    x = SectorSupportPosition.X + SectorSupportSize.Thickness;
    t5 = G4Translate3D(x, y, 0.);
    if (curvol.plane == 2)
      t5 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t5;
    solids.plane[curvol.plane - 1].is =
      new(std::nothrow) G4IntersectionSolid("Intersection_" + Plane_Name,
                                            solids.plane[curvol.plane - 1].tube,
                                            solids.plane[curvol.plane - 1].box1, t1);
    if (solids.plane[curvol.plane - 1].is == NULL)
      B2FATAL(MemErr);
    solids.plane[curvol.plane - 1].ss1 =
      new(std::nothrow) G4SubtractionSolid("Subtraction1_" + Plane_Name,
                                           solids.plane[curvol.plane - 1].is,
                                           solids.plane[curvol.plane - 1].box2, t2);
    if (solids.plane[curvol.plane - 1].ss1 == NULL)
      B2FATAL(MemErr);
    solids.plane[curvol.plane - 1].ss2 =
      new(std::nothrow) G4SubtractionSolid("Subtraction2_" + Plane_Name,
                                           solids.plane[curvol.plane - 1].ss1,
                                           solids.plane[curvol.plane - 1].prism1->getSolid(),
                                           t3);
    if (solids.plane[curvol.plane - 1].ss2 == NULL)
      B2FATAL(MemErr);
    solids.plane[curvol.plane - 1].ss3 =
      new(std::nothrow) G4SubtractionSolid("Subtraction3_" + Plane_Name,
                                           solids.plane[curvol.plane - 1].ss2,
                                           solids.plane[curvol.plane - 1].prism2->getSolid(),
                                           t4);
    if (solids.plane[curvol.plane - 1].ss3 == NULL)
      B2FATAL(MemErr);
    solids.plane[curvol.plane - 1].ss4 =
      new(std::nothrow) G4SubtractionSolid("Subtraction4_" + Plane_Name,
                                           solids.plane[curvol.plane - 1].ss3,
                                           solids.plane[curvol.plane - 1].prism3->getSolid(),
                                           t5);
    if (solids.plane[curvol.plane - 1].ss4 == NULL)
      B2FATAL(MemErr);
    solids.plane[curvol.plane - 1].plane =
      subtractBoardSolids(solids.plane[curvol.plane - 1].ss4, Plane_Name);
  }
  if (solids.plane[curvol.plane - 1].plane == NULL)
    B2FATAL(MemErr);
  logicPlane =
    new(std::nothrow) EKLMLogicalVolume(solids.plane[curvol.plane - 1].plane,
                                        mat.air, Plane_Name,
                                        planeNumber(curvol.endcap, curvol.layer, curvol.sector, curvol.plane));
  if (logicPlane == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicPlane, false);
  physiPlane = new(std::nothrow) G4PVPlacement(transf->plane[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1][curvol.plane - 1],
                                               logicPlane, Plane_Name, mlv,
                                               false, 1, false);
  if (physiPlane == NULL)
    B2FATAL(MemErr);
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
  EKLMLogicalVolume* logicSectionReadoutBoard;
  G4PVPlacement* physiSectionReadoutBoard;
  std::string Board_Name = "SectionReadoutBoard_" +
                           boost::lexical_cast<std::string>(curvol.board) +
                           "_Plane_" +
                           boost::lexical_cast<std::string>(curvol.plane) +
                           "_" +
                           mlv->GetName();
  if (solidSectionReadoutBoard == NULL)
    solidSectionReadoutBoard = new(std::nothrow) G4Box(Board_Name,
                                                       0.5 * BoardSize.length,
                                                       0.5 * BoardSize.height,
                                                       0.5 * BoardSize.width);
  if (solidSectionReadoutBoard == NULL)
    B2FATAL(MemErr);
  logicSectionReadoutBoard =
    new(std::nothrow) EKLMLogicalVolume(solidSectionReadoutBoard,
                                        mat.air,
                                        Board_Name,
                                        boardNumber(curvol.endcap, curvol.layer, curvol.sector, curvol.plane, curvol.board));
  if (logicSectionReadoutBoard == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSectionReadoutBoard, false);
  physiSectionReadoutBoard =
    new(std::nothrow) G4PVPlacement(*BoardTransform[curvol.plane - 1][curvol.board - 1],
                                    logicSectionReadoutBoard,
                                    Board_Name, mlv, false, 1, false);
  if (physiSectionReadoutBoard == NULL)
    B2FATAL(MemErr);
  createBaseBoard(logicSectionReadoutBoard);
  if (m_mode != EKLM_DETECTOR_NORMAL)
    for (i = 1; i <= nStripBoard; i++)
      createStripBoard(i, logicSectionReadoutBoard);
}

void EKLM::GeoEKLMBelleII::createBaseBoard(G4LogicalVolume* mlv)
{
  static G4Box* solidBaseBoard = NULL;
  G4LogicalVolume* logicBaseBoard;
  G4PVPlacement* physiBaseBoard;
  G4Transform3D t;
  std::string Board_Name = "BaseBoard_" + mlv->GetName();
  if (solidBaseBoard == NULL)
    solidBaseBoard = new(std::nothrow) G4Box(Board_Name, 0.5 * BoardSize.length,
                                             0.5 * BoardSize.base_height,
                                             0.5 * BoardSize.base_width);
  if (solidBaseBoard == NULL)
    B2FATAL(MemErr);
  logicBaseBoard =
    new(std::nothrow) G4LogicalVolume(solidBaseBoard, mat.silicon,
                                      Board_Name);
  if (logicBaseBoard == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicBaseBoard, true);
  geometry::setColor(*logicBaseBoard, "#0000ffff");
  t = G4Translate3D(0., -0.5 * BoardSize.height + 0.5 * BoardSize.base_height,
                    0.);
  physiBaseBoard =
    new(std::nothrow) G4PVPlacement(t, logicBaseBoard, Board_Name, mlv, false,
                                    1, false);
  if (physiBaseBoard == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicBaseBoard);
}

void EKLM::GeoEKLMBelleII::createStripBoard(int iBoard, G4LogicalVolume* mlv)
{
  static G4Box* solidStripBoard = NULL;
  EKLMLogicalVolume* logicStripBoard;
  G4PVPlacement* physiStripBoard;
  G4Transform3D t;
  std::string Board_Name = "StripBoard_" +
                           boost::lexical_cast<std::string>(iBoard) + "_" +
                           mlv->GetName();
  if (solidStripBoard == NULL)
    solidStripBoard =
      new(std::nothrow) G4Box(Board_Name, 0.5 * BoardSize.strip_length,
                              0.5 * BoardSize.strip_height,
                              0.5 * BoardSize.strip_width);
  if (solidStripBoard == NULL)
    B2FATAL(MemErr);
  if (m_mode == EKLM_DETECTOR_NORMAL)
    logicStripBoard =
      new(std::nothrow) EKLMLogicalVolume(solidStripBoard, mat.silicon,
                                          Board_Name, 0, 0, m_mode,
                                          EKLM_NOT_SENSITIVE);
  else
    logicStripBoard =
      new(std::nothrow) EKLMLogicalVolume(solidStripBoard, mat.silicon,
                                          Board_Name, 0, m_sensitive, m_mode,
                                          EKLM_SENSITIVE_BOARD);
  if (logicStripBoard == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicStripBoard, true);
  geometry::setColor(*logicStripBoard, "#0000ffff");
  t = G4Translate3D(-0.5 * BoardSize.length + StripBoardPosition[iBoard - 1].x,
                    -0.5 * BoardSize.height + BoardSize.base_height +
                    0.5 * BoardSize.strip_height, 0.);
  physiStripBoard =
    new(std::nothrow) G4PVPlacement(t, logicStripBoard, Board_Name, mlv, false,
                                    1, false);
  if (physiStripBoard == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicStripBoard);
}

void EKLM::GeoEKLMBelleII::createSectionSupport(int iSectionSupport,
                                                G4LogicalVolume* mlv)
{
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4LogicalVolume* logicSectionSupport;
  G4PVPlacement* physiSectionSupport;
  std::string SectionSupportName;
  SectionSupportName = "SectionSupport_" +
                       boost::lexical_cast<std::string>(iSectionSupport) +
                       "_" + mlv->GetName();
  if (solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup == NULL) {
    solids.secsup[curvol.plane - 1][iSectionSupport - 1].topbox =
      new(std::nothrow) G4Box("BoxTop_" + SectionSupportName,
                              0.5 * (SectionSupportPosition[curvol.plane - 1]
                                     [iSectionSupport - 1].length -
                                     SectionSupportPosition[curvol.plane - 1]
                                     [iSectionSupport - 1].deltal_left -
                                     SectionSupportPosition[curvol.plane - 1]
                                     [iSectionSupport - 1].deltal_right),
                              0.5 * SectionSupportTopWidth,
                              0.5 * SectionSupportTopThickness);
    solids.secsup[curvol.plane - 1][iSectionSupport - 1].midbox =
      new(std::nothrow) G4Box("BoxMiddle_" + SectionSupportName,
                              0.5 * (SectionSupportPosition[curvol.plane - 1]
                                     [iSectionSupport - 1].length -
                                     SectionSupportPosition[curvol.plane - 1]
                                     [iSectionSupport - 1].deltal_left -
                                     SectionSupportPosition[curvol.plane - 1]
                                     [iSectionSupport - 1].deltal_right),
                              0.5 * SectionSupportMiddleWidth,
                              0.5 * SectionSupportMiddleThickness);
    solids.secsup[curvol.plane - 1][iSectionSupport - 1].botbox =
      new(std::nothrow) G4Box("BoxBottom_" + SectionSupportName,
                              0.5 * SectionSupportPosition[curvol.plane - 1]
                              [iSectionSupport - 1].length,
                              0.5 * SectionSupportTopWidth,
                              0.5 * SectionSupportTopThickness);
    if (solids.secsup[curvol.plane - 1][iSectionSupport - 1].topbox == NULL ||
        solids.secsup[curvol.plane - 1][iSectionSupport - 1].midbox == NULL ||
        solids.secsup[curvol.plane - 1][iSectionSupport - 1].botbox == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(0., 0., 0.5 * (SectionSupportMiddleThickness +
                                      SectionSupportTopThickness));
    t2 = G4Translate3D(0.5 * (SectionSupportPosition[curvol.plane - 1]
                              [iSectionSupport - 1].deltal_right -
                              SectionSupportPosition[curvol.plane - 1]
                              [iSectionSupport - 1].deltal_left),
                       0., -0.5 * (SectionSupportMiddleThickness +
                                   SectionSupportTopThickness));
    solids.secsup[curvol.plane - 1][iSectionSupport - 1].us =
      new(std::nothrow) G4UnionSolid("Union1_" + SectionSupportName,
                                     solids.secsup[curvol.plane - 1][iSectionSupport - 1].midbox,
                                     solids.secsup[curvol.plane - 1][iSectionSupport - 1].topbox,
                                     t1);
    if (solids.secsup[curvol.plane - 1][iSectionSupport - 1].us == NULL)
      B2FATAL(MemErr);
    solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup =
      new(std::nothrow) G4UnionSolid(SectionSupportName,
                                     solids.secsup[curvol.plane - 1][iSectionSupport - 1].us,
                                     solids.secsup[curvol.plane - 1][iSectionSupport - 1].botbox,
                                     t2);
  }
  if (solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup == NULL)
    B2FATAL(MemErr);
  logicSectionSupport =
    new(std::nothrow) G4LogicalVolume(solids.secsup[curvol.plane - 1][iSectionSupport - 1].secsup,
                                      mat.duralumin, SectionSupportName);
  if (logicSectionSupport == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSectionSupport, true);
  geometry::setColor(*logicSectionSupport, "#ff0000ff");
  t = G4Translate3D(0.5 * (SectionSupportPosition[curvol.plane - 1]
                           [iSectionSupport - 1].deltal_left -
                           SectionSupportPosition[curvol.plane - 1]
                           [iSectionSupport - 1].deltal_right) +
                    SectionSupportPosition[curvol.plane - 1][iSectionSupport - 1].x,
                    SectionSupportPosition[curvol.plane - 1][iSectionSupport - 1].y,
                    SectionSupportPosition[curvol.plane - 1][iSectionSupport - 1].z);
  physiSectionSupport =
    new(std::nothrow) G4PVPlacement(t, logicSectionSupport, SectionSupportName,
                                    mlv, false, 1, false);
  if (physiSectionSupport == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicSectionSupport);
}

void EKLM::GeoEKLMBelleII::createPlasticSheetElement(int iSheetPlane, int iSheet,
                                                     G4LogicalVolume* mlv)
{
  double z;
  G4PVPlacement* physiSheet;
  G4Transform3D t;
  std::string Sheet_Name = "Sheet_" +
                           boost::lexical_cast<std::string>(iSheet) +
                           "_SheetPlane_" +
                           boost::lexical_cast<std::string>(iSheetPlane) +
                           "_" + mlv->GetName();
  if (logvol.psheet[iSheet - 1] == NULL) {
    logvol.psheet[iSheet - 1] =
      new(std::nothrow) G4LogicalVolume(solids.psheet[iSheet - 1],
                                        mat.polystyrol, Sheet_Name);
    if (logvol.psheet[iSheet - 1] == NULL)
      B2FATAL(MemErr);
    geometry::setVisibility(*logvol.psheet[iSheet - 1], false);
    geometry::setColor(*logvol.psheet[iSheet - 1], "#00ff00ff");
  }
  z = 0.5 * (StripSize.thickness + PlasticSheetWidth);
  if (iSheetPlane == 2)
    z = -z;
  getSheetTransform(&t, (iSheet - 1) * 15);
  t = t * G4Translate3D(0, 0, z);
  physiSheet = new(std::nothrow) G4PVPlacement(t, logvol.psheet[iSheet - 1],
                                               Sheet_Name, mlv,
                                               false, 1, false);
  if (physiSheet == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logvol.psheet[iSheet - 1]);
}

void EKLM::GeoEKLMBelleII::createStripVolume(G4LogicalVolume* mlv)
{
  G4PVPlacement* physiStripVolume;
  G4Transform3D t;
  std::string StripVolume_Name = "StripVolume_" +
                                 boost::lexical_cast<std::string>(curvol.strip)
                                 + "_" + mlv->GetName();
  if (logvol.stripvol[curvol.strip - 1] == NULL) {
    logvol.stripvol[curvol.strip - 1] =
      new(std::nothrow) EKLMLogicalVolume(solids.stripvol[curvol.strip - 1],
                                          mat.air,
                                          StripVolume_Name,
                                          stripNumber(curvol.endcap, curvol.layer, curvol.sector, curvol.plane, curvol.strip));
    if (logvol.stripvol[curvol.strip - 1] == NULL)
      B2FATAL(MemErr);
    geometry::setVisibility(*logvol.stripvol[curvol.strip - 1], false);
    createStrip(logvol.stripvol[curvol.strip - 1]);
    if (m_mode != EKLM_DETECTOR_NORMAL)
      createSiPM(logvol.stripvol[curvol.strip - 1]);
  }
  t = transf->strip[curvol.endcap - 1][curvol.layer - 1][curvol.sector - 1][curvol.plane - 1][curvol.strip - 1] *
      G4Translate3D(0.5 * StripSize.rss_size, 0.0, 0.0);
  physiStripVolume = new(std::nothrow)
  G4PVPlacement(t, logvol.stripvol[curvol.strip - 1], StripVolume_Name, mlv,
                false, 1, false);
  if (physiStripVolume == NULL)
    B2FATAL(MemErr);
}

void EKLM::GeoEKLMBelleII::createStrip(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicStrip;
  G4PVPlacement* physiStrip;
  G4Transform3D t;
  std::string Strip_Name = "Strip_" + mlv->GetName();
  logicStrip = new(std::nothrow) G4LogicalVolume(solids.strip[curvol.strip - 1],
                                                 mat.polystyrene, Strip_Name);
  if (logicStrip == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicStrip, true);
  geometry::setColor(*logicStrip, "#ffffffff");
  createStripGroove(logicStrip);
  createStripSensitive(logicStrip);
  t = G4Translate3D(-0.5 * StripSize.rss_size, 0., 0.);
  physiStrip =
    new(std::nothrow) G4PVPlacement(t, logicStrip, Strip_Name, mlv, false,
                                    1, false);
  if (physiStrip == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicStrip);
}

void EKLM::GeoEKLMBelleII::createStripGroove(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicGroove;
  G4PVPlacement* physiGroove;
  G4Transform3D t;
  std::string Groove_Name = "Groove_" + mlv->GetName();
  logicGroove =
    new(std::nothrow) G4LogicalVolume(solids.groove[curvol.strip - 1],
                                      mat.gel, Groove_Name);
  if (logicGroove == NULL)
    B2FATAL(MemErr);
  geometry::setColor(*logicGroove, "#00ff00ff");
  geometry::setVisibility(*logicGroove, true);
  t = G4Translate3D(0., 0.,
                    0.5 * (StripSize.thickness - StripSize.groove_depth));
  physiGroove =
    new(std::nothrow) G4PVPlacement(t, logicGroove, Groove_Name, mlv, false,
                                    1, false);
  if (physiGroove == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicGroove);
}

void EKLM::GeoEKLMBelleII::createStripSensitive(G4LogicalVolume* mlv)
{
  EKLMLogicalVolume* logicSensitive;
  G4PVPlacement* physiSensitive;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Sensitive_Name = "Sensitive_" + mlv->GetName();
  logicSensitive =
    new(std::nothrow) EKLMLogicalVolume(solids.scint[curvol.strip - 1].sens,
                                        mat.polystyrene, Sensitive_Name,
                                        0, m_sensitive, m_mode,
                                        EKLM_SENSITIVE_STRIP);
  if (logicSensitive == NULL)
    B2FATAL(MemErr);
  geometry::setColor(*logicSensitive, "#ffffffff");
  geometry::setVisibility(*logicSensitive, false);
  t = G4Translate3D(0., 0., 0.);
  physiSensitive =
    new(std::nothrow) G4PVPlacement(t, logicSensitive, Sensitive_Name, mlv,
                                    false, 1, false);
  if (physiSensitive == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicSensitive);
}

void EKLM::GeoEKLMBelleII::createSiPM(G4LogicalVolume* mlv)
{
  G4LogicalVolume* logicSiPM;
  G4PVPlacement* physiSiPM;
  G4Transform3D t;
  std::string SiPM_Name = "SiPM_" + mlv->GetName();
  if (m_mode == EKLM_DETECTOR_NORMAL)
    logicSiPM =
      new(std::nothrow) EKLMLogicalVolume(solids.sipm, mat.silicon, SiPM_Name,
                                          0, 0, m_mode, EKLM_SENSITIVE_SIPM);
  else
    logicSiPM =
      new(std::nothrow) EKLMLogicalVolume(solids.sipm, mat.silicon, SiPM_Name, 0,
                                          m_sensitive, m_mode, EKLM_SENSITIVE_SIPM);
  if (logicSiPM == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSiPM, true);
  geometry::setColor(*logicSiPM, "#0000ffff");
  t = G4Translate3D(0.5 * StripPosition[curvol.strip - 1].length, 0., 0.);
  physiSiPM =
    new(std::nothrow) G4PVPlacement(t, logicSiPM, SiPM_Name, mlv, false,
                                    curvol.strip, false);
  if (physiSiPM == NULL)
    B2FATAL(MemErr);
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
  m_sensitive =
    new(std::nothrow) EKLMSensitiveDetector("EKLMSensitiveDetector");
  if (m_sensitive == NULL)
    B2FATAL(MemErr);
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

