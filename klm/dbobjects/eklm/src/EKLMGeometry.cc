/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/eklm/EKLMGeometry.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

/* Class EKLMGeometry::EndcapStructureGeometry. */

EKLMGeometry::EndcapStructureGeometry::EndcapStructureGeometry()
{
}

EKLMGeometry::EndcapStructureGeometry::~EndcapStructureGeometry()
{
}

/* Class EKLMGeometry::ElementPosition. */

EKLMGeometry::ElementPosition::ElementPosition()
{
}

/* Class EKLMGeometry::SectorSupportGeometry. */

EKLMGeometry::SectorSupportGeometry::SectorSupportGeometry()
{
}

/* Class EKLMGeometry::PlasticSheetGeometry. */

EKLMGeometry::PlasticSheetGeometry::PlasticSheetGeometry()
{
}

/* Class EKLMGeometry::SegmentSupportGeometry. */

EKLMGeometry::SegmentSupportGeometry::SegmentSupportGeometry()
{
}

/* Class EKLMGeometry::SegmentSupportPosition. */

EKLMGeometry::SegmentSupportPosition::SegmentSupportPosition()
{
}

/* Class EKLMGeometry::StripGeometry. */

EKLMGeometry::StripGeometry::StripGeometry()
{
}

/* Class EKLMGeometry::Point. */

EKLMGeometry::Point::Point()
{
}

/* Class EKLMGeometry::ShieldDetailGeometry. */

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry()
{
}

EKLMGeometry::ShieldDetailGeometry::ShieldDetailGeometry(
  const ShieldDetailGeometry& geometry) : TObject(geometry)
{
  /* cppcheck-suppress variableScope */
  int i;
  m_LengthX = geometry.getLengthX();
  m_LengthY = geometry.getLengthY();
  m_NPoints = geometry.getNPoints();
  if (m_NPoints > 0) {
    m_Points = new Point[m_NPoints];
    for (i = 0; i < m_NPoints; i++)
      m_Points[i] = *geometry.getPoint(i);
  } else
    m_Points = nullptr;
}

EKLMGeometry::ShieldDetailGeometry&
EKLMGeometry::ShieldDetailGeometry::operator=(
  const ShieldDetailGeometry& geometry)
{
  /* cppcheck-suppress variableScope */
  int i;
  if (&geometry == this)
    return *this;
  m_LengthX = geometry.getLengthX();
  m_LengthY = geometry.getLengthY();
  m_NPoints = geometry.getNPoints();
  if (m_Points != nullptr)
    delete[] m_Points;
  if (m_NPoints > 0) {
    m_Points = new Point[m_NPoints];
    for (i = 0; i < m_NPoints; i++)
      m_Points[i] = *geometry.getPoint(i);
  } else
    m_Points = nullptr;
  return *this;
}

EKLMGeometry::ShieldDetailGeometry::~ShieldDetailGeometry()
{
  if (m_Points != nullptr)
    delete[] m_Points;
}

void EKLMGeometry::ShieldDetailGeometry::setNPoints(int nPoints)
{
  if (nPoints < 0)
    B2FATAL("Number of points must be nonnegative.");
  m_NPoints = nPoints;
  if (m_Points != nullptr)
    delete[] m_Points;
  if (m_NPoints > 0)
    m_Points = new Point[m_NPoints];
  else
    m_Points = nullptr;
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
}

void EKLMGeometry::ShieldGeometry::setDetailACenter(double x, double y)
{
  m_DetailACenter.setX(x);
  m_DetailACenter.setY(y);
}

void EKLMGeometry::ShieldGeometry::setDetailBCenter(double x, double y)
{
  m_DetailBCenter.setX(x);
  m_DetailBCenter.setY(y);
}

void EKLMGeometry::ShieldGeometry::setDetailCCenter(double x, double y)
{
  m_DetailCCenter.setX(x);
  m_DetailCCenter.setY(y);
}

/* Class EKLMGeometry. */

EKLMGeometry::EKLMGeometry() :
  m_ElementNumbers(&(EKLMElementNumbers::Instance())),
  m_NSections(0),
  m_NLayers(0),
  m_NDetectorLayers(nullptr),
  m_NSectors(0),
  m_NPlanes(0),
  m_NSegments(0),
  m_NSegmentSupportElementsSector(0),
  m_NStrips(0),
  m_SolenoidZ(0),
  m_LayerShiftZ(0),
  m_SegmentSupportPosition(nullptr),
  m_StripPosition(nullptr)
{
}

