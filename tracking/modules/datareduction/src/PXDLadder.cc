#include <tracking/modules/datareduction/PXDLadder.h>
#include <cmath>

using namespace std;
using namespace Belle2;

PXDLadder::PXDLadder(TVector3& position, TVector3& normal, TVector3& size)
{
  _position = position;
  _normal = normal;
  _size = size;
}


PXDLadder::~PXDLadder()
{
  clearRegions();
}


PXDLadder& PXDLadder::addRegion(double widthStart, double widthEnd, double lengthStart, double lengthEnd)
{
  _regions.push_back(new RegionOfInterest(widthStart, widthEnd, lengthStart, lengthEnd));
  return *this;
}


void PXDLadder::clearRegions()
{
  list<RegionOfInterest*>::iterator listIter;

  for (listIter = _regions.begin(); listIter != _regions.end(); ++listIter) {
    delete *listIter;
  }
  _regions.clear();
}

TVector3 PXDLadder::convertToLocal(const double* pos)
{
  TVector3 hitPos(pos[0], pos[1], pos[2]);
  hitPos -= _position;
  //Rotate center to local ladder frame
  hitPos.RotateZ(-_normal.Phi() + M_PI / 2.0);
  return hitPos;
}

TVector2 PXDLadder::convertToRelative(const double* pos)
{
  TVector3 hitPos = convertToLocal(pos);
  TVector3 s = 0.5 * _size;
  return TVector2((hitPos(0) + s(0)) / (2 * s(0)), (hitPos(2) + s(2)) / (2 * s(2)));
}

bool PXDLadder::isInLadder(const double* pos)
{
  TVector3 s = 0.5 * _size;
  TVector3 local = convertToLocal(pos);
  bool in(true);
  for (int i = 0; i < 3; ++i) {
    in &= fabs(local(i)) <= s(i);
  }
  return in;
}

