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

/* Class EKLMGeometry::EndcapStructureGeometry. */

EKLMGeometry::EndcapStructureGeometry::EndcapStructureGeometry()
{
  m_Phi = 0;
  m_NSides = 0;
}

EKLMGeometry::EndcapStructureGeometry::~EndcapStructureGeometry()
{
}

double EKLMGeometry::EndcapStructureGeometry::getPhi() const
{
  return m_Phi;
}

void EKLMGeometry::EndcapStructureGeometry::setPhi(double phi)
{
  m_Phi = phi;
}

int EKLMGeometry::EndcapStructureGeometry::getNSides() const
{
  return m_NSides;
}

void EKLMGeometry::EndcapStructureGeometry::setNSides(int nSides)
{
  m_NSides = nSides;
}

/* Class EKLMGeometry::ElementPosition. */

EKLMGeometry::ElementPosition::ElementPosition()
{
  m_InnerR = 0;
  m_OuterR = 0;
  m_Length = 0;
  m_X = 0;
  m_Y = 0;
  m_Z = 0;
}

double EKLMGeometry::ElementPosition::getInnerR() const
{
  return m_InnerR;
}

void EKLMGeometry::ElementPosition::setInnerR(double innerR)
{
  m_InnerR = innerR;
}

double EKLMGeometry::ElementPosition::getOuterR() const
{
  return m_OuterR;
}

void EKLMGeometry::ElementPosition::setOuterR(double outerR)
{
  m_OuterR = outerR;
}

double EKLMGeometry::ElementPosition::getLength() const
{
  return m_Length;
}

void EKLMGeometry::ElementPosition::setLength(double length)
{
  m_Length = length;
}

double EKLMGeometry::ElementPosition::getX() const
{
  return m_X;
}

void EKLMGeometry::ElementPosition::setX(double x)
{
  m_X = x;
}

double EKLMGeometry::ElementPosition::getY() const
{
  return m_Y;
}

void EKLMGeometry::ElementPosition::setY(double y)
{
  m_Y = y;
}

double EKLMGeometry::ElementPosition::getZ() const
{
  return m_Z;
}

void EKLMGeometry::ElementPosition::setZ(double z)
{
  m_Z = z;
}

/* Class EKLMGeometry::SectorSupportGeometry. */

EKLMGeometry::SectorSupportGeometry::SectorSupportGeometry()
{
  m_Thickness = 0;
  m_DeltaLY = 0;
  m_CornerX = 0;
  m_Corner1LX = 0;
  m_Corner1Width = 0;
  m_Corner1Thickness = 0;
  m_Corner1Z = 0;
  m_Corner2LX = 0;
  m_Corner2LY = 0;
  m_Corner2Thickness = 0;
  m_Corner2Z = 0;
  m_Corner3LX = 0;
  m_Corner3LY = 0;
  m_Corner3Thickness = 0;
  m_Corner3Z = 0;
  m_Corner4LX = 0;
  m_Corner4LY = 0;
  m_Corner4Thickness = 0;
  m_Corner4Z = 0;
  m_CornerAngle = 0;
}

double EKLMGeometry::SectorSupportGeometry::getThickness() const
{
  return m_Thickness;
}

void EKLMGeometry::SectorSupportGeometry::setThickness(double thickness)
{
  m_Thickness = thickness;
}

double EKLMGeometry::SectorSupportGeometry::getDeltaLY() const
{
  return m_DeltaLY;
}

void EKLMGeometry::SectorSupportGeometry::setDeltaLY(double deltaLY)
{
  m_DeltaLY = deltaLY;
}

double EKLMGeometry::SectorSupportGeometry::getCornerX() const
{
  return m_CornerX;
}

void EKLMGeometry::SectorSupportGeometry::setCornerX(double cornerX)
{
  m_CornerX = cornerX;
}

