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
#include <eklm/geometry/GeometryData.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

int EKLM::GeometryData::save(const char* file)
{
  int res;
  FILE* f;
  /* Create file. */
  f = fopen(file, "w");
  if (f == NULL)
    return -1;
  /* Fill transformation data. */
  EKLM::fillTransforms(&transf);
  /* Write and close file. */
  res = writeTransforms(f, &transf);
  if (res != 0)
    return res;
  fclose(f);
  return 0;
}

int EKLM::GeometryData::read(const char* file)
{
  int i;
  int n;
  int res;
  char str[32];
  FILE* f;
  f = fopen(file, "r");
  if (f == NULL)
    return -1;
  res = readTransforms(f, &transf);
  if (res != 0)
    return res;
  fclose(f);
  /* Fill strip data. */
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
             "Layer/Sector/Plane/Strips");
  n = gd.getNumberNodes("Strip");
  if (n != 75)
    B2FATAL("Unexpected number of strips in EKLM geometry XML data!");
  for (i = 0; i < 75; i++) {
    GearDir gds(gd);
    snprintf(str, 32, "/Strip[%d]", i + 1);
    gds.append(str);
    m_stripLen[i] = gds.getLength("Length");
  }
  return 0;
}

double EKLM::GeometryData::getStripLength(int strip)
{
  return m_stripLen[strip - 1];
}

bool EKLM::GeometryData::intersection(EKLMDigit* hit1, EKLMDigit* hit2,
                                      HepGeom::Point3D<double> *cross,
                                      double* d1, double* d2)
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
  double l1 = getStripLength(hit1->getStrip()) * 10.0; /* mm */
  HepGeom::Point3D<double> s1_1(-0.5 * l1, 0.0, 0.0);
  HepGeom::Point3D<double> s1_2(0.5 * l1, 0.0, 0.0);
  HepGeom::Transform3D* tr1 = getStripTransform(&transf, hit1);
  HepGeom::Point3D<double> s1_1g = (*tr1) * s1_1;
  HepGeom::Point3D<double> s1_2g = (*tr1) * s1_2;
  /* Coordinates of strip 2 ends. */
  double l2 = getStripLength(hit2->getStrip()) * 10.0; /* mm */
  HepGeom::Point3D<double> s2_1(-0.5 * l2, 0.0, 0.0);
  HepGeom::Point3D<double> s2_2(0.5 * l2, 0.0, 0.0);
  HepGeom::Transform3D* tr2 = getStripTransform(&transf, hit2);
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
  return true;
}

