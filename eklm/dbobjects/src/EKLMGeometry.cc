/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMGeometry.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMGeometry::EndcapStructureGeometry::EndcapStructureGeometry()
{
  Phi = 0;
  Dphi = 0;
  Nsides = 0;
  Nboundary = 0;
  Z = NULL;
  Rmin = NULL;
  Rmax = NULL;
  Zsub = 0;
  Rminsub = 0;
  Rmaxsub = 0;
}

EKLMGeometry::EndcapStructureGeometry::EndcapStructureGeometry(
  const EndcapStructureGeometry& geometry) : TObject(geometry)
{
  Phi = geometry.Phi;
  Dphi = geometry.Dphi;
  Nsides = geometry.Nsides;
  Nboundary = geometry.Nboundary;
  Z = new double[Nboundary];
  memcpy(Z, geometry.Z, Nboundary * sizeof(double));
  Rmin = new double[Nboundary];
  memcpy(Rmin, geometry.Rmin, Nboundary * sizeof(double));
  Rmax = new double[Nboundary];
  memcpy(Rmax, geometry.Rmax, Nboundary * sizeof(double));
  Zsub = geometry.Zsub;
  Rminsub = geometry.Rminsub;
  Rmaxsub = geometry.Rmaxsub;
}

EKLMGeometry::EndcapStructureGeometry&
EKLMGeometry::EndcapStructureGeometry::operator=(
  const EndcapStructureGeometry& geometry)
{
  if (&geometry == this)
    return *this;
  Phi = geometry.Phi;
  Dphi = geometry.Dphi;
  Nsides = geometry.Nsides;
  Nboundary = geometry.Nboundary;
  if (Z != NULL)
    delete[] Z;
  Z = new double[Nboundary];
  memcpy(Z, geometry.Z, Nboundary * sizeof(double));
  if (Rmin != NULL)
    delete[] Rmin;
  Rmin = new double[Nboundary];
  memcpy(Rmin, geometry.Rmin, Nboundary * sizeof(double));
  if (Rmax != NULL)
    delete[] Rmax;
  Rmax = new double[Nboundary];
  memcpy(Rmax, geometry.Rmax, Nboundary * sizeof(double));
  Zsub = geometry.Zsub;
  Rminsub = geometry.Rminsub;
  Rmaxsub = geometry.Rmaxsub;
  return *this;
}

EKLMGeometry::EndcapStructureGeometry::~EndcapStructureGeometry()
{
  if (Z != NULL)
    delete[] Z;
  if (Rmin != NULL)
    delete[] Rmin;
  if (Rmax != NULL)
    delete[] Rmax;
}

EKLMGeometry::ElementPosition::ElementPosition()
{
  InnerR = 0;
  OuterR = 0;
  Length = 0;
  X = 0;
  Y = 0;
  Z = 0;
}

EKLMGeometry::SectorSupportGeometry::SectorSupportGeometry()
{
  Thickness = 0;
  DeltaLY = 0;
  CornerX = 0;
  Corner1LX = 0;
  Corner1Width = 0;
  Corner1Thickness = 0;
  Corner1Z = 0;
  Corner2LX = 0;
  Corner2LY = 0;
  Corner2Thickness = 0;
  Corner2Z = 0;
  Corner3LX = 0;
  Corner3LY = 0;
  Corner3Thickness = 0;
  Corner3Z = 0;
  Corner4LX = 0;
  Corner4LY = 0;
  Corner4Thickness = 0;
  Corner4Z = 0;
  CornerAngle = 0;
}

EKLMGeometry::PlasticSheetGeometry::PlasticSheetGeometry()
{
  Width = 0;
  DeltaL = 0;
}

EKLMGeometry::SegmentSupportGeometry::SegmentSupportGeometry()
{
  TopWidth = 0;
  TopThickness = 0;
  MiddleWidth = 0;
  MiddleThickness = 0;
}

/* Class EKLMGeometry::SegmentSupportPosition. */

EKLMGeometry::SegmentSupportPosition::SegmentSupportPosition()
{
  m_DeltaLRight = 0;
  m_DeltaLLeft = 0;
  m_Length = 0;
  m_X = 0;
  m_Y = 0;
  m_Z = 0;
}

