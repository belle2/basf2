/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/Polygon2D.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::AlignmentChecker::AlignmentChecker()
{
  int iPlane, iSegmentSupport, j;
  double lx, ly;
  HepGeom::Point3D<double> supportRectangle[4];
  HepGeom::Transform3D t;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const EKLMGeometry::ElementPosition* sectorSupportPosition =
    m_GeoDat->getSectorSupportPosition();
  const EKLMGeometry::SegmentSupportPosition* segmentSupportPos;
  const EKLMGeometry::SegmentSupportGeometry* segmentSupportGeometry =
    m_GeoDat->getSegmentSupportGeometry();
  m_LineCorner1 = new LineSegment2D(sectorSupportGeometry->getCorner1AInner(),
                                    sectorSupportGeometry->getCorner1BInner());
  m_ArcOuter = new Arc2D(
    0, 0, sectorSupportPosition->getOuterR() -
    sectorSupportGeometry->getThickness(),
    atan2(sectorSupportGeometry->getCorner2Inner().y(),
          sectorSupportGeometry->getCorner2Inner().x()),
    atan2(sectorSupportGeometry->getCorner1BInner().y(),
          sectorSupportGeometry->getCorner1BInner().x()));
  m_Line23 = new LineSegment2D(sectorSupportGeometry->getCorner2Inner(),
                               sectorSupportGeometry->getCorner3Inner());
  m_ArcInner = new Arc2D(
    0, 0, sectorSupportPosition->getInnerR() +
    sectorSupportGeometry->getThickness(),
    atan2(sectorSupportGeometry->getCorner3Inner().y(),
          sectorSupportGeometry->getCorner3Inner().x()),
    atan2(sectorSupportGeometry->getCorner4Inner().y(),
          sectorSupportGeometry->getCorner4Inner().x()));
  m_Line41 = new LineSegment2D(sectorSupportGeometry->getCorner4Inner(),
                               sectorSupportGeometry->getCorner1AInner());
  m_SegmentSupport = new Polygon2D** [m_GeoDat->getNPlanes()];
  for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
    m_SegmentSupport[iPlane - 1] =
      new Polygon2D*[m_GeoDat->getNSegments() + 1];
    for (iSegmentSupport = 1; iSegmentSupport <= m_GeoDat->getNSegments() + 1;
         iSegmentSupport++) {
      segmentSupportPos =
        m_GeoDat->getSegmentSupportPosition(iPlane, iSegmentSupport);
      lx = 0.5 * (segmentSupportPos->getLength() -
                  segmentSupportPos->getDeltaLLeft() -
                  segmentSupportPos->getDeltaLRight());
      ly = 0.5 * (segmentSupportGeometry->getMiddleWidth());
      supportRectangle[0].setX(lx);
      supportRectangle[0].setY(ly);
      supportRectangle[0].setZ(0);
      supportRectangle[1].setX(-lx);
      supportRectangle[1].setY(ly);
      supportRectangle[1].setZ(0);
      supportRectangle[2].setX(-lx);
      supportRectangle[2].setY(-ly);
      supportRectangle[2].setZ(0);
      supportRectangle[3].setX(lx);
      supportRectangle[3].setY(-ly);
      supportRectangle[3].setZ(0);
      t = HepGeom::Translate3D(
            0.5 * (segmentSupportPos->getDeltaLLeft() -
                   segmentSupportPos->getDeltaLRight()) +
            segmentSupportPos->getX(), segmentSupportPos->getY(), 0);
      if (iPlane == 1)
        t = HepGeom::Rotate3D(180. * CLHEP::deg,
                              HepGeom::Vector3D<double>(1., 1., 0.)) * t;
      for (j = 0; j < 4; j++)
        supportRectangle[j] = t * supportRectangle[j];
      m_SegmentSupport[iPlane - 1][iSegmentSupport - 1] =
        new Polygon2D(supportRectangle, 4);
    }
  }
}

EKLM::AlignmentChecker::~AlignmentChecker()
{
  int iPlane, iSegmentSupport;
  delete m_LineCorner1;
  delete m_ArcOuter;
  delete m_Line23;
  delete m_ArcInner;
  delete m_Line41;
  for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
    for (iSegmentSupport = 1; iSegmentSupport <= m_GeoDat->getNSegments() + 1;
         iSegmentSupport++) {
      delete m_SegmentSupport[iPlane - 1][iSegmentSupport - 1];
    }
    delete[] m_SegmentSupport[iPlane - 1];
  }
  delete[] m_SegmentSupport;
}

bool EKLM::AlignmentChecker::
checkSegmentAlignment(int iPlane, int iSegment,
                      EKLMAlignmentData* alignment) const
{
  int i, j, iStrip;
  double lx, ly;
  HepGeom::Point3D<double> stripRectangle[4];
  HepGeom::Transform3D t;
  const EKLMGeometry::ElementPosition* stripPosition;
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  ly = 0.5 * stripGeometry->getWidth();
  for (i = 1; i <= m_GeoDat->getNStripsSegment(); i++) {
    iStrip = m_GeoDat->getNStripsSegment() * (iSegment - 1) + i;
    stripPosition = m_GeoDat->getStripPosition(iStrip);
    lx = 0.5 * stripPosition->getLength();
    stripRectangle[0].setX(lx);
    stripRectangle[0].setY(ly);
    stripRectangle[0].setZ(0);
    stripRectangle[1].setX(-lx);
    stripRectangle[1].setY(ly);
    stripRectangle[1].setZ(0);
    stripRectangle[2].setX(-lx);
    stripRectangle[2].setY(-ly);
    stripRectangle[2].setZ(0);
    stripRectangle[3].setX(lx);
    stripRectangle[3].setY(-ly);
    stripRectangle[3].setZ(0);
    t = HepGeom::Translate3D(alignment->getDx() * CLHEP::cm / Unit::cm,
                             alignment->getDy() * CLHEP::cm / Unit::cm, 0) *
        HepGeom::Translate3D(stripPosition->getX(), stripPosition->getY(), 0) *
        HepGeom::RotateZ3D(alignment->getDalpha() * CLHEP::rad / Unit::rad);
    if (iPlane == 1)
      t = HepGeom::Rotate3D(180. * CLHEP::deg,
                            HepGeom::Vector3D<double>(1., 1., 0.)) * t;
    for (j = 0; j < 4; j++)
      stripRectangle[j] = t * stripRectangle[j];
    Polygon2D stripPolygon(stripRectangle, 4);
    if (stripPolygon.hasIntersection(*m_LineCorner1))
      return false;
    if (stripPolygon.hasIntersection(*m_ArcOuter))
      return false;
    if (stripPolygon.hasIntersection(*m_Line23))
      return false;
    if (stripPolygon.hasIntersection(*m_ArcInner))
      return false;
    if (stripPolygon.hasIntersection(*m_Line41))
      return false;
    for (j = 0; j <= m_GeoDat->getNSegments(); j++) {
      if (stripPolygon.hasIntersection(*m_SegmentSupport[iPlane - 1][j]))
        return false;
    }
  }
  return true;
}

bool EKLM::AlignmentChecker::checkAlignment(EKLMAlignment* alignment) const
{
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment;
  EKLMAlignmentData* alignmentData;
  for (iEndcap = 1; iEndcap <= m_GeoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                              iSegment);
            alignmentData = alignment->getSegmentAlignment(segment);
            if (alignmentData == NULL)
              B2FATAL("Incomplete alignment data.");
            if (!checkSegmentAlignment(iPlane, iSegment, alignmentData))
              return false;
          }
        }
      }
    }
  }
  return true;
}

