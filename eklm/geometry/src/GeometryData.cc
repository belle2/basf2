/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Units/PhysicalConstants.h>

/* Belle2 headers. */
#include <eklm/geometry/Circle2D.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/geometry/Line2D.h>
#include <framework/database/Database.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char c_MemErr[] = "Memory allocation error.";

const EKLM::GeometryData&
EKLM::GeometryData::Instance(enum DataSource dataSource)
{
  static EKLM::GeometryData gd(dataSource);
  return gd;
}

/**
 * Read position data.
 * @param epos Position data.
 * @param gd   XML data directory.
 */
static void readPositionData(struct EKLMGeometry::ElementPosition* epos, GearDir* gd)
{
  epos->X = gd->getLength("PositionX") * CLHEP::cm;
  epos->Y = gd->getLength("PositionY") * CLHEP::cm;
  epos->Z = gd->getLength("PositionZ") * CLHEP::cm;
}

/**
 * Read size data.
 * @param epos Position data.
 * @param gd   XML data directory.
 */
static void readSizeData(struct EKLMGeometry::ElementPosition* epos, GearDir* gd)
{
  epos->InnerR = gd->getLength("InnerR") * CLHEP::cm;
  epos->OuterR = gd->getLength("OuterR") * CLHEP::cm;
  epos->Length = gd->getLength("Length") * CLHEP::cm;
}

/**
 * Read sector support geometry data.
 * @param ssg Sector support geometry data.
 * @param gd  XML data directory.
 */
static void readSectorSupportGeometry(
  struct EKLMGeometry::SectorSupportGeometry* ssg, GearDir* gd)
{
  ssg->Thickness = gd->getLength("Thickness") * CLHEP::cm;
  ssg->DeltaLY = gd->getLength("DeltaLY") * CLHEP::cm;
  ssg->CornerX = gd->getLength("CornerX") * CLHEP::cm;
  ssg->Corner1LX = gd->getLength("Corner1LX") * CLHEP::cm;
  ssg->Corner1Width = gd->getLength("Corner1Width") * CLHEP::cm;
  ssg->Corner1Thickness = gd->getLength("Corner1Thickness") * CLHEP::cm;
  ssg->Corner1Z = gd->getLength("Corner1Z") * CLHEP::cm;
  ssg->Corner2LX = gd->getLength("Corner2LX") * CLHEP::cm;
  ssg->Corner2LY = gd->getLength("Corner2LY") * CLHEP::cm;
  ssg->Corner2Thickness = gd->getLength("Corner2Thickness") * CLHEP::cm;
  ssg->Corner2Z = gd->getLength("Corner2Z") * CLHEP::cm;
  ssg->Corner3LX = gd->getLength("Corner3LX") * CLHEP::cm;
  ssg->Corner3LY = gd->getLength("Corner3LY") * CLHEP::cm;
  ssg->Corner3Thickness = gd->getLength("Corner3Thickness") * CLHEP::cm;
  ssg->Corner3Z = gd->getLength("Corner3Z") * CLHEP::cm;
  ssg->Corner4LX = gd->getLength("Corner4LX") * CLHEP::cm;
  ssg->Corner4LY = gd->getLength("Corner4LY") * CLHEP::cm;
  ssg->Corner4Thickness = gd->getLength("Corner4Thickness") * CLHEP::cm;
  ssg->Corner4Z = gd->getLength("Corner4Z") * CLHEP::cm;
}

/**
 * Read shield layer detail geometry data.
 * @param sdg Shield layer detail geometry data.
 * @param gd  XML data directory.
 */
