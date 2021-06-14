/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/AlignmentChecker.h>

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMAlignment.h>
#include <klm/eklm/geometry/Polygon2D.h>

/* Belle 2 headers. */
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLM::AlignmentChecker::AlignmentChecker(bool printOverlaps) :
  m_PrintOverlaps(printOverlaps),
  m_GeoDat(&(EKLM::GeometryData::Instance())),
  m_ElementNumbers(&(EKLMElementNumbers::Instance()))
{
  int iPlane, iSegmentSupport;
  const EKLMGeometry::SectorSupportGeometry* sectorSupportGeometry =
    m_GeoDat->getSectorSupportGeometry();
  const EKLMGeometry::ElementPosition* sectorSupportPosition =
    m_GeoDat->getSectorSupportPosition();
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
         iSegmentSupport++)
      m_SegmentSupport[iPlane - 1][iSegmentSupport - 1] = nullptr;
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
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1] != nullptr)
        delete m_SegmentSupport[iPlane - 1][iSegmentSupport - 1];
    }
    delete[] m_SegmentSupport[iPlane - 1];
  }
  delete[] m_SegmentSupport;
}

bool EKLM::AlignmentChecker::
checkSectorAlignment(int section, int layer, int sector,
                     const KLMAlignmentData* sectorAlignment) const
{
  int iPlane, iSegmentSupport, iSegment, j;
  double lx, ly;
  HepGeom::Point3D<double> supportRectangle[4];
  HepGeom::Transform3D t;
  const EKLMGeometry::SegmentSupportPosition* segmentSupportPos;
  const EKLMGeometry::SegmentSupportGeometry* segmentSupportGeometry =
    m_GeoDat->getSegmentSupportGeometry();
  KLMAlignmentData segmentAlignment(0, 0, 0, 0, 0, 0);
  for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
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
      t = HepGeom::Translate3D(sectorAlignment->getDeltaU() * CLHEP::cm / Unit::cm,
                               sectorAlignment->getDeltaV() * CLHEP::cm / Unit::cm,
                               0) *
          HepGeom::RotateZ3D(sectorAlignment->getDeltaGamma() *
                             CLHEP::rad / Unit::rad) * t;
      for (j = 0; j < 4; j++)
        supportRectangle[j] = t * supportRectangle[j];
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1] != nullptr)
        delete m_SegmentSupport[iPlane - 1][iSegmentSupport - 1];
      m_SegmentSupport[iPlane - 1][iSegmentSupport - 1] =
        new Polygon2D(supportRectangle, 4);
    }
  }
  for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
    for (iSegmentSupport = 1; iSegmentSupport <= m_GeoDat->getNSegments() + 1;
         iSegmentSupport++) {
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1]->hasIntersection(
            *m_LineCorner1)) {
        if (m_PrintOverlaps)
          B2ERROR("Segment support overlaps with corner 1."
                  << LogVar("Section", section) << LogVar("Layer", layer)
                  << LogVar("Sector", sector)
                  << LogVar("Segment support", iSegmentSupport));
        return false;
      }
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1]->hasIntersection(
            *m_ArcOuter)) {
        if (m_PrintOverlaps)
          B2ERROR("Segment support overlaps with outer arc."
                  << LogVar("Section", section) << LogVar("Layer", layer)
                  << LogVar("Sector", sector)
                  << LogVar("Segment support", iSegmentSupport));
        return false;
      }
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1]->hasIntersection(
            *m_Line23)) {
        if (m_PrintOverlaps)
          B2ERROR("Segment support overlaps with line 2-3."
                  << LogVar("Section", section) << LogVar("Layer", layer)
                  << LogVar("Sector", sector)
                  << LogVar("Segment support", iSegmentSupport));
        return false;
      }
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1]->hasIntersection(
            *m_ArcInner)) {
        if (m_PrintOverlaps)
          B2ERROR("Segment support overlaps with inner arc."
                  << LogVar("Section", section) << LogVar("Layer", layer)
                  << LogVar("Sector", sector)
                  << LogVar("Segment support", iSegmentSupport));
        return false;
      }
      if (m_SegmentSupport[iPlane - 1][iSegmentSupport - 1]->hasIntersection(
            *m_Line41)) {
        if (m_PrintOverlaps)
          B2ERROR("Segment support overlaps with line 4-1."
                  << LogVar("Section", section) << LogVar("Layer", layer)
                  << LogVar("Sector", sector)
                  << LogVar("Segment support", iSegmentSupport));
        return false;
      }
    }
  }
  for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
    for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
      if (!checkSegmentAlignment(section, layer, sector, iPlane, iSegment,
                                 sectorAlignment, &segmentAlignment, true))
        return false;
    }
  }
  return true;
}

