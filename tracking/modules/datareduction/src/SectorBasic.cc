
#include "SectorBasic.h"
#include <cmath>

#include "lcio.h"
#include <EVENT/TrackerHit.h>

using namespace std;


bool SectorBasic::addHit(TrackerHit* hit)
{
  if (isHitInSector(hit)) {
    _hitList.push_back(hit);
    return true;
  } else return false;
}


/*bool SectorBasic::addPXDLadder(PXDLadder* ladder)
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
  unsigned int SectorBasic::getLadderNumber()
{
  return _ladderList.size();
}

void SectorBasic::clearLadders()
{
  _ladderList.clear();
}

bool SectorBasic::isLadderInSector(PXDLadder* ladder)
{
  TVector3 p = ladder->getPosition();
  TVector3 s = ladder->getSize();
  TVector3 n = ladder->getNormal();
  double angle = atan2(n(0),n(1));
  double left_x = cos(angle)*s(0)/2.0;
  double left_y = sin(angle)*s(0)/2.0;

  int left = checkPoint(p(0)-left_x,p(1)+left_y);
  int right = checkPoint(p(0)+left_x,p(1)-left_y);
  return (left==0 || right==0 || left+right==0);
}

void SectorBasic::setIntersection(LadderEntry& ladderEntry)
{
  //Intersect with "bottom" of sector
  TVector3 p = ladderEntry.ladder->getPosition();
  TVector3 s = ladderEntry.ladder->getSize();
  TVector3 n = ladderEntry.ladder->getNormal();
  double width = s(0)/2;
  TVector2 pos(p(0),p(1));
  TVector2 dir(width,0);
  pos = pos.Rotate(-_RotationAngle) + TVector2(_DownShift,0);
  dir = dir.Rotate(n.Phi()-M_PI/2.0-_RotationAngle);
  if(dir.X()==0) return;
  double t = -pos.X()/dir.X();
  double len = (t*dir).Mod();
  if(len < width){
     int d = pos.X()<0?-1:1;
     if(d*t<0) ladderEntry.start = max(ladderEntry.start,(1-d*len/width)/2);
     else      ladderEntry.end = min(ladderEntry.end,(1+d*len/width)/2);

  }
}


void SectorBasic::setMinMaxRadius(LadderEntry& ladderEntry)
{
  TVector3 p = ladderEntry.ladder->getPosition();
  TVector3 s = ladderEntry.ladder->getSize();
  TVector3 n = ladderEntry.ladder->getNormal();

  TVector2 dir(n(1),-n(0));
  dir = dir.Unit();
  TVector2 pos(p(0),p(1));
  TVector2 pos1 = pos + dir * ((ladderEntry.start-0.5)*s(0));
  TVector2 pos2 = pos + dir * ((ladderEntry.end-0.5)*s(0));

  double t = (pos*dir);
  TVector2 tPos = pos - t*dir;

  double ladderRad = tPos.Mod();
  double StartRad = pos1.Mod();
  double EndRad = pos2.Mod();

  if (checkPoint(tPos.X(),tPos.Y()) == 0) ladderEntry.minRadius = min(ladderRad,min(StartRad,EndRad));
  else ladderEntry.minRadius = min(StartRad,EndRad);

  ladderEntry.maxRadius = max(StartRad,EndRad);
}
*/


unsigned int SectorBasic::getHitNumber()
{
  return _hitList.size();
}


void SectorBasic::clearHits()
{
  _hitList.clear();
}

bool SectorBasic::isHitInSector(TrackerHit* hit)
{
  double XPosiSVDDigi = hit->getPosition()[0];
  double YPosiSVDDigi = hit->getPosition()[1];
  return checkPoint(XPosiSVDDigi, YPosiSVDDigi) == 0;
}

int SectorBasic::checkPoint(double x, double y)
{
  return -2;
}