double EKLMGeometry::SegmentSupportPosition::getDeltaLRight() const
{
  return m_DeltaLRight;
}

void EKLMGeometry::SegmentSupportPosition::setDeltaLRight(double deltaLRight)
{
  m_DeltaLRight = deltaLRight;
}

double EKLMGeometry::SegmentSupportPosition::getDeltaLLeft() const
{
  return m_DeltaLLeft;
}

void EKLMGeometry::SegmentSupportPosition::setDeltaLLeft(double deltaLLeft)
{
  m_DeltaLLeft = deltaLLeft;
}

double EKLMGeometry::SegmentSupportPosition::getLength() const
{
  return m_Length;
}

void EKLMGeometry::SegmentSupportPosition::setLength(double length)
{
  m_Length = length;
}

double EKLMGeometry::SegmentSupportPosition::getX() const
{
  return m_X;
}

void EKLMGeometry::SegmentSupportPosition::setX(double x)
{
  m_X = x;
}

double EKLMGeometry::SegmentSupportPosition::getY() const
{
  return m_Y;
}

void EKLMGeometry::SegmentSupportPosition::setY(double y)
{
  m_Y = y;
}

double EKLMGeometry::SegmentSupportPosition::getZ() const
{
  return m_Z;
}

void EKLMGeometry::SegmentSupportPosition::setZ(double z)
{
  m_Z = z;
}

/* Class EKLMGeometry::StripGeometry. */

EKLMGeometry::StripGeometry::StripGeometry()
{
  m_Width = 0;
  m_Thickness = 0;
  m_GrooveDepth = 0;
  m_GrooveWidth = 0;
  m_NoScintillationThickness = 0;
  m_RSSSize = 0;
}

double EKLMGeometry::StripGeometry::getWidth() const
{
  return m_Width;
}

void EKLMGeometry::StripGeometry::setWidth(double width)
{
  m_Width = width;
}

double EKLMGeometry::StripGeometry::getThickness() const
{
  return m_Thickness;
}

void EKLMGeometry::StripGeometry::setThickness(double thickness)
{
  m_Thickness = thickness;
}

double EKLMGeometry::StripGeometry::getGrooveDepth() const
{
  return m_GrooveDepth;
}

void EKLMGeometry::StripGeometry::setGrooveDepth(double grooveDepth)
{
  m_GrooveDepth = grooveDepth;
}

double EKLMGeometry::StripGeometry::getGrooveWidth() const
{
  return m_GrooveWidth;
}

void EKLMGeometry::StripGeometry::setGrooveWidth(double grooveWidth)
{
  m_GrooveWidth = grooveWidth;
}

double EKLMGeometry::StripGeometry::getNoScintillationThickness() const
{
  return m_NoScintillationThickness;
}

void EKLMGeometry::StripGeometry::setNoScintillationThickness(double thickness)
{
  m_NoScintillationThickness = thickness;
}

double EKLMGeometry::StripGeometry::getRSSSize() const
{
  return m_RSSSize;
}

void EKLMGeometry::StripGeometry::setRSSSize(double rssSize)
{
  m_RSSSize = rssSize;
}

/* Class EKLMGeometry::Point. */

EKLMGeometry::Point::Point()
{
  m_X = 0;
  m_Y = 0;
}

double EKLMGeometry::Point::getX() const
{
  return m_X;
}

void EKLMGeometry::Point::setX(double x)
{
  m_X = x;
}

double EKLMGeometry::Point::getY() const
{
  return m_Y;
}

void EKLMGeometry::Point::setY(double y)
{
  m_Y = y;
}

/* Class EKLMGeometry::ShieldDetailGeometry. */

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry()
{
  m_LengthX = 0;
  m_LengthY = 0;
  m_NPoints = 0;
  m_Points = NULL;
}

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry(
  const ShieldDetailGeometry& geometry) : TObject(geometry)
{
  int i;
  m_LengthX = geometry.getLengthX();
  m_LengthY = geometry.getLengthY();
  m_NPoints = geometry.getNPoints();
  if (m_NPoints > 0) {
    m_Points = new Point[m_NPoints];
    for (i = 0; i < m_NPoints; i++)
      m_Points[i] = *geometry.getPoint(i);
  } else
    m_Points = NULL;
}

