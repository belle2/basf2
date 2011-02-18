
#include <tracking/modules/datareduction/SectorArc.h>
#include <cmath>

#include <tracking/modules/datareduction/HoughTransformStraight.h>

#include "TVector2.h"

using namespace std;
using namespace Belle2;

SectorArc::SectorArc(double RotationAngle, double DownShift, double SectorWidth, double RadiusSmall, double RadiusBig):
    SectorBasic(RotationAngle, DownShift, SectorWidth), _RadiusSmall(min(RadiusSmall, RadiusBig)), _RadiusBig(max(RadiusSmall, RadiusBig))
{
  //Signed radius, so swap if both are negative, otherwise we get closed sectors
  if ((_RadiusSmall < 0 && _RadiusBig < 0)) {
    double tmp = _RadiusSmall;
    _RadiusSmall = _RadiusBig;
    _RadiusBig = tmp;
  }
  double ySmall = _RadiusSmall - (_RadiusSmall > 0 ? -1 : 1) * _SectorWidth;
  double yBig   = _RadiusBig  - (_RadiusBig > 0 ? 1 : -1)  * _SectorWidth;
  if (_RadiusSmall<0 && _RadiusBig>0) {
    yBig += 2 * _SectorWidth;
  }
  _CenterSmall = TVector2(-_DownShift, ySmall);
  _CenterBig = TVector2(-_DownShift, yBig);
}


SectorArc::~SectorArc()
{

}

std::pair<double, double> SectorArc::intersectCircle(PXDLadder &ladder, TVector2 center, double radius, bool in)
{
  TVector3 p = ladder.getPosition();
  TVector3 s = ladder.getSize();
  TVector3 n = ladder.getNormal();
  double width = s(0) / 2;
  //Shift center to local ladder frame
  center = center.Rotate(_RotationAngle) + TVector2(-p(0), -p(1));
  //Rotate center to local ladder frame
  center = center.Rotate(-n.Phi() + M_PI / 2.0);
  if (fabs(center.Y()) > radius) return pair<double, double>(0, 1);
  double d = sqrt(radius * radius - center.Y() * center.Y());
  //calculate intersections in range 0,1
  double x1 = (center.X() - d + width) / (2 * width);
  double x2 = (center.X() + d + width) / (2 * width);
  //if in, all between x1 and x2 is valid, else everything outside of the two
  double start(0);
  double end(1);
  if (in) {
    //x1 is always smaller x2, so one restricts start and one end;
    if (x1 >= 1 || x2 <= 0) {
      start = 0;
      end   = 0;
    } else {
      if (x1 >= 0 && x1 <= 1) start = x1;
      if (x2 >= 0 && x2 <= 1) end   = x2;
    }
  } else {
    //valid ends outside limit
    //FIXME: if line is left and right of circle, start>end. Makes sense but has to be interpreted
    if (x1 <= 0 && x2 >= 1) {
      start = 0;
      end   = 0;
    } else {
      if (x1 >= 0 && x1 <= 1) end   = x1;
      if (x2 >= 0 && x2 <= 1) start = x2;
    }
  }
  return pair<double, double>(start, end);
}

void SectorArc::setIntersection(LadderEntry& ladderEntry)
{
  pair<double, double> imin = intersectCircle(*(ladderEntry.ladder), _CenterSmall, fabs(_RadiusSmall), false);
  pair<double, double> imax = intersectCircle(*(ladderEntry.ladder), _CenterBig, fabs(_RadiusBig), !(_RadiusSmall<0 && _RadiusBig>0));
  ladderEntry.start = max(imin.first, imax.first);
  ladderEntry.end = min(imin.second, imax.second);

  SectorBasic::setIntersection(ladderEntry);
}


int SectorArc::checkPoint(double x, double y)
{
  double xrot = x * cos(-_RotationAngle) - y * sin(-_RotationAngle) + _DownShift;
  //Wrong side, return -2 to signal not using ever
  if (xrot < 0) return -2;
  double yrot = x * sin(-_RotationAngle) + y * cos(-_RotationAngle);
  // Distance to center of minpt
  double ymin = yrot - _CenterSmall.Y();
  bool overmin = sqrt(xrot * xrot + ymin * ymin) >= fabs(_RadiusSmall);

  // Distance to center if maxpt;
  double ymax = yrot - _CenterBig.Y();
  bool undermax = sqrt(xrot * xrot + ymax * ymax) <= fabs(_RadiusBig);

  bool in = undermax && overmin;
  //If sign-change, then condition differs
  if (_RadiusSmall<0 && _RadiusBig>0) in = !undermax && overmin;
  if (in) return 0;
  if (!overmin) return -1;
  return 1;
}

#ifdef CAIRO_OUTPUT

void SectorArc::draw(cairo_t* cairo)
{
  double ymin = _CenterSmall.Y();
  double ymax = _CenterBig.Y();
  double rmin = fabs(_RadiusSmall);
  double rmax = fabs(_RadiusBig);
  cairo_save(cairo);
  cairo_rotate(cairo, _RotationAngle);
  if (_RadiusSmall<0 && _RadiusBig>0) {
    cairo_arc(cairo, -_DownShift, ymax, rmax, -M_PI / 2, M_PI / 2);
    cairo_line_to(cairo, -_DownShift, 5000*_RadiusBig / rmax);
    cairo_line_to(cairo, 5000, 5000*_RadiusBig / rmax);
    cairo_line_to(cairo, 5000, 5000*_RadiusSmall / rmin);
    cairo_line_to(cairo, -_DownShift, 5000*_RadiusSmall / rmin);
    cairo_arc(cairo, -_DownShift, ymin, rmin, -M_PI / 2, M_PI / 2);
  } else {
    cairo_arc(cairo, -_DownShift, ymax, rmax, -M_PI / 2, M_PI / 2);
    cairo_arc_negative(cairo, -_DownShift, ymin, rmin, M_PI / 2, -M_PI / 2);
  }
  cairo_close_path(cairo);
  cairo_set_source_rgba(cairo, color[0]*0.6, color[1]*0.6, color[2]*0.6, 0.2);
  cairo_fill_preserve(cairo);
  cairo_set_source_rgb(cairo, color[0]*0.6, color[1]*0.6, color[2]*0.6);
  cairo_stroke(cairo);
  cairo_restore(cairo);

  SectorBasic::draw(cairo);
}
#endif