bool EKLM::AlignmentChecker::
checkSegmentAlignment(int section, int layer, int sector, int plane, int segment,
                      const KLMAlignmentData* sectorAlignment,
                      const KLMAlignmentData* segmentAlignment,
                      bool calledFromSectorCheck) const
{
  /* cppcheck-suppress variableScope */
  int i, j, iStrip;
  /* cppcheck-suppress variableScope */
  double lx, ly;
  HepGeom::Point3D<double> stripRectangle[4];
  HepGeom::Transform3D t;
  const EKLMGeometry::StripGeometry* stripGeometry =
    m_GeoDat->getStripGeometry();
  if (!calledFromSectorCheck) {
    if (!checkSectorAlignment(section, layer, sector, sectorAlignment))
      return false;
  }
  ly = 0.5 * stripGeometry->getWidth();
  for (i = 1; i <= m_ElementNumbers->getNStripsSegment(); i++) {
    iStrip = m_ElementNumbers->getNStripsSegment() * (segment - 1) + i;
    const EKLMGeometry::ElementPosition* stripPosition =
      m_GeoDat->getStripPosition(iStrip);
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
    t = HepGeom::Translate3D(segmentAlignment->getDeltaU() * CLHEP::cm / Unit::cm,
                             segmentAlignment->getDeltaV() * CLHEP::cm / Unit::cm,
                             0) *
        HepGeom::Translate3D(stripPosition->getX(), stripPosition->getY(), 0) *
        HepGeom::RotateZ3D(segmentAlignment->getDeltaGamma() *
                           CLHEP::rad / Unit::rad);
    if (plane == 1)
      t = HepGeom::Rotate3D(180. * CLHEP::deg,
                            HepGeom::Vector3D<double>(1., 1., 0.)) * t;
    t = HepGeom::Translate3D(sectorAlignment->getDeltaU() * CLHEP::cm / Unit::cm,
                             sectorAlignment->getDeltaV() * CLHEP::cm / Unit::cm,
                             0) *
        HepGeom::RotateZ3D(sectorAlignment->getDeltaGamma() *
                           CLHEP::rad / Unit::rad) * t;
    for (j = 0; j < 4; j++)
      stripRectangle[j] = t * stripRectangle[j];
    Polygon2D stripPolygon(stripRectangle, 4);
    if (stripPolygon.hasIntersection(*m_LineCorner1)) {
      if (m_PrintOverlaps)
        B2ERROR("Strip overlaps with corner 1."
                << LogVar("Section", section) << LogVar("Layer", layer)
                << LogVar("Sector", sector) << LogVar("Plane", plane)
                << LogVar("Strip", iStrip));
      return false;
    }
    if (stripPolygon.hasIntersection(*m_ArcOuter)) {
      if (m_PrintOverlaps)
        B2ERROR("Strip overlaps with outer arc."
                << LogVar("Section", section) << LogVar("Layer", layer)
                << LogVar("Sector", sector) << LogVar("Plane", plane)
                << LogVar("Strip", iStrip));
      B2ERROR("Overlap (section " << section << ", layer " << layer <<
              ", sector " << sector << ", plane " << plane <<
              "): strip " << iStrip << ", outer arc.");
      return false;
    }
    if (stripPolygon.hasIntersection(*m_Line23)) {
      if (m_PrintOverlaps)
        B2ERROR("Strip overlaps with line 2-3."
                << LogVar("Section", section) << LogVar("Layer", layer)
                << LogVar("Sector", sector) << LogVar("Plane", plane)
                << LogVar("Strip", iStrip));
      return false;
    }
    if (stripPolygon.hasIntersection(*m_ArcInner)) {
      if (m_PrintOverlaps)
        B2ERROR("Strip overlaps with inner arc."
                << LogVar("Section", section) << LogVar("Layer", layer)
                << LogVar("Sector", sector) << LogVar("Plane", plane)
                << LogVar("Strip", iStrip));
      B2ERROR("Overlap (section " << section << ", layer " << layer <<
              ", sector " << sector << ", plane " << plane <<
              "): strip " << iStrip << ", inner arc.");
      return false;
    }
    if (stripPolygon.hasIntersection(*m_Line41)) {
      if (m_PrintOverlaps)
        B2ERROR("Strip overlaps with line 4-1."
                << LogVar("Section", section) << LogVar("Layer", layer)
                << LogVar("Sector", sector) << LogVar("Plane", plane)
                << LogVar("Strip", iStrip));
      return false;
    }
    for (j = 0; j <= m_GeoDat->getNSegments(); j++) {
      if (stripPolygon.hasIntersection(*m_SegmentSupport[plane - 1][j])) {
        if (m_PrintOverlaps)
          B2ERROR("Strip overlaps with segment support."
                  << LogVar("Section", section) << LogVar("Layer", layer)
                  << LogVar("Sector", sector) << LogVar("Plane", plane)
                  << LogVar("Strip", iStrip)
                  << LogVar("Segment support", j + 1));
        return false;
      }
    }
  }
  return true;
}

bool EKLM::AlignmentChecker::checkAlignment(
  const EKLMAlignment* alignment,
  const EKLMSegmentAlignment* segmentAlignment) const
{
  int iSection, iLayer, iSector, iPlane, iSegment, sector, segment;
  for (iSection = 1; iSection <= m_GeoDat->getNSections(); iSection++) {
    for (iLayer = 1; iLayer <= m_GeoDat->getNDetectorLayers(iSection);
         iLayer++) {
      for (iSector = 1; iSector <= m_GeoDat->getNSectors(); iSector++) {
        sector = m_ElementNumbers->sectorNumber(iSection, iLayer, iSector);
        const KLMAlignmentData* sectorAlignment =
          alignment->getModuleAlignment(sector);
        if (sectorAlignment == nullptr)
          B2FATAL("Incomplete alignment data.");
        if (!checkSectorAlignment(iSection, iLayer, iSector, sectorAlignment))
          return false;
        for (iPlane = 1; iPlane <= m_GeoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= m_GeoDat->getNSegments(); iSegment++) {
            segment = m_ElementNumbers->segmentNumber(
                        iSection, iLayer, iSector, iPlane, iSegment);
            const KLMAlignmentData* segmentAlignmentData =
              /* cppcheck-suppress nullPointerRedundantCheck */
              segmentAlignment->getSegmentAlignment(segment);
            if (segmentAlignment == nullptr)
              B2FATAL("Incomplete alignment data.");
            if (!checkSegmentAlignment(iSection, iLayer, iSector, iPlane,
                                       iSegment, sectorAlignment,
                                       segmentAlignmentData, false))
              return false;
          }
        }
      }
    }
  }
  return true;
}