static void readShieldDetailGeometry(
  struct EKLMGeometry::ShieldDetailGeometry* sdg, GearDir* gd)
{
  int i;
  sdg->LengthX = gd->getLength("LengthX") * CLHEP::cm;
  sdg->LengthY = gd->getLength("LengthY") * CLHEP::cm;
  sdg->NPoints = gd->getNumberNodes("Point");
  if (sdg->NPoints == 0) {
    sdg->Points = NULL;
    return;
  }
  try {
    sdg->Points = new EKLMGeometry::Point[sdg->NPoints];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (i = 0; i < sdg->NPoints; i++) {
    GearDir point(*gd);
    point.append((boost::format("/Point[%1%]") % (i + 1)).str());
    sdg->Points[i].X = point.getLength("X") * CLHEP::cm;
    sdg->Points[i].Y = point.getLength("Y") * CLHEP::cm;
  }
}

void EKLM::GeometryData::calculateSectorSupportGeometry()
{
  Line2D line23Outer(0, m_SectorSupportPosition.Y, 1, 0);
  Line2D line23Inner(0, m_SectorSupportPosition.Y +
                     m_SectorSupportGeometry.Thickness, 1, 0);
  Line2D line23Prism(0, m_SectorSupportPosition.Y +
                     m_SectorSupportGeometry.Thickness +
                     m_SectorSupportGeometry.Corner3LY, 1, 0);
  Line2D line41Outer(m_SectorSupportPosition.X, 0, 0, 1);
  Line2D line41Inner(m_SectorSupportPosition.X +
                     m_SectorSupportGeometry.Thickness, 0, 0, 1);
  Line2D line41Prism(m_SectorSupportPosition.X +
                     m_SectorSupportGeometry.Thickness +
                     m_SectorSupportGeometry.Corner4LX, 0, 0, 1);
  Line2D line41Corner1B(m_SectorSupportPosition.X +
                        m_SectorSupportGeometry.CornerX, 0, 0, 1);
  Circle2D circleInnerOuter(0, 0, m_SectorSupportPosition.InnerR);
  Circle2D circleInnerInner(0, 0, m_SectorSupportPosition.InnerR +
                            m_SectorSupportGeometry.Thickness);
  Circle2D circleOuterInner(0, 0, m_SectorSupportPosition.OuterR -
                            m_SectorSupportGeometry.Thickness);
  Circle2D circleOuterOuter(0, 0, m_SectorSupportPosition.OuterR);
  HepGeom::Point3D<double> intersections[2];
  /* Corner 1. */
  m_SectorSupportGeometry.Corner1A.setX(m_SectorSupportPosition.X);
  m_SectorSupportGeometry.Corner1A.setY(m_SectorSupportPosition.OuterR -
                                        m_SectorSupportGeometry.DeltaLY);
  m_SectorSupportGeometry.Corner1A.setZ(0);
  line41Corner1B.findIntersection(circleOuterOuter, intersections);
  m_SectorSupportGeometry.Corner1B = intersections[1];
  m_SectorSupportGeometry.CornerAngle =
    atan2(m_SectorSupportGeometry.Corner1B.y() -
          m_SectorSupportGeometry.Corner1A.y(),
          m_SectorSupportGeometry.Corner1B.x() -
          m_SectorSupportGeometry.Corner1A.x()) *
    CLHEP::rad;
  m_SectorSupportGeometry.Corner1AInner.setX(m_SectorSupportPosition.X +
                                             m_SectorSupportGeometry.Thickness);
  m_SectorSupportGeometry.Corner1AInner.setY(
    m_SectorSupportGeometry.Corner1A.y() -
    m_SectorSupportGeometry.Thickness *
    (1.0 / cos(m_SectorSupportGeometry.CornerAngle) -
     tan(m_SectorSupportGeometry.CornerAngle)));
  m_SectorSupportGeometry.Corner1AInner.setZ(0);
  Line2D lineCorner1(m_SectorSupportGeometry.Corner1AInner.x(),
                     m_SectorSupportGeometry.Corner1AInner.y(),
                     m_SectorSupportGeometry.Corner1B.x() -
                     m_SectorSupportGeometry.Corner1A.x(),
                     m_SectorSupportGeometry.Corner1B.y() -
                     m_SectorSupportGeometry.Corner1A.y());
  lineCorner1.findIntersection(circleOuterInner, intersections);
  m_SectorSupportGeometry.Corner1BInner = intersections[1];
  /* Corner 2. */
  line23Inner.findIntersection(circleOuterInner, intersections);
  m_SectorSupportGeometry.Corner2Inner = intersections[1];
  /* Corner 3. */
  line23Outer.findIntersection(circleInnerOuter, intersections);
  m_SectorSupportGeometry.Corner3 = intersections[1];
  line23Inner.findIntersection(circleInnerInner, intersections);
  m_SectorSupportGeometry.Corner3Inner = intersections[1];
  line23Prism.findIntersection(circleInnerInner, intersections);
  m_SectorSupportGeometry.Corner3Prism.setX(intersections[1].x());
  m_SectorSupportGeometry.Corner3Prism.setY(
    m_SectorSupportPosition.Y + m_SectorSupportGeometry.Thickness);
  m_SectorSupportGeometry.Corner3Prism.setZ(0);
  /* Corner 4. */
  line41Outer.findIntersection(circleInnerOuter, intersections);
  m_SectorSupportGeometry.Corner4 = intersections[1];
  line41Inner.findIntersection(circleInnerInner, intersections);
  m_SectorSupportGeometry.Corner4Inner = intersections[1];
  line41Prism.findIntersection(circleInnerInner, intersections);
  m_SectorSupportGeometry.Corner4Prism.setX(
    m_SectorSupportPosition.X + m_SectorSupportGeometry.Thickness);
  m_SectorSupportGeometry.Corner4Prism.setY(intersections[1].y());
  m_SectorSupportGeometry.Corner4Prism.setZ(0);
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
    strips.push_back(m_StripPosition[i].Length);
    mapLengthStrip.insert(std::pair<double, int>(m_StripPosition[i].Length, i));
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
  if (m_StripAllToLen == NULL)
    B2FATAL(c_MemErr);
  for (i = 0; i < m_NStrips; i++) {
    itm = mapLengthStrip2.find(m_StripPosition[i].Length);
    if (itm == mapLengthStrip2.end())
      B2FATAL(err);
    m_StripAllToLen[i] = itm->second;
  }
}

void EKLM::GeometryData::readXMLDataStrips()
{
  int i;
  GearDir Strips("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Endcap/"
                 "Layer/Sector/Plane/Strips");
  m_NStrips = Strips.getNumberNodes("Strip");
  checkStrip(m_NStrips);
  m_StripGeometry.Width  = Strips.getLength("Width") * CLHEP::cm;
  m_StripGeometry.Thickness = Strips.getLength("Thickness") * CLHEP::cm;
  m_StripGeometry.GrooveDepth = Strips.getLength("GrooveDepth") * CLHEP::cm;
  m_StripGeometry.GrooveWidth = Strips.getLength("GrooveWidth") * CLHEP::cm;
  m_StripGeometry.NoScintillationThickness =
    Strips.getLength("NoScintillationThickness") * CLHEP::cm;
  m_StripGeometry.RSSSize = Strips.getLength("RSSSize") * CLHEP::cm;
  try {
    m_StripPosition = new struct EKLMGeometry::ElementPosition[m_NStrips];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (i = 0; i < m_NStrips; i++) {
    GearDir StripContent(Strips);
    StripContent.append((boost::format("/Strip[%1%]") % (i + 1)).str());
    m_StripPosition[i].Length = StripContent.getLength("Length") * CLHEP::cm;
    m_StripPosition[i].X = StripContent.getLength("PositionX") * CLHEP::cm;
    m_StripPosition[i].Y = StripContent.getLength("PositionY") * CLHEP::cm;
    m_StripPosition[i].Z = StripContent.getLength("PositionZ") * CLHEP::cm;
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
  double a, b, c, d, t, maxt, x1, y1, x2, y2, u;
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
static void EKLMPointToCLHEP(const EKLMGeometry::Point& pointEKLM,
                             HepGeom::Point3D<double>& pointCLHEP)
{
  pointCLHEP.setX(pointEKLM.X);
  pointCLHEP.setY(pointEKLM.Y);
  pointCLHEP.setZ(0);
}

void EKLM::GeometryData::calculateShieldGeometry()
{
  int i;
  double r, l, dx, dy;
  const double asqrt2 = 1.0 / sqrt(2.0);
  HepGeom::Point3D<double> points[8];
  r = m_SectorSupportPosition.InnerR + m_SectorSupportGeometry.Thickness;
  /* Detail A. */
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[0], points[0]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[1], points[1]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[2], points[2]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[3], points[3]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[4], points[4]);
  points[5].setX(m_ShieldGeometry.DetailA.LengthX);
  points[5].setY(m_ShieldGeometry.DetailA.LengthY);
  points[5].setZ(0);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[5], points[6]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailA.Points[6], points[7]);
  l = 0.5 * (m_ShieldGeometry.DetailA.LengthX +
             m_ShieldGeometry.DetailB.LengthX);
  m_ShieldGeometry.DetailACenter.setX(-asqrt2 * l);
  m_ShieldGeometry.DetailACenter.setY(asqrt2 * l);
  for (i = 0; i < 8; i++)
    points[i] = HepGeom::Translate3D(m_ShieldGeometry.DetailACenter.x(),
                                     m_ShieldGeometry.DetailACenter.y(), 0) *
                HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
                HepGeom::Translate3D(-m_ShieldGeometry.DetailA.LengthX / 2,
                                     -m_ShieldGeometry.DetailA.LengthY / 2, 0) *
                points[i];
  getDetailDxDy(points, 8, r, 1, 1, dx, dy);
  m_ShieldGeometry.DetailACenter.setX(m_ShieldGeometry.DetailACenter.x() + dx);
  m_ShieldGeometry.DetailACenter.setY(m_ShieldGeometry.DetailACenter.y() + dy);
  m_ShieldGeometry.DetailACenter.setZ(0);
  /* Details B, D, E. */
  points[0].setX(0);
  points[0].setY(-m_ShieldGeometry.DetailD.LengthY);
  points[0].setZ(0);
  points[1].setX(m_ShieldGeometry.DetailD.LengthX);
  points[1].setY(0);
  points[1].setZ(0);
  points[2].setX(m_ShieldGeometry.DetailB.LengthX -
                 m_ShieldGeometry.DetailD.LengthX);
  points[2].setY(0);
  points[2].setZ(0);
  points[3].setX(m_ShieldGeometry.DetailB.LengthX);
  points[3].setY(-m_ShieldGeometry.DetailD.LengthY);
  points[3].setZ(0);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailB.Points[0], points[4]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailB.Points[1], points[5]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailB.Points[2], points[6]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailB.Points[3], points[7]);
  /* Detail B center coordinates are (0, 0), not necessary to set them here. */
  for (i = 0; i < 8; i++)
    points[i] = HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
                HepGeom::Translate3D(-m_ShieldGeometry.DetailB.LengthX / 2,
                                     -m_ShieldGeometry.DetailB.LengthY / 2, 0) *
                points[i];
  getDetailDxDy(points, 8, r, 1, 1, dx, dy);
  m_ShieldGeometry.DetailBCenter.setX(m_ShieldGeometry.DetailBCenter.x() + dx);
  m_ShieldGeometry.DetailBCenter.setY(m_ShieldGeometry.DetailBCenter.y() + dy);
  m_ShieldGeometry.DetailBCenter.setZ(0);
  /* Detail C. */
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[0], points[0]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[1], points[1]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[2], points[2]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[3], points[3]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[4], points[4]);
  points[5].setX(m_ShieldGeometry.DetailC.LengthX);
  points[5].setY(m_ShieldGeometry.DetailC.LengthY);
  points[5].setZ(0);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[5], points[6]);
  EKLMPointToCLHEP(m_ShieldGeometry.DetailC.Points[6], points[7]);
  l = 0.5 * (m_ShieldGeometry.DetailB.LengthX +
             m_ShieldGeometry.DetailC.LengthX);
  m_ShieldGeometry.DetailCCenter.setX(asqrt2 * l);
  m_ShieldGeometry.DetailCCenter.setY(-asqrt2 * l);
  for (i = 0; i < 8; i++)
    points[i] = HepGeom::Translate3D(m_ShieldGeometry.DetailCCenter.x(),
                                     m_ShieldGeometry.DetailCCenter.y(), 0) *
                HepGeom::RotateZ3D(-45.0 * CLHEP::deg) *
                HepGeom::RotateY3D(180.0 * CLHEP::deg) *
                HepGeom::Translate3D(-m_ShieldGeometry.DetailC.LengthX / 2,
                                     -m_ShieldGeometry.DetailC.LengthY / 2, 0) *
                points[i];
  getDetailDxDy(points, 8, r, 1, 1, dx, dy);
  m_ShieldGeometry.DetailCCenter.setX(m_ShieldGeometry.DetailCCenter.x() + dx);
  m_ShieldGeometry.DetailCCenter.setY(m_ShieldGeometry.DetailCCenter.y() + dy);
  m_ShieldGeometry.DetailCCenter.setZ(0);
}

