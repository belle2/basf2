/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/TransformData.h>

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMSegmentAlignment.h>
#include <klm/eklm/geometry/AlignmentChecker.h>
#include <klm/eklm/geometry/GeometryData.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

EKLM::TransformData::TransformData(bool global, Displacement displacementType)
{
  /* cppcheck-suppress variableScope */
  int iSection, iLayer, iSector, iPlane, iSegment, iStrip, sector, segment;
  int nSections, nLayers, nSectors, nPlanes, nStrips, nSegments, nStripsSegment;
  int nDetectorLayers;
  AlignmentChecker alignmentChecker(true);
  m_ElementNumbers = &(EKLMElementNumbers::Instance());
  m_GeoDat = &(GeometryData::Instance());
  nSections = m_GeoDat->getNSections();
  nSectors = m_GeoDat->getNSectors();
  nLayers = m_GeoDat->getNLayers();
  nPlanes = m_GeoDat->getNPlanes();
  nStrips = m_GeoDat->getNStrips();
  nSegments = m_GeoDat->getNSegments();
  nStripsSegment = m_ElementNumbers->getNStripsSegment();
  m_Section = new HepGeom::Transform3D[nSections];
  m_Layer = new HepGeom::Transform3D*[nSections];
  m_Sector = new HepGeom::Transform3D** [nSections];
  m_Plane = new HepGeom::Transform3D** *[nSections];
  m_PlaneDisplacement = new HepGeom::Transform3D** *[nSections];
  m_Segment = new HepGeom::Transform3D**** [nSections];
  m_Strip = new HepGeom::Transform3D**** [nSections];
  m_StripInverse = new HepGeom::Transform3D**** [nSections];
  for (iSection = 0; iSection < nSections; iSection++) {
    m_GeoDat->getSectionTransform(&m_Section[iSection], iSection);
    nDetectorLayers = m_GeoDat->getNDetectorLayers(iSection + 1);
    m_Layer[iSection] = new HepGeom::Transform3D[nLayers];
    m_Sector[iSection] = new HepGeom::Transform3D*[nLayers];
    m_Plane[iSection] = new HepGeom::Transform3D** [nLayers];
    m_PlaneDisplacement[iSection] = new HepGeom::Transform3D** [nLayers];
    m_Segment[iSection] = new HepGeom::Transform3D** *[nLayers];
    m_Strip[iSection] = new HepGeom::Transform3D** *[nLayers];
    m_StripInverse[iSection] = new HepGeom::Transform3D** *[nLayers];
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      m_GeoDat->getLayerTransform(&m_Layer[iSection][iLayer], iLayer);
      m_Sector[iSection][iLayer] = new HepGeom::Transform3D[nSectors];
      if (iLayer < nDetectorLayers) {
        m_Plane[iSection][iLayer] = new HepGeom::Transform3D*[nSectors];
        m_PlaneDisplacement[iSection][iLayer] =
          new HepGeom::Transform3D*[nSectors];
        m_Segment[iSection][iLayer] = new HepGeom::Transform3D** [nSectors];
        m_Strip[iSection][iLayer] = new HepGeom::Transform3D** [nSectors];
        m_StripInverse[iSection][iLayer] = new HepGeom::Transform3D** [nSectors];
      }
      for (iSector = 0; iSector < nSectors; iSector++) {
        m_GeoDat->getSectorTransform(&m_Sector[iSection][iLayer][iSector],
                                     iSector);
        if (iLayer >= nDetectorLayers)
          continue;
        m_Plane[iSection][iLayer][iSector] = new HepGeom::Transform3D[nPlanes];
        m_PlaneDisplacement[iSection][iLayer][iSector] =
          new HepGeom::Transform3D[nPlanes];
        m_Segment[iSection][iLayer][iSector] =
          new HepGeom::Transform3D*[nPlanes];
        m_Strip[iSection][iLayer][iSector] = new HepGeom::Transform3D*[nPlanes];
        m_StripInverse[iSection][iLayer][iSector] =
          new HepGeom::Transform3D*[nPlanes];
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          m_GeoDat->getPlaneTransform(
            &m_Plane[iSection][iLayer][iSector][iPlane], iPlane);
          m_PlaneDisplacement[iSection][iLayer][iSector][iPlane] =
            HepGeom::Translate3D(0, 0, 0);
          m_Segment[iSection][iLayer][iSector][iPlane] =
            new HepGeom::Transform3D[nSegments];
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            m_Segment[iSection][iLayer][iSector][iPlane][iSegment] =
              HepGeom::Translate3D(0, 0, 0);
          }
          m_Strip[iSection][iLayer][iSector][iPlane] =
            new HepGeom::Transform3D[nStrips];
          m_StripInverse[iSection][iLayer][iSector][iPlane] =
            new HepGeom::Transform3D[nStrips];
          for (iStrip = 0; iStrip < nStrips; iStrip++) {
            m_GeoDat->getStripTransform(
              &m_Strip[iSection][iLayer][iSector][iPlane][iStrip], iStrip);
          }
        }
      }
    }
  }
  /* Read alignment data from the database and modify transformations. */
  if (displacementType != c_None) {
    std::string payload, segmentPayload;
    if (displacementType == c_Displacement) {
      payload = "EKLMDisplacement";
      segmentPayload = "EKLMSegmentDisplacement";
    } else {
      payload = "EKLMAlignment";
      segmentPayload = "EKLMSegmentAlignment";
    }
    DBObjPtr<EKLMAlignment> alignment(payload);
    DBObjPtr<EKLMSegmentAlignment> segmentAlignment(segmentPayload);
    if (!alignment.isValid())
      B2FATAL("No EKLM displacement (alignment) data.");
    if (displacementType == c_Displacement) {
      if (!alignmentChecker.checkAlignment(&(*alignment), &(*segmentAlignment)))
        B2FATAL("EKLM displacement data are incorrect, overlaps exist.");
    }
    for (iSection = 1; iSection <= nSections; iSection++) {
      nDetectorLayers = m_GeoDat->getNDetectorLayers(iSection);
      for (iLayer = 1; iLayer <= nDetectorLayers; iLayer++) {
        for (iSector = 1; iSector <= nSectors; iSector++) {
          sector = m_ElementNumbers->sectorNumber(iSection, iLayer, iSector);
          const KLMAlignmentData* sectorAlignment =
            alignment->getModuleAlignment(sector);
          if (sectorAlignment == nullptr)
            B2FATAL("Incomplete EKLM displacement (alignment) data.");
          for (iPlane = 1; iPlane <= nPlanes; iPlane++) {
            /* First plane is rotated. */
            if (iPlane == 1) {
              m_PlaneDisplacement[iSection - 1][iLayer - 1][iSector - 1][iPlane - 1] =
                HepGeom::Translate3D(
                  sectorAlignment->getDeltaV() * CLHEP::cm / Unit::cm,
                  sectorAlignment->getDeltaU() * CLHEP::cm / Unit::cm, 0) *
                HepGeom::RotateZ3D(-sectorAlignment->getDeltaGamma() *
                                   CLHEP::rad / Unit::rad);
            } else {
              m_PlaneDisplacement[iSection - 1][iLayer - 1][iSector - 1][iPlane - 1] =
                HepGeom::Translate3D(
                  sectorAlignment->getDeltaU() * CLHEP::cm / Unit::cm,
                  sectorAlignment->getDeltaV() * CLHEP::cm / Unit::cm, 0) *
                HepGeom::RotateZ3D(sectorAlignment->getDeltaGamma() *
                                   CLHEP::rad / Unit::rad);
            }
            for (iSegment = 1; iSegment <= nSegments; iSegment++) {
              segment = m_ElementNumbers->segmentNumber(
                          iSection, iLayer, iSector, iPlane, iSegment);
              const KLMAlignmentData* segmentAlignmentData =
                segmentAlignment->getSegmentAlignment(segment);
              if (segmentAlignmentData == nullptr)
                B2FATAL("Incomplete EKLM displacement (alignment) data.");
              m_Segment[iSection - 1][iLayer - 1][iSector - 1][iPlane - 1]
              [iSegment - 1] =
                HepGeom::Translate3D(
                  segmentAlignmentData->getDeltaU() * CLHEP::cm / Unit::cm,
                  segmentAlignmentData->getDeltaV() * CLHEP::cm / Unit::cm, 0) *
                m_Segment[iSection - 1][iLayer - 1][iSector - 1][iPlane - 1]
                [iSegment - 1] *
                HepGeom::RotateZ3D(segmentAlignmentData->getDeltaGamma() *
                                   CLHEP::rad / Unit::rad);
              for (iStrip = 1; iStrip <= nStripsSegment; iStrip++) {
                m_Strip[iSection - 1][iLayer - 1][iSector - 1][iPlane - 1]
                [nStripsSegment * (iSegment - 1) + iStrip - 1] =
                  HepGeom::Translate3D(
                    segmentAlignmentData->getDeltaU() * CLHEP::cm / Unit::cm,
                    segmentAlignmentData->getDeltaV() * CLHEP::cm / Unit::cm,
                    0) *
                  m_Strip[iSection - 1][iLayer - 1][iSector - 1][iPlane - 1]
                  [nStripsSegment * (iSegment - 1) + iStrip - 1] *
                  HepGeom::RotateZ3D(segmentAlignmentData->getDeltaGamma() *
                                     CLHEP::rad / Unit::rad);
              }
            }
          }
        }
      }
    }
  }
  if (global)
    transformsToGlobal();
}

