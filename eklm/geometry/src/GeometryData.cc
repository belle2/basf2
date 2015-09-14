/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/GeometryData2.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

static bool compareLength(double a, double b)
{
  return a < b;
}

EKLM::GeometryData::GeometryData()
{
  m_nStrip = -1;
  m_nStripDifferent = -1;
  m_StripLen = NULL;
  m_StripLenToAll = NULL;
  m_StripAllToLen = NULL;
  m_MinZForward = 0;
  m_MaxZBackward = 0;
}

EKLM::GeometryData::~GeometryData()
{
  if (m_StripLen != NULL)
    free(m_StripLen);
  if (m_StripLenToAll != NULL)
    free(m_StripLenToAll);
  if (m_StripAllToLen != NULL)
    free(m_StripAllToLen);
}

void EKLM::GeometryData::read()
{
  const char err[] = "Strip sorting algorithm error.";
  int i, iEndcap, iLayer, iSector, iPlane, iSegment, segment;
  char str[32];
  double l, solenoidZ, endcapZ, endcapLength;
  std::vector<double> strips;
  std::vector<double>::iterator it;
  std::map<double, int> mapLengthStrip;
  std::map<double, int> mapLengthStrip2;
  std::map<double, int>::iterator itm;
  EKLMAlignmentData* alignmentData;
  EKLM::fillTransforms(&transf);
  /* Read alignment data from the database and modify transformations. */
  DBObjPtr<EKLMAlignment> alignment("EKLMAlignment");
  if (alignment.isValid()) {
    for (iEndcap = 1; iEndcap <= 2; iEndcap++) {
      for (iLayer = 1; iLayer <= EKLM::GeometryData2::Instance().
           getNDetectorLayers(iEndcap); iLayer++) {
        for (iSector = 1; iSector <= 4; iSector++) {
          for (iPlane = 1; iPlane <= 2; iPlane++) {
            for (iSegment = 1; iSegment <= 5; iSegment++) {
              segment = EKLM::segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                            iSegment);
              alignmentData = alignment->getAlignmentData(segment);
              if (alignmentData == NULL)
                B2FATAL("Incomplete alignment data in the database.");
            }
          }
        }
      }
    }
  } else
    B2INFO("Could not read alignment data from the database, "
           "using default positions.");
  /* Read position data. */
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
  solenoidZ = gd.getLength("SolenoidZ");
  gd.append("/Endcap");
  endcapZ = gd.getLength("PositionZ");
  endcapLength = gd.getLength("Length");
  m_MinZForward = solenoidZ + endcapZ - 0.5 * endcapLength;
  m_MaxZBackward = solenoidZ - endcapZ + 0.5 * endcapLength;
  /* Fill strip data. */
  GearDir gd2("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
              "Layer/Sector/Plane/Strips");
  m_nStrip = gd2.getNumberNodes("Strip");
  m_StripLen = (double*)malloc(m_nStrip * sizeof(double));
  if (m_StripLen == NULL)
    B2FATAL(MemErr);
  for (i = 0; i < m_nStrip; i++) {
    GearDir gds(gd2);
    snprintf(str, 32, "/Strip[%d]", i + 1);
    gds.append(str);
    m_StripLen[i] = gds.getLength("Length");
    strips.push_back(m_StripLen[i]);
    mapLengthStrip.insert(std::pair<double, int>(m_StripLen[i], i));
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
    B2FATAL(MemErr);
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
    B2FATAL(MemErr);
  for (i = 0; i < m_nStrip; i++) {
    itm = mapLengthStrip2.find(m_StripLen[i]);
    if (itm == mapLengthStrip2.end())
      B2FATAL(err);
    m_StripAllToLen[i] = itm->second;
  }
}

double EKLM::GeometryData::getStripLength(int strip)
{
  return m_StripLen[strip - 1];
}

int EKLM::GeometryData::getStripLengthIndex(int positionIndex)
{
  return m_StripAllToLen[positionIndex];
}

int EKLM::GeometryData::getStripPositionIndex(int lengthIndex)
{
  return m_StripLenToAll[lengthIndex];
}

int EKLM::GeometryData::getNStripsDifferentLength()
{
  return m_nStripDifferent;
}

bool EKLM::GeometryData::intersection(EKLMDigit* hit1, EKLMDigit* hit2,
                                      HepGeom::Point3D<double>* cross,
                                      double* d1, double* d2, double* sd)
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
  double l1 = getStripLength(hit1->getStrip()) / Unit::mm * CLHEP::mm;
  HepGeom::Point3D<double> s1_1(-0.5 * l1, 0.0, 0.0);
  HepGeom::Point3D<double> s1_2(0.5 * l1, 0.0, 0.0);
  HepGeom::Transform3D* tr1 = getStripLocalToGlobal(&transf, hit1);
  HepGeom::Point3D<double> s1_1g = (*tr1) * s1_1;
  HepGeom::Point3D<double> s1_2g = (*tr1) * s1_2;
  /* Coordinates of strip 2 ends. */
  double l2 = getStripLength(hit2->getStrip()) / Unit::mm * CLHEP::mm;
  HepGeom::Point3D<double> s2_1(-0.5 * l2, 0.0, 0.0);
  HepGeom::Point3D<double> s2_2(0.5 * l2, 0.0, 0.0);
  HepGeom::Transform3D* tr2 = getStripLocalToGlobal(&transf, hit2);
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
  if (t1 < 0.0 || t1 > 1.0)
    return false;
  if (t2 < 0.0 || t2 > 1.0)
    return false;
  /* Segments intersect, set return values. */
  HepGeom::Point3D<double> s1_cg = s1_1g + v1 * t1;
  HepGeom::Point3D<double> s2_cg = s2_1g + v2 * t2;
  *d1 = s1_2g.distance(s1_cg) * Unit::mm;
  *d2 = s2_2g.distance(s2_cg) * Unit::mm;
  *cross = 0.5 * (s1_cg + s2_cg) * Unit::mm;
  *sd = s1_cg.distance(s2_cg) * Unit::mm;
  if (s2_cg.mag2() < s1_cg.mag2())
    *sd = - *sd;
  return true;
}

bool EKLM::GeometryData::hitInEKLM(double z)
{
  return (z > m_MinZForward) || (z < m_MaxZBackward);
}