EKLMGeometry::ShieldDetailGeometry&
EKLMGeometry::ShieldDetailGeometry::operator=(
  const ShieldDetailGeometry& geometry)
{
  int i;
  if (&geometry == this)
    return *this;
  m_LengthX = geometry.getLengthX();
  m_LengthY = geometry.getLengthY();
  m_NPoints = geometry.getNPoints();
  if (m_Points != NULL)
    delete[] m_Points;
  if (m_NPoints > 0) {
    m_Points = new Point[m_NPoints];
    for (i = 0; i < m_NPoints; i++)
      m_Points[i] = *geometry.getPoint(i);
  } else
    m_Points = NULL;
  return *this;
}

EKLMGeometry::ShieldDetailGeometry::~ShieldDetailGeometry()
{
  if (m_Points != NULL)
    delete[] m_Points;
}

double EKLMGeometry::ShieldDetailGeometry::getLengthX() const
{
  return m_LengthX;
}

void EKLMGeometry::ShieldDetailGeometry::setLengthX(double lengthX)
{
  m_LengthX = lengthX;
}

double EKLMGeometry::ShieldDetailGeometry::getLengthY() const
{
  return m_LengthY;
}

void EKLMGeometry::ShieldDetailGeometry::setLengthY(double lengthY)
{
  m_LengthY = lengthY;
}

int EKLMGeometry::ShieldDetailGeometry::getNPoints() const
{
  return m_NPoints;
}

void EKLMGeometry::ShieldDetailGeometry::setNPoints(int nPoints)
{
  if (nPoints < 0)
    B2FATAL("Number of points must be nonnegative.");
  m_NPoints = nPoints;
  if (m_Points != NULL)
    delete[] m_Points;
  if (m_NPoints > 0)
    m_Points = new Point[m_NPoints];
  else
    m_Points = NULL;
}

const EKLMGeometry::Point*
EKLMGeometry::ShieldDetailGeometry::getPoint(int i) const
{
  if (i < 0 || i >= m_NPoints)
    B2FATAL("Number of point must be from 0 to " << m_NPoints - 1 << ".");
  return &m_Points[i];
}

void EKLMGeometry::ShieldDetailGeometry::setPoint(int i, const Point& point)
{
  if (i < 0 || i >= m_NPoints)
    B2FATAL("Number of point must be from 0 to " << m_NPoints - 1 << ".");
  m_Points[i] = point;
}

/* Class EKLMGeometry::ShieldGeometry. */

EKLMGeometry::ShieldGeometry::ShieldGeometry()
{
  m_Thickness = 0;
}

double EKLMGeometry::ShieldGeometry::getThickness() const
{
  return m_Thickness;
}

void EKLMGeometry::ShieldGeometry::setThickness(double thickness)
{
  m_Thickness = thickness;
}

const EKLMGeometry::ShieldDetailGeometry*
EKLMGeometry::ShieldGeometry::getDetailA() const
{
  return &m_DetailA;
}

void EKLMGeometry::ShieldGeometry::setDetailA(
  const ShieldDetailGeometry& geometry)
{
  m_DetailA = geometry;
}

const EKLMGeometry::ShieldDetailGeometry*
EKLMGeometry::ShieldGeometry::getDetailB() const
{
  return &m_DetailB;
}

void EKLMGeometry::ShieldGeometry::setDetailB(
  const ShieldDetailGeometry& geometry)
{
  m_DetailB = geometry;
}

const EKLMGeometry::ShieldDetailGeometry*
EKLMGeometry::ShieldGeometry::getDetailC() const
{
  return &m_DetailC;
}

void EKLMGeometry::ShieldGeometry::setDetailC(
  const ShieldDetailGeometry& geometry)
{
  m_DetailC = geometry;
}

const EKLMGeometry::ShieldDetailGeometry*
EKLMGeometry::ShieldGeometry::getDetailD() const
{
  return &m_DetailD;
}

void EKLMGeometry::ShieldGeometry::setDetailD(
  const ShieldDetailGeometry& geometry)
{
  m_DetailD = geometry;
}

const EKLMGeometry::Point*
EKLMGeometry::ShieldGeometry::getDetailACenter() const
{
  return &m_DetailACenter;
}

