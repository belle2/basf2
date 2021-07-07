/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/eklm/geometry/GeometryData.h>

/* KLM headers. */
#include <klm/eklm/geometry/Circle2D.h>
#include <klm/eklm/geometry/Line2D.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/database/Database.h>
#include <framework/logging/Logger.h>

/* CLHEP headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Units/SystemOfUnits.h>

/* C++ headers. */
#include <string>

using namespace Belle2;

static const char c_MemErr[] = "Memory allocation error.";

const EKLM::GeometryData&
EKLM::GeometryData::Instance(enum DataSource dataSource, const GearDir* gearDir)
{
  static EKLM::GeometryData gd(dataSource, gearDir);
  return gd;
}

/**
 * Read position data.
 * @param epos Position data.
 * @param gd   XML data directory.
 */
static void readPositionData(EKLMGeometry::ElementPosition* epos, GearDir* gd)
{
  epos->setX(gd->getLength("X") * CLHEP::cm);
  epos->setY(gd->getLength("Y") * CLHEP::cm);
  epos->setZ(gd->getLength("Z") * CLHEP::cm);
}

/**
 * Read size data.
 * @param epos Position data.
 * @param gd   XML data directory.
 */
static void readSizeData(EKLMGeometry::ElementPosition* epos, GearDir* gd)
{
  epos->setInnerR(gd->getLength("InnerR") * CLHEP::cm);
  epos->setOuterR(gd->getLength("OuterR") * CLHEP::cm);
  epos->setLength(gd->getLength("Length") * CLHEP::cm);
}

/**
 * Read sector support geometry data.
 * @param ssg Sector support geometry data.
 * @param gd  XML data directory.
 */
static void readSectorSupportGeometry(
  EKLMGeometry::SectorSupportGeometry* ssg, GearDir* gd)
{
  ssg->setThickness(gd->getLength("Thickness") * CLHEP::cm);
  ssg->setDeltaLY(gd->getLength("DeltaLY") * CLHEP::cm);
  ssg->setCornerX(gd->getLength("CornerX") * CLHEP::cm);
  ssg->setCorner1LX(gd->getLength("Corner1LX") * CLHEP::cm);
  ssg->setCorner1Width(gd->getLength("Corner1Width") * CLHEP::cm);
  ssg->setCorner1Thickness(gd->getLength("Corner1Thickness") * CLHEP::cm);
  ssg->setCorner1Z(gd->getLength("Corner1Z") * CLHEP::cm);
  ssg->setCorner2LX(gd->getLength("Corner2LX") * CLHEP::cm);
  ssg->setCorner2LY(gd->getLength("Corner2LY") * CLHEP::cm);
  ssg->setCorner2Thickness(gd->getLength("Corner2Thickness") * CLHEP::cm);
  ssg->setCorner2Z(gd->getLength("Corner2Z") * CLHEP::cm);
  ssg->setCorner3LX(gd->getLength("Corner3LX") * CLHEP::cm);
  ssg->setCorner3LY(gd->getLength("Corner3LY") * CLHEP::cm);
  ssg->setCorner3Thickness(gd->getLength("Corner3Thickness") * CLHEP::cm);
  ssg->setCorner3Z(gd->getLength("Corner3Z") * CLHEP::cm);
  ssg->setCorner4LX(gd->getLength("Corner4LX") * CLHEP::cm);
  ssg->setCorner4LY(gd->getLength("Corner4LY") * CLHEP::cm);
  ssg->setCorner4Thickness(gd->getLength("Corner4Thickness") * CLHEP::cm);
  ssg->setCorner4Z(gd->getLength("Corner4Z") * CLHEP::cm);
}

/**
 * Read shield layer detail geometry data.
 * @param sdg Shield layer detail geometry data.
 * @param gd  XML data directory.
 */
