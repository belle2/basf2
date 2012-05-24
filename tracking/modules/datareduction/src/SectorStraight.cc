#include <tracking/modules/datareduction/SectorStraight.h>
#include <tracking/modules/datareduction/HoughTransformStraight.h>

#include <framework/logging/Logger.h>
#include <cmath>

#include <TVector2.h>

using namespace std;
using namespace Belle2;


SectorStraight::SectorStraight(double RotationAngle, double DownShift, double SectorWidth, double OpeningAngle):
  SectorBasic(RotationAngle, DownShift, SectorWidth)
{
  _OpeningAngle = OpeningAngle;
}


SectorStraight::~SectorStraight()
{

}


pair<double, double> SectorStraight::intersectLine(PXDLadder& ladder, TVector2 pos, TVector2 dir, bool bigger)
{
  //Intersect ladder with line by transforming line in local ladder coordinates
  TVector3 p = ladder.getPosition();
  TVector3 s = ladder.getSize();
  TVector3 n = ladder.getNormal();
  double width = s(0) / 2;

  //Shift to local ladder frame
  pos = pos.Rotate(_RotationAngle) + TVector2(-p(0), -p(1));
  dir = dir.Rotate(_RotationAngle);
  //Rotate center to local ladder frame
  pos = pos.Rotate(-n.Phi() + M_PI / 2.0);
  dir = dir.Rotate(-n.Phi() + M_PI / 2.0);

  //If parallel, no intersection
  if (fabs(dir.Y()) < 1e-6) return pair<double, double>(0, 1);

  //Calculate intersection in range 0,1
  double t = -pos.Y() / dir.Y();
  TVector2 intersect = pos + (t * dir);
  double x = (intersect.X() + width) / (2 * width);

  double start = 0;
  double end = 1;

  //Multiplicative conditions:
  // - is line minimum or maximum
  int b = bigger ? 1 : -1;
  // - line ascending or descending
  int a = dir.Phi() < M_PI ? -1 : 1;

  if (a * b > 0) {
    if (x > 0 && x < 1) start = x;
  } else {
    if (x > 0 && x < 1) end = x;
  }
  return pair<double, double>(start, end);
}

void SectorStraight::setIntersection(LadderEntry& ladderEntry)
{
  TVector2 pSmall(-_DownShift, _SectorWidth);
  TVector2 pBig(-_DownShift, -_SectorWidth);
  TVector2 dSmall = TVector2(1, 0).Rotate(_OpeningAngle / 2.0);
  TVector2 dBig = TVector2(1, 0).Rotate(-_OpeningAngle / 2.0);

  pair<double, double> imin = intersectLine(*(ladderEntry.ladder), pSmall, dSmall, false);
  pair<double, double> imax = intersectLine(*(ladderEntry.ladder), pBig, dBig, true);
  ladderEntry.start = max(imin.first, imax.first);
  ladderEntry.end = min(imin.second, imax.second);

  SectorBasic::setIntersection(ladderEntry);
}


int SectorStraight::checkPoint(double x, double y)
{
  double xrot = x * cos(-_RotationAngle) - y * sin(-_RotationAngle) + _DownShift;
  //Wrong side, return -2 to signal not using ever
  if (xrot < 0) return -2;
  double yrot = x * sin(-_RotationAngle) + y * cos(-_RotationAngle);

  double m = tan(_OpeningAngle / 2.0);
  double ymin = xrot * m + _SectorWidth;
  if (fabs(yrot) < +ymin) return 0;
  if (yrot > ymin) return 1;
  return -1;
}

#ifdef CAIRO_OUTPUT
void SectorStraight::draw(cairo_t* cairo)
{
  cairo_save(cairo);
  cairo_rotate(cairo, _RotationAngle);
  //cairo_arc(cairo,-_DownShift,0,500,-_OpeningAngle/2.,_OpeningAngle/2.);
  cairo_move_to(cairo, -_DownShift, _SectorWidth);
  cairo_rel_line_to(cairo, 500, 500 * tan(_OpeningAngle / 2.0));
  cairo_rel_line_to(cairo, 0, -1000 * tan(_OpeningAngle / 2.0) - 2 * _SectorWidth);
  cairo_line_to(cairo, -_DownShift, -_SectorWidth);
  cairo_close_path(cairo);
  cairo_set_source_rgba(cairo, color[0] * 0.6, color[1] * 0.6, color[2] * 0.6, 0.2);
  cairo_fill_preserve(cairo);
  cairo_set_source_rgb(cairo, color[0] * 0.6, color[1] * 0.6, color[2] * 0.6);
  cairo_stroke(cairo);
  cairo_restore(cairo);

  SectorBasic::draw(cairo);
}
#endif