void EKLMGeometry::ShieldGeometry::setDetailACenter(double x, double y)
{
  m_DetailACenter.setX(x);
  m_DetailACenter.setY(y);
}

const EKLMGeometry::Point*
EKLMGeometry::ShieldGeometry::getDetailBCenter() const
{
  return &m_DetailBCenter;
}

void EKLMGeometry::ShieldGeometry::setDetailBCenter(double x, double y)
{
  m_DetailBCenter.setX(x);
  m_DetailBCenter.setY(y);
}

const EKLMGeometry::Point*
EKLMGeometry::ShieldGeometry::getDetailCCenter() const
{
  return &m_DetailCCenter;
}

void EKLMGeometry::ShieldGeometry::setDetailCCenter(double x, double y)
{
  m_DetailCCenter.setX(x);
  m_DetailCCenter.setY(y);
}

/* Class EKLMGeometry::BoardGeometry. */

double EKLMGeometry::BoardGeometry::getLength() const
{
  return m_Length;
}

void EKLMGeometry::BoardGeometry::setLength(double length)
{
  m_Length = length;
}

double EKLMGeometry::BoardGeometry::getWidth() const
{
  return m_Width;
}

void EKLMGeometry::BoardGeometry::setWidth(double width)
{
  m_Width = width;
}

double EKLMGeometry::BoardGeometry::getHeight() const
{
  return m_Height;
}

void EKLMGeometry::BoardGeometry::setHeight(double height)
{
  m_Height = height;
}

double EKLMGeometry::BoardGeometry::getBaseWidth() const
{
  return m_BaseWidth;
}

void EKLMGeometry::BoardGeometry::setBaseWidth(double baseWidth)
{
  m_BaseWidth = baseWidth;
}

double EKLMGeometry::BoardGeometry::getBaseHeight() const
{
  return m_BaseHeight;
}

void EKLMGeometry::BoardGeometry::setBaseHeight(double baseHeight)
{
  m_BaseHeight = baseHeight;
}

double EKLMGeometry::BoardGeometry::getStripLength() const
{
  return m_StripLength;
}

void EKLMGeometry::BoardGeometry::setStripLength(double stripLength)
{
  m_StripLength = stripLength;
}

double EKLMGeometry::BoardGeometry::getStripWidth() const
{
  return m_StripWidth;
}

void EKLMGeometry::BoardGeometry::setStripWidth(double stripWidth)
{
  m_StripWidth = stripWidth;
}

double EKLMGeometry::BoardGeometry::getStripHeight() const
{
  return m_StripHeight;
}

void EKLMGeometry::BoardGeometry::setStripHeight(double stripHeight)
{
  m_StripHeight = stripHeight;
}

EKLMGeometry::BoardGeometry::BoardGeometry()
{
  m_Length = 0;
  m_Width = 0;
  m_Height = 0;
  m_BaseWidth = 0;
  m_BaseHeight = 0;
  m_StripLength = 0;
  m_StripWidth = 0;
  m_StripHeight = 0;
}

/* Class EKLMGeometry::BoardPosition. */

EKLMGeometry::BoardPosition::BoardPosition()
{
  m_R = 0;
  m_Phi = 0;
}

double EKLMGeometry::BoardPosition::getR() const
{
  return m_R;
}

void EKLMGeometry::BoardPosition::setR(double r)
{
  m_R = r;
}

double EKLMGeometry::BoardPosition::getPhi() const
{
  return m_Phi;
}

void EKLMGeometry::BoardPosition::setPhi(double phi)
{
  m_Phi = phi;
}

/* Class EKLMGeometry::StripBoardPosition. */

EKLMGeometry::StripBoardPosition::StripBoardPosition()
{
  m_X = 0;
}

double EKLMGeometry::StripBoardPosition::getX() const
{
  return m_X;
}

void EKLMGeometry::StripBoardPosition::setX(double x)
{
  m_X = x;
}

/* Class EKLMGeometry. */

EKLMGeometry::EKLMGeometry()
{
  m_Mode = c_DetectorNormal,
  m_NEndcaps = 0;
  m_NLayers = 0;
  m_NDetectorLayers = NULL;
  m_NSectors = 0;
  m_NPlanes = 0;
  m_NSegments = 0;
  m_NSegmentSupportElementsSector = 0;
  m_NStripsSegment = 0;
  m_NStrips = 0;
  m_NBoards = 0;
  m_NBoardsSector = 0;
  m_NStripBoards = 0;
  m_SolenoidZ = 0;
  m_LayerShiftZ = 0;
  m_SegmentSupportPosition = NULL;
  m_StripPosition = NULL;
  m_BoardPosition = NULL;
  m_StripBoardPosition = NULL;
}