double EKLMGeometry::SectorSupportGeometry::getCorner1LX() const
{
  return m_Corner1LX;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1LX(double corner1LX)
{
  m_Corner1LX = corner1LX;
}

double EKLMGeometry::SectorSupportGeometry::getCorner1Width() const
{
  return m_Corner1Width;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1Width(double corner1Width)
{
  m_Corner1Width = corner1Width;
}

double EKLMGeometry::SectorSupportGeometry::getCorner1Thickness() const
{
  return m_Corner1Thickness;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1Thickness(
  double corner1Thickness)
{
  m_Corner1Thickness = corner1Thickness;
}

double EKLMGeometry::SectorSupportGeometry::getCorner1Z() const
{
  return m_Corner1Z;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1Z(double corner1Z)
{
  m_Corner1Z = corner1Z;
}

double EKLMGeometry::SectorSupportGeometry::getCorner2LX() const
{
  return m_Corner2LX;
}

void EKLMGeometry::SectorSupportGeometry::setCorner2LX(double corner2LX)
{
  m_Corner2LX = corner2LX;
}

double EKLMGeometry::SectorSupportGeometry::getCorner2LY() const
{
  return m_Corner2LY;
}

void EKLMGeometry::SectorSupportGeometry::setCorner2LY(double corner2LY)
{
  m_Corner2LY = corner2LY;
}

double EKLMGeometry::SectorSupportGeometry::getCorner2Thickness() const
{
  return m_Corner2Thickness;
}

void EKLMGeometry::SectorSupportGeometry::setCorner2Thickness(
  double corner2Thickness)
{
  m_Corner2Thickness = corner2Thickness;
}

double EKLMGeometry::SectorSupportGeometry::getCorner2Z() const
{
  return m_Corner2Z;
}

void EKLMGeometry::SectorSupportGeometry::setCorner2Z(double corner2Z)
{
  m_Corner2Z = corner2Z;
}

double EKLMGeometry::SectorSupportGeometry::getCorner3LX() const
{
  return m_Corner3LX;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3LX(double corner3LX)
{
  m_Corner3LX = corner3LX;
}

double EKLMGeometry::SectorSupportGeometry::getCorner3LY() const
{
  return m_Corner3LY;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3LY(double corner3LY)
{
  m_Corner3LY = corner3LY;
}

double EKLMGeometry::SectorSupportGeometry::getCorner3Thickness() const
{
  return m_Corner3Thickness;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3Thickness(
  double corner3Thickness)
{
  m_Corner3Thickness = corner3Thickness;
}

double EKLMGeometry::SectorSupportGeometry::getCorner3Z() const
{
  return m_Corner3Z;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3Z(double corner3Z)
{
  m_Corner3Z = corner3Z;
}

double EKLMGeometry::SectorSupportGeometry::getCorner4LX() const
{
  return m_Corner4LX;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4LX(double corner4LX)
{
  m_Corner4LX = corner4LX;
}

double EKLMGeometry::SectorSupportGeometry::getCorner4LY() const
{
  return m_Corner4LY;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4LY(double corner4LY)
{
  m_Corner4LY = corner4LY;
}

double EKLMGeometry::SectorSupportGeometry::getCorner4Thickness() const
{
  return m_Corner4Thickness;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4Thickness(
  double corner4Thickness)
{
  m_Corner4Thickness = corner4Thickness;
}

double EKLMGeometry::SectorSupportGeometry::getCorner4Z() const
{
  return m_Corner4Z;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4Z(double corner4Z)
{
  m_Corner4Z = corner4Z;
}

double EKLMGeometry::SectorSupportGeometry::getCornerAngle() const
{
  return m_CornerAngle;
}

void EKLMGeometry::SectorSupportGeometry::setCornerAngle(double cornerAngle)
{
  m_CornerAngle = cornerAngle;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner1A() const
{
  return m_Corner1A;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1A(
  const HepGeom::Point3D<double>& corner1A)
{
  m_Corner1A = corner1A;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner1AInner() const
{
  return m_Corner1AInner;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1AInner(
  const HepGeom::Point3D<double>& corner1AInner)
{
  m_Corner1AInner = corner1AInner;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner1B() const
{
  return m_Corner1B;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1B(
  const HepGeom::Point3D<double>& corner1B)
{
  m_Corner1B = corner1B;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner1BInner() const
{
  return m_Corner1BInner;
}

void EKLMGeometry::SectorSupportGeometry::setCorner1BInner(
  const HepGeom::Point3D<double>& corner1BInner)
{
  m_Corner1BInner = corner1BInner;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner2Inner() const
{
  return m_Corner2Inner;
}

void EKLMGeometry::SectorSupportGeometry::setCorner2Inner(
  const HepGeom::Point3D<double>& corner2Inner)
{
  m_Corner2Inner = corner2Inner;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner3() const
{
  return m_Corner3;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3(
  const HepGeom::Point3D<double>& corner3)
{
  m_Corner3 = corner3;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner3Inner() const
{
  return m_Corner3Inner;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3Inner(
  const HepGeom::Point3D<double>& corner3Inner)
{
  m_Corner3Inner = corner3Inner;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner3Prism() const
{
  return m_Corner3Prism;
}

void EKLMGeometry::SectorSupportGeometry::setCorner3Prism(
  const HepGeom::Point3D<double>& corner3Prism)
{
  m_Corner3Prism = corner3Prism;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner4() const
{
  return m_Corner4;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4(
  const HepGeom::Point3D<double>& corner4)
{
  m_Corner4 = corner4;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner4Inner() const
{
  return m_Corner4Inner;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4Inner(
  const HepGeom::Point3D<double>& corner4Inner)
{
  m_Corner4Inner = corner4Inner;
}

const HepGeom::Point3D<double>&
EKLMGeometry::SectorSupportGeometry::getCorner4Prism() const
{
  return m_Corner4Prism;
}

void EKLMGeometry::SectorSupportGeometry::setCorner4Prism(
  const HepGeom::Point3D<double>& corner4Prism)
{
  m_Corner4Prism = corner4Prism;
}

/* Class EKLMGeometry::PlasticSheetGeometry. */

EKLMGeometry::PlasticSheetGeometry::PlasticSheetGeometry()
{
  m_Width = 0;
  m_DeltaL = 0;
}

double EKLMGeometry::PlasticSheetGeometry::getWidth() const
{
  return m_Width;
}

void EKLMGeometry::PlasticSheetGeometry::setWidth(double width)
{
  m_Width = width;
}

double EKLMGeometry::PlasticSheetGeometry::getDeltaL() const
{
  return m_DeltaL;
}

void EKLMGeometry::PlasticSheetGeometry::setDeltaL(double deltaL)
{
  m_DeltaL = deltaL;
}

/* Class EKLMGeometry::SegmentSupportGeometry. */

EKLMGeometry::SegmentSupportGeometry::SegmentSupportGeometry()
{
  m_TopWidth = 0;
  m_TopThickness = 0;
  m_MiddleWidth = 0;
  m_MiddleThickness = 0;
}

double EKLMGeometry::SegmentSupportGeometry::getTopWidth() const
{
  return m_TopWidth;
}

void EKLMGeometry::SegmentSupportGeometry::setTopWidth(double topWidth)
{
  m_TopWidth = topWidth;
}

double EKLMGeometry::SegmentSupportGeometry::getTopThickness() const
{
  return m_TopThickness;
}

void EKLMGeometry::SegmentSupportGeometry::setTopThickness(double topThickness)
{
  m_TopThickness = topThickness;
}

double EKLMGeometry::SegmentSupportGeometry::getMiddleWidth() const
{
  return m_MiddleWidth;
}

void EKLMGeometry::SegmentSupportGeometry::setMiddleWidth(double middleWidth)
{
  m_MiddleWidth = middleWidth;
}

double EKLMGeometry::SegmentSupportGeometry::getMiddleThickness() const
{
  return m_MiddleThickness;
}

void EKLMGeometry::SegmentSupportGeometry::setMiddleThickness(
  double middleThickness)
{
  m_MiddleThickness = middleThickness;
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

/* Class EKLMGeometry. */

EKLMGeometry::EKLMGeometry()
{
  m_NEndcaps = 0;
  m_NLayers = 0;
  m_NDetectorLayers = NULL;
  m_NSectors = 0;
  m_NPlanes = 0;
  m_NSegments = 0;
  m_NSegmentSupportElementsSector = 0;
  m_NStripsSegment = 0;
  m_NStrips = 0;
  m_SolenoidZ = 0;
  m_LayerShiftZ = 0;
  m_SegmentSupportPosition = NULL;
  m_StripPosition = NULL;
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
  m_ShieldGeometry(*geometry.getShieldGeometry())
{
  int i, j;
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
  m_StripPosition = new ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
}

EKLMGeometry::~EKLMGeometry()
{
  if (m_NDetectorLayers != NULL)
    delete[] m_NDetectorLayers;
  if (m_SegmentSupportPosition != NULL)
    delete[] m_SegmentSupportPosition;
  if (m_StripPosition != NULL)
    delete[] m_StripPosition;
}

EKLMGeometry& EKLMGeometry::operator=(const EKLMGeometry& geometry)
{
  int i, j;
  if (&geometry == this)
    return *this;
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
  m_StripPosition = new ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
  m_ShieldGeometry = *geometry.getShieldGeometry();
  return *this;
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