EKLM::TransformData::~TransformData()
{
  int iSection, iLayer, iSector, iPlane;
  /* cppcheck-suppress variableScope */
  int nSections, nLayers, nDetectorLayers, nSectors, nPlanes;
  nSections = m_GeoDat->getNSections();
  nLayers = m_GeoDat->getNLayers();
  nSectors = m_GeoDat->getNSectors();
  nPlanes = m_GeoDat->getNPlanes();
  for (iSection = 0; iSection < nSections; iSection++) {
    nDetectorLayers = m_GeoDat->getNDetectorLayers(iSection + 1);
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      delete[] m_Sector[iSection][iLayer];
      if (iLayer >= nDetectorLayers)
        continue;
      for (iSector = 0; iSector < nSectors; iSector++) {
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          delete[] m_Segment[iSection][iLayer][iSector][iPlane];
          delete[] m_Strip[iSection][iLayer][iSector][iPlane];
          delete[] m_StripInverse[iSection][iLayer][iSector][iPlane];
        }
        delete[] m_Plane[iSection][iLayer][iSector];
        delete[] m_PlaneDisplacement[iSection][iLayer][iSector];
        delete[] m_Segment[iSection][iLayer][iSector];
        delete[] m_Strip[iSection][iLayer][iSector];
        delete[] m_StripInverse[iSection][iLayer][iSector];
      }
      delete[] m_Plane[iSection][iLayer];
      delete[] m_PlaneDisplacement[iSection][iLayer];
      delete[] m_Segment[iSection][iLayer];
      delete[] m_Strip[iSection][iLayer];
      delete[] m_StripInverse[iSection][iLayer];
    }
    delete[] m_Layer[iSection];
    delete[] m_Sector[iSection];
    delete[] m_PlaneDisplacement[iSection];
    delete[] m_Plane[iSection];
    delete[] m_Segment[iSection];
    delete[] m_Strip[iSection];
    delete[] m_StripInverse[iSection];
  }
  delete[] m_Section;
  delete[] m_Layer;
  delete[] m_Sector;
  delete[] m_PlaneDisplacement;
  delete[] m_Plane;
  delete[] m_Segment;
  delete[] m_Strip;
  delete[] m_StripInverse;
}

