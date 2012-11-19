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
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <eklm/geoeklm/G4TriangularPrism.h>
#include <eklm/simeklm/EKLMSensitiveDetector.h>

#include <eklm/geoeklm/StructureEndcap.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>
#include <boost/format.hpp>

#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <geometry/utilities.h>

#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Polyhedra.hh>
#include <G4PVPlacement.hh>
#include <G4Transform3D.hh>
#include <G4ReflectedSolid.hh>

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <eklm/geoeklm/EKLMTransformationFactory.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

/* Register the creator */
geometry::CreatorFactory<GeoEKLMBelleII> GeoEKLMFactory("EKLMBelleII");

GeoEKLMBelleII::GeoEKLMBelleII()
{
  SectorSupportSize.CornerAngle = -1;
}

GeoEKLMBelleII::~GeoEKLMBelleII()
{
  int i, j;
  for (i = 0; i < nPlane; i++) {
    for (j = 0; j < nBoard; j++)
      delete BoardTransform[i][j];
    free(BoardTransform[i]);
    free(BoardPosition[i]);
    free(SectionSupportPosition[i]);
  }
  free(StripPosition);
  free(StripBoardPosition);
  delete m_sensitive;
  freeSolids();
}

void GeoEKLMBelleII::mallocSolids()
{
  int i;
  solids.list = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.list == NULL)
    B2FATAL(MemErr);
  solids.stripvol = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.stripvol == NULL)
    B2FATAL(MemErr);
  solids.strip = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.strip == NULL)
    B2FATAL(MemErr);
  solids.groove = (G4Box**)malloc(nStrip * sizeof(G4Box*));
  if (solids.groove == NULL)
    B2FATAL(MemErr);
  solids.scint = (struct EKLMScintillatorSolids*)
                 malloc(nStrip * sizeof(struct EKLMScintillatorSolids));
  if (solids.scint == NULL)
    B2FATAL(MemErr);
  solids.plane = (struct EKLMPlaneSolids*)
                 malloc(nPlane * sizeof(struct EKLMPlaneSolids));
  if (solids.plane == NULL)
    B2FATAL(MemErr);
  solids.secsup = (struct EKLMSectionSupportSolids**)
                  malloc(nPlane * sizeof(struct EKLMSectionSupportSolids*));
  if (solids.secsup == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < nPlane; i++) {
    solids.secsup[i] = (struct EKLMSectionSupportSolids*)
                       malloc((nSection + 1) *
                              sizeof(struct EKLMSectionSupportSolids));
    if (solids.plane == NULL)
      B2FATAL(MemErr);
  }
  for (i = 0; i < nStrip; i++) {
    solids.list[i] = NULL;
    solids.stripvol[i] = NULL;
    solids.strip[i] = NULL;
    solids.groove[i] = NULL;
  }
  memset(solids.scint, 0, nStrip * sizeof(struct EKLMScintillatorSolids));
  memset(solids.plane, 0, nPlane * sizeof(struct EKLMPlaneSolids));
  for (i = 0; i < nPlane; i++)
    memset(solids.secsup[i], 0,
           (nSection + 1) * sizeof(struct EKLMSectionSupportSolids));
}