EKLMGeometry::EKLMGeometry(const EKLMGeometry& geometry) :
  TObject(geometry),
  m_ElementNumbers(&(EKLMElementNumbers::Instance())),
  m_EndcapStructureGeometry(*geometry.getEndcapStructureGeometry()),
  m_SectionPosition(*geometry.getSectionPosition()),
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
  m_NSections = geometry.getNSections();
  m_NLayers = geometry.getNLayers();
  m_NDetectorLayers = new int[m_NSections];
  m_NDetectorLayers[0] = geometry.getNDetectorLayers(1);
  if (m_NSections == 2)
    m_NDetectorLayers[1] = geometry.getNDetectorLayers(2);
  m_NSectors = geometry.getNSectors();
  m_NPlanes = geometry.getNPlanes();
  m_NSegments = geometry.getNSegments();
  m_NSegmentSupportElementsSector = geometry.getNSegmentSupportElementsSector();
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
  if (m_NDetectorLayers != nullptr)
    delete[] m_NDetectorLayers;
  if (m_SegmentSupportPosition != nullptr)
    delete[] m_SegmentSupportPosition;
  if (m_StripPosition != nullptr)
    delete[] m_StripPosition;
}

EKLMGeometry& EKLMGeometry::operator=(const EKLMGeometry& geometry)
{
  int i, j;
  if (&geometry == this)
    return *this;
  m_NSections = geometry.getNSections();
  m_NLayers = geometry.getNLayers();
  if (m_NDetectorLayers != nullptr)
    delete[] m_NDetectorLayers;
  m_NDetectorLayers = new int[m_NSections];
  m_NDetectorLayers[0] = geometry.getNDetectorLayers(1);
  if (m_NSections == 2)
    m_NDetectorLayers[1] = geometry.getNDetectorLayers(2);
  m_NSectors = geometry.getNSectors();
  m_NPlanes = geometry.getNPlanes();
  m_NSegments = geometry.getNSegments();
  m_NSegmentSupportElementsSector = geometry.getNSegmentSupportElementsSector();
  m_NStrips = geometry.getNStrips();
  m_SolenoidZ = geometry.getSolenoidZ();
  m_EndcapStructureGeometry = *geometry.getEndcapStructureGeometry();
  m_SectionPosition = *geometry.getSectionPosition();
  m_LayerPosition = *geometry.getLayerPosition();
  m_LayerShiftZ = geometry.getLayerShiftZ();
  m_SectorPosition = *geometry.getSectorPosition();
  m_SectorSupportPosition = *geometry.getSectorSupportPosition();
  m_SectorSupportGeometry = *geometry.getSectorSupportGeometry();
  m_PlanePosition = *geometry.getPlanePosition();
  m_PlasticSheetGeometry = *geometry.getPlasticSheetGeometry();
  m_SegmentSupportGeometry = *geometry.getSegmentSupportGeometry();
  if (m_SegmentSupportPosition != nullptr)
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
  if (m_StripPosition != nullptr)
    delete[] m_StripPosition;
  m_StripPosition = new ElementPosition[m_NStrips];
  for (i = 0; i < m_NStrips; i++)
    m_StripPosition[i] = *geometry.getStripPosition(i + 1);
  m_ShieldGeometry = *geometry.getShieldGeometry();
  return *this;
}

/* Numbers of geometry elements. */

int EKLMGeometry::getNDetectorLayers(int section) const
{
  m_ElementNumbers->checkSection(section);
  return m_NDetectorLayers[section - 1];
}

/* Element number checks. */

void EKLMGeometry::checkDetectorLayerNumber(int section, int layer) const
{
  /* cppcheck-suppress variableScope */
  const char* sectionName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_NLayers ||
      layer > m_ElementNumbers->getMaximalDetectorLayerNumber(section))
    B2FATAL("Number of detector layers in the " << sectionName[section - 1] <<
            " section must be from 0 to the number of layers ( " <<
            m_NLayers << ").");
}

void EKLMGeometry::checkDetectorLayer(int section, int layer) const
{
  /* cppcheck-suppress variableScope */
  const char* sectionName[2] = {"backward", "forward"};
  if (layer < 0 || layer > m_NDetectorLayers[section - 1])
    B2FATAL("Number of layer must be less from 1 to the number of "
            "detector layers in the " << sectionName[section - 1] << " section ("
            << m_NDetectorLayers[section - 1] << ").");
}

void EKLMGeometry::checkSegmentSupport(int support) const
{
  if (support <= 0 || support > m_ElementNumbers->getMaximalSegmentNumber() + 1)
    B2FATAL("Number of segment support element must be from 1 to " <<
            m_ElementNumbers->getMaximalSegmentNumber() + 1 << ".");
}

void EKLMGeometry::checkStripSegment(int strip) const
{
  if (strip <= 0 || strip > m_ElementNumbers->getNStripsSegment())
    B2FATAL("Number of strip in a segment must be from 1 to " <<
            m_ElementNumbers->getNStripsSegment() << ".");
}

/* Positions, coordinates, sizes. */

const EKLMGeometry::SegmentSupportPosition*
EKLMGeometry::getSegmentSupportPosition(int plane, int support) const
{
  m_ElementNumbers->checkPlane(plane);
  checkSegmentSupport(support);
  return &m_SegmentSupportPosition[(plane - 1) * (m_NSegments + 1) +
                                   support - 1];
}

const EKLMGeometry::ElementPosition*
EKLMGeometry::getStripPosition(int strip) const
{
  m_ElementNumbers->checkStrip(strip);
  return &m_StripPosition[strip - 1];
}