void EKLM::GeometryData::readEndcapStructureGeometry()
{
  int i;
  GearDir d("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/ESTR");
  GearDir d1(d);
  GearDir d2(d);
  d1.append("/EndcapKLM");
  d2.append("/EndcapKLMsub");
  m_EndcapStructureGeometry.Phi = d1.getAngle("Phi") * CLHEP::rad;
  m_EndcapStructureGeometry.Dphi = d1.getAngle("Dphi") * CLHEP::rad;
  m_EndcapStructureGeometry.Nsides = d1.getInt("Nsides");
  m_EndcapStructureGeometry.Nboundary = d1.getNumberNodes("ZBoundary");
  m_EndcapStructureGeometry.Z =
    (double*)malloc(m_EndcapStructureGeometry.Nboundary * sizeof(double));
  if (m_EndcapStructureGeometry.Z == NULL)
    B2FATAL(c_MemErr);
  m_EndcapStructureGeometry.Rmin =
    (double*)malloc(m_EndcapStructureGeometry.Nboundary * sizeof(double));
  if (m_EndcapStructureGeometry.Rmin == NULL)
    B2FATAL(c_MemErr);
  m_EndcapStructureGeometry.Rmax =
    (double*)malloc(m_EndcapStructureGeometry.Nboundary * sizeof(double));
  if (m_EndcapStructureGeometry.Rmax == NULL)
    B2FATAL(c_MemErr);
  for (i = 0; i < m_EndcapStructureGeometry.Nboundary; i++) {
    GearDir d4(d1);
    d4.append((boost::format("/ZBoundary[%1%]") % (i + 1)).str());
    m_EndcapStructureGeometry.Z[i] = d4.getLength("Zposition") * CLHEP::cm;
    m_EndcapStructureGeometry.Rmin[i] = d4.getLength("InnerRadius") * CLHEP::cm;
    m_EndcapStructureGeometry.Rmax[i] = d4.getLength("OuterRadius") * CLHEP::cm;
  }
  m_EndcapStructureGeometry.Zsub = d2.getLength("Length") * CLHEP::cm;
  m_EndcapStructureGeometry.Rminsub = d2.getLength("InnerRadius") * CLHEP::cm;
  m_EndcapStructureGeometry.Rmaxsub = d2.getLength("OuterRadius") * CLHEP::cm;
}