EKLMGeometry::EKLMGeometry(const EKLMGeometry& geometry) :
  EKLMElementNumbers(geometry),
  m_EndcapStructureGeometry(*geometry.getEndcapStructureGeometry()),
  m_EndcapPosition(*geometry.getEndcapPosition()),
  m_LayerPosition(*geometry.getLayerPosition()),
  m_SectorPosition(*geometry.getSectorPosition()),
  m_SectorSupportPosition(*geometry.getSectorSupportPosition()),
  m_SectorSupportGeometry(*geometry.getSectorSupportGeometry()),
  m_PlanePosition(*geometry.getPlanePosition()),
  m_PlasticSheetGeometry(*geometry.getPlasticSheetGeometry()),
  m_SegmentSupportGeometry(*geometry.getSegmentSupportGeometry()),
  m_StripGeometry(*geometry.getStripGeometry()),
  m_ShieldGeometry(*geometry.getShieldGeometry()),
  m_BoardGeometry(*geometry.getBoardGeometry())
{
  int i, j;
  m_Mode = geometry.getDetectorMode();
  m_NEndcaps = geometry.getNEndcaps();
  m_NLayers = geometry.getNLayers();
  m_NDetectorLayers = new int[m_NEndcaps];
  m_NDetectorLayers[0] = geometry.getNDetectorLayers(1);
  if (m_NEndcaps == 2)
    m_NDetectorLayers[1] = geometry.getNDetectorLayers(2);
  m_NSectors = geometry.getNSectors();
  m_NPlanes = geometry.getNPlanes();
  m_NSegments = geometry.getNSegments();
  m_NSegmentSupportElementsSector = geometry.getNSegmentSupportElementsSector();
  m_NStripsSegment = geometry.getNStripsSegment();
  m_NStrips = geometry.getNStrips();
  m_NBoards = geometry.getNBoards();
  m_NBoardsSector = geometry.getNBoardsSector();
  m_NStripBoards = geometry.getNStripBoards();
  m_SolenoidZ = geometry.getSolenoidZ();
  m_LayerShiftZ = geometry.getLayerShiftZ();
  m_SegmentSupportPosition =
    new SegmentSupportPosition[m_NSegmentSupportElementsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j <= m_NSegments; j++) {
      m_SegmentSupportPosition[i * (m_NSegments + 1) + j] =
        *geometry.getSegmentSupportPosition(i + 1, j + 1);
    }
  }
  m_StripPosition = new struct ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
  m_BoardPosition = new BoardPosition[m_NBoardsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j < m_NBoards; j++) {
      m_BoardPosition[i * m_NBoards + i] =
        *geometry.getBoardPosition(i + 1, j + 1);
    }
  }
  m_StripBoardPosition = new StripBoardPosition[m_NStripBoards];
  for (i = 0; i < m_NStripBoards; i++)
    m_StripBoardPosition[i] = *geometry.getStripBoardPosition(i + 1);
}

EKLMGeometry::~EKLMGeometry()
{
  if (m_NDetectorLayers != NULL)
    delete[] m_NDetectorLayers;
  if (m_SegmentSupportPosition != NULL)
    delete[] m_SegmentSupportPosition;
  if (m_StripPosition != NULL)
    delete[] m_StripPosition;
  if (m_BoardPosition != NULL)
    delete[] m_BoardPosition;
  if (m_StripBoardPosition != NULL)
    delete[] m_StripBoardPosition;
}

