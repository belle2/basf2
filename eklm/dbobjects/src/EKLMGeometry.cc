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

static const char c_ModeErr[] =
  "It is forbidden to read the requested data if mode is not "
  "c_DetectorBackground.";

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

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry()
{
  LengthX = 0;
  LengthY = 0;
  Points = NULL;
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

EKLMGeometry::EKLMGeometry() : m_SegmentSupportPosition {NULL, NULL},
  m_BoardPosition {NULL, NULL}
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
  m_StripPosition = NULL;
  m_StripBoardPosition = NULL;
}

EKLMGeometry::~EKLMGeometry()
{
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
  if (m_Mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  return m_NBoards;
}

int EKLMGeometry::getNStripBoards() const
{
  if (m_Mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
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
  return &m_SegmentSupportPosition[plane - 1][support - 1];
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
  if (m_Mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  return &m_BoardGeometry;
}

const EKLMGeometry::BoardPosition*
EKLMGeometry::getBoardPosition(int plane, int segment) const
{
  if (m_Mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  checkPlane(plane);
  checkSegment(segment);
  return &m_BoardPosition[plane - 1][segment - 1];
}

const EKLMGeometry::StripBoardPosition*
EKLMGeometry::getStripBoardPosition(int board) const
{
  if (m_Mode != c_DetectorBackground)
    B2FATAL(c_ModeErr);
  checkSegment(board);
  return &m_StripBoardPosition[board - 1];
}