void EKLM::TransformData::transformsToGlobal()
{
  int iSection, iLayer, iSector, iPlane, iSegment, iStrip;
  /* cppcheck-suppress variableScope */
  int nSections, nLayers, nDetectorLayers, nSectors, nPlanes, nSegments, nStrips;
  nSections = m_GeoDat->getNSections();
  nLayers = m_GeoDat->getNLayers();
  nSectors = m_GeoDat->getNSectors();
  nPlanes = m_GeoDat->getNPlanes();
  nSegments = m_GeoDat->getNSegments();
  nStrips = m_GeoDat->getNStrips();
  for (iSection = 0; iSection < nSections; iSection++) {
    nDetectorLayers = m_GeoDat->getNDetectorLayers(iSection + 1);
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      m_Layer[iSection][iLayer] = m_Section[iSection] * m_Layer[iSection][iLayer];
      for (iSector = 0; iSector < nSectors; iSector++) {
        m_Sector[iSection][iLayer][iSector] =
          m_Layer[iSection][iLayer] * m_Sector[iSection][iLayer][iSector];
        if (iLayer >= nDetectorLayers)
          continue;
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          m_Plane[iSection][iLayer][iSector][iPlane] =
            m_Sector[iSection][iLayer][iSector] *
            m_Plane[iSection][iLayer][iSector][iPlane];
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            m_Segment[iSection][iLayer][iSector][iPlane][iSegment] =
              m_Plane[iSection][iLayer][iSector][iPlane] *
              m_PlaneDisplacement[iSection][iLayer][iSector][iPlane] *
              m_Segment[iSection][iLayer][iSector][iPlane][iSegment];
          }
          for (iStrip = 0; iStrip < nStrips; iStrip++) {
            m_Strip[iSection][iLayer][iSector][iPlane][iStrip] =
              m_Plane[iSection][iLayer][iSector][iPlane] *
              m_PlaneDisplacement[iSection][iLayer][iSector][iPlane] *
              m_Strip[iSection][iLayer][iSector][iPlane][iStrip];
            m_StripInverse[iSection][iLayer][iSector][iPlane][iStrip] =
              m_Strip[iSection][iLayer][iSector][iPlane][iStrip].inverse();
          }
        }
      }
    }
  }
}