void EKLM::GeometryData::initializeFromGearbox()
{
  int i, j, k, mode;
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/Content");
  mode = gd.getInt("Mode");
  if (mode < 0 || mode > 1)
    B2FATAL("EKLM started with unknown geometry mode " << mode << ".");
  m_Mode = (enum DetectorMode)mode;
  readEndcapStructureGeometry();
  m_SolenoidZ = gd.getLength("SolenoidZ") * CLHEP::cm;
  m_NEndcaps = gd.getInt("NEndcaps");
  checkEndcap(m_NEndcaps);
  GearDir EndCap(gd);
  EndCap.append("/Endcap");
  readPositionData(&m_EndcapPosition, &EndCap);
  readSizeData(&m_EndcapPosition, &EndCap);
  m_MinZForward = m_SolenoidZ + m_EndcapPosition.Z -
                  0.5 * m_EndcapPosition.Length;
  m_MaxZBackward = m_SolenoidZ - m_EndcapPosition.Z +
                   0.5 * m_EndcapPosition.Length;
  m_NLayers = EndCap.getInt("NLayers");
  checkLayer(m_NLayers);
  m_NDetectorLayers = new int[m_NEndcaps];
  m_NDetectorLayers[0] = EndCap.getInt("NDetectorLayersBackward");
  checkDetectorLayerNumber(1, m_NDetectorLayers[0]);
  if (m_NEndcaps == 2) {
    m_NDetectorLayers[1] = EndCap.getInt("NDetectorLayersForward");
    checkDetectorLayerNumber(2, m_NDetectorLayers[1]);
  }
  GearDir Layer(EndCap);
  Layer.append("/Layer");
  readSizeData(&m_LayerPosition, &Layer);
  m_LayerShiftZ = Layer.getLength("ShiftZ") * CLHEP::cm;
  m_NSectors = Layer.getInt("NSectors");
  checkSector(m_NSectors);
  GearDir Sector(Layer);
  Sector.append("/Sector");
  readSizeData(&m_SectorPosition, &Sector);
  m_NPlanes = Sector.getInt("NPlanes");
  checkPlane(m_NPlanes);
  GearDir SectorSupport(Sector);
  SectorSupport.append("/SectorSupport");
  readPositionData(&m_SectorSupportPosition, &SectorSupport);
  readSizeData(&m_SectorSupportPosition, &SectorSupport);
  readSectorSupportGeometry(&m_SectorSupportGeometry, &SectorSupport);
  calculateSectorSupportGeometry();
  GearDir Plane(Sector);
  Plane.append("/Plane");
  readPositionData(&m_PlanePosition, &Plane);
  readSizeData(&m_PlanePosition, &Plane);
  m_NSegments = Plane.getInt("NSegments");
  if (m_NSegments <= 0)
    B2FATAL("Number of segments must be positive.");
  m_NSegmentSupportElementsSector = (m_NSegments + 1) * m_NPlanes;
  m_PlasticSheetGeometry.Width = Plane.getLength("PlasticSheetWidth") *
                                 CLHEP::cm;
  m_PlasticSheetGeometry.DeltaL = Plane.getLength("PlasticSheetDeltaL") *
                                  CLHEP::cm;
  GearDir Segments(Plane);
  Segments.append("/Segments");
  m_SegmentSupportGeometry.TopWidth =
    Segments.getLength("TopWidth") * CLHEP::cm;
  m_SegmentSupportGeometry.TopThickness =
    Segments.getLength("TopThickness") * CLHEP::cm;
  m_SegmentSupportGeometry.MiddleWidth =
    Segments.getLength("MiddleWidth") * CLHEP::cm;
  m_SegmentSupportGeometry.MiddleThickness =
    Segments.getLength("MiddleThickness") * CLHEP::cm;
  try {
    m_SegmentSupportPosition =
      new struct EKLMGeometry::SegmentSupportPosition[
        m_NSegmentSupportElementsSector];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (j = 0; j < m_NPlanes; j++) {
    for (i = 0; i <= m_NSegments; i++) {
      k = j * (m_NSegments + 1) + i;
      GearDir SegmentSupportContent(Segments);
      SegmentSupportContent.append(
        (boost::format("/SegmentSupportData[%1%]") % (j + 1)).str());
      SegmentSupportContent.append(
        (boost::format("/SegmentSupport[%1%]") % (i + 1)).str());
      m_SegmentSupportPosition[k].Length =
        SegmentSupportContent.getLength("Length") * CLHEP::cm;
      m_SegmentSupportPosition[k].X =
        SegmentSupportContent.getLength("PositionX") * CLHEP::cm;
      m_SegmentSupportPosition[k].Y =
        SegmentSupportContent.getLength("PositionY") * CLHEP::cm;
      m_SegmentSupportPosition[k].Z =
        SegmentSupportContent.getLength("PositionZ") * CLHEP::cm;
      m_SegmentSupportPosition[k].DeltaLRight =
        SegmentSupportContent.getLength("DeltaLRight") * CLHEP::cm;
      m_SegmentSupportPosition[k].DeltaLLeft =
        SegmentSupportContent.getLength("DeltaLLeft") * CLHEP::cm;
    }
  }
  m_NStripsSegment = Plane.getInt("NStripsSegment");
  readXMLDataStrips();
  GearDir shield(Sector);
  shield.append("/Shield");
  m_ShieldGeometry.Thickness = shield.getLength("Thickness") * CLHEP::cm;
  GearDir shieldDetailA(shield);
  shieldDetailA.append("/Detail[@id=\"A\"]");
  readShieldDetailGeometry(&m_ShieldGeometry.DetailA, &shieldDetailA);
  GearDir shieldDetailB(shield);
  shieldDetailB.append("/Detail[@id=\"B\"]");
  readShieldDetailGeometry(&m_ShieldGeometry.DetailB, &shieldDetailB);
  GearDir shieldDetailC(shield);
  shieldDetailC.append("/Detail[@id=\"C\"]");
  readShieldDetailGeometry(&m_ShieldGeometry.DetailC, &shieldDetailC);
  GearDir shieldDetailD(shield);
  shieldDetailD.append("/Detail[@id=\"D\"]");
  readShieldDetailGeometry(&m_ShieldGeometry.DetailD, &shieldDetailD);
  m_NBoards = Sector.getInt("NBoards");
  checkSegment(m_NBoards);
  m_NBoardsSector = m_NBoards * m_NPlanes;
  GearDir Boards(Sector);
  Boards.append("/Boards");
  m_BoardGeometry.Length = Boards.getLength("Length") * CLHEP::cm;
  m_BoardGeometry.Width = Boards.getLength("Width") * CLHEP::cm;
  m_BoardGeometry.Height = Boards.getLength("Height") * CLHEP::cm;
  m_BoardGeometry.BaseWidth = Boards.getLength("BaseWidth") * CLHEP::cm;
  m_BoardGeometry.BaseHeight = Boards.getLength("BaseHeight") * CLHEP::cm;
  m_BoardGeometry.StripLength = Boards.getLength("StripLength") * CLHEP::cm;
  m_BoardGeometry.StripWidth = Boards.getLength("StripWidth") * CLHEP::cm;
  m_BoardGeometry.StripHeight = Boards.getLength("StripHeight") * CLHEP::cm;
  m_NStripBoards = Boards.getInt("NStripBoards");
  try {
    m_BoardPosition = new struct BoardPosition[m_NBoardsSector];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (j = 0; j < m_NPlanes; j++) {
    for (i = 0; i < m_NBoards; i++) {
      k = j * m_NBoards + i;
      GearDir BoardContent(Boards);
      BoardContent.append((boost::format("/BoardData[%1%]") % (j + 1)).str());
      BoardContent.append((boost::format("/Board[%1%]") % (i + 1)).str());
      m_BoardPosition[k].Phi = BoardContent.getLength("Phi") * CLHEP::rad;
      m_BoardPosition[k].R = BoardContent.getLength("Radius") * CLHEP::cm;
    }
  }
  try {
    m_StripBoardPosition = new struct StripBoardPosition[m_NStripBoards];
  } catch (std::bad_alloc& ba) {
    B2FATAL(c_MemErr);
  }
  for (i = 0; i < m_NStripBoards; i++) {
    GearDir StripBoardContent(Boards);
    StripBoardContent.append((boost::format("/StripBoardData/Board[%1%]") %
                              (i + 1)).str());
    m_StripBoardPosition[i].X = StripBoardContent.getLength("PositionX") *
                                CLHEP::cm;
  }
  m_Geometry = new EKLMGeometry(*this);
}

void EKLM::GeometryData::initializeFromDatabase()
{
}

EKLM::GeometryData::GeometryData(enum DataSource dataSource)
{
  m_Geometry = NULL;
  switch (dataSource) {
    case c_Gearbox:
      initializeFromGearbox();
      break;
    case c_Database:
      initializeFromDatabase();
      break;
  }
  fillStripIndexArrays();
  calculateShieldGeometry();
}

/**
 * Free shield layer detail geometry data.
 * @param sdg Shield layer detail geometry data.
 */
static void freeShieldDetail(struct EKLMGeometry::ShieldDetailGeometry* sdg)
{
  if (sdg->Points != NULL)
    delete[] sdg->Points;
}

EKLM::GeometryData::~GeometryData()
{
  if (m_Geometry != NULL)
    delete m_Geometry;
  delete[] m_NDetectorLayers;
  free(m_EndcapStructureGeometry.Z);
  free(m_EndcapStructureGeometry.Rmin);
  free(m_EndcapStructureGeometry.Rmax);
  delete[] m_SegmentSupportPosition;
  delete[] m_BoardPosition;
  delete[] m_StripBoardPosition;
  free(m_StripLenToAll);
  free(m_StripAllToLen);
  freeShieldDetail(&m_ShieldGeometry.DetailA);
  freeShieldDetail(&m_ShieldGeometry.DetailB);
  freeShieldDetail(&m_ShieldGeometry.DetailC);
  freeShieldDetail(&m_ShieldGeometry.DetailD);
}

void EKLM::GeometryData::saveToDatabase(const IntervalOfValidity& iov) const
{
  Database::Instance().storeData("EKLMGeometry", (TObject*)m_Geometry, iov);
}

double EKLM::GeometryData::getStripLength(int strip) const
{
  return m_StripPosition[strip - 1].Length;
}

int EKLM::GeometryData::getStripLengthIndex(int positionIndex) const
{
  return m_StripAllToLen[positionIndex];
}

int EKLM::GeometryData::getStripPositionIndex(int lengthIndex) const
{
  return m_StripLenToAll[lengthIndex];
}

int EKLM::GeometryData::getNStripsDifferentLength() const
{
  return m_nStripDifferent;
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
EKLM::GeometryData::getEndcapTransform(HepGeom::Transform3D* t, int n) const
{
  if (n == 0)
    *t = HepGeom::Translate3D(m_EndcapPosition.X, m_EndcapPosition.Y,
                              -m_EndcapPosition.Z + m_SolenoidZ);
  else
    *t = HepGeom::Translate3D(m_EndcapPosition.X, m_EndcapPosition.Y,
                              m_EndcapPosition.Z + m_SolenoidZ) *
         HepGeom::RotateY3D(180.*CLHEP::deg);
}

void
EKLM::GeometryData::getLayerTransform(HepGeom::Transform3D* t, int n) const
{
  *t = HepGeom::Translate3D(0.0, 0.0, m_EndcapPosition.Length / 2.0 -
                            (n + 1) * m_LayerShiftZ +
                            0.5 * m_LayerPosition.Length);
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
    *t = HepGeom::Translate3D(m_PlanePosition.X, m_PlanePosition.Y,
                              m_PlanePosition.Z) *
         HepGeom::Rotate3D(180. * CLHEP::deg,
                           HepGeom::Vector3D<double>(1., 1., 0.));
  else
    *t = HepGeom::Translate3D(m_PlanePosition.X, m_PlanePosition.Y,
                              -m_PlanePosition.Z);
}

void
EKLM::GeometryData::getStripTransform(HepGeom::Transform3D* t, int n) const
{
  *t = HepGeom::Translate3D(m_StripPosition[n].X, m_StripPosition[n].Y, 0.0);
}

void
EKLM::GeometryData::getSheetTransform(HepGeom::Transform3D* t, int n) const
{
  double y;
  y = m_StripPosition[n].Y;
  if (n % 15 == 0)
    y = y + 0.5 * m_PlasticSheetGeometry.DeltaL;
  else if (n % 15 == 14)
    y = y - 0.5 * m_PlasticSheetGeometry.DeltaL;
  *t = HepGeom::Translate3D(m_StripPosition[n].X, y, 0.0);
}