static void readShieldDetailGeometry(
  EKLMGeometry::ShieldDetailGeometry* sdg, GearDir* gd)
{
  int i, n;
  std::string name;
  EKLMGeometry::Point p;
  sdg->setLengthX(gd->getLength("LengthX") * CLHEP::cm);
  sdg->setLengthY(gd->getLength("LengthY") * CLHEP::cm);
  n = gd->getNumberNodes("Point");
  sdg->setNPoints(n);
  for (i = 0; i < n; i++) {
    GearDir point(*gd);
    name = "/Point[" + std::to_string(i + 1) + "]";
    point.append(name.c_str());
    p.setX(point.getLength("X") * CLHEP::cm);
    p.setY(point.getLength("Y") * CLHEP::cm);
    sdg->setPoint(i, p);
  }
}

void EKLM::GeometryData::calculateSectorSupportGeometry()
{
  HepGeom::Point3D<double> p;
  Line2D line23Outer(0, m_SectorSupportPosition.getY(), 1, 0);
  Line2D line23Inner(0, m_SectorSupportPosition.getY() +
                     m_SectorSupportGeometry.getThickness(), 1, 0);
  Line2D line23Prism(0, m_SectorSupportPosition.getY() +
                     m_SectorSupportGeometry.getThickness() +
                     m_SectorSupportGeometry.getCorner3LY(), 1, 0);
  Line2D line41Outer(m_SectorSupportPosition.getX(), 0, 0, 1);
  Line2D line41Inner(m_SectorSupportPosition.getX() +
                     m_SectorSupportGeometry.getThickness(), 0, 0, 1);
  Line2D line41Prism(m_SectorSupportPosition.getX() +
                     m_SectorSupportGeometry.getThickness() +
                     m_SectorSupportGeometry.getCorner4LX(), 0, 0, 1);
  Line2D line41Corner1B(m_SectorSupportPosition.getX() +
                        m_SectorSupportGeometry.getCornerX(), 0, 0, 1);
  Circle2D circleInnerOuter(0, 0, m_SectorSupportPosition.getInnerR());
  Circle2D circleInnerInner(0, 0, m_SectorSupportPosition.getInnerR() +
                            m_SectorSupportGeometry.getThickness());
  Circle2D circleOuterInner(0, 0, m_SectorSupportPosition.getOuterR() -
                            m_SectorSupportGeometry.getThickness());
  Circle2D circleOuterOuter(0, 0, m_SectorSupportPosition.getOuterR());
  HepGeom::Point3D<double> intersections[2];
  /* Corner 1. */
  p.setX(m_SectorSupportPosition.getX());
  p.setY(m_SectorSupportPosition.getOuterR() -
         m_SectorSupportGeometry.getDeltaLY());
  p.setZ(0);
  m_SectorSupportGeometry.setCorner1A(p);
  line41Corner1B.findIntersection(circleOuterOuter, intersections);
  m_SectorSupportGeometry.setCorner1B(intersections[1]);
  m_SectorSupportGeometry.setCornerAngle(
    atan2(m_SectorSupportGeometry.getCorner1B().y() -
          m_SectorSupportGeometry.getCorner1A().y(),
          m_SectorSupportGeometry.getCorner1B().x() -
          m_SectorSupportGeometry.getCorner1A().x()) * CLHEP::rad);
  p.setX(m_SectorSupportPosition.getX() +
         m_SectorSupportGeometry.getThickness());
  p.setY(m_SectorSupportGeometry.getCorner1A().y() -
         m_SectorSupportGeometry.getThickness() *
         (1.0 / cos(m_SectorSupportGeometry.getCornerAngle()) -
          tan(m_SectorSupportGeometry.getCornerAngle())));
  p.setZ(0);
  m_SectorSupportGeometry.setCorner1AInner(p);
  Line2D lineCorner1(m_SectorSupportGeometry.getCorner1AInner().x(),
                     m_SectorSupportGeometry.getCorner1AInner().y(),
                     m_SectorSupportGeometry.getCorner1B().x() -
                     m_SectorSupportGeometry.getCorner1A().x(),
                     m_SectorSupportGeometry.getCorner1B().y() -
                     m_SectorSupportGeometry.getCorner1A().y());
  lineCorner1.findIntersection(circleOuterInner, intersections);
  m_SectorSupportGeometry.setCorner1BInner(intersections[1]);
  /* Corner 2. */
  line23Inner.findIntersection(circleOuterInner, intersections);
  m_SectorSupportGeometry.setCorner2Inner(intersections[1]);
  /* Corner 3. */
  line23Outer.findIntersection(circleInnerOuter, intersections);
  m_SectorSupportGeometry.setCorner3(intersections[1]);
  line23Inner.findIntersection(circleInnerInner, intersections);
  m_SectorSupportGeometry.setCorner3Inner(intersections[1]);
  line23Prism.findIntersection(circleInnerInner, intersections);
  p.setX(intersections[1].x());
  p.setY(m_SectorSupportPosition.getY() +
         m_SectorSupportGeometry.getThickness());
  p.setZ(0);
  m_SectorSupportGeometry.setCorner3Prism(p);
  /* Corner 4. */
  line41Outer.findIntersection(circleInnerOuter, intersections);
  m_SectorSupportGeometry.setCorner4(intersections[1]);
  line41Inner.findIntersection(circleInnerInner, intersections);
  m_SectorSupportGeometry.setCorner4Inner(intersections[1]);
  line41Prism.findIntersection(circleInnerInner, intersections);
  p.setX(m_SectorSupportPosition.getX() +
         m_SectorSupportGeometry.getThickness());
  p.setY(intersections[1].y());
  p.setZ(0);
  m_SectorSupportGeometry.setCorner4Prism(p);
}

