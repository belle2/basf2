/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <CLHEP/Units/PhysicalConstants.h>

/* Belle2 headers. */
#include <eklm/geometry/GeometryData2.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char c_MemErr[] = "Memory allocation error.";
static const char c_ModeErr[] =
  "Requested data are defined only for c_DetectorBackground mode.";
static const char c_PlaneErr[] = "Number of plane must be from 1 to 2.";
static const char c_SegmentErr[] = "Number of segment must be from 1 to 5.";
static const char c_SupportErr[] =
  "Number of segment support element must be from 1 to 6.";
static const char c_BoardErr[] = "Number of board must be from 1 to 15.";
static const char c_StripErr[] = "Number of strip must be from 1 to 75.";

const EKLM::GeometryData2& EKLM::GeometryData2::Instance()
{
  static EKLM::GeometryData2 gd;
  return gd;
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
static void readSectorSupportGeometry(struct EKLM::SectorSupportGeometry* ssg,
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
void EKLM::GeometryData2::calculateSectorSupportGeometry()
{
  /* Corner 1. */
  m_SectorSupportGeometry.Corner1A.X = m_SectorSupportPosition.X;
  m_SectorSupportGeometry.Corner1A.Y = m_SectorSupportPosition.outerR -
                                       m_SectorSupportGeometry.DeltaLY;
  m_SectorSupportGeometry.Corner1B.X = m_SectorSupportPosition.X +
                                       m_SectorSupportGeometry.CornerX;
  m_SectorSupportGeometry.Corner1B.Y =
    sqrt(m_SectorSupportPosition.outerR * m_SectorSupportPosition.outerR -
         m_SectorSupportGeometry.Corner1B.X *
         m_SectorSupportGeometry.Corner1B.X);
  m_SectorSupportGeometry.CornerAngle =
    atan2(m_SectorSupportGeometry.Corner1B.Y -
          m_SectorSupportGeometry.Corner1A.Y,
          m_SectorSupportGeometry.Corner1B.X -
          m_SectorSupportGeometry.Corner1A.X) *
    CLHEP::rad;
  m_SectorSupportGeometry.Corner1AInner.X = m_SectorSupportPosition.X +
                                            m_SectorSupportGeometry.Thickness;
  m_SectorSupportGeometry.Corner1AInner.Y =
    m_SectorSupportGeometry.Corner1A.Y -
    m_SectorSupportGeometry.Thickness *
    (1.0 / cos(m_SectorSupportGeometry.CornerAngle) -
     tan(m_SectorSupportGeometry.CornerAngle));
  /* Corner 3. */
  m_SectorSupportGeometry.Corner3.X =
    sqrt(m_SectorSupportPosition.innerR * m_SectorSupportPosition.innerR -
         m_SectorSupportPosition.Y * m_SectorSupportPosition.Y);
  m_SectorSupportGeometry.Corner3.Y = m_SectorSupportPosition.Y;
  /* Corner 4. */
  m_SectorSupportGeometry.Corner4.X = m_SectorSupportPosition.X;
  m_SectorSupportGeometry.Corner4.Y =
    sqrt(m_SectorSupportPosition.innerR * m_SectorSupportPosition.innerR -
         m_SectorSupportPosition.X * m_SectorSupportPosition.X);
}

static bool compareLength(double a, double b)
{
  return a < b;
}

void EKLM::GeometryData2::fillStripIndexArrays()
{
  const char err[] = "Strip sorting algorithm error.";
  int i;
  double l;
  std::vector<double> strips;
  std::vector<double>::iterator it;
  std::map<double, int> mapLengthStrip;
  std::map<double, int> mapLengthStrip2;
  std::map<double, int>::iterator itm;
  for (i = 0; i < m_nStrip; i++) {
    strips.push_back(m_StripPosition[i].length);
    mapLengthStrip.insert(std::pair<double, int>(m_StripPosition[i].length, i));
  }
  sort(strips.begin(), strips.end(), compareLength);
  l = strips[0];
  m_nStripDifferent = 1;
  for (it = strips.begin(); it != strips.end(); ++it) {
    if ((*it) != l) {
      l = (*it);
      m_nStripDifferent++;
    }
  }
  m_StripLenToAll = (int*)malloc(m_nStripDifferent * sizeof(int));
  if (m_StripLenToAll == NULL)
    B2FATAL(c_MemErr);
  i = 0;
  l = strips[0];
  itm = mapLengthStrip.find(l);
  if (itm == mapLengthStrip.end())
    B2FATAL(err);
  m_StripLenToAll[i] = itm->second;
  mapLengthStrip2.insert(std::pair<double, int>(l, i));
  for (it = strips.begin(); it != strips.end(); ++it) {
    if ((*it) != l) {
      l = (*it);
      i++;
      itm = mapLengthStrip.find(l);
      if (itm == mapLengthStrip.end())
        B2FATAL(err);
      m_StripLenToAll[i] = itm->second;
      mapLengthStrip2.insert(std::pair<double, int>(l, i));
    }
  }
  m_StripAllToLen = (int*)malloc(m_nStrip * sizeof(int));
  if (m_StripAllToLen == NULL)
    B2FATAL(c_MemErr);
  for (i = 0; i < m_nStrip; i++) {
    itm = mapLengthStrip2.find(m_StripPosition[i].length);
    if (itm == mapLengthStrip2.end())
      B2FATAL(err);
    m_StripAllToLen[i] = itm->second;
  }
}

void EKLM::GeometryData2::readXMLDataStrips()
{
  int i;
  GearDir Strips("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
                 "Layer/Sector/Plane/Strips");
  m_nStrip = Strips.getNumberNodes("Strip");
  m_StripGeometry.width  = Strips.getLength("Width") * CLHEP::cm;
  m_StripGeometry.thickness = Strips.getLength("Thickness") * CLHEP::cm;
  m_StripGeometry.groove_depth = Strips.getLength("GrooveDepth") * CLHEP::cm;
  m_StripGeometry.groove_width = Strips.getLength("GrooveWidth") * CLHEP::cm;
  m_StripGeometry.no_scintillation_thickness =
    Strips.getLength("NoScintillationThickness") * CLHEP::cm;
  m_StripGeometry.rss_size = Strips.getLength("RSSSize") * CLHEP::cm;
  m_StripPosition = (struct EKLM::ElementPosition*)
                    malloc(m_nStrip * sizeof(struct EKLM::ElementPosition));
  if (m_StripPosition == NULL)
    B2FATAL(c_MemErr);
  for (i = 0; i < m_nStrip; i++) {
    GearDir StripContent(Strips);
    StripContent.append((boost::format("/Strip[%1%]") % (i + 1)).str());
    m_StripPosition[i].length = StripContent.getLength("Length") * CLHEP::cm;
    m_StripPosition[i].X = StripContent.getLength("PositionX") * CLHEP::cm;
    m_StripPosition[i].Y = StripContent.getLength("PositionY") * CLHEP::cm;
    m_StripPosition[i].Z = StripContent.getLength("PositionZ") * CLHEP::cm;
  }
}

EKLM::GeometryData2::GeometryData2()
{
  int i;
  int j;
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
  m_mode = (enum DetectorMode)gd.getInt("Mode");
  if (m_mode < 0 || m_mode > 2)
    B2FATAL("EKLM started with unknown geometry mode " << m_mode << ".");
  m_solenoidZ = gd.getLength("SolenoidZ") * CLHEP::cm;
  GearDir EndCap(gd);
  EndCap.append("/Endcap");
  readPositionData(&m_EndcapPosition, &EndCap);
  readSizeData(&m_EndcapPosition, &EndCap);
  m_MinZForward = m_solenoidZ + m_EndcapPosition.Z -
                  0.5 * m_EndcapPosition.length;
  m_MaxZBackward = m_solenoidZ - m_EndcapPosition.Z +
                   0.5 * m_EndcapPosition.length;
  m_nLayer = EndCap.getInt("nLayer");
  if (m_nLayer < 1 || m_nLayer > 14)
    B2FATAL("Number of layers must be from 1 to 14.");
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
  readSizeData(&m_LayerPosition, &Layer);
  m_LayerShiftZ = Layer.getLength("ShiftZ") * CLHEP::cm;
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readSizeData(&m_SectorPosition, &Sector);
  m_nPlane = Sector.getInt("nPlane");
  if (m_nPlane < 1 || m_nPlane > 2)
    B2FATAL("Number of strip planes must be from 1 to 2.");
  if (m_mode == c_DetectorBackground) {
    m_nBoard = Sector.getInt("m_nBoard");
    if (m_nBoard < 1 || m_nBoard > 5)
      B2FATAL("Number of readout boards must be from 1 to 5.");
    GearDir Boards(Sector);
    Boards.append("/Boards");
    m_BoardGeometry.length = Boards.getLength("Length") * CLHEP::cm;
    m_BoardGeometry.width = Boards.getLength("Width") * CLHEP::cm;
    m_BoardGeometry.height = Boards.getLength("Height") * CLHEP::cm;
    m_BoardGeometry.base_width = Boards.getLength("BaseWidth") * CLHEP::cm;
    m_BoardGeometry.base_height = Boards.getLength("BaseHeight") * CLHEP::cm;
    m_BoardGeometry.strip_length = Boards.getLength("StripLength") * CLHEP::cm;
    m_BoardGeometry.strip_width = Boards.getLength("StripWidth") * CLHEP::cm;
    m_BoardGeometry.strip_height = Boards.getLength("StripHeight") * CLHEP::cm;
    m_nStripBoard = Boards.getInt("nStripBoard");
    for (j = 0; j < m_nPlane; j++) {
      m_BoardPosition[j] = (struct BoardPosition*)
                           malloc(m_nBoard * sizeof(struct BoardPosition));
      if (m_BoardPosition[j] == NULL)
        B2FATAL(c_MemErr);
      for (i = 0; i < m_nBoard; i++) {
        GearDir BoardContent(Boards);
        BoardContent.append((boost::format("/BoardData[%1%]") % (j + 1)).str());
        BoardContent.append((boost::format("/Board[%1%]") % (i + 1)).str());
        m_BoardPosition[j][i].phi = BoardContent.getLength("Phi") * CLHEP::rad;
        m_BoardPosition[j][i].r = BoardContent.getLength("Radius") * CLHEP::cm;
      }
    }
    m_StripBoardPosition = (struct StripBoardPosition*)
                           malloc(m_nStripBoard *
                                  sizeof(struct StripBoardPosition));
    if (m_StripBoardPosition == NULL)
      B2FATAL(c_MemErr);
    for (i = 0; i < m_nStripBoard; i++) {
      GearDir StripBoardContent(Boards);
      StripBoardContent.append((boost::format("/StripBoardData/Board[%1%]") %
                                (i + 1)).str());
      m_StripBoardPosition[i].x = StripBoardContent.getLength("PositionX") *
                                  CLHEP::cm;
    }
  }
  GearDir SectorSupport(Sector);
  SectorSupport.append("/SectorSupport");
  readPositionData(&m_SectorSupportPosition, &SectorSupport);
  readSizeData(&m_SectorSupportPosition, &SectorSupport);
  readSectorSupportGeometry(&m_SectorSupportGeometry, &SectorSupport);
  calculateSectorSupportGeometry();
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(&m_PlanePosition, &Plane);
  readSizeData(&m_PlanePosition, &Plane);
  readXMLDataStrips();
  fillStripIndexArrays();
  m_nSegment = Plane.getInt("nSegment");
  if (m_nSegment <= 0)
    B2FATAL("Number of segments must be positive.");
  m_PlasticSheetGeometry.Width = Plane.getLength("PlasticSheetWidth") *
                                 CLHEP::cm;
  m_PlasticSheetGeometry.DeltaL = Plane.getLength("PlasticSheetDeltaL") *
                                  CLHEP::cm;
  GearDir Segments(Plane);
  Segments.append("/Segments");
  m_SegmentSupportGeometry.TopWidth =
    Segments.getLength("TopWidth") * CLHEP::cm;
  m_SegmentSupportGeometry.TopThickness =
    Segments.getLength("TopThickness") * CLHEP::cm;
  m_SegmentSupportGeometry.MiddleWidth =
    Segments.getLength("MiddleWidth") * CLHEP::cm;
  m_SegmentSupportGeometry.MiddleThickness =
    Segments.getLength("MiddleThickness") * CLHEP::cm;
  for (j = 0; j < m_nPlane; j++) {
    m_SegmentSupportPosition[j] =
      (struct EKLM::SegmentSupportPosition*)
      malloc((m_nSegment + 1) * sizeof(struct EKLM::SegmentSupportPosition));
    if (m_SegmentSupportPosition[j] == NULL)
      B2FATAL(c_MemErr);
    for (i = 0; i <= m_nSegment; i++) {
      GearDir SegmentSupportContent(Segments);
      SegmentSupportContent.append(
        (boost::format("/SegmentSupportData[%1%]") % (j + 1)).str());
      SegmentSupportContent.append(
        (boost::format("/SegmentSupport[%1%]") % (i +  1)).str());
      m_SegmentSupportPosition[j][i].length =
        SegmentSupportContent.getLength("Length") * CLHEP::cm;
      m_SegmentSupportPosition[j][i].x =
        SegmentSupportContent.getLength("PositionX") * CLHEP::cm;
      m_SegmentSupportPosition[j][i].y =
        SegmentSupportContent.getLength("PositionY") * CLHEP::cm;
      m_SegmentSupportPosition[j][i].z =
        SegmentSupportContent.getLength("PositionZ") * CLHEP::cm;
      m_SegmentSupportPosition[j][i].deltal_right =
        SegmentSupportContent.getLength("DeltaLRight") * CLHEP::cm;
      m_SegmentSupportPosition[j][i].deltal_left =
        SegmentSupportContent.getLength("DeltaLLeft") * CLHEP::cm;
    }
  }
}

EKLM::GeometryData2::~GeometryData2()
{
  int i;
  for (i = 0; i < m_nPlane; i++)
    free(m_SegmentSupportPosition[i]);
  if (m_mode == c_DetectorBackground) {
    for (i = 0; i < m_nPlane; i++)
      free(m_BoardPosition[i]);
    free(m_StripBoardPosition);
  }
  free(m_StripLenToAll);
  free(m_StripAllToLen);
}

EKLM::DetectorMode EKLM::GeometryData2::getDetectorMode() const
{
  return m_mode;
}

int EKLM::GeometryData2::getNLayers() const
{
  return m_nLayer;
}

int EKLM::GeometryData2::getNDetectorLayers(int endcap) const
{
  if (endcap <= 0 || endcap > 2)
    B2FATAL("Number of endcap must be 1 (backward) or 2 (forward).");
  if (endcap == 1)
    return m_nLayerBackward;
  return m_nLayerForward;
}

int EKLM::GeometryData2::getNPlanes() const
{
  return m_nPlane;
}

int EKLM::GeometryData2::getNBoards() const
{
  if (m_mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  return m_nBoard;
}

int EKLM::GeometryData2::getNStripBoards() const
{
  if (m_mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  return m_nStripBoard;
}

int EKLM::GeometryData2::getNStrips() const
{
  return m_nStrip;
}

int EKLM::GeometryData2::getNSegments() const
{
  return m_nSegment;
}

double EKLM::GeometryData2::getSolenoidZ() const
{
  return m_solenoidZ;
}

const EKLM::ElementPosition* EKLM::GeometryData2::getEndcapPosition() const
{
  return &m_EndcapPosition;
}

const EKLM::ElementPosition* EKLM::GeometryData2::getLayerPosition() const
{
  return &m_LayerPosition;
}

double EKLM::GeometryData2::getLayerShiftZ() const
{
  return m_LayerShiftZ;
}

const EKLM::ElementPosition* EKLM::GeometryData2::getSectorPosition() const
{
  return &m_SectorPosition;
}

const EKLM::ElementPosition*
EKLM::GeometryData2::getSectorSupportPosition() const
{
  return &m_SectorSupportPosition;
}

const EKLM::SectorSupportGeometry*
EKLM::GeometryData2::getSectorSupportGeometry() const
{
  return &m_SectorSupportGeometry;
}

const EKLM::BoardGeometry* EKLM::GeometryData2::getBoardGeometry() const
{
  if (m_mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  return &m_BoardGeometry;
}

const EKLM::BoardPosition*
EKLM::GeometryData2::getBoardPosition(int plane, int segment) const
{
  if (m_mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  if (plane <= 0 || plane > 2)
    B2FATAL(c_PlaneErr);
  if (segment <= 0 || segment > 5)
    B2FATAL(c_SegmentErr);
  return &m_BoardPosition[plane - 1][segment - 1];
}

const EKLM::StripBoardPosition*
EKLM::GeometryData2::getStripBoardPosition(int board) const
{
  if (m_mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  if (board <= 0 || board > 15)
    B2FATAL(c_BoardErr);
  return &m_StripBoardPosition[board - 1];
}

const EKLM::ElementPosition* EKLM::GeometryData2::getPlanePosition() const
{
  return &m_PlanePosition;
}

const EKLM::SegmentSupportPosition*
EKLM::GeometryData2::getSegmentSupportPosition(int plane, int support) const
{
  if (plane <= 0 || plane > 2)
    B2FATAL(c_PlaneErr);
  if (support <= 0 || support > 6)
    B2FATAL(c_SupportErr);
  return &m_SegmentSupportPosition[plane - 1][support - 1];
}

const EKLM::SegmentSupportGeometry*
EKLM::GeometryData2::getSegmentSupportGeometry() const
{
  return &m_SegmentSupportGeometry;
}

const EKLM::PlasticSheetGeometry*
EKLM::GeometryData2::getPlasticSheetGeometry() const
{
  return &m_PlasticSheetGeometry;
}

const EKLM::ElementPosition*
EKLM::GeometryData2::getStripPosition(int strip) const
{
  if (strip <= 0 || strip > 75)
    B2FATAL(c_StripErr);
  return &m_StripPosition[strip - 1];
}

double EKLM::GeometryData2::getStripLength(int strip) const
{
  return m_StripPosition[strip - 1].length;
}

const EKLM::StripGeometry* EKLM::GeometryData2::getStripGeometry() const
{
  return &m_StripGeometry;
}

int EKLM::GeometryData2::getStripLengthIndex(int positionIndex) const
{
  return m_StripAllToLen[positionIndex];
}

int EKLM::GeometryData2::getStripPositionIndex(int lengthIndex) const
{
  return m_StripLenToAll[lengthIndex];
}

int EKLM::GeometryData2::getNStripsDifferentLength() const
{
  return m_nStripDifferent;
}

bool EKLM::GeometryData2::hitInEKLM(double z) const
{
  double zMm;
  zMm = z / Unit::cm * CLHEP::cm;
  return (zMm > m_MinZForward) || (zMm < m_MaxZBackward);
}

/*
 * Note that numbers of elements are 0-based for all transformation functions.
 */
void
EKLM::GeometryData2::getEndcapTransform(HepGeom::Transform3D* t, int n) const
{
  if (n == 0)
    *t = HepGeom::Translate3D(m_EndcapPosition.X, m_EndcapPosition.Y,
                              -m_EndcapPosition.Z + m_solenoidZ);
  else
    *t = HepGeom::Translate3D(m_EndcapPosition.X, m_EndcapPosition.Y,
                              m_EndcapPosition.Z + m_solenoidZ) *
         HepGeom::RotateY3D(180.*CLHEP::deg);
}

void
EKLM::GeometryData2::getLayerTransform(HepGeom::Transform3D* t, int n) const
{
  *t = HepGeom::Translate3D(0.0, 0.0, m_EndcapPosition.length / 2.0 -
                            (n + 1) * m_LayerShiftZ +
                            0.5 * m_LayerPosition.length);
}

void
EKLM::GeometryData2::getSectorTransform(HepGeom::Transform3D* t, int n) const
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

void
EKLM::GeometryData2::getPlaneTransform(HepGeom::Transform3D* t, int n) const
{
  if (n == 0)
    *t = HepGeom::Translate3D(m_PlanePosition.X, m_PlanePosition.Y,
                              m_PlanePosition.Z) *
         HepGeom::Rotate3D(180. * CLHEP::deg,
                           HepGeom::Vector3D<double>(1., 1., 0.));
  else
    *t = HepGeom::Translate3D(m_PlanePosition.X, m_PlanePosition.Y,
                              -m_PlanePosition.Z);
}

void
EKLM::GeometryData2::getStripTransform(HepGeom::Transform3D* t, int n) const
{
  *t = HepGeom::Translate3D(m_StripPosition[n].X, m_StripPosition[n].Y, 0.0);
}

void
EKLM::GeometryData2::getSheetTransform(HepGeom::Transform3D* t, int n) const
{
  double y;
  y = m_StripPosition[n].Y;
  if (n % 15 == 0)
    y = y + 0.5 * m_PlasticSheetGeometry.DeltaL;
  else if (n % 15 == 14)
    y = y - 0.5 * m_PlasticSheetGeometry.DeltaL;
  *t = HepGeom::Translate3D(m_StripPosition[n].X, y, 0.0);
}