void GeoEKLMBelleII::freeSolids()
{
  int i;
  free(solids.list);
  free(solids.stripvol);
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

void GeoEKLMBelleII::createMaterials()
{
  mat.air = geometry::Materials::get("Air");
  mat.polystyrene = geometry::Materials::get("EKLMPolystyrene");
  mat.polystyrol = geometry::Materials::get("EKLMPolystyrol");
  mat.gel = geometry::Materials::get("EKLMGel");
  mat.iron = geometry::Materials::get("EKLMIron");
  mat.duralumin = geometry::Materials::get("EKLMDuralumin");
  mat.silicon = geometry::Materials::get("EKLMSilicon");
}

void GeoEKLMBelleII::readPositionData(struct EKLMElementPosition& epos,
                                      GearDir& content)
{
  epos.innerR = content.getLength("InnerR") * cm;
  epos.outerR = content.getLength("OuterR") * cm;
  epos.length = content.getLength("Length") * cm;
  epos.X = content.getLength("PositionX") * cm;
  epos.Y = content.getLength("PositionY") * cm;
  epos.Z = content.getLength("PositionZ") * cm;
}

void GeoEKLMBelleII::readXMLData(const GearDir& content)
{
  int i;
  int j;
  GearDir gd(content);
  m_mode = (enum EKLMDetectorMode)gd.getInt("Mode");
  if (m_mode < 0 || m_mode > 2)
    B2FATAL("EKLM started with unknown geometry mode " << m_mode << ".");
  m_outputFile = gd.getString("StripLengthAndTransformationMatrixDBFile");
  GearDir EndCap(gd);
  EndCap.append("/EndCap");
  readPositionData(EndcapPosition, EndCap);
  EndcapPosition.Z = EndcapPosition.Z + EndcapPosition.length / 2.0;
  nLayer = EndCap.getInt("nLayer");
  GearDir Layer(EndCap);
  Layer.append("/Layer");
  readPositionData(LayerPosition, Layer);
  Layer_shiftZ = Layer.getLength("ShiftZ") * cm;
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readPositionData(SectorPosition, Sector);
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
    BoardPosition[j] = (struct EKLMBoardPosition*)
                       malloc(nBoard * sizeof(struct EKLMBoardPosition));
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
  StripBoardPosition = (struct EKLMStripBoardPosition*)
                       malloc(nStripBoard *
                              sizeof(struct EKLMStripBoardPosition));
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
  readPositionData(SectorSupportPosition, SectorSupport);
  SectorSupportSize.Thickness = SectorSupport.getLength("Thickness") * cm;
  SectorSupportSize.DeltaLY = SectorSupport.getLength("DeltaLY") * cm;
  SectorSupportSize.CornerX = SectorSupport.getLength("CornerX") * cm;
  SectorSupportSize.TopCornerHeight = SectorSupport.
                                      getLength("TopCornerHeight") * cm;
  SectorSupportSize.Corner1LX = SectorSupport.getLength("Corner1LX") * cm;
  SectorSupportSize.Corner1Width = SectorSupport.getLength("Corner1Width") * cm;
  SectorSupportSize.Corner1Thickness = SectorSupport.
                                       getLength("Corner1Thickness") * cm;
  SectorSupportSize.Corner1Z = SectorSupport.getLength("Corner1Z") * cm;
  SectorSupportSize.Corner2LX = SectorSupport.getLength("Corner2LX") * cm;
  SectorSupportSize.Corner2LY = SectorSupport.getLength("Corner2LY") * cm;
  SectorSupportSize.Corner2Thickness = SectorSupport.
                                       getLength("Corner2Thickness") * cm;
  SectorSupportSize.Corner2Z = SectorSupport.getLength("Corner2Z") * cm;
  SectorSupportSize.Corner3LX = SectorSupport.getLength("Corner3LX") * cm;
  SectorSupportSize.Corner3LY = SectorSupport.getLength("Corner3LY") * cm;
  SectorSupportSize.Corner3Thickness = SectorSupport.
                                       getLength("Corner3Thickness") * cm;
  SectorSupportSize.Corner3Z = SectorSupport.getLength("Corner3Z") * cm;
  SectorSupportSize.Corner4LX = SectorSupport.getLength("Corner4LX") * cm;
  SectorSupportSize.Corner4LY = SectorSupport.getLength("Corner4LY") * cm;
  SectorSupportSize.Corner4Thickness = SectorSupport.
                                       getLength("Corner4Thickness") * cm;
  SectorSupportSize.Corner4Z = SectorSupport.getLength("Corner4Z") * cm;
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(PlanePosition, Plane);
  nSection = Plane.getInt("nSection");
  PlasticListWidth = Plane.getLength("PlasticListWidth") * cm;
  PlasticListDeltaL = Plane.getLength("PlasticListDeltaL") * cm;
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
  StripPosition = (struct EKLMElementPosition*)
                  malloc(nStrip * sizeof(struct EKLMElementPosition));
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
    SectionSupportPosition[j] = (struct EKLMSectionSupportPosition*)
                                malloc((nSection + 1) *
                                       sizeof(struct EKLMSectionSupportPosition));
    if (SectionSupportPosition[j] == NULL)
      B2FATAL(MemErr);
    for (i = 0; i <= nSection; i++) {
      GearDir SectionSupportContent(Sections);
      SectionSupportContent.append((boost::format(
                                      "/SectionSupportData[%1%]") % (j + 1)).str());
      SectionSupportContent.append((boost::format(
                                      "/SectionSupport[%1%]") % (i +  1)).str());
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
}

void GeoEKLMBelleII::createEndcap(int iEndcap, G4LogicalVolume* mlv)
{
  int i;
  double z;
  double phi;
  double dphi;
  int nsides;
  int nBoundary;
  double zsub;
  double rminsub;
  double rmaxsub;
  G4Polyhedra* boct;
  G4Tubs* atube;
  G4SubtractionSolid* solidEndcap;
  G4LogicalVolume* logicEndcap;
  G4PVPlacementGT* physiEndcap;
  G4Transform3D t;
  StructureEndcap EndcapMgr;
  EndcapMgr.read();
  phi = EndcapMgr.phi();
  dphi = EndcapMgr.dphi();
  nsides = EndcapMgr.nsides();
  nBoundary = EndcapMgr.nBoundary();
  double struct_z[nBoundary];
  double rmin[nBoundary];
  double rmax[nBoundary];
  for (int izBoundary  = 0; izBoundary < nBoundary; izBoundary++) {
    struct_z[izBoundary] = EndcapMgr.z(izBoundary);
    rmin[izBoundary] = EndcapMgr.rmin(izBoundary);
    rmax[izBoundary] = EndcapMgr.rmax(izBoundary);
  }
  //m_matnamesub = EndcapMgr.matnamesub();
  zsub = EndcapMgr.zsub();
  rminsub = EndcapMgr.rminsub();
  rmaxsub = EndcapMgr.rmaxsub();
  std::string Endcap_Name = "Endcap_" + boost::lexical_cast<std::string>(iEndcap);
  boct = new G4Polyhedra("tempoct", phi, dphi, nsides, nBoundary, struct_z,
                         rmin, rmax);
  atube = new G4Tubs("tempatube", rminsub, rmaxsub, zsub, 0.0, 360.0 * deg);
  solidEndcap = new G4SubtractionSolid(Endcap_Name, boct, atube);
  if (solidEndcap == NULL)
    B2FATAL(MemErr);
  if (iEndcap == 1)
    z = -EndcapPosition.Z + 94.0 * cm;
  else
    z = EndcapPosition.Z;
  t = G4Translate3D(EndcapPosition.X, EndcapPosition.Y, z);
  logicEndcap = new G4LogicalVolume(solidEndcap, mat.iron, Endcap_Name);
  if (logicEndcap == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicEndcap, true);
  geometry::setColor(*logicEndcap, "#ffffff22");
  physiEndcap = new G4PVPlacementGT(t, t, logicEndcap, Endcap_Name, mlv,
                                    iEndcap);
  if (physiEndcap == NULL)
    B2FATAL(MemErr);
  for (i = 1; i <= nLayer; i++)
    createLayer(i, iEndcap, physiEndcap);
}

void GeoEKLMBelleII::createLayer(int iLayer, int iEndcap,
                                 G4PVPlacementGT* mpvgt)
{
  int i;
  static G4Tubs* solidLayer = NULL;
  G4LogicalVolume* logicLayer;
  G4PVPlacementGT* physiLayer;
  G4Transform3D t;
  std::string Layer_Name = "Layer_" + boost::lexical_cast<std::string>(iLayer) + "_" +
                           mpvgt->GetName();
  if (solidLayer == NULL)
    solidLayer = new G4Tubs(Layer_Name, LayerPosition.innerR,
                            LayerPosition.outerR, LayerPosition.length / 2.0,
                            0. * deg, 360. * deg);
  if (solidLayer == NULL)
    B2FATAL(MemErr);
  logicLayer = new G4LogicalVolume(solidLayer, mat.air, Layer_Name);
  if (logicLayer == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicLayer, false);
  LayerPosition.Z = -EndcapPosition.length / 2.0 + iLayer * Layer_shiftZ +
                    0.5 * LayerPosition.length;
  if (iEndcap == 1)
    LayerPosition.Z = -LayerPosition.Z;
  t = G4Translate3D(0.0, 0.0, LayerPosition.Z);
  physiLayer = new G4PVPlacementGT(mpvgt, t, logicLayer, Layer_Name, iLayer);
  if (physiLayer == NULL)
    B2FATAL(MemErr);
  for (i = 1; i <= 4; i++)
    createSector(i, physiLayer);
}

void GeoEKLMBelleII::createSector(int iSector, G4PVPlacementGT* mpvgt)
{
  int i;
  int j;
  static G4Tubs* solidSector = NULL;
  G4LogicalVolume* logicSector;
  G4PVPlacementGT* physiSector;
  G4Transform3D t;
  std::string Sector_Name = "Sector_" + boost::lexical_cast<std::string>(iSector) + "_" +
                            mpvgt->GetName();
  if (solidSector == NULL)
    solidSector = new G4Tubs(Sector_Name, SectorPosition.innerR,
                             SectorPosition.outerR, 0.5 * SectorPosition.length,
                             0.0, 90.0 * deg);
  if (solidSector == NULL)
    B2FATAL(MemErr);
  logicSector = new G4LogicalVolume(solidSector, mat.air, Sector_Name);
  if (logicSector == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSector, false);
  switch (iSector) {
    case 1:
      t = G4Translate3D(0., 0., 0.);
      break;
    case 2:
      t = G4RotateY3D(180.0 * deg);
      break;
    case 3:
      t = G4RotateZ3D(90.0 * deg) * G4RotateY3D(180.0 * deg);
      break;
    case 4:
      t = G4RotateZ3D(-90.0 * deg);
      break;
  }
  physiSector = new G4PVPlacementGT(mpvgt, t, logicSector, Sector_Name,
                                    iSector);
  if (physiSector == NULL)
    B2FATAL(MemErr);
  createSectorSupport(physiSector);
  for (i = 1; i <= 2; i++)
    createSectorCover(i, physiSector);
  calcBoardTransform();
  for (i = 1; i <= nPlane; i++)
    createPlane(i, physiSector);
  for (i = 1; i <= nPlane; i++)
    for (j = 1; j <= nBoard; j++)
      createSectionReadoutBoard(i, j, physiSector);
}

void GeoEKLMBelleII::calcBoardTransform()
{
  int i;
  int j;
  for (i = 0; i < nPlane; i++) {
    BoardTransform[i] = (G4Transform3D**)
                        malloc(sizeof(G4Transform3D*) * nBoard);
    if (BoardTransform[i] == NULL)
      B2FATAL(MemErr);
    for (j = 0; j < nBoard; j++) {
      BoardTransform[i][j] = new G4Transform3D(
        G4RotateZ3D(BoardPosition[i][j].phi) *
        G4Translate3D(BoardPosition[i][j].r -
                      0.5 * BoardSize.height, 0., 0.) *
        G4RotateZ3D(90.0 * deg));
    }
  }
}

void GeoEKLMBelleII::createSectorCover(int iCover, G4PVPlacementGT* mpvgt)
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
  G4PVPlacementGT* physiCover;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t;
  std::string Cover_Name = "Cover_" + boost::lexical_cast<std::string>(iCover) + "_" +
                           mpvgt->GetName();
  if (solidCover == NULL) {
    lz = 0.5 * (SectorPosition.length - SectorSupportPosition.length);
    solidCoverTube = new G4Tubs("Tube_" + Cover_Name,
                                SectorSupportPosition.innerR,
                                SectorSupportPosition.outerR, 0.5 * lz, 0.0,
                                90.0 * deg);
    solidCoverBox = new G4Box("Box_" + Cover_Name,
                              0.5 * SectorSupportPosition.outerR,
                              0.5 * SectorSupportPosition.outerR,
                              0.5 * lz);
    if (solidCoverTube == NULL || solidCoverBox == NULL)
      B2FATAL(MemErr);
    box = new G4Box("SubtractionBox_" + Cover_Name,
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
    is = new G4IntersectionSolid("Intersection_" + Cover_Name, solidCoverTube,
                                 solidCoverBox, t1);
    if (is == NULL)
      B2FATAL(MemErr);
    solidCover = new G4SubtractionSolid(Cover_Name, is, box, t2);
  }
  if (solidCover == NULL)
    B2FATAL(MemErr);
  logicCover = new G4LogicalVolume(solidCover, mat.duralumin, Cover_Name);
  if (logicCover == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCover, false);
  geometry::setColor(*logicCover, "#ff000022");
  z = 0.25 * (SectorPosition.length + SectorSupportPosition.length);
  if (iCover == 2)
    z = -z;
  t = G4Translate3D(0., 0., z);
  physiCover = new G4PVPlacementGT(mpvgt, t, logicCover, Cover_Name, iCover);
  if (physiCover == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicCover);
}

G4Box* GeoEKLMBelleII::createSectorSupportBoxX(G4PVPlacementGT* mpvgt,
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
  return new G4Box("BoxX_Support_" + mpvgt->GetName(), 0.5 * (x2 - x1),
                   0.5 * SectorSupportSize.Thickness,
                   0.5 * SectorSupportPosition.length);
}

G4Box* GeoEKLMBelleII::createSectorSupportBoxY(G4PVPlacementGT* mpvgt,
                                               G4Transform3D& t)
{
  double y1;
  double y2;
  y1 = sqrt(SectorSupportPosition.innerR * SectorSupportPosition.innerR -
            SectorSupportPosition.X * SectorSupportPosition.X);
  y2 = SectorSupportPosition.outerR - SectorSupportSize.DeltaLY;
  t = G4Translate3D(SectorSupportPosition.X + 0.5 * SectorSupportSize.Thickness,
                    0.5 * (y1 + y2), 0.) * G4RotateZ3D(90. * deg);
  return new G4Box("BoxY_Support_" + mpvgt->GetName(), 0.5 * (y2 - y1),
                   0.5 * SectorSupportSize.Thickness,
                   0.5 * SectorSupportPosition.length);
}

double GeoEKLMBelleII::getSectorSupportCornerAngle()
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

G4Box* GeoEKLMBelleII::createSectorSupportBoxTop(G4PVPlacementGT* mpvgt,
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
  return new G4Box("BoxTop_Support_" + mpvgt->GetName(), 0.5 * sqrt((x2 - x1) *
                   (x2 - x1) + (y2 - y1) * (y2 - y1)),
                   0.5 * SectorSupportSize.Thickness,
                   0.5 * SectorSupportPosition.length);
}

G4Tubs* GeoEKLMBelleII::createSectorSupportInnerTube(G4PVPlacementGT* mpvgt)
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
  return new G4Tubs("InnerTube_Support_" + mpvgt->GetName(),
                    SectorSupportPosition.innerR,
                    SectorSupportPosition.innerR + SectorSupportSize.Thickness,
                    0.5 * SectorSupportPosition.length,
                    std::min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
}

G4Tubs* GeoEKLMBelleII::createSectorSupportOuterTube(G4PVPlacementGT* mpvgt)
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
  return new G4Tubs("OuterTube_Support" + mpvgt->GetName(),
                    SectorSupportPosition.outerR - SectorSupportSize.Thickness,
                    SectorSupportPosition.outerR,
                    0.5 * SectorSupportPosition.length,
                    std::min(ang1, ang2) * rad, fabs(ang1 - ang2) * rad);
}

void GeoEKLMBelleII::createSectorSupportCorner1(G4PVPlacementGT* mpvgt)
{
  double lx;
  double x;
  static G4Tubs* solidCorner1Tube;
  static G4Box* solidCorner1Box1;
  static G4Box* solidCorner1Box2;
  static G4IntersectionSolid* is1;
  static G4IntersectionSolid* solidCorner1 = NULL;
  G4LogicalVolume* logicCorner1;
  G4PVPlacementGT* physiCorner1;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  std::string Corner1_Name = "Corner1_" + mpvgt->GetName();
  if (solidCorner1 == NULL) {
    lx = SectorSupportSize.CornerX + SectorSupportSize.Corner1LX -
         SectorSupportSize.Thickness;
    solidCorner1Tube = new G4Tubs("Tube_" + Corner1_Name, 0.,
                                  SectorSupportPosition.outerR -
                                  SectorSupportSize.Thickness,
                                  0.5 * SectorSupportSize.Corner1Thickness,
                                  0., 90. * deg);
    solidCorner1Box1 = new G4Box("Box1_" + Corner1_Name, 0.5 * lx,
                                 0.5 * SectorSupportPosition.outerR,
                                 0.5 * SectorSupportSize.Corner1Thickness);
    solidCorner1Box2 = new G4Box("Box2_" + Corner1_Name,
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
    is1 = new G4IntersectionSolid("Intersection1_" + Corner1_Name,
                                  solidCorner1Tube, solidCorner1Box1, t1);
    if (is1 == NULL)
      B2FATAL(MemErr);
    solidCorner1 = new G4IntersectionSolid(Corner1_Name, is1, solidCorner1Box2,
                                           t2);
  }
  if (solidCorner1 == NULL)
    B2FATAL(MemErr);
  logicCorner1 = new G4LogicalVolume(solidCorner1, mat.duralumin, Corner1_Name);
  if (logicCorner1 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner1, true);
  geometry::setColor(*logicCorner1, "#ff0000ff");
  t = G4Translate3D(0., 0., SectorSupportSize.Corner1Z);
  physiCorner1 = new G4PVPlacementGT(mpvgt, t, logicCorner1, Corner1_Name);
  if (physiCorner1 == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicCorner1);
}

void GeoEKLMBelleII::createSectorSupportCorner2(G4PVPlacementGT* mpvgt)
{
  static double r;
  static double x;
  static double y;
  static G4TriangularPrism* solidCorner2Prism;
  static G4Tubs* solidCorner2Tubs;
  static G4SubtractionSolid* solidCorner2 = NULL;
  G4LogicalVolume* logicCorner2;
  G4PVPlacementGT* physiCorner2;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner2_Name = "Corner2_" + mpvgt->GetName();
  if (solidCorner2 == NULL) {
    r = SectorSupportPosition.outerR - SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    x = sqrt(r * r - y * y);
    solidCorner2Prism = new G4TriangularPrism("Prism_" + Corner2_Name,
                                              SectorSupportSize.Corner2LY,
                                              90. * deg,
                                              SectorSupportSize.Corner2LX,
                                              180. * deg,
                                              0.5 * SectorSupportSize.
                                              Corner2Thickness);
    solidCorner2Tubs = createSectorSupportOuterTube(mpvgt);
    if (solidCorner2Prism == NULL || solidCorner2Tubs == NULL)
      B2FATAL(MemErr);
    if (solidCorner2Prism->getSolid() == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(-x, -y, 0.);
    solidCorner2 = new G4SubtractionSolid(Corner2_Name,
                                          solidCorner2Prism->getSolid(),
                                          solidCorner2Tubs, t1);
    delete solidCorner2Prism;
  }
  if (solidCorner2 == NULL)
    B2FATAL(MemErr);
  logicCorner2 = new G4LogicalVolume(solidCorner2, mat.duralumin, Corner2_Name);
  if (logicCorner2 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner2, true);
  geometry::setColor(*logicCorner2, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner2Z);
  physiCorner2 = new G4PVPlacementGT(mpvgt, t, logicCorner2, Corner2_Name);
  if (physiCorner2 == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicCorner2);
}

void GeoEKLMBelleII::createSectorSupportCorner3(G4PVPlacementGT* mpvgt)
{
  static double r;
  static double x;
  static double y;
  static G4TriangularPrism* solidCorner3Prism;
  static G4Tubs* solidCorner3Tubs;
  static G4SubtractionSolid* solidCorner3 = NULL;
  G4LogicalVolume* logicCorner3;
  G4PVPlacementGT* physiCorner3;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner3_Name = "Corner3_" + mpvgt->GetName();
  if (solidCorner3 == NULL) {
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
        SectorSupportSize.Corner3LY;
    x = sqrt(r * r - y * y);
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    solidCorner3Prism = new G4TriangularPrism("Prism_" + Corner3_Name,
                                              SectorSupportSize.Corner3LX, 0.,
                                              SectorSupportSize.Corner3LY,
                                              90. * deg,
                                              0.5 * SectorSupportSize.
                                              Corner3Thickness);
    solidCorner3Tubs = createSectorSupportInnerTube(mpvgt);
    if (solidCorner3Prism == NULL || solidCorner3Tubs == NULL)
      B2FATAL(MemErr);
    if (solidCorner3Prism->getSolid() == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(-x, -y, 0.);
    solidCorner3 = new G4SubtractionSolid(Corner3_Name,
                                          solidCorner3Prism->getSolid(),
                                          solidCorner3Tubs, t1);
    delete solidCorner3Prism;
  }
  if (solidCorner3 == NULL)
    B2FATAL(MemErr);
  logicCorner3 = new G4LogicalVolume(solidCorner3, mat.duralumin, Corner3_Name);
  if (logicCorner3 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner3, true);
  geometry::setColor(*logicCorner3, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner3Z);
  physiCorner3 = new G4PVPlacementGT(mpvgt, t, logicCorner3, Corner3_Name);
  if (physiCorner3 == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicCorner3);
}

void GeoEKLMBelleII::createSectorSupportCorner4(G4PVPlacementGT* mpvgt)
{
  static double r;
  static double x;
  static double y;
  static G4TriangularPrism* solidCorner4Prism;
  static G4Tubs* solidCorner4Tubs;
  static G4SubtractionSolid* solidCorner4 = NULL;
  G4LogicalVolume* logicCorner4;
  G4PVPlacementGT* physiCorner4;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Corner4_Name = "Corner4_" + mpvgt->GetName();
  if (solidCorner4 == NULL) {
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    x = SectorSupportPosition.X + SectorSupportSize.Thickness +
        SectorSupportSize.Corner4LX;
    y = sqrt(r * r - x * x);
    x = SectorSupportPosition.X + SectorSupportSize.Thickness;
    solidCorner4Prism = new G4TriangularPrism("Corner4_" + mpvgt->GetName(),
                                              SectorSupportSize.Corner4LX, 0.,
                                              SectorSupportSize.Corner4LY,
                                              90. * deg,
                                              0.5 * SectorSupportSize.
                                              Corner4Thickness);
    solidCorner4Tubs = createSectorSupportInnerTube(mpvgt);
    if (solidCorner4Prism == NULL || solidCorner4Tubs == NULL)
      B2FATAL(MemErr);
    if (solidCorner4Prism->getSolid() == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(-x, -y, 0.);
    solidCorner4 = new G4SubtractionSolid(Corner4_Name,
                                          solidCorner4Prism->getSolid(),
                                          solidCorner4Tubs, t1);
    delete solidCorner4Prism;
  }
  if (solidCorner4 == NULL)
    B2FATAL(MemErr);
  logicCorner4 = new G4LogicalVolume(solidCorner4, mat.duralumin, Corner4_Name);
  if (logicCorner4 == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicCorner4, true);
  geometry::setColor(*logicCorner4, "#ff0000ff");
  t = G4Translate3D(x, y, SectorSupportSize.Corner4Z);
  physiCorner4 = new G4PVPlacementGT(mpvgt, t, logicCorner4, Corner4_Name);
  if (physiCorner4 == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicCorner4);
}

void GeoEKLMBelleII::createSectorSupport(G4PVPlacementGT* mpvgt)
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
  G4PVPlacementGT* physiSectorSupport;
  G4Transform3D t;
  G4Transform3D tbx;
  G4Transform3D tby;
  G4Transform3D tbt;
  std::string SectorSupportName = "Support_" + mpvgt->GetName();
  if (solidSectorSupport == NULL) {
    solidBoxX = createSectorSupportBoxX(mpvgt, tbx);
    solidBoxY = createSectorSupportBoxY(mpvgt, tby);
    solidBoxTop = createSectorSupportBoxTop(mpvgt, tbt);
    solidOuterTube = createSectorSupportOuterTube(mpvgt);
    solidInnerTube = createSectorSupportInnerTube(mpvgt);
    solidLimitationTube = new G4Tubs("LimitationTube_" + SectorSupportName,
                                     0., SectorSupportPosition.outerR,
                                     0.5 * SectorSupportPosition.length,
                                     0., 90.*deg);
    if (solidBoxX == NULL || solidBoxY == NULL || solidOuterTube == NULL ||
        solidInnerTube == NULL || solidLimitationTube == NULL)
      B2FATAL(MemErr);
    t = G4Translate3D(0., 0., SectorSupportPosition.Z);
    us1 = new G4UnionSolid("Union1_" + SectorSupportName, solidInnerTube,
                           solidBoxY, tby);
    if (us1 == NULL)
      B2FATAL(MemErr);
    us2 = new G4UnionSolid("Union2_" + SectorSupportName, us1, solidBoxX,
                           tbx);
    if (us2 == NULL)
      B2FATAL(MemErr);
    us3 = new G4UnionSolid("Union3_" + SectorSupportName, us2,
                           solidOuterTube, G4Translate3D(0., 0., 0.));
    if (us3 == NULL)
      B2FATAL(MemErr);
    us4 = new G4UnionSolid("Union4_" + SectorSupportName,
                           us3, solidBoxTop, tbt);
    if (us4 == NULL)
      B2FATAL(MemErr);
    solidSectorSupport = new G4IntersectionSolid(SectorSupportName, us4,
                                                 solidLimitationTube,
                                                 G4Translate3D(0., 0., 0.));
  }
  if (solidSectorSupport == NULL)
    B2FATAL(MemErr);
  logicSectorSupport = new G4LogicalVolume(solidSectorSupport, mat.duralumin,
                                           SectorSupportName);
  if (logicSectorSupport == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSectorSupport, true);
  geometry::setColor(*logicSectorSupport, "#ff0000ff");
  physiSectorSupport = new G4PVPlacementGT(mpvgt, t, logicSectorSupport,
                                           SectorSupportName);
  if (physiSectorSupport == NULL)
    B2FATAL(MemErr);
  createSectorSupportCorner1(mpvgt);
  createSectorSupportCorner2(mpvgt);
  createSectorSupportCorner3(mpvgt);
  createSectorSupportCorner4(mpvgt);
  printVolumeMass(logicSectorSupport);
}

G4SubtractionSolid* GeoEKLMBelleII::
subtractBoardSolids(G4SubtractionSolid* plane, int iPlane,
                    std::string Plane_Name)
{
  int i;
  int j;
  G4Transform3D t;
  G4Box* solidBoardBox;
  G4SubtractionSolid** ss[2];
  G4SubtractionSolid* prev_solid;
  solidBoardBox = new G4Box("PlateBox_" + Plane_Name, 0.5 * BoardSize.length,
                            0.5 * BoardSize.height,
                            0.5 * (PlanePosition.length + PlanePosition.Z));
  for (i = 0; i < nPlane; i++) {
    ss[i] = new G4SubtractionSolid*[nBoard];
    if (ss[i] == NULL)
      return NULL;
    for (j = 0; j < nBoard; j++) {
      t = *BoardTransform[i][j];
      if (iPlane == 2)
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
      ss[i][j] = new G4SubtractionSolid("BoardSubtraction_" +
                                        boost::lexical_cast<std::string>(i) + "_" +
                                        boost::lexical_cast<std::string>(j) + Plane_Name,
                                        prev_solid, solidBoardBox, t);
    }
  }
  return ss[nPlane - 1][nBoard - 1];
}

void GeoEKLMBelleII::createPlane(int iPlane, G4PVPlacementGT* mpvgt)
{
  int i;
  int j;
  double r;
  double x;
  double y;
  double box_x;
  double box_lx;
  double ang;
  G4LogicalVolume* logicPlane;
  G4PVPlacementGT* physiPlane;
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4Transform3D t3;
  G4Transform3D t4;
  G4Transform3D t5;
  std::string Plane_Name = "Plane_" + boost::lexical_cast<std::string>(iPlane) + "_" +
                           mpvgt->GetName();
  if (solids.plane[iPlane - 1].plane == NULL) {
    solids.plane[iPlane - 1].tube = new G4Tubs("Tube_" + Plane_Name, PlanePosition.innerR,
                                               PlanePosition.outerR, 0.5 * PlanePosition.length,
                                               0.0, 90.0 * deg);
    box_x = std::max(SectorSupportPosition.Y, SectorSupportPosition.X) +
            SectorSupportSize.Thickness;
    box_lx =  PlanePosition.outerR - box_x;
    solids.plane[iPlane - 1].box1 = new G4Box("Box_" + Plane_Name, 0.5 * box_lx, 0.5 * box_lx,
                                              0.5 * PlanePosition.length);
    solids.plane[iPlane - 1].prism1 = new G4TriangularPrism("TriangularPrism1_" + Plane_Name,
                                                            SectorSupportSize.Corner2LY,
                                                            90. * deg,
                                                            SectorSupportSize.Corner2LX,
                                                            180. * deg, PlanePosition.length);
    solids.plane[iPlane - 1].prism2 = new G4TriangularPrism("TriangularPrism2_" + Plane_Name,
                                                            SectorSupportSize.Corner3LX, 0.,
                                                            SectorSupportSize.Corner3LY,
                                                            90. * deg, PlanePosition.length);
    solids.plane[iPlane - 1].prism3 = new G4TriangularPrism("TriangularPrism3_" + Plane_Name,
                                                            SectorSupportSize.Corner4LX, 0.,
                                                            SectorSupportSize.Corner4LY,
                                                            90. * deg, PlanePosition.length);
    if (solids.plane[iPlane - 1].tube == NULL || solids.plane[iPlane - 1].box1 == NULL ||
        solids.plane[iPlane - 1].prism1 == NULL || solids.plane[iPlane - 1].prism2 == NULL ||
        solids.plane[iPlane - 1].prism3 == NULL)
      B2FATAL(MemErr);
    if (solids.plane[iPlane - 1].prism1->getSolid() == NULL ||
        solids.plane[iPlane - 1].prism2->getSolid() == NULL ||
        solids.plane[iPlane - 1].prism3->getSolid() == NULL)
      B2FATAL(MemErr);
    solids.plane[iPlane - 1].box2 = new G4Box("Box1_" + Plane_Name, 0.5 * box_lx, 0.5 * box_lx,
                                              PlanePosition.length);
    if (solids.plane[iPlane - 1].box2 == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(0.5 * (PlanePosition.outerR + box_x),
                       0.5 * (PlanePosition.outerR + box_x), 0.);
    if (iPlane == 2)
      t1 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t1;
    ang = getSectorSupportCornerAngle();
    x = std::max(SectorSupportPosition.Y, SectorSupportPosition.X);
    y = SectorSupportPosition.outerR -
        SectorSupportSize.DeltaLY -
        SectorSupportSize.TopCornerHeight;
    if (iPlane == 1) {
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
    if (iPlane == 2)
      t3 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t3;
    r = SectorSupportPosition.innerR + SectorSupportSize.Thickness;
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness +
        SectorSupportSize.Corner3LY;
    x = sqrt(r * r - y * y);
    y = SectorSupportPosition.Y + SectorSupportSize.Thickness;
    t4 = G4Translate3D(x, y, 0.);
    if (iPlane == 2)
      t4 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t4;
    x = SectorSupportPosition.X + SectorSupportSize.Thickness +
        SectorSupportSize.Corner4LX;
    y = sqrt(r * r - x * x);
    x = SectorSupportPosition.X + SectorSupportSize.Thickness;
    t5 = G4Translate3D(x, y, 0.);
    if (iPlane == 2)
      t5 = G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.)) * t5;
    solids.plane[iPlane - 1].is = new G4IntersectionSolid("Intersection_" + Plane_Name, solids.plane[iPlane - 1].tube,
                                                          solids.plane[iPlane - 1].box1, t1);
    if (solids.plane[iPlane - 1].is == NULL)
      B2FATAL(MemErr);
    solids.plane[iPlane - 1].ss1 = new G4SubtractionSolid("Subtraction1_" + Plane_Name, solids.plane[iPlane - 1].is, solids.plane[iPlane - 1].box2, t2);
    if (solids.plane[iPlane - 1].ss1 == NULL)
      B2FATAL(MemErr);
    solids.plane[iPlane - 1].ss2 = new G4SubtractionSolid("Subtraction2_" + Plane_Name, solids.plane[iPlane - 1].ss1,
                                                          solids.plane[iPlane - 1].prism1->getSolid(), t3);
    if (solids.plane[iPlane - 1].ss2 == NULL)
      B2FATAL(MemErr);
    solids.plane[iPlane - 1].ss3 = new G4SubtractionSolid("Subtraction3_" + Plane_Name, solids.plane[iPlane - 1].ss2,
                                                          solids.plane[iPlane - 1].prism2->getSolid(), t4);
    if (solids.plane[iPlane - 1].ss3 == NULL)
      B2FATAL(MemErr);
    solids.plane[iPlane - 1].ss4 = new G4SubtractionSolid("Subtraction4_" + Plane_Name, solids.plane[iPlane - 1].ss3,
                                                          solids.plane[iPlane - 1].prism3->getSolid(), t5);
    if (solids.plane[iPlane - 1].ss4 == NULL)
      B2FATAL(MemErr);
    solids.plane[iPlane - 1].plane = subtractBoardSolids(solids.plane[iPlane - 1].ss4, iPlane, Plane_Name);
  }
  if (solids.plane[iPlane - 1].plane == NULL)
    B2FATAL(MemErr);
  logicPlane = new G4LogicalVolume(solids.plane[iPlane - 1].plane, mat.air, Plane_Name);
  if (logicPlane == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicPlane, false);
  if (iPlane == 1) {
    t = G4Translate3D(PlanePosition.X, PlanePosition.Y, PlanePosition.Z);
  } else {
    t = G4Translate3D(PlanePosition.X, PlanePosition.Y, -PlanePosition.Z) *
        G4Rotate3D(180. * deg, G4ThreeVector(1., 1., 0.));
  }
  physiPlane = new G4PVPlacementGT(mpvgt, t, logicPlane, Plane_Name, iPlane);
  if (physiPlane == NULL)
    B2FATAL(MemErr);
  for (i = 1; i <= nSection + 1; i++)
    createSectionSupport(i, iPlane, physiPlane);
  for (i = 1; i <= 2; i++)
    for (j = 1; j <= nStrip; j++)
      createPlasticListElement(i, j, physiPlane);
  for (i = 1; i <= nStrip; i++)
    createStripVolume(i, physiPlane);
}

void GeoEKLMBelleII::createSectionReadoutBoard(int iPlane, int iBoard,
                                               G4PVPlacementGT* mpvgt)
{
  int i;
  static G4Box* solidSectionReadoutBoard = NULL;
  G4LogicalVolume* logicSectionReadoutBoard;
  G4PVPlacementGT* physiSectionReadoutBoard;
  std::string Board_Name = "SectionReadoutBoard_" +
                           boost::lexical_cast<std::string>(iBoard) + "_Plane_" +
                           boost::lexical_cast<std::string>(iPlane) + "_" +
                           mpvgt->GetName();
  if (solidSectionReadoutBoard == NULL)
    solidSectionReadoutBoard = new G4Box(Board_Name,
                                         0.5 * BoardSize.length,
                                         0.5 * BoardSize.height,
                                         0.5 * BoardSize.width);
  if (solidSectionReadoutBoard == NULL)
    B2FATAL(MemErr);
  logicSectionReadoutBoard = new G4LogicalVolume(solidSectionReadoutBoard, mat.air,
                                                 Board_Name);
  if (logicSectionReadoutBoard == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSectionReadoutBoard, false);
  physiSectionReadoutBoard = new G4PVPlacementGT(mpvgt,
                                                 *BoardTransform[iPlane - 1]
                                                 [iBoard - 1],
                                                 logicSectionReadoutBoard,
                                                 Board_Name,
                                                 10 * iPlane + iBoard);
  if (physiSectionReadoutBoard == NULL)
    B2FATAL(MemErr);
  createBaseBoard(physiSectionReadoutBoard);
  if (m_mode != EKLM_DETECTOR_NORMAL)
    for (i = 1; i <= nStripBoard; i++)
      createStripBoard(i, physiSectionReadoutBoard);
}

void GeoEKLMBelleII::createBaseBoard(G4PVPlacementGT* mpvgt)
{
  static G4Box* solidBaseBoard = NULL;
  G4LogicalVolume* logicBaseBoard;
  G4PVPlacementGT* physiBaseBoard;
  G4Transform3D t;
  std::string Board_Name = "BaseBoard_" + mpvgt->GetName();
  if (solidBaseBoard == NULL)
    solidBaseBoard = new G4Box(Board_Name, 0.5 * BoardSize.length,
                               0.5 * BoardSize.base_height,
                               0.5 * BoardSize.base_width);
  if (solidBaseBoard == NULL)
    B2FATAL(MemErr);
  logicBaseBoard = new G4LogicalVolume(solidBaseBoard, mat.silicon,
                                       Board_Name);
  if (logicBaseBoard == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicBaseBoard, true);
  geometry::setColor(*logicBaseBoard, "#0000ffff");
  t = G4Translate3D(0., -0.5 * BoardSize.height + 0.5 * BoardSize.base_height,
                    0.);
  physiBaseBoard = new G4PVPlacementGT(mpvgt, t, logicBaseBoard, Board_Name);
  if (physiBaseBoard == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicBaseBoard);
}

void GeoEKLMBelleII::createStripBoard(int iBoard, G4PVPlacementGT* mpvgt)
{
  static G4Box* solidStripBoard = NULL;
  G4LogicalVolume* logicStripBoard;
  G4PVPlacementGT* physiStripBoard;
  G4Transform3D t;
  std::string Board_Name = "StripBoard_" + boost::lexical_cast<std::string>(iBoard) + "_" +
                           mpvgt->GetName();
  if (solidStripBoard == NULL)
    solidStripBoard = new G4Box(Board_Name, 0.5 * BoardSize.strip_length,
                                0.5 * BoardSize.strip_height,
                                0.5 * BoardSize.strip_width);
  if (solidStripBoard == NULL)
    B2FATAL(MemErr);
  if (m_mode == EKLM_DETECTOR_NORMAL)
    logicStripBoard = new G4LogicalVolume(solidStripBoard, mat.silicon,
                                          Board_Name);
  else
    logicStripBoard = new G4LogicalVolume(solidStripBoard, mat.silicon,
                                          Board_Name, 0, m_sensitive, 0);
  if (logicStripBoard == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicStripBoard, true);
  geometry::setColor(*logicStripBoard, "#0000ffff");
  t = G4Translate3D(-0.5 * BoardSize.length + StripBoardPosition[iBoard - 1].x,
                    -0.5 * BoardSize.height + BoardSize.base_height +
                    0.5 * BoardSize.strip_height, 0.);
  physiStripBoard = new G4PVPlacementGT(mpvgt, t, logicStripBoard, Board_Name,
                                        iBoard, m_mode);
  if (physiStripBoard == NULL)
    B2FATAL(MemErr);
  physiStripBoard->setVolumeType(EKLM_SENSITIVE_BOARD);
  printVolumeMass(logicStripBoard);
}

void GeoEKLMBelleII::createSectionSupport(int iSectionSupport, int iPlane,
                                          G4PVPlacementGT* mpvgt)
{
  G4Transform3D t;
  G4Transform3D t1;
  G4Transform3D t2;
  G4LogicalVolume* logicSectionSupport;
  G4PVPlacementGT* physiSectionSupport;
  std::string SectionSupportName = "SectionSupport_" +
                                   boost::lexical_cast<std::string>(iSectionSupport) +
                                   "_" + mpvgt->GetName();
  if (solids.secsup[iPlane - 1][iSectionSupport - 1].secsup == NULL) {
    solids.secsup[iPlane - 1][iSectionSupport - 1].topbox = new G4Box("BoxTop_" + SectionSupportName,
        0.5 * (SectionSupportPosition[iPlane - 1]
               [iSectionSupport - 1].length -
               SectionSupportPosition[iPlane - 1]
               [iSectionSupport - 1].deltal_left -
               SectionSupportPosition[iPlane - 1]
               [iSectionSupport - 1].deltal_right),
        0.5 * SectionSupportTopWidth,
        0.5 * SectionSupportTopThickness);
    solids.secsup[iPlane - 1][iSectionSupport - 1].midbox =  new G4Box("BoxMiddle_" + SectionSupportName,
        0.5 * (SectionSupportPosition[iPlane - 1]
               [iSectionSupport - 1].length -
               SectionSupportPosition[iPlane - 1]
               [iSectionSupport - 1].deltal_left -
               SectionSupportPosition[iPlane - 1]
               [iSectionSupport - 1].deltal_right),
        0.5 * SectionSupportMiddleWidth,
        0.5 * SectionSupportMiddleThickness);
    solids.secsup[iPlane - 1][iSectionSupport - 1].botbox = new G4Box("BoxBottom_" + SectionSupportName,
        0.5 * SectionSupportPosition[iPlane - 1]
        [iSectionSupport - 1].length,
        0.5 * SectionSupportTopWidth,
        0.5 * SectionSupportTopThickness);
    if (solids.secsup[iPlane - 1][iSectionSupport - 1].topbox == NULL || solids.secsup[iPlane - 1][iSectionSupport - 1].midbox == NULL || solids.secsup[iPlane - 1][iSectionSupport - 1].botbox == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(0., 0., 0.5 * (SectionSupportMiddleThickness +
                                      SectionSupportTopThickness));
    t2 = G4Translate3D(0.5 * (SectionSupportPosition[iPlane - 1]
                              [iSectionSupport - 1].deltal_right -
                              SectionSupportPosition[iPlane - 1]
                              [iSectionSupport - 1].deltal_left),
                       0., -0.5 * (SectionSupportMiddleThickness +
                                   SectionSupportTopThickness));
    solids.secsup[iPlane - 1][iSectionSupport - 1].us = new G4UnionSolid("Union1_" + SectionSupportName, solids.secsup[iPlane - 1][iSectionSupport - 1].midbox,
        solids.secsup[iPlane - 1][iSectionSupport - 1].topbox, t1);
    if (solids.secsup[iPlane - 1][iSectionSupport - 1].us == NULL)
      B2FATAL(MemErr);
    solids.secsup[iPlane - 1][iSectionSupport - 1].secsup = new G4UnionSolid(SectionSupportName, solids.secsup[iPlane - 1][iSectionSupport - 1].us, solids.secsup[iPlane - 1][iSectionSupport - 1].botbox,
        t2);
  }
  if (solids.secsup[iPlane - 1][iSectionSupport - 1].secsup == NULL)
    B2FATAL(MemErr);
  logicSectionSupport = new G4LogicalVolume(solids.secsup[iPlane - 1][iSectionSupport - 1].secsup, mat.duralumin,
                                            SectionSupportName);
  if (logicSectionSupport == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSectionSupport, true);
  geometry::setColor(*logicSectionSupport, "#ff0000ff");
  t = G4Translate3D(0.5 * (SectionSupportPosition[iPlane - 1]
                           [iSectionSupport - 1].deltal_left -
                           SectionSupportPosition[iPlane - 1]
                           [iSectionSupport - 1].deltal_right) +
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].x,
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].y,
                    SectionSupportPosition[iPlane - 1][iSectionSupport - 1].z);
  physiSectionSupport = new G4PVPlacementGT(mpvgt, t, logicSectionSupport,
                                            SectionSupportName,
                                            iSectionSupport);
  if (physiSectionSupport == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicSectionSupport);
}

void GeoEKLMBelleII::createPlasticListElement(int iListPlane, int iList,
                                              G4PVPlacementGT* mpvgt)
{
  double ly;
  double y;
  double z;
  G4LogicalVolume* logicList;
  G4PVPlacementGT* physiList;
  G4Transform3D t;
  std::string List_Name = "List_" + boost::lexical_cast<std::string>(iList) + "_ListPlane_"
                          + boost::lexical_cast<std::string>(iListPlane) + "_" +
                          mpvgt->GetName();
  if (solids.list[iList - 1] == NULL) {
    ly = StripSize.width;
    if (iList % 15 <= 1)
      ly = ly - PlasticListDeltaL;
    solids.list[iList - 1] = new G4Box(List_Name,
                                       0.5 * StripPosition[iList - 1].length,
                                       0.5 * ly, 0.5 * PlasticListWidth);
  }
  if (solids.list[iList - 1] == NULL)
    B2FATAL(MemErr);
  logicList = new G4LogicalVolume(solids.list[iList - 1],
                                  mat.polystyrol, List_Name);
  if (logicList == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicList, false);
  geometry::setColor(*logicList, "#00ff00ff");
  y = StripPosition[iList - 1].Y;
  if (iList % 15 == 1)
    y = y + 0.5 * PlasticListDeltaL;
  else if (iList % 15 == 0)
    y = y - 0.5 * PlasticListDeltaL;
  z = 0.5 * (StripSize.thickness + PlasticListWidth);
  if (iListPlane == 2)
    z = -z;
  t = G4Translate3D(StripPosition[iList - 1].X, y, z);
  physiList = new G4PVPlacementGT(mpvgt, t, logicList, List_Name,
                                  iList * 100 + iListPlane);
  if (physiList == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicList);
}

void GeoEKLMBelleII::createStripVolume(int iStrip, G4PVPlacementGT* mpvgt)
{
  G4LogicalVolume* logicStripVolume;
  G4PVPlacementGT* physiStripVolume;
  G4Transform3D t;
  std::string StripVolume_Name = "StripVolume_" + boost::lexical_cast<std::string>(iStrip)
                                 + "_" + mpvgt->GetName();
  if (solids.stripvol[iStrip - 1] == NULL)
    solids.stripvol[iStrip - 1] = new G4Box(StripVolume_Name,
                                            0.5 * (StripPosition[iStrip - 1].length +
                                                   StripSize.rss_size),
                                            0.5 * StripSize.width,
                                            0.5 * StripSize.thickness);
  if (solids.stripvol[iStrip - 1] == NULL)
    B2FATAL(MemErr);
  logicStripVolume = new G4LogicalVolume(solids.stripvol[iStrip - 1], mat.air,
                                         StripVolume_Name);
  if (logicStripVolume == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicStripVolume, false);
  t = G4Translate3D(StripPosition[iStrip - 1].X + 0.5 * StripSize.rss_size,
                    StripPosition[iStrip - 1].Y, 0.0);
  physiStripVolume = new G4PVPlacementGT(mpvgt, t, logicStripVolume,
                                         StripVolume_Name, iStrip);
  if (physiStripVolume == NULL)
    B2FATAL(MemErr);
  createStrip(iStrip, physiStripVolume);
  if (m_mode != EKLM_DETECTOR_NORMAL)
    createSiPM(iStrip, physiStripVolume);
}

void GeoEKLMBelleII::createStrip(int iStrip, G4PVPlacementGT* mpvgt)
{
  G4LogicalVolume* logicStrip;
  G4PVPlacementGT* physiStrip;
  G4Transform3D t;
  std::string Strip_Name = "Strip_" + mpvgt->GetName();
  if (solids.strip[iStrip - 1] == NULL)
    solids.strip[iStrip - 1] = new G4Box(Strip_Name,
                                         0.5 * StripPosition[iStrip - 1].length,
                                         0.5 * StripSize.width,
                                         0.5 * StripSize.thickness);
  if (solids.strip[iStrip - 1] == NULL)
    B2FATAL(MemErr);
  logicStrip = new G4LogicalVolume(solids.strip[iStrip - 1],
                                   mat.polystyrene, Strip_Name);
  if (logicStrip == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicStrip, true);
  geometry::setColor(*logicStrip, "#ffffffff");
  t = G4Translate3D(-0.5 * StripSize.rss_size, 0., 0.);
  physiStrip = new G4PVPlacementGT(mpvgt, t, logicStrip, Strip_Name, iStrip);
  if (physiStrip == NULL)
    B2FATAL(MemErr);
  createStripGroove(iStrip, physiStrip);
  createStripSensitive(iStrip, physiStrip);

  //*** saving information to EKLMTransformationFactory
  std::vector<int> history = physiStrip->getIdHistory();

  (EKLMTransformationFactory::getInstance())->addMatrixEntry(history[0], // endcap
                                                             history[1], // sector
                                                             history[2], // layer
                                                             history[3], // plane
                                                             history[4], //strip
                                                             physiStrip->getTransform());
  (EKLMTransformationFactory::getInstance())->addLengthEntry(history[4], StripPosition[iStrip - 1].length);
  //** done

  printVolumeMass(logicStrip);
}

void GeoEKLMBelleII::createStripGroove(int iStrip, G4PVPlacementGT* mpvgt)
{
  G4LogicalVolume* logicGroove;
  G4PVPlacementGT* physiGroove;
  G4Transform3D t;
  std::string Groove_Name = "Groove_" + mpvgt->GetName();
  if (solids.groove[iStrip - 1] == NULL) {
    solids.groove[iStrip - 1] = new G4Box(Groove_Name,
                                          0.5 * StripPosition[iStrip - 1].length,
                                          0.5 * StripSize.groove_width,
                                          0.5 * StripSize.groove_depth);
  }
  if (solids.groove[iStrip - 1] == NULL)
    B2FATAL(MemErr);
  logicGroove = new G4LogicalVolume(solids.groove[iStrip - 1],
                                    mat.gel, Groove_Name);
  if (logicGroove == NULL)
    B2FATAL(MemErr);
  geometry::setColor(*logicGroove, "#00ff00ff");
  geometry::setVisibility(*logicGroove, true);
  t = G4Translate3D(0., 0.,
                    0.5 * (StripSize.thickness - StripSize.groove_depth));
  physiGroove = new G4PVPlacementGT(mpvgt, t, logicGroove, Groove_Name, iStrip);
  if (physiGroove == NULL)
    B2FATAL(MemErr);
  printVolumeMass(logicGroove);
}

void GeoEKLMBelleII::createStripSensitive(int iStrip, G4PVPlacementGT* mpvgt)
{
  G4LogicalVolume* logicSensitive;
  G4PVPlacementGT* physiSensitive;
  G4Transform3D t;
  G4Transform3D t1;
  std::string Sensitive_Name = "Sensitive_" + mpvgt->GetName();
  if (solids.scint[iStrip - 1].sens == NULL) {
    solids.scint[iStrip - 1].box = new G4Box("Box_" + Sensitive_Name,
                                             0.5 * StripPosition[iStrip - 1].length -
                                             StripSize.no_scintillation_thickness,
                                             0.5 * StripSize.width -
                                             StripSize.no_scintillation_thickness,
                                             0.5 * StripSize.thickness -
                                             StripSize.no_scintillation_thickness);
    if (solids.scint[iStrip - 1].box == NULL)
      B2FATAL(MemErr);
    t1 = G4Translate3D(0., 0.,
                       0.5 * (StripSize.thickness - StripSize.groove_depth));
    solids.scint[iStrip - 1].sens = new G4SubtractionSolid(Sensitive_Name,
                                                           solids.scint[iStrip - 1].box,
                                                           solids.groove[iStrip - 1], t1);
  }
  if (solids.scint[iStrip - 1].sens == NULL)
    B2FATAL(MemErr);
  logicSensitive = new G4LogicalVolume(solids.scint[iStrip - 1].sens,
                                       mat.polystyrene, Sensitive_Name,
                                       0, m_sensitive, 0);
  if (logicSensitive == NULL)
    B2FATAL(MemErr);
  geometry::setColor(*logicSensitive, "#ffffffff");
  geometry::setVisibility(*logicSensitive, false);
  t = G4Translate3D(0., 0., 0.);
  physiSensitive = new G4PVPlacementGT(mpvgt, t, logicSensitive, Sensitive_Name,
                                       iStrip, m_mode);
  if (physiSensitive == NULL)
    B2FATAL(MemErr);
  physiSensitive->setVolumeType(EKLM_SENSITIVE_STRIP);
  printVolumeMass(logicSensitive);
}

void GeoEKLMBelleII::createSiPM(int iStrip, G4PVPlacementGT* mpvgt)
{
  static G4Box* solidSiPM = NULL;
  G4LogicalVolume* logicSiPM;
  G4PVPlacementGT* physiSiPM;
  G4Transform3D t;
  std::string SiPM_Name = "SiPM_" + mpvgt->GetName();
  if (solidSiPM == NULL)
    solidSiPM = new G4Box(SiPM_Name, 0.5 * StripSize.rss_size,
                          0.5 * StripSize.rss_size, 0.5 * StripSize.rss_size);
  if (solidSiPM == NULL)
    B2FATAL(MemErr);
  if (m_mode == EKLM_DETECTOR_NORMAL)
    logicSiPM = new G4LogicalVolume(solidSiPM, mat.silicon, SiPM_Name);
  else
    logicSiPM = new G4LogicalVolume(solidSiPM, mat.silicon, SiPM_Name, 0,
                                    m_sensitive, 0);
  if (logicSiPM == NULL)
    B2FATAL(MemErr);
  geometry::setVisibility(*logicSiPM, true);
  geometry::setColor(*logicSiPM, "#0000ffff");
  t = G4Translate3D(0.5 * StripPosition[iStrip - 1].length, 0., 0.);
  physiSiPM = new G4PVPlacementGT(mpvgt, t, logicSiPM, SiPM_Name, iStrip,
                                  m_mode);
  if (physiSiPM == NULL)
    B2FATAL(MemErr);
  physiSiPM->setVolumeType(EKLM_SENSITIVE_SIPM);
  printVolumeMass(logicSiPM);
}

void GeoEKLMBelleII::printVolumeMass(G4LogicalVolume* lv)
{
  if (m_mode == EKLM_DETECTOR_PRINTMASSES)
    printf("Volume %s: mass = %g g\n", lv->GetName().c_str(),
           lv->GetMass() / g);
}

void GeoEKLMBelleII::create(const GearDir& content, G4LogicalVolume& topVolume,
                            geometry::GeometryTypes type)
{
  int i;
  (void)type;
  readXMLData(content);
  m_sensitive = new EKLMSensitiveDetector("EKLMSensitiveDetector");
  if (m_sensitive == NULL)
    B2FATAL(MemErr);
  createMaterials();
  mallocSolids();
  for (i = 1; i <= 2; i++)
    createEndcap(i, &topVolume);
  if (m_mode == EKLM_DETECTOR_PRINTMASSES) {
    printf("EKLM started in mode EKLM_DETECTOR_PRINTMASSES. Exiting now.\n");
    exit(0);
  }

  // save infrormation to file

  if (m_outputFile.size() != 0)
    (EKLMTransformationFactory::getInstance())->writeToFile(m_outputFile.c_str());

}

