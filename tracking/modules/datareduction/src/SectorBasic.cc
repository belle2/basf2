
#include <tracking/modules/datareduction/SectorBasic.h>
#include <cmath>
#include <tracking/modules/datareduction/TrackerHit.h>

using namespace Belle2;
using namespace std;


bool SectorBasic::addHit(TrackerHit* hit)
{
  if (isHitInSector(hit)) {
    _hitList.push_back(hit);
    return true;
  } else return false;
}


bool SectorBasic::addPXDLadder(PXDLadder* ladder)
{
  if (isLadderInSector(ladder)) {

    LadderEntry entry;
    entry.ladder = ladder;

    setIntersection(entry);
    setMinMaxRadius(entry);
    _ladderList.push_back(entry);

    return true;
  } else return false;
}


unsigned int SectorBasic::getHitNumber()
{
  return _hitList.size();
}


void SectorBasic::clearHits()
{
  _hitList.clear();
}


unsigned int SectorBasic::getLadderNumber()
{
  return _ladderList.size();
}


void SectorBasic::clearLadders()
{
  _ladderList.clear();
}


bool SectorBasic::isHitInSector(TrackerHit* hit)
{
  double XPosiSVDDigi = hit->getPosition()[0];
  double YPosiSVDDigi = hit->getPosition()[1];
  return checkPoint(XPosiSVDDigi, YPosiSVDDigi) == 0;
}


bool SectorBasic::isLadderInSector(PXDLadder* ladder)
{
  TVector3 p = ladder->getPosition();
  TVector3 s = ladder->getSize();
  TVector3 n = ladder->getNormal();
  double angle = atan2(n(0), n(1));
  double left_x = cos(angle) * s(0) / 2.0;
  double left_y = sin(angle) * s(0) / 2.0;

  int left = checkPoint(p(0) - left_x, p(1) + left_y);
  int right = checkPoint(p(0) + left_x, p(1) - left_y);
  return (left == 0 || right == 0 || left + right == 0);
}


int SectorBasic::checkPoint(double x, double y)
{
  return -2;
}


void SectorBasic::setIntersection(LadderEntry& ladderEntry)
{
  //Intersect with "bottom" of sector
  TVector3 p = ladderEntry.ladder->getPosition();
  TVector3 s = ladderEntry.ladder->getSize();
  TVector3 n = ladderEntry.ladder->getNormal();
  double width = s(0) / 2;
  TVector2 pos(p(0), p(1));
  TVector2 dir(width, 0);
  pos = pos.Rotate(-_RotationAngle) + TVector2(_DownShift, 0);
  dir = dir.Rotate(n.Phi() - M_PI / 2.0 - _RotationAngle);
  if (dir.X() == 0) return;
  double t = -pos.X() / dir.X();
  double len = (t * dir).Mod();
  if (len < width) {
    int d = pos.X() < 0 ? -1 : 1;
    if (d*t < 0) ladderEntry.start = max(ladderEntry.start, (1 - d * len / width) / 2);
    else      ladderEntry.end = min(ladderEntry.end, (1 + d * len / width) / 2);

  }
}


void SectorBasic::setMinMaxRadius(LadderEntry& ladderEntry)
{
  TVector3 p = ladderEntry.ladder->getPosition();
  TVector3 s = ladderEntry.ladder->getSize();
  TVector3 n = ladderEntry.ladder->getNormal();

  TVector2 dir(n(1), -n(0));
  dir = dir.Unit();
  TVector2 pos(p(0), p(1));
  TVector2 pos1 = pos + dir * ((ladderEntry.start - 0.5) * s(0));
  TVector2 pos2 = pos + dir * ((ladderEntry.end - 0.5) * s(0));

  double t = (pos * dir);
  TVector2 tPos = pos - t * dir;

  double ladderRad = tPos.Mod();
  double StartRad = pos1.Mod();
  double EndRad = pos2.Mod();

  if (checkPoint(tPos.X(), tPos.Y()) == 0) ladderEntry.minRadius = min(ladderRad, min(StartRad, EndRad));
  else ladderEntry.minRadius = min(StartRad, EndRad);

  ladderEntry.maxRadius = max(StartRad, EndRad);
}