static bool compareLength(double a, double b)
{
  return a < b;
}

void EKLM::GeometryData::fillStripIndexArrays()
{
  const char err[] = "Strip sorting algorithm error.";
  int i;
  double l;
  std::vector<double> strips;
  std::vector<double>::iterator it;
  std::map<double, int> mapLengthStrip;
  std::map<double, int> mapLengthStrip2;
  std::map<double, int>::iterator itm;
  for (i = 0; i < m_NStrips; i++) {
    strips.push_back(m_StripPosition[i].getLength());
    mapLengthStrip.insert(
      std::pair<double, int>(m_StripPosition[i].getLength(), i));
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
  if (m_StripLenToAll == nullptr)
    B2FATAL(c_MemErr);
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
  m_StripAllToLen = (int*)malloc(m_NStrips * sizeof(int));
  if (m_StripAllToLen == nullptr)
    B2FATAL(c_MemErr);
  for (i = 0; i < m_NStrips; i++) {
    itm = mapLengthStrip2.find(m_StripPosition[i].getLength());
    if (itm == mapLengthStrip2.end())
      B2FATAL(err);
    m_StripAllToLen[i] = itm->second;
  }
}

void EKLM::GeometryData::readXMLDataStrips(const GearDir& gd)
{
  int i;
  std::string name;
  GearDir Strips(gd);
  Strips.append("/Strip");
  m_StripGeometry.setWidth(Strips.getLength("Width") * CLHEP::cm);
  m_StripGeometry.setThickness(Strips.getLength("Thickness") * CLHEP::cm);
  m_StripGeometry.setGrooveDepth(Strips.getLength("GrooveDepth") * CLHEP::cm);
  m_StripGeometry.setGrooveWidth(Strips.getLength("GrooveWidth") * CLHEP::cm);
  m_StripGeometry.setNoScintillationThickness(
    Strips.getLength("NoScintillationThickness") * CLHEP::cm);
  m_StripGeometry.setRSSSize(Strips.getLength("RSSSize") * CLHEP::cm);
  try {
    m_StripPosition = new EKLMGeometry::ElementPosition[m_NStrips];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (i = 0; i < m_NStrips; i++) {
    GearDir StripContent(Strips);
    name = "/Strip[" + std::to_string(i + 1) + "]";
    StripContent.append(name.c_str());
    m_StripPosition[i].setLength(StripContent.getLength("Length") * CLHEP::cm);
    m_StripPosition[i].setX(StripContent.getLength("X") * CLHEP::cm);
    m_StripPosition[i].setY(StripContent.getLength("Y") * CLHEP::cm);
    m_StripPosition[i].setZ(StripContent.getLength("Z") * CLHEP::cm);
  }
}

/**
 * Get detail dx, dy.
 * @param[in]  points  Points.
 * @param[in]  nPoints Number of points.
 * @param[in]  r       Radius.
 * @param[in]  kx      X coefficient.
 * @param[in]  ky      Y coefficient.
 * @param[out] dx      Dx.
 * @param[out] dy      Dy.
 */
static void getDetailDxDy(HepGeom::Point3D<double>* points, int nPoints,
                          double r, double kx, double ky,
                          double& dx, double& dy)
{
  int i;
  /* Variable maxt is initialized to avoid a false-positive warning. */
  /* cppcheck-suppress variableScope */
  double a, b, c, d, t, maxt = 0, x1, y1, x2, y2, u;
  bool intersection;
  /*
   * Contact by one of the detail points.
   * Solve equation (x1 + kx * t)^2 + (y1 + ky * t)^2 = R^2,
   * (kx^2 + ky^2) * t^2 + 2 * (kx * x1 + ky * y1) * t + x1^2 + y1^2 - r^2 = 0.
   */
  a = kx * kx + ky * ky;
  intersection = false;
  for (i = 0; i < nPoints; i++) {
    x1 = points[i].x();
    y1 = points[i].y();
    b = 2.0 * (kx * x1 + ky * y1);
    c = x1 * x1 + y1 * y1 - r * r;
    d = b * b - 4.0 * a * c;
    if (d >= 0) {
      t = (-b + sqrt(d)) / (2.0 * a);
      if (!intersection) {
        intersection = true;
        maxt = t;
      } else {
        if (t > maxt)
          maxt = t;
      }
    }
  }
  if (!intersection)
    B2FATAL("Shield layer geometry calculation error.");
  /*
   * Contact by one of the detail lines.
   * Find t such as the equation
   * (x1 + kx * t + (x2 - x1) * u)^2 + (y1 + ky * t + (y2 - y1) * u) = r^2
   * has one solution relatively to u. Equation on t is
   * 0 = ((x2 - x1) * (x1 + kx * t) + (y2 - y1) * (y1 + ky * t))^2 +
   * ((x2 - x1)^2 + (y2 - y1)^2) * ((x1 + kx * t)^2 + (y1 + ky * t)^2 - r^2),
   * t = (x1 * y2 - x2 * y1 +- r * sqrt((x2 - x1)^2 + (y2 - y1)^2)) /
   *     ((x2 - x1) * ky - (y2 - y1) * kx).
   */
  for (i = 0; i < nPoints; i++) {
    x1 = points[i].x();
    y1 = points[i].y();
    if (i < nPoints - 1) {
      x2 = points[i + 1].x();
      y2 = points[i + 1].y();
    } else {
      x2 = points[0].x();
      y2 = points[0].y();
    }
    a = (x2 - x1) * ky - (y2 - y1) * kx;
    if (a == 0)
      continue;
    b = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    t = (x1 * y2 - x2 * y1 + r * sqrt(b)) / a;
    /*
     * Check whether intersection occurs between the translated points
     * (x1 + kx * t, y1 + ky * t) and (x2 + kx * t, y2 + ky * t).
     * (find solition of the orginal equation relatively to u for that).
     */
    u = -((x2 - x1) * (x1 + kx * t) + (y2 - y1) * (y2 + ky * t)) / b;
    if (u < 0 || u > 1)
      continue;
    if (t > maxt)
      maxt = t;
  }
  dx = kx * maxt;
  dy = ky * maxt;
}

/**
 * Convert EKLMGeometry::Point to CLHEP point.
 * @param[in]  pointEKLM  EKLM point.
 * @param[out] pointCLHEP CLHEP point.
 */
static void EKLMPointToCLHEP(const EKLMGeometry::Point* pointEKLM,
                             HepGeom::Point3D<double>& pointCLHEP)
{
  pointCLHEP.setX(pointEKLM->getX());
  pointCLHEP.setY(pointEKLM->getY());
  pointCLHEP.setZ(0);
}

void EKLM::GeometryData::calculateShieldGeometry()
{
  int i;
  double r, l, dx, dy, xCenter, yCenter;
  const double asqrt2 = 1.0 / sqrt(2.0);
  HepGeom::Point3D<double> points[8];
  const ShieldDetailGeometry* detailA = m_ShieldGeometry.getDetailA();
  const ShieldDetailGeometry* detailB = m_ShieldGeometry.getDetailB();
  const ShieldDetailGeometry* detailC = m_ShieldGeometry.getDetailC();
  const ShieldDetailGeometry* detailD = m_ShieldGeometry.getDetailD();
  r = m_SectorSupportPosition.getInnerR() +
      m_SectorSupportGeometry.getThickness();
  /* Detail A. */
  EKLMPointToCLHEP(detailA->getPoint(0), points[0]);
  EKLMPointToCLHEP(detailA->getPoint(1), points[1]);
  EKLMPointToCLHEP(detailA->getPoint(2), points[2]);
  EKLMPointToCLHEP(detailA->getPoint(3), points[3]);
  EKLMPointToCLHEP(detailA->getPoint(4), points[4]);
  points[5].setX(detailA->getLengthX());
  points[5].setY(detailA->getLengthY());
  points[5].setZ(0);
  EKLMPointToCLHEP(detailA->getPoint(5), points[6]);
  EKLMPointToCLHEP(detailA->getPoint(6), points[7]);
  l = 0.5 * (detailA->getLengthX() + detailB->getLengthX());
  xCenter = -asqrt2 * l;
  yCenter = asqrt2 * l;
  for (i = 0; i < 8; i++)
    points[i] = HepGeom::Translate3D(xCenter, yCenter, 0) *
                HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
                HepGeom::Translate3D(-detailA->getLengthX() / 2,
                                     -detailA->getLengthY() / 2, 0) *
                points[i];
  getDetailDxDy(points, 8, r, 1, 1, dx, dy);
  m_ShieldGeometry.setDetailACenter(xCenter + dx, yCenter + dy);
  /* Details B, D, E. */
  points[0].setX(0);
  points[0].setY(-detailD->getLengthY());
  points[0].setZ(0);
  points[1].setX(detailD->getLengthX());
  points[1].setY(0);
  points[1].setZ(0);
  points[2].setX(detailB->getLengthX() - detailD->getLengthX());
  points[2].setY(0);
  points[2].setZ(0);
  points[3].setX(detailB->getLengthX());
  points[3].setY(-detailD->getLengthY());
  points[3].setZ(0);
  EKLMPointToCLHEP(detailB->getPoint(0), points[4]);
  EKLMPointToCLHEP(detailB->getPoint(1), points[5]);
  EKLMPointToCLHEP(detailB->getPoint(2), points[6]);
  EKLMPointToCLHEP(detailB->getPoint(3), points[7]);
  /* Detail B center coordinates before its shift are (0, 0). */
  for (i = 0; i < 8; i++)
    points[i] = HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
                HepGeom::Translate3D(-detailB->getLengthX() / 2,
                                     -detailB->getLengthY() / 2, 0) *
                points[i];
  getDetailDxDy(points, 8, r, 1, 1, dx, dy);
  m_ShieldGeometry.setDetailBCenter(dx, dy);
  /* Detail C. */
  EKLMPointToCLHEP(detailC->getPoint(0), points[0]);
  EKLMPointToCLHEP(detailC->getPoint(1), points[1]);
  EKLMPointToCLHEP(detailC->getPoint(2), points[2]);
  EKLMPointToCLHEP(detailC->getPoint(3), points[3]);
  EKLMPointToCLHEP(detailC->getPoint(4), points[4]);
  points[5].setX(detailC->getLengthX());
  points[5].setY(detailC->getLengthY());
  points[5].setZ(0);
  EKLMPointToCLHEP(detailC->getPoint(5), points[6]);
  EKLMPointToCLHEP(detailC->getPoint(6), points[7]);
  l = 0.5 * (detailB->getLengthX() + detailC->getLengthX());
  xCenter = asqrt2 * l;
  yCenter = -asqrt2 * l;
  for (i = 0; i < 8; i++)
    points[i] = HepGeom::Translate3D(xCenter, yCenter, 0) *
                HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
                HepGeom::RotateY3D(180.0 * CLHEP::deg) *
                HepGeom::Translate3D(-detailC->getLengthX() / 2,
                                     -detailC->getLengthY() / 2, 0) *
                points[i];
  getDetailDxDy(points, 8, r, 1, 1, dx, dy);
  m_ShieldGeometry.setDetailCCenter(xCenter + dx, yCenter + dy);
}

void EKLM::GeometryData::readEndcapStructureGeometry(const GearDir& gd)
{
  GearDir d(gd);
  d.append("/EndcapStructure");
  m_EndcapStructureGeometry.setPhi(d.getAngle("Phi") * CLHEP::rad);
  m_EndcapStructureGeometry.setNSides(d.getInt("NSides"));
}

void EKLM::GeometryData::initializeFromGearbox(const GearDir* gearDir)
{
  int i, j, k;
  std::string name;
  ShieldDetailGeometry shieldDetailGeometry;
  GearDir gd(*gearDir);
  gd.append("/EKLM");
  /* Numbers of elements. */
  m_NSections = gd.getInt("NSections");
  m_ElementNumbers->checkSection(m_NSections);
  m_NLayers = gd.getInt("NLayers");
  m_ElementNumbers->checkLayer(m_NLayers);
  m_NDetectorLayers = new int[m_NSections];
  m_NDetectorLayers[0] = gd.getInt("NDetectorLayersBackward");
  checkDetectorLayerNumber(1, m_NDetectorLayers[0]);
  if (m_NSections == 2) {
    m_NDetectorLayers[1] = gd.getInt("NDetectorLayersForward");
    checkDetectorLayerNumber(2, m_NDetectorLayers[1]);
  }
  m_NSectors = gd.getInt("NSectors");
  m_ElementNumbers->checkSector(m_NSectors);
  m_NPlanes = gd.getInt("NPlanes");
  m_ElementNumbers->checkPlane(m_NPlanes);
  m_NSegments = gd.getInt("NSegments");
  m_ElementNumbers->checkSegment(m_NSegments);
  m_NSegmentSupportElementsSector = (m_NSegments + 1) * m_NPlanes;
  m_NStrips = gd.getInt("NStrips");
  m_ElementNumbers->checkStrip(m_NStrips);
  /* Geometry parameters. */
  m_SolenoidZ = gd.getLength("SolenoidZ") * CLHEP::cm;
  readEndcapStructureGeometry(gd);
  GearDir section(gd);
  section.append("/Section");
  readPositionData(&m_SectionPosition, &section);
  readSizeData(&m_SectionPosition, &section);
  GearDir layer(gd);
  layer.append("/Layer");
  readSizeData(&m_LayerPosition, &layer);
  m_LayerShiftZ = layer.getLength("ShiftZ") * CLHEP::cm;
  GearDir sector(gd);
  sector.append("/Sector");
  readSizeData(&m_SectorPosition, &sector);
  GearDir sectorSupport(gd);
  sectorSupport.append("/SectorSupport");
  readPositionData(&m_SectorSupportPosition, &sectorSupport);
  readSizeData(&m_SectorSupportPosition, &sectorSupport);
  readSectorSupportGeometry(&m_SectorSupportGeometry, &sectorSupport);
  GearDir plane(gd);
  plane.append("/Plane");
  readPositionData(&m_PlanePosition, &plane);
  readSizeData(&m_PlanePosition, &plane);
  GearDir plasticSheet(gd);
  plasticSheet.append("/PlasticSheet");
  m_PlasticSheetGeometry.setWidth(plasticSheet.getLength("Width") * CLHEP::cm);
  m_PlasticSheetGeometry.setDeltaL(plasticSheet.getLength("DeltaL") *
                                   CLHEP::cm);
  GearDir segmentSupport(gd);
  segmentSupport.append("/SegmentSupport");
  m_SegmentSupportGeometry.setTopWidth(
    segmentSupport.getLength("TopWidth") * CLHEP::cm);
  m_SegmentSupportGeometry.setTopThickness(
    segmentSupport.getLength("TopThickness") * CLHEP::cm);
  m_SegmentSupportGeometry.setMiddleWidth(
    segmentSupport.getLength("MiddleWidth") * CLHEP::cm);
  m_SegmentSupportGeometry.setMiddleThickness(
    segmentSupport.getLength("MiddleThickness") * CLHEP::cm);
  try {
    m_SegmentSupportPosition =
      new EKLMGeometry::SegmentSupportPosition[m_NSegmentSupportElementsSector];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (j = 0; j < m_NPlanes; j++) {
    for (i = 0; i <= m_NSegments; i++) {
      k = j * (m_NSegments + 1) + i;
      GearDir segmentSupport2(segmentSupport);
      name = "/SegmentSupportPlane[" + std::to_string(j + 1) + "]";
      segmentSupport2.append(name.c_str());
      name = "/SegmentSupport[" + std::to_string(i + 1) + "]";
      segmentSupport2.append(name.c_str());
      m_SegmentSupportPosition[k].setLength(
        segmentSupport2.getLength("Length") * CLHEP::cm);
      m_SegmentSupportPosition[k].setX(
        segmentSupport2.getLength("X") * CLHEP::cm);
      m_SegmentSupportPosition[k].setY(
        segmentSupport2.getLength("Y") * CLHEP::cm);
      m_SegmentSupportPosition[k].setZ(
        segmentSupport2.getLength("Z") * CLHEP::cm);
      m_SegmentSupportPosition[k].setDeltaLRight(
        segmentSupport2.getLength("DeltaLRight") * CLHEP::cm);
      m_SegmentSupportPosition[k].setDeltaLLeft(
        segmentSupport2.getLength("DeltaLLeft") * CLHEP::cm);
    }
  }
  readXMLDataStrips(gd);
  GearDir shield(gd);
  shield.append("/Shield");
  m_ShieldGeometry.setThickness(shield.getLength("Thickness") * CLHEP::cm);
  GearDir shieldDetailA(shield);
  shieldDetailA.append("/Detail[@id=\"A\"]");
  readShieldDetailGeometry(&shieldDetailGeometry, &shieldDetailA);
  m_ShieldGeometry.setDetailA(shieldDetailGeometry);
  GearDir shieldDetailB(shield);
  shieldDetailB.append("/Detail[@id=\"B\"]");
  readShieldDetailGeometry(&shieldDetailGeometry, &shieldDetailB);
  m_ShieldGeometry.setDetailB(shieldDetailGeometry);
  GearDir shieldDetailC(shield);
  shieldDetailC.append("/Detail[@id=\"C\"]");
  readShieldDetailGeometry(&shieldDetailGeometry, &shieldDetailC);
  m_ShieldGeometry.setDetailC(shieldDetailGeometry);
  GearDir shieldDetailD(shield);
  shieldDetailD.append("/Detail[@id=\"D\"]");
  readShieldDetailGeometry(&shieldDetailGeometry, &shieldDetailD);
  m_ShieldGeometry.setDetailD(shieldDetailGeometry);
  m_Geometry = new EKLMGeometry(*this);
}

void EKLM::GeometryData::initializeFromDatabase()
{
  DBObjPtr<EKLMGeometry> eklmGeometry;
  if (!eklmGeometry.isValid())
    B2FATAL("No EKLM geometry data in the database.");
  EKLMGeometry::operator=(*eklmGeometry);
  m_Geometry = new EKLMGeometry(*this);
}

EKLM::GeometryData::GeometryData(enum DataSource dataSource,
                                 const GearDir* gearDir)
{
  m_Geometry = nullptr;
  switch (dataSource) {
    case c_Gearbox:
      initializeFromGearbox(gearDir);
      break;
    case c_Database:
      initializeFromDatabase();
      break;
  }
  m_MinZForward = m_SolenoidZ + m_SectionPosition.getZ() -
                  0.5 * m_SectionPosition.getLength();
  m_MaxZBackward = m_SolenoidZ - m_SectionPosition.getZ() +
                   0.5 * m_SectionPosition.getLength();
  calculateSectorSupportGeometry();
  fillStripIndexArrays();
  calculateShieldGeometry();
}

EKLM::GeometryData::~GeometryData()
{
  if (m_Geometry != nullptr)
    delete m_Geometry;
  free(m_StripLenToAll);
  free(m_StripAllToLen);
}

void EKLM::GeometryData::saveToDatabase(const IntervalOfValidity& iov) const
{
  Database::Instance().storeData("EKLMGeometry", m_Geometry, iov);
}

bool EKLM::GeometryData::hitInEKLM(double z) const
{
  double zMm;
  zMm = z / Unit::cm * CLHEP::cm;
  return (zMm > m_MinZForward) || (zMm < m_MaxZBackward);
}

/*
 * Note that numbers of elements are 0-based for all transformation functions.
 */
void
EKLM::GeometryData::getSectionTransform(HepGeom::Transform3D* t, int n) const
{
  if (n == 0)
    *t = HepGeom::Translate3D(m_SectionPosition.getX(), m_SectionPosition.getY(),
                              -m_SectionPosition.getZ() + m_SolenoidZ);
  else
    *t = HepGeom::Translate3D(m_SectionPosition.getX(), m_SectionPosition.getY(),
                              m_SectionPosition.getZ() + m_SolenoidZ) *
         HepGeom::RotateY3D(180.*CLHEP::deg);
}

void
EKLM::GeometryData::getLayerTransform(HepGeom::Transform3D* t, int n) const
{
  *t = HepGeom::Translate3D(0.0, 0.0, m_SectionPosition.getLength() / 2.0 -
                            (n + 1) * m_LayerShiftZ +
                            0.5 * m_LayerPosition.getLength());
}

void
EKLM::GeometryData::getSectorTransform(HepGeom::Transform3D* t, int n) const
{
  switch (n) {
    case 0:
      *t = HepGeom::Translate3D(0., 0., 0.);
      break;
    case 1:
      *t = HepGeom::RotateY3D(180.0 * CLHEP::deg);
      break;
    case 2:
      *t = HepGeom::RotateZ3D(90.0 * CLHEP::deg) *
           HepGeom::RotateY3D(180.0 * CLHEP::deg);
      break;
    case 3:
      *t = HepGeom::RotateZ3D(-90.0 * CLHEP::deg);
      break;
  }
}

void
EKLM::GeometryData::getPlaneTransform(HepGeom::Transform3D* t, int n) const
{
  if (n == 0)
    *t = HepGeom::Translate3D(m_PlanePosition.getX(), m_PlanePosition.getY(),
                              m_PlanePosition.getZ()) *
         HepGeom::Rotate3D(180. * CLHEP::deg,
                           HepGeom::Vector3D<double>(1., 1., 0.));
  else
    *t = HepGeom::Translate3D(m_PlanePosition.getX(), m_PlanePosition.getY(),
                              -m_PlanePosition.getZ());
}

void
EKLM::GeometryData::getStripTransform(HepGeom::Transform3D* t, int n) const
{
  *t = HepGeom::Translate3D(m_StripPosition[n].getX(),
                            m_StripPosition[n].getY(), 0.0);
}

void
EKLM::GeometryData::getSheetTransform(HepGeom::Transform3D* t, int n) const
{
  double y;
  y = m_StripPosition[n].getY();
  if (n % m_ElementNumbers->getNStripsSegment() == 0)
    y = y + 0.5 * m_PlasticSheetGeometry.getDeltaL();
  else if (n % m_ElementNumbers->getNStripsSegment() ==
           m_ElementNumbers->getNStripsSegment() - 1)
    y = y - 0.5 * m_PlasticSheetGeometry.getDeltaL();
  *t = HepGeom::Translate3D(m_StripPosition[n].getX(), y, 0.0);
}