const HepGeom::Transform3D*
EKLM::TransformData::getSectionTransform(int section) const
{
  return &m_Section[section - 1];
}

const HepGeom::Transform3D*
EKLM::TransformData::getLayerTransform(int section, int layer) const
{
  return &m_Layer[section - 1][layer - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getSectorTransform(int section, int layer, int sector) const
{
  return &m_Sector[section - 1][layer - 1][sector - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getPlaneTransform(int section, int layer, int sector, int plane) const
{
  return &m_Plane[section - 1][layer - 1][sector - 1][plane - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getPlaneDisplacement(int section, int layer, int sector, int plane) const
{
  return &m_PlaneDisplacement[section - 1][layer - 1][sector - 1][plane - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getSegmentTransform(int section, int layer, int sector, int plane,
                    int segment) const
{
  return &m_Segment[section - 1][layer - 1][sector - 1][plane - 1][segment - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getStripTransform(int section, int layer, int sector, int plane, int strip) const
{
  return &m_Strip[section - 1][layer - 1][sector - 1][plane - 1][strip - 1];
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripLocalToGlobal(KLMDigit* hit) const
{
  return &(m_Strip[hit->getSection() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripGlobalToLocal(KLMDigit* hit) const
{
  return &(m_StripInverse[hit->getSection() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripGlobalToLocal(int section, int layer, int sector,
                                           int plane, int strip) const
{
  return &(m_StripInverse[section - 1][layer - 1][sector - 1][plane - 1]
           [strip - 1]);
}

bool EKLM::TransformData::intersection(KLMDigit* hit1, KLMDigit* hit2,
                                       HepGeom::Point3D<double>* cross,
                                       double* d1, double* d2, double* sd,
                                       bool segments) const
{
  /* Hits must be from the same sector, */
  if (hit1->getSection() != hit2->getSection())
    return false;
  if (hit1->getLayer() != hit2->getLayer())
    return false;
  if (hit1->getSector() != hit2->getSector())
    return false;
  /* but different planes. */
  if (hit1->getPlane() == hit2->getPlane())
    return false;
  /* Coordinates of strip 1 ends. */
  double l1 = m_GeoDat->getStripLength(hit1->getStrip());
  HepGeom::Point3D<double> s1_1(-0.5 * l1, 0.0, 0.0);
  HepGeom::Point3D<double> s1_2(0.5 * l1, 0.0, 0.0);
  const HepGeom::Transform3D* tr1 = getStripLocalToGlobal(hit1);
  HepGeom::Point3D<double> s1_1g = (*tr1) * s1_1;
  HepGeom::Point3D<double> s1_2g = (*tr1) * s1_2;
  /* Coordinates of strip 2 ends. */
  double l2 = m_GeoDat->getStripLength(hit2->getStrip());
  HepGeom::Point3D<double> s2_1(-0.5 * l2, 0.0, 0.0);
  HepGeom::Point3D<double> s2_2(0.5 * l2, 0.0, 0.0);
  const HepGeom::Transform3D* tr2 = getStripLocalToGlobal(hit2);
  HepGeom::Point3D<double> s2_1g = (*tr2) * s2_1;
  HepGeom::Point3D<double> s2_2g = (*tr2) * s2_2;
  /**
   * Line parametric equations:
   * (s1_1g) + (s1_2g - s1_1g) * t1 = a1 + v1 * t1,
   * (s2_1g) + (s2_2g - s2_1g) * t2 = a2 + v2 * t2.
   * Points of closest approach:
   * t1 = ((v1,v2)*(d,v2) - v2^2*(d,v1)) / (v1^2*v2^2 - (v1,v2)^2)
   * t2 = - ((v1,v2)*(d,v1) - v1^2*(d,v2)) / (v1^2*v2^2 - (v1,v2)^2)
   * where d = a1 - a2.
   */
  HepGeom::Vector3D<double> v1 = s1_2g - s1_1g;
  HepGeom::Vector3D<double> v2 = s2_2g - s2_1g;
  HepGeom::Vector3D<double> d = s1_1g - s2_1g;
  double v1sq = v1.mag2();
  double v2sq = v2.mag2();
  double v1dv2 = v1.dot(v2);
  double ddv1 = d.dot(v1);
  double ddv2 = d.dot(v2);
  double den = v1sq * v2sq - v1dv2 * v1dv2;
  double t1 = (v1dv2 * ddv2 - v2sq * ddv1) / den;
  double t2 = (- v1dv2 * ddv1 + v1sq * ddv2) / den;
  /* Segments do not intersect. */
  if (segments) {
    if (t1 < 0.0 || t1 > 1.0)
      return false;
    if (t2 < 0.0 || t2 > 1.0)
      return false;
  }
  /* Segments intersect, set return values. */
  HepGeom::Point3D<double> s1_cg = s1_1g + v1 * t1;
  HepGeom::Point3D<double> s2_cg = s2_1g + v2 * t2;
  *d1 = s1_2g.distance(s1_cg) / CLHEP::mm * Unit::mm;
  *d2 = s2_2g.distance(s2_cg) / CLHEP::mm * Unit::mm;
  *cross = 0.5 * (s1_cg + s2_cg) / CLHEP::mm * Unit::mm;
  *sd = s1_cg.distance(s2_cg) / CLHEP::mm * Unit::mm;
  if (s2_cg.mag2() < s1_cg.mag2())
    *sd = - *sd;
  return true;
}

int EKLM::TransformData::getSectorByPosition(
  int section, const HepGeom::Point3D<double>& position) const
{
  int sector;
  double x;
  if (section == 1) {
    x = position.x();
  } else {
    x = -position.x();
  }
  if (position.y() > 0) {
    if (x > 0)
      sector = 1;
    else
      sector = 2;
  } else {
    if (x > 0)
      sector = 4;
    else
      sector = 3;
  }
  return sector;
}

int EKLM::TransformData::getStripsByIntersection(
  const HepGeom::Point3D<double>& intersection, int* strip1, int* strip2) const
{
  /* cppcheck-suppress variableScope */
  int section, layer, sector, plane, segment, strip, stripSegment, stripGlobal;
  /* cppcheck-suppress variableScope */
  int nLayers, nPlanes, nSegments, nStripsSegment, minDistanceSegment;
  double solenoidCenter, firstLayerCenter, layerShift;
  /* cppcheck-suppress variableScope */
  double x, y, z, l, minY, maxY;
  double minDistance = 0, minDistanceNew, stripWidth;
  HepGeom::Point3D<double> intersectionClhep, intersectionLocal;
  intersectionClhep = intersection * CLHEP::cm / Unit::cm;
  solenoidCenter = m_GeoDat->getSolenoidZ() / CLHEP::cm * Unit::cm;
  if (intersection.z() < solenoidCenter)
    section = 1;
  else
    section = 2;
  firstLayerCenter =
    (m_GeoDat->getSectionPosition()->getZ()
     - 0.5 * m_GeoDat->getSectionPosition()->getLength()
     + m_GeoDat->getLayerShiftZ()
     - 0.5 * m_GeoDat->getLayerPosition()->getLength()) /
    CLHEP::cm * Unit::cm;
  layerShift = m_GeoDat->getLayerShiftZ() / CLHEP::cm * Unit::cm;
  z = fabs(intersection.z() - solenoidCenter);
  layer = round((z - firstLayerCenter) / layerShift) + 1;
  if (layer <= 0)
    layer = 1;
  nLayers = m_GeoDat->getNDetectorLayers(section);
  if (layer > nLayers)
    layer = nLayers;
  sector = getSectorByPosition(section, intersection);
  nPlanes = m_GeoDat->getNPlanes();
  nSegments = m_GeoDat->getNSegments();
  nStripsSegment = m_ElementNumbers->getNStripsSegment();
  stripWidth = m_GeoDat->getStripGeometry()->getWidth() / CLHEP::cm * Unit::cm;
  minY = -stripWidth / 2;
  maxY = (double(nStripsSegment) - 0.5) * stripWidth;
  for (plane = 1; plane <= nPlanes; plane++) {
    minDistanceSegment = 1;
    for (segment = 1; segment <= nSegments; segment++) {
      strip = (segment - 1) * nStripsSegment;
      intersectionLocal = m_StripInverse[section - 1][layer - 1]
                          [sector - 1][plane - 1][strip] * intersectionClhep;
      y = intersectionLocal.y() / CLHEP::cm * Unit::cm;
      if (y < minY) {
        minDistanceNew = minY - y;
      } else if (y > maxY) {
        minDistanceNew = y - maxY;
      } else {
        minDistance = 0;
        minDistanceSegment = segment;
        break;
      }
      if (segment == 1) {
        minDistance = minDistanceNew;
      } else if (minDistanceNew < minDistance) {
        minDistance = minDistanceNew;
        minDistanceSegment = segment;
      }
    }
    /*
     * The intersection is required to be strictly within a segment,
     * this condition might be adjusted later.
     */
    if (minDistance > 0)
      return -1;
    strip = (minDistanceSegment - 1) * nStripsSegment;
    intersectionLocal = m_StripInverse[section - 1][layer - 1]
                        [sector - 1][plane - 1][strip] * intersectionClhep;
    y = intersectionLocal.y() / CLHEP::cm * Unit::cm;
    stripSegment = ceil((y - 0.5 * stripWidth) / stripWidth);
    if (stripSegment <= 0)
      stripSegment = 1;
    else if (stripSegment > nStripsSegment)
      stripSegment = nStripsSegment;
    strip = stripSegment + (minDistanceSegment - 1) * nStripsSegment;
    intersectionLocal = m_StripInverse[section - 1][layer - 1]
                        [sector - 1][plane - 1][strip - 1] * intersectionClhep;
    x = intersectionLocal.x();
    l = m_GeoDat->getStripLength(strip);
    /*
     * The intersection is required to be strictly within the strip length,
     * this condition might be adjusted later.
     */
    if (fabs(x) > 0.5 * l)
      return -1;
    stripGlobal = m_ElementNumbers->stripNumber(
                    section, layer, sector, plane, strip);
    if (plane == 1)
      *strip1 = stripGlobal;
    else
      *strip2 = stripGlobal;
  }
  return 0;
}