EKLMGeometry& EKLMGeometry::operator=(const EKLMGeometry& geometry)
{
  int i, j;
  if (&geometry == this)
    return *this;
  m_Mode = geometry.getDetectorMode();
  m_NEndcaps = geometry.getNEndcaps();
  m_NLayers = geometry.getNLayers();
  if (m_NDetectorLayers != NULL)
    delete[] m_NDetectorLayers;
  m_NDetectorLayers = new int[m_NEndcaps];
  m_NDetectorLayers[0] = geometry.getNDetectorLayers(1);
  if (m_NEndcaps == 2)
    m_NDetectorLayers[1] = geometry.getNDetectorLayers(2);
  m_NSectors = geometry.getNSectors();
  m_NPlanes = geometry.getNPlanes();
  m_NSegments = geometry.getNSegments();
  m_NSegmentSupportElementsSector = geometry.getNSegmentSupportElementsSector();
  m_NStripsSegment = geometry.getNStripsSegment();
  m_NStrips = geometry.getNStrips();
  m_NBoards = geometry.getNBoards();
  m_NBoardsSector = geometry.getNBoardsSector();
  m_NStripBoards = geometry.getNStripBoards();
  m_SolenoidZ = geometry.getSolenoidZ();
  m_EndcapStructureGeometry = *geometry.getEndcapStructureGeometry();
  m_EndcapPosition = *geometry.getEndcapPosition();
  m_LayerPosition = *geometry.getLayerPosition();
  m_LayerShiftZ = geometry.getLayerShiftZ();
  m_SectorPosition = *geometry.getSectorPosition();
  m_SectorSupportPosition = *geometry.getSectorSupportPosition();
  m_SectorSupportGeometry = *geometry.getSectorSupportGeometry();
  m_PlanePosition = *geometry.getPlanePosition();
  m_PlasticSheetGeometry = *geometry.getPlasticSheetGeometry();
  m_SegmentSupportGeometry = *geometry.getSegmentSupportGeometry();
  if (m_SegmentSupportPosition != NULL)
    delete[] m_SegmentSupportPosition;
  m_SegmentSupportPosition =
    new SegmentSupportPosition[m_NSegmentSupportElementsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j <= m_NSegments; j++) {
      m_SegmentSupportPosition[i * (m_NSegments + 1) + j] =
        *geometry.getSegmentSupportPosition(i + 1, j + 1);
    }
  }
  m_StripGeometry = *geometry.getStripGeometry();
  if (m_StripPosition != NULL)
    delete[] m_StripPosition;
  m_StripPosition = new struct ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
  m_ShieldGeometry = *geometry.getShieldGeometry();
  m_BoardGeometry = *geometry.getBoardGeometry();
  if (m_BoardPosition != NULL)
    delete[] m_BoardPosition;
  m_BoardPosition = new BoardPosition[m_NBoardsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j < m_NBoards; j++) {
      m_BoardPosition[i * m_NBoards + i] =
        *geometry.getBoardPosition(i + 1, j + 1);
    }
  }
  if (m_StripBoardPosition != NULL)
    delete[] m_StripBoardPosition;
  m_StripBoardPosition = new StripBoardPosition[m_NStripBoards];
  for (i = 0; i < m_NStripBoards; i++)
    m_StripBoardPosition[i] = *geometry.getStripBoardPosition(i + 1);
  return *this;
}

/* Global settings. */

EKLMGeometry::DetectorMode EKLMGeometry::getDetectorMode() const
{
  return m_Mode;
}

/* Numbers of geometry elements. */

int EKLMGeometry::getNEndcaps() const
{
  return m_NEndcaps;
}

int EKLMGeometry::getNLayers() const
{
  return m_NLayers;
}

int EKLMGeometry::getNDetectorLayers(int endcap) const
{
  checkEndcap(endcap);
  return m_NDetectorLayers[endcap - 1];
}

int EKLMGeometry::getNSectors() const
{
  return m_NSectors;
}

int EKLMGeometry::getNPlanes() const
{
  return m_NPlanes;
}

int EKLMGeometry::getNSegments() const
{
  return m_NSegments;
}

int EKLMGeometry::getNSegmentSupportElementsSector() const
{
  return m_NSegmentSupportElementsSector;
}

int EKLMGeometry::getNStripsSegment() const
{
  return m_NStripsSegment;
}

int EKLMGeometry::getNStrips() const
{
  return m_NStrips;
}

int EKLMGeometry::getNBoards() const
{
  return m_NBoards;
}

int EKLMGeometry::getNBoardsSector() const
{
  return m_NBoardsSector;
}

int EKLMGeometry::getNStripBoards() const
{
  return m_NStripBoards;
}

/* Element number checks. */

