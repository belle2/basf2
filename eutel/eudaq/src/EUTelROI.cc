// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
// Author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
// Version $Id: EUTelROI.cc,v 1.1 2007-06-29 15:07:46 bulgheroni Exp $
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */

// eutelescope includes ".h"
#include <eutel/eudaq/EUTelROI.h>

// system <>
#include <limits>
#include <cstdlib>


using namespace std;
using namespace eutelescope;

EUTelROI::EUTelROI(float xBottomLeft, float yBottomLeft, float xTopRight, float yTopRight)  :
  _xBottomLeft(xBottomLeft),  _yBottomLeft(yBottomLeft),
  _xTopRight(xTopRight),      _yTopRight(yTopRight) ,
  _detectorID(std::numeric_limits<int>::min())
{

  consistencyCheck();
}

EUTelROI::EUTelROI(int detectorID, float xBottomLeft, float yBottomLeft, float xTopRight, float yTopRight)  :
  _xBottomLeft(xBottomLeft),  _yBottomLeft(yBottomLeft),
  _xTopRight(xTopRight),      _yTopRight(yTopRight),
  _detectorID(detectorID)
{

  consistencyCheck();

}

void EUTelROI::getCorners(float* xBottomLeft, float* yBottomLeft, float* xTopRight, float* yTopRight) const
{
  *xBottomLeft = _xBottomLeft;
  *yBottomLeft = _yBottomLeft;
  *xTopRight   = _xTopRight;
  *yTopRight   = _yTopRight;
}

int EUTelROI::getDetectorID() const
{
  return _detectorID;
}

bool EUTelROI::isInside(int detectorID, float x, float y) const
{

  if (detectorID != _detectorID) return false;
  return isInside(x, y);

}

bool EUTelROI::isInside(float x, float y) const
{

  if ((x >= _xBottomLeft) && (x <= _xTopRight) &&
      (y >= _yBottomLeft) && (y <= _yTopRight)) {
    return true;
  }

  return false;
}

void EUTelROI::consistencyCheck() const
{

  if (_xBottomLeft > _xTopRight) exit(-1);
  if (_yBottomLeft > _yTopRight) exit(-1);

}


std::ostream& eutelescope::operator<< (std::ostream& os, EUTelROI roi)
{
  if (roi._detectorID != std::numeric_limits<int>::min())
    os << " Detector ID = " << roi._detectorID << endl;

  os << " Bottom left corner (" << roi._xBottomLeft << ", " << roi._yBottomLeft << ")" << endl
     << " Top right corner   (" << roi._xTopRight   << ", " << roi._yTopRight << ")" ;
  return os;
}