#ifdef CAIRO_OUTPUT
#include <boost/format.hpp>
/*
#include <boost/random.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/nondet_random.hpp>
#include "marlin/Processor.h"
#include <iomanip>*/

string SectorBasic::name()
{
  return (boost::format("%.2f°, down=%.2f mm, width=%.2f mm") % (_RotationAngle / M_PI*180) % _DownShift % (_SectorWidth*2)).str();
}

void SectorBasic::makePoint(cairo_t* cairo, double x, double y, int status)
{
  if (status <= -10) status = checkPoint(x, y);
  switch (status) {
    case -2:
      cairo_set_source_rgb(cairo, 1, 0, 1);
      break;
    case -1:
      cairo_set_source_rgb(cairo, 1, 0, 0);
      break;
    case 0:
      cairo_set_source_rgb(cairo, 0, 1, 0);
      break;
    case 1:
      cairo_set_source_rgb(cairo, 0, 0, 1);
      break;
  }
  cairo_arc(cairo, x, y, 1, 0, 2*M_PI);
  cairo_fill(cairo);
}

void SectorBasic::draw(cairo_t* cairo)
{
  cairo_save(cairo);

  /*boost::mt19937 rnd_gen;
  boost::variate_generator<boost::mt19937, boost::uniform_int<> > rndm(rnd_gen,boost::uniform_int<>(-150,150));
  boost::variate_generator<boost::mt19937, boost::uniform_int<> > rndm_phi(rnd_gen,boost::uniform_int<>(0,360));
  for(int i=0; i<10000; i++){
    double x = rndm();
    double y = rndm();
    makePoint(cairo,x,y);
  }
  for(int i=0; i<5000; i++){
    TVector3 p(rndm(),rndm(),0);
    TVector3 n(0,1,0);
    n.RotateZ(rndm_phi()/180.0*M_PI);
    TVector3 s(20,0,0);
    LadderEntry entry;
    addPXDLadder(new PXDLadder(p,n,s));
  }*/

  for (list<LadderEntry>::iterator it = _ladderList.begin(); it != _ladderList.end(); it++) {
    PXDLadder &ladder = *(it->ladder);
    TVector3 p = ladder.getPosition();
    TVector3 s = ladder.getSize();
    TVector3 n = ladder.getNormal();
    double angle = atan2(n(1), n(0)) - M_PI / 2.0;
    double width = s(0) / 2.0;
    cairo_save(cairo);
    cairo_translate(cairo, p(0), p(1));
    cairo_rotate(cairo, angle);
    cairo_set_source_rgb(cairo, color[0], color[1], color[2]);
    cairo_move_to(cairo, -width + 2*width*it->start, 0);
    cairo_line_to(cairo, -width + 2*width*it->end, 0);
    cairo_stroke(cairo);

    /*cairo_move_to(cairo,-width,0);
    cairo_line_to(cairo,+width,0);
    cairo_stroke(cairo);
    cairo_set_line_width(cairo,0.2);
    cairo_set_source_rgb(cairo,0.8,0,0);
    cairo_move_to(cairo,-width+2*width*it->start-1,1);
    cairo_line_to(cairo,-width+2*width*it->start,0);
    cairo_line_to(cairo,-width+2*width*it->start-1,-1);
    cairo_stroke(cairo);
    cairo_set_source_rgb(cairo,0,0,0.8);
    cairo_move_to(cairo,-width+2*width*it->end+1,1);
    cairo_line_to(cairo,-width+2*width*it->end,0);
    cairo_line_to(cairo,-width+2*width*it->end+1,-1);
    cairo_stroke(cairo);*/

    cairo_restore(cairo);
  }
  cairo_set_source_rgb(cairo, color[0], color[1], color[2]);
  for (list<TrackerHit*>::iterator it = _hitList.begin(); it != _hitList.end(); it++) {
    TrackerHit &hit = **it;
    const double* pos = hit.getPosition();
    cairo_arc(cairo, pos[0], pos[1], 2, 0, 2*M_PI);
    cairo_close_path(cairo);
    cairo_fill(cairo);
  }
  cairo_restore(cairo);
}
#endif