void EKLMGeometry::checkDetectorLayerNumber(int endcap, int layer) const
{
  const char* endcapName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_NLayers ||
      layer > m_MaximalDetectorLayerNumber[endcap - 1])
    B2FATAL("Number of detector layers in the " << endcapName[endcap - 1] <<
            " endcap must be from 0 to the number of layers ( " <<
            m_NLayers << ").");
}

void EKLMGeometry::checkDetectorLayer(int endcap, int layer) const
{
  const char* endcapName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_NDetectorLayers[endcap - 1])
    B2FATAL("Number of layer must be less from 1 to the number of "
            "detector layers in the " << endcapName[endcap - 1] << " endcap ("
            << m_NDetectorLayers[endcap - 1] << ").");
}

void EKLMGeometry::checkSegmentSupport(int support) const
{
  if (support <= 0 || support > m_MaximalSegmentNumber + 1)
    B2FATAL("Number of segment support element must be from 1 to " <<
            m_MaximalSegmentNumber + 1 << ".");
}

void EKLMGeometry::checkStripSegment(int strip) const
{
  if (strip <= 0 || strip > m_NStripsSegment)
    B2FATAL("Number of strip in a segment must be from 1 to " <<
            m_NStripsSegment << ".");
}

/* Positions, coordinates, sizes. */

double EKLMGeometry::getSolenoidZ() const
{
  return m_SolenoidZ;
}

const EKLMGeometry::EndcapStructureGeometry*
EKLMGeometry::getEndcapStructureGeometry() const
{
  return &m_EndcapStructureGeometry;
}

const EKLMGeometry::ElementPosition* EKLMGeometry::getEndcapPosition() const
{
  return &m_EndcapPosition;
}

const EKLMGeometry::ElementPosition* EKLMGeometry::getLayerPosition() const
{
  return &m_LayerPosition;
}

double EKLMGeometry::getLayerShiftZ() const
{
  return m_LayerShiftZ;
}

const EKLMGeometry::ElementPosition* EKLMGeometry::getSectorPosition() const
{
  return &m_SectorPosition;
}

const EKLMGeometry::ElementPosition*
EKLMGeometry::getSectorSupportPosition() const
{
  return &m_SectorSupportPosition;
}

const EKLMGeometry::SectorSupportGeometry*
EKLMGeometry::getSectorSupportGeometry() const
{
  return &m_SectorSupportGeometry;
}

const EKLMGeometry::ElementPosition* EKLMGeometry::getPlanePosition() const
{
  return &m_PlanePosition;
}

const EKLMGeometry::PlasticSheetGeometry*
EKLMGeometry::getPlasticSheetGeometry() const
{
  return &m_PlasticSheetGeometry;
}

const EKLMGeometry::SegmentSupportGeometry*
EKLMGeometry::getSegmentSupportGeometry() const
{
  return &m_SegmentSupportGeometry;
}

const EKLMGeometry::SegmentSupportPosition*
EKLMGeometry::getSegmentSupportPosition(int plane, int support) const
{
  checkPlane(plane);
  checkSegmentSupport(support);
  return &m_SegmentSupportPosition[(plane - 1) * (m_NSegments + 1) +
                                   support - 1];
}

const EKLMGeometry::StripGeometry* EKLMGeometry::getStripGeometry() const
{
  return &m_StripGeometry;
}

const EKLMGeometry::ElementPosition*
EKLMGeometry::getStripPosition(int strip) const
{
  checkStrip(strip);
  return &m_StripPosition[strip - 1];
}

const EKLMGeometry::ShieldGeometry* EKLMGeometry::getShieldGeometry() const
{
  return &m_ShieldGeometry;
}

const EKLMGeometry::BoardGeometry* EKLMGeometry::getBoardGeometry() const
{
  return &m_BoardGeometry;
}

const EKLMGeometry::BoardPosition*
EKLMGeometry::getBoardPosition(int plane, int segment) const
{
  checkPlane(plane);
  checkSegment(segment);
  return &m_BoardPosition[(plane - 1) * m_NBoards + segment - 1];
}

const EKLMGeometry::StripBoardPosition*
EKLMGeometry::getStripBoardPosition(int board) const
{
  checkStripSegment(board);
  return &m_StripBoardPosition[board - 1];
}

