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

EKLMGeometry::SegmentSupportPosition::SegmentSupportPosition()
{
  DeltaLRight = 0;
  DeltaLLeft = 0;
  Length = 0;
  X = 0;
  Y = 0;
  Z = 0;
}

EKLMGeometry::StripGeometry::StripGeometry()
{
  Width = 0;
  Thickness = 0;
  GrooveDepth = 0;
  GrooveWidth = 0;
  NoScintillationThickness = 0;
  RSSSize = 0;
}

EKLMGeometry::Point::Point()
{
  X = 0;
  Y = 0;
}

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry()
{
  LengthX = 0;
  LengthY = 0;
  NPoints = 0;
  Points = NULL;
}

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry(
  const ShieldDetailGeometry& geometry) : TObject(geometry)
{
  int i;
  LengthX = geometry.LengthX;
  LengthY = geometry.LengthY;
  NPoints = geometry.NPoints;
  Points = new Point[NPoints];
  for (i = 0; i < NPoints; i++)
    Points[i] = geometry.Points[i];
}

EKLMGeometry::ShieldDetailGeometry&
EKLMGeometry::ShieldDetailGeometry::operator=(
  const ShieldDetailGeometry& geometry)
{
  int i;
  if (&geometry == this)
    return *this;
  LengthX = geometry.LengthX;
  LengthY = geometry.LengthY;
  NPoints = geometry.NPoints;
  if (Points != NULL)
    delete[] Points;
  Points = new Point[NPoints];
  for (i = 0; i < NPoints; i++)
    Points[i] = geometry.Points[i];
  return *this;
}

EKLMGeometry::ShieldGeometry::ShieldGeometry()
{
  Thickness = 0;
}

EKLMGeometry::BoardGeometry::BoardGeometry()
{
  Length = 0;
  Width = 0;
  Height = 0;
  BaseWidth = 0;
  BaseHeight = 0;
  StripLength = 0;
  StripWidth = 0;
  StripHeight = 0;
}

EKLMGeometry::BoardPosition::BoardPosition()
{
  R = 0;
  Phi = 0;
}

EKLMGeometry::StripBoardPosition::StripBoardPosition()
{
  X = 0;
}

EKLMGeometry::EKLMGeometry()
{
  m_Mode = c_DetectorNormal,
  m_NEndcaps = 0;
  m_NLayers = 0;
  m_NDetectorLayers = NULL;
  m_NSectors = 0;
  m_NPlanes = 0;
  m_NSegments = 0;
  m_NStripsSegment = 0;
  m_NStrips = 0;
  m_NBoards = 0;
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
    new struct SegmentSupportPosition[m_NSegmentSupportElementsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j <= m_NSegments; j++) {
      m_SegmentSupportPosition[i * (m_NSegments + 1) + j] =
        *geometry.getSegmentSupportPosition(i + 1, j + 1);
    }
  }
  m_StripPosition = new struct ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
  m_BoardPosition = new struct BoardPosition[m_NBoardsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j < m_NBoards; j++) {
      m_BoardPosition[i * m_NBoards + i] =
        *geometry.getBoardPosition(i + 1, j + 1);
    }
  }
  m_StripBoardPosition = new struct StripBoardPosition[m_NStripBoards];
  for (i = 0; i < m_NStripBoards; i++)
    m_StripBoardPosition[i] = *geometry.getStripBoardPosition(i + 1);
}

EKLMGeometry::~EKLMGeometry()
{
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
  m_LayerShiftZ = geometry.getLayerShiftZ();
  if (m_SegmentSupportPosition != NULL)
    delete[] m_SegmentSupportPosition;
  m_SegmentSupportPosition =
    new struct SegmentSupportPosition[m_NSegmentSupportElementsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j <= m_NSegments; j++) {
      m_SegmentSupportPosition[i * (m_NSegments + 1) + j] =
        *geometry.getSegmentSupportPosition(i + 1, j + 1);
    }
  }
  if (m_StripPosition != NULL)
    delete[] m_StripPosition;
  m_StripPosition = new struct ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
  if (m_BoardPosition != NULL)
    delete[] m_BoardPosition;
  m_BoardPosition = new struct BoardPosition[m_NBoardsSector];
  for (i = 0; i < m_NPlanes; i++) {
    for (j = 0; j < m_NBoards; j++) {
      m_BoardPosition[i * m_NBoards + i] =
        *geometry.getBoardPosition(i + 1, j + 1);
    }
  }
  if (m_StripBoardPosition != NULL)
    delete[] m_StripBoardPosition;
  m_StripBoardPosition = new struct StripBoardPosition[m_NStripBoards];
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

