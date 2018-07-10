/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/TransformData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

EKLM::TransformData::TransformData(bool global, Displacement displacementType)
{
  int iEndcap, iLayer, iSector, iPlane, iSegment, iStrip, sector, segment;
  int nEndcaps, nLayers, nSectors, nPlanes, nStrips, nSegments, nStripsSegment;
  int nDetectorLayers;
  std::string payload;
  const EKLMAlignmentData* sectorAlignment, *segmentAlignment;
  AlignmentChecker alignmentChecker(true);
  m_GeoDat = &(GeometryData::Instance());
  nEndcaps = m_GeoDat->getNEndcaps();
  nSectors = m_GeoDat->getNSectors();
  nLayers = m_GeoDat->getNLayers();
  nPlanes = m_GeoDat->getNPlanes();
  nStrips = m_GeoDat->getNStrips();
  nSegments = m_GeoDat->getNSegments();
  nStripsSegment = m_GeoDat->getNStripsSegment();
  m_Endcap = new HepGeom::Transform3D[nEndcaps];
  m_Layer = new HepGeom::Transform3D*[nEndcaps];
  m_Sector = new HepGeom::Transform3D** [nEndcaps];
  m_Plane = new HepGeom::Transform3D** *[nEndcaps];
  m_PlaneDisplacement = new HepGeom::Transform3D** *[nEndcaps];
  m_Segment = new HepGeom::Transform3D**** [nEndcaps];
  m_Strip = new HepGeom::Transform3D**** [nEndcaps];
  m_StripInverse = new HepGeom::Transform3D**** [nEndcaps];
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    m_GeoDat->getEndcapTransform(&m_Endcap[iEndcap], iEndcap);
    nDetectorLayers = m_GeoDat->getNDetectorLayers(iEndcap + 1);
    m_Layer[iEndcap] = new HepGeom::Transform3D[nLayers];
    m_Sector[iEndcap] = new HepGeom::Transform3D*[nLayers];
    m_Plane[iEndcap] = new HepGeom::Transform3D** [nLayers];
    m_PlaneDisplacement[iEndcap] = new HepGeom::Transform3D** [nLayers];
    m_Segment[iEndcap] = new HepGeom::Transform3D** *[nLayers];
    m_Strip[iEndcap] = new HepGeom::Transform3D** *[nLayers];
    m_StripInverse[iEndcap] = new HepGeom::Transform3D** *[nLayers];
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      m_GeoDat->getLayerTransform(&m_Layer[iEndcap][iLayer], iLayer);
      m_Sector[iEndcap][iLayer] = new HepGeom::Transform3D[nSectors];
      if (iLayer < nDetectorLayers) {
        m_Plane[iEndcap][iLayer] = new HepGeom::Transform3D*[nSectors];
        m_PlaneDisplacement[iEndcap][iLayer] =
          new HepGeom::Transform3D*[nSectors];
        m_Segment[iEndcap][iLayer] = new HepGeom::Transform3D** [nSectors];
        m_Strip[iEndcap][iLayer] = new HepGeom::Transform3D** [nSectors];
        m_StripInverse[iEndcap][iLayer] = new HepGeom::Transform3D** [nSectors];
      }
      for (iSector = 0; iSector < nSectors; iSector++) {
        m_GeoDat->getSectorTransform(&m_Sector[iEndcap][iLayer][iSector],
                                     iSector);
        if (iLayer >= nDetectorLayers)
          continue;
        m_Plane[iEndcap][iLayer][iSector] = new HepGeom::Transform3D[nPlanes];
        m_PlaneDisplacement[iEndcap][iLayer][iSector] =
          new HepGeom::Transform3D[nPlanes];
        m_Segment[iEndcap][iLayer][iSector] =
          new HepGeom::Transform3D*[nPlanes];
        m_Strip[iEndcap][iLayer][iSector] = new HepGeom::Transform3D*[nPlanes];
        m_StripInverse[iEndcap][iLayer][iSector] =
          new HepGeom::Transform3D*[nPlanes];
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          m_GeoDat->getPlaneTransform(
            &m_Plane[iEndcap][iLayer][iSector][iPlane], iPlane);
          m_PlaneDisplacement[iEndcap][iLayer][iSector][iPlane] =
            HepGeom::Translate3D(0, 0, 0);
          m_Segment[iEndcap][iLayer][iSector][iPlane] =
            new HepGeom::Transform3D[nSegments];
          for (iSegment = 0; iSegment < nSegments; iSegment++) {
            m_Segment[iEndcap][iLayer][iSector][iPlane][iSegment] =
              HepGeom::Translate3D(0, 0, 0);
          }
          m_Strip[iEndcap][iLayer][iSector][iPlane] =
            new HepGeom::Transform3D[nStrips];
          m_StripInverse[iEndcap][iLayer][iSector][iPlane] =
            new HepGeom::Transform3D[nStrips];
          for (iStrip = 0; iStrip < nStrips; iStrip++) {
            m_GeoDat->getStripTransform(
              &m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip], iStrip);
          }
        }
      }
    }
  }
  /* Read alignment data from the database and modify transformations. */
  if (displacementType != c_None) {
    if (displacementType == c_Displacement)
      payload = "EKLMDisplacement";
    else
      payload = "EKLMAlignment";
    DBObjPtr<EKLMAlignment> alignment(payload);
    if (!alignment.isValid())
      B2FATAL("No EKLM displacement (alignment) data.");
    if (displacementType == c_Displacement) {
      if (!alignmentChecker.checkAlignment(&(*alignment)))
        B2FATAL("EKLM displacement data are incorrect, overlaps exist.");
    }
    for (iEndcap = 1; iEndcap <= nEndcaps; iEndcap++) {
      nDetectorLayers = m_GeoDat->getNDetectorLayers(iEndcap);
      for (iLayer = 1; iLayer <= nDetectorLayers; iLayer++) {
        for (iSector = 1; iSector <= nSectors; iSector++) {
          sector = m_GeoDat->sectorNumber(iEndcap, iLayer, iSector);
          sectorAlignment = alignment->getSectorAlignment(sector);
          if (sectorAlignment == NULL)
            B2FATAL("Incomplete EKLM displacement (alignment) data.");
          for (iPlane = 1; iPlane <= nPlanes; iPlane++) {
            /* First plane is rotated. */
            if (iPlane == 1) {
              m_PlaneDisplacement[iEndcap - 1][iLayer - 1][iSector - 1][iPlane - 1] =
                HepGeom::Translate3D(
                  sectorAlignment->getDy() * CLHEP::cm / Unit::cm,
                  sectorAlignment->getDx() * CLHEP::cm / Unit::cm, 0) *
                HepGeom::RotateZ3D(-sectorAlignment->getDalpha() *
                                   CLHEP::rad / Unit::rad);
            } else {
              m_PlaneDisplacement[iEndcap - 1][iLayer - 1][iSector - 1][iPlane - 1] =
                HepGeom::Translate3D(
                  sectorAlignment->getDx() * CLHEP::cm / Unit::cm,
                  sectorAlignment->getDy() * CLHEP::cm / Unit::cm, 0) *
                HepGeom::RotateZ3D(sectorAlignment->getDalpha() *
                                   CLHEP::rad / Unit::rad);
            }
            for (iSegment = 1; iSegment <= nSegments; iSegment++) {
              segment = m_GeoDat->segmentNumber(iEndcap, iLayer, iSector,
                                                iPlane, iSegment);
              segmentAlignment = alignment->getSegmentAlignment(segment);
              if (segmentAlignment == NULL)
                B2FATAL("Incomplete EKLM displacement (alignment) data.");
              m_Segment[iEndcap - 1][iLayer - 1][iSector - 1][iPlane - 1]
              [iSegment - 1] =
                HepGeom::Translate3D(
                  segmentAlignment->getDx() * CLHEP::cm / Unit::cm,
                  segmentAlignment->getDy() * CLHEP::cm / Unit::cm, 0) *
                m_Segment[iEndcap - 1][iLayer - 1][iSector - 1][iPlane - 1]
                [iSegment - 1] *
                HepGeom::RotateZ3D(segmentAlignment->getDalpha() *
                                   CLHEP::rad / Unit::rad);
              for (iStrip = 1; iStrip <= nStripsSegment; iStrip++) {
                m_Strip[iEndcap - 1][iLayer - 1][iSector - 1][iPlane - 1]
                [nStripsSegment * (iSegment - 1) + iStrip - 1] =
                  HepGeom::Translate3D(
                    segmentAlignment->getDx() * CLHEP::cm / Unit::cm,
                    segmentAlignment->getDy() * CLHEP::cm / Unit::cm, 0) *
                  m_Strip[iEndcap - 1][iLayer - 1][iSector - 1][iPlane - 1]
                  [nStripsSegment * (iSegment - 1) + iStrip - 1] *
                  HepGeom::RotateZ3D(segmentAlignment->getDalpha() *
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
  int iEndcap, iLayer, iSector, iPlane;
  int nEndcaps, nLayers, nDetectorLayers, nSectors, nPlanes;
  nEndcaps = m_GeoDat->getNEndcaps();
  nLayers = m_GeoDat->getNLayers();
  nSectors = m_GeoDat->getNSectors();
  nPlanes = m_GeoDat->getNPlanes();
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    nDetectorLayers = m_GeoDat->getNDetectorLayers(iEndcap + 1);
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      delete[] m_Sector[iEndcap][iLayer];
      if (iLayer >= nDetectorLayers)
        continue;
      for (iSector = 0; iSector < nSectors; iSector++) {
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          delete[] m_Segment[iEndcap][iLayer][iSector][iPlane];
          delete[] m_Strip[iEndcap][iLayer][iSector][iPlane];
          delete[] m_StripInverse[iEndcap][iLayer][iSector][iPlane];
        }
        delete[] m_Plane[iEndcap][iLayer][iSector];
        delete[] m_PlaneDisplacement[iEndcap][iLayer][iSector];
        delete[] m_Segment[iEndcap][iLayer][iSector];
        delete[] m_Strip[iEndcap][iLayer][iSector];
        delete[] m_StripInverse[iEndcap][iLayer][iSector];
      }
      delete[] m_Plane[iEndcap][iLayer];
      delete[] m_PlaneDisplacement[iEndcap][iLayer];
      delete[] m_Segment[iEndcap][iLayer];
      delete[] m_Strip[iEndcap][iLayer];
      delete[] m_StripInverse[iEndcap][iLayer];
    }
    delete[] m_Layer[iEndcap];
    delete[] m_Sector[iEndcap];
    delete[] m_PlaneDisplacement[iEndcap];
    delete[] m_Plane[iEndcap];
    delete[] m_Segment[iEndcap];
    delete[] m_Strip[iEndcap];
    delete[] m_StripInverse[iEndcap];
  }
  delete[] m_Endcap;
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
  int iEndcap, iLayer, iSector, iPlane, iStrip;
  int nEndcaps, nLayers, nDetectorLayers, nSectors, nPlanes, nStrips;
  nEndcaps = m_GeoDat->getNEndcaps();
  nLayers = m_GeoDat->getNLayers();
  nSectors = m_GeoDat->getNSectors();
  nPlanes = m_GeoDat->getNPlanes();
  nStrips = m_GeoDat->getNStrips();
  for (iEndcap = 0; iEndcap < nEndcaps; iEndcap++) {
    nDetectorLayers = m_GeoDat->getNDetectorLayers(iEndcap + 1);
    for (iLayer = 0; iLayer < nLayers; iLayer++) {
      m_Layer[iEndcap][iLayer] = m_Endcap[iEndcap] * m_Layer[iEndcap][iLayer];
      for (iSector = 0; iSector < nSectors; iSector++) {
        m_Sector[iEndcap][iLayer][iSector] =
          m_Layer[iEndcap][iLayer] * m_Sector[iEndcap][iLayer][iSector];
        if (iLayer >= nDetectorLayers)
          continue;
        for (iPlane = 0; iPlane < nPlanes; iPlane++) {
          m_Plane[iEndcap][iLayer][iSector][iPlane] =
            m_Sector[iEndcap][iLayer][iSector] *
            m_Plane[iEndcap][iLayer][iSector][iPlane];
          for (iStrip = 0; iStrip < nStrips; iStrip++) {
            m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip] =
              m_Plane[iEndcap][iLayer][iSector][iPlane] *
              m_PlaneDisplacement[iEndcap][iLayer][iSector][iPlane] *
              m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip];
            m_StripInverse[iEndcap][iLayer][iSector][iPlane][iStrip] =
              m_Strip[iEndcap][iLayer][iSector][iPlane][iStrip].inverse();
          }
        }
      }
    }
  }
}

const HepGeom::Transform3D*
EKLM::TransformData::getEndcapTransform(int endcap) const
{
  return &m_Endcap[endcap - 1];
}

const HepGeom::Transform3D*
EKLM::TransformData::getLayerTransform(int endcap, int layer) const
{
  return &m_Layer[endcap - 1][layer - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getSectorTransform(int endcap, int layer, int sector) const
{
  return &m_Sector[endcap - 1][layer - 1][sector - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getPlaneTransform(int endcap, int layer, int sector, int plane) const
{
  return &m_Plane[endcap - 1][layer - 1][sector - 1][plane - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getPlaneDisplacement(int endcap, int layer, int sector, int plane) const
{
  return &m_PlaneDisplacement[endcap - 1][layer - 1][sector - 1][plane - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getSegmentTransform(int endcap, int layer, int sector, int plane,
                    int segment) const
{
  return &m_Segment[endcap - 1][layer - 1][sector - 1][plane - 1][segment - 1];
}

const HepGeom::Transform3D* EKLM::TransformData::
getStripTransform(int endcap, int layer, int sector, int plane, int strip) const
{
  return &m_Strip[endcap - 1][layer - 1][sector - 1][plane - 1][strip - 1];
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripLocalToGlobal(EKLMDigit* hit) const
{
  return &(m_Strip[hit->getEndcap() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripGlobalToLocal(EKLMDigit* hit) const
{
  return &(m_StripInverse[hit->getEndcap() - 1][hit->getLayer() - 1]
           [hit->getSector() - 1][hit->getPlane() - 1][hit->getStrip() - 1]);
}

const HepGeom::Transform3D*
EKLM::TransformData::getStripGlobalToLocal(int endcap, int layer, int sector,
                                           int plane, int strip) const
{
  return &(m_StripInverse[endcap - 1][layer - 1][sector - 1][plane - 1]
           [strip - 1]);
}

bool EKLM::TransformData::intersection(EKLMDigit* hit1, EKLMDigit* hit2,
                                       HepGeom::Point3D<double>* cross,
                                       double* d1, double* d2, double* sd,
                                       bool segments)
{
  /* Hits must be from the same sector, */
  if (hit1->getEndcap() != hit2->getEndcap())
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

